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

#include "languageengine.h"
#include "application.h"

#include <PhTranslateLib>

#include <QDir>
#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QFontDatabase>
#include <QJsonDocument>

QString Language::name() const
{
    const QMetaObject *localeMetaObject = &QLocale::staticMetaObject;
    const QMetaEnum localeLanguageEnum =
            localeMetaObject->enumerator(localeMetaObject->indexOfEnumerator("Language"));
    return QString::fromLatin1(localeLanguageEnum.valueToKey(this->code));
}

QFont Language::defaultFont() const
{
    return LanguageEngine::instance()->defaultLanguageFont(this->code);
}

QStringList Language::aptFontFamilies() const
{
    return LanguageEngine::instance()->aptFontFamilies(this->code);
}

int Language::localeScript() const
{
    return QLocale(QLocale::Language(this->code)).script();
}

int Language::charScript() const
{
    // There is no Qt API to help us with this. Therefore we have to do it manually.
    // This also means that it will probably not be exhaustive.
    const QMap<QLocale::Language, QChar::Script> dict = {
        // Latin scripts
        { QLocale::English, QChar::Script_Latin },
        { QLocale::French, QChar::Script_Latin },
        { QLocale::German, QChar::Script_Latin },
        { QLocale::Spanish, QChar::Script_Latin },
        { QLocale::Portuguese, QChar::Script_Latin },
        { QLocale::Dutch, QChar::Script_Latin },
        { QLocale::Swedish, QChar::Script_Latin },
        { QLocale::Turkish, QChar::Script_Latin },
        { QLocale::Vietnamese, QChar::Script_Latin }, // Vietnamese uses Latin with diacritics

        // Cyrillic scripts
        { QLocale::Russian, QChar::Script_Cyrillic },
        { QLocale::Ukrainian, QChar::Script_Cyrillic },
        { QLocale::Belarusian, QChar::Script_Cyrillic },
        { QLocale::Serbian, QChar::Script_Cyrillic }, // Serbian can also use Latin script though
        { QLocale::Bulgarian, QChar::Script_Cyrillic },
        { QLocale::Macedonian, QChar::Script_Cyrillic },

        // Arabic script
        { QLocale::Arabic, QChar::Script_Arabic },
        { QLocale::Persian, QChar::Script_Arabic },
        { QLocale::Urdu, QChar::Script_Arabic },
        { QLocale::Pashto, QChar::Script_Arabic },
        { QLocale::Kurdish, QChar::Script_Arabic },

        // Han / CJK scripts
        { QLocale::Chinese, QChar::Script_Han }, // Traditional and Simplified Chinese
        { QLocale::Japanese, QChar::Script_Han }, // Kanji (+ Hiragana and Katakana)
        { QLocale::Korean, QChar::Script_Hangul }, // Hangul is Korean script

        // Indic scripts (major Indian languages)
        { QLocale::Hindi, QChar::Script_Devanagari },
        { QLocale::Marathi, QChar::Script_Devanagari },
        { QLocale::Sanskrit, QChar::Script_Devanagari },
        { QLocale::Nepali, QChar::Script_Devanagari },
        { QLocale::Bengali, QChar::Script_Bengali },
        { QLocale::Assamese, QChar::Script_Bengali }, // Bengali script for Assamese
        { QLocale::Gujarati, QChar::Script_Gujarati },
        { QLocale::Punjabi, QChar::Script_Gurmukhi },
        { QLocale::Oriya, QChar::Script_Oriya },
        { QLocale::Tamil, QChar::Script_Tamil },
        { QLocale::Telugu, QChar::Script_Telugu },
        { QLocale::Kannada, QChar::Script_Kannada },
        { QLocale::Malayalam, QChar::Script_Malayalam },

        // Other scripts
        { QLocale::Greek, QChar::Script_Greek },
        { QLocale::Hebrew, QChar::Script_Hebrew },
        { QLocale::Armenian, QChar::Script_Armenian },
        { QLocale::Georgian, QChar::Script_Georgian },
        { QLocale::Thai, QChar::Script_Thai },
        { QLocale::Lao, QChar::Script_Lao },
        { QLocale::Mongolian, QChar::Script_Mongolian },
        { QLocale::Tibetan, QChar::Script_Tibetan },
        { QLocale::Sinhala, QChar::Script_Sinhala },
        { QLocale::Khmer, QChar::Script_Khmer },
        { QLocale::Cherokee, QChar::Script_Cherokee },
    };

    return dict.value(QLocale::Language(this->code), QChar::Script_Latin);
}

int Language::fontWritingSystem() const
{
    // There is no Qt API to help us with this. Therefore we have to do it manually.
    // This also means that it will probably not be exhaustive.
    const QMap<QChar::Script, QFontDatabase::WritingSystem> dict = {
        { QChar::Script_Latin, QFontDatabase::Latin },
        { QChar::Script_Cyrillic, QFontDatabase::Cyrillic },
        { QChar::Script_Greek, QFontDatabase::Greek },
        { QChar::Script_Arabic, QFontDatabase::Arabic },
        { QChar::Script_Armenian, QFontDatabase::Armenian },
        { QChar::Script_Hebrew, QFontDatabase::Hebrew },
        { QChar::Script_Thai, QFontDatabase::Thai },
        { QChar::Script_Lao, QFontDatabase::Lao },
        { QChar::Script_Tibetan, QFontDatabase::Tibetan },
        { QChar::Script_Devanagari, QFontDatabase::Devanagari },
        { QChar::Script_Bengali, QFontDatabase::Bengali },
        { QChar::Script_Gurmukhi, QFontDatabase::Gurmukhi },
        { QChar::Script_Gujarati, QFontDatabase::Gujarati },
        { QChar::Script_Oriya, QFontDatabase::Oriya },
        { QChar::Script_Tamil, QFontDatabase::Tamil },
        { QChar::Script_Telugu, QFontDatabase::Telugu },
        { QChar::Script_Kannada, QFontDatabase::Kannada },
        { QChar::Script_Malayalam, QFontDatabase::Malayalam },
        { QChar::Script_Sinhala, QFontDatabase::Sinhala },
        { QChar::Script_Myanmar, QFontDatabase::Myanmar },
        { QChar::Script_Khmer, QFontDatabase::Khmer }
    };

    return dict.value(QChar::Script(this->charScript()), QFontDatabase::Any);
}

QList<TransliterationOption> Language::transliterationOptions() const
{
    return LanguageEngine::instance()->queryTransliterationOptions(this->code);
}

TransliterationOption Language::preferredTransliterationOption() const
{
    const QList<TransliterationOption> options = this->transliterationOptions();
    if (this->preferredTransliterationOptionId.isEmpty())
        return options.isEmpty() ? TransliterationOption() : options.first();

    auto it =
            std::find_if(options.begin(), options.end(), [=](const TransliterationOption &option) {
                return (option.id == this->preferredTransliterationOptionId);
            });
    return it != options.end() ? *it : TransliterationOption();
}

///////////////////////////////////////////////////////////////////////////////

AbstractLanguagesModel::AbstractLanguagesModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(this, &QAbstractListModel::modelReset, this, &AbstractLanguagesModel::countChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &AbstractLanguagesModel::countChanged);
    connect(this, &QAbstractListModel::rowsInserted, this, &AbstractLanguagesModel::countChanged);
}

AbstractLanguagesModel::~AbstractLanguagesModel() { }

int AbstractLanguagesModel::indexOfLanguage(int code) const
{
    auto it = std::find_if(m_languages.begin(), m_languages.end(),
                           [code](const Language &language) { return language.code == code; });
    return it != m_languages.end() ? std::distance(m_languages.begin(), it) : -1;
}

bool AbstractLanguagesModel::hasLanguage(int code) const
{
    return this->findLanguage(code).isValid();
}

Language AbstractLanguagesModel::findLanguage(int code) const
{
    auto it = std::find_if(m_languages.begin(), m_languages.end(),
                           [code](const Language &language) { return language.code == code; });
    if (it != m_languages.end())
        return *it;

    return Language();
}

QHash<int, QByteArray> AbstractLanguagesModel::roleNames() const
{
    return { { LanguageRole, QByteArrayLiteral("language") } };
}

QVariant AbstractLanguagesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_languages.size())
        return QVariant();

    const Language &language = m_languages[index.row()];
    if (role == LanguageRole)
        return QVariant::fromValue<Language>(language);

    return QVariant();
}

int AbstractLanguagesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_languages.size();
}

void AbstractLanguagesModel::addLanguage(const Language &language)
{
    this->updateLanguage(language);
}

void AbstractLanguagesModel::removeLanguage(const Language &language)
{
    if (!language.isValid())
        return;

    int row = this->indexOfLanguage(language.code);
    this->removeLanguageAt(row);
}

void AbstractLanguagesModel::removeLanguageAt(int row)
{
    if (row >= 0) {
        this->beginRemoveRows(QModelIndex(), row, row);
        m_languages.removeAt(row);
        this->endRemoveRows();
    }
}

void AbstractLanguagesModel::updateLanguage(const Language &language)
{
    if (!language.isValid())
        return;

    int row = this->indexOfLanguage(language.code);
    if (row < 0) {
        this->beginInsertRows(QModelIndex(), m_languages.size(), m_languages.size());
        m_languages.append(language);
        this->endInsertRows();
    } else {
        const QModelIndex index = this->index(row, 0);
        m_languages[row] = language;
        emit dataChanged(index, index);
    }
}

void AbstractLanguagesModel::setLanguages(const QList<Language> &languages)
{
    this->beginResetModel();

    m_languages.clear();

    for (const Language &language : languages) {
        if (language.isValid())
            m_languages.append(language);
    }

    this->endResetModel();
}

///////////////////////////////////////////////////////////////////////////////

SupportedLanguages::SupportedLanguages(QObject *parent) : AbstractLanguagesModel(parent)
{
    QTimer::singleShot(0, this, &SupportedLanguages::loadLanguages);

    connect(qApp, &QCoreApplication::aboutToQuit, this, &SupportedLanguages::saveLanguages);
}

SupportedLanguages::~SupportedLanguages() { }

void SupportedLanguages::addLanguage(int code)
{
    this->updateLanguage(code);
}

void SupportedLanguages::removeLanguage(int code)
{
    int row = this->indexOfLanguage(code);
    this->removeLanguageAt(row);
}

void SupportedLanguages::updateLanguage(int code)
{
    Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);
    this->AbstractLanguagesModel::updateLanguage(language);
}

bool SupportedLanguages::assignLanguageFont(int code, const QFont &font)
{
    Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);
    if (language.font != font) {
        language.font = font;
        this->AbstractLanguagesModel::updateLanguage(language);
        emit languageFontChanged(code);
    }

    return true;
}

bool SupportedLanguages::assignLanguageShortcut(int code, const QString &nativeSequence)
{
    if (DefaultTransliteration::supportedLanguageCodes().contains(code))
        return false; // Keyboard shortcuts cannot be modified for built-in languages.

    Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);
    if (language.keySequence.toString(QKeySequence::NativeText) == nativeSequence)
        return true;

    language.keySequence = QKeySequence::fromString(nativeSequence, QKeySequence::NativeText);
    if (language.keySequence.isEmpty())
        return false;

    this->AbstractLanguagesModel::updateLanguage(language);
    emit languageShortcutChanged(code);

    return true;
}

void SupportedLanguages::loadLanguages()
{
    // Load list of languages from a settings file. If none was found, then we can load built-in
    // languages
    m_settingsFileName = QFileInfo(Application::instance()->settingsFilePath())
                                 .absoluteDir()
                                 .absoluteFilePath(QStringLiteral("supported-languages.json"));

    QFile file(m_settingsFileName);
    if (file.open(QFile::ReadOnly)) {
        QList<Language> languages;

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        const QJsonArray array = doc.array();
        for (const QJsonValue &arrayItem : array) {
            const QJsonObject item = arrayItem.toObject();

            Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(
                    item.value("code").toInt());
            if (!language.isValid())
                continue;

            const QString shortcut = item.value("shortcut").toString();
            if (!shortcut.isEmpty()) {
                const QKeySequence keySequence =
                        QKeySequence::fromString(shortcut, QKeySequence::NativeText);
                if (!keySequence.isEmpty())
                    language.keySequence = keySequence;
            }

            const QString fontFamily = item.value("font-family").toString();
            if (Application::fontDatabase().hasFamily(fontFamily))
                language.font = QFont(fontFamily);

            language.preferredTransliterationOptionId = item.value("option").toString();
            if (!language.preferredTransliterationOption().isValid())
                language.preferredTransliterationOptionId = QString();

            languages.append(language);
        }

        if (!languages.isEmpty())
            this->setLanguages(languages);
    }

    if (this->count() == 0)
        this->loadBuiltInLanguages();

    connect(LanguageEngine::instance(), &LanguageEngine::transliterationOptionsUpdated, this,
            &SupportedLanguages::transliterationOptionsUpdated);
}

void SupportedLanguages::saveLanguages()
{
    if (m_settingsFileName.isEmpty())
        return;

    QFile file(m_settingsFileName);
    if (!file.open(QFile::WriteOnly))
        return;

    const QList<Language> languages = this->languages();

    QJsonArray array;

    for (const Language &language : languages) {
        QJsonObject item;
        item.insert("code", language.code);
        item.insert("name", language.name());
        if (language.code != QLocale::English
            && !DefaultTransliteration::supportedLanguageCodes().contains(language.code)) {
            item.insert("shortcut", language.keySequence.toString(QKeySequence::NativeText));
        } else {
            item.insert("default-shortcut",
                        language.keySequence.toString(QKeySequence::NativeText));
        }

        const QFont defaultFont = language.defaultFont();
        if (!(language.font == qApp->font() || language.font == defaultFont))
            item.insert("font-family", language.font.family());
        else
            item.insert("default-font-family", language.defaultFont().family());

        if (!language.preferredTransliterationOptionId.isEmpty()
            && language.preferredTransliterationOptionId != DefaultTransliteration::driver())
            item.insert("option", language.preferredTransliterationOptionId);

        array.append(item);
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
}

void SupportedLanguages::loadBuiltInLanguages()
{
    Application::log("SupportedLanguages::loadBuiltInLanguages()");

    QList<Language> languages;

    Language language =
            LanguageEngine::instance()->availableLanguages()->findLanguage(QLocale::English);
    language.keySequence = QKeySequence(QStringLiteral("Alt+E"));
    languages.append(language);

    const QList<int> builtInLanguages = DefaultTransliteration::supportedLanguageCodes();
    for (int code : builtInLanguages) {
        Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);

        switch (code) {
        case QLocale::Tamil:
            language.keySequence = QKeySequence(QStringLiteral("Alt+L"));
            break;
        case QLocale::Marathi:
            language.keySequence = QKeySequence(QStringLiteral("Alt+R"));
            break;
        default:
            language.keySequence =
                    QKeySequence(QStringLiteral("Alt+") + language.name().at(0).toUpper());
        }

        languages.append(language);
    }

    this->AbstractLanguagesModel::setLanguages(languages);
}

void SupportedLanguages::transliterationOptionsUpdated()
{
    QList<Language> languages = this->languages();
    for (Language &language : languages) {
        // Check if the preferred transliteration option still works
        const TransliterationOption pto = language.preferredTransliterationOption();
        if (pto.isValid())
            continue;

        // Fallback to first other option if available
        const QList<TransliterationOption> options = language.transliterationOptions();
        if (!options.isEmpty()) {
            language.preferredTransliterationOptionId = options.first().id;
            this->AbstractLanguagesModel::updateLanguage(language);
        } else {
            // Remove the language if no longer available.
            this->AbstractLanguagesModel::removeLanguage(language);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

AvailableLanguages::AvailableLanguages(QObject *parent) : AbstractLanguagesModel(parent)
{
    this->initialize();
}

AvailableLanguages::~AvailableLanguages() { }

void AvailableLanguages::initialize()
{
    const QMetaObject *localeMetaObject = &QLocale::staticMetaObject;
    const QMetaEnum localeLanguageEnum =
            localeMetaObject->enumerator(localeMetaObject->indexOfEnumerator("Language"));

    if (localeLanguageEnum.isValid()) {
        QList<Language> languages;

        for (int i = 0; i < localeLanguageEnum.keyCount(); i++) {
            Language language;
            language.code = localeLanguageEnum.value(i);
            languages.append(language);
        }

        this->setLanguages(languages);
    }
}

///////////////////////////////////////////////////////////////////////////////

DefaultTransliteration::DefaultTransliteration(QObject *parent) : QObject(parent) { }

DefaultTransliteration::~DefaultTransliteration() { }

QString DefaultTransliteration::driver()
{
    return QStringLiteral("PhTranslator");
}

QList<int> DefaultTransliteration::DefaultTransliteration::supportedLanguageCodes()
{
    return { QLocale::Bengali,   QLocale::Gujarati, QLocale::Hindi, QLocale::Kannada,
             QLocale::Malayalam, QLocale::Marathi,  QLocale::Oriya, QLocale::Punjabi,
             QLocale::Sanskrit,  QLocale::Tamil,    QLocale::Telugu };
}

QString DefaultTransliteration::onWord(const QString &word, int code)
{
    if (word.isEmpty())
        return word;

    void *translator = [code]() -> void * {
        switch (code) {
        case QLocale::Bengali:
            return GetBengaliTranslator();
        case QLocale::Gujarati:
            return GetGujaratiTranslator();
        case QLocale::Hindi:
            return GetHindiTranslator();
        case QLocale::Kannada:
            return GetKannadaTranslator();
        case QLocale::Malayalam:
            return GetMalayalamTranslator();
        case QLocale::Marathi:
            return GetMalayalamTranslator();
        case QLocale::Oriya:
            return GetOriyaTranslator();
        case QLocale::Punjabi:
            return GetPunjabiTranslator();
        case QLocale::Sanskrit:
            return GetSanskritTranslator();
        case QLocale::Tamil:
            return GetTamilTranslator();
        case QLocale::Telugu:
            return GetTeluguTranslator();
        default:
            break;
        }
        return nullptr;
    }();

    if (translator == nullptr)
        return word;

    const std::wstring wtext = word.toStdWString();
    return QString::fromStdWString(Translate(translator, wtext.c_str()));
}

template<class T>
QList<AlphabetMapping> loadFromPhTranslatorDefs(const T *array, int size)
{
    QList<AlphabetMapping> ret;

    for (int i = 0; i < size; i++) {
        const T *item = &array[i];
        QString latin = QString::fromLatin1(item->phRep);
        QString unicode;

        if (sizeof(T) == sizeof(PhTranslation::VowelDef)) {
            const PhTranslation::VowelDef *vitem =
                    reinterpret_cast<const PhTranslation::VowelDef *>(item);
            unicode = QString::fromWCharArray(&(vitem->uCode), 1) + QStringLiteral(", ")
                    + QString::fromWCharArray(&(vitem->dCode), 1);
        } else {
            unicode = QString::fromWCharArray(&(item->uCode), 1);
        }

        ret << AlphabetMapping({ latin, unicode });
    }

    return ret;
}

AlphabetMappings DefaultTransliteration::alphabetMappingsFor(int languageCode)
{
    AlphabetMappings ret;

    if (!supportedLanguageCodes().contains(languageCode))
        return ret;

    ret.language = LanguageEngine::instance()->availableLanguages()->findLanguage(languageCode);

#define COUNT(x) (sizeof(x) / sizeof(x[0]))
#define LOAD(x)                                                                                    \
    {                                                                                              \
        ret.vowels = loadFromPhTranslatorDefs<PhTranslation::VowelDef>(                            \
                PhTranslation::x::Vowels, COUNT(PhTranslation::x::Vowels));                        \
        ret.consonants = loadFromPhTranslatorDefs<PhTranslation::ConsonantDef>(                    \
                PhTranslation::x::Consonants, COUNT(PhTranslation::x::Consonants));                \
        ret.digits = loadFromPhTranslatorDefs<PhTranslation::DigitDef>(                            \
                PhTranslation::x::Digits, COUNT(PhTranslation::x::Digits));                        \
        ret.symbols = loadFromPhTranslatorDefs<PhTranslation::SpecialSymbolDef>(                   \
                PhTranslation::x::SpecialSymbols, COUNT(PhTranslation::x::SpecialSymbols));        \
    }

    switch (languageCode) {
    case QLocale::Bengali:
        LOAD(Bengali)
        break;
    case QLocale::Gujarati:
        LOAD(Gujarati)
        break;
    case QLocale::Hindi:
        LOAD(Hindi)
        break;
    case QLocale::Kannada:
        LOAD(Kannada)
        break;
    case QLocale::Malayalam:
        LOAD(Malayalam)
        break;
    case QLocale::Marathi:
        LOAD(Hindi)
        break;
    case QLocale::Oriya:
        LOAD(Oriya)
        break;
    case QLocale::Punjabi:
        LOAD(Punjabi)
        break;
    case QLocale::Sanskrit:
        LOAD(Sanskrit)
        break;
    case QLocale::Tamil:
        LOAD(Tamil)
        break;
    case QLocale::Telugu:
        LOAD(Telugu)
        break;
    }

#undef COUNT
#undef LOAD

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AbstractTransliterationEngine *TransliterationOption::transliterator() const
{
    return qobject_cast<AbstractTransliterationEngine *>(transliteratorObject);
}

bool TransliterationOption::isValid() const
{
    AbstractTransliterationEngine *t = this->transliterator();
    return t != nullptr && languageCode >= 0 && t->canActivate(*this);
}

void TransliterationOption::activate()
{
    AbstractTransliterationEngine *t = this->transliterator();
    if (t)
        t->activate(*this);
}

QString TransliterationOption::transliterateWord(const QString &word) const
{
    AbstractTransliterationEngine *t = this->transliterator();
    if (t)
        return t->transliterateWord(word, *this);

    return word;
}

///////////////////////////////////////////////////////////////////////////////

AbstractTransliterationEngine::AbstractTransliterationEngine(QObject *parent) : QObject(parent) { }

AbstractTransliterationEngine::~AbstractTransliterationEngine() { }

///////////////////////////////////////////////////////////////////////////////

StaticTransliterationEngine::StaticTransliterationEngine(QObject *parent)
    : AbstractTransliterationEngine(parent)
{
}

StaticTransliterationEngine::~StaticTransliterationEngine() { }

QString StaticTransliterationEngine::name() const
{
    return DefaultTransliteration::driver();
}

QList<TransliterationOption> StaticTransliterationEngine::options(int lang) const
{
    if (DefaultTransliteration::supportedLanguageCodes().contains(lang)) {
        return QList<TransliterationOption>(
                { { (QObject *)this, lang, this->name(), this->name(), true } });
    }

    return QList<TransliterationOption>();
}

bool StaticTransliterationEngine::canActivate(const TransliterationOption &option)
{
    return DefaultTransliteration::supportedLanguageCodes().contains(option.languageCode);
}

void StaticTransliterationEngine::activate(const TransliterationOption &option)
{
    // Nothing to do here
    Q_UNUSED(option)
}

void StaticTransliterationEngine::release(const TransliterationOption &option)
{
    // Nothing to do here
    Q_UNUSED(option)
}

QString StaticTransliterationEngine::transliterateWord(const QString &word,
                                                       const TransliterationOption &lang) const
{
    return DefaultTransliteration::onWord(word, lang.languageCode);
}

///////////////////////////////////////////////////////////////////////////////

LanguageEngine *LanguageEngine::instance()
{
    static QPointer<LanguageEngine> theInstance(new LanguageEngine(qApp));
    return theInstance;
}

LanguageEngine::LanguageEngine(QObject *parent) : QObject(parent)
{
    m_availableLanguages = new AvailableLanguages(this);
    m_supportedLanguages = new SupportedLanguages(this);

    // Static should always be a fallback, prefer the platform input method by
    // default. So, that should go first in the list.
    m_transliterators << new PlatformTransliterationEngine(this);
    m_transliterators << new StaticTransliterationEngine(this);
    for (AbstractTransliterationEngine *transliterator : qAsConst(m_transliterators))
        connect(transliterator, &AbstractTransliterationEngine::capacityChanged, this,
                &LanguageEngine::transliterationOptionsUpdated);

    const QList<QPair<int, QStringList>> bundledFonts = {
        { QLocale::Gujarati,
          { QStringLiteral(":/font/Gujarati/HindVadodara-Regular.ttf"),
            QStringLiteral(":/font/Gujarati/HindVadodara-Bold.ttf"),
            QStringLiteral(":/font/Oriya/BalooBhaina2-Regular.ttf") } },
        { QLocale::Oriya,
          { QStringLiteral(":/font/Oriya/BalooBhaina2-Regular.ttf"),
            QStringLiteral(":/font/Oriya/BalooBhaina2-Bold.ttf") } },
        { QLocale::Punjabi,
          { QStringLiteral(":/font/Punjabi/BalooPaaji2-Regular.ttf"),
            QStringLiteral(":/font/Punjabi/BalooPaaji2-Bold.ttf") } },
        { QLocale::Malayalam,
          { QStringLiteral(":/font/Malayalam/BalooChettan2-Regular.ttf"),
            QStringLiteral(":/font/Malayalam/BalooChettan2-Bold.ttf") } },
        { QLocale::Marathi, { QStringLiteral(":/font/Marathi/Shusha-Normal.ttf") } },
        { QLocale::Hindi,
          { QStringLiteral(":/font/Hindi/Mukta-Regular.ttf"),
            QStringLiteral(":/font/Hindi/Mukta-Bold.ttf") } },
        { QLocale::Telugu,
          { QStringLiteral(":/font/Telugu/HindGuntur-Regular.ttf"),
            QStringLiteral(":/font/Telugu/HindGuntur-Bold.ttf") } },
        { QLocale::Sanskrit,
          { QStringLiteral(":/font/Sanskrit/Mukta-Regular.ttf"),
            QStringLiteral(":/font/Sanskrit/Mukta-Bold.ttf") } },
        { QLocale::English,
          { QStringLiteral(":/font/English/CourierPrime-BoldItalic.ttf"),
            QStringLiteral(":/font/English/CourierPrime-Bold.ttf"),
            QStringLiteral(":/font/English/CourierPrime-Italic.ttf"),
            QStringLiteral(":/font/English/CourierPrime-Regular.ttf") } },
        { QLocale::Kannada,
          { QStringLiteral(":/font/Kannada/BalooTamma2-Regular.ttf"),
            QStringLiteral(":/font/Kannada/BalooTamma2-Bold.ttf") } },
        { QLocale::Tamil,
          { QStringLiteral(":/font/Tamil/HindMadurai-Regular.ttf"),
            QStringLiteral(":/font/Tamil/HindMadurai-Bold.ttf") } },
        { QLocale::Bengali,
          { QStringLiteral(":/font/Bengali/HindSiliguri-Regular.ttf"),
            QStringLiteral(":/font/Bengali/HindSiliguri-Bold.ttf") } }
    };

    const QFont safeDefaultFont(QStringLiteral("Courier Prime"));

    for (const QPair<int, QStringList> &bundle : bundledFonts) {
        int fontId = -1;
        for (const QString &fontFile : bundle.second) {
            const int id = QFontDatabase::addApplicationFont(fontFile);
            if (fontId < 0)
                fontId = id;
        }

        const QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        m_defaultLanguageFont[bundle.first] =
                fontFamilies.isEmpty() ? safeDefaultFont : QFont(fontFamilies.constFirst());
    }

    m_defaultLanguageFont[QLocale::AnyLanguage] = safeDefaultFont;
}

LanguageEngine::~LanguageEngine() { }

QFont LanguageEngine::defaultLanguageFont(int languageCode) const
{
    if (m_defaultLanguageFont.contains(languageCode))
        return m_defaultLanguageFont.value(languageCode);

    /*
     Ofcourse, we could have invoked aptFontFamilies() and instantiated a QFont on the
     first returned value. But that would mean compiling a list of all applicable font
     family names, only to pick the first one. That's honestly pointless.
     */
    const QFont safeDefault = m_defaultLanguageFont.value(QLocale::AnyLanguage);
    const Language language = m_availableLanguages->findLanguage(languageCode);

    QFontDatabase::WritingSystem writingSystem =
            QFontDatabase::WritingSystem(language.fontWritingSystem());
    if (writingSystem == QFontDatabase::Any || writingSystem == QFontDatabase::Latin)
        return safeDefault;

    const QStringList aptFontFamilies = Application::fontDatabase().families(writingSystem);
    if (aptFontFamilies.isEmpty())
        return safeDefault;

    const auto it = std::find_if(aptFontFamilies.begin(), aptFontFamilies.end(),
                                 [=](const QString &fontFamily) {
                                     return Application::fontDatabase().isFixedPitch(fontFamily);
                                 });
    if (it != aptFontFamilies.end())
        return QFont(*it);

    return safeDefault;
}

QStringList LanguageEngine::aptFontFamilies(int languageCode) const
{
    QStringList ret;

    // We could have used QSet instead of doing this circus, but we need
    // font family names to show up in a specific order.
    auto includeFontFamilies = [&ret](const QStringList &families) {
        for (const QString &family : families) {
            if (ret.isEmpty() || !ret.contains(family))
                ret.append(family);
        }
    };

    if (m_defaultLanguageFont.contains(languageCode))
        includeFontFamilies({ m_defaultLanguageFont[languageCode].family() });

    const Language language = m_availableLanguages->findLanguage(languageCode);
    const QFont safeDefault = m_defaultLanguageFont.value(QLocale::AnyLanguage);
    QFontDatabase::WritingSystem writingSystem =
            QFontDatabase::WritingSystem(language.fontWritingSystem());
    if (writingSystem == QFontDatabase::Any || writingSystem == QFontDatabase::Latin)
        includeFontFamilies({ safeDefault.family() });

    const QStringList availableFontFamilies = Application::fontDatabase().families(writingSystem);
    QStringList aptFontFamilies;
    std::copy_if(availableFontFamilies.begin(), availableFontFamilies.end(),
                 aptFontFamilies.begin(), [](const QString &fontFamily) {
                     return Application::fontDatabase().isFixedPitch(fontFamily);
                 });
    includeFontFamilies(aptFontFamilies);

    return ret;
}

QList<TransliterationOption> LanguageEngine::queryTransliterationOptions(int language) const
{
    QList<TransliterationOption> ret;
    for (AbstractTransliterationEngine *transliterator : m_transliterators) {
        const QList<TransliterationOption> options = transliterator->options(language);
        if (!options.isEmpty())
            ret += options;
    }

    return ret;
}

void LanguageEngine::init(const char *uri, QQmlEngine *qmlEngine)
{
    Q_UNUSED(qmlEngine)

    const char *reason = "Instantiation from QML not allowed.";

    // @uri io.scrite.components
    // @reason Instantiation from QML not allowed.
    qmlRegisterSingletonInstance(uri, 1, 0, "LanguageEngine", LanguageEngine::instance());
    qmlRegisterUncreatableMetaObject(QLocale::staticMetaObject, uri, 1, 0, "QtLocale", reason);
    qmlRegisterUncreatableMetaObject(QFontDatabase::staticMetaObject, uri, 1, 0, "QtFontDatabase",
                                     reason);
    qmlRegisterUncreatableMetaObject(QtChar::staticMetaObject, uri, 1, 0, "QtChar", reason);
}
