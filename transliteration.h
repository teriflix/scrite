/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
#include <QObject>
#include <QJsonArray>
#include <QQmlEngine>

class QTextCursor;
class QTextDocument;
class QCoreApplication;
class QQuickTextDocument;

class TransliterationSettings : public QObject
{
    Q_OBJECT

public:
    static TransliterationSettings *instance(QCoreApplication *app=nullptr);
    ~TransliterationSettings();

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

    Q_PROPERTY(QJsonArray languages READ languages NOTIFY languagesChanged)
    QJsonArray languages() const;
    Q_SIGNAL void languagesChanged();

    Q_INVOKABLE QJsonArray getLanguages() const { return this->languages(); }

    void *transliterator() const { return m_transliterator; }
    Q_INVOKABLE QString transliteratedWord(const QString &word) const;

private:
    TransliterationSettings(QObject *parent=nullptr);

private:
    void *m_transliterator;
    Language m_language;
    QMap<Language,bool> m_activeLanguages;
};

class Transliterator : public QObject
{
    Q_OBJECT

public:
    ~Transliterator();

    static Transliterator *qmlAttachedProperties(QObject *object);

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

    Q_SIGNAL void transliterationSuggestion(int from, int to, const QString &replacement, const QString &original);

private:
    Transliterator(QObject *parent=nullptr);
    QTextDocument *document() const;
    void onTextDocumentDestroyed();
    void processTransliteration(int from, int charsRemoved, int charsAdded);
    void transliterate(QTextCursor &cursor);

private:
    Mode m_mode;
    int m_cursorPosition;
    bool m_hasActiveFocus;
    QQuickTextDocument* m_textDocument;
};
Q_DECLARE_METATYPE(Transliterator*)
QML_DECLARE_TYPEINFO(Transliterator, QML_HAS_ATTACHED_PROPERTIES)

#endif // TRANSLITERATION_H
