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
#include <QKeySequence>
#include <QAbstractListModel>

class LanguageEngine;
class AbstractTransliterationEngine;

struct TransliterationOption
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_PROPERTY(QObject* transliterator MEMBER transliteratorObject)
    QPointer<QObject> transliteratorObject; // must be of type AbstractTransliterator

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
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_PROPERTY(int code MEMBER code)
    int code = -1;

    Q_PROPERTY(QString name READ name)
    QString name() const;

    Q_PROPERTY(QKeySequence keySequence MEMBER keySequence)
    QKeySequence keySequence;

    Q_PROPERTY(QString shortcut READ shortcut)
    QString shortcut() const { return keySequence.toString(); }

    Q_PROPERTY(QFont font MEMBER font)
    QFont font;

    Q_PROPERTY(QFont defaultFont READ defaultFont)
    QFont defaultFont() const;

    Q_PROPERTY(QStringList aptFontFamilies READ aptFontFamilies)
    QStringList aptFontFamilies() const;

    Q_PROPERTY(int localeScript READ localeScript)
    int localeScript() const; // Returns QLocale::Script

    Q_PROPERTY(int charScript READ charScript)
    int charScript() const; // Returns QChar::Script

    Q_PROPERTY(int fontWritingSystem READ fontWritingSystem)
    int fontWritingSystem() const; // Returns QFontDatabase::WritingSystem

    Q_PROPERTY(QString preferredTransliterationOptionId MEMBER preferredTransliterationOptionId)
    QString preferredTransliterationOptionId;

    Q_PROPERTY(QList<TransliterationOption> transliterationOptions READ transliterationOptions)
    QList<TransliterationOption> transliterationOptions() const;

    Q_PROPERTY(TransliterationOption preferredTransliterationOption READ preferredTransliterationOption)
    TransliterationOption preferredTransliterationOption() const;

    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const { return code >= 0; }

    Language &operator=(const Language &other)
    {
        this->code = other.code;
        this->font = other.font;
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

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_languages.size(); }
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

private:
    QList<Language> m_languages;
};

class SupportedLanguages : public AbstractLanguagesModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~SupportedLanguages();

    Q_INVOKABLE void addLanguage(int code);
    Q_INVOKABLE void removeLanguage(int code);
    Q_INVOKABLE void updateLanguage(int code);

    Q_INVOKABLE bool assignLanguageFont(int code, const QFont &font);
    Q_INVOKABLE bool assignLanguageShortcut(int code, const QString &nativeSequence);

signals:
    void languageFontChanged(int code);
    void languageShortcutChanged(int code);

private:
    explicit SupportedLanguages(QObject *parent = nullptr);

    void loadLanguages();
    void saveLanguages();
    void loadBuiltInLanguages();
    void transliterationOptionsUpdated();

private:
    friend class LanguageEngine;
    QString m_settingsFileName;
};

class AvailableLanguages : public AbstractLanguagesModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static AvailableLanguages *instance();
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
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

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
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

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

    Q_INVOKABLE static QString onWord(const QString &word, int code);
    Q_INVOKABLE static AlphabetMappings alphabetMappingsFor(int languageCode);
};

class AbstractTransliterationEngine : public QObject
{
    Q_OBJECT

public:
    explicit AbstractTransliterationEngine(QObject *parent = nullptr);
    ~AbstractTransliterationEngine();

    /* Unique name among all transliterators supported */
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
    QString transliterateWord(const QString &word, const TransliterationOption &lang) const;
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
This class ties all the pieces together.
*/
class LanguageEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static LanguageEngine *instance();
    ~LanguageEngine();

    Q_PROPERTY(AvailableLanguages* availableLanguages READ availableLanguages CONSTANT)
    AvailableLanguages *availableLanguages() const { return m_availableLanguages; }

    Q_PROPERTY(SupportedLanguages* supportedLanguages READ supportedLanguages CONSTANT)
    SupportedLanguages *supportedLanguages() const { return m_supportedLanguages; }

    Q_INVOKABLE QFont defaultLanguageFont(int languageCode) const;
    Q_INVOKABLE QStringList aptFontFamilies(int languageCode) const;

    QList<TransliterationOption> queryTransliterationOptions(int language) const;

    static void init(const char *uri, QQmlEngine *qmlEngine);

signals:
    void transliterationOptionsUpdated();

private:
    explicit LanguageEngine(QObject *parent = nullptr);

private:
    QMap<int, QFont> m_defaultLanguageFont;
    AvailableLanguages *m_availableLanguages = nullptr;
    SupportedLanguages *m_supportedLanguages = nullptr;
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
