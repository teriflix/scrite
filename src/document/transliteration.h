/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TRANSLITERATION_H
#define TRANSLITERATION_H

#include "execlatertimer.h"

#include <QMap>
#include <QFont>
#include <QEvent>
#include <QObject>
#include <QJsonArray>
#include <QQmlEngine>
#include <QStaticText>
#include <QJsonObject>
#include <QFontDatabase>
#include <QQuickPaintedItem>
#include <QSyntaxHighlighter>

#include "qobjectproperty.h"

class QTextCursor;
class QTextDocument;
class QCoreApplication;
class QQuickTextDocument;

class TransliterationEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static TransliterationEngine *instance(QCoreApplication *app = nullptr);
    ~TransliterationEngine();

    // Must be manually kept in sync with SceneElementFormat::DefaultLanguage &
    // SystemInputSource::Language
    enum Language {
        English,
        Bengali,
        Gujarati,
        Hindi,
        Kannada,
        Malayalam,
        Marathi,
        Oriya,
        Punjabi,
        Sanskrit,
        Tamil,
        Telugu
    };
    Q_ENUM(Language)
    Q_PROPERTY(Language language READ language WRITE setLanguage NOTIFY languageChanged)
    void setLanguage(Language val);
    Language language() const { return m_language; }
    Q_SIGNAL void languageChanged();

    Q_PROPERTY(QString languageAsString READ languageAsString NOTIFY languageChanged)
    QString languageAsString() const;
    static QString languageAsString(Language language);

    Q_PROPERTY(QJsonObject alphabetMappings READ alphabetMappings NOTIFY languageChanged)
    QJsonObject alphabetMappings() const { return this->alphabetMappingsFor(m_language); }

    Q_PROPERTY(QFont font READ font NOTIFY languageChanged)
    QFont font() const { return this->languageFont(m_language); }

    Q_INVOKABLE QString shortcutLetter(TransliterationEngine::Language val) const;

    Q_INVOKABLE QJsonObject alphabetMappingsFor(TransliterationEngine::Language val) const;

    Q_INVOKABLE void cycleLanguage();

    Q_INVOKABLE void markLanguage(TransliterationEngine::Language language, bool active);
    Q_INVOKABLE bool queryLanguage(TransliterationEngine::Language language) const;
    QMap<Language, bool> activeLanguages() const { return m_activeLanguages; }

    Q_PROPERTY(QJsonArray languages READ languages NOTIFY languagesChanged)
    QJsonArray languages() const;
    Q_SIGNAL void languagesChanged();

    Q_INVOKABLE QJsonArray getLanguages() const { return this->languages(); }

    void *transliterator() const;
    static void *transliteratorFor(TransliterationEngine::Language language);
    static Language languageOf(void *transliterator);

    Q_INVOKABLE void setTextInputSourceIdForLanguage(TransliterationEngine::Language language,
                                                     const QString &id);
    Q_INVOKABLE QString
    textInputSourceIdForLanguage(TransliterationEngine::Language language) const;

    Q_PROPERTY(QJsonObject languageTextInputSourceMap READ languageTextInputSourceMap NOTIFY languageTextInputSourceMapChanged)
    QJsonObject languageTextInputSourceMap() const;
    Q_SIGNAL void languageTextInputSourceMapChanged();

    Q_INVOKABLE QString transliteratedWord(const QString &word) const;
    Q_INVOKABLE QString transliteratedParagraph(const QString &paragraph,
                                                bool includingLastWord = true) const;
    Q_INVOKABLE QString transliteratedWordInLanguage(
            const QString &word, TransliterationEngine::Language language) const;

    static QString transliteratedWord(const QString &word, void *transliterator);
    static QString transliteratedParagraph(const QString &paragraph, void *transliterator,
                                           bool includingLastWord = true);

    Q_INVOKABLE QFont languageFont(TransliterationEngine::Language language) const
    {
        return this->languageFont(language, true);
    }
    QFont languageFont(TransliterationEngine::Language language, bool preferAppFonts) const;
    QStringList languageFontFilePaths(TransliterationEngine::Language language) const;

    Q_INVOKABLE QJsonObject
    availableLanguageFontFamilies(TransliterationEngine::Language language) const;
    Q_INVOKABLE QString preferredFontFamilyForLanguage(TransliterationEngine::Language language);
    Q_INVOKABLE void setPreferredFontFamilyForLanguage(TransliterationEngine::Language language,
                                                       const QString &fontFamily);
    Q_SIGNAL void preferredFontFamilyForLanguageChanged(TransliterationEngine::Language language,
                                                        const QString &fontFamily);

    static Language languageForScript(QChar::Script script);
    static QChar::Script scriptForLanguage(TransliterationEngine::Language language);
    static QFontDatabase::WritingSystem
    writingSystemForLanguage(TransliterationEngine::Language language);

    struct Boundary
    {
        int end = -1;
        int start = -1;
        QFont font;
        QString string;
        TransliterationEngine::Language language = TransliterationEngine::English;
        void evalStringLanguageAndFont(const QString &sourceText);
        void append(const QChar &ch, int pos);
        bool isEmpty() const;
    };
    QList<Boundary> evaluateBoundaries(const QString &text,
                                       bool bundleCommonScriptChars = false) const;
    void evaluateBoundariesAndInsertText(QTextCursor &cursor, const QString &text) const;

    static QChar::Script determineScript(const QString &val);

    Q_INVOKABLE QString formattedHtmlOf(const QString &text) const;

private:
    TransliterationEngine(QObject *parent = nullptr);

private:
    void *m_transliterator = nullptr;
    Language m_language = English;
    QMap<Language, QString> m_tisMap;
    QMap<Language, bool> m_activeLanguages;
    QMap<Language, int> m_languageBundledFontId;
    QMap<Language, QString> m_languageFontFamily;
    QMap<Language, QStringList> m_languageFontFilePaths;
    mutable QMap<Language, QStringList> m_availableLanguageFontFamilies;
};

class FontSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    FontSyntaxHighlighter(QObject *parent = nullptr);
    ~FontSyntaxHighlighter();

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text);
};

class Transliterator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(Transliterator)

public:
    ~Transliterator();

    static Transliterator *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged RESET resetTextDocument)
    void setTextDocument(QQuickTextDocument *val);
    QQuickTextDocument *textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(bool textDocumentUndoRedoEnabled READ isTextDocumentUndoRedoEnabled WRITE setTextDocumentUndoRedoEnabled NOTIFY textDocumentUndoRedoEnabledChanged)
    void setTextDocumentUndoRedoEnabled(bool val);
    bool isTextDocumentUndoRedoEnabled() const { return m_textDocumentUndoRedoEnabled; }
    Q_SIGNAL void textDocumentUndoRedoEnabledChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_PROPERTY(bool hasActiveFocus READ hasActiveFocus WRITE setHasActiveFocus NOTIFY hasActiveFocusChanged)
    void setHasActiveFocus(bool val);
    bool hasActiveFocus() const { return m_hasActiveFocus; }
    Q_SIGNAL void hasActiveFocusChanged();

    Q_PROPERTY(bool applyLanguageFonts READ isApplyLanguageFonts WRITE setApplyLanguageFonts NOTIFY applyLanguageFontsChanged)
    void setApplyLanguageFonts(bool val);
    bool isApplyLanguageFonts() const { return m_applyLanguageFonts; }
    Q_SIGNAL void applyLanguageFontsChanged();

    Q_PROPERTY(bool transliterateCurrentWordOnly READ isTransliterateCurrentWordOnly WRITE setTransliterateCurrentWordOnly NOTIFY transliterateCurrentWordOnlyChanged)
    void setTransliterateCurrentWordOnly(bool val);
    bool isTransliterateCurrentWordOnly() const { return m_transliterateCurrentWordOnly; }
    Q_SIGNAL void transliterateCurrentWordOnlyChanged();

    Q_INVOKABLE void enableFromNextWord() { m_enableFromNextWord = true; }

    enum Mode { AutomaticMode, SuggestionMode };
    Q_ENUM(Mode)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    void setMode(Mode val);
    Mode mode() const { return m_mode; }
    Q_SIGNAL void modeChanged();

    Q_INVOKABLE void transliterateLastWord();
    Q_INVOKABLE void transliterate(int from, int to);
    Q_INVOKABLE void transliterateToLanguage(int from, int to, int language);

    Q_SIGNAL void aboutToTransliterate(int from, int to, const QString &replacement,
                                       const QString &original);
    Q_SIGNAL void finishedTransliterating(int from, int to, const QString &replacement,
                                          const QString &original);

    Q_SIGNAL void transliterationSuggestion(int from, int to, const QString &replacement,
                                            const QString &original);

private:
    Transliterator(QObject *parent = nullptr);
    QTextDocument *document() const;
    void resetTextDocument();
    void processTransliteration(int from, int charsRemoved, int charsAdded);
    void transliterate(QTextCursor &cursor, void *transliterator = nullptr, bool force = false);

private:
    bool m_enabled = true;
    bool m_enableFromNextWord = false;
    Mode m_mode = AutomaticMode;
    int m_cursorPosition = -1;
    bool m_hasActiveFocus = false;
    bool m_applyLanguageFonts = false;
    bool m_transliterateCurrentWordOnly = true;
    bool m_textDocumentUndoRedoEnabled = false;
    QPointer<FontSyntaxHighlighter> m_fontHighlighter;
    QObjectProperty<QQuickTextDocument> m_textDocument;
};

class TransliterationEvent : public QEvent
{
public:
    static QEvent::Type EventType();

    TransliterationEvent(int start, int end, const QString &original,
                         TransliterationEngine::Language language, const QString &replacement);
    ~TransliterationEvent();

    int start() const { return m_start; }
    int end() const { return m_end; }
    QString original() const { return m_original; }
    QString replacement() const { return m_replacement; }
    TransliterationEngine::Language language() const { return m_language; }

private:
    int m_start = 0;
    int m_end = 0;
    QString m_original;
    QString m_replacement;
    TransliterationEngine::Language m_language = TransliterationEngine::English;
};

class TransliteratedText : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    TransliteratedText(QQuickItem *parent = nullptr);
    ~TransliteratedText();

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged();

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    void setFont(const QFont &val);
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_PROPERTY(qreal contentWidth READ contentWidth NOTIFY contentWidthChanged)

    qreal contentWidth() const { return m_contentWidth; }
    Q_SIGNAL void contentWidthChanged();

    Q_PROPERTY(qreal contentHeight READ contentHeight NOTIFY contentHeightChanged)
    qreal contentHeight() const { return m_contentHeight; }
    Q_SIGNAL void contentHeightChanged();

    // QQuickPaintedItem interface
    void paint(QPainter *painter);

private:
    void timerEvent(QTimerEvent *te);
    void updateStaticText();
    void updateStaticTextLater();

    void setContentWidth(qreal val);
    void setContentHeight(qreal val);

private:
    QFont m_font;
    QColor m_color;
    QString m_text;
    qreal m_contentWidth = 0;
    qreal m_contentHeight = 0;
    QStaticText m_staticText;
    ExecLaterTimer m_updateTimer;
};

#endif // TRANSLITERATION_H
