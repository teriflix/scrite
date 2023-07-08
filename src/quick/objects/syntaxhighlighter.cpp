/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "application.h"
#include "textlimiter.h"
#include "scritedocument.h"
#include "transliteration.h"
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
    ~SyntaxHighlighterUserData() { }

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
    }

    emit textDocumentChanged();

    this->documentContentsChanged();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    for (AbstractSyntaxHighlighterDelegate *delegate : qAsConst(m_sortedDelegates)) {
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
                                    int index)
{
    return reinterpret_cast<SyntaxHighlighter *>(list->data)->delegateAt(index);
}

int SyntaxHighlighter::staticDelegateCount(
        QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list)
{
    return reinterpret_cast<SyntaxHighlighter *>(list->data)->delegateCount();
}

void SyntaxHighlighter::sortDelegates()
{
    m_sortedDelegates = m_delegates;
    std::sort(m_sortedDelegates.begin(), m_sortedDelegates.end(),
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
    for (AbstractSyntaxHighlighterDelegate *delegate : qAsConst(m_sortedDelegates))
        delegate->documentsContentsChange(from, charsRemoved, charsAdded);
}

void SyntaxHighlighter::documentContentsChanged()
{
    for (AbstractSyntaxHighlighterDelegate *delegate : qAsConst(m_sortedDelegates))
        delegate->documentContentsChanged();
}

///////////////////////////////////////////////////////////////////////////////

LanguageFontSyntaxHighlighterDelegate::LanguageFontSyntaxHighlighterDelegate(QObject *parent)
    : AbstractSyntaxHighlighterDelegate(parent)
{
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
        qDebug() << "PAF: " << defaultFont;

        QTextCharFormat defaultFormat;
        defaultFormat.setFont(defaultFont);
        this->setFormat(0, block.length(), defaultFormat);
    }

    const QList<TransliterationEngine::Boundary> boundaries =
            TransliterationEngine::instance()->evaluateBoundaries(text);

    for (const TransliterationEngine::Boundary &boundary : boundaries) {
        if (boundary.isEmpty() || boundary.language == TransliterationEngine::English)
            continue;

        QTextCharFormat format;
        format.setFontFamily(boundary.font.family());
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
    ~SpellCheckSyntaxHighlighterUserData() { }

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

    const QList<TextFragment> fragments = userData->misspelledFragments();
    if (!fragments.isEmpty()) {
        for (const TextFragment &fragment : fragments) {
            if (!fragment.isValid())
                continue;

            const QString word = text.mid(fragment.start(), fragment.length());
            const QChar::Script script = TransliterationEngine::determineScript(word);
            if (script != QChar::Script_Latin)
                continue;

            QTextCharFormat spellingErrorFormat;
            if (m_backgroundColor.alpha() > 0)
                spellingErrorFormat.setBackground(m_backgroundColor);
            if (m_textColor.alpha() > 0)
                spellingErrorFormat.setForeground(m_textColor);

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
            charFormat.setForeground(m_textColor);
        if (m_backgroundColor.alpha() > 0)
            charFormat.setBackground(m_backgroundColor);
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
