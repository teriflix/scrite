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

#ifndef LANGUAGEENGINE_H
#define LANGUAGEENGINE_H

#include <QFont>
#include <QObject>
#include <QQmlEngine>
#include <QTextLayout>
#include <QKeySequence>
#include <QFontDatabase>
#include <QAbstractListModel>

class QTextCursor;
class LanguageEngine;
class AbstractTransliterationEngine;

struct TransliterationOption
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_PROPERTY(QObject* transliterator MEMBER transliteratorObject)
    QPointer<QObject> transliteratorObject; // must be of type AbstractTransliterationEngine

    Q_PROPERTY(int languageCode MEMBER languageCode)
    int languageCode = -1;

    Q_PROPERTY(QString id MEMBER id)
    QString id;

    Q_PROPERTY(QString name MEMBER name)
    QString name;

    // This should be set to true if the app needs to show a popup to display
    // transliteration, and if transliterateWord() will accept an English text
    // to return transliterated text in another language.
    // This should be set to false, unless there is a transliteration engine that's
    // statically linked to the Scrite code, which is the case for platform/OS
    // input methods.
    Q_PROPERTY(bool inApp MEMBER inApp)
    bool inApp = false;

    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const;

    Q_INVOKABLE void activate();
    Q_INVOKABLE QString transliterateWord(const QString &word) const;

    AbstractTransliterationEngine *transliterator() const;

    TransliterationOption &operator=(const TransliterationOption &other)
    {
        this->transliteratorObject = other.transliteratorObject;
        this->languageCode = other.languageCode;
        this->id = other.id;
        this->name = other.name;
        this->inApp = other.inApp;
        return *this;
    }
    bool operator==(const TransliterationOption &other) const
    {
        return languageCode == other.languageCode && id == other.id && name == other.name
                && inApp == other.inApp;
    }
    bool operator!=(const TransliterationOption &other) const { return !(*this == other); }
};
Q_DECLARE_METATYPE(TransliterationOption)
Q_DECLARE_METATYPE(QList<TransliterationOption>)

struct Language
{
    Q_GADGET

public:
    Q_PROPERTY(int code MEMBER code)
    int code = -1;

    Q_PROPERTY(QString name READ name)
    QString name() const;

    Q_PROPERTY(QString nativeName READ nativeName)
    QString nativeName() const;

    Q_PROPERTY(QKeySequence keySequence MEMBER keySequence)
    QKeySequence keySequence;

    Q_PROPERTY(QString preferredTransliterationOptionId MEMBER preferredTransliterationOptionId)
    QString preferredTransliterationOptionId;

    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const { return code >= 0; }

    Q_INVOKABLE int localeScript() const; // Returns QLocale::Script
    Q_INVOKABLE int charScript() const; // Returns QChar::Script
    Q_INVOKABLE int fontWritingSystem() const; // Returns QFontDatabase::WritingSystem
    Q_INVOKABLE QFont font() const;
    Q_INVOKABLE QString shortcut() const { return keySequence.toString(); }
    Q_INVOKABLE QStringList fontFamilies() const;
    Q_INVOKABLE TransliterationOption preferredTransliterationOption() const;
    Q_INVOKABLE QList<TransliterationOption> transliterationOptions() const;

    Language &operator=(const Language &other)
    {
        this->code = other.code;
        this->keySequence = other.keySequence;
        this->preferredTransliterationOptionId = other.preferredTransliterationOptionId;
        return *this;
    }
    bool operator==(const Language &other) const { return code == other.code; }
    bool operator!=(const Language &other) const { return !(*this == other); }
};
Q_DECLARE_METATYPE(Language)

class AbstractLanguagesModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~AbstractLanguagesModel();

    Q_INVOKABLE int indexOfLanguage(int code) const;
    Q_INVOKABLE bool hasLanguage(int code) const; // here code should be from QLocale::Language
    Q_INVOKABLE Language findLanguage(int code) const;
    Q_INVOKABLE Language languageAt(int index) const;

    Q_PROPERTY(int count READ count NOTIFY countChanged) int count() const
    {
        return m_languages.size();
    }
    Q_SIGNAL void countChanged();

    enum { LanguageRole = Qt::UserRole };
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

protected:
    explicit AbstractLanguagesModel(QObject *parent = nullptr);

    void addLanguage(const Language &language);
    void removeLanguage(const Language &language);
    void removeLanguageAt(int row);
    void updateLanguage(const Language &language);
    void setLanguages(const QList<Language> &languages);
    const QList<Language> &languages() const { return m_languages; }

    virtual void initialize();
    virtual QJsonValue toJson() const;
    virtual void fromJson(const QJsonValue &value);

private:
    friend class LanguageEngine;

    QList<Language> m_languages;
};

class SupportedLanguages : public AbstractLanguagesModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~SupportedLanguages();

    Q_PROPERTY(int activeLanguageCode READ activeLanguageCode WRITE setActiveLanguageCode NOTIFY activeLanguageCodeChanged)
    void setActiveLanguageCode(int val);
    int activeLanguageCode() const { return m_activeLanguageCode; }
    Q_SIGNAL void activeLanguageCodeChanged();

    Q_PROPERTY(Language activeLanguage READ activeLanguage NOTIFY activeLanguageCodeChanged)
    Language activeLanguage() const;

    Q_PROPERTY(int activeLanguageRow READ activeLanguageRow NOTIFY activeLanguageRowChanged)
    int activeLanguageRow() const;
    Q_SIGNAL void activeLanguageRowChanged();

    Q_INVOKABLE void addLanguage(int code);
    Q_INVOKABLE void removeLanguage(int code);
    Q_INVOKABLE void updateLanguage(int code);

    Q_INVOKABLE bool assignLanguageFontFamily(int code, const QString &fontFamily); // Helper
    Q_INVOKABLE bool assignLanguageShortcut(int code, const QString &nativeSequence);

signals:
    void languageShortcutChanged(int code);

private:
    explicit SupportedLanguages(QObject *parent = nullptr);

    void loadBuiltInLanguages();
    void transliterationOptionsUpdated();
    void onScriptFontFamilyChanged(QChar::Script script, const QString &fontFamily);

    void initialize();
    QJsonValue toJson() const;
    void fromJson(const QJsonValue &value);

    void onDataChanged(const QModelIndex &start, const QModelIndex &end);
    void verifyActiveLanguage();

private:
    friend class LanguageEngine;
    int m_activeLanguageCode = -1;
};

class AvailableLanguages : public AbstractLanguagesModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~AvailableLanguages();

private:
    explicit AvailableLanguages(QObject *parent = nullptr);

    void initialize();

    friend class LanguageEngine;
};

/*
Helpers for using the static transliterator only.
*/
struct AlphabetMapping
{
    Q_GADGET

public:
    Q_PROPERTY(QString latin MEMBER latin)
    QString latin;

    Q_PROPERTY(QString unicode MEMBER unicode)
    QString unicode;

    bool operator==(const AlphabetMapping &other) const
    {
        return latin == other.latin && unicode == other.unicode;
    }
    bool operator!=(const AlphabetMapping &other) const { return !(*this == other); }
};
Q_DECLARE_METATYPE(AlphabetMapping)
Q_DECLARE_METATYPE(QList<AlphabetMapping>)

struct AlphabetMappings
{
    Q_GADGET

public:
    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const
    {
        return language.isValid() && !consonants.isEmpty() && !digits.isEmpty()
                && !symbols.isEmpty() && !vowels.isEmpty();
    }

    Q_PROPERTY(Language language MEMBER language)
    Language language;

    Q_PROPERTY(QList<AlphabetMapping> consonants MEMBER consonants)
    QList<AlphabetMapping> consonants;

    Q_PROPERTY(QList<AlphabetMapping> digits MEMBER digits)
    QList<AlphabetMapping> digits;

    Q_PROPERTY(QList<AlphabetMapping> symbols MEMBER symbols)
    QList<AlphabetMapping> symbols;

    Q_PROPERTY(QList<AlphabetMapping> vowels MEMBER vowels)
    QList<AlphabetMapping> vowels;
};
Q_DECLARE_METATYPE(AlphabetMappings)

class DefaultTransliteration : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit DefaultTransliteration(QObject *parent = nullptr);
    ~DefaultTransliteration();

    Q_PROPERTY(QString driver READ driver CONSTANT)
    static QString driver();

    Q_PROPERTY(QList<int> supportedLanguageCodes READ supportedLanguageCodes CONSTANT)
    static QList<int> supportedLanguageCodes();

    Q_INVOKABLE bool supportsLanguageCode(int code) const;

    Q_INVOKABLE static QString onWord(const QString &word, int code);
    Q_INVOKABLE static QString onParagraph(const QString &paragraph, int code);

    Q_INVOKABLE static AlphabetMappings alphabetMappingsFor(int languageCode);

    static QString shortcut(int languageCode);
};

class AbstractTransliterationEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit AbstractTransliterationEngine(QObject *parent = nullptr);
    ~AbstractTransliterationEngine();

    /* Unique name among all transliterators supported */
    Q_PROPERTY(QString name READ name CONSTANT)
    virtual QString name() const = 0;

    /** should return true if it supports transliteration to the said language */
    virtual bool canTransliterate(int lang) const { return !this->options(lang).isEmpty(); }

    /** should return a list of transliteration options */
    virtual QList<TransliterationOption> options(int lang) const = 0;

    /** called to verify if an option is still available */
    virtual bool canActivate(const TransliterationOption &option) = 0;

    /** called as soon as editor receives focus **/
    virtual void activate(const TransliterationOption &option) = 0;

    /** called as soon as editor loses focus **/
    virtual void release(const TransliterationOption &option) = 0;

    /** caled to fetch the transliterated word for the said language **/
    virtual QString transliterateWord(const QString &word,
                                      const TransliterationOption &lang) const = 0;

signals:
    /** Implementations must emit this signal when their capacity to support
        a one or more languages has changed at run time. */
    void capacityChanged();
};

/*
This is a fallback engine, that does nothing -- which means basically any keystroke
is passed through without transliteration.
*/
class FallbackTransliterationEngine : public AbstractTransliterationEngine
{
    Q_OBJECT

public:
    explicit FallbackTransliterationEngine(QObject *parent = nullptr);
    ~FallbackTransliterationEngine();

    // AbstractTransliterator interface
    QString name() const;
    QList<TransliterationOption> options(int lang) const;
    bool canActivate(const TransliterationOption &option);
    void activate(const TransliterationOption &option);
    void release(const TransliterationOption &option);
    QString transliterateWord(const QString &word, const TransliterationOption &option) const;
};

/*
The static transliterator is built on top of a third-party tool called PhTranslator.
It only supports a handful of Indian languages.
*/
class StaticTransliterationEngine : public AbstractTransliterationEngine
{
    Q_OBJECT

public:
    explicit StaticTransliterationEngine(QObject *parent = nullptr);
    ~StaticTransliterationEngine();

    // AbstractTransliterator interface
    QString name() const;
    QList<TransliterationOption> options(int lang) const;
    bool canActivate(const TransliterationOption &option);
    void activate(const TransliterationOption &option);
    void release(const TransliterationOption &option);
    QString transliterateWord(const QString &word, const TransliterationOption &option) const;
};

/*
This class is implemented for each operating system separately, to pull in support for
transliteration from the OS.
*/
class PlatformTransliterationEngine : public AbstractTransliterationEngine
{
    Q_OBJECT

public:
    explicit PlatformTransliterationEngine(QObject *parent = nullptr);
    ~PlatformTransliterationEngine();

    // AbstractTransliterator interface
    QString name() const;
    QList<TransliterationOption> options(int lang) const;
    bool canActivate(const TransliterationOption &option);
    void activate(const TransliterationOption &option);
    void release(const TransliterationOption &option);
    QString transliterateWord(const QString &word, const TransliterationOption &option) const;
};

/*
Process QInputMethod events for an editor, and routes them through the any InApp transliteration
option supplied to the 'option' property.
 */
class LanguageTransliterator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(LanguageTransliterator)
    QML_UNCREATABLE("Use as attached property.")

public:
    ~LanguageTransliterator();

    static LanguageTransliterator *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(TransliterationOption option READ option WRITE setOption NOTIFY optionChanged)
    void setOption(const TransliterationOption &val);
    TransliterationOption option() const { return m_option; }
    Q_SIGNAL void optionChanged();

    Q_PROPERTY(QObject* popup READ popup WRITE setPopup NOTIFY popupChanged)
    void setPopup(QObject *val);
    QObject *popup() const { return m_popup; }
    Q_SIGNAL void popupChanged();

    Q_PROPERTY(QString currentWord READ currentWord NOTIFY currentWordChanged)
    QString currentWord() const { return m_currentWord.originalString; }
    Q_SIGNAL void currentWordChanged();

    Q_PROPERTY(QString commitString READ commitString NOTIFY commitStringChanged)
    QString commitString() const { return m_currentWord.commitString; }
    Q_SIGNAL void commitStringChanged();

    Q_PROPERTY(QRect textRect READ textRect NOTIFY textRectChanged)
    QRect textRect() const { return m_currentWord.textRect; }
    Q_SIGNAL void textRectChanged() const;

    Q_PROPERTY(QObject* editor READ editor CONSTANT)
    QObject *editor() const { return m_editor; }

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    LanguageTransliterator(QObject *parent = nullptr);
    bool updateWordFromInput(const QKeyEvent *keyEvent);
    bool commitWordToEditor();
    void resetCurrentWord();

private:
    struct Word
    {
        int start = -1;
        int end = -1;
        QRect textRect;
        QString commitString;
        QString originalString;
    } m_currentWord;

    bool m_enabled = false;
    QObject *m_editor = nullptr;
    QObject *m_popup = nullptr;
    TransliterationOption m_option;
};

/*
Code to evaluate language boundaries and perform operations on them.
 */
struct ScriptBoundary
{
    Q_GADGET

public:
    Q_PROPERTY(int start MEMBER start)
    int start = -1;

    Q_PROPERTY(int end MEMBER end)
    int end = -1;

    Q_PROPERTY(QString text MEMBER text)
    QString text;

    Q_PROPERTY(QChar::Script script MEMBER script)
    QChar::Script script = QChar::Script_Unknown;

    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const { return start >= 0 && end > 0 && end - start > 1 && !text.isEmpty(); }
};
Q_DECLARE_METATYPE(ScriptBoundary)
Q_DECLARE_METATYPE(QList<ScriptBoundary>)

/*
This class ties all the pieces together.
*/
class LanguageEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static LanguageEngine *instance();
    ~LanguageEngine();

    Q_PROPERTY(AvailableLanguages* availableLanguages READ availableLanguages CONSTANT)
    AvailableLanguages *availableLanguages() const { return m_availableLanguages; }

    Q_PROPERTY(SupportedLanguages* supportedLanguages READ supportedLanguages CONSTANT)
    SupportedLanguages *supportedLanguages() const { return m_supportedLanguages; }

    Q_INVOKABLE bool setScriptFontFamily(QChar::Script script, const QString &fontFamily);
    Q_INVOKABLE QString scriptFontFamily(QChar::Script script) const;

    Q_INVOKABLE QStringList scriptFontFamilies(QChar::Script script) const;

    QList<TransliterationOption> queryTransliterationOptions(int language) const;

    static QList<ScriptBoundary> determineBoundaries(const QString &paragraph);
    static void determineBoundariesAndInsertText(QTextCursor &cursor, const QString &paragraph);
    static QString formattedInHtml(const QString &paragraph);
    static int wordCount(const QString &paragraph);
    static QVector<QTextLayout::FormatRange>
    mergeTextFormats(const QList<ScriptBoundary> &boundaries,
                     const QVector<QTextLayout::FormatRange> &formats);

    static void init(const char *uri, QQmlEngine *qmlEngine);

signals:
    void scriptFontFamilyChanged(QChar::Script script, const QString &fontFamily);
    void transliterationOptionsUpdated();

private:
    explicit LanguageEngine(QObject *parent = nullptr);

    void loadConfiguration();
    void saveConfiguration();

private:
    QString m_configFileName;
    AvailableLanguages *m_availableLanguages = nullptr;
    SupportedLanguages *m_supportedLanguages = nullptr;
    QMap<QChar::Script, QString> m_defaultScriptFontFamily, m_scriptFontFamily;
    QList<AbstractTransliterationEngine *> m_transliterators;
};

// This is done purely for the sake of exposing QChar enums to QML
class QtChar : public QChar
{
    Q_GADGET

public:
    Q_ENUM(SpecialCharacter)
    Q_ENUM(Category)
    Q_ENUM(Script)
};

#endif // LANGUAGEENGINE_H
