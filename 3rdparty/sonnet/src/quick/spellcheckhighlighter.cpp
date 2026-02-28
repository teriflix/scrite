// SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
// SPDX-FileCopyrightText: 2020 Christian Mollekopf <mollekopf@kolabsystems.com>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "spellcheckhighlighter.h"
#include "guesslanguage.h"
#include "languagefilter_p.h"
#include "loader_p.h"
#include "settingsimpl_p.h"
#include "speller.h"
#include "tokenizer_p.h"

#include "quick_debug.h"

#include <QColor>
#include <QHash>
#include <QKeyEvent>
#include <QMetaMethod>
#include <QTextBoundaryFinder>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTimer>
#include <memory>

using namespace Sonnet;

// Cache of previously-determined languages (when using AutoDetectLanguage)
// There is one such cache per block (paragraph)
class LanguageCache : public QTextBlockUserData
{
public:
    // Key: QPair<start, length>
    // Value: language name
    QMap<QPair<int, int>, QString> languages;

    // Remove all cached language information after @p pos
    void invalidate(int pos)
    {
        QMutableMapIterator<QPair<int, int>, QString> it(languages);
        it.toBack();
        while (it.hasPrevious()) {
            it.previous();
            if (it.key().first + it.key().second >= pos) {
                it.remove();
            } else {
                break;
            }
        }
    }

    QString languageAtPos(int pos) const
    {
        // The data structure isn't really great for such lookups...
        QMapIterator<QPair<int, int>, QString> it(languages);
        while (it.hasNext()) {
            it.next();
            if (it.key().first <= pos && it.key().first + it.key().second >= pos) {
                return it.value();
            }
        }
        return QString();
    }
};

class HighlighterPrivate
{
public:
    HighlighterPrivate(SpellcheckHighlighter *qq)
        : q(qq)
    {
        tokenizer = std::make_unique<WordTokenizer>();
        active = true;
        automatic = false;
        autoDetectLanguageDisabled = false;
        connected = false;
        wordCount = 0;
        errorCount = 0;
        intraWordEditing = false;
        completeRehighlightRequired = false;
        spellColor = spellColor.isValid() ? spellColor : Qt::red;
        languageFilter = std::make_unique<LanguageFilter>(new SentenceTokenizer());

        loader = Loader::openLoader();
        loader->settings()->restore();

        spellchecker = std::make_unique<Speller>();
        spellCheckerFound = spellchecker->isValid();
        rehighlightRequest = new QTimer(q);
        q->connect(rehighlightRequest, &QTimer::timeout, q, &SpellcheckHighlighter::slotRehighlight);

        if (!spellCheckerFound) {
            return;
        }

        disablePercentage = loader->settings()->disablePercentageWordError();
        disableWordCount = loader->settings()->disableWordErrorCount();

        completeRehighlightRequired = true;
        rehighlightRequest->setInterval(0);
        rehighlightRequest->setSingleShot(true);
        rehighlightRequest->start();

        // Danger red from our color scheme
        errorFormat.setForeground(spellColor);
        errorFormat.setUnderlineColor(spellColor);
        errorFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

        selectedErrorFormat.setForeground(spellColor);
        auto bg = spellColor;
        bg.setAlphaF(0.1);
        selectedErrorFormat.setBackground(bg);
        selectedErrorFormat.setUnderlineColor(spellColor);
        selectedErrorFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

        quoteFormat.setForeground(QColor{"#7f8c8d"});
    }

    ~HighlighterPrivate();
    std::unique_ptr<WordTokenizer> tokenizer;
    std::unique_ptr<LanguageFilter> languageFilter;
    Loader *loader = nullptr;
    std::unique_ptr<Speller> spellchecker;

    QTextCharFormat errorFormat;
    QTextCharFormat selectedErrorFormat;
    QTextCharFormat quoteFormat;
    std::unique_ptr<Sonnet::GuessLanguage> languageGuesser;
    QString selectedWord;
    QQuickTextDocument *document = nullptr;
    int cursorPosition = 0;
    int selectionStart = 0;
    int selectionEnd = 0;

    int autoCompleteBeginPosition = -1;
    int autoCompleteEndPosition = -1;
    int wordIsMisspelled = false;
    bool active = false;
    bool automatic = false;
    bool autoDetectLanguageDisabled = false;
    bool completeRehighlightRequired = false;
    bool intraWordEditing = false;
    bool spellCheckerFound = false; // cached d->dict->isValid() value
    bool connected = false;
    int disablePercentage = 0;
    int disableWordCount = 0;
    int wordCount = 0;
    int errorCount = 0;
    QTimer *rehighlightRequest = nullptr;
    QColor spellColor;
    SpellcheckHighlighter *const q;
};

HighlighterPrivate::~HighlighterPrivate()
{
}

SpellcheckHighlighter::SpellcheckHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
    , d(new HighlighterPrivate(this))
{
}

SpellcheckHighlighter::~SpellcheckHighlighter()
{
    if (document()) {
        disconnect(document(), nullptr, this, nullptr);
    }
}

bool SpellcheckHighlighter::spellCheckerFound() const
{
    return d->spellCheckerFound;
}

void SpellcheckHighlighter::slotRehighlight()
{
    if (d->completeRehighlightRequired) {
        d->wordCount = 0;
        d->errorCount = 0;
        rehighlight();
    } else {
        // rehighlight the current para only (undo/redo safe)
        QTextCursor cursor = textCursor();
        if (cursor.hasSelection()) {
            cursor.clearSelection();
        }
        cursor.insertText(QString());
    }
    // if (d->checksDone == d->checksRequested)
    // d->completeRehighlightRequired = false;
    QTimer::singleShot(0, this, &SpellcheckHighlighter::slotAutoDetection);
}

bool SpellcheckHighlighter::automatic() const
{
    return d->automatic;
}

bool SpellcheckHighlighter::autoDetectLanguageDisabled() const
{
    return d->autoDetectLanguageDisabled;
}

bool SpellcheckHighlighter::intraWordEditing() const
{
    return d->intraWordEditing;
}

void SpellcheckHighlighter::setIntraWordEditing(bool editing)
{
    d->intraWordEditing = editing;
}

void SpellcheckHighlighter::setAutomatic(bool automatic)
{
    if (automatic == d->automatic) {
        return;
    }

    d->automatic = automatic;
    if (d->automatic) {
        slotAutoDetection();
    }
}

void SpellcheckHighlighter::setAutoDetectLanguageDisabled(bool autoDetectDisabled)
{
    d->autoDetectLanguageDisabled = autoDetectDisabled;
}

void SpellcheckHighlighter::slotAutoDetection()
{
    bool savedActive = d->active;

    // don't disable just because 1 of 4 is misspelled.
    if (d->automatic && d->wordCount >= 10) {
        // tme = Too many errors
        /* clang-format off */
        bool tme = (d->errorCount >= d->disableWordCount)
                   && (d->errorCount * 100 >= d->disablePercentage * d->wordCount);
        /* clang-format on */

        if (d->active && tme) {
            d->active = false;
        } else if (!d->active && !tme) {
            d->active = true;
        }
    }

    if (d->active != savedActive) {
        if (d->active) {
            Q_EMIT activeChanged(tr("As-you-type spell checking enabled."));
        } else {
            qCDebug(SONNET_LOG_QUICK) << "Sonnet: Disabling spell checking, too many errors";
            Q_EMIT activeChanged(
                tr("Too many misspelled words. "
                   "As-you-type spell checking disabled."));
        }

        d->completeRehighlightRequired = true;
        d->rehighlightRequest->setInterval(100);
        d->rehighlightRequest->setSingleShot(true);
    }
}

void SpellcheckHighlighter::setActive(bool active)
{
    if (active == d->active) {
        return;
    }
    d->active = active;
    Q_EMIT activeChanged();
    rehighlight();

    if (d->active) {
        Q_EMIT activeChanged(tr("As-you-type spell checking enabled."));
    } else {
        Q_EMIT activeChanged(tr("As-you-type spell checking disabled."));
    }
}

bool SpellcheckHighlighter::active() const
{
    return d->active;
}

static bool hasNotEmptyText(const QString &text)
{
    for (int i = 0; i < text.length(); ++i) {
        if (!text.at(i).isSpace()) {
            return true;
        }
    }
    return false;
}

void SpellcheckHighlighter::contentsChange(int pos, int add, int rem)
{
    // Invalidate the cache where the text has changed
    const QTextBlock &lastBlock = document()->findBlock(pos + add - rem);
    QTextBlock block = document()->findBlock(pos);
    do {
        LanguageCache *cache = dynamic_cast<LanguageCache *>(block.userData());
        if (cache) {
            cache->invalidate(pos - block.position());
        }
        block = block.next();
    } while (block.isValid() && block < lastBlock);
}

void SpellcheckHighlighter::highlightBlock(const QString &text)
{
    if (!hasNotEmptyText(text) || !d->active || !d->spellCheckerFound) {
        return;
    }

    // Avoid spellchecking quotes
    if (text.isEmpty() || text.at(0) == QLatin1Char('>')) {
        setFormat(0, text.length(), d->quoteFormat);
        return;
    }

    if (!d->connected) {
        connect(textDocument(), &QTextDocument::contentsChange, this, &SpellcheckHighlighter::contentsChange);
        d->connected = true;
    }
    QTextCursor cursor = textCursor();
    const int index = cursor.position() + 1;

    const int lengthPosition = text.length() - 1;

    if (index != lengthPosition //
        || (lengthPosition > 0 && !text[lengthPosition - 1].isLetter())) {
        d->languageFilter->setBuffer(text);

        LanguageCache *cache = dynamic_cast<LanguageCache *>(currentBlockUserData());
        if (!cache) {
            cache = new LanguageCache;
            setCurrentBlockUserData(cache);
        }

        const bool autodetectLanguage = d->spellchecker->testAttribute(Speller::AutoDetectLanguage);
        while (d->languageFilter->hasNext()) {
            Sonnet::Token sentence = d->languageFilter->next();
            if (autodetectLanguage && !d->autoDetectLanguageDisabled) {
                QString lang;
                QPair<int, int> spos = QPair<int, int>(sentence.position(), sentence.length());
                // try cache first
                if (cache->languages.contains(spos)) {
                    lang = cache->languages.value(spos);
                } else {
                    lang = d->languageFilter->language();
                    if (!d->languageFilter->isSpellcheckable()) {
                        lang.clear();
                    }
                    cache->languages[spos] = lang;
                }
                if (lang.isEmpty()) {
                    continue;
                }
                d->spellchecker->setLanguage(lang);
            }

            d->tokenizer->setBuffer(sentence.toString());
            int offset = sentence.position();
            while (d->tokenizer->hasNext()) {
                Sonnet::Token word = d->tokenizer->next();
                if (!d->tokenizer->isSpellcheckable()) {
                    continue;
                }
                ++d->wordCount;
                if (d->spellchecker->isMisspelled(word.toString())) {
                    ++d->errorCount;
                    if (word.position() + offset <= cursor.position() && cursor.position() <= word.position() + offset + word.length()) {
                        setMisspelledSelected(word.position() + offset, word.length());
                    } else {
                        setMisspelled(word.position() + offset, word.length());
                    }
                } else {
                    unsetMisspelled(word.position() + offset, word.length());
                }
            }
        }
    }
    // QTimer::singleShot( 0, this, SLOT(checkWords()) );
    setCurrentBlockState(0);
}

QStringList SpellcheckHighlighter::suggestions(int mousePosition, int max)
{
    if (!textDocument()) {
        return {};
    }

    Q_EMIT changeCursorPosition(mousePosition, mousePosition);

    QTextCursor cursor = textCursor();

    QTextCursor cursorAtMouse(textDocument());
    cursorAtMouse.setPosition(mousePosition);

    // Check if the user clicked a selected word
    const bool selectedWordClicked = cursor.hasSelection() && mousePosition >= cursor.selectionStart() && mousePosition <= cursor.selectionEnd();

    // Get the word under the (mouse-)cursor and see if it is misspelled.
    // Don't include apostrophes at the start/end of the word in the selection.
    QTextCursor wordSelectCursor(cursorAtMouse);
    wordSelectCursor.clearSelection();
    wordSelectCursor.select(QTextCursor::WordUnderCursor);
    d->selectedWord = wordSelectCursor.selectedText();

    // Clear the selection again, we re-select it below (without the apostrophes).
    wordSelectCursor.setPosition(wordSelectCursor.position() - d->selectedWord.size());
    if (d->selectedWord.startsWith(QLatin1Char('\'')) || d->selectedWord.startsWith(QLatin1Char('\"'))) {
        d->selectedWord = d->selectedWord.right(d->selectedWord.size() - 1);
        wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
    }
    if (d->selectedWord.endsWith(QLatin1Char('\'')) || d->selectedWord.endsWith(QLatin1Char('\"'))) {
        d->selectedWord.chop(1);
    }

    wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d->selectedWord.size());

    Q_EMIT wordUnderMouseChanged();

    bool isMouseCursorInsideWord = true;
    if ((mousePosition < wordSelectCursor.selectionStart() || mousePosition >= wordSelectCursor.selectionEnd()) //
        && (d->selectedWord.length() > 1)) {
        isMouseCursorInsideWord = false;
    }

    wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d->selectedWord.size());

    d->wordIsMisspelled = isMouseCursorInsideWord && !d->selectedWord.isEmpty() && d->spellchecker->isMisspelled(d->selectedWord);
    Q_EMIT wordIsMisspelledChanged();

    if (!d->wordIsMisspelled || selectedWordClicked) {
        return QStringList{};
    }

    LanguageCache *cache = dynamic_cast<LanguageCache *>(cursor.block().userData());
    if (cache) {
        const QString cachedLanguage = cache->languageAtPos(cursor.positionInBlock());
        if (!cachedLanguage.isEmpty()) {
            d->spellchecker->setLanguage(cachedLanguage);
        }
    }
    QStringList suggestions = d->spellchecker->suggest(d->selectedWord);
    if (max >= 0 && suggestions.count() > max) {
        suggestions = suggestions.mid(0, max);
    }

    return suggestions;
}

QString SpellcheckHighlighter::currentLanguage() const
{
    return d->spellchecker->language();
}

void SpellcheckHighlighter::setCurrentLanguage(const QString &lang)
{
    QString prevLang = d->spellchecker->language();
    d->spellchecker->setLanguage(lang);
    d->spellCheckerFound = d->spellchecker->isValid();
    if (!d->spellCheckerFound) {
        qCDebug(SONNET_LOG_QUICK) << "No dictionary for \"" << lang << "\" staying with the current language.";
        d->spellchecker->setLanguage(prevLang);
        return;
    }
    d->wordCount = 0;
    d->errorCount = 0;
    if (d->automatic || d->active) {
        d->rehighlightRequest->start(0);
    }
}

void SpellcheckHighlighter::setMisspelled(int start, int count)
{
    setFormat(start, count, d->errorFormat);
}

void SpellcheckHighlighter::setMisspelledSelected(int start, int count)
{
    setFormat(start, count, d->selectedErrorFormat);
}

void SpellcheckHighlighter::unsetMisspelled(int start, int count)
{
    setFormat(start, count, QTextCharFormat());
}

void SpellcheckHighlighter::addWordToDictionary(const QString &word)
{
    d->spellchecker->addToPersonal(word);
    rehighlight();
}

void SpellcheckHighlighter::ignoreWord(const QString &word)
{
    d->spellchecker->addToSession(word);
    rehighlight();
}

void SpellcheckHighlighter::replaceWord(const QString &replacement, int at)
{
    QTextCursor textCursorUnderUserCursor(textDocument());
    textCursorUnderUserCursor.setPosition(at == -1 ? d->cursorPosition : at);

    // Get the word under the cursor
    QTextCursor wordSelectCursor(textCursorUnderUserCursor);
    wordSelectCursor.clearSelection();
    wordSelectCursor.select(QTextCursor::WordUnderCursor);

    auto selectedWord = wordSelectCursor.selectedText();

    // Trim leading and trailing apostrophes
    wordSelectCursor.setPosition(wordSelectCursor.position() - selectedWord.size());
    if (selectedWord.startsWith(QLatin1Char('\'')) || selectedWord.startsWith(QLatin1Char('\"'))) {
        selectedWord = selectedWord.right(selectedWord.size() - 1);
        wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
    }
    if (selectedWord.endsWith(QLatin1Char('\'')) || d->selectedWord.endsWith(QLatin1Char('\"'))) {
        selectedWord.chop(1);
    }

    wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d->selectedWord.size());

    wordSelectCursor.insertText(replacement);
}

QQuickTextDocument *SpellcheckHighlighter::quickDocument() const
{
    return d->document;
}

void SpellcheckHighlighter::setQuickDocument(QQuickTextDocument *document)
{
    if (document == d->document) {
        return;
    }

    if (d->document) {
        d->document->parent()->removeEventFilter(this);
        d->document->textDocument()->disconnect(this);
    }
    d->document = document;
    document->parent()->installEventFilter(this);
    setDocument(document->textDocument());
    Q_EMIT documentChanged();
}

void SpellcheckHighlighter::setDocument(QTextDocument *document)
{
    d->connected = false;
    QSyntaxHighlighter::setDocument(document);
}

int SpellcheckHighlighter::cursorPosition() const
{
    return d->cursorPosition;
}

void SpellcheckHighlighter::setCursorPosition(int position)
{
    if (position == d->cursorPosition) {
        return;
    }

    d->cursorPosition = position;
    d->rehighlightRequest->start(0);
    Q_EMIT cursorPositionChanged();
}

int SpellcheckHighlighter::selectionStart() const
{
    return d->selectionStart;
}

void SpellcheckHighlighter::setSelectionStart(int position)
{
    if (position == d->selectionStart) {
        return;
    }

    d->selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int SpellcheckHighlighter::selectionEnd() const
{
    return d->selectionEnd;
}

void SpellcheckHighlighter::setSelectionEnd(int position)
{
    if (position == d->selectionEnd) {
        return;
    }

    d->selectionEnd = position;
    Q_EMIT selectionEndChanged();
}

QTextCursor SpellcheckHighlighter::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc) {
        return QTextCursor();
    }

    QTextCursor cursor(doc);
    if (d->selectionStart != d->selectionEnd) {
        cursor.setPosition(d->selectionStart);
        cursor.setPosition(d->selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(d->cursorPosition);
    }
    return cursor;
}

QTextDocument *SpellcheckHighlighter::textDocument() const
{
    if (!d->document) {
        return nullptr;
    }

    return d->document->textDocument();
}

bool SpellcheckHighlighter::wordIsMisspelled() const
{
    return d->wordIsMisspelled;
}

QString SpellcheckHighlighter::wordUnderMouse() const
{
    return d->selectedWord;
}

QColor SpellcheckHighlighter::misspelledColor() const
{
    return d->spellColor;
}

void SpellcheckHighlighter::setMisspelledColor(const QColor &color)
{
    if (color == d->spellColor) {
        return;
    }
    d->spellColor = color;
    Q_EMIT misspelledColorChanged();
}

bool SpellcheckHighlighter::isWordMisspelled(const QString &word)
{
    return d->spellchecker->isMisspelled(word);
}

bool SpellcheckHighlighter::eventFilter(QObject *o, QEvent *e)
{
    if (!d->spellCheckerFound) {
        return false;
    }
    if (o == d->document->parent() && (e->type() == QEvent::KeyPress)) {
        QKeyEvent *k = static_cast<QKeyEvent *>(e);

        if (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return || k->key() == Qt::Key_Up || k->key() == Qt::Key_Down || k->key() == Qt::Key_Left
            || k->key() == Qt::Key_Right || k->key() == Qt::Key_PageUp || k->key() == Qt::Key_PageDown || k->key() == Qt::Key_Home || k->key() == Qt::Key_End
            || (k->modifiers() == Qt::ControlModifier
                && (k->key() == Qt::Key_A || k->key() == Qt::Key_B || k->key() == Qt::Key_E || k->key() == Qt::Key_N
                    || k->key() == Qt::Key_P))) { /* clang-format on */
            if (intraWordEditing()) {
                setIntraWordEditing(false);
                d->completeRehighlightRequired = true;
                d->rehighlightRequest->setInterval(500);
                d->rehighlightRequest->setSingleShot(true);
                d->rehighlightRequest->start();
            }
        } else {
            setIntraWordEditing(true);
        }
        if (k->key() == Qt::Key_Space //
            || k->key() == Qt::Key_Enter //
            || k->key() == Qt::Key_Return) {
            QTimer::singleShot(0, this, SLOT(slotAutoDetection()));
        }
    } else if (d->document && e->type() == QEvent::MouseButtonPress) {
        if (intraWordEditing()) {
            setIntraWordEditing(false);
            d->completeRehighlightRequired = true;
            d->rehighlightRequest->setInterval(0);
            d->rehighlightRequest->setSingleShot(true);
            d->rehighlightRequest->start();
        }
    }
    return false;
}

#include "moc_spellcheckhighlighter.cpp"
