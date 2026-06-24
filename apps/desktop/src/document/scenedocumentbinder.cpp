/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "utils.h"
#include "fountain.h"
#include "screenplayformat.h"
#include "scenedocumentbinder_p.h"
#include "application.h"
#include "languageengine.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "qobjectserializer.h"
#include "scenedocumentbinder.h"
#include "spellcheckservice.h"

#include <QPointer>
#include <QMarginsF>
#include <QSettings>
#include <QMetaEnum>
#include <QMimeData>
#include <QClipboard>
#include <QPdfWriter>
#include <QScopeGuard>
#include <QTextCursor>
#include <QPageLayout>
#include <QJsonObject>
#include <QStyleHints>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QTextBoundaryFinder>
#include <QScopedValueRollback>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>

Q_DECLARE_METATYPE(QTextCharFormat)

TextFormat::TextFormat(QObject *parent) : QObject(parent) { }

TextFormat::~TextFormat() { }

void TextFormat::setBold(bool val)
{
    if (m_bold == val)
        return;

    m_bold = val;
    emit boldChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontWeight });
}

void TextFormat::setItalics(bool val)
{
    if (m_italics == val)
        return;

    m_italics = val;
    emit italicsChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontItalic });
}

void TextFormat::setUnderline(bool val)
{
    if (m_underline == val)
        return;

    m_underline = val;
    emit underlineChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontUnderline, QTextFormat::TextUnderlineStyle });
}

void TextFormat::setStrikeout(bool val)
{
    if (m_strikeout == val)
        return;

    m_strikeout = val;
    emit strikeoutChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontStrikeOut });
}

void TextFormat::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::ForegroundBrush });
}

void TextFormat::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::BackgroundBrush });
}

void TextFormat::setLink(const QString &val)
{
    if (m_link == val)
        return;

    m_link = val;
    emit linkChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::AnchorHref });
}

void TextFormat::reset()
{
    m_updatingFromFormat = true;

    this->resetTextColor();
    this->resetBackgroundColor();
    this->setBold(false);
    this->setItalics(false);
    this->setUnderline(false);
    this->setLink(QString());

    m_updatingFromFormat = false;

    emit formatChanged(allProperties());
}

void TextFormat::updateFromCharFormat(const QTextCharFormat &format)
{
    if (m_updatingFromFormat)
        return;

    QScopedValueRollback<bool> rollback(m_updatingFromFormat);
    m_updatingFromFormat = true;

    if (format.hasProperty(QTextFormat::ForegroundBrush))
        this->setTextColor(format.foreground().color());
    else
        this->resetTextColor();

    if (format.hasProperty(QTextFormat::BackgroundBrush))
        this->setBackgroundColor(format.background().color());
    else
        this->resetBackgroundColor();

    if (format.hasProperty(QTextFormat::FontWeight))
        this->setBold(format.fontWeight() == QFont::Bold);
    else
        this->setBold(false);

    if (format.hasProperty(QTextFormat::FontItalic))
        this->setItalics(format.fontItalic());
    else
        this->setItalics(false);

    if (format.hasProperty(QTextFormat::TextUnderlineStyle))
        this->setUnderline(format.fontUnderline());
    else
        this->setUnderline(false);

    if (format.hasProperty(QTextFormat::FontStrikeOut))
        this->setStrikeout(format.fontStrikeOut());
    else
        this->setStrikeout(false);

    if (format.isAnchor() && format.hasProperty(QTextFormat::AnchorHref))
        this->setLink(format.anchorHref());
    else
        this->setLink(QString());
}

QTextCharFormat TextFormat::toCharFormat(const QList<int> &properties) const
{
    QTextCharFormat format;

    if (properties.isEmpty() || properties.contains(QTextFormat::ForegroundBrush))
        if (m_textColor != Qt::transparent)
            format.setForeground(m_textColor);

    if (properties.isEmpty() || properties.contains(QTextFormat::BackgroundBrush))
        if (m_backgroundColor != Qt::transparent)
            format.setBackground(m_backgroundColor);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontWeight))
        if (m_bold)
            format.setFontWeight(QFont::Bold);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontItalic))
        if (m_italics)
            format.setFontItalic(m_italics);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontUnderline)
        || properties.contains(QTextFormat::TextUnderlineStyle))
        if (m_underline)
            format.setFontUnderline(m_underline);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontStrikeOut))
        if (m_strikeout)
            format.setFontStrikeOut(m_strikeout);

    if (properties.isEmpty() || properties.contains(QTextFormat::AnchorHref)) {
        if (!m_link.isEmpty()) {
            format.setAnchor(true);
            format.setAnchorHref(m_link);
        } else {
            format.setAnchor(false);
            format.setAnchorHref(QString());
        }
    }

    return format;
}

QList<int> TextFormat::allProperties()
{
    return { QTextFormat::ForegroundBrush,    QTextFormat::BackgroundBrush,
             QTextFormat::FontWeight,         QTextFormat::FontItalic,
             QTextFormat::FontUnderline,      QTextFormat::FontStrikeOut,
             QTextFormat::TextUnderlineStyle, QTextFormat::AnchorHref };
}

///////////////////////////////////////////////////////////////////////////////

class BlockKeyStrokes : public QObject
{
public:
    BlockKeyStrokes() { qApp->installEventFilter(this); }
    ~BlockKeyStrokes() { qApp->removeEventFilter(this); }

protected:
    bool eventFilter(QObject *watched, QEvent *event)
    {
        Q_UNUSED(watched);
        if (QList<int>({ QEvent::KeyPress, QEvent::KeyRelease, QEvent::ShortcutOverride,
                         QEvent::Shortcut })
                    .contains(event->type()))
            return true;
        return false;
    }
};

SceneDocumentBlockUserData::SceneDocumentBlockUserData(const QTextBlock &textBlock,
                                                       SceneElement *element,
                                                       SceneDocumentBinder *binder)
    : m_textBlock(textBlock), m_sceneElement(element), m_binder(binder)
{
    if (m_binder->isSpellCheckEnabled()) {
        m_spellCheck = element->spellCheck();
        m_spellCheckConnection =
                QObject::connect(m_spellCheck, SIGNAL(misspelledFragmentsChanged()), m_binder,
                                 SLOT(onSpellCheckUpdated()), Qt::UniqueConnection);
        m_spellCheck->scheduleUpdate();
    }

    if (m_textBlock.isValid())
        m_textBlock.setUserData(this);
}

SceneDocumentBlockUserData::~SceneDocumentBlockUserData()
{
    if (m_spellCheckConnection)
        QObject::disconnect(m_spellCheckConnection);
}

bool SceneDocumentBlockUserData::isValid() const
{
    return m_textBlock.isValid() && !m_sceneElement.isNull();
}

void SceneDocumentBlockUserData::resetFormat()
{
    m_formatMTime = -1;
    blockFormat = QTextBlockFormat();
    charFormat = QTextCharFormat();
}

bool SceneDocumentBlockUserData::updateFromFormat(const SceneElementFormat *format)
{
    if (format->isModified(&m_formatMTime)) {
        this->blockFormat = format->createBlockFormat(m_sceneElement->alignment());
        this->charFormat = format->createCharFormat();
        if (this->blockFormat.hasProperty(QTextFormat::BackgroundBrush))
            this->blockFormat.setBackground(colorTransformBrush(this->blockFormat.background()));
        if (this->charFormat.hasProperty(QTextFormat::ForegroundBrush))
            this->charFormat.setForeground(colorTransformBrush(this->charFormat.foreground()));
        return true;
    }

    return false;
}

void SceneDocumentBlockUserData::initializeSpellCheck(SceneDocumentBinder *binder)
{
    if (binder->isSpellCheckEnabled()) {
        m_spellCheck = m_sceneElement->spellCheck();
        if (!m_spellCheckConnection)
            m_spellCheckConnection =
                    QObject::connect(m_spellCheck, SIGNAL(misspelledFragmentsChanged()), binder,
                                     SLOT(onSpellCheckUpdated()), Qt::UniqueConnection);
        m_spellCheck->scheduleUpdate();
    } else {
        if (m_spellCheckConnection)
            QObject::disconnect(m_spellCheckConnection);

        m_spellCheck = nullptr;
    }
}

bool SceneDocumentBlockUserData::shouldUpdateFromSpellCheck()
{
    return !m_spellCheck.isNull() && m_spellCheck->isModified(&m_spellCheckMTime);
}

void SceneDocumentBlockUserData::scheduleSpellCheckUpdate()
{
    if (!m_spellCheck.isNull())
        m_spellCheck->scheduleUpdate();
}

QList<TextFragment> SceneDocumentBlockUserData::misspelledFragments() const
{
    if (!m_spellCheck.isNull())
        return m_spellCheck->misspelledFragments();
    return QList<TextFragment>();
}

TextFragment SceneDocumentBlockUserData::findMisspelledFragment(int start, int end) const
{
    const QList<TextFragment> fragments = this->misspelledFragments();
    for (const TextFragment &fragment : fragments) {
        if ((fragment.start() >= start && fragment.start() < end)
            || (fragment.end() > start && fragment.end() <= end))
            return fragment;
    }
    return TextFragment();
}

void SceneDocumentBlockUserData::polishTextLater()
{
    if (m_binder->m_autoPolishParagraphs) {
        m_pendingTasks += PolishTextTask;
        m_binder->m_sceneElementTaskTimer.start(500, m_binder);
    }
}

void SceneDocumentBlockUserData::autoCapitalizeLater()
{
    if (m_binder->m_autoCapitalizeSentences) {
        m_pendingTasks += AutoCapitalizeTask;
        m_binder->m_sceneElementTaskTimer.start(500, m_binder);
    }
}

QBrush SceneDocumentBlockUserData::colorTransformBrush(const QBrush &brush)
{
    QBrush ret = brush;
    ret.setColor(Utils::Color::transform(brush.color()));
    return ret;
}

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(const QTextBlock &block)
{
    SceneDocumentBlockUserData *ret = get(block.userData());

    if (ret && ret->m_textBlock != block)
        ret->m_textBlock = block;

    return ret;
}

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(QTextBlockUserData *userData)
{
    if (userData == nullptr)
        return nullptr;

    SceneDocumentBlockUserData *userData2 =
            reinterpret_cast<SceneDocumentBlockUserData *>(userData);
    return userData2->type == SceneDocumentBlockUserData::Type && userData2->isValid() ? userData2
                                                                                       : nullptr;
}

void SceneDocumentBlockUserData::polishTextNow()
{
    BlockKeyStrokes blockKeyStrokes;

    if (m_binder.isNull() || !m_textBlock.isValid())
        return;

    if (!m_binder->m_autoPolishParagraphs)
        return;

    // If the block that this object represents is currently being edited by the user
    // then lets not polish it right now.
    if (m_binder->m_cursorPosition >= 0) {
        QTextCursor cursor(m_textBlock);
        cursor.setPosition(m_binder->m_cursorPosition);
        if (cursor.block() == m_textBlock)
            return;
    }

    {
        // This is to avoid recursive edits
        QScopedValueRollback<bool> rollback(m_binder->m_sceneElementTaskIsRunning, true);

        // If the block has no text, then there is no point in polishing text
        if (m_textBlock.text().isEmpty())
            return;

        // Find out the previous scene, so that polishing of text may be done keeping
        // previous scene element in context.
        ScreenplayElement *spElement = m_binder->m_screenplayElement;
        ScreenplayElement *prevSpElement = nullptr;
        if (spElement) {
            const Screenplay *screenplay = spElement->screenplay();
            const int spElementIndex = screenplay->indexOfElement(spElement);
            for (int i = spElementIndex - 1; i >= 0; i--) {
                ScreenplayElement *spe = screenplay->elementAt(i);
                if (spe->elementType() == ScreenplayElement::SceneElementType) {
                    prevSpElement = spe;
                    break;
                }
            }
        }
        Scene *previousScene = prevSpElement ? prevSpElement->scene() : nullptr;

        // If the scene element that this object represents has no polish to apply, then
        // we can simply quit. Polishing means adding/removing CONT'D, : etc..
        if (m_sceneElement == nullptr || !m_sceneElement->polishText(previousScene))
            return;

        // Mark current cursor position if required, so that we can get back to it
        // once we are done applying all edits done as a part of the polish operation.
        bool cursorPositionMarked = false;
        if (m_binder->m_cursorPosition > m_textBlock.position())
            cursorPositionMarked = this->markCursorPosition();

        // Reset format so that its applied by the highlighter again
        this->resetFormat();

        // Apply the polished text
        QTextCursor cursor(m_textBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        const QString text = m_sceneElement->text();
        if (text.isEmpty())
            cursor.removeSelectedText();
        else
            cursor.insertText(text);

        // Restore cursor position
        if (cursorPositionMarked) {
            const int cp = this->markedCursorPosition(true);
            emit m_binder->requestCursorPosition(cp);
        }
    }

    // Rehighlight the block
    m_binder->rehighlightBlock(m_textBlock);
}

void SceneDocumentBlockUserData::autoCapitalizeNow()
{
    BlockKeyStrokes blockKeyStrokes;

    // If the block has no text, then there is no point in polishing text
    if (m_textBlock.text().isEmpty() || m_sceneElement == nullptr)
        return;

    if (!m_binder->m_autoCapitalizeSentences
        || LanguageEngine::instance()->supportedLanguages()->activeLanguage().charScript()
                != QChar::Script_Latin)
        return;

    // Auto-capitalize needs to be done only on action and dialogue paragraphs.
    const QList<int> capitalizePositions =
            m_sceneElement->autoCapitalizePositions(m_binder->m_autoCapitalizeExceptions);
    if (capitalizePositions.isEmpty())
        return;

    {
        // This is to avoid recursive edits
        QScopedValueRollback<bool> rollback(m_binder->m_sceneElementTaskIsRunning, true);

        // Mark current cursor position if required, so that we can get back to it
        // once we are done applying all edits done as a part of the capitalize
        // operation
        bool cursorPositionMarked = false;
        if (m_binder->m_cursorPosition > m_textBlock.position())
            cursorPositionMarked = this->markCursorPosition();

        // Reset format so that its applied by the highlighter again
        this->resetFormat();

        // Capitalize letters as determined.
        QTextCursor cursor(m_textBlock);
        for (int pos : capitalizePositions) {
            cursor.setPosition(m_textBlock.position() + pos);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            cursor.insertText(cursor.selectedText().toUpper());
            cursor.clearSelection();
            cursor.setPosition(m_textBlock.position() + pos);
        }

        // Store changes into the element.
        m_sceneElement->setText(m_textBlock.text());
        m_sceneElement->setTextFormats(m_textBlock.textFormats());

        // Restore cursor position
        if (cursorPositionMarked) {
            const int cp = this->markedCursorPosition(true);
            emit m_binder->requestCursorPosition(cp);
        }
    }

    // Rehighlight the block
    m_binder->rehighlightBlock(m_textBlock);
}

void SceneDocumentBlockUserData::performPendingTasks()
{
    for (int task : std::as_const(m_pendingTasks)) {
        if (task == PolishTextTask)
            this->polishTextNow();
        else if (task == AutoCapitalizeTask)
            this->autoCapitalizeNow();
    }

    m_pendingTasks.clear();
}

bool SceneDocumentBlockUserData::markCursorPosition()
{
    if (m_binder.isNull() || !m_textBlock.isValid())
        return false;

    if (m_binder->m_cursorPosition < 0
        || m_binder->m_cursorPosition > m_textBlock.document()->characterCount())
        return false;

    QTextCursor cursor(m_textBlock);
    cursor.setPosition(m_binder->m_cursorPosition);
    cursor.insertText(QString(QChar::LastValidCodePoint));
    return true;
}

int SceneDocumentBlockUserData::markedCursorPosition(bool removeMarker)
{
    if (m_binder.isNull() || !m_textBlock.isValid())
        return 0;

    QTextCursor cursor = m_textBlock.document()->find(QString(QChar::LastValidCodePoint));
    if (cursor.isNull())
        return 0;

    const int cp = cursor.hasSelection() ? cursor.selectionStart() : cursor.position();
    if (removeMarker) {
        if (cursor.hasSelection())
            cursor.removeSelectedText();
        else
            cursor.deleteChar();
    }
    return cp;
}

class SpellCheckCursor : public QTextCursor
{
public:
    explicit SpellCheckCursor(QTextDocument *document, int position) : QTextCursor(document)
    {
        this->setPosition(position);
        this->select(QTextCursor::WordUnderCursor);
        m_format = this->charFormat();

        m_blockData = SceneDocumentBlockUserData::get(this->block());
        if (m_blockData != nullptr) {
            const int start = this->selectionStart() - this->block().position();
            const int end = this->selectionEnd() - this->block().position();
            m_misspelledFragment = m_blockData->findMisspelledFragment(start, end);
        }
    }
    ~SpellCheckCursor() { }

    QString word() const { return this->selectedText(); }
    bool isMisspelled() const { return m_misspelledFragment.isValid(); }
    QStringList suggestions() const
    {
        QLocale::Language activeLanguage = QLocale::Language(
                LanguageEngine::instance()->supportedLanguages()->activeLanguageCode());
        QList<QLocale::Language> languages = m_misspelledFragment.languages();
        if (!languages.contains(activeLanguage))
            return m_misspelledFragment.suggestions();

        QStringList ret = m_misspelledFragment.languageSuggestions(activeLanguage);
        languages.removeAll(activeLanguage);
        for (QLocale::Language lang : std::as_const(languages)) {
            ret += m_misspelledFragment.languageSuggestions(lang);
        }
        return ret;
    }

    void replace(const QString &word)
    {
        if (this->word().isEmpty())
            return;

        this->removeSelectedText();
        const int start = this->position();
        this->insertText(word);
        const int end = this->position();
        this->setPosition(start);
        this->setPosition(end, QTextCursor::KeepAnchor);
        this->setCharFormat(m_format);
        this->setPosition(end);
    }

    void resetCharFormat() { this->replace(this->word()); }

private:
    SceneDocumentBlockUserData *m_blockData = nullptr;
    TextFragment m_misspelledFragment;
    QTextCharFormat m_format;
};

SceneDocumentBinder::SceneDocumentBinder(QObject *parent)
    : QSyntaxHighlighter(parent),
      m_initializeDocumentTimer("SceneDocumentBinder.m_initializeDocumentTimer"),
      m_rehighlightTimer("SceneDocumentBinder.m_rehighlightTimer"),
      m_sceneElementTaskTimer("SceneDocumentBinder.m_sceneElementTaskTimer"),
      m_textDocument(this, "textDocument"),
      m_scene(this, "scene"),
      m_currentElement(this, "currentElement"),
      m_screenplayElement(this, "screenplayElement"),
      m_screenplayFormat(this, "screenplayFormat"),
      m_textFormat(new TextFormat(this))
{
    connect(this, &SceneDocumentBinder::currentElementChanged, this,
            &SceneDocumentBinder::nextTabFormatChanged);
    connect(m_textFormat, &TextFormat::formatChanged, this,
            &SceneDocumentBinder::onTextFormatChanged);
    connect(this, &SceneDocumentBinder::currentElementChanged, this,
            &SceneDocumentBinder::activateCurrentElementDefaultLanguage);
    connect(m_textFormat, &TextFormat::formatChanged, this,
            &SceneDocumentBinder::activateCurrentElementDefaultLanguage);
    connect(this, &SceneDocumentBinder::selectionStartPositionChanged, this,
            &SceneDocumentBinder::selectedElementsChanged);
    connect(this, &SceneDocumentBinder::selectionEndPositionChanged, this,
            &SceneDocumentBinder::selectedElementsChanged);

    connect(LanguageEngine::instance(), &LanguageEngine::scriptFontFamilyChanged, this,
            &SceneDocumentBinder::refresh);

    QStyleHints *styleHints = qApp->styleHints();
    connect(styleHints, &QStyleHints::colorSchemeChanged, this, &SceneDocumentBinder::refresh);
}

SceneDocumentBinder::~SceneDocumentBinder() { }

void SceneDocumentBinder::setScreenplayFormat(ScreenplayFormat *val)
{
    if (m_screenplayFormat == val)
        return;

    if (m_screenplayFormat != nullptr) {
        disconnect(m_screenplayFormat, &ScreenplayFormat::formatChanged, this,
                   &SceneDocumentBinder::refresh);
        disconnect(m_screenplayFormat, &ScreenplayFormat::inTransactionChanged, this,
                   &SceneDocumentBinder::rehighlightLater);
    }

    m_screenplayFormat = val;
    if (m_screenplayFormat != nullptr) {
        connect(m_screenplayFormat, &ScreenplayFormat::formatChanged, this,
                &SceneDocumentBinder::refresh);
        connect(m_screenplayFormat, &ScreenplayFormat::inTransactionChanged, this,
                &SceneDocumentBinder::rehighlightLater);

        if (qFuzzyCompare(m_textWidth, 0.0))
            this->setTextWidth(m_screenplayFormat->pageLayout()->contentWidth());
    }

    emit screenplayFormatChanged();

    this->initializeDocumentLater();
}

void SceneDocumentBinder::setScene(Scene *val)
{
    if (m_scene == val)
        return;

    if (m_scene != nullptr) {
        disconnect(m_scene, &Scene::sceneElementChanged, this,
                   &SceneDocumentBinder::onSceneElementChanged);
        disconnect(m_scene, &Scene::sceneAboutToReset, this,
                   &SceneDocumentBinder::onSceneAboutToReset);
        disconnect(m_scene, &Scene::sceneReset, this, &SceneDocumentBinder::onSceneReset);
        disconnect(m_scene, &Scene::sceneRefreshed, this, &SceneDocumentBinder::onSceneRefreshed);
        disconnect(m_scene, &QAbstractItemModel::rowsInserted, this,
                   &SceneDocumentBinder::syncDocumentFromScene);
        disconnect(m_scene, &QAbstractItemModel::rowsRemoved, this,
                   &SceneDocumentBinder::syncDocumentFromScene);
    }

    m_scene = val;

    if (m_scene != nullptr) {
        connect(m_scene, &Scene::sceneElementChanged, this,
                &SceneDocumentBinder::onSceneElementChanged);
        connect(m_scene, &Scene::sceneAboutToReset, this,
                &SceneDocumentBinder::onSceneAboutToReset);
        connect(m_scene, &Scene::sceneReset, this, &SceneDocumentBinder::onSceneReset);
        connect(m_scene, &Scene::sceneRefreshed, this, &SceneDocumentBinder::onSceneRefreshed);
        connect(m_scene, &QAbstractItemModel::rowsInserted, this,
                &SceneDocumentBinder::syncDocumentFromScene);
        connect(m_scene, &QAbstractItemModel::rowsRemoved, this,
                &SceneDocumentBinder::syncDocumentFromScene);
    }

    emit sceneChanged();

    this->initializeDocumentLater();
}

void SceneDocumentBinder::setScreenplayElement(ScreenplayElement *val)
{
    if (m_screenplayElement == val)
        return;

    if (m_screenplayElement != nullptr) {
        Screenplay *screenplay = m_screenplayElement->screenplay();
        disconnect(screenplay, &Screenplay::elementMoved, this,
                   &SceneDocumentBinder::polishAllSceneElements);
    }

    m_screenplayElement = val;

    if (m_screenplayElement != nullptr) {
        Screenplay *screenplay = m_screenplayElement->screenplay();
        connect(screenplay, &Screenplay::elementMoved, this,
                &SceneDocumentBinder::polishAllSceneElements, Qt::UniqueConnection);
    }

    emit screenplayElementChanged();

    this->polishAllSceneElements();
}

void SceneDocumentBinder::setTextDocument(QQuickTextDocument *val)
{
    if (m_textDocument == val)
        return;

    if (this->document() != nullptr) {
        this->document()->setUndoRedoEnabled(true);

        disconnect(this->document(), &QTextDocument::contentsChange, this,
                   &SceneDocumentBinder::onContentsChange);
        disconnect(this->document(), &QTextDocument::blockCountChanged, this,
                   &SceneDocumentBinder::syncSceneFromDocument);

        if (m_scene != nullptr)
            disconnect(m_scene, &Scene::sceneElementChanged, this,
                       &SceneDocumentBinder::onSceneElementChanged);

        this->setCurrentElement(nullptr);
        this->setCursorPosition(-1);
    }

    m_textDocument = val;
    if (m_textDocument != nullptr)
        this->QSyntaxHighlighter::setDocument(m_textDocument->textDocument());
    else
        this->QSyntaxHighlighter::setDocument(nullptr);
    this->setDocumentLoadCount(0);

    this->evaluateAutoCompleteHintsAndCompletionPrefix();

    this->initializeDocumentLater();

    if (m_textDocument != nullptr) {
        this->document()->setUndoRedoEnabled(false);

        connect(this->document(), &QTextDocument::contentsChange, this,
                &SceneDocumentBinder::onContentsChange, Qt::UniqueConnection);
        connect(this->document(), &QTextDocument::blockCountChanged, this,
                &SceneDocumentBinder::syncSceneFromDocument, Qt::UniqueConnection);

        if (m_scene != nullptr)
            connect(m_scene, &Scene::sceneElementChanged, this,
                    &SceneDocumentBinder::onSceneElementChanged, Qt::UniqueConnection);

#if 0 // At the moment, this seems to be causing more trouble than help.
        this->document()->setTextWidth(m_textWidth);
#endif

        this->setCursorPosition(0);
    } else
        this->setCursorPosition(-1);

    emit textDocumentChanged();
}

void SceneDocumentBinder::setSpellCheckEnabled(bool val)
{
    if (m_spellCheckEnabled == val)
        return;

    m_spellCheckEnabled = val;
    emit spellCheckEnabledChanged();

    this->refresh();
}

void SceneDocumentBinder::setLiveSpellCheckEnabled(bool val)
{
    if (m_liveSpellCheckEnabled == val)
        return;

    m_liveSpellCheckEnabled = val;
    emit liveSpellCheckEnabledChanged();
}

void SceneDocumentBinder::setAutoCapitalizeSentences(bool val)
{
    if (m_autoCapitalizeSentences == val)
        return;

    m_autoCapitalizeSentences = val;
    emit autoCapitalizeSentencesChanged();
}

void SceneDocumentBinder::setAutoCapitalizeExceptions(const QStringList &val)
{
    if (m_autoCapitalizeExceptions == val)
        return;

    m_autoCapitalizeExceptions = val;
    emit autoCapitalizeExceptionsChanged();

    SceneElement::autoCapitalizeExceptionsList() = val;

    QStringList &gil = SpellCheckService::globalIgnoreList();
    gil.clear();
    for (const QString &exc : val) {
        const QStringList parts = exc.split(QLatin1Char('.'), Qt::SkipEmptyParts);
        for (const QString &part : parts)
            if (part.length() > 1)
                gil.append(part.toLower());
    }
    gil.removeDuplicates();
}

void SceneDocumentBinder::setAutoPolishParagraphs(bool val)
{
    if (m_autoPolishParagraphs == val)
        return;

    m_autoPolishParagraphs = val;
    emit autoPolishParagraphsChanged();
}

void SceneDocumentBinder::setTextWidth(qreal val)
{
    if (qFuzzyCompare(m_textWidth, val))
        return;

    m_textWidth = val;
#if 0 // At the moment, this seems to be causing more trouble than help.
    if(this->document() != nullptr)
        this->document()->setTextWidth(m_textWidth);
#endif

    emit textWidthChanged();
}

void SceneDocumentBinder::setBottomMargin(qreal val)
{
    if (qFuzzyCompare(m_bottomMargin, val))
        return;

    m_bottomMargin = qMax(0.0, val);

    if (m_textDocument != nullptr && !m_initializingDocument && m_documentLoadCount > 0) {
        QTextDocument *document = m_textDocument->textDocument();
        if (document != nullptr) {
            QTextFrameFormat frameFormat = document->rootFrame()->frameFormat();
            frameFormat.setBottomMargin(m_bottomMargin);
            document->rootFrame()->setFrameFormat(frameFormat);
        }
    }

    emit bottomMarginChanged();
}

void SceneDocumentBinder::setCursorPosition(int val)
{
    if (qApp->closingDown())
        return;

#if 0
    Utils::Gui::log("SceneDocumentBinder(" + this->objectName() + ") cursorPosition: "
                     + QString::number(m_cursorPosition) + " to " + QString::number(val));
#endif
    if (m_initializingDocument || m_pastingContent || m_cursorPosition == val)
        return;

    QScopedValueRollback<bool> rollbackAcceptTextFormatChanges(m_acceptTextFormatChanges, false);
    auto cleanup = qScopeGuard([=]() {
        this->evaluateAutoCompleteHintsAndCompletionPrefix();
        if (m_cursorPosition >= 0)
            qApp->installEventFilter(this);
        else
            qApp->removeEventFilter(this);
    });

    if (m_textDocument == nullptr || this->document() == nullptr) {
        m_cursorPosition = -1;
        m_currentElementCursorPosition = -1;
        m_textFormat->reset();
        emit cursorPositionChanged();
        return;
    }

    m_cursorPosition = val;
    m_currentElementCursorPosition = -1;
    if (m_scene != nullptr)
        m_scene->setCursorPosition(m_cursorPosition);

    if (m_cursorPosition < 0) {
        m_currentElementCursorPosition = -1;
        m_textFormat->reset();
        emit cursorPositionChanged();
        this->setCurrentElement(nullptr);
        return;
    }

#if 1
    // Even if the document is empty, it should have one block for action or character
    // or whatever else default paragraph type, unless the document associated with
    // the scene is yet to be loaded from the scene content itself.
    if ((this->document()->isEmpty() || m_cursorPosition > this->document()->characterCount())
        && m_initializeDocumentTimer.isActive()) {
        m_textFormat->reset();
        emit cursorPositionChanged();
        return;
    }
#endif

    this->setWordUnderCursorIsMisspelled(false);
    this->setSpellingSuggestions(QStringList());

    SpellCheckCursor cursor(this->document(), val);

    QTextBlock block = cursor.block();
    if (!block.isValid()) {
        qDebug("[%d] There is no block at the cursor position %d.", __LINE__, val);
        emit cursorPositionChanged();
        m_textFormat->reset();
        return;
    }

    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData == nullptr) {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if (userData == nullptr) {
        this->setCurrentElement(nullptr);
        m_textFormat->reset();
        qWarning("[%d] TextDocument has a block at %d that isnt backed by a SceneElement!!",
                 __LINE__, val);
    } else {
        this->setCurrentElement(userData->sceneElement());
        this->setWordUnderCursorIsMisspelled(cursor.isMisspelled());
        this->setSpellingSuggestions(cursor.suggestions());

        if (m_selectionStartPosition >= 0 && m_selectionEndPosition > 0
            && m_selectionStartPosition != m_selectionEndPosition) {
            cursor.setPosition(m_selectionStartPosition);
            cursor.setPosition(m_selectionEndPosition, QTextCursor::KeepAnchor);
        }
        const QTextCharFormat format = cursor.charFormat();
        m_textFormat->updateFromCharFormat(format);
    }

    m_currentElementCursorPosition = m_cursorPosition - block.position();
    emit cursorPositionChanged();
}

void SceneDocumentBinder::setSelectionStartPosition(int val)
{
    if (m_selectionStartPosition == val)
        return;

    m_selectionStartPosition = val;
    emit selectionStartPositionChanged();
    emit selectedTextChanged();
    emit selectedBlockCountChanged();
}

void SceneDocumentBinder::setSelectionEndPosition(int val)
{
    if (m_selectionEndPosition == val)
        return;

    m_selectionEndPosition = val;
    emit selectionEndPositionChanged();
    emit selectedTextChanged();
    emit selectedBlockCountChanged();
}

int SceneDocumentBinder::selectedBlockCount() const
{
    if (m_selectionStartPosition >= 0 && m_selectionEndPosition > m_selectionStartPosition) {
        QTextCursor cursor(m_textDocument->textDocument());
        cursor.setPosition(m_selectionStartPosition);

        if (!cursor.movePosition(QTextCursor::EndOfBlock))
            return 0;

        if (m_selectionEndPosition < cursor.position())
            return 0;

        if (!cursor.movePosition(QTextCursor::StartOfBlock))
            return 0;

        int blockCount = 1;
        while (1) {
            if (!cursor.movePosition(QTextCursor::NextBlock))
                break;

            if (m_selectionEndPosition > cursor.position())
                ++blockCount;
        }

        return blockCount;
    }

    return 0;
}

bool SceneDocumentBinder::changeTextCase(TextCasing casing)
{
    struct Fragment
    {
        int start = -1;
        int end = -1;
        QTextBlock block;
    };
    QVector<Fragment> fragments;

    if (m_selectionStartPosition >= 0 && m_selectionEndPosition > 0
        && m_selectionEndPosition > m_selectionStartPosition) {
        QTextCursor cursor(this->document());
        cursor.setPosition(m_selectionStartPosition);
        while (1) {
            Fragment fragment;
            fragment.block = cursor.block();
            fragment.start = qMax(m_selectionStartPosition, fragment.block.position());
            fragment.end = qMin(m_selectionEndPosition, [](const QTextBlock &block) {
                QTextCursor c(block);
                c.movePosition(QTextCursor::EndOfBlock);
                return c.position();
            }(fragment.block));
            fragments.append(fragment);
            if (!cursor.movePosition(QTextCursor::NextBlock))
                break;
            if (cursor.atEnd() || cursor.position() > m_selectionEndPosition)
                break;
        }
    } else if (m_cursorPosition >= 0) {
        QTextCursor cursor(this->document());
        cursor.setPosition(m_cursorPosition);
        cursor.select(QTextCursor::WordUnderCursor);

        if (cursor.hasSelection()) {
            Fragment fragment;
            fragment.end = cursor.selectionEnd();
            fragment.start = cursor.selectionStart();
            fragment.block = cursor.block();
            fragments.append(fragment);
        }
    }

    if (fragments.isEmpty())
        return false;

    auto changeTextCase = [casing](const QString &text) {
        switch (casing) {
        case LowerCase:
            return text.toLower();
        case UpperCase:
            return text.toUpper();
        }
        return text;
    };

    QTextCursor cursor(this->document());

    for (int i = fragments.length() - 1; i >= 0; i--) {
        const Fragment fragment = fragments.at(i);

        cursor.setPosition(fragment.start);
        cursor.setPosition(fragment.end, QTextCursor::KeepAnchor);
        cursor.insertText(changeTextCase(cursor.selectedText()));
        cursor.clearSelection();

        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(fragment.block);
        if (userData)
            userData->autoCapitalizeLater();
    }

    return true;
}

void SceneDocumentBinder::setApplyTextFormat(bool val)
{
    if (m_applyTextFormat == val)
        return;

    m_applyTextFormat = val;
    emit applyTextFormatChanged();
}

void SceneDocumentBinder::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void SceneDocumentBinder::setTransitions(const QStringList &val)
{
    if (m_transitions == val)
        return;

    m_transitions = val;
    emit transitionsChanged();
}

void SceneDocumentBinder::setShots(const QStringList &val)
{
    if (m_shots == val)
        return;

    m_shots = val;
    emit shotsChanged();
}

QList<SceneElement *> SceneDocumentBinder::selectedElements() const
{
    QList<SceneElement *> ret;

    if (m_selectionStartPosition < 0 || m_selectionEndPosition < 0
        || m_selectionEndPosition < m_selectionStartPosition)
        return ret;

    QTextDocument *doc = m_textDocument ? m_textDocument->textDocument() : nullptr;

    QTextCursor cursor(doc);
    cursor.setPosition(m_selectionStartPosition);

    while (1) {
        QTextBlock block = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            ret << userData->sceneElement();

        if (!cursor.movePosition(QTextCursor::NextBlock))
            break;

        if (cursor.position() > m_selectionEndPosition)
            break;
    }

    return ret;
}

SceneElement *SceneDocumentBinder::sceneElementAt(int cursorPosition) const
{
    QTextDocument *doc = m_textDocument ? m_textDocument->textDocument() : nullptr;
    if (doc == nullptr || cursorPosition < 0)
        return nullptr;

    QTextCursor cursor(doc);
    cursor.setPosition(cursorPosition);

    QTextBlock block = cursor.block();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData)
        return userData->sceneElement();

    return nullptr;
}

QRectF SceneDocumentBinder::sceneElementBoundingRect(SceneElement *sceneElement) const
{
    if (sceneElement == nullptr)
        return QRectF();

    QTextDocument *doc = m_textDocument->textDocument();

    QTextBlock block = doc->firstBlock();
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData && userData->sceneElement() == sceneElement) {
            QAbstractTextDocumentLayout *layout = doc->documentLayout();
            QRectF blockRect = layout->blockBoundingRect(block);

            SceneElementFormat *elementFormat =
                    m_screenplayFormat->elementFormat(sceneElement->type());

            const qreal dpr = m_screenplayFormat->devicePixelRatio();
            const qreal contentWidth = doc->textWidth();
            const qreal leftMargin = contentWidth * elementFormat->leftMargin() * dpr;
            blockRect.moveLeft(blockRect.left() + leftMargin);

            return blockRect;
        }

        block = block.next();
    }

    return QRectF();
}

void SceneDocumentBinder::setForceSyncDocument(bool val)
{
    if (m_forceSyncDocument == val)
        return;

    m_forceSyncDocument = val;
    emit forceSyncDocumentChanged();
}

void SceneDocumentBinder::setApplyLanguageFonts(bool val)
{
    if (m_applyLanguageFonts == val)
        return;

    m_applyLanguageFonts = val;
    emit applyLanguageFontsChanged();

    this->refresh();
}

QString SceneDocumentBinder::nextTabFormatAsString() const
{
    auto typeToString = [](int type) {
        switch (type) {
        case SceneElement::Action:
            return QStringLiteral("Action");
        case SceneElement::Character:
            return QStringLiteral("Character");
        case SceneElement::Dialogue:
            return QStringLiteral("Dialogue");
        case SceneElement::Parenthetical:
            return QStringLiteral("Parenthetical");
        case SceneElement::Shot:
            return QStringLiteral("Shot");
        case SceneElement::Transition:
            return QStringLiteral("Transition");
        case SceneElement::Heading:
            return QStringLiteral("Scene Heading");
        }
        return QStringLiteral("Unknown");
    };

    const int ntf = this->nextTabFormat();
    if (ntf < 0)
        return QStringLiteral("Change Format");

    const QString current =
            m_currentElement ? typeToString(m_currentElement->type()) : typeToString(-1);
    const QString next = typeToString(ntf);
    return current + QString(QChar(0x2192)) + next;
}

int SceneDocumentBinder::nextTabFormat() const
{
    if (m_cursorPosition < 0 || m_textDocument == nullptr || m_currentElement == nullptr
        || this->document() == nullptr)
        return -1;

    const int elementNr = m_scene->indexOfElement(m_currentElement);
    if (elementNr < 0)
        return -1;

    switch (m_currentElement->type()) {
    case SceneElement::Action:
        return SceneElement::Character;
    case SceneElement::Character:
        if (m_tabHistory.isEmpty())
            return SceneElement::Action;
        return SceneElement::Transition;
    case SceneElement::Dialogue:
        return SceneElement::Parenthetical;
    case SceneElement::Parenthetical:
        return SceneElement::Dialogue;
    case SceneElement::Shot:
        return SceneElement::Transition;
    case SceneElement::Transition:
        return SceneElement::Action;
    default:
        break;
    }

    return m_currentElement->type();
}

void SceneDocumentBinder::tab()
{
    const int ntf = this->nextTabFormat();
    if (ntf < 0)
        return;

    m_currentElement->setType(SceneElement::Type(ntf));
    m_tabHistory.append(m_currentElement->type());
    emit nextTabFormatChanged();
}

void SceneDocumentBinder::backtab()
{
    // Do nothing. It doesnt work anyway!
}

bool SceneDocumentBinder::canGoUp()
{
    if (m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));
    return cursor.movePosition(QTextCursor::Up);
}

bool SceneDocumentBinder::canGoDown()
{
    if (m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));
    return cursor.movePosition(QTextCursor::Down);
}

void SceneDocumentBinder::refresh()
{
    if (this->document()) {
        QTextBlock block = this->document()->firstBlock();
        while (block.isValid()) {
            SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
            if (userData) {
                userData->resetFormat();
                userData->initializeSpellCheck(this);
                userData->autoCapitalizeLater();
                userData->polishTextLater();
            }

            block = block.next();
        }

        this->rehighlightLater();
    }
}

void SceneDocumentBinder::reload()
{
    this->initializeDocument();
}

QStringList SceneDocumentBinder::spellingSuggestionsForWordAt(int position) const
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return QStringList();

    SpellCheckCursor cursor(this->document(), position);
    if (cursor.isMisspelled())
        return cursor.suggestions();

    return QStringList();
}

void SceneDocumentBinder::replaceWordAt(int position, const QString &with)
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return;

    SpellCheckCursor cursor(this->document(), position);
    if (!cursor.isMisspelled())
        return;

    cursor.replace(with);
    this->setSpellingSuggestions(QStringList());
    this->setWordUnderCursorIsMisspelled(false);
}

void SceneDocumentBinder::addWordAtPositionToDictionary(int position)
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return;

    SpellCheckCursor cursor(this->document(), position);
    if (!cursor.isMisspelled())
        return;

    if (SpellCheckService::addToDictionary(cursor.word())) {
        cursor.resetCharFormat();
        this->setSpellingSuggestions(QStringList());
        this->setWordUnderCursorIsMisspelled(false);
    }
}

void SceneDocumentBinder::addWordAtPositionToIgnoreList(int position)
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return;

    SpellCheckCursor cursor(this->document(), position);
    if (!cursor.isMisspelled())
        return;

    ScriteDocument::instance()->addToSpellCheckIgnoreList(cursor.word());
    cursor.resetCharFormat();
    this->setSpellingSuggestions(QStringList());
    this->setWordUnderCursorIsMisspelled(false);
}

void SceneDocumentBinder::setCompletionMode(CompletionMode val)
{
    if (m_completionMode == val)
        return;

    m_completionMode = val;
    emit completionModeChanged();
}

int SceneDocumentBinder::lastCursorPosition() const
{
    if (m_cursorPosition < 0 || this->document() == nullptr)
        return 0;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));
    cursor.movePosition(QTextCursor::End);
    return cursor.position();
}

int SceneDocumentBinder::cursorPositionAtBlock(int blockNumber) const
{
    if (this->document() != nullptr) {
        const QTextBlock block = this->document()->findBlockByNumber(blockNumber);
        if (m_cursorPosition >= block.position()
            && m_cursorPosition < block.position() + block.length())
            return m_cursorPosition;

        return block.position() + block.length() - 1;
    }

    return -1;
}

int SceneDocumentBinder::currentBlockPosition() const
{
    QTextCursor cursor(this->document());
    cursor.setPosition(m_cursorPosition);
    return cursor.block().position();
}

QString SceneDocumentBinder::selectedText() const
{
    if (m_selectionStartPosition >= 0 && m_selectionEndPosition > m_selectionStartPosition) {
        QTextCursor cursor(m_textDocument->textDocument());
        cursor.setPosition(m_selectionStartPosition);
        cursor.setPosition(m_selectionEndPosition, QTextCursor::KeepAnchor);
        return cursor.selectedText();
    }

    return QString();
}

QString SceneDocumentBinder::wordUnderCursor() const
{
    if (m_cursorPosition >= 0) {
        QTextCursor cursor(m_textDocument->textDocument());
        cursor.setPosition(m_cursorPosition);
        cursor.select(QTextCursor::WordUnderCursor);
        return cursor.selectedText();
    }

    return QString();
}

QString SceneDocumentBinder::hyperlinkUnderCursor() const
{
    if (m_cursorPosition >= 0) {
        QTextCursor cursor(m_textDocument->textDocument());
        cursor.setPosition(m_cursorPosition);

        QTextCharFormat format = cursor.charFormat();
        return format.isAnchor() ? format.anchorHref() : QString();
    }

    return QString();
}

int SceneDocumentBinder::hyperlinkUnderCursorStartPosition() const
{
    if (m_cursorPosition >= 0) {
        QTextCursor cursor(m_textDocument->textDocument());
        cursor.setPosition(m_cursorPosition);

        QTextCharFormat format = cursor.charFormat();
        if (format.isAnchor() && format.anchorHref() != "") {
            const QString href = format.anchorHref();
            while (1) {
                if (!cursor.movePosition(QTextCursor::Left))
                    break;

                format = cursor.charFormat();
                if (!format.isAnchor() || format.anchorHref() != href) {
                    break;
                }
            }

            return cursor.position();
        }
    }

    return -1;
}

int SceneDocumentBinder::hyperlinkUnderCursorEndPosition() const
{
    if (m_cursorPosition >= 0) {
        QTextCursor cursor(m_textDocument->textDocument());
        cursor.setPosition(m_cursorPosition);

        QTextCharFormat format = cursor.charFormat();
        if (format.isAnchor() && format.anchorHref() != "") {
            const QString href = format.anchorHref();
            while (1) {
                if (!cursor.movePosition(QTextCursor::Right))
                    break;
                format = cursor.charFormat();
                if (!format.isAnchor() || format.anchorHref() != href) {
                    cursor.movePosition(QTextCursor::Left);
                    break;
                }
            }

            return cursor.position();
        }
    }

    return -1;
}

QFont SceneDocumentBinder::currentFont() const
{
    if (this->document() == nullptr)
        return QFont();

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));

    QTextCharFormat format = cursor.charFormat();
    return format.font();
}

void SceneDocumentBinder::copy(int fromPosition, int toPosition)
{
    if (this->document() == nullptr)
        return;

    const bool allTextSelected = [=]() -> bool {
        if (fromPosition > 0)
            return false;

        QTextCursor cursor(m_textDocument->textDocument());
        cursor.movePosition(QTextCursor::End);
        return toPosition == cursor.position();
    }();

    QJsonArray content;

    auto addParaToContent = [&content](int type, int alignment, const QString &text,
                                       const QVector<QTextLayout::FormatRange> &formats =
                                               QVector<QTextLayout::FormatRange>()) {
        QJsonObject para;
        para.insert(QStringLiteral("type"), type);
        if (alignment >= 0)
            para.insert(QStringLiteral("alignment"), alignment);
        para.insert(QStringLiteral("text"), text);

        if (!formats.isEmpty()) {
            const QJsonArray jformats = SceneElement::textFormatsToJson(formats);
            para.insert(QStringLiteral("formats"), jformats);
        }

        content.append(para);
    };

    Fountain::Body fBody;

    if (allTextSelected && m_scene->heading()->isEnabled()) {
        // Copy the scene heading and synopsis to both fountain and JSON representations
        Fountain::Element fElement;
        fElement.text = m_scene->heading()->displayText();
        fElement.sceneNumber =
                m_screenplayElement ? m_screenplayElement->userSceneNumber() : QString();
        fElement.type = Fountain::Element::SceneHeading;
        fBody.append(fElement);

        fElement = Fountain::Element();
        if (!m_scene->synopsis().isEmpty()) {
            fElement.text = m_scene->synopsis();
            fElement.type = Fountain::Element::Synopsis;
            fBody.append(fElement);
        }

        addParaToContent(SceneElement::Heading, 0, m_scene->heading()->displayText());

        // Add scene number and synopsis to scene heading itself.
        QJsonObject headingPara = content.last().toObject();
        headingPara.insert(QStringLiteral("sceneNumber"), m_screenplayElement->userSceneNumber());
        headingPara.insert(QStringLiteral("synopsis"), m_scene->synopsis());
        content[content.size() - 1] = headingPara;
    }

    QTextCursor cursor(this->document());
    cursor.setPosition(fromPosition);

    QTextBlock block = cursor.block();
    while (block.isValid() && toPosition > block.position()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData == nullptr) {
            block = block.next();
            continue;
        }

        const int bstart = block.position();
        const int bend = [=]() {
            QTextCursor c(block);
            c.movePosition(QTextCursor::EndOfBlock);
            return c.position();
        }();
        cursor.setPosition(qMax(fromPosition, bstart));
        cursor.setPosition(qMin(toPosition, bend), QTextCursor::KeepAnchor);

        SceneElement *element = userData->sceneElement();

        const QVector<QTextLayout::FormatRange> blockFormats = block.textFormats();
        QVector<QTextLayout::FormatRange> formatsToCopy;
        formatsToCopy.reserve(blockFormats.size());
        for (const QTextLayout::FormatRange &format : blockFormats) {
            const int fstart = format.start - (fromPosition <= bstart ? 0 : fromPosition - bstart);
            const int flength = format.length + qMin(fstart, 0);

            QTextLayout::FormatRange fmt;
            fmt.start = qMax(fstart, 0);
            fmt.length = flength;
            fmt.format = format.format;
            formatsToCopy.append(fmt);
        }

        addParaToContent(element->type(), element->alignment(), cursor.selectedText(),
                         formatsToCopy);

        Fountain::Element fElement;
        fElement.text = cursor.selectedText();
        fElement.formats = formatsToCopy;
        switch (element->type()) {
        default:
        case SceneElement::Action:
            fElement.type = Fountain::Element::Action;
            break;
        case SceneElement::Character:
            fElement.type = Fountain::Element::Character;
            break;
        case SceneElement::Dialogue:
            fElement.type = Fountain::Element::Dialogue;
            break;
        case SceneElement::Parenthetical:
            fElement.type = Fountain::Element::Parenthetical;
            break;
        case SceneElement::Shot:
            fElement.type = Fountain::Element::Shot;
            break;
        case SceneElement::Transition:
            fElement.type = Fountain::Element::Transition;
            break;
        }

        fBody.append(fElement);

        block = block.next();
    }

    const QByteArray contentJson = QJsonDocument(content).toJson(QJsonDocument::Compact);

    QClipboard *clipboard = Application::instance()->clipboard();
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("scrite/scene"), contentJson);
    mimeData->setText(Fountain::Writer(fBody,
                                       fBody.size() > 1 ? Screenplay::fountainCopyOptions()
                                                        : Fountain::Writer::NoOption)
                              .toString());
    clipboard->setMimeData(mimeData);
}

int SceneDocumentBinder::paste(int fromPosition)
{
    if (this->document() == nullptr || m_pastingContent)
        return -1;

    QScopedValueRollback<bool> pastingContentRollback(m_pastingContent, true);

    struct Paragraph
    {
        Paragraph() { }
        Paragraph(const QString &_text, SceneElement::Type _type = SceneElement::Action)
            : text(_text), type(_type)
        {
        }

        QString text;
        SceneElement::Type type = SceneElement::Action;
        Qt::Alignment alignment;
        QVector<QTextLayout::FormatRange> formats;
    };

    QVector<Paragraph> paragraphs;

    const QClipboard *clipboard = Application::instance()->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    const QByteArray contentJson = mimeData->data(QStringLiteral("scrite/scene"));
    if (contentJson.isEmpty()) {
        if (mimeData->hasText()) {
            const QString text = mimeData->text();
            if (text.contains('\n')) {
                Fountain::Parser parser(text, Screenplay::fountainPasteOptions());

                bool applySceneHeading = fromPosition == 0 && m_scene->isEmpty();

                const Fountain::Body fBody = parser.body();
                if (fBody.size() == 1 && fBody.first().type == Fountain::Element::Action) {
                    const QStringList lines = text.split('\n');
                    for (const QString &line : lines) {
                        Paragraph paragraph;
                        paragraph.text = line;
                        paragraphs.append(paragraph);
                    }
                } else {
                    for (const Fountain::Element &element : fBody) {
                        Paragraph paragraph;
                        paragraph.text = element.text;
                        paragraph.formats = element.formats;

                        bool includeParagraph = true;
                        switch (element.type) {
                        case Fountain::Element::SceneHeading:
                            if (applySceneHeading) {
                                m_scene->heading()->parseFrom(element.text);
                                if (!element.sceneNumber.isEmpty()
                                    && m_screenplayElement != nullptr)
                                    m_screenplayElement->setUserSceneNumber(element.sceneNumber);
                                applySceneHeading = false;
                                includeParagraph = false;
                            } else {
                                paragraph.type = SceneElement::Action;
                                applySceneHeading = false;
                            }
                            break;
                        case Fountain::Element::Action:
                            paragraph.type = SceneElement::Action;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Character:
                            paragraph.type = SceneElement::Character;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Parenthetical:
                            paragraph.type = SceneElement::Parenthetical;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Dialogue:
                            paragraph.type = SceneElement::Dialogue;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Shot:
                            paragraph.type = SceneElement::Shot;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Transition:
                            paragraph.type = SceneElement::Transition;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Synopsis:
                            includeParagraph = false;
                            if (!element.text.isEmpty()) {
                                QString synopsis = m_scene->synopsis();
                                if (!synopsis.isEmpty())
                                    synopsis += "\n\n";
                                synopsis += element.text;
                                m_scene->setSynopsis(element.text);
                            }
                            break;
                        default:
                            includeParagraph = false;
                            break;
                        }

                        if (includeParagraph)
                            paragraphs.append(paragraph);
                    }
                }
            } else {
                Paragraph paragraph;
                paragraph.text = text;
                paragraphs.append(paragraph);
            }
        }
    } else {
        const QJsonArray content = QJsonDocument::fromJson(contentJson).array();
        if (content.isEmpty())
            return -1;

        bool applySceneHeading = fromPosition == 0 && m_scene->isEmpty();

        for (const QJsonValue &item : content) {
            const QJsonObject itemObject = item.toObject();
            const int type = itemObject.value(QStringLiteral("type")).toInt();
            const int alignment = itemObject.value(QStringLiteral("alignment")).toInt();
            const QString text = itemObject.value(QStringLiteral("text")).toString();

            if (applySceneHeading && type == SceneElement::Heading) {
                m_scene->heading()->parseFrom(text);
                m_screenplayElement->setUserSceneNumber(
                        itemObject.value(QStringLiteral("sceneNumber")).toString());
                m_scene->setSynopsis(itemObject.value(QStringLiteral("synopsis")).toString());
                applySceneHeading = false;
                continue;
            }

            Paragraph paragraph;
            paragraph.type = (type < SceneElement::Min || type > SceneElement::Max
                              || type == SceneElement::Heading)
                    ? SceneElement::Action
                    : SceneElement::Type(type);
            paragraph.text = text;
            paragraph.alignment = alignment == 0 ? Qt::Alignment() : Qt::Alignment(alignment);
            paragraph.formats = SceneElement::textFormatsFromJson(
                    itemObject.value(QStringLiteral("formats")).toArray());
            paragraphs.append(paragraph);

            applySceneHeading = false;
        }
    }

    fromPosition = fromPosition >= 0 ? fromPosition : m_cursorPosition;

    QTextCursor cursor(this->document());
    cursor.setPosition(fromPosition);

    const bool pasteFormatting = paragraphs.size() > 1;
    QTextBlock lastPastedBlock;

    for (int i = 0; i < paragraphs.size(); i++) {
        const Paragraph paragraph = paragraphs.at(i);
        if (i > 0)
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());

        const int pasteStart = cursor.position();
        cursor.insertText(paragraph.text);
        const int pasteEnd = cursor.position();

        if (!paragraph.formats.isEmpty()) {
            cursor.setPosition(pasteStart);
            for (const QTextLayout::FormatRange &format : paragraph.formats) {
                cursor.setPosition(pasteStart + format.start);
                cursor.setPosition(pasteStart + format.start + format.length,
                                   QTextCursor::KeepAnchor);
                cursor.setCharFormat(format.format);
                cursor.clearSelection();
            }
        }

        cursor.movePosition(QTextCursor::StartOfBlock);

        lastPastedBlock = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(lastPastedBlock);
        if (userData && userData->sceneElement())
            userData->sceneElement()->setText(lastPastedBlock.text());

        if (userData && pasteFormatting) {
            if (userData->sceneElement()) {
                userData->sceneElement()->setType(paragraph.type);
                userData->sceneElement()->setAlignment(paragraph.alignment);
                userData->sceneElement()->dropAllChanges();

                const SceneElementFormat *format =
                        m_screenplayFormat->elementFormat(paragraph.type);
                userData->blockFormat = format->createBlockFormat(paragraph.alignment);
                userData->charFormat = format->createCharFormat();
                cursor.setBlockFormat(userData->blockFormat);

                this->rehighlightBlock(lastPastedBlock);
            }
        }

        cursor.setPosition(pasteEnd);
    }

    m_sceneElementTaskTimer.stop();
    this->performAllSceneElementTasks();

    emit requestCursorPosition(cursor.position());

    // QTimer::singleShot(50, this, [this, cp]() {
    //     this->refresh();
    //     emit requestCursorPosition(cp);
    // });

    return cursor.position();
}

void SceneDocumentBinder::setApplyFormattingEvenInTransaction(bool val)
{
    if (m_applyFormattingEvenInTransaction == val)
        return;

    m_applyFormattingEvenInTransaction = val;
    emit applyFormattingEvenInTransactionChanged();
}

void SceneDocumentBinder::classBegin() { }

void SceneDocumentBinder::componentComplete()
{
    m_initializeDocumentTimer.stop();
    this->initializeDocument();
}

void SceneDocumentBinder::highlightBlock(const QString &text)
{
    if (m_initializingDocument || m_sceneElementTaskIsRunning)
        return;

    if (m_screenplayFormat == nullptr)
        return;

    QTextBlock block = this->QSyntaxHighlighter::currentBlock();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData == nullptr) {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if (userData == nullptr) {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    SceneElement *element = userData->sceneElement();
    if (element == nullptr) {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    auto colorTransformBrush = [](const QBrush &brush) -> QBrush {
        QBrush ret = brush;
        ret.setColor(Utils::Color::transform(brush.color()));
        return ret;
    };

    // Basic formatting
    const SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());
    if (userData->updateFromFormat(format))
        this->applyBlockFormatLater(block);

    this->mergeFormat(0, block.length(), userData->charFormat);

    // Per-language fonts.
    if (m_applyLanguageFonts) {
        const QList<ScriptBoundary> boundaries = LanguageEngine::determineBoundaries(text);

        for (const ScriptBoundary &boundary : boundaries) {
            if (!boundary.isValid() /*|| boundary.script == QChar::Script_Latin*/)
                continue;

            QTextCharFormat format;
            format.setFontFamilies({ boundary.fontFamily() });
            this->mergeFormat(boundary.start, boundary.end - boundary.start + 1, format);
        }

        if (m_currentElement == element)
            emit currentFontChanged();
    }

    // Spelling mistakes.
    const QList<TextFragment> fragments = userData->misspelledFragments();
    const QColor spellingBackgroundColor = Utils::Color::transform(QColor(255, 0, 0, 64));
    const QColor spellingTextColor = Utils::Color::textColorFor(spellingBackgroundColor);
    if (!fragments.isEmpty()) {
        for (const TextFragment &fragment : fragments) {
            if (!fragment.isValid())
                continue;

            const QString word = text.mid(fragment.start(), fragment.length());
            const QChar::Script script = LanguageEngine::determineScript(word);
            if (script != QChar::Script_Latin)
                continue;

            QTextCharFormat spellingErrorFormat;
            spellingErrorFormat.setForeground(spellingTextColor);
            spellingErrorFormat.setBackground(spellingBackgroundColor);
            this->mergeFormat(fragment.start(), fragment.length(), spellingErrorFormat);
        }

        emit spellingMistakesDetected();
    }

    /*
    Suppose that a paragraph of text has custom text and/or background
    colors applied to one or more words in it. When spell check is enabled
    and a paragraph of text has spelling mistakes, then the custom colors
    are not rendered until all the spelling mistakes are fixed.

         It appears that setting background color to light-red (as we do here)
         for misspelled words, causes all colors to disappear in that paragraph.
         I am unable to reproduce this issue on a separate sample Qt app; and I
         am unable to find out why its causing this issue here.

          Until we figure out why this is happening, we reapply background and
          foreground colors whenever spelling mistakes are detected.
          */

    const QVector<QTextLayout::FormatRange> formats = block.textFormats();
    for (const QTextLayout::FormatRange &format : formats) {
        if (format.format.hasProperty(QTextFormat::BackgroundBrush)
            || format.format.hasProperty(QTextFormat::ForegroundBrush)) {
            QTextCharFormat charFormat;
            if (format.format.hasProperty(QTextFormat::BackgroundBrush))
                charFormat.setBackground(colorTransformBrush(format.format.background()));
            if (format.format.hasProperty(QTextFormat::ForegroundBrush))
                charFormat.setForeground(colorTransformBrush(format.format.foreground()));
            this->mergeFormat(format.start, format.length, charFormat);
        }
    }

    // Links should appear in blue text and underline format.
    // const QVector<QTextLayout::FormatRange> formats = block.textFormats();
    for (const QTextLayout::FormatRange &format : formats) {
        if (format.format.isAnchor() && !format.format.anchorHref().isEmpty()) {
            QTextCharFormat linkFormat;
            linkFormat.setForeground(Utils::Color::transform(Qt::blue));
            linkFormat.setFontUnderline(true);
            this->mergeFormat(format.start, format.length, linkFormat);
        }
    }
}

void SceneDocumentBinder::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_initializeDocumentTimer.timerId()) {
        m_initializeDocumentTimer.stop();
        this->initializeDocument();
    } else if (te->timerId() == m_rehighlightTimer.timerId()) {
        m_rehighlightTimer.stop();

        const int nrBlocks = this->document()->blockCount();
        const int nrTresholdBlocks = nrBlocks >> 1;
        const QList<QTextBlock> queue = m_rehighlightBlockQueue;
        m_rehighlightBlockQueue.clear();

        if (queue.size() > nrTresholdBlocks || queue.isEmpty()) {
            this->QSyntaxHighlighter::rehighlight();
        } else {
            for (const QTextBlock &block : queue)
                this->rehighlightBlock(block);
        }
    } else if (te->timerId() == m_sceneElementTaskTimer.timerId()) {
        m_sceneElementTaskTimer.stop();
        this->performAllSceneElementTasks();
    } else if (te->timerId() == m_applyBlockFormatTimer.timerId()) {
        m_applyBlockFormatTimer.stop();

        const QList<QTextBlock> queue = m_applyBlockFormatQueue;
        m_applyBlockFormatQueue.clear();
        for (const QTextBlock &block : queue) {
            SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
            if (userData) {
                QTextCursor cursor(block);
                cursor.setBlockFormat(userData->blockFormat);
            }
        }
    }
}

bool SceneDocumentBinder::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    static QList<int> keyEvents(
            { QEvent::KeyPress, QEvent::KeyRelease, QEvent::Shortcut, QEvent::ShortcutOverride });

    if (m_cursorPosition >= 0 && keyEvents.contains(event->type())) {
        if (m_sceneElementTaskTimer.isActive())
            m_sceneElementTaskTimer.start(500, this);
    }

    return false;
}

void SceneDocumentBinder::mergeFormat(int start, int count, const QTextCharFormat &format)
{
    for (int i = start; i < start + count; i++) {
        QTextCharFormat mergedFormat = this->format(i);
        mergedFormat.merge(format);
        this->setFormat(i, 1, mergedFormat);
    }
}

void SceneDocumentBinder::resetScene()
{
    m_scene = nullptr;
    emit sceneChanged();

    this->initializeDocumentLater();
}

void SceneDocumentBinder::resetTextDocument()
{
    m_textDocument = nullptr;

    this->QSyntaxHighlighter::setDocument(nullptr);
    this->setDocumentLoadCount(0);
    this->setCursorPosition(-1);
    QTimer::singleShot(0, this, &SceneDocumentBinder::evaluateAutoCompleteHintsAndCompletionPrefix);

    emit textDocumentChanged();
}

void SceneDocumentBinder::resetScreenplayFormat()
{
    m_screenplayFormat = nullptr;
    emit screenplayFormatChanged();
}

void SceneDocumentBinder::resetScreenplayElement()
{
    if (m_screenplayElement != nullptr) {
        Screenplay *screenplay = m_screenplayElement->screenplay();
        disconnect(screenplay, &Screenplay::elementMoved, this,
                   &SceneDocumentBinder::polishAllSceneElements);
    }

    m_screenplayElement = nullptr;
    emit screenplayElementChanged();
}

void SceneDocumentBinder::initializeDocument()
{
    if (m_textDocument == nullptr || m_scene == nullptr || m_screenplayFormat == nullptr)
        return;

    m_initializingDocument = true;

    m_tabHistory.clear();

    QFont defaultFont = m_screenplayFormat->defaultFont();
    defaultFont.setPointSize(defaultFont.pointSize() + m_screenplayFormat->fontPointSizeDelta());

    QTextDocument *document = m_textDocument->textDocument();
    QSignalBlocker documentSignalBlocker(document);
    if (m_documentLoadCount > 0)
        documentSignalBlocker.unblock();
    document->setDefaultFont(defaultFont);
    document->setUseDesignMetrics(true);

    QTextCursor cursor(document);
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();

    const int nrElements = m_scene->elementCount();

    QList<QTextBlock> blocks;

    // In the first pass, we simply insert text into the document.
    for (int i = 0; i < nrElements; i++) {
        SceneElement *element = m_scene->elementAt(i);
        if (i > 0)
            cursor.insertBlock();

        QTextBlock block = cursor.block();
        if (!block.isValid() && i == 0) {
            cursor.insertBlock();
            block = cursor.block();
        }

        SceneDocumentBlockUserData *userData = new SceneDocumentBlockUserData(block, element, this);
        block.setUserData(userData);
        cursor.insertText(element->text());
        blocks.append(block);
    }

    // In the second pass, we apply formatting to inserted text. We have to do this in the second
    // pass, because QTextDocument tends to pass character format at the last position of the
    // previous block, into the next block also. So for instance, if we have a fully bold paragraph
    // followed by a normal paragraph, QTextDocument will apply fully bold to both if we apply
    // text-formats while inserting text.
    for (QTextBlock &block : blocks) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        const SceneElement *element = userData->sceneElement();
        const SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());

        cursor = QTextCursor(block);

        userData->resetFormat();
        userData->updateFromFormat(format);
        cursor.setBlockFormat(userData->blockFormat);

        const QVector<QTextLayout::FormatRange> formatRanges =
                userData->sceneElement()->textFormats();
        if (formatRanges.isEmpty())
            continue;

        const int startPos = cursor.position();

        for (const QTextLayout::FormatRange &formatRange : formatRanges) {
            cursor.setPosition(startPos + formatRange.start);
            cursor.setPosition(startPos + formatRange.start + formatRange.length,
                               QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(formatRange.format);
            cursor.clearSelection();
        }

        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    }

    documentSignalBlocker.unblock();

    if (m_cursorPosition <= 0 && m_currentElement == nullptr && nrElements == 1)
        this->setCurrentElement(m_scene->elementAt(0));

    this->setDocumentLoadCount(m_documentLoadCount + 1);
    m_initializingDocument = false;

    {
        QTextFrameFormat frameFormat = document->rootFrame()->frameFormat();
        frameFormat.setBottomMargin(m_bottomMargin);
        document->rootFrame()->setFrameFormat(frameFormat);
    }

    this->QSyntaxHighlighter::rehighlight();
    this->polishAllSceneElements();

    emit documentInitialized();
}

void SceneDocumentBinder::initializeDocumentLater()
{
    m_initializeDocumentTimer.start(0, this);
}

void SceneDocumentBinder::setDocumentLoadCount(int val)
{
    if (m_documentLoadCount == val)
        return;

    m_documentLoadCount = val;
    emit documentLoadCountChanged();
}

void SceneDocumentBinder::setCurrentElement(SceneElement *val)
{
    if (m_currentElement == val)
        return;

    if (m_currentElement != nullptr) {
        disconnect(m_currentElement, &SceneElement::aboutToDelete, this,
                   &SceneDocumentBinder::resetCurrentElement);
        disconnect(m_currentElement, &SceneElement::typeChanged, this,
                   &SceneDocumentBinder::nextTabFormatChanged);
    }

    m_currentElement = val;

    if (m_currentElement != nullptr) {
        connect(m_currentElement, &SceneElement::aboutToDelete, this,
                &SceneDocumentBinder::resetCurrentElement);
        connect(m_currentElement, &SceneElement::typeChanged, this,
                &SceneDocumentBinder::nextTabFormatChanged);
    }

    emit currentElementChanged();

    m_tabHistory.clear();
    this->polishAllSceneElements();

    emit currentFontChanged();
}

void SceneDocumentBinder::resetCurrentElement()
{
    m_currentElement = nullptr;
    emit currentElementChanged();

    m_tabHistory.clear();
    this->evaluateAutoCompleteHintsAndCompletionPrefix();

    emit currentFontChanged();
}

void SceneDocumentBinder::activateCurrentElementDefaultLanguage()
{
    if (m_currentElement && m_screenplayFormat) {
        SceneElementFormat *format = m_screenplayFormat->elementFormat(m_currentElement->type());
        if (format != nullptr) {
#if 0
            Utils::Gui::log("SceneDocumentBinder(" + this->objectName()
                             + ") activating default language for "
                             + QString::number(m_currentElement->type()));
#endif
            format->activateDefaultLanguage();
        }
    }
}

class ForceCursorPositionHack : public QObject
{
public:
    explicit ForceCursorPositionHack(const QTextBlock &block, int cp, SceneDocumentBinder *binder);
    ~ForceCursorPositionHack();

    void timerEvent(QTimerEvent *event);

private:
    QTextBlock m_block;
    int m_cursorBlockPosition = 0; // within m_block
    ExecLaterTimer m_timer;
    SceneDocumentBinder *m_binder = nullptr;
};

ForceCursorPositionHack::ForceCursorPositionHack(const QTextBlock &block, int cbp,
                                                 SceneDocumentBinder *binder)
    : QObject(const_cast<QTextDocument *>(block.document())),
      m_block(block),
      m_cursorBlockPosition(cbp),
      m_timer("ForceCursorPositionHack.m_timer"),
      m_binder(binder)
{
    m_timer.start(0, this);
}

ForceCursorPositionHack::~ForceCursorPositionHack() { }

void ForceCursorPositionHack::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();

        QScopedValueRollback<bool> rollback(m_binder->m_sceneElementTaskIsRunning, true);

        QTextCursor cursor(m_block);
        cursor.insertText(QStringLiteral("("));
        cursor.deletePreviousChar();

        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(m_block);
        if (userData && userData->sceneElement()->type() == SceneElement::Parenthetical) {
            const QString bo = QStringLiteral("(");
            const QString bc = QStringLiteral(")");

            if (m_block.text().isEmpty()) {
                cursor.insertText(QStringLiteral("()"));
                m_cursorBlockPosition = 1;
            } else {
                const QString blockText = m_block.text();
                if (!blockText.startsWith(bo)) {
                    cursor.insertText(bo);
                    m_cursorBlockPosition += 1;
                }
                if (!blockText.endsWith(bc)) {
                    cursor.movePosition(QTextCursor::EndOfBlock);
                    cursor.insertText(bc);
                }
            }
        }

        if (!m_binder->m_autoCompleteHints.isEmpty())
            m_binder->evaluateAutoCompleteHintsAndCompletionPrefix();

        emit m_binder->requestCursorPosition(m_block.position() + m_cursorBlockPosition);

        GarbageCollector::instance()->add(this);
    }
}

void SceneDocumentBinder::onSceneElementChanged(SceneElement *element,
                                                Scene::SceneElementChangeType type)
{
    if (m_initializingDocument)
        return;

    if (m_textDocument == nullptr || this->document() == nullptr || m_scene == nullptr
        || element->scene() != m_scene)
        return;

    if (m_forceSyncDocument)
        this->initializeDocumentLater();

    if (type != Scene::ElementTypeChange)
        return;

    if (m_currentElement != nullptr && element == m_currentElement) {
        SceneElementFormat *format = m_screenplayFormat->elementFormat(m_currentElement->type());
        if (format != nullptr)
            format->activateDefaultLanguage();
    }

    this->evaluateAutoCompleteHintsAndCompletionPrefix();

    auto updateBlock = [=](const QTextBlock &block) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData != nullptr && userData->sceneElement() == element) {
            // Text changes from scene element to block are not applied
            // Only element type changes can be applied.
            const SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());
            userData->resetFormat();
            userData->updateFromFormat(format);

            QTextCursor cursor(block);
            cursor.setBlockFormat(userData->blockFormat);

            if (m_cursorPosition >= block.position()
                && m_cursorPosition <= block.position() + block.length())
                new ForceCursorPositionHack(block, m_cursorPosition - block.position(), this);

            this->rehighlightBlockLater(block);

            return true;
        }
        return false;
    };

    const int elementNr = m_scene->indexOfElement(element);
    QTextBlock block;

    if (elementNr >= 0) {
        block = this->document()->findBlockByNumber(elementNr);
        if (updateBlock(block))
            return;
    }

    block = this->document()->firstBlock();
    while (block.isValid()) {
        if (updateBlock(block))
            return;

        block = block.next();
    }
}

void SceneDocumentBinder::onSpellCheckUpdated()
{
    if (m_scene == nullptr || this->document() == nullptr || m_initializingDocument)
        return;

    SpellCheckService *spellCheck = qobject_cast<SpellCheckService *>(this->sender());
    if (spellCheck == nullptr)
        return;

    SceneElement *element = qobject_cast<SceneElement *>(spellCheck->parent());
    if (element == nullptr)
        return;

    const int elementIndex = m_scene->indexOfElement(element);
    if (elementIndex < 0)
        return;

    const QTextBlock block = this->document()->findBlockByNumber(elementIndex);
    if (block.isValid())
        this->rehighlightBlockLater(block);
}

void SceneDocumentBinder::onContentsChange(int from, int charsRemoved, int charsAdded)
{
    if (m_initializingDocument || m_sceneIsBeingReset || m_sceneElementTaskIsRunning
        || m_cursorPosition < 0)
        return;

    if (m_textDocument == nullptr || m_scene == nullptr || this->document() == nullptr)
        return;

    /* If m_cursorPosition > 0, it means that the user is currently typing in the TextArea within
     * SceneContentEditor. And it so happens that cursor-position will get set only after
     * the text on the sceneElement is set here. This causes the undo command to have
     * scene-position that is out of sync with the actual position. Hence, we evaluate the
     * cursor position based on the parameters given to this function before we set the text
     * on the SceneElement
     */
    auto updateCursorPosition = [=]() {
        QTextCursor cursor(m_textDocument->textDocument());
        cursor.movePosition(QTextCursor::End);
        int newCursorPosition =
                qBound(0, charsAdded > 0 ? from + charsAdded : from, cursor.position());
        m_scene->setCursorPosition(newCursorPosition);
        return newCursorPosition;
    };
    auto guard = qScopeGuard(updateCursorPosition);
    if (m_cursorPosition < 0)
        guard.dismiss();

    m_tabHistory.clear();

    if (m_sceneElementTaskTimer.isActive())
        m_sceneElementTaskTimer.start(500, this);

    if (m_scene->elementCount() != this->document()->blockCount()) {
        /**
          If the number of paragraphs in the document is differnet from the number of
          paragraphs in our internal Scene data structure, then we better sync it once.
          This can happen when user pastes more than 1 paragraphs at once or if the user
          deletes more than 1 paragraphs at once.
          */
        this->syncSceneFromDocument();
        return;
    }

    QTextCursor cursor(this->document());
    cursor.setPosition(from);

    // Auto-capitalize first letter of each sentence.
    if (m_autoCapitalizeSentences && charsRemoved == 0) {
        QTextBlock block = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            userData->autoCapitalizeLater();
    }

    // Fixed an issue that caused formatting to not get applied on the next
    // character, when there is no selection or word under the cursor.
    if (charsAdded == 1 && charsRemoved == 0 && m_applyNextCharFormat) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
        if (cursor.selectedText() != QStringLiteral(" ")) {
            m_applyNextCharFormat = false;
            cursor.setCharFormat(m_nextCharFormat);

            const QTextCharFormat ncf = m_nextCharFormat;
            QTextDocument *doc = this->document();
            QTimer::singleShot(0, this, [from, ncf, doc]() {
                QTextCursor cursor(doc);
                cursor.setPosition(from);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                cursor.setCharFormat(ncf);
            });
        }
    }

    do {
        QTextBlock block = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData == nullptr) {
            this->syncSceneFromDocument();
            return;
        }

        SceneElement *sceneElement = userData->sceneElement();
        if (sceneElement == nullptr) {
            qWarning("[%d] TextDocument has a block at %d that isnt backed by a SceneElement!!",
                     __LINE__, from);
            return;
        }

        sceneElement->setText(block.text());
        sceneElement->setTextFormats(block.textFormats());

        if (m_spellCheckEnabled && m_liveSpellCheckEnabled
            && ((charsAdded > 0 || charsRemoved > 0) && charsAdded != charsRemoved))
            userData->scheduleSpellCheckUpdate();

        if (!cursor.movePosition(QTextCursor::NextBlock))
            break;
    } while (!cursor.atEnd() && cursor.position() < from + charsAdded);

    if (m_cursorPosition >= 0) {
        cursor.setPosition(m_cursorPosition);
        m_textFormat->updateFromCharFormat(cursor.charFormat());
    }
}

void SceneDocumentBinder::syncSceneFromDocument(int nrBlocks)
{
    if (m_initializingDocument || m_sceneIsBeingReset)
        return;

    if (m_textDocument == nullptr || m_scene == nullptr)
        return;

    // Ofcourse we are refreshing the scene because the document changed.
    // But when we refresh the scene, the scene emits sceneRefreshed() signal
    // which will cause SceneDocumentBinder::onSceneRefreshed() to be called,
    // which is entirely unnecessary. We use this boolean to avoid that.
    QScopedValueRollback<bool> rollback(m_sceneIsBeingRefreshed, true);

    if (nrBlocks < 0)
        nrBlocks = this->document()->blockCount();

    /*
     * Ensure that blocks on the QTextDocument are in sync with
     * SceneElements in the Scene. I know that we are using a for loop
     * to make this happen, so we are (many-times) needlessly looping
     * over blocks that have already been touched, thereby making
     * this function slow. Still, I feel that this is better. A scene
     * would not have more than a few blocks, atbest 100 blocks.
     * So its better we sync it like this.
     */

    bool doPolishElements = false;

    // Decide whether to activate the undo capture. The capture collapses all individual inserts,
    // text updates, and removals into a single atomic undo step. This is required for:
    //   • Paste / bulk import (one or more new blocks with text)
    //   • Any operation that removes existing paragraphs (multi-paragraph delete, cut, or a
    //     Backspace/Delete that merges two paragraphs)
    // For an interactive Return keypress the capture is not needed — the lone
    // SceneInsertElement command is sufficient — and adding it would create a redundant undo step.
    //
    // Heuristics for new-block classification:
    //   1. More than one new block → definitely a paste / bulk insert.
    //   2. Exactly one new block but it already carries text → a single pasted line.
    //      A Return keypress always produces one new *empty* block.
    int newBlockCount = 0;
    bool anyNewBlockHasText = false;
    for (QTextBlock b = this->document()->begin(); b.isValid(); b = b.next()) {
        if (SceneDocumentBlockUserData::get(b) == nullptr) {
            ++newBlockCount;
            if (!b.text().isEmpty())
                anyNewBlockHasText = true;
        }
    }
    // elementsToRemove > 0 means the document now has fewer blocks than the scene has elements:
    // paragraphs were deleted (selection-delete, cut, or paragraph-merge via Backspace/Delete).
    const int elementsToRemove = m_scene->elementCount() - nrBlocks;
    const bool needsUndoCapture = newBlockCount > 1 || anyNewBlockHasText || elementsToRemove > 0;

    if (needsUndoCapture)
        m_scene->beginUndoCapture();

    QList<SceneElement *> elementList;
    elementList.reserve(nrBlocks);

    QTextBlock block = this->document()->begin();
    QTextBlock previousBlock;
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData == nullptr) {
            SceneElement *newElement = new SceneElement(m_scene);

            if (previousBlock.isValid()) {
                SceneDocumentBlockUserData *prevUserData =
                        SceneDocumentBlockUserData::get(previousBlock);
                SceneElement *prevElement = prevUserData->sceneElement();
                newElement->setType(prevElement->type());

                switch (prevElement->type()) {
                case SceneElement::Action:
                    newElement->setType(SceneElement::Action);
                    newElement->setAlignment(prevElement->alignment());
                    break;
                case SceneElement::Character:
                    newElement->setType(SceneElement::Dialogue);
                    break;
                case SceneElement::Dialogue:
                    newElement->setType(SceneElement::Character);
                    break;
                case SceneElement::Parenthetical:
                    newElement->setType(SceneElement::Dialogue);
                    break;
                case SceneElement::Shot:
                    newElement->setType(SceneElement::Action);
                    break;
                case SceneElement::Transition:
                    newElement->setType(SceneElement::Action);
                    break;
                default:
                    newElement->setType(SceneElement::Action);
                    break;
                }

                m_scene->insertElementAfter(newElement, prevElement);
            } else {
                newElement->setType(SceneElement::Action);
                m_scene->insertElementAt(newElement, 0);
            }

            userData = new SceneDocumentBlockUserData(block, newElement, this);
            block.setUserData(userData);
            doPolishElements = true;
        }

        elementList.append(userData->sceneElement());
        userData->sceneElement()->setText(block.text());
        userData->sceneElement()->setTextFormats(block.textFormats());
        userData->autoCapitalizeLater();

        previousBlock = block;
        block = block.next();
    }

    m_scene->setElementsList(elementList);

    if (needsUndoCapture)
        m_scene->endUndoCapture();

    if (doPolishElements)
        this->polishAllSceneElements();
}

void SceneDocumentBinder::syncDocumentFromScene()
{
    if (m_textDocument == nullptr || m_scene == nullptr || m_initializingDocument
        || m_sceneIsBeingReset || m_sceneIsBeingRefreshed)
        return;

    QTextDocument *document = m_textDocument->textDocument();
    const QList<SceneElement *> sceneElements = m_scene->elementsList();

    // Block document signals so that cursor operations below do not trigger
    // onContentsChange → syncSceneFromDocument, which would write stale text
    // back into scene elements before reconciliation is complete.
    QScopedValueRollback<bool> initGuard(m_initializingDocument, true);
    QSignalBlocker signalBlocker(document);

    // Pass 1: remove blocks whose scene element is no longer present in the scene.
    const QSet<SceneElement *> sceneElementSet(sceneElements.begin(), sceneElements.end());

    QList<QTextBlock> staleBlocks;
    for (QTextBlock b = document->begin(); b.isValid(); b = b.next()) {
        SceneDocumentBlockUserData *ud = SceneDocumentBlockUserData::get(b);
        if (ud != nullptr && !sceneElementSet.contains(ud->sceneElement()))
            staleBlocks.prepend(b); // reverse order: delete last-first
    }

    for (const QTextBlock &b : staleBlocks) {
        QTextCursor cursor(document);
        if (b.next().isValid()) {
            // Select from start of this block through start of next block (includes separator).
            cursor.setPosition(b.position());
            cursor.setPosition(b.next().position(), QTextCursor::KeepAnchor);
        } else if (b.previous().isValid()) {
            // Last block: select the separator of the previous block through end of this block's
            // content, so the previous block absorbs the end-of-document position.
            cursor.setPosition(b.previous().position() + b.previous().length() - 1);
            cursor.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
        } else {
            // Only block in document — just clear its content.
            cursor.setPosition(b.position());
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        }
        cursor.removeSelectedText();
    }

    // Pass 2: insert blocks for scene elements that have no corresponding block yet.
    QMap<SceneElement *, QTextBlock> elementToBlock;
    for (QTextBlock b = document->begin(); b.isValid(); b = b.next()) {
        SceneDocumentBlockUserData *ud = SceneDocumentBlockUserData::get(b);
        if (ud != nullptr)
            elementToBlock[ud->sceneElement()] = b;
    }

    for (int i = 0; i < sceneElements.size(); i++) {
        SceneElement *element = sceneElements.at(i);
        if (elementToBlock.contains(element))
            continue;

        QTextCursor cursor(document);
        if (i == 0) {
            // Insert at document start: split at position 0, cursor moves to new block 1,
            // then back to the new empty block 0.
            cursor.movePosition(QTextCursor::Start);
            cursor.insertBlock();
            cursor.movePosition(QTextCursor::PreviousBlock);
        } else {
            QTextBlock prevBlock = elementToBlock.value(sceneElements.at(i - 1));
            if (!prevBlock.isValid()) {
                this->initializeDocumentLater();
                return;
            }
            cursor = QTextCursor(prevBlock);
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.insertBlock();
        }

        QTextBlock newBlock = cursor.block();
        SceneDocumentBlockUserData *userData =
                new SceneDocumentBlockUserData(newBlock, element, this);
        cursor.insertText(element->text());

        const SceneElementFormat *fmt = m_screenplayFormat->elementFormat(element->type());
        userData->resetFormat();
        userData->updateFromFormat(fmt);
        QTextCursor fmtCursor(newBlock);
        fmtCursor.setBlockFormat(userData->blockFormat);

        elementToBlock[element] = newBlock;
    }

    this->rehighlightLater();
}

void SceneDocumentBinder::evaluateAutoCompleteHintsAndCompletionPrefix()
{
    QStringList hints;
    QStringList priorityHints;
    QString completionPrefix;
    int completionStart = -1;
    int completionEnd = -1;

    if (m_textDocument == nullptr || m_currentElement == nullptr || m_cursorPosition < 0) {
        this->setAutoCompleteHints(hints, priorityHints);
        this->setCompletionPrefix(completionPrefix, completionStart, completionEnd);
        return;
    }

    QTextCursor cursor(m_textDocument->textDocument());
    cursor.setPosition(m_cursorPosition);

    const QTextBlock block = cursor.block();
    completionStart = block.position();
    completionEnd = m_cursorPosition;

    CompletionMode completionMode = NoCompletionMode;

    switch (m_currentElement->type()) {
    case SceneElement::Character: {
        const QString bracketOpen = QLatin1String(" (");
        const QString blockText = block.text();
        if (blockText != bracketOpen && blockText.contains(bracketOpen)) {
            const QTextCursor bracketCursor =
                    m_textDocument->textDocument()->find(bracketOpen, block.position());
            if (m_cursorPosition > bracketCursor.selectionStart()) {
                /*
                There are several common notations that can be used in brackets after a character's
                name in a screenplay. Here are a few examples:

                - (V.O.) - This stands for "voiceover" and indicates that the character's dialogue
                is being heard on the soundtrack, but they are not physically present in the scene.
                - (O.S.) - This stands for "off-screen" and indicates that the character is speaking
                from outside the frame or from a location that is not visible to the audience.
                - (O.C.) - This stands for "off-camera" and indicates that the character is speaking
                from a location that is not within the frame of the camera, but they are physically
                present in the scene.
                - (CONT'D) - This indicates that the character's dialogue continues from the
                previous page or shot.
                - (PHONE) - This indicates that the character is speaking on the phone.
                - (INTO PHONE) - This indicates that the character is speaking into a phone or other
                communication device.
                - (FILTERED) - This indicates that the character's voice is being filtered or
                altered in some way.
                - (SUBTITLED) - This indicates that the character's dialogue is being presented as
                subtitles on the screen.
                - (THROUGH TRANSLATOR) - This indicates that the character is speaking through a
                translator or interpreter.
                - (OVER RADIO) - This indicates that the character is speaking over a radio or other
                communication device.
                - (ON TV) - This indicates that the character is speaking on a television or other
                video device.
                - (ON COMPUTER) - This indicates that the character is speaking through a computer
                or other electronic device.
                - (ON SPEAKERPHONE) - This indicates that the character is speaking on a
                speakerphone or other device that allows multiple people to hear the conversation.
                - (OVER INTERCOM) - This indicates that the character is speaking over an intercom
                or other public address system.

                In Scrite, CONT'D is automatically generated, so we don't really have to list it.
                But we will list it anyway because users will flapg it as a bug.
                 */
                static QStringList commonBracketNotations(
                        { QLatin1String("V.O."), QLatin1String("O.S."), QLatin1String("O.C."),
                          QLatin1String("CONT'D"), QLatin1String("PHONE"),
                          QLatin1String("INTO PHONE"), QLatin1String("FILTERED"),
                          QLatin1String("SUBTITLED"), QLatin1String("THROUGH TRANSLATOR"),
                          QLatin1String("OVER RADIO"), QLatin1String("ON TV"),
                          QLatin1String("ON COMPUTER"), QLatin1String("ON SPEAKERPHONE"),
                          QLatin1String("OVER INTERCOM") });
                hints = commonBracketNotations;
                priorityHints = commonBracketNotations;
                completionStart = bracketCursor.position();

                cursor.setPosition(bracketCursor.position());
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                completionPrefix = cursor.selectedText().simplified();
                completionMode = CharacterBracketNotationCompletionMode;
            } else {
                cursor.setPosition(block.position());
                cursor.setPosition(m_cursorPosition, QTextCursor::KeepAnchor);
                hints = m_characterNames;
                priorityHints = m_scene->characterNames();
                completionPrefix = cursor.selectedText().trimmed();
                completionStart = block.position();
                completionEnd = bracketCursor.selectionStart();
                completionMode = CharacterNameCompletionMode;
            }
        } else {
            hints = m_characterNames;
            priorityHints = m_scene->characterNames();
            completionPrefix = blockText;
            completionMode = CharacterNameCompletionMode;
        }
    } break;
    case SceneElement::Transition:
        hints = m_transitions;
        completionPrefix = block.text();
        completionMode = TransitionCompletionMode;
        break;
    case SceneElement::Shot:
        hints = m_shots;
        completionPrefix = block.text();
        completionMode = ShotCompletionMode;
        break;
    default:
        break;
    }

    this->setAutoCompleteHints(hints, priorityHints);
    this->setCompletionPrefix(completionPrefix, completionStart, completionEnd);
    this->setCompletionMode(completionMode);
}

void SceneDocumentBinder::setAutoCompleteHintsFor(SceneElement::Type val)
{
    if (m_autoCompleteHintsFor == val)
        return;

    m_autoCompleteHintsFor = val;
    emit autoCompleteHintsForChanged();
}

void SceneDocumentBinder::setAutoCompleteHints(const QStringList &hints,
                                               const QStringList &priorityHints)
{
    if (m_autoCompleteHints == hints && m_priorityAutoCompleteHints == priorityHints)
        return;

    m_autoCompleteHints = hints;
    m_priorityAutoCompleteHints = hints.isEmpty() ? QStringList() : priorityHints;
    emit autoCompleteHintsChanged();

    if (m_autoCompleteHints.isEmpty())
        this->setCompletionPrefix(QString());
}

void SceneDocumentBinder::setCompletionPrefix(const QString &prefix, int start, int end)
{
    if (m_completionPrefix == prefix)
        return;

    m_completionPrefix = prefix;
    m_completionPrefixStart = start;
    m_completionPrefixEnd = end;
    emit completionPrefixChanged();
}

void SceneDocumentBinder::setSpellingSuggestions(const QStringList &val)
{
    if (m_spellingSuggestions == val)
        return;

    m_spellingSuggestions = val;
    emit spellingSuggestionsChanged();
}

void SceneDocumentBinder::setWordUnderCursorIsMisspelled(bool val)
{
    if (m_wordUnderCursorIsMisspelled == val)
        return;

    m_wordUnderCursorIsMisspelled = val;
    emit wordUnderCursorIsMisspelledChanged();
}

void SceneDocumentBinder::onSceneAboutToReset()
{
    m_sceneIsBeingReset = true;
}

void SceneDocumentBinder::onSceneReset(int position)
{
    this->initializeDocument();

    if (position >= 0) {
        QTextCursor cursor(this->document());
        cursor.movePosition(QTextCursor::End);
        position = qBound(0, position, cursor.position());
        QTimer::singleShot(100, this, [=]() { emit requestCursorPosition(position); });
    }

    m_sceneIsBeingReset = false;
}

void SceneDocumentBinder::onSceneRefreshed()
{
    if (m_sceneIsBeingRefreshed)
        return;

    QScopedValueRollback<bool> rollback1(m_sceneIsBeingRefreshed, true);
    QScopedValueRollback<bool> rollback2(m_sceneIsBeingReset, true);

    const int cp = m_cursorPosition;
    this->setCursorPosition(-1);
    this->initializeDocument();
    if (cp >= 0)
        emit requestCursorPosition(cp);
}

void SceneDocumentBinder::rehighlightLater()
{
    if (!m_applyFormattingEvenInTransaction) {
        if (!m_screenplayFormat.isNull() && m_screenplayFormat->isInTransaction())
            return;
    }

    m_rehighlightTimer.start(0, this);
}

void SceneDocumentBinder::rehighlightBlockLater(const QTextBlock &block)
{
    m_rehighlightBlockQueue.removeOne(block);
    m_rehighlightBlockQueue << block;
    this->rehighlightLater();
}

void SceneDocumentBinder::applyBlockFormatLater(const QTextBlock &block)
{
    m_applyBlockFormatQueue.removeOne(block);
    m_applyBlockFormatQueue << block;
    m_applyBlockFormatTimer.start(0, this);
}

void SceneDocumentBinder::onTextFormatChanged(const QList<int> &properties)
{
    if (!m_acceptTextFormatChanges || !m_applyTextFormat
        || m_textFormat->isUpdatingFromCharFormat())
        return;

    auto guard = qScopeGuard([=]() { m_scene->endUndoCapture(); });
    m_scene->beginUndoCapture();

    const QTextCharFormat updatedFormat = m_textFormat->toCharFormat(properties);

    /**
     * I wish this function was simpler. I wish we could simply apply the curent
     * char format from m_textFormat to the selected text. But there are real-usage
     * complexities to deal with.
     *
     * For instance, its possible that the user selects a text fragment that already
     * has a few formatted sub-fragments. While applying a new format over this, it
     * should append the new format on top of the existing ones. Ofcourse we can use
     * mergeCharFormat() for this purpose, only when we are appending new format
     * properties over existing ones. But we cannot use that for removing already set
     * properties. For this reason, we will have to figure out all Fragments across
     * various blocks of text and then rework existing formats in those blocks
     * explicitly to match the new text format set by the user in the toolbar shown
     * within ScreenplayEditor.
     */

    struct Fragment
    {
        int start = -1;
        int end = -1;
        QTextBlock block;
    };
    QVector<Fragment> fragments;

    QTextCursor cursor(this->document());
    if (m_selectionStartPosition >= 0 && m_selectionEndPosition > 0
        && m_selectionStartPosition != m_selectionEndPosition) {
        cursor.setPosition(m_selectionStartPosition);
        while (1) {
            Fragment fragment;
            fragment.block = cursor.block();
            fragment.start = qMax(m_selectionStartPosition, fragment.block.position());
            fragment.end = qMin(m_selectionEndPosition, [](const QTextBlock &block) {
                QTextCursor c(block);
                c.movePosition(QTextCursor::EndOfBlock);
                return c.position();
            }(fragment.block));
            fragments.append(fragment);
            if (!cursor.movePosition(QTextCursor::NextBlock))
                break;
            if (cursor.atEnd() || cursor.position() > m_selectionEndPosition)
                break;
        }
        cursor.setPosition(m_selectionStartPosition);
        cursor.setPosition(m_selectionEndPosition, QTextCursor::KeepAnchor);
    } else if (m_cursorPosition >= 0) {
        cursor.setPosition(m_cursorPosition);
        cursor.select(QTextCursor::WordUnderCursor);

        if (!cursor.hasSelection()) {
            QTextCharFormat format = cursor.charFormat();
            for (int prop : properties)
                format.clearProperty(prop);
            format.merge(updatedFormat);
            m_nextCharFormat = format;
            m_applyNextCharFormat = true;
            return;
        }

        Fragment fragment;
        fragment.end = cursor.selectionEnd();
        fragment.start = cursor.selectionStart();
        fragment.block = cursor.block();
        fragments.append(fragment);
    } else
        return;

    for (const Fragment &fragment : fragments) {
        const QVector<QTextLayout::FormatRange> textFormats = fragment.block.textFormats();
        for (const QTextLayout::FormatRange &textFormat : textFormats) {
            int start = fragment.block.position() + textFormat.start;
            int end = fragment.block.position() + textFormat.start + textFormat.length;

            if ((start >= fragment.start && start <= fragment.end)
                || (end >= fragment.start && end <= fragment.end)
                || (start < fragment.start && end > fragment.end)) {

                start = qMax(start, fragment.start);
                end = qMin(end, fragment.end);

                cursor.setPosition(start);
                cursor.setPosition(end, QTextCursor::KeepAnchor);

                QTextCharFormat format = textFormat.format;
                for (int prop : properties)
                    format.clearProperty(prop);
                format.merge(updatedFormat);
                cursor.setCharFormat(format);
            }
        }
    }
}

void SceneDocumentBinder::polishAllSceneElements()
{
    QTextBlock block = this->document()->firstBlock();
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            userData->polishTextLater();
        block = block.next();
    }
}

void SceneDocumentBinder::polishSceneElement(SceneElement *element)
{
    const int blockNr = m_scene->indexOfElement(element);
    if (blockNr < 0)
        return;

    QTextBlock block = this->document()->findBlockByNumber(blockNr);
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData && userData->sceneElement() == element) {
        userData->polishTextLater();
        return;
    }

    block = this->document()->firstBlock();
    while (block.isValid()) {
        userData = SceneDocumentBlockUserData::get(block);
        if (userData && userData->sceneElement() == element) {
            userData->polishTextLater();
            return;
        }
        block = block.next();
    }
}

void SceneDocumentBinder::performAllSceneElementTasks()
{
    QTextBlock block = this->document()->firstBlock();
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            userData->performPendingTasks();
        block = block.next();
    }
}
