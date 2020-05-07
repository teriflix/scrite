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

#include <QMap>
#include <QFont>
#include <QObject>
#include <QJsonArray>
#include <QQmlEngine>

class QTextCursor;
class QTextDocument;
class QCoreApplication;
class QQuickTextDocument;

class TransliterationEngine : public QObject
{
    Q_OBJECT

public:
    static TransliterationEngine *instance(QCoreApplication *app=nullptr);
    ~TransliterationEngine();

    enum Language
    {
        English,
        Bengali,
        Gujarati,
        Hindi,
        Kannada,
        Malayalam,
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

    Q_INVOKABLE QString shortcutLetter(Language val);

    Q_INVOKABLE void cycleLanguage();

    Q_INVOKABLE void markLanguage(Language language, bool active);
    Q_INVOKABLE bool queryLanguage(Language language) const;
    QMap<Language,bool> activeLanguages() const { return m_activeLanguages; }

    Q_PROPERTY(QJsonArray languages READ languages NOTIFY languagesChanged)
    QJsonArray languages() const;
    Q_SIGNAL void languagesChanged();

    Q_INVOKABLE QJsonArray getLanguages() const { return this->languages(); }

    void *transliterator() const { return m_transliterator; }
    void *transliteratorFor(Language language) const;
    Language languageOf(void *transliterator) const;

    Q_INVOKABLE QString transliteratedWord(const QString &word) const;
    Q_INVOKABLE QString transliteratedSentence(const QString &sentence, bool includingLastWord=true) const;

    QFont languageFont(Language language) const;
    QStringList languageFontFilePaths(Language language) const;
    static Language languageForScript(QChar::Script script);

    struct Boundary
    {
        int end = -1;
        int start = -1;
        QFont font;
        QString string;
        TransliterationEngine::Language language = TransliterationEngine::English;

        void append(const QChar &ch, int pos) {
            string.append(ch);
            if(start < 0)
                start = pos;
            end = pos+1;
        }

        bool isEmpty() const {
            return end < 0 || start < 0 || start == end;
        }
    };
    QList<Boundary> evaluateBoundaries(const QString &text) const;
    void evaluateBoundariesAndInsertText(QTextCursor &cursor, const QString &text) const;

private:
    TransliterationEngine(QObject *parent=nullptr);

private:
    void *m_transliterator = nullptr;
    Language m_language = English;
    QMap<Language,bool> m_activeLanguages;
    QMap<Language,int> m_languageFontIdMap;
    QMap<Language,QStringList> m_languageFontFilePaths;
};

class Transliterator : public QObject
{
    Q_OBJECT

public:
    ~Transliterator();

    static Transliterator *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged)
    void setTextDocument(QQuickTextDocument* val);
    QQuickTextDocument* textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_PROPERTY(bool hasActiveFocus READ hasActiveFocus WRITE setHasActiveFocus NOTIFY hasActiveFocusChanged)
    void setHasActiveFocus(bool val);
    bool hasActiveFocus() const { return m_hasActiveFocus; }
    Q_SIGNAL void hasActiveFocusChanged();

    Q_INVOKABLE void enableFromNextWord() { m_enableFromNextWord = true; }

    enum Mode
    {
        AutomaticMode,
        SuggestionMode
    };
    Q_ENUM(Mode)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    void setMode(Mode val);
    Mode mode() const { return m_mode; }
    Q_SIGNAL void modeChanged();

    Q_INVOKABLE void transliterateLastWord();
    Q_INVOKABLE void transliterate(int from, int to);
    Q_INVOKABLE void transliterateToLanguage(int from, int to, int language);

    Q_SIGNAL void aboutToTransliterate(int from, int to, const QString &replacement, const QString &original);
    Q_SIGNAL void finishedTransliterating(int from, int to, const QString &replacement, const QString &original);

    Q_SIGNAL void transliterationSuggestion(int from, int to, const QString &replacement, const QString &original);

private:
    Transliterator(QObject *parent=nullptr);
    QTextDocument *document() const;
    void onTextDocumentDestroyed();
    void processTransliteration(int from, int charsRemoved, int charsAdded);
    void transliterate(QTextCursor &cursor, void *transliterator=nullptr);

private:
    bool m_enabled = true;
    bool m_enableFromNextWord = false;
    Mode m_mode = AutomaticMode;
    int m_cursorPosition = -1;
    bool m_hasActiveFocus = false;
    QQuickTextDocument* m_textDocument = nullptr;
};
Q_DECLARE_METATYPE(Transliterator*)
QML_DECLARE_TYPEINFO(Transliterator, QML_HAS_ATTACHED_PROPERTIES)

#endif // TRANSLITERATION_H
