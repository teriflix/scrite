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

#include <QRegularExpression>

#include "utils.h"
#include "application.h"
#include "textlimiter.h"
#include "scritedocument.h"
#include "languageengine.h"
#include "spellcheckservice.h"
#include "syntaxhighlighter.h"

AbstractSyntaxHighlighterDelegate::AbstractSyntaxHighlighterDelegate(QObject *parent)
    : QObject(parent)
{
    m_highlighter = qobject_cast<SyntaxHighlighter *>(parent);

    connect(this, &AbstractSyntaxHighlighterDelegate::enabledChanged, this,
            &AbstractSyntaxHighlighterDelegate::rehighlight);
}

AbstractSyntaxHighlighterDelegate::~AbstractSyntaxHighlighterDelegate() { }

QTextDocument *AbstractSyntaxHighlighterDelegate::document() const
{
    return m_highlighter == nullptr ? nullptr : m_highlighter->document();
}

void AbstractSyntaxHighlighterDelegate::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void AbstractSyntaxHighlighterDelegate::rehighlight()
{
    if (m_highlighter != nullptr)
        m_highlighter->rehighlight();
}

void AbstractSyntaxHighlighterDelegate::rehighlightBlock(const QTextBlock &block)
{
    if (m_highlighter != nullptr)
        m_highlighter->rehighlightBlock(block);
}

void AbstractSyntaxHighlighterDelegate::documentsContentsChange(int from, int charsRemoved,
                                                                int charsAdded)
{
    Q_UNUSED(from)
    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)
}

void AbstractSyntaxHighlighterDelegate::documentContentsChanged() { }

void AbstractSyntaxHighlighterDelegate::mergeFormat(int start, int count,
                                                    const QTextCharFormat &format)
{
    for (int i = start; i < start + count; i++) {
        QTextCharFormat mergedFormat = this->format(i);
        mergedFormat.merge(format);
        this->setFormat(i, 1, mergedFormat);
    }
}

void AbstractSyntaxHighlighterDelegate::setFormat(int start, int count,
                                                  const QTextCharFormat &format)
{
    if (m_highlighter != nullptr)
        m_highlighter->setFormat(start, count, format);
}

void AbstractSyntaxHighlighterDelegate::setFormat(int start, int count, const QColor &color)
{
    if (m_highlighter != nullptr)
        m_highlighter->setFormat(start, count, color);
}

void AbstractSyntaxHighlighterDelegate::setFormat(int start, int count, const QFont &font)
{
    if (m_highlighter != nullptr)
        m_highlighter->setFormat(start, count, font);
}

QTextCharFormat AbstractSyntaxHighlighterDelegate::format(int pos) const
{
    if (m_highlighter != nullptr)
        return m_highlighter->format(pos);

    return QTextCharFormat();
}

int AbstractSyntaxHighlighterDelegate::previousBlockState() const
{
    if (m_highlighter != nullptr)
        return m_highlighter->previousBlockState();

    return -1;
}

int AbstractSyntaxHighlighterDelegate::currentBlockState() const
{
    if (m_highlighter != nullptr)
        return m_highlighter->currentBlockState();

    return -1;
}

void AbstractSyntaxHighlighterDelegate::setCurrentBlockState(int newState)
{
    if (m_highlighter != nullptr)
        m_highlighter->setCurrentBlockState(newState);
}

void AbstractSyntaxHighlighterDelegate::setCurrentBlockUserData(QTextBlockUserData *data)
{
    if (m_highlighter != nullptr)
        m_highlighter->setCurrentBlockDelegateUserData(this, data);
}

QTextBlockUserData *AbstractSyntaxHighlighterDelegate::currentBlockUserData() const
{
    if (m_highlighter != nullptr)
        return m_highlighter->getCurrentBlockDelegateUserData(this);

    return nullptr;
}

void AbstractSyntaxHighlighterDelegate::setBlockUserData(QTextBlock &block,
                                                         QTextBlockUserData *data)
{
    if (m_highlighter != nullptr)
        m_highlighter->setBlockDelegateUserData(block, this, data);
}

QTextBlockUserData *AbstractSyntaxHighlighterDelegate::blockUserData(const QTextBlock &block) const
{
    if (m_highlighter != nullptr)
        return m_highlighter->getBlockDelegateUserData(block, this);

    return nullptr;
}

QTextBlock AbstractSyntaxHighlighterDelegate::currentBlock() const
{
    if (m_highlighter != nullptr)
        return m_highlighter->currentBlock();

    return QTextBlock();
}

///////////////////////////////////////////////////////////////////////////////

class SyntaxHighlighterUserData : public QTextBlockUserData
{
public:
    explicit SyntaxHighlighterUserData() { }
    ~SyntaxHighlighterUserData() override { }

    void setDelegateUserData(AbstractSyntaxHighlighterDelegate *delegate, QTextBlockUserData *data)
    {
        m_userDataMap[delegate] = data;
    }

    QTextBlockUserData *getDelegateUserData(const AbstractSyntaxHighlighterDelegate *delegate) const
    {
        return m_userDataMap.value(delegate, (QTextBlockUserData *)nullptr);
    }

private:
    QHash<const AbstractSyntaxHighlighterDelegate *, QTextBlockUserData *> m_userDataMap;
};

SyntaxHighlighter::SyntaxHighlighter(QObject *parent) : QSyntaxHighlighter(parent)
{
    connect(this, &SyntaxHighlighter::delegateCountChanged, this,
            &SyntaxHighlighter::sortDelegates);
    connect(this, &SyntaxHighlighter::delegateCountChanged, this, &SyntaxHighlighter::rehighlight);

    if (parent->inherits("QQuickTextEdit")) {
        QVariant textDocVal = parent->property("textDocument");
        QQuickTextDocument *doc = textDocVal.value<QQuickTextDocument *>();
        if (doc)
            this->setTextDocument(doc);
        else {
            QObject *docObj = textDocVal.value<QObject *>();
            this->setTextDocument(qobject_cast<QQuickTextDocument *>(docObj));
        }

        connect(parent, SIGNAL(fontChanged(QFont)), this, SLOT(rehighlight()));
    } else if (parent->inherits("QTextDocument"))
        this->setDocument(qobject_cast<QTextDocument *>(parent));
    else if (parent->inherits("QQuickTextDocument"))
        this->setTextDocument(qobject_cast<QQuickTextDocument *>(parent));

    if (parent->inherits("QQuickItem")) {
        QQuickItem *qmlItem = qobject_cast<QQuickItem *>(parent);
        connect(qmlItem, &QQuickItem::widthChanged, this, &SyntaxHighlighter::rehighlight);
        connect(qmlItem, &QQuickItem::heightChanged, this, &SyntaxHighlighter::rehighlight);
    }
}

SyntaxHighlighter::~SyntaxHighlighter() { }

SyntaxHighlighter *SyntaxHighlighter::qmlAttachedProperties(QObject *object)
{
    return new SyntaxHighlighter(object);
}

void SyntaxHighlighter::setTextDocument(QQuickTextDocument *val)
{
    if (m_textDocument == val)
        return;

    if (m_textDocument != nullptr) {
        disconnect(m_textDocument->textDocument(), &QTextDocument::contentsChange, this,
                   &SyntaxHighlighter::documentContentsChange);
        disconnect(m_textDocument->textDocument(), &QTextDocument::contentsChanged, this,
                   &SyntaxHighlighter::documentContentsChanged);
    }

    m_textDocument = val;
    this->QSyntaxHighlighter::setDocument(
            m_textDocument == nullptr ? nullptr : m_textDocument->textDocument());

    if (m_textDocument != nullptr) {
        connect(m_textDocument->textDocument(), &QTextDocument::contentsChange, this,
                &SyntaxHighlighter::documentContentsChange);
        connect(m_textDocument->textDocument(), &QTextDocument::contentsChanged, this,
                &SyntaxHighlighter::documentContentsChanged);

        QTextDocument *doc = m_textDocument->textDocument();
        if (doc != nullptr)
            doc->setUndoRedoEnabled(m_textDocumentUndoRedoEnabled);
    }

    emit textDocumentChanged();

    this->documentContentsChanged();
}

void SyntaxHighlighter::setTextDocumentUndoRedoEnabled(bool val)
{
    if (m_textDocumentUndoRedoEnabled == val)
        return;

    m_textDocumentUndoRedoEnabled = val;

    if (m_textDocument != nullptr) {
        QTextDocument *doc = m_textDocument->textDocument();
        if (doc != nullptr)
            doc->setUndoRedoEnabled(val);
    }

    emit textDocumentUndoRedoEnabledChanged();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    for (AbstractSyntaxHighlighterDelegate *delegate : std::as_const(m_sortedDelegates)) {
        if (delegate->isEnabled())
            delegate->highlightBlock(text);
    }
}

QQmlListProperty<AbstractSyntaxHighlighterDelegate> SyntaxHighlighter::delegates()
{
    return QQmlListProperty<AbstractSyntaxHighlighterDelegate>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &SyntaxHighlighter::staticAppendDelegate, &SyntaxHighlighter::staticDelegateCount,
            &SyntaxHighlighter::staticDelegateAt, &SyntaxHighlighter::staticClearDelegates);
}

void SyntaxHighlighter::addDelegate(AbstractSyntaxHighlighterDelegate *ptr)
{
    if (ptr == nullptr || m_delegates.indexOf(ptr) >= 0)
        return;

    m_delegates.append(ptr);
    ptr->m_highlighter = this;

    if (ptr->parent() == nullptr || !ptr->instantiatedInQml())
        ptr->setParent(this);

    connect(ptr, &AbstractSyntaxHighlighterDelegate::aboutToDelete, this,
            &SyntaxHighlighter::onDelegateAboutToDelete);

    ptr->documentContentsChanged();
    emit delegateCountChanged();
}

void SyntaxHighlighter::removeDelegate(AbstractSyntaxHighlighterDelegate *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_delegates.indexOf(ptr);
    if (index < 0)
        return;

    ptr->m_highlighter = nullptr;
    disconnect(ptr, &AbstractSyntaxHighlighterDelegate::aboutToDelete, this,
               &SyntaxHighlighter::onDelegateAboutToDelete);
    m_delegates.removeAt(index);
    emit delegateCountChanged();

    if (ptr->parent() == this && !ptr->instantiatedInQml())
        ptr->deleteLater();
}

AbstractSyntaxHighlighterDelegate *SyntaxHighlighter::delegateAt(int index) const
{
    return index < 0 || index >= m_delegates.size() ? nullptr : m_delegates.at(index);
}

void SyntaxHighlighter::clearDelegates()
{
    while (m_delegates.size())
        this->removeDelegate(m_delegates.first());
}

AbstractSyntaxHighlighterDelegate *SyntaxHighlighter::findDelegate(const QString &className,
                                                                   const QString &objectName) const
{
    for (AbstractSyntaxHighlighterDelegate *delegate : m_delegates) {
        if (delegate->inherits(qPrintable(className))) {
            if (objectName.isEmpty() || delegate->objectName() == objectName)
                return delegate;
        }
    }

    return nullptr;
}

void SyntaxHighlighter::staticAppendDelegate(
        QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list,
        AbstractSyntaxHighlighterDelegate *ptr)
{
    reinterpret_cast<SyntaxHighlighter *>(list->data)->addDelegate(ptr);
}

void SyntaxHighlighter::staticClearDelegates(
        QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list)
{
    reinterpret_cast<SyntaxHighlighter *>(list->data)->clearDelegates();
}

AbstractSyntaxHighlighterDelegate *
SyntaxHighlighter::staticDelegateAt(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list,
                                    qsizetype index)
{
    return reinterpret_cast<SyntaxHighlighter *>(list->data)->delegateAt(index);
}

qsizetype
SyntaxHighlighter::staticDelegateCount(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list)
{
    return reinterpret_cast<SyntaxHighlighter *>(list->data)->delegateCount();
}

void SyntaxHighlighter::sortDelegates()
{
    m_sortedDelegates = m_delegates;
    // Stable, so that delegates of equal priority run in the order they were added.
    std::stable_sort(m_sortedDelegates.begin(), m_sortedDelegates.end(),
                     [](AbstractSyntaxHighlighterDelegate *a, AbstractSyntaxHighlighterDelegate *b) {
                         return a->priority() < b->priority();
                     });
}

void SyntaxHighlighter::onDelegateAboutToDelete(AbstractSyntaxHighlighterDelegate *ptr)
{
    this->removeDelegate(ptr);
}

void SyntaxHighlighter::setCurrentBlockDelegateUserData(AbstractSyntaxHighlighterDelegate *delegate,
                                                        QTextBlockUserData *data)
{
    QTextBlockUserData *blockData = this->QSyntaxHighlighter::currentBlockUserData();
    SyntaxHighlighterUserData *userData = static_cast<SyntaxHighlighterUserData *>(blockData);
    if (userData == nullptr) {
        userData = new SyntaxHighlighterUserData;
        this->QSyntaxHighlighter::setCurrentBlockUserData(userData);
    }

    userData->setDelegateUserData(delegate, data);
}

QTextBlockUserData *SyntaxHighlighter::getCurrentBlockDelegateUserData(
        const AbstractSyntaxHighlighterDelegate *delegate) const
{
    QTextBlockUserData *blockData = this->QSyntaxHighlighter::currentBlockUserData();
    SyntaxHighlighterUserData *userData = static_cast<SyntaxHighlighterUserData *>(blockData);
    if (userData == nullptr)
        return nullptr;

    return userData->getDelegateUserData(delegate);
}

void SyntaxHighlighter::setBlockDelegateUserData(QTextBlock &block,
                                                 AbstractSyntaxHighlighterDelegate *delegate,
                                                 QTextBlockUserData *data)
{
    QTextBlockUserData *blockData = block.userData();
    SyntaxHighlighterUserData *userData = static_cast<SyntaxHighlighterUserData *>(blockData);
    if (userData == nullptr) {
        userData = new SyntaxHighlighterUserData;
        block.setUserData(userData);
    }

    userData->setDelegateUserData(delegate, data);
}

QTextBlockUserData *
SyntaxHighlighter::getBlockDelegateUserData(const QTextBlock &block,
                                            const AbstractSyntaxHighlighterDelegate *delegate) const
{
    QTextBlockUserData *blockData = block.userData();
    SyntaxHighlighterUserData *userData = static_cast<SyntaxHighlighterUserData *>(blockData);
    if (userData == nullptr)
        return nullptr;

    return userData->getDelegateUserData(delegate);
}

void SyntaxHighlighter::documentContentsChange(int from, int charsRemoved, int charsAdded)
{
    for (AbstractSyntaxHighlighterDelegate *delegate : std::as_const(m_sortedDelegates))
        delegate->documentsContentsChange(from, charsRemoved, charsAdded);
}

void SyntaxHighlighter::documentContentsChanged()
{
    for (AbstractSyntaxHighlighterDelegate *delegate : std::as_const(m_sortedDelegates))
        delegate->documentContentsChanged();
}

///////////////////////////////////////////////////////////////////////////////

LanguageFontSyntaxHighlighterDelegate::LanguageFontSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
    connect(LanguageEngine::instance(), &LanguageEngine::scriptFontFamilyChanged, this,
            &AbstractSyntaxHighlighterDelegate::rehighlight);
}

LanguageFontSyntaxHighlighterDelegate::~LanguageFontSyntaxHighlighterDelegate() { }

void LanguageFontSyntaxHighlighterDelegate::setDefaultFont(const QVariant &val)
{
    if (m_defaultFont == val)
        return;

    if (val.isValid()) {
        if (val.userType() != QMetaType::QFont)
            return;
    }

    m_defaultFont = val;
    emit defaultFontChanged();
    this->rehighlight();
}

void LanguageFontSyntaxHighlighterDelegate::setEnforceDefaultFont(bool val)
{
    if (m_enforceDefaultFont == val)
        return;

    m_enforceDefaultFont = val;
    emit enforceDefaultFontChanged();
    this->rehighlight();
}

void LanguageFontSyntaxHighlighterDelegate::highlightBlock(const QString &text)
{
    if (m_enforceDefaultFont) {
        const QTextDocument *doc = this->document();
        const QFont defaultFont =
                m_defaultFont.isValid() && m_defaultFont.userType() == QMetaType::QFont
                ? m_defaultFont.value<QFont>()
                : doc->defaultFont();
        const QTextBlock block = this->currentBlock();

        QTextCharFormat defaultFormat;
        defaultFormat.setFont(defaultFont);
        this->setFormat(0, block.length(), defaultFormat);
    }

    const QList<ScriptBoundary> boundaries = LanguageEngine::determineBoundaries(text);

    for (const ScriptBoundary &boundary : boundaries) {
        if (!boundary.isValid() || boundary.script == QChar::Script_Latin)
            continue;

        QTextCharFormat format;
        format.setFontFamilies({ boundary.fontFamily() });
        this->mergeFormat(boundary.start, boundary.end - boundary.start + 1, format);
    }
}

///////////////////////////////////////////////////////////////////////////////

HeadingFontSyntaxHighlighterDelegate::HeadingFontSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
    QFont font = Application::instance()->font();
    font.setPointSize(Application::instance()->idealFontPointSize());
    this->initializeWithNormalFontAs(font);
}

HeadingFontSyntaxHighlighterDelegate::~HeadingFontSyntaxHighlighterDelegate() { }

void HeadingFontSyntaxHighlighterDelegate::setH1(const QFont &val)
{
    if (m_h1 == val)
        return;

    m_h1 = val;
    emit h1Changed();
    this->rehighlight();
}

void HeadingFontSyntaxHighlighterDelegate::setH2(const QFont &val)
{
    if (m_h2 == val)
        return;

    m_h2 = val;
    emit h2Changed();
    this->rehighlight();
}

void HeadingFontSyntaxHighlighterDelegate::setH3(const QFont &val)
{
    if (m_h3 == val)
        return;

    m_h3 = val;
    emit h3Changed();
    this->rehighlight();
}

void HeadingFontSyntaxHighlighterDelegate::setH4(const QFont &val)
{
    if (m_h4 == val)
        return;

    m_h4 = val;
    emit h4Changed();
    this->rehighlight();
}

void HeadingFontSyntaxHighlighterDelegate::setH5(const QFont &val)
{
    if (m_h5 == val)
        return;

    m_h5 = val;
    emit h5Changed();
    this->rehighlight();
}

void HeadingFontSyntaxHighlighterDelegate::setNormal(const QFont &val)
{
    if (m_normal == val)
        return;

    m_normal = val;
    emit normalChanged();
    this->rehighlight();
}

void HeadingFontSyntaxHighlighterDelegate::initializeWithNormalFontAs(const QFont &font)
{
    m_normal = font;

    m_h1 = m_normal;
    m_h1.setPointSize(m_normal.pointSize() + 8);
    m_h1.setBold(true);

    m_h2 = m_normal;
    m_h1.setPointSize(m_normal.pointSize() + 6);
    m_h2.setBold(true);

    m_h3 = m_normal;
    m_h1.setPointSize(m_normal.pointSize() + 4);
    m_h3.setBold(true);

    m_h4 = m_normal;
    m_h1.setPointSize(m_normal.pointSize() + 2);
    m_h4.setBold(true);

    m_h5 = m_normal;
    m_h5.setBold(true);
}

void HeadingFontSyntaxHighlighterDelegate::highlightBlock(const QString &text)
{
    const QTextBlock block = this->currentBlock();
    const QList<QFont> headingFonts({ m_normal, m_h1, m_h2, m_h3, m_h4, m_h5, m_normal });
    const int headingLevel =
            qMin(qMax(0, block.blockFormat().headingLevel()), headingFonts.size() - 1);
    const QFont headingFont = headingFonts.at(headingLevel);

    QTextCharFormat charFormat;
    charFormat.setFontPointSize(headingFont.pointSize());
    charFormat.setFontWeight(headingFont.weight());
    charFormat.setFontItalic(headingFont.italic());
    charFormat.setFontUnderline(headingFont.underline());
    charFormat.setFontCapitalization(headingFont.capitalization());
    charFormat.setFontLetterSpacing(headingFont.letterSpacing());
    this->mergeFormat(0, text.length(), charFormat);
}

///////////////////////////////////////////////////////////////////////////////

class SpellCheckSyntaxHighlighterUserData : public QTextBlockUserData
{
public:
    SpellCheckSyntaxHighlighterUserData(SpellCheckSyntaxHighlighterDelegate *delegate,
                                        const QTextBlock &block)
        : m_textBlock(block), m_delegate(delegate)
    {
        QObject::connect(&m_spellCheck, &SpellCheckService::misspelledFragmentsChanged, m_delegate,
                         [=]() {
                             m_delegate->rehighlightBlock(m_textBlock);
                             m_delegate->checkForSpellingMistakeInCurrentWord();
                             if (m_spellCheck.misspelledFragments().size() > 0)
                                 emit m_delegate->spellingMistakesDetected();
                         });
    }
    ~SpellCheckSyntaxHighlighterUserData() override { }

    void checkSpellings(const QString &text)
    {
        if (m_spellCheck.text() != text) {
            m_spellCheck.setText(text);
            m_spellCheck.scheduleUpdate();
        }
    }

    void checkSpellings() { this->checkSpellings(m_textBlock.text()); }

    QList<TextFragment> misspelledFragments() const { return m_spellCheck.misspelledFragments(); }

private:
    QString m_text;
    QTextBlock m_textBlock;
    SpellCheckService m_spellCheck;
    SpellCheckSyntaxHighlighterDelegate *m_delegate = nullptr;
};

SpellCheckSyntaxHighlighterDelegate::SpellCheckSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
}

SpellCheckSyntaxHighlighterDelegate::~SpellCheckSyntaxHighlighterDelegate() { }

void SpellCheckSyntaxHighlighterDelegate::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();
}

void SpellCheckSyntaxHighlighterDelegate::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();
}

void SpellCheckSyntaxHighlighterDelegate::setCursorPosition(int val)
{
    if (m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    emit cursorPositionChanged();

    this->checkForSpellingMistakeInCurrentWord();
}

QStringList
SpellCheckSyntaxHighlighterDelegate::spellingSuggestionsForWordAt(int cursorPosition) const
{
    TextFragment fragment;
    if (!this->findMisspelledTextFragment(cursorPosition, fragment))
        return QStringList();

    return fragment.suggestions();
}

void SpellCheckSyntaxHighlighterDelegate::replaceWordAt(int cursorPosition, const QString &with)
{
    QTextCursor cursor(this->document());
    SpellCheckSyntaxHighlighterUserData *ud = nullptr;

    if (!this->wordCursor(cursorPosition, cursor, ud))
        return;

    cursor.insertText(with);
    ud->checkSpellings();
}

void SpellCheckSyntaxHighlighterDelegate::addWordAtPositionToDictionary(int cursorPosition)
{
    QTextCursor cursor(this->document());
    SpellCheckSyntaxHighlighterUserData *ud = nullptr;

    if (!this->wordCursor(cursorPosition, cursor, ud))
        return;

    const QString word = cursor.selectedText();

    if (SpellCheckService::addToDictionary(word))
        ud->checkSpellings();
}

void SpellCheckSyntaxHighlighterDelegate::addWordAtPositionToIgnoreList(int cursorPosition)
{
    QTextCursor cursor(this->document());
    SpellCheckSyntaxHighlighterUserData *ud = nullptr;

    if (!this->wordCursor(cursorPosition, cursor, ud))
        return;

    const QString word = cursor.selectedText();
    ScriteDocument::instance()->addToSpellCheckIgnoreList(word);

    ud->checkSpellings();
}

void SpellCheckSyntaxHighlighterDelegate::checkForSpellingMistakeInCurrentWord()
{
    TextFragment fragment;
    if (m_cursorPosition >= 0 && this->findMisspelledTextFragment(m_cursorPosition, fragment)) {
        this->setWordUnderCursorIsMisspelled(true);
        this->setSpellingSuggestionsForWordUnderCursor(fragment.suggestions());
    } else {
        this->setWordUnderCursorIsMisspelled(false);
        this->setSpellingSuggestionsForWordUnderCursor(QStringList());
    }
}

void SpellCheckSyntaxHighlighterDelegate::highlightBlock(const QString &text)
{
    const QTextBlock block = this->currentBlock();

    SpellCheckSyntaxHighlighterUserData *userData =
            this->currentBlockUserData<SpellCheckSyntaxHighlighterUserData>();
    if (userData == nullptr) {
        userData = new SpellCheckSyntaxHighlighterUserData(this, block);
        this->setCurrentBlockUserData(userData);
    }

    userData->checkSpellings(text);

    const QColor spellingBackgroundColor = Utils::Color::transform(m_backgroundColor);
    const QColor spellingTextColor =
            Utils::Color::textColorFor(Utils::Color::stacked(m_textColor, spellingBackgroundColor));

    const QList<TextFragment> fragments = userData->misspelledFragments();
    if (!fragments.isEmpty()) {
        for (const TextFragment &fragment : fragments) {
            if (!fragment.isValid())
                continue;

            const QString word = text.mid(fragment.start(), fragment.length());
            const QChar::Script script = LanguageEngine::determineScript(word);
            if (script != QChar::Script_Latin)
                continue;

            QTextCharFormat spellingErrorFormat;
            if (spellingBackgroundColor.alpha() > 0)
                spellingErrorFormat.setBackground(spellingBackgroundColor);
            if (spellingTextColor.alpha() > 0)
                spellingErrorFormat.setForeground(spellingTextColor);

            this->mergeFormat(fragment.start(), fragment.length(), spellingErrorFormat);
        }
    }
}

void SpellCheckSyntaxHighlighterDelegate::setWordUnderCursorIsMisspelled(bool val)
{
    if (m_wordUnderCursorIsMisspelled == val)
        return;

    m_wordUnderCursorIsMisspelled = val;
    emit wordUnderCursorIsMisspelledChanged();
}

void SpellCheckSyntaxHighlighterDelegate::setSpellingSuggestionsForWordUnderCursor(
        const QStringList &val)
{
    if (m_spellingSuggestionsForWordUnderCursor == val)
        return;

    m_spellingSuggestionsForWordUnderCursor = val;
    emit spellingSuggestionsForWordUnderCursorChanged();
}

bool SpellCheckSyntaxHighlighterDelegate::findMisspelledTextFragment(
        int cursorPosition, TextFragment &misspelledFragment) const
{
    QTextCursor cursor(this->document());
    SpellCheckSyntaxHighlighterUserData *ud = nullptr;
    if (!this->wordCursor(cursorPosition, cursor, ud))
        return false;

    // We don't want the whole word selected
    cursor.setPosition(cursorPosition);

    const QTextBlock block = cursor.block();

    const QList<TextFragment> mispelledFragments = ud->misspelledFragments();
    const int blockCursorPosition = cursorPosition - block.position();
    for (const TextFragment &fragment : mispelledFragments) {
        if (fragment.start() <= blockCursorPosition && fragment.end() >= blockCursorPosition) {
            misspelledFragment = fragment;
            return true;
        }
    }

    return false;
}

bool SpellCheckSyntaxHighlighterDelegate::wordCursor(int cursorPosition, QTextCursor &cursor,
                                                     SpellCheckSyntaxHighlighterUserData *&ud) const
{
    ud = nullptr;

    if (cursorPosition < 0)
        return false;

    cursor = QTextCursor(this->document());
    cursor.movePosition(QTextCursor::End);
    if (cursorPosition > cursor.position())
        return false;

    cursor.setPosition(cursorPosition);

    const QTextBlock block = cursor.block();
    ud = this->blockUserData<SpellCheckSyntaxHighlighterUserData>(block);
    if (ud == nullptr)
        return false;

    cursor.select(QTextCursor::WordUnderCursor);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

TextLimiterSyntaxHighlighterDelegate::TextLimiterSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
    m_textLimiter = new TextLimiter(this);

    connect(this, &TextLimiterSyntaxHighlighterDelegate::cursorLimitPositionChanged, this,
            &AbstractSyntaxHighlighterDelegate::rehighlight);
}

TextLimiterSyntaxHighlighterDelegate::~TextLimiterSyntaxHighlighterDelegate() { }

void TextLimiterSyntaxHighlighterDelegate::setTextLimiter(TextLimiter *val)
{
    if (m_textLimiter == val)
        return;

    if (m_textLimiter->parent() == this)
        m_textLimiter->deleteLater();

    m_textLimiter = val == nullptr ? new TextLimiter(this) : val;
    emit textLimiterChanged();

    this->evaluateCursorLimitPosition();
}

void TextLimiterSyntaxHighlighterDelegate::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();

    this->rehighlight();
}

void TextLimiterSyntaxHighlighterDelegate::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();

    this->rehighlight();
}

void TextLimiterSyntaxHighlighterDelegate::highlightBlock(const QString &text)
{
    if (m_cursorLimitPosition < 0)
        return;

    const QTextBlock block = this->currentBlock();
    const int start = qMax(0, m_cursorLimitPosition - block.position());
    const int count = qMax(0, text.length() - start);

    if (count > 0) {
        QTextCharFormat charFormat;
        if (m_textColor.alpha() > 0)
            charFormat.setForeground(Utils::Color::transform(m_textColor));
        if (m_backgroundColor.alpha() > 0)
            charFormat.setBackground(Utils::Color::transform(m_backgroundColor));
        this->mergeFormat(start, count, charFormat);
    }
}

void TextLimiterSyntaxHighlighterDelegate::setCursorLimitPosition(int val)
{
    if (m_cursorLimitPosition == val)
        return;

    m_cursorLimitPosition = val;
    emit cursorLimitPositionChanged();
}

void TextLimiterSyntaxHighlighterDelegate::evaluateCursorLimitPosition()
{
    QTextDocument *doc = this->document();
    if (doc == nullptr || doc->isEmpty()) {
        m_textLimiter->setText(QString());
        this->setCursorLimitPosition(-1);
        return;
    }

    m_textLimiter->setText(doc->toPlainText());

    QTextCursor cursor(doc);
    int wordLimitPosition = -1;
    int letterLimitPosition = -1;

    // count letters
    int letterCount = 0;
    while (!cursor.atEnd()) {
        if (letterLimitPosition < 0 && letterCount >= m_textLimiter->maxLetterCount()) {
            letterLimitPosition = cursor.position();
            if (m_textLimiter->countMode() == AbstractTextLimiter::CountInLimitedText)
                break;
        }

        if (cursor.movePosition(QTextCursor::NextCharacter))
            ++letterCount;
    }
    cursor.movePosition(QTextCursor::Start);

    // count words
    int wordCount = 0;
    while (!cursor.atEnd()) {
        if (wordLimitPosition < 0 && wordCount >= m_textLimiter->maxWordCount()) {
            wordLimitPosition = cursor.position();
            if (m_textLimiter->countMode() == AbstractTextLimiter::CountInLimitedText)
                break;
        }

        if (cursor.movePosition(QTextCursor::NextWord))
            ++wordCount;
    }

    int limitPosition = -1;

    if (wordLimitPosition >= 0 || letterLimitPosition >= 0) {
        switch (m_textLimiter->mode()) {
        case AbstractTextLimiter::LowerOfWordAndLetterCount:
            limitPosition = wordLimitPosition >= 0 && letterLimitPosition >= 0
                    ? qMin(wordLimitPosition, letterLimitPosition)
                    : (wordLimitPosition >= 0 ? wordLimitPosition : letterLimitPosition);
            break;
        case AbstractTextLimiter::MatchWordCountOnly:
            limitPosition = wordLimitPosition;
            break;
        case AbstractTextLimiter::MatchLetterCountOnly:
            limitPosition = letterLimitPosition;
            break;
        }
    }

    this->setCursorLimitPosition(limitPosition);
}

///////////////////////////////////////////////////////////////////////////////

StoryBeatsSyntaxHighlighterDelegate::StoryBeatsSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
}

StoryBeatsSyntaxHighlighterDelegate::~StoryBeatsSyntaxHighlighterDelegate() { }

void StoryBeatsSyntaxHighlighterDelegate::setTemplateColor(const QColor &val)
{
    if (m_templateColor == val)
        return;

    m_templateColor = val;
    emit templateColorChanged();

    this->rehighlight();
}

void StoryBeatsSyntaxHighlighterDelegate::setActColor(const QColor &val)
{
    if (m_actColor == val)
        return;

    m_actColor = val;
    emit actColorChanged();

    this->rehighlight();
}

void StoryBeatsSyntaxHighlighterDelegate::setBeatColor(const QColor &val)
{
    if (m_beatColor == val)
        return;

    m_beatColor = val;
    emit beatColorChanged();

    this->rehighlight();
}

void StoryBeatsSyntaxHighlighterDelegate::setDescriptionColor(const QColor &val)
{
    if (m_descriptionColor == val)
        return;

    m_descriptionColor = val;
    emit descriptionColorChanged();

    this->rehighlight();
}

void StoryBeatsSyntaxHighlighterDelegate::applyStyle(int start, int count, const QColor &color,
                                                     bool bold, int deltaPtSize,
                                                     const QString &fontFamily)
{
    if (count <= 0 || color.alpha() == 0)
        return;

    QTextCharFormat charFormat;
    charFormat.setForeground(color);
    if (!fontFamily.isEmpty())
        charFormat.setFontFamilies({ fontFamily });
    if (bold)
        charFormat.setFontWeight(QFont::Bold);
    if (deltaPtSize != 0) {
        qreal basePtSize = this->format(start).font().pointSizeF();
        if (basePtSize <= 0 && this->document() != nullptr)
            basePtSize = this->document()->defaultFont().pointSizeF();
        if (basePtSize > 0)
            charFormat.setFontPointSize(basePtSize + deltaPtSize);
    }
    this->mergeFormat(start, count, charFormat);
}

void StoryBeatsSyntaxHighlighterDelegate::highlightBlock(const QString &text)
{
    if (text.isEmpty())
        return;

    // Only the first ':' after the name separates it from the description, because
    // descriptions themselves contain colons.
    int nameEnd = -1;
    int descriptionStart = -1;

    if (text.at(0) == QChar('[')) {
        const int close = text.indexOf(QChar(']'));
        if (close < 0)
            return;

        this->applyStyle(0, close + 1, m_templateColor, true,
                         Utils::Platform::isMacOSDesktop() ? 2 : 1);

        nameEnd = close + 1;
        descriptionStart = text.indexOf(QChar(':'), nameEnd);
        if (descriptionStart >= 0)
            ++descriptionStart;
    } else if (text.at(0) == QChar('<')) {
        const int close = text.indexOf(QChar('>'));
        if (close < 0)
            return;

        this->applyStyle(0, close + 1, m_actColor, true);

        const int colon = text.indexOf(QChar(':'), close + 1);
        nameEnd = colon < 0 ? text.length() : colon;
        this->applyStyle(close + 1, nameEnd - (close + 1), m_beatColor, true);

        descriptionStart = colon < 0 ? -1 : colon + 1;
    } else {
        return;
    }

    // Descriptions are dense prose, so they are drawn one point smaller than the names.
    if (descriptionStart >= 0)
        this->applyStyle(descriptionStart, text.length() - descriptionStart, m_descriptionColor,
                         false, Utils::Platform::isMacOSDesktop() ? -1 : -2);
}

///////////////////////////////////////////////////////////////////////////////

QmlSyntaxHighlighterDelegate::QmlSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
}

QmlSyntaxHighlighterDelegate::~QmlSyntaxHighlighterDelegate() { }

void QmlSyntaxHighlighterDelegate::setForeground(int start, int count, const QColor &color)
{
    if (count <= 0)
        return;

    QTextCharFormat charFormat;
    charFormat.setForeground(color);
    this->mergeFormat(start, count, charFormat);
}

void QmlSyntaxHighlighterDelegate::setBackground(int start, int count, const QColor &color)
{
    if (count <= 0)
        return;

    QTextCharFormat charFormat;
    charFormat.setBackground(color);
    this->mergeFormat(start, count, charFormat);
}

void QmlSyntaxHighlighterDelegate::setFont(int start, int count, const QFont &font)
{
    if (count <= 0)
        return;

    QTextCharFormat charFormat;
    charFormat.setFont(font);
    this->mergeFormat(start, count, charFormat);
}

QFont QmlSyntaxHighlighterDelegate::font(const QString &family, int pointSize, bool bold,
                                         bool italic, bool underline) const
{
    QFont ret(family, pointSize);
    ret.setBold(bold);
    ret.setItalic(italic);
    ret.setUnderline(underline);
    return ret;
}

void QmlSyntaxHighlighterDelegate::highlightBlock(const QString &text)
{
    emit highlightRequest(text);
}

///////////////////////////////////////////////////////////////////////////////

MarkdownSyntaxHighlighterDelegate::MarkdownSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
}

MarkdownSyntaxHighlighterDelegate::~MarkdownSyntaxHighlighterDelegate() { }

void MarkdownSyntaxHighlighterDelegate::setFont(const QFont &val)
{
    if (m_font == val)
        return;

    m_font = val;
    emit fontChanged();

    this->rehighlight();
}

void MarkdownSyntaxHighlighterDelegate::setColor(QColor &member, const QColor &val,
                                                 void (MarkdownSyntaxHighlighterDelegate::*changed)())
{
    if (member == val)
        return;

    member = val;
    emit(this->*changed)();

    this->rehighlight();
}

QFont MarkdownSyntaxHighlighterDelegate::variantOfFont(int deltaPtSize, bool bold, bool italic,
                                                       bool underline) const
{
    QFont ret(m_font.family(), m_font.pointSize() + deltaPtSize);
    ret.setBold(bold);
    ret.setItalic(italic);
    ret.setUnderline(underline);
    return ret;
}

void MarkdownSyntaxHighlighterDelegate::applyStyle(int start, int count, const QColor &foreground,
                                                   const QFont *font, const QColor &background)
{
    if (count <= 0)
        return;

    QTextCharFormat charFormat;
    if (foreground.isValid())
        charFormat.setForeground(foreground);
    if (background.isValid())
        charFormat.setBackground(background);
    if (font != nullptr)
        charFormat.setFont(*font);
    this->mergeFormat(start, count, charFormat);
}

// Colors, and sizes up, headings. Returns true if text is a heading.
bool MarkdownSyntaxHighlighterDelegate::highlightHeading(const QString &text)
{
    static const QRegularExpression regex(QStringLiteral("^(#{1,6})[ \\t]+\\S.*$"));

    const QRegularExpressionMatch match = regex.match(text);
    if (!match.hasMatch())
        return false;

    const int level = match.capturedLength(1);
    const QFont font = this->variantOfFont(qMax(0, 5 - level), true, false, false);
    this->applyStyle(0, text.length(), m_headingColors[level - 1], &font);
    return true;
}

// Lines that are not highlighted any further. Returns true if text is one of them.
bool MarkdownSyntaxHighlighterDelegate::highlightCodeFenceOrRule(const QString &text)
{
    static const QRegularExpression fence(QStringLiteral("^\\s*(```|~~~)"));
    static const QRegularExpression rule(QStringLiteral("^\\s*([-*_])(\\s*\\1){2,}\\s*$"));

    if (fence.match(text).hasMatch()) {
        this->applyStyle(0, text.length(), m_codeColor, nullptr, m_codeBackground);
        return true;
    }

    if (rule.match(text).hasMatch()) {
        this->applyStyle(0, text.length(), m_markerColor);
        return true;
    }

    return false;
}

void MarkdownSyntaxHighlighterDelegate::highlightBlockLevel(const QString &text)
{
    static const QRegularExpression quote(QStringLiteral("^\\s*>"));
    static const QRegularExpression listMarker(QStringLiteral("^\\s*([-*+]|\\d+[.)])[ \\t]"));

    if (quote.match(text).hasMatch())
        this->applyStyle(0, text.length(), m_quoteColor);

    const QRegularExpressionMatch marker = listMarker.match(text);
    if (marker.hasMatch())
        this->applyStyle(marker.capturedStart(1), marker.capturedLength(1), m_markerColor);
}

void MarkdownSyntaxHighlighterDelegate::highlightInline(const QString &text)
{
    static const QRegularExpression strike(QStringLiteral("~~\\S(?:.*?\\S)?~~"));
    static const QRegularExpression asteriskItalic(
            QStringLiteral("\\*(?!\\*)\\S(?:[^*]*?\\S)?\\*(?!\\*)"));
    static const QRegularExpression underscoreItalic(
            QStringLiteral("(^|\\W)(_\\S(?:[^_]*?\\S)?_)(?!\\w)"));
    static const QRegularExpression bold(QStringLiteral("(\\*\\*|__)\\S(?:.*?\\S)?\\1"));
    static const QRegularExpression boldItalic(QStringLiteral("(\\*\\*\\*|___)\\S(?:.*?\\S)?\\1"));
    static const QRegularExpression link(QStringLiteral("\\[[^\\]\\r\\n]+\\]\\([^)\\r\\n]+\\)"));
    static const QRegularExpression code(QStringLiteral("`[^`\\r\\n]+`"));

    // Calls func(start, count) for each non-empty match of the given capture group.
    auto forEachMatch = [&text](const QRegularExpression &regex, int group, auto func) {
        QRegularExpressionMatchIterator it = regex.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            if (match.capturedLength(group) > 0)
                func(match.capturedStart(group), match.capturedLength(group));
        }
    };

    const QFont italicFont = this->variantOfFont(0, false, true, false);
    const QFont boldFont = this->variantOfFont(0, true, false, false);
    const QFont boldItalicFont = this->variantOfFont(0, true, true, false);
    const QFont linkFont = this->variantOfFont(0, false, false, true);

    // Emphasis from the least to the most specific, so that the most specific wins.
    forEachMatch(strike, 0, [&](int s, int n) { this->applyStyle(s, n, m_markerColor); });
    forEachMatch(asteriskItalic, 0,
                 [&](int s, int n) { this->applyStyle(s, n, m_emphasisColor, &italicFont); });
    forEachMatch(underscoreItalic, 2,
                 [&](int s, int n) { this->applyStyle(s, n, m_emphasisColor, &italicFont); });
    forEachMatch(bold, 0, [&](int s, int n) { this->applyStyle(s, n, m_emphasisColor, &boldFont); });
    forEachMatch(boldItalic, 0,
                 [&](int s, int n) { this->applyStyle(s, n, m_emphasisColor, &boldItalicFont); });
    forEachMatch(link, 0, [&](int s, int n) { this->applyStyle(s, n, m_linkColor, &linkFont); });
    forEachMatch(code, 0,
                 [&](int s, int n) { this->applyStyle(s, n, m_codeColor, nullptr, m_codeBackground); });
}

void MarkdownSyntaxHighlighterDelegate::highlightBlock(const QString &text)
{
    if (text.isEmpty())
        return;

    if (!this->highlightHeading(text) && !this->highlightCodeFenceOrRule(text)) {
        this->highlightBlockLevel(text);
        this->highlightInline(text);
    }
}
