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

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QQmlEngine>
#include <QQmlParserStatus>
#include <QSyntaxHighlighter>
#include <QQuickTextDocument>

#include "textlimiter.h"

class SyntaxHighlighter;

// This class is supposed to mimic QSyntaxHighlighter, without actually being one.
class AbstractSyntaxHighlighterDelegate : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Abstract class, cannot be instantiated.")
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit AbstractSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~AbstractSyntaxHighlighterDelegate() override;

    Q_SIGNAL void aboutToDelete(AbstractSyntaxHighlighterDelegate *ptr);

    QTextDocument *document() const;

    bool instantiatedInQml() const { return m_intantiatedInQml; }

    // clang-format off
    Q_PROPERTY(bool enabled
               READ isEnabled
               WRITE setEnabled
               NOTIFY enabledChanged)
    // clang-format on
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    // QQmlParserStatus interface
    void classBegin() override { m_intantiatedInQml = true; }
    void componentComplete() override { m_intantiatedInQml = true; }

public Q_SLOTS:
    void rehighlight();
    void rehighlightBlock(const QTextBlock &block);

protected:
    virtual void highlightBlock(const QString &text) = 0;
    virtual int priority() const { return 0; }
    virtual void documentsContentsChange(int from, int charsRemoved, int charsAdded);
    virtual void documentContentsChanged();

    void mergeFormat(int start, int count, const QTextCharFormat &format);

    void setFormat(int start, int count, const QTextCharFormat &format);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    QTextCharFormat format(int pos) const;

    int previousBlockState() const;
    int currentBlockState() const;
    void setCurrentBlockState(int newState);

    void setCurrentBlockUserData(QTextBlockUserData *data);
    QTextBlockUserData *currentBlockUserData() const;

    template<class T>
    T *currentBlockUserData() const
    {
        QTextBlockUserData *ret = this->currentBlockUserData();
        return static_cast<T *>(ret);
    }

    void setBlockUserData(QTextBlock &block, QTextBlockUserData *data);
    QTextBlockUserData *blockUserData(const QTextBlock &block) const;

    template<class T>
    T *blockUserData(const QTextBlock &block) const
    {
        QTextBlockUserData *ret = this->blockUserData(block);
        return static_cast<T *>(ret);
    }

    QTextBlock currentBlock() const;

private:
    friend class SyntaxHighlighter;
    SyntaxHighlighter *m_highlighter = nullptr;
    bool m_enabled = true;
    bool m_intantiatedInQml = false;
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(SyntaxHighlighter)
    // clang-format off
    Q_CLASSINFO("DefaultProperty", "delegates")
    // clang-format on

public:
    explicit SyntaxHighlighter(QObject *parent = nullptr);
    ~SyntaxHighlighter() override;

    static SyntaxHighlighter *qmlAttachedProperties(QObject *object);
    static SyntaxHighlighter *get(QObject *object) { return qmlAttachedProperties(object); }

    // clang-format off
    Q_PROPERTY(QQuickTextDocument *textDocument
               READ textDocument
               WRITE setTextDocument
               NOTIFY textDocumentChanged)
    // clang-format on
    void setTextDocument(QQuickTextDocument *val);
    QQuickTextDocument *textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    // clang-format off
    Q_PROPERTY(bool textDocumentUndoRedoEnabled
               READ isTextDocumentUndoRedoEnabled
               WRITE setTextDocumentUndoRedoEnabled
               NOTIFY textDocumentUndoRedoEnabledChanged)
    // clang-format on
    void setTextDocumentUndoRedoEnabled(bool val);
    bool isTextDocumentUndoRedoEnabled() const { return m_textDocumentUndoRedoEnabled; }
    Q_SIGNAL void textDocumentUndoRedoEnabledChanged();

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text) override;

public:
    // clang-format off
    Q_PROPERTY(QQmlListProperty<AbstractSyntaxHighlighterDelegate> delegates
               READ delegates
               NOTIFY delegateCountChanged)
    // clang-format on
    QQmlListProperty<AbstractSyntaxHighlighterDelegate> delegates();
    Q_INVOKABLE void addDelegate(AbstractSyntaxHighlighterDelegate *ptr);
    Q_INVOKABLE void removeDelegate(AbstractSyntaxHighlighterDelegate *ptr);
    Q_INVOKABLE AbstractSyntaxHighlighterDelegate *delegateAt(int index) const;
    // clang-format off
    Q_PROPERTY(int delegateCount
               READ delegateCount
               NOTIFY delegateCountChanged)
    // clang-format on
    int delegateCount() const { return m_delegates.size(); }
    Q_INVOKABLE void clearDelegates();
    Q_SIGNAL void delegateCountChanged();

    Q_INVOKABLE AbstractSyntaxHighlighterDelegate *
    findDelegate(const QString &className, const QString &objectName = QString()) const;

    template<class T>
    T *findDelegate(const QString &objectName = QString()) const
    {
        for (AbstractSyntaxHighlighterDelegate *delegate : m_delegates) {
            T *tdelegate = qobject_cast<T *>(delegate);
            if (tdelegate) {
                if (objectName.isEmpty() || delegate->objectName() == objectName)
                    return tdelegate;
            }
        }

        return (T *)nullptr;
    }

private:
    static void staticAppendDelegate(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list,
                                     AbstractSyntaxHighlighterDelegate *ptr);
    static void staticClearDelegates(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list);
    static AbstractSyntaxHighlighterDelegate *
    staticDelegateAt(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list, qsizetype index);
    static qsizetype staticDelegateCount(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list);

    void sortDelegates();

    void onDelegateAboutToDelete(AbstractSyntaxHighlighterDelegate *ptr);

    void setCurrentBlockDelegateUserData(AbstractSyntaxHighlighterDelegate *delegate,
                                         QTextBlockUserData *data);
    QTextBlockUserData *
    getCurrentBlockDelegateUserData(const AbstractSyntaxHighlighterDelegate *delegate) const;

    template<class T>
    T *getCurrentBlockDelegateUserData(const AbstractSyntaxHighlighterDelegate *delegate) const
    {
        QTextBlockUserData *userData = getCurrentBlockDelegateUserData(delegate);
        return static_cast<T *>(userData);
    }

    void setBlockDelegateUserData(QTextBlock &block, AbstractSyntaxHighlighterDelegate *delegate,
                                  QTextBlockUserData *data);
    QTextBlockUserData *
    getBlockDelegateUserData(const QTextBlock &block,
                             const AbstractSyntaxHighlighterDelegate *delegate) const;
    template<class T>
    T *getBlockDelegateUserData(const QTextBlock &block,
                                const AbstractSyntaxHighlighterDelegate *delegate) const
    {
        QTextBlockUserData *userData = this->getBlockDelegateUserData(block, delegate);
        return static_cast<T *>(userData);
    }

    void documentContentsChange(int from, int charsRemoved, int charsAdded);
    void documentContentsChanged();

private:
    friend class AbstractSyntaxHighlighterDelegate;
    bool m_textDocumentUndoRedoEnabled = false;
    QQuickTextDocument *m_textDocument = nullptr;
    QList<AbstractSyntaxHighlighterDelegate *> m_delegates, m_sortedDelegates;
};

class LanguageFontSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LanguageFontSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~LanguageFontSyntaxHighlighterDelegate() override;

    // If specified, it has to be a QFont value. When enforceDefaultFont is true,
    // the default font set here or that of this->document().defaultFont() will be
    // force applied to the document.
    // clang-format off
    Q_PROPERTY(QVariant defaultFont
               READ defaultFont
               WRITE setDefaultFont
               NOTIFY defaultFontChanged)
    // clang-format on
    void setDefaultFont(const QVariant &val);
    QVariant defaultFont() const { return m_defaultFont; }
    Q_SIGNAL void defaultFontChanged();

    // Keep this true if you want default font to be applied always.
    // clang-format off
    Q_PROPERTY(bool enforceDefaultFont
               READ isEnforceDefaultFont
               WRITE setEnforceDefaultFont
               NOTIFY enforceDefaultFontChanged)
    // clang-format on
    void setEnforceDefaultFont(bool val);
    bool isEnforceDefaultFont() const { return m_enforceDefaultFont; }
    Q_SIGNAL void enforceDefaultFontChanged();

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text) override;
    int priority() const override { return -1; } // because default font has to be applied first.

private:
    bool m_enforceDefaultFont = true;
    QVariant m_defaultFont;
};

class HeadingFontSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit HeadingFontSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~HeadingFontSyntaxHighlighterDelegate() override;

    // All attributes of the font, except family will be applied.

    // clang-format off
    Q_PROPERTY(QFont h1
               READ h1
               WRITE setH1
               NOTIFY h1Changed)
    // clang-format on
    void setH1(const QFont &val);
    QFont h1() const { return m_h1; }
    Q_SIGNAL void h1Changed();

    // clang-format off
    Q_PROPERTY(QFont h2
               READ h2
               WRITE setH2
               NOTIFY h2Changed)
    // clang-format on
    void setH2(const QFont &val);
    QFont h2() const { return m_h2; }
    Q_SIGNAL void h2Changed();

    // clang-format off
    Q_PROPERTY(QFont h3
               READ h3
               WRITE setH3
               NOTIFY h3Changed)
    // clang-format on
    void setH3(const QFont &val);
    QFont h3() const { return m_h3; }
    Q_SIGNAL void h3Changed();

    // clang-format off
    Q_PROPERTY(QFont h4
               READ h4
               WRITE setH4
               NOTIFY h4Changed)
    // clang-format on
    void setH4(const QFont &val);
    QFont h4() const { return m_h4; }
    Q_SIGNAL void h4Changed();

    // clang-format off
    Q_PROPERTY(QFont h5
               READ h5
               WRITE setH5
               NOTIFY h5Changed)
    // clang-format on
    void setH5(const QFont &val);
    QFont h5() const { return m_h5; }
    Q_SIGNAL void h5Changed();

    // clang-format off
    Q_PROPERTY(QFont normal
               READ normal
               WRITE setNormal
               NOTIFY normalChanged)
    // clang-format on
    void setNormal(const QFont &val);
    QFont normal() const { return m_normal; }
    Q_SIGNAL void normalChanged();

    Q_INVOKABLE void initializeWithNormalFontAs(const QFont &font);

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text) override;

private:
    QFont m_h1;
    QFont m_h2;
    QFont m_h3;
    QFont m_h4;
    QFont m_h5;
    QFont m_normal;
};

struct TextFragment;
class SpellCheckSyntaxHighlighterUserData;
class SpellCheckSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SpellCheckSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~SpellCheckSyntaxHighlighterDelegate() override;

    // clang-format off
    Q_PROPERTY(QColor textColor
               READ textColor
               WRITE setTextColor
               NOTIFY textColorChanged)
    // clang-format on
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    // clang-format off
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    // clang-format off
    Q_PROPERTY(int cursorPosition
               READ cursorPosition
               WRITE setCursorPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    // clang-format off
    Q_PROPERTY(bool wordUnderCursorIsMisspelled
               READ isWordUnderCursorIsMisspelled
               NOTIFY wordUnderCursorIsMisspelledChanged)
    // clang-format on
    bool isWordUnderCursorIsMisspelled() const { return m_wordUnderCursorIsMisspelled; }
    Q_SIGNAL void wordUnderCursorIsMisspelledChanged();

    // clang-format off
    Q_PROPERTY(QStringList spellingSuggestionsForWordUnderCursor
               READ spellingSuggestionsForWordUnderCursor
               NOTIFY spellingSuggestionsForWordUnderCursorChanged)
    // clang-format on
    QStringList spellingSuggestionsForWordUnderCursor() const
    {
        return m_spellingSuggestionsForWordUnderCursor;
    }
    Q_SIGNAL void spellingSuggestionsForWordUnderCursorChanged();

    Q_INVOKABLE QStringList spellingSuggestionsForWordAt(int cursorPosition) const;

    Q_INVOKABLE void replaceWordAt(int cursorPosition, const QString &with);
    Q_INVOKABLE void replaceWordUnderCursor(const QString &with)
    {
        this->replaceWordAt(m_cursorPosition, with);
    }

    Q_INVOKABLE void addWordAtPositionToDictionary(int cursorPosition);
    Q_INVOKABLE void addWordUnderCursorToDictionary()
    {
        this->addWordAtPositionToDictionary(m_cursorPosition);
    }

    Q_INVOKABLE void addWordAtPositionToIgnoreList(int cursorPosition);
    Q_INVOKABLE void addWordUnderCursorToIgnoreList()
    {
        this->addWordAtPositionToIgnoreList(m_cursorPosition);
    }

    void checkForSpellingMistakeInCurrentWord();

signals:
    void spellingMistakesDetected();

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text) override;

private:
    void setWordUnderCursorIsMisspelled(bool val);
    void setSpellingSuggestionsForWordUnderCursor(const QStringList &val);

    bool findMisspelledTextFragment(int cursorPosition, TextFragment &misspelledFragment) const;
    bool wordCursor(int cursorPosition, QTextCursor &cursor,
                    SpellCheckSyntaxHighlighterUserData *&ud) const;

private:
    QColor m_textColor = QColor(0, 0, 0, 0);
    QColor m_backgroundColor = QColor(255, 0, 0, 64);

    int m_cursorPosition = -1;
    bool m_wordUnderCursorIsMisspelled = false;
    QStringList m_spellingSuggestionsForWordUnderCursor;
};

class TextLimiterSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TextLimiterSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~TextLimiterSyntaxHighlighterDelegate() override;

    // clang-format off
    Q_PROPERTY(TextLimiter *textLimiter
               READ textLimiter
               WRITE setTextLimiter
               NOTIFY textLimiterChanged)
    // clang-format on
    void setTextLimiter(TextLimiter *val);
    TextLimiter *textLimiter() const { return m_textLimiter; }
    Q_SIGNAL void textLimiterChanged();

    // clang-format off
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    // clang-format off
    Q_PROPERTY(QColor textColor
               READ textColor
               WRITE setTextColor
               NOTIFY textColorChanged)
    // clang-format on
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    // clang-format off
    Q_PROPERTY(int cursorLimitPosition
               READ cursorLimitPosition
               NOTIFY cursorLimitPositionChanged)
    // clang-format on
    int cursorLimitPosition() const { return m_cursorLimitPosition; }
    Q_SIGNAL void cursorLimitPositionChanged();

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text) override;
    void documentContentsChanged() override { this->evaluateCursorLimitPosition(); }

private:
    void setCursorLimitPosition(int val);
    void evaluateCursorLimitPosition();

private:
    int m_cursorLimitPosition = -1;
    TextLimiter *m_textLimiter = nullptr;
    QColor m_backgroundColor = QColor(0, 0, 0, 0);
    QColor m_textColor = QColor(Qt::red);
};

// Highlights lines in the storybeats.lst format:
//     [Beat Sheet Template]: description
//     <ACT NAME>Beat Name: description
// Colors with zero alpha leave the corresponding text unformatted.
class StoryBeatsSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit StoryBeatsSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~StoryBeatsSyntaxHighlighterDelegate() override;

    // clang-format off
    Q_PROPERTY(QColor templateColor
               READ templateColor
               WRITE setTemplateColor
               NOTIFY templateColorChanged)
    // clang-format on
    void setTemplateColor(const QColor &val);
    QColor templateColor() const { return m_templateColor; }
    Q_SIGNAL void templateColorChanged();

    // clang-format off
    Q_PROPERTY(QColor actColor
               READ actColor
               WRITE setActColor
               NOTIFY actColorChanged)
    // clang-format on
    void setActColor(const QColor &val);
    QColor actColor() const { return m_actColor; }
    Q_SIGNAL void actColorChanged();

    // clang-format off
    Q_PROPERTY(QColor beatColor
               READ beatColor
               WRITE setBeatColor
               NOTIFY beatColorChanged)
    // clang-format on
    void setBeatColor(const QColor &val);
    QColor beatColor() const { return m_beatColor; }
    Q_SIGNAL void beatColorChanged();

    // clang-format off
    Q_PROPERTY(QColor descriptionColor
               READ descriptionColor
               WRITE setDescriptionColor
               NOTIFY descriptionColorChanged)
    // clang-format on
    void setDescriptionColor(const QColor &val);
    QColor descriptionColor() const { return m_descriptionColor; }
    Q_SIGNAL void descriptionColorChanged();

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text) override;

private:
    void applyStyle(int start, int count, const QColor &color, bool bold = false,
                    int deltaPtSize = 0, const QString &fontFamily = QString());

private:
    QColor m_templateColor = QColor(Qt::darkMagenta);
    QColor m_actColor = QColor(Qt::darkGreen);
    QColor m_beatColor = QColor(Qt::blue);
    QColor m_descriptionColor = QColor(0, 0, 0, 0);
};

// Lets QML do the highlighting. For every block, highlightRequest(text) is emitted
// synchronously. Handlers can call setForeground(), setBackground() and setFont() on
// ranges of that text. Those calls are only meaningful from within the handler.
class QmlSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SyntaxHighlighterDelegate)

public:
    explicit QmlSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~QmlSyntaxHighlighterDelegate() override;

    Q_INVOKABLE void setForeground(int start, int count, const QColor &color);
    Q_INVOKABLE void setBackground(int start, int count, const QColor &color);
    Q_INVOKABLE void setFont(int start, int count, const QFont &font);

    Q_INVOKABLE QFont font(const QString &family, int pointSize, bool bold = false,
                           bool italic = false, bool underline = false) const;

    Q_SIGNAL void highlightRequest(const QString &text);

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text) override;
};

// Highlights markdown. Blocks are highlighted one at a time, with no state from one block to the
// next, so what spans lines is not highlighted: only the lines that open and close a fenced code
// block are. Headings get no emphasis inside them, and emphasis inside emphasis is not nested.
// Only the size, weight, italic and underline of the font are set, never the family, so the fonts
// applied by LanguageFontSyntaxHighlighterDelegate are kept.
class MarkdownSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MarkdownSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~MarkdownSyntaxHighlighterDelegate() override;

    // Only the point size is used. The size of headings is derived from it.
    // clang-format off
    Q_PROPERTY(QFont font
               READ font
               WRITE setFont
               NOTIFY fontChanged)
    // clang-format on
    void setFont(const QFont &val);
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    // clang-format off
    Q_PROPERTY(QColor heading1Color
               READ heading1Color
               WRITE setHeading1Color
               NOTIFY heading1ColorChanged)
    // clang-format on
    void setHeading1Color(const QColor &val) { this->setColor(m_headingColors[0], val, &MarkdownSyntaxHighlighterDelegate::heading1ColorChanged); }
    QColor heading1Color() const { return m_headingColors[0]; }
    Q_SIGNAL void heading1ColorChanged();

    // clang-format off
    Q_PROPERTY(QColor heading2Color
               READ heading2Color
               WRITE setHeading2Color
               NOTIFY heading2ColorChanged)
    // clang-format on
    void setHeading2Color(const QColor &val) { this->setColor(m_headingColors[1], val, &MarkdownSyntaxHighlighterDelegate::heading2ColorChanged); }
    QColor heading2Color() const { return m_headingColors[1]; }
    Q_SIGNAL void heading2ColorChanged();

    // clang-format off
    Q_PROPERTY(QColor heading3Color
               READ heading3Color
               WRITE setHeading3Color
               NOTIFY heading3ColorChanged)
    // clang-format on
    void setHeading3Color(const QColor &val) { this->setColor(m_headingColors[2], val, &MarkdownSyntaxHighlighterDelegate::heading3ColorChanged); }
    QColor heading3Color() const { return m_headingColors[2]; }
    Q_SIGNAL void heading3ColorChanged();

    // clang-format off
    Q_PROPERTY(QColor heading4Color
               READ heading4Color
               WRITE setHeading4Color
               NOTIFY heading4ColorChanged)
    // clang-format on
    void setHeading4Color(const QColor &val) { this->setColor(m_headingColors[3], val, &MarkdownSyntaxHighlighterDelegate::heading4ColorChanged); }
    QColor heading4Color() const { return m_headingColors[3]; }
    Q_SIGNAL void heading4ColorChanged();

    // clang-format off
    Q_PROPERTY(QColor heading5Color
               READ heading5Color
               WRITE setHeading5Color
               NOTIFY heading5ColorChanged)
    // clang-format on
    void setHeading5Color(const QColor &val) { this->setColor(m_headingColors[4], val, &MarkdownSyntaxHighlighterDelegate::heading5ColorChanged); }
    QColor heading5Color() const { return m_headingColors[4]; }
    Q_SIGNAL void heading5ColorChanged();

    // clang-format off
    Q_PROPERTY(QColor heading6Color
               READ heading6Color
               WRITE setHeading6Color
               NOTIFY heading6ColorChanged)
    // clang-format on
    void setHeading6Color(const QColor &val) { this->setColor(m_headingColors[5], val, &MarkdownSyntaxHighlighterDelegate::heading6ColorChanged); }
    QColor heading6Color() const { return m_headingColors[5]; }
    Q_SIGNAL void heading6ColorChanged();

    // clang-format off
    Q_PROPERTY(QColor emphasisColor
               READ emphasisColor
               WRITE setEmphasisColor
               NOTIFY emphasisColorChanged)
    // clang-format on
    void setEmphasisColor(const QColor &val) { this->setColor(m_emphasisColor, val, &MarkdownSyntaxHighlighterDelegate::emphasisColorChanged); }
    QColor emphasisColor() const { return m_emphasisColor; }
    Q_SIGNAL void emphasisColorChanged();

    // clang-format off
    Q_PROPERTY(QColor codeColor
               READ codeColor
               WRITE setCodeColor
               NOTIFY codeColorChanged)
    // clang-format on
    void setCodeColor(const QColor &val) { this->setColor(m_codeColor, val, &MarkdownSyntaxHighlighterDelegate::codeColorChanged); }
    QColor codeColor() const { return m_codeColor; }
    Q_SIGNAL void codeColorChanged();

    // clang-format off
    Q_PROPERTY(QColor codeBackground
               READ codeBackground
               WRITE setCodeBackground
               NOTIFY codeBackgroundChanged)
    // clang-format on
    void setCodeBackground(const QColor &val) { this->setColor(m_codeBackground, val, &MarkdownSyntaxHighlighterDelegate::codeBackgroundChanged); }
    QColor codeBackground() const { return m_codeBackground; }
    Q_SIGNAL void codeBackgroundChanged();

    // clang-format off
    Q_PROPERTY(QColor linkColor
               READ linkColor
               WRITE setLinkColor
               NOTIFY linkColorChanged)
    // clang-format on
    void setLinkColor(const QColor &val) { this->setColor(m_linkColor, val, &MarkdownSyntaxHighlighterDelegate::linkColorChanged); }
    QColor linkColor() const { return m_linkColor; }
    Q_SIGNAL void linkColorChanged();

    // clang-format off
    Q_PROPERTY(QColor markerColor
               READ markerColor
               WRITE setMarkerColor
               NOTIFY markerColorChanged)
    // clang-format on
    void setMarkerColor(const QColor &val) { this->setColor(m_markerColor, val, &MarkdownSyntaxHighlighterDelegate::markerColorChanged); }
    QColor markerColor() const { return m_markerColor; }
    Q_SIGNAL void markerColorChanged();

    // clang-format off
    Q_PROPERTY(QColor quoteColor
               READ quoteColor
               WRITE setQuoteColor
               NOTIFY quoteColorChanged)
    // clang-format on
    void setQuoteColor(const QColor &val) { this->setColor(m_quoteColor, val, &MarkdownSyntaxHighlighterDelegate::quoteColorChanged); }
    QColor quoteColor() const { return m_quoteColor; }
    Q_SIGNAL void quoteColorChanged();

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text) override;

private:
    void setColor(QColor &member, const QColor &val, void (MarkdownSyntaxHighlighterDelegate::*changed)());
    QTextCharFormat fontFormat(int deltaPtSize, bool bold, bool italic, bool underline) const;
    void applyStyle(int start, int count, const QColor &foreground,
                    const QTextCharFormat *fontFormat = nullptr,
                    const QColor &background = QColor());

    bool highlightHeading(const QString &text);
    bool highlightCodeFenceOrRule(const QString &text);
    void highlightBlockLevel(const QString &text);
    void highlightInline(const QString &text);

private:
    QFont m_font;
    QColor m_headingColors[6] = { QColor("#1d4ed8"), QColor("#7c3aed"), QColor("#be185d"),
                                  QColor("#c2410c"), QColor("#a16207"), QColor("#4d7c0f") };
    QColor m_emphasisColor = QColor("#0369a1");
    QColor m_codeColor = QColor("#9a3412");
    QColor m_codeBackground = QColor::fromRgbF(0.5, 0.5, 0.5, 0.15);
    QColor m_linkColor = QColor("#2563eb");
    QColor m_markerColor = QColor("#6b7280");
    QColor m_quoteColor = QColor("#6b7280");
};

#endif // SYNTAXHIGHLIGHTER_H
