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
#include <QQuickImageProvider>
#include <QQuickItem>

class QTextCursor;
class LanguageEngine;
class AbstractTransliterationEngine;

struct TransliterationOption
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(QObject *transliterator
               MEMBER transliteratorObject)
    // clang-format on
    QPointer<QObject> transliteratorObject; // must be of type AbstractTransliterationEngine

    // clang-format off
    Q_PROPERTY(int languageCode
               MEMBER languageCode)
    // clang-format on
    int languageCode = -1;

    // clang-format off
    Q_PROPERTY(QString id
               MEMBER id)
    // clang-format on
    QString id;

    // clang-format off
    Q_PROPERTY(QString name
               MEMBER name)
    // clang-format on
    QString name;

    // This should be set to true if the app needs to show a popup to display
    // transliteration, and if transliterateWord() will accept an English text
    // to return transliterated text in another language.
    // This should be set to false, unless there is a transliteration engine that's
    // statically linked to the Scrite code, which is the case for platform/OS
    // input methods.
    // clang-format off
    Q_PROPERTY(bool inApp
               MEMBER inApp)
    // clang-format on
    bool inApp = false;

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const;

    Q_INVOKABLE bool activate();
    Q_INVOKABLE QString transliterateWord(const QString &word) const;
    Q_INVOKABLE QString transliterateParagraph(const QString &paragraph) const;

    AbstractTransliterationEngine *transliterator() const;

    TransliterationOption() { }
    TransliterationOption(QObject *_transliteratorObject, int _languageCode, const QString &_id,
                          const QString &_name, bool _inApp)
        : transliteratorObject(_transliteratorObject),
          languageCode(_languageCode),
          id(_id),
          name(_name),
          inApp(_inApp)
    {
    }
    TransliterationOption(const TransliterationOption &other) { *this = other; }
    ~TransliterationOption() { }
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
    // clang-format off
    Q_PROPERTY(int code
               MEMBER code)
    // clang-format on
    int code = -1;

    // clang-format off
    Q_PROPERTY(QString name
               READ name)
    // clang-format on
    QString name() const;

    // clang-format off
    Q_PROPERTY(QString nativeName
               READ nativeName)
    // clang-format on
    QString nativeName() const;

    // clang-format off
    Q_PROPERTY(QString shortName
               READ shortName)
    // clang-format on
    QString shortName() const;

    // clang-format off
    Q_PROPERTY(QString glyph
               READ glyph)
    // clang-format on
    QString glyph() const;

    // clang-format off
    Q_PROPERTY(QUrl iconSource
               READ iconSource)
    // clang-format on
    QUrl iconSource() const;

    // clang-format off
    Q_PROPERTY(QKeySequence keySequence
               MEMBER keySequence)
    // clang-format on
    QKeySequence keySequence;

    // clang-format off
    Q_PROPERTY(QString preferredTransliterationOptionId
               MEMBER preferredTransliterationOptionId)
    // clang-format on
    QString preferredTransliterationOptionId;

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const { return code >= 0; }

    Q_INVOKABLE int charScript() const; // Returns QtChar::Script
    Q_INVOKABLE int localeScript() const; // Returns QLocale::Script
    Q_INVOKABLE int fontWritingSystem() const; // Returns QFontDatabase::WritingSystem
    Q_INVOKABLE bool activate();
    Q_INVOKABLE QString charScriptName() const;
    Q_INVOKABLE QString localeScriptName() const;
    Q_INVOKABLE QString fontWritingSystemName() const;
    Q_INVOKABLE QFont font() const;
    Q_INVOKABLE QString shortcut() const { return keySequence.toString(); }
    Q_INVOKABLE QStringList fontFamilies() const;
    Q_INVOKABLE TransliterationOption preferredTransliterationOption() const;
    Q_INVOKABLE QList<TransliterationOption> transliterationOptions() const;

    Language() { }
    Language(int _code, const QKeySequence &_keySequence,
             const QString &_preferredTransliterationOptionId)
        : code(_code),
          keySequence(_keySequence),
          preferredTransliterationOptionId(_preferredTransliterationOptionId)
    {
    }
    Language(const Language &other) { *this = other; }
    ~Language() { }
    Language &operator=(const Language &other)
    {
        this->code = other.code;
        this->keySequence = other.keySequence;
        this->preferredTransliterationOptionId = other.preferredTransliterationOptionId;
        return *this;
    }
    bool operator==(const Language &other) const { return code == other.code; }
    bool operator!=(const Language &other) const { return !(*this == other); }

    static QChar::Script scriptForLanguage(QLocale::Language language,
                                           QChar::Script defaultScript = QChar::Script_Latin);
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

    // clang-format off
    Q_PROPERTY(QList<int> languageCodes
               READ languageCodes
               NOTIFY languagesCodesChanged)
    // clang-format on
    QList<int> languageCodes() const { return m_languageCodes; }

    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    // clang-format on
    int count() const { return m_languages.size(); }
    Q_SIGNAL void countChanged();

    enum {
        LanguageRole = Qt::UserRole,
        CodeRole,
        NameRole,
        NativeNameRole,
        KeySequenceRole,
        PreferredTransliterationOptionIdRole
    };
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

signals:
    void languagesCodesChanged(const QList<int> &languageCodes);

protected:
    explicit AbstractLanguagesModel(QObject *parent = nullptr);

    int addLanguage(const Language &language);
    int removeLanguage(const Language &language);
    int removeLanguageAt(int row);
    int updateLanguage(const Language &language);
    void setLanguages(const QList<Language> &languages);
    const QList<Language> &languages() const { return m_languages; }

    virtual void initialize();
    virtual QJsonValue toJson() const;
    virtual void fromJson(const QJsonValue &value);

private:
    void updateLanguageCodes();

    friend class LanguageEngine;

    QList<Language> m_languages;
    QList<int> m_languageCodes; // each int is a QLocale::Language value
};

class SupportedLanguages : public AbstractLanguagesModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~SupportedLanguages();

    // clang-format off
    Q_PROPERTY(int activeLanguageCode
               READ activeLanguageCode
               WRITE setActiveLanguageCode
               NOTIFY activeLanguageCodeChanged)
    // clang-format on
    void setActiveLanguageCode(int val);
    int activeLanguageCode() const { return m_activeLanguageCode; }
    Q_SIGNAL void activeLanguageCodeChanged();

    // clang-format off
    Q_PROPERTY(int defaultLanguageCode
               READ defaultLanguageCode
               WRITE setDefaultLanguageCode
               NOTIFY defaultLanguageCodeChanged)
    // clang-format on
    void setDefaultLanguageCode(int val);
    int defaultLanguageCode() const { return m_defaultLanguageCode; }
    Q_SIGNAL void defaultLanguageCodeChanged();

    // clang-format off
    Q_PROPERTY(Language activeLanguage
               READ activeLanguage
               NOTIFY activeLanguageCodeChanged)
    // clang-format on
    Language activeLanguage() const;

    // clang-format off
    Q_PROPERTY(Language defaultLanguage
               READ defaultLanguage
               NOTIFY defaultLanguageCodeChanged)
    // clang-format on
    Language defaultLanguage() const;

    // clang-format off
    Q_PROPERTY(int activeLanguageRow
               READ activeLanguageRow
               NOTIFY activeLanguageRowChanged)
    // clang-format on
    int activeLanguageRow() const;
    Q_SIGNAL void activeLanguageRowChanged();

    Q_INVOKABLE int addLanguage(int code);
    Q_INVOKABLE int removeLanguage(int code);
    Q_INVOKABLE int updateLanguage(int code);

    Q_INVOKABLE bool assignLanguageShortcut(int code, const QString &nativeSequence);
    Q_INVOKABLE bool assignLanguageFontFamily(int code, const QString &fontFamily); // Helper
    Q_INVOKABLE bool useLanguageTransliterator(int code, const TransliterationOption &option);
    Q_INVOKABLE bool useLanguageTransliteratorId(int code, const QString &id);
    Q_INVOKABLE bool resetLanguageTranslator(int code);

signals:
    void languageShortcutChanged(int code);
    void languageTransliteratorChanged(int code);

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    explicit SupportedLanguages(QObject *parent = nullptr);

    void loadBuiltInLanguages();
    void reviewLoadBuiltInLanguages();
    void transliterationOptionsUpdated();
    void onScriptFontFamilyChanged(QChar::Script script, const QString &fontFamily);

    void initialize();
    QJsonValue toJson() const;
    void fromJson(const QJsonValue &value);

    void onDataChanged(const QModelIndex &start, const QModelIndex &end);
    void verifyActiveLanguage();
    void ensureActiveLanguage();

private:
    friend class LanguageEngine;
    int m_activeLanguageCode = -1;
    int m_defaultLanguageCode = -1;
    bool m_loadedBuiltInLanguages = false;
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
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(QString latin
               MEMBER latin)
    // clang-format on
    QString latin;

    // clang-format off
    Q_PROPERTY(QString unicode
               MEMBER unicode)
    // clang-format on
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
    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const
    {
        return language.isValid() && !consonants.isEmpty() && !digits.isEmpty()
                && !symbols.isEmpty() && !vowels.isEmpty();
    }

    // clang-format off
    Q_PROPERTY(Language language
               MEMBER language)
    // clang-format on
    Language language;

    // clang-format off
    Q_PROPERTY(QList<AlphabetMapping> consonants
               MEMBER consonants)
    // clang-format on
    QList<AlphabetMapping> consonants;

    // clang-format off
    Q_PROPERTY(QList<AlphabetMapping> digits
               MEMBER digits)
    // clang-format on
    QList<AlphabetMapping> digits;

    // clang-format off
    Q_PROPERTY(QList<AlphabetMapping> symbols
               MEMBER symbols)
    // clang-format on
    QList<AlphabetMapping> symbols;

    // clang-format off
    Q_PROPERTY(QList<AlphabetMapping> vowels
               MEMBER vowels)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QString driver
               READ driver
               CONSTANT )
    // clang-format on
    static QString driver();

    // clang-format off
    Q_PROPERTY(QList<int> supportedLanguageCodes
               READ supportedLanguageCodes
               CONSTANT )
    // clang-format on
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

    /** Returns the default language supported by this engine */
    // clang-format off
    Q_PROPERTY(int defaultLanguage
               READ defaultLanguage
               NOTIFY defaultLanguageChanged)
    // clang-format on
    virtual int defaultLanguage() const { return QLocale::English; }
    Q_SIGNAL void defaultLanguageChanged();

    /* Unique name among all transliterators supported */
    // clang-format off
    Q_PROPERTY(QString name
               READ name
               CONSTANT )
    // clang-format on
    virtual QString name() const = 0;

    /** should return true if it supports transliteration to the said language */
    virtual bool canTransliterate(int lang) const { return !this->options(lang).isEmpty(); }

    /** should return a list of transliteration options */
    virtual QList<TransliterationOption> options(int lang) const = 0;

    bool doActivate(const TransliterationOption &option);

    /** called to verify if an option is still available */
    virtual bool canActivate(const TransliterationOption &option) = 0;

    /** caled to fetch the transliterated word for the said language **/
    virtual QString transliterateWord(const QString &word,
                                      const TransliterationOption &lang) const = 0;

protected:
    /** called as soon as editor receives focus **/
    virtual bool activate(const TransliterationOption &option) = 0;

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
    bool activate(const TransliterationOption &option);
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

    int defaultLanguage() const;

    // Activates the default language and returns its code
    int activateDefaultLanguage();

    // AbstractTransliterator interface
    QString name() const;
    QList<TransliterationOption> options(int lang) const;
    bool canActivate(const TransliterationOption &option);
    bool activate(const TransliterationOption &option);
    QString transliterateWord(const QString &word, const TransliterationOption &option) const;
};

/*
This is a fallback engine, that does nothing -- which means basically any keystroke
is passed through without transliteration.
*/
class FallbackTransliterationEngine : public AbstractTransliterationEngine
{
    Q_OBJECT

public:
    explicit FallbackTransliterationEngine(AbstractTransliterationEngine *platformEngine,
                                           QObject *parent = nullptr);
    ~FallbackTransliterationEngine();

    // AbstractTransliterator interface
    int defaultLanguage() const;
    QString name() const;
    QList<TransliterationOption> options(int lang) const;
    bool canActivate(const TransliterationOption &option);
    bool activate(const TransliterationOption &option);
    QString transliterateWord(const QString &word, const TransliterationOption &option) const;

private:
    QPointer<AbstractTransliterationEngine> m_platformEngine;
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

    // clang-format off
    Q_PROPERTY(bool enabled
               READ isEnabled
               WRITE setEnabled
               NOTIFY enabledChanged)
    // clang-format on
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    // clang-format off
    Q_PROPERTY(TransliterationOption option
               READ option
               WRITE setOption
               NOTIFY optionChanged)
    // clang-format on
    void setOption(const TransliterationOption &val);
    TransliterationOption option() const { return m_option; }
    Q_SIGNAL void optionChanged();

    // clang-format off
    Q_PROPERTY(QObject *popup
               READ popup
               WRITE setPopup
               NOTIFY popupChanged)
    // clang-format on
    void setPopup(QObject *val);
    QObject *popup() const { return m_popup; }
    Q_SIGNAL void popupChanged();

    // clang-format off
    Q_PROPERTY(QString currentWord
               READ currentWord
               NOTIFY currentWordChanged)
    // clang-format on
    QString currentWord() const { return m_currentWord.originalString; }
    Q_SIGNAL void currentWordChanged();

    // clang-format off
    Q_PROPERTY(QString commitString
               READ commitString
               NOTIFY commitStringChanged)
    // clang-format on
    QString commitString() const { return m_currentWord.commitString; }
    Q_SIGNAL void commitStringChanged();

    // clang-format off
    Q_PROPERTY(QRect textRect
               READ textRect
               NOTIFY textRectChanged)
    // clang-format on
    QRect textRect() const { return m_currentWord.textRect; }
    Q_SIGNAL void textRectChanged() const;

    // clang-format off
    Q_PROPERTY(QObject *editor
               READ editor
               CONSTANT )
    // clang-format on
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
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(int start
               MEMBER start)
    // clang-format on
    int start = -1;

    // clang-format off
    Q_PROPERTY(int end
               MEMBER end)
    // clang-format on
    int end = -1;

    // clang-format off
    Q_PROPERTY(QString text
               MEMBER text)
    // clang-format on
    QString text;

    // clang-format off
    Q_PROPERTY(QChar::Script script
               MEMBER script)
    // clang-format on
    QChar::Script script = QChar::Script_Unknown;

    // clang-format off
    Q_PROPERTY(QString fontFamily
               READ fontFamily)
    // clang-format on
    QString fontFamily() const;

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const;
};
Q_DECLARE_METATYPE(ScriptBoundary)
Q_DECLARE_METATYPE(QList<ScriptBoundary>)

class LanguageIconProvider : public QQuickImageProvider
{
public:
    explicit LanguageIconProvider();
    ~LanguageIconProvider();

    static QString name();
    static QUrl iconUrlFor(const Language &language);

    // QQuickImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

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

    // clang-format off
    Q_PROPERTY(AvailableLanguages *availableLanguages
               READ availableLanguages
               CONSTANT )
    // clang-format on
    AvailableLanguages *availableLanguages() const { return m_availableLanguages; }

    // clang-format off
    Q_PROPERTY(SupportedLanguages *supportedLanguages
               READ supportedLanguages
               CONSTANT )
    // clang-format on
    SupportedLanguages *supportedLanguages() const { return m_supportedLanguages; }

    // clang-format off
    Q_PROPERTY(bool handleLanguageSwitch
               READ isHandleLanguageSwitch
               WRITE setHandleLanguageSwitch
               NOTIFY handleLanguageSwitchChanged)
    // clang-format on
    void setHandleLanguageSwitch(bool val);
    bool isHandleLanguageSwitch() const { return m_handleLanguageSwitch; }
    Q_SIGNAL void handleLanguageSwitchChanged();

    Q_INVOKABLE bool setScriptFontFamily(QChar::Script script, const QString &fontFamily);
    Q_INVOKABLE QString scriptFontFamily(QChar::Script script) const;

    Q_INVOKABLE QStringList scriptFontFamilies(QChar::Script script) const;

    Q_INVOKABLE bool hasPlatformLanguages() const;

    QList<int> platformLanguages() const;
    QList<TransliterationOption> queryTransliterationOptions(int language) const;

    static QChar::Script determineScript(const QString &text);
    static QList<ScriptBoundary> determineBoundaries(const QString &paragraph);
    static void determineBoundariesAndInsertText(QTextCursor &cursor, const QString &paragraph);
    static QString formattedInHtml(const QString &paragraph);
    static int wordCount(const QString &paragraph);
    static int fastWordCount(const QString &paragraph);
    static int sentenceCount(const QString &paragraph);
    static int fastSentenceCount(const QString &paragraph);
    static QVector<QTextLayout::FormatRange>
    mergeTextFormats(const QList<ScriptBoundary> &boundaries,
                     const QVector<QTextLayout::FormatRange> &formats);
    static void polishFontsAndInsertTextAtCursor(
            QTextCursor &cursor, const QString &text,
            const QVector<QTextLayout::FormatRange> &formats = QVector<QTextLayout::FormatRange>());

    static void init(const char *uri, QQmlEngine *qmlEngine);

signals:
    void scriptFontFamilyChanged(QChar::Script script, const QString &fontFamily);
    void transliterationOptionsUpdated();

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    explicit LanguageEngine(QObject *parent = nullptr);

    void loadConfiguration();
    void saveConfiguration();
    void activateTransliterationOptionOnActiveLanguage();

private:
    bool m_handleLanguageSwitch = true;
    QString m_configFileName;
    AvailableLanguages *m_availableLanguages = nullptr;
    SupportedLanguages *m_supportedLanguages = nullptr;
    QMap<QChar::Script, QString> m_defaultScriptFontFamily, m_scriptFontFamily;
    QList<AbstractTransliterationEngine *> m_transliterators;
};

class PlatformLanguageObserver : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    PlatformLanguageObserver(QQuickItem *parent = nullptr);
    ~PlatformLanguageObserver();

    // clang-format off
    Q_PROPERTY(int activeLanguageCode
               READ activeLanguageCode
               NOTIFY activeLanguageCodeChanged)
    // clang-format on
    int activeLanguageCode() const { return m_activeLanguageCode; }
    Q_SIGNAL void activeLanguageCodeChanged();

    // clang-format off
    Q_PROPERTY(Language activeLanguage
               READ activeLanguage
               NOTIFY activeLanguageCodeChanged)
    // clang-format on
    Language activeLanguage() const;

protected:
    void setupObservation();

private:
    void setActiveLanguageCode(int val);

private:
    int m_activeLanguageCode = -1;
};

class QtChar : public QObject
{
    Q_OBJECT

private:
    QtChar() = delete;
    QtChar(QObject *parent) = delete;
    QtChar(const QtChar &other) = delete;
    QtChar &operator=(const QtChar &other) = delete;

public:
    // A complete copy of QChar::Script, because its just not possible to bundle this into an enum
    // For instance, Q_ENUM_NS(QChar::Script) just doesnt work as advertised.
    // This also means that everytime we use a new Qt version, we will have to sync this enum with
    // QChar::Script. Manual labour, I know. I wish there was a better way to do this.
    enum Script {
        Script_Unknown,
        Script_Inherited,
        Script_Common,

        Script_Latin,
        Script_Greek,
        Script_Cyrillic,
        Script_Armenian,
        Script_Hebrew,
        Script_Arabic,
        Script_Syriac,
        Script_Thaana,
        Script_Devanagari,
        Script_Bengali,
        Script_Gurmukhi,
        Script_Gujarati,
        Script_Oriya,
        Script_Tamil,
        Script_Telugu,
        Script_Kannada,
        Script_Malayalam,
        Script_Sinhala,
        Script_Thai,
        Script_Lao,
        Script_Tibetan,
        Script_Myanmar,
        Script_Georgian,
        Script_Hangul,
        Script_Ethiopic,
        Script_Cherokee,
        Script_CanadianAboriginal,
        Script_Ogham,
        Script_Runic,
        Script_Khmer,
        Script_Mongolian,
        Script_Hiragana,
        Script_Katakana,
        Script_Bopomofo,
        Script_Han,
        Script_Yi,
        Script_OldItalic,
        Script_Gothic,
        Script_Deseret,
        Script_Tagalog,
        Script_Hanunoo,
        Script_Buhid,
        Script_Tagbanwa,
        Script_Coptic,

        // Unicode 4.0 additions
        Script_Limbu,
        Script_TaiLe,
        Script_LinearB,
        Script_Ugaritic,
        Script_Shavian,
        Script_Osmanya,
        Script_Cypriot,
        Script_Braille,

        // Unicode 4.1 additions
        Script_Buginese,
        Script_NewTaiLue,
        Script_Glagolitic,
        Script_Tifinagh,
        Script_SylotiNagri,
        Script_OldPersian,
        Script_Kharoshthi,

        // Unicode 5.0 additions
        Script_Balinese,
        Script_Cuneiform,
        Script_Phoenician,
        Script_PhagsPa,
        Script_Nko,

        // Unicode 5.1 additions
        Script_Sundanese,
        Script_Lepcha,
        Script_OlChiki,
        Script_Vai,
        Script_Saurashtra,
        Script_KayahLi,
        Script_Rejang,
        Script_Lycian,
        Script_Carian,
        Script_Lydian,
        Script_Cham,

        // Unicode 5.2 additions
        Script_TaiTham,
        Script_TaiViet,
        Script_Avestan,
        Script_EgyptianHieroglyphs,
        Script_Samaritan,
        Script_Lisu,
        Script_Bamum,
        Script_Javanese,
        Script_MeeteiMayek,
        Script_ImperialAramaic,
        Script_OldSouthArabian,
        Script_InscriptionalParthian,
        Script_InscriptionalPahlavi,
        Script_OldTurkic,
        Script_Kaithi,

        // Unicode 6.0 additions
        Script_Batak,
        Script_Brahmi,
        Script_Mandaic,

        // Unicode 6.1 additions
        Script_Chakma,
        Script_MeroiticCursive,
        Script_MeroiticHieroglyphs,
        Script_Miao,
        Script_Sharada,
        Script_SoraSompeng,
        Script_Takri,

        // Unicode 7.0 additions
        Script_CaucasianAlbanian,
        Script_BassaVah,
        Script_Duployan,
        Script_Elbasan,
        Script_Grantha,
        Script_PahawhHmong,
        Script_Khojki,
        Script_LinearA,
        Script_Mahajani,
        Script_Manichaean,
        Script_MendeKikakui,
        Script_Modi,
        Script_Mro,
        Script_OldNorthArabian,
        Script_Nabataean,
        Script_Palmyrene,
        Script_PauCinHau,
        Script_OldPermic,
        Script_PsalterPahlavi,
        Script_Siddham,
        Script_Khudawadi,
        Script_Tirhuta,
        Script_WarangCiti,

        // Unicode 8.0 additions
        Script_Ahom,
        Script_AnatolianHieroglyphs,
        Script_Hatran,
        Script_Multani,
        Script_OldHungarian,
        Script_SignWriting,

        // Unicode 9.0 additions
        Script_Adlam,
        Script_Bhaiksuki,
        Script_Marchen,
        Script_Newa,
        Script_Osage,
        Script_Tangut,

        // Unicode 10.0 additions
        Script_MasaramGondi,
        Script_Nushu,
        Script_Soyombo,
        Script_ZanabazarSquare,

        // Unicode 12.1 additions
        Script_Dogra,
        Script_GunjalaGondi,
        Script_HanifiRohingya,
        Script_Makasar,
        Script_Medefaidrin,
        Script_OldSogdian,
        Script_Sogdian,
        Script_Elymaic,
        Script_Nandinagari,
        Script_NyiakengPuachueHmong,
        Script_Wancho,

        // Unicode 13.0 additions
        Script_Chorasmian,
        Script_DivesAkuru,
        Script_KhitanSmallScript,
        Script_Yezidi,

        ScriptCount
    };
    Q_ENUM(Script)
};

#endif // LANGUAGEENGINE_H
