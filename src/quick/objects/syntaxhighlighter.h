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

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QQmlEngine>
#include <QQmlParserStatus>
#include <QSyntaxHighlighter>
#include <QQuickTextDocument>

class SyntaxHighlighter;

// This class is supposed to mimic QSyntaxHighlighter, without actually being one.
class AbstractSyntaxHighlighterDelegate : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit AbstractSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~AbstractSyntaxHighlighterDelegate();

    Q_SIGNAL void aboutToDelete(AbstractSyntaxHighlighterDelegate *ptr);

    QTextDocument *document() const;

    bool instantiatedInQml() const { return m_intantiatedInQml; }

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    // QQmlParserStatus interface
    void classBegin() { m_intantiatedInQml = true; }
    void componentComplete() { m_intantiatedInQml = true; }

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
    Q_CLASSINFO("DefaultProperty", "delegates")

public:
    explicit SyntaxHighlighter(QObject *parent = nullptr);
    ~SyntaxHighlighter();

    static SyntaxHighlighter *qmlAttachedProperties(QObject *object);
    static SyntaxHighlighter *get(QObject *object) { return qmlAttachedProperties(object); }

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged)
    void setTextDocument(QQuickTextDocument *val);
    QQuickTextDocument *textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text);

public:
    Q_PROPERTY(QQmlListProperty<AbstractSyntaxHighlighterDelegate> delegates READ delegates NOTIFY delegateCountChanged)
    QQmlListProperty<AbstractSyntaxHighlighterDelegate> delegates();
    Q_INVOKABLE void addDelegate(AbstractSyntaxHighlighterDelegate *ptr);
    Q_INVOKABLE void removeDelegate(AbstractSyntaxHighlighterDelegate *ptr);
    Q_INVOKABLE AbstractSyntaxHighlighterDelegate *delegateAt(int index) const;
    Q_PROPERTY(int delegateCount READ delegateCount NOTIFY delegateCountChanged)
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
    staticDelegateAt(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list, int index);
    static int staticDelegateCount(QQmlListProperty<AbstractSyntaxHighlighterDelegate> *list);

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
    QQuickTextDocument *m_textDocument = nullptr;
    QList<AbstractSyntaxHighlighterDelegate *> m_delegates, m_sortedDelegates;
};

class LanguageFontSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LanguageFontSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~LanguageFontSyntaxHighlighterDelegate();

    // If specified, it has to be a QFont value. When enforceDefaultFont is true,
    // the default font set here or that of this->document().defaultFont() will be
    // force applied to the document.
    Q_PROPERTY(QVariant defaultFont READ defaultFont WRITE setDefaultFont NOTIFY defaultFontChanged)
    void setDefaultFont(const QVariant &val);
    QVariant defaultFont() const { return m_defaultFont; }
    Q_SIGNAL void defaultFontChanged();

    // Keep this true if you want default font to be applied always.
    Q_PROPERTY(bool enforceDefaultFont READ isEnforceDefaultFont WRITE setEnforceDefaultFont NOTIFY enforceDefaultFontChanged)
    void setEnforceDefaultFont(bool val);
    bool isEnforceDefaultFont() const { return m_enforceDefaultFont; }
    Q_SIGNAL void enforceDefaultFontChanged();

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text);
    int priority() const { return -1; } // because default font has to be applied first.

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
    ~HeadingFontSyntaxHighlighterDelegate();

    // All attributes of the font, except family will be applied.

    Q_PROPERTY(QFont h1 READ h1 WRITE setH1 NOTIFY h1Changed)
    void setH1(const QFont &val);
    QFont h1() const { return m_h1; }
    Q_SIGNAL void h1Changed();

    Q_PROPERTY(QFont h2 READ h2 WRITE setH2 NOTIFY h2Changed)
    void setH2(const QFont &val);
    QFont h2() const { return m_h2; }
    Q_SIGNAL void h2Changed();

    Q_PROPERTY(QFont h3 READ h3 WRITE setH3 NOTIFY h3Changed)
    void setH3(const QFont &val);
    QFont h3() const { return m_h3; }
    Q_SIGNAL void h3Changed();

    Q_PROPERTY(QFont h4 READ h4 WRITE setH4 NOTIFY h4Changed)
    void setH4(const QFont &val);
    QFont h4() const { return m_h4; }
    Q_SIGNAL void h4Changed();

    Q_PROPERTY(QFont h5 READ h5 WRITE setH5 NOTIFY h5Changed)
    void setH5(const QFont &val);
    QFont h5() const { return m_h5; }
    Q_SIGNAL void h5Changed();

    Q_PROPERTY(QFont normal READ normal WRITE setNormal NOTIFY normalChanged)
    void setNormal(const QFont &val);
    QFont normal() const { return m_normal; }
    Q_SIGNAL void normalChanged();

    Q_INVOKABLE void initializeWithNormalFontAs(const QFont &font);

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text);

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
    ~SpellCheckSyntaxHighlighterDelegate();

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_PROPERTY(bool wordUnderCursorIsMisspelled READ isWordUnderCursorIsMisspelled NOTIFY wordUnderCursorIsMisspelledChanged)
    bool isWordUnderCursorIsMisspelled() const { return m_wordUnderCursorIsMisspelled; }
    Q_SIGNAL void wordUnderCursorIsMisspelledChanged();

    Q_PROPERTY(QStringList spellingSuggestionsForWordUnderCursor READ spellingSuggestionsForWordUnderCursor NOTIFY spellingSuggestionsForWordUnderCursorChanged)
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
    void highlightBlock(const QString &text);

private:
    void setWordUnderCursorIsMisspelled(bool val);
    void setSpellingSuggestionsForWordUnderCursor(const QStringList &val);

    bool findMisspelledTextFragment(int cursorPosition, TextFragment &misspelledFragment) const;
    bool wordCursor(int cursorPosition, QTextCursor &cursor,
                    SpellCheckSyntaxHighlighterUserData *&ud) const;

private:
    QColor m_textColor = QColor(0, 0, 0, 0);
    QColor m_backgroundColor = QColor(255, 0, 0, 32);

    int m_cursorPosition = -1;
    bool m_wordUnderCursorIsMisspelled = false;
    QStringList m_spellingSuggestionsForWordUnderCursor;
};

class TextLimiter;
class TextLimiterSyntaxHighlighterDelegate : public AbstractSyntaxHighlighterDelegate
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TextLimiterSyntaxHighlighterDelegate(QObject *parent = nullptr);
    ~TextLimiterSyntaxHighlighterDelegate();

    Q_PROPERTY(TextLimiter* textLimiter READ textLimiter WRITE setTextLimiter NOTIFY textLimiterChanged)
    void setTextLimiter(TextLimiter *val);
    TextLimiter *textLimiter() const { return m_textLimiter; }
    Q_SIGNAL void textLimiterChanged();

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(int cursorLimitPosition READ cursorLimitPosition NOTIFY cursorLimitPositionChanged)
    int cursorLimitPosition() const { return m_cursorLimitPosition; }
    Q_SIGNAL void cursorLimitPositionChanged();

protected:
    // AbstractSyntaxHighlighterDelegate interface
    void highlightBlock(const QString &text);
    void documentContentsChanged() { this->evaluateCursorLimitPosition(); }

private:
    void setCursorLimitPosition(int val);
    void evaluateCursorLimitPosition();

private:
    int m_cursorLimitPosition = -1;
    TextLimiter *m_textLimiter = nullptr;
    QColor m_backgroundColor = QColor(0, 0, 0, 0);
    QColor m_textColor = QColor(Qt::red);
};

#endif // SYNTAXHIGHLIGHTER_H
