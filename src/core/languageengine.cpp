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
#include <QWindow>
#include <QFileInfo>
#include <QTextBlock>
#include <QScopeGuard>
#include <QApplication>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QTextDocument>
#include <QTextBoundaryFinder>

static const QMap<QLocale::Language, QChar::Script> languageScriptMap()
{
    // There is no Qt API to help us with this. Therefore we have to do it manually.
    // This also means that it will probably not be exhaustive.

    return {
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
}

static const QMap<QChar::Script, QFontDatabase::WritingSystem> scriptWritingSystemMap()
{
    // There is no Qt API to help us with this. Therefore we have to do it manually.
    // This also means that it will probably not be exhaustive.

    return { { QChar::Script_Latin, QFontDatabase::Latin },
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
             { QChar::Script_Khmer, QFontDatabase::Khmer } };
}

QString Language::name() const
{
    const QMetaEnum localeLanguageEnum = QMetaEnum::fromType<QLocale::Language>();
    return QString::fromLatin1(localeLanguageEnum.valueToKey(this->code));
}

QString Language::nativeName() const
{
    return QLocale(QLocale::Language(this->code)).nativeLanguageName();
}

QFont Language::font() const
{
    return LanguageEngine::instance()->scriptFontFamily(QChar::Script(charScript()));
}

QStringList Language::fontFamilies() const
{
    return LanguageEngine::instance()->scriptFontFamilies(QChar::Script(charScript()));
}

int Language::localeScript() const
{
    return QLocale(QLocale::Language(this->code)).script();
}

int Language::charScript() const
{
    return languageScriptMap().value(QLocale::Language(this->code), QChar::Script_Latin);
}

int Language::fontWritingSystem() const
{
    return scriptWritingSystemMap().value(QChar::Script(this->charScript()), QFontDatabase::Any);
}

bool Language::activate()
{
    TransliterationOption option = this->preferredTransliterationOption();
    return option.activate();
}

QString Language::charScriptName() const
{
    const QMetaEnum scriptEnum = QMetaEnum::fromType<Language::CharScript>();
    const QString scriptName = QString::fromLatin1(scriptEnum.valueToKey(this->charScript()));
    if (!scriptName.isEmpty())
        return scriptName.mid(7); // After Script_
    return QString();
}

QString Language::localeScriptName() const
{
    const QMetaEnum scriptEnum = QMetaEnum::fromType<QLocale::Script>();
    const QString scriptName = QString::fromLatin1(scriptEnum.valueToKey(this->localeScript()));
    if (!scriptName.isEmpty())
        return scriptName.left(scriptName.length() - 6); // Drop the Script suffix
    return QString();
}

QString Language::fontWritingSystemName() const
{
    const QMetaEnum writingSystemEnum = QMetaEnum::fromType<QFontDatabase::WritingSystem>();
    const QString writingSystemName =
            QString::fromLatin1(writingSystemEnum.valueToKey(this->fontWritingSystem()));
    return writingSystemName;
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

Language AbstractLanguagesModel::languageAt(int index) const
{
    return index < 0 || index >= m_languages.size() ? Language() : m_languages.at(index);
}

QHash<int, QByteArray> AbstractLanguagesModel::roleNames() const
{
    return { { LanguageRole, QByteArrayLiteral("language") },
             { CodeRole, QByteArray("languageCode") },
             { NameRole, QByteArrayLiteral("languageName") },
             { NativeNameRole, QByteArrayLiteral("nativeLanguageName") },
             { KeySequenceRole, QByteArrayLiteral("languageKeySequence") },
             { PreferredTransliterationOptionIdRole,
               QByteArrayLiteral("preferredTransliterationOptionId") } };
}

QVariant AbstractLanguagesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_languages.size())
        return QVariant();

    const Language &language = m_languages[index.row()];
    switch (role) {
    case LanguageRole:
        return QVariant::fromValue<Language>(language);
    case CodeRole:
        return language.code;
    case NameRole:
        return language.name();
    case NativeNameRole:
        return language.nativeName();
    case KeySequenceRole:
        return language.keySequence;
    case PreferredTransliterationOptionIdRole:
        return language.preferredTransliterationOptionId;
    };

    return QVariant();
}

int AbstractLanguagesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_languages.size();
}

int AbstractLanguagesModel::addLanguage(const Language &language)
{
    return this->updateLanguage(language);
}

int AbstractLanguagesModel::removeLanguage(const Language &language)
{
    if (!language.isValid())
        return -1;

    int row = this->indexOfLanguage(language.code);
    this->removeLanguageAt(row);
    return row;
}

int AbstractLanguagesModel::removeLanguageAt(int row)
{
    if (row >= 0) {
        this->beginRemoveRows(QModelIndex(), row, row);
        m_languages.removeAt(row);
        this->endRemoveRows();
        return row;
    }

    return -1;
}

int AbstractLanguagesModel::updateLanguage(const Language &language)
{
    if (!language.isValid())
        return -1;

    int row = this->indexOfLanguage(language.code);
    if (row < 0) {
        const int insertRow = language.code == QLocale::English ? 0 : m_languages.size();
        this->beginInsertRows(QModelIndex(), insertRow, insertRow);
        m_languages.insert(insertRow, language);
        this->endInsertRows();
        return insertRow;
    } else {
        const QModelIndex index = this->index(row, 0);
        m_languages[row] = language;
        emit dataChanged(index, index);
        return row;
    }

    return -1;
}

void AbstractLanguagesModel::setLanguages(const QList<Language> &languages)
{
    this->beginResetModel();

    m_languages.clear();

    for (const Language &language : languages) {
        if (language.isValid()) {
            if (language.code == QLocale::English)
                m_languages.prepend(language);
            else
                m_languages.append(language);
        }
    }

    this->endResetModel();
}

void AbstractLanguagesModel::initialize()
{
    // Nothing to do here
}

QJsonValue AbstractLanguagesModel::toJson() const
{
    return QJsonValue();
}

void AbstractLanguagesModel::fromJson(const QJsonValue &value)
{
    Q_UNUSED(value);
}

///////////////////////////////////////////////////////////////////////////////

SupportedLanguages::SupportedLanguages(QObject *parent) : AbstractLanguagesModel(parent)
{
    connect(this, &SupportedLanguages::activeLanguageCodeChanged, this,
            &SupportedLanguages::activeLanguageRowChanged);
    connect(this, &SupportedLanguages::rowsInserted, this,
            &SupportedLanguages::verifyActiveLanguage);
    connect(this, &SupportedLanguages::rowsRemoved, this,
            &SupportedLanguages::verifyActiveLanguage);
    connect(this, &SupportedLanguages::rowsMoved, this, &SupportedLanguages::verifyActiveLanguage);
    connect(this, &SupportedLanguages::modelReset, this, &SupportedLanguages::verifyActiveLanguage);
    connect(this, &SupportedLanguages::dataChanged, this, &SupportedLanguages::onDataChanged);
}

SupportedLanguages::~SupportedLanguages() { }

void SupportedLanguages::setActiveLanguageCode(int val)
{
    if (m_activeLanguageCode == val)
        return;

    const Language language = this->findLanguage(val);

    m_activeLanguageCode = val >= 0 && language.isValid() ? val : -1;

    if (language.isValid()) {
        const TransliterationOption option = language.preferredTransliterationOption();
        if (option.isValid()) {
            AbstractTransliterationEngine *engine =
                    qobject_cast<AbstractTransliterationEngine *>(option.transliteratorObject);
            if (engine)
                engine->activate(option);
        }
    }

    emit activeLanguageCodeChanged();
}

void SupportedLanguages::setDefaultLanguageCode(int val)
{
    if (m_defaultLanguageCode == val)
        return;

    m_defaultLanguageCode = val;
    emit defaultLanguageCodeChanged();
}

Language SupportedLanguages::activeLanguage() const
{
    if (m_activeLanguageCode >= 0)
        return this->findLanguage(m_activeLanguageCode);

    return Language();
}

Language SupportedLanguages::defaultLanguage() const
{
    Language language = this->findLanguage(m_defaultLanguageCode < 0 ? QLocale::English
                                                                     : m_defaultLanguageCode);
    if (!language.isValid()) {
        if (!this->hasLanguage(QLocale::English))
            const_cast<SupportedLanguages *>(this)->addLanguage(QLocale::English);
        language = this->findLanguage(QLocale::English);
    }

    return language;
}

int SupportedLanguages::activeLanguageRow() const
{
    return this->indexOfLanguage(m_activeLanguageCode);
}

int SupportedLanguages::addLanguage(int code)
{
    return this->updateLanguage(code);
}

int SupportedLanguages::removeLanguage(int code)
{
    if (this->count() == 1 || code == QLocale::English)
        return -1;

    int row = this->indexOfLanguage(code);

    this->removeLanguageAt(row);

    if (m_defaultLanguageCode == code)
        this->setDefaultLanguageCode(this->languageAt(0).code);

    if (m_activeLanguageCode == code)
        this->setActiveLanguageCode(m_defaultLanguageCode);

    return row;
}

int SupportedLanguages::updateLanguage(int code)
{
    Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);
    if (language.isValid()) {
        if (DefaultTransliteration::supportedLanguageCodes().contains(language.code)) {
            language.keySequence =
                    QKeySequence::fromString(DefaultTransliteration::shortcut(language.code));
        }
        return this->AbstractLanguagesModel::updateLanguage(language);
    }
    return -1;
}

bool SupportedLanguages::assignLanguageShortcut(int code, const QString &nativeSequence)
{
    if (DefaultTransliteration::supportedLanguageCodes().contains(code))
        return false; // Keyboard shortcuts cannot be modified for built-in languages.

    Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);
    if (!language.isValid())
        return false;

    if (language.keySequence.toString() == nativeSequence)
        return true;

    language.keySequence = QKeySequence::fromString(nativeSequence);
    if (language.keySequence.isEmpty())
        return false;

    this->AbstractLanguagesModel::updateLanguage(language);
    emit languageShortcutChanged(code);

    return true;
}

bool SupportedLanguages::assignLanguageFontFamily(int code, const QString &fontFamily)
{
    const Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);
    if (language.isValid()) {
        return LanguageEngine::instance()->setScriptFontFamily(QChar::Script(language.charScript()),
                                                               fontFamily);
    }

    return false;
}

bool SupportedLanguages::useLanguageTransliterator(int code, const TransliterationOption &option)
{
    if (!option.isValid())
        return false;

    Language language = this->findLanguage(code);
    if (!language.isValid())
        return false;

    if (language.preferredTransliterationOptionId == option.id)
        return true;

    language.preferredTransliterationOptionId = option.id;
    this->AbstractLanguagesModel::updateLanguage(language);
    emit languageTransliteratorChanged(code);

    return true;
}

bool SupportedLanguages::useLanguageTransliteratorId(int code, const QString &id)
{
    if (id.isEmpty())
        return false;

    Language language = this->findLanguage(code);
    if (!language.isValid())
        return false;

    const QList<TransliterationOption> options = language.transliterationOptions();
    for (const TransliterationOption &option : options) {
        if (option.id == id)
            return this->useLanguageTransliterator(code, option);
    }

    return false;
}

bool SupportedLanguages::resetLanguageTranslator(int code)
{
    Language language = this->findLanguage(code);
    if (!language.isValid())
        return false;

    if (language.preferredTransliterationOptionId.isEmpty())
        return true;

    language.preferredTransliterationOptionId = QString();
    this->AbstractLanguagesModel::updateLanguage(language);
    emit languageTransliteratorChanged(code);

    return true;
}

void SupportedLanguages::loadBuiltInLanguages()
{
    QList<Language> languages;

    Language language =
            LanguageEngine::instance()->availableLanguages()->findLanguage(QLocale::English);
    language.keySequence = QKeySequence(DefaultTransliteration::shortcut(QLocale::English));
    languages.append(language);

    const QList<int> builtInLanguages = DefaultTransliteration::supportedLanguageCodes();
    for (int code : builtInLanguages) {
        Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(code);
        language.keySequence = QKeySequence::fromString(DefaultTransliteration::shortcut(code));
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

void SupportedLanguages::onScriptFontFamilyChanged(QChar::Script script, const QString &fontFamily)
{
    Q_UNUSED(fontFamily);

    const QList<Language> &languages = this->languages();
    for (int i = 0; i < languages.size(); i++) {
        const Language &language = languages[i];
        if (language.charScript() == script) {
            const QModelIndex index = this->index(i, 0);
            emit dataChanged(index, index);
        }
    }
}

void SupportedLanguages::initialize()
{
    connect(LanguageEngine::instance(), &LanguageEngine::transliterationOptionsUpdated, this,
            &SupportedLanguages::transliterationOptionsUpdated);
    connect(LanguageEngine::instance(), &LanguageEngine::scriptFontFamilyChanged, this,
            &SupportedLanguages::onScriptFontFamilyChanged);
}

QJsonValue SupportedLanguages::toJson() const
{
    const QList<Language> languages = this->languages();
    if (languages.isEmpty())
        return QJsonValue();

    QJsonArray array;

    for (const Language &language : languages) {
        QJsonObject item;
        item.insert("code", language.code);
        item.insert("name", language.name());
        if (language.code != QLocale::English
            && !DefaultTransliteration::supportedLanguageCodes().contains(language.code)) {
            item.insert("shortcut", language.keySequence.toString());
        } else {
            item.insert("default-shortcut", language.keySequence.toString());
        }

        if (!language.preferredTransliterationOptionId.isEmpty()
            && language.preferredTransliterationOptionId != DefaultTransliteration::driver())
            item.insert("option", language.preferredTransliterationOptionId);

        if (m_activeLanguageCode == language.code)
            item.insert("active", true);

        if (m_defaultLanguageCode == language.code)
            item.insert("default", true);

        array.append(item);
    }

    return array;
}

void SupportedLanguages::fromJson(const QJsonValue &value)
{
    auto scopeGuard = qScopeGuard([=]() {
        if (this->count() == 0) {
            this->loadBuiltInLanguages();
            this->setActiveLanguageCode(QLocale::English);
        }
    });

    if (value.isNull())
        return;

    QList<Language> languages;

    const QJsonArray array = value.toArray();
    if (array.isEmpty())
        return;

    int activeLanguageCode = -1;
    int defaultLanguageCode = -1;

    for (const QJsonValue &arrayItem : array) {
        const QJsonObject item = arrayItem.toObject();

        Language language = LanguageEngine::instance()->availableLanguages()->findLanguage(
                item.value("code").toInt());
        if (!language.isValid())
            continue;

        const QString shortcut = item.value("shortcut").toString();
        if (!shortcut.isEmpty()) {
            const QKeySequence keySequence = QKeySequence::fromString(shortcut);
            if (!keySequence.isEmpty())
                language.keySequence = keySequence;
        } else {
            const QString defaultShortcut = DefaultTransliteration::shortcut(language.code);
            if (!defaultShortcut.isEmpty())
                language.keySequence = QKeySequence::fromString(defaultShortcut);
        }

        language.preferredTransliterationOptionId = item.value("option").toString();
        if (!language.preferredTransliterationOption().isValid())
            language.preferredTransliterationOptionId = QString();

        if (item.value("active").toBool() == true)
            activeLanguageCode = language.code;

        if (item.value("default").toBool() == true)
            defaultLanguageCode = language.code;

        languages.append(language);
    }

    if (!languages.isEmpty())
        this->setLanguages(languages);

    if (!this->hasLanguage(QLocale::English))
        this->addLanguage(QLocale::English);

    if (activeLanguageCode < 0 || defaultLanguageCode < 0) {
        activeLanguageCode = activeLanguageCode < 0 ? QLocale::English : activeLanguageCode;
        defaultLanguageCode = defaultLanguageCode < 0 ? QLocale::English : defaultLanguageCode;
    }

    this->setActiveLanguageCode(activeLanguageCode);
    this->setDefaultLanguageCode(defaultLanguageCode);
}

void SupportedLanguages::onDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    const int alr = this->activeLanguageRow();
    if (alr >= start.row() && alr <= end.row())
        emit activeLanguageCodeChanged(); // So that a new language is returned against
                                          // activeLanguage() with updated properties.
}

void SupportedLanguages::verifyActiveLanguage()
{
    Language language = this->findLanguage(m_activeLanguageCode);
    if (!language.isValid()) {
        const Language language = this->languageAt(0);
        this->setActiveLanguageCode(language.isValid() ? language.code : -1);
    }

    emit activeLanguageRowChanged(); // even if its not required, its safe to emit this.
}

///////////////////////////////////////////////////////////////////////////////

AvailableLanguages::AvailableLanguages(QObject *parent) : AbstractLanguagesModel(parent) { }

AvailableLanguages::~AvailableLanguages() { }

void AvailableLanguages::initialize()
{
    const QMetaEnum localeLanguageEnum = QMetaEnum::fromType<QLocale::Language>();

    const QList<QLocale::Language> languagesToSkip({ QLocale::AnyLanguage, QLocale::C });

    QSet<QLocale::Language> languagesAdded;

    if (localeLanguageEnum.isValid()) {
        QList<Language> languages;

        for (int i = 0; i < localeLanguageEnum.keyCount(); i++) {
            QLocale::Language _language = QLocale::Language(localeLanguageEnum.value(i));
            if (languagesToSkip.contains(_language) || languagesAdded.contains(_language))
                continue;

            const QLocale locale(_language);
            if (locale.textDirection() != Qt::LeftToRight)
                continue;

            Language language;
            language.code = _language;
            languages.append(language);

            languagesAdded += _language;
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

bool DefaultTransliteration::supportsLanguageCode(int code) const
{
    return supportedLanguageCodes().contains(code);
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

QString DefaultTransliteration::onParagraph(const QString &paragraph, int code)
{
    if (paragraph.isEmpty() || !supportedLanguageCodes().contains(code))
        return paragraph;

    QString ret;

    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, paragraph);

    while (boundaryFinder.position() < paragraph.length()) {
        if (!(boundaryFinder.boundaryReasons().testFlag(QTextBoundaryFinder::StartOfItem))) {
            if (boundaryFinder.toNextBoundary() == -1) {
                break;
            }
            continue;
        }

        const int start = boundaryFinder.position();
        const int end = boundaryFinder.toNextBoundary();
        if (end < 0)
            break;

        if (end - start < 1)
            continue;

        ret += onWord(paragraph.mid(start, end - start), code);

        const int next = boundaryFinder.toNextBoundary();
        if (next < 0)
            break;

        if (next - end >= 1)
            ret += paragraph.midRef(end, next - end);
    }

    return ret;
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

QString DefaultTransliteration::shortcut(int languageCode)
{
    QString prefix = QStringLiteral("Alt+");

    switch (languageCode) {
    case QLocale::Bengali:
        return prefix + "B";
    case QLocale::Gujarati:
        return prefix + "G";
    case QLocale::Hindi:
        return prefix + "H";
    case QLocale::Kannada:
        return prefix + "K";
    case QLocale::Malayalam:
        return prefix + "M";
    case QLocale::Marathi:
        return prefix + "R";
    case QLocale::Oriya:
        return prefix + "O";
    case QLocale::Punjabi:
        return prefix + "P";
    case QLocale::Sanskrit:
        return prefix + "S";
    case QLocale::Tamil:
        return prefix + "L";
    case QLocale::Telugu:
        return prefix + "T";
    case QLocale::English:
        return prefix + "E";
    default:
        break;
    }

    return QString();
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

bool TransliterationOption::activate()
{
    AbstractTransliterationEngine *t = this->transliterator();
    if (t)
        return t->activate(*this);

    return false;
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

FallbackTransliterationEngine::FallbackTransliterationEngine(QObject *parent)
    : AbstractTransliterationEngine(parent)
{
}

FallbackTransliterationEngine::~FallbackTransliterationEngine() { }

QString FallbackTransliterationEngine::name() const
{
    return QStringLiteral("Default");
}

QList<TransliterationOption> FallbackTransliterationEngine::options(int lang) const
{
    return { { (QObject *)this, lang, this->name(), QStringLiteral("Keyboard Layout"), false } };
}

bool FallbackTransliterationEngine::canActivate(const TransliterationOption &option)
{
    return option.transliteratorObject == this;
}

bool FallbackTransliterationEngine::activate(const TransliterationOption &option)
{
    // Do nothing
    Q_UNUSED(option)
    return true;
}

void FallbackTransliterationEngine::release(const TransliterationOption &option)
{
    // Do nothing
}

QString FallbackTransliterationEngine::transliterateWord(const QString &word,
                                                         const TransliterationOption &option) const
{
    Q_UNUSED(option);
    return word;
}

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
                { { (QObject *)this, lang, QStringLiteral("Built-In"), this->name(), true } });
    }

    return QList<TransliterationOption>();
}

bool StaticTransliterationEngine::canActivate(const TransliterationOption &option)
{
    return option.transliteratorObject == this
            && DefaultTransliteration::supportedLanguageCodes().contains(option.languageCode);
}

bool StaticTransliterationEngine::activate(const TransliterationOption &option)
{
    // Nothing to do here
    Q_UNUSED(option)
    return true;
}

void StaticTransliterationEngine::release(const TransliterationOption &option)
{
    // Nothing to do here
    Q_UNUSED(option)
}

QString StaticTransliterationEngine::transliterateWord(const QString &word,
                                                       const TransliterationOption &option) const
{
    return DefaultTransliteration::onWord(word, option.languageCode);
}

///////////////////////////////////////////////////////////////////////////////

LanguageTransliterator::LanguageTransliterator(QObject *parent) : QObject(parent)
{
    if (parent != nullptr) {
        QInputMethodQueryEvent imQuery(Qt::ImEnabled);
        if (qApp->sendEvent(parent, &imQuery)) {
            if (imQuery.value(Qt::ImEnabled).toBool()) {
                parent->installEventFilter(this);
                m_editor = parent;
            }
        }
    }
}

LanguageTransliterator::~LanguageTransliterator() { }

LanguageTransliterator *LanguageTransliterator::qmlAttachedProperties(QObject *object)
{
    return new LanguageTransliterator(object);
}

void LanguageTransliterator::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void LanguageTransliterator::setOption(const TransliterationOption &val)
{
    if (m_option == val)
        return;

    m_option = val;
    emit optionChanged();
}

void LanguageTransliterator::setPopup(QObject *val)
{
    if (m_popup == val)
        return;

    if (m_popup != nullptr) {
        m_popup->setProperty("transliterator", QVariant());
    }

    m_popup = val;
    emit popupChanged();

    if (m_popup != nullptr) {
        m_popup->setProperty("transliterator", QVariant::fromValue<QObject *>(this));
    }
}

bool LanguageTransliterator::eventFilter(QObject *object, QEvent *event)
{
    if (m_editor != nullptr && object == m_editor && m_enabled && m_option.isValid()
        && m_option.inApp) {
        if (event->type() == QEvent::FocusOut) {
            this->commitWordToEditor();
            return false;
        }

        AbstractTransliterationEngine *transliterationEngine =
                qobject_cast<AbstractTransliterationEngine *>(m_option.transliteratorObject);

        if (event->type() == QEvent::FocusIn) {
            transliterationEngine->activate(m_option);
            return false;
        }

        if (m_editor == qApp->focusObject() && event->type() == QEvent::KeyPress) {
            if (transliterationEngine == nullptr) {
                this->resetCurrentWord();
                return false;
            }

            QInputMethodQueryEvent imQuery(Qt::ImEnabled);
            if (qApp->sendEvent(m_editor, &imQuery) && !imQuery.value(Qt::ImEnabled).toBool())
                return false;

            const QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);

            if (keyEvent->key() == Qt::Key_Escape) {
                this->resetCurrentWord();
                return true;
            }

            if (!this->updateWordFromInput(keyEvent)) {
                const QList<int> exceptions = { Qt::Key_Backspace, Qt::Key_Delete, Qt::Key_Shift,
                                                Qt::Key_Control,   Qt::Key_Alt,    Qt::Key_Meta,
                                                Qt::Key_CapsLock,  Qt::Key_NumLock };
                if (exceptions.contains(keyEvent->key()))
                    return false;

                this->commitWordToEditor();
            }
        }

        return false;
    }

    return QObject::eventFilter(object, event);
}

bool LanguageTransliterator::updateWordFromInput(const QKeyEvent *keyEvent)
{
    if (m_editor == nullptr || m_editor != qApp->focusObject())
        return false;

    const QList<int> delimterKeys = { Qt::Key_Space, Qt::Key_Tab, Qt::Key_Return, Qt::Key_Enter };
    if (delimterKeys.contains(keyEvent->key()))
        return false;

    const QString inputText = keyEvent->text();
    if (inputText.length() == 1 && inputText[0].isPunct())
        return false;

    QInputMethodQueryEvent query(Qt::ImCursorPosition | Qt::ImCursorRectangle);
    qApp->sendEvent(m_editor, &query);

    const int cursorPosition = query.value(Qt::ImCursorPosition).toInt();
    const QRect cursorRect = query.value(Qt::ImCursorRectangle).toRect();

    if (m_currentWord.start < 0 || m_currentWord.originalString.isEmpty())
        m_currentWord.start = cursorPosition;
    m_currentWord.end = cursorPosition;

    if (keyEvent->key() == Qt::Key_Backspace) {
        if (!m_currentWord.originalString.isEmpty())
            m_currentWord.originalString.remove(m_currentWord.originalString.length() - 1, 1);
    } else
        m_currentWord.originalString += inputText;
    emit currentWordChanged();

    AbstractTransliterationEngine *transliterationEngine =
            qobject_cast<AbstractTransliterationEngine *>(m_option.transliteratorObject);

    m_currentWord.commitString =
            transliterationEngine->transliterateWord(m_currentWord.originalString, m_option);
    emit commitStringChanged();

    if (!m_currentWord.textRect.isValid() || m_currentWord.textRect.isEmpty())
        m_currentWord.textRect = cursorRect;
    else
        m_currentWord.textRect |= cursorRect;
    emit textRectChanged();

    return true;
}

bool LanguageTransliterator::commitWordToEditor()
{
    if (m_editor == nullptr || m_currentWord.originalString.isEmpty())
        return false;

    if (m_currentWord.commitString == m_currentWord.originalString) {
        this->resetCurrentWord();
        return false;
    }

    QInputMethodQueryEvent query(Qt::ImCursorPosition);
    qApp->sendEvent(m_editor, &query);
    const int cp = query.value(Qt::ImCursorPosition).toInt();
    const int offset = m_currentWord.start - cp;

    QInputMethodEvent commitEvent;
    commitEvent.setCommitString(m_currentWord.commitString, offset, qAbs(offset));
    qApp->sendEvent(m_editor, &commitEvent);

    this->resetCurrentWord();
    return true;
}

void LanguageTransliterator::resetCurrentWord()
{
    m_currentWord.start = -1;
    m_currentWord.end = -1;

    m_currentWord.originalString.clear();
    emit currentWordChanged();

    m_currentWord.commitString.clear();
    emit commitStringChanged();

    QTimer::singleShot(100, this, [=]() {
        m_currentWord.textRect = QRect();
        emit textRectChanged();
    });
}

///////////////////////////////////////////////////////////////////////////////

QString ScriptBoundary::fontFamily() const
{
    return LanguageEngine::instance()->scriptFontFamily(this->script);
}

bool ScriptBoundary::isValid() const
{
    return this->start >= 0 && this->end > 0 && this->end - this->start > 1
            && !this->text.isEmpty();
}

///////////////////////////////////////////////////////////////////////////////

LanguageEngine *LanguageEngine::instance()
{
    static bool typesRegistered = false;
    if (!typesRegistered) {
        qRegisterMetaType<Language>("Language");
        qRegisterMetaType<ScriptBoundary>("ScriptBoundary");
        qRegisterMetaType<AlphabetMapping>("AlphabetMapping");
        qRegisterMetaType<AlphabetMappings>("AlphabetMappings");
        qRegisterMetaType<TransliterationOption>("TransliterationOption");

        qRegisterMetaType<QList<AlphabetMapping>>("QList<AlphabetMapping>");
        qRegisterMetaType<QList<ScriptBoundary>>("QList<ScriptBoundary>");
        qRegisterMetaType<QList<TransliterationOption>>("QList<TransliterationOption>");

        typesRegistered = true;
    }

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
    m_transliterators << new FallbackTransliterationEngine(this);
    for (AbstractTransliterationEngine *transliterator : qAsConst(m_transliterators))
        connect(transliterator, &AbstractTransliterationEngine::capacityChanged, this,
                &LanguageEngine::transliterationOptionsUpdated);

    const QList<std::tuple<QLocale::Language, QChar::Script, QStringList>> bundledFonts = {
        { QLocale::Gujarati,
          QChar::Script_Gujarati,
          { QStringLiteral(":/font/Gujarati/HindVadodara-Regular.ttf"),
            QStringLiteral(":/font/Gujarati/HindVadodara-Bold.ttf"),
            QStringLiteral(":/font/Oriya/BalooBhaina2-Regular.ttf") } },
        { QLocale::Oriya,
          QChar::Script_Oriya,
          { QStringLiteral(":/font/Oriya/BalooBhaina2-Regular.ttf"),
            QStringLiteral(":/font/Oriya/BalooBhaina2-Bold.ttf") } },
        { QLocale::Punjabi,
          QChar::Script_Gurmukhi,
          { QStringLiteral(":/font/Punjabi/BalooPaaji2-Regular.ttf"),
            QStringLiteral(":/font/Punjabi/BalooPaaji2-Bold.ttf") } },
        { QLocale::Malayalam,
          QChar::Script_Malayalam,
          { QStringLiteral(":/font/Malayalam/BalooChettan2-Regular.ttf"),
            QStringLiteral(":/font/Malayalam/BalooChettan2-Bold.ttf") } },
        { QLocale::Marathi,
          QChar::Script_Devanagari,
          { QStringLiteral(":/font/Marathi/Shusha-Normal.ttf") } },
        { QLocale::Hindi,
          QChar::Script_Devanagari,
          { QStringLiteral(":/font/Hindi/Mukta-Regular.ttf"),
            QStringLiteral(":/font/Hindi/Mukta-Bold.ttf") } },
        { QLocale::Telugu,
          QChar::Script_Telugu,
          { QStringLiteral(":/font/Telugu/HindGuntur-Regular.ttf"),
            QStringLiteral(":/font/Telugu/HindGuntur-Bold.ttf") } },
        { QLocale::Sanskrit,
          QChar::Script_Devanagari,
          { QStringLiteral(":/font/Sanskrit/Mukta-Regular.ttf"),
            QStringLiteral(":/font/Sanskrit/Mukta-Bold.ttf") } },
        { QLocale::English,
          QChar::Script_Latin,
          { QStringLiteral(":/font/English/CourierPrime-BoldItalic.ttf"),
            QStringLiteral(":/font/English/CourierPrime-Bold.ttf"),
            QStringLiteral(":/font/English/CourierPrime-Italic.ttf"),
            QStringLiteral(":/font/English/CourierPrime-Regular.ttf") } },
        { QLocale::Kannada,
          QChar::Script_Kannada,
          { QStringLiteral(":/font/Kannada/BalooTamma2-Regular.ttf"),
            QStringLiteral(":/font/Kannada/BalooTamma2-Bold.ttf") } },
        { QLocale::Tamil,
          QChar::Script_Tamil,
          { QStringLiteral(":/font/Tamil/HindMadurai-Regular.ttf"),
            QStringLiteral(":/font/Tamil/HindMadurai-Bold.ttf") } },
        { QLocale::Bengali,
          QChar::Script_Bengali,
          { QStringLiteral(":/font/Bengali/HindSiliguri-Regular.ttf"),
            QStringLiteral(":/font/Bengali/HindSiliguri-Bold.ttf") } }
    };

    const QString safeDefaultFontFamily(QStringLiteral("Courier Prime"));

    for (const std::tuple<QLocale::Language, QChar::Script, QStringList> &bundle : bundledFonts) {
        int fontId = -1;
        for (const QString &fontFile : std::get<2>(bundle)) {
            const int id = QFontDatabase::addApplicationFont(fontFile);
            if (fontId < 0)
                fontId = id;
        }

        const QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        m_defaultScriptFontFamily[std::get<1>(bundle)] =
                fontFamilies.isEmpty() ? safeDefaultFontFamily : fontFamilies.constFirst();
    }
    m_defaultScriptFontFamily[QChar::Script_Unknown] = safeDefaultFontFamily;

    connect(qApp, &QCoreApplication::aboutToQuit, this, &LanguageEngine::saveConfiguration);

    QTimer::singleShot(0, this, &LanguageEngine::loadConfiguration);
}

LanguageEngine::~LanguageEngine() { }

bool LanguageEngine::setScriptFontFamily(QChar::Script script, const QString &fontFamily)
{
    if (m_scriptFontFamily.value(script) == fontFamily || fontFamily.isEmpty())
        return false;

    if (!Application::fontDatabase().hasFamily(fontFamily))
        return false;

    m_scriptFontFamily[script] = fontFamily;
    emit scriptFontFamilyChanged(script, fontFamily);

    return true;
}

QString LanguageEngine::scriptFontFamily(QChar::Script script) const
{
    if (m_scriptFontFamily.contains(script))
        return m_scriptFontFamily.value(script);

    if (m_defaultScriptFontFamily.contains(script))
        return m_defaultScriptFontFamily.value(script);

    QFontDatabase::WritingSystem writingSystem =
            scriptWritingSystemMap().value(script, QFontDatabase::Any);

    const QString safeDefault = m_defaultScriptFontFamily.value(QChar::Script_Unknown);
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
        return *it;

    return safeDefault;
}

QStringList LanguageEngine::scriptFontFamilies(QChar::Script script) const
{
    QStringList ret;

    // We could have used QSet instead of doing this circus, but we need
    // font family names to show up in a specific order.
    auto includeFontFamily = [&ret](const QString &family) {
        if (ret.isEmpty() || !ret.contains(family))
            ret.append(family);
    };

    if (m_defaultScriptFontFamily.contains(script))
        includeFontFamily(m_defaultScriptFontFamily[script]);

    const QString safeDefault = m_defaultScriptFontFamily.value(QChar::Script_Unknown);
    QFontDatabase::WritingSystem writingSystem =
            scriptWritingSystemMap().value(script, QFontDatabase::Any);
    if (writingSystem == QFontDatabase::Any || writingSystem == QFontDatabase::Latin)
        includeFontFamily(safeDefault);

    QFontDatabase &fontDb = Application::fontDatabase();

    const QStringList availableFontFamilies = fontDb.families(writingSystem);
    if (!availableFontFamilies.isEmpty()) {
        // For some reason, std::copy_if() was crashing
        for (const QString &fontFamily : availableFontFamilies) {
            if (script != QChar::Script_Latin || fontDb.isFixedPitch(fontFamily))
                includeFontFamily(fontFamily);
        }
    }

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

QChar::Script LanguageEngine::determineScript(const QString &text)
{
    for (const QChar &ch : text) {
        if (ch.script() != QChar::Script_Common && ch.script() != QChar::Script_Inherited)
            return ch.script();
    }

    return QChar::Script_Latin;
}

QList<ScriptBoundary> LanguageEngine::determineBoundaries(const QString &paragraph)
{
    if (paragraph.isEmpty())
        return {};

    /**
     * This function returns a list of boundaries where different language text-snippets can be
     * found.
     *
     * In a paragraph, if we have words in different languages - then a boundary for each is
     * created. Consequetive words in a single language are bundled into a single boundary. If a
     * single word is written using letters from multiple languages, then the entire word is assumed
     * to be in the language of the first uncommong letter found in it.
     *
     * Please note, language detection can never be accurate. We try to guess the language based on
     * their script. Multiple languages can share the same script, so we can never know for sure.
     */
    QList<ScriptBoundary> ret;

    // First, break apart each word into a separate boundary
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, paragraph);
    while (boundaryFinder.position() < paragraph.length()) {
        if (!(boundaryFinder.boundaryReasons().testFlag(QTextBoundaryFinder::StartOfItem))) {
            if (boundaryFinder.toNextBoundary() == -1)
                break;
            continue;
        }

        const int bstart = boundaryFinder.position();
        const int bend = boundaryFinder.toNextBoundary();

        ScriptBoundary item;
        item.start = bstart;
        item.end = bend < 0 ? paragraph.length() - 1 : bend;
        if (item.end - item.start < 1)
            continue;

        item.text = paragraph.mid(item.start, item.end - item.start);
        item.script = determineScript(item.text);

        ret.append(item);

        if (bend < 0)
            break;
    }

    // If no boundaries were found, then the whole paragraph is one boundary.
    if (ret.isEmpty()) {
        ScriptBoundary item;
        item.start = 0;
        item.end = paragraph.length();
        item.text = paragraph;
        item.script = determineScript(item.text);
        ret.append(item);
        return ret;
    }

    // If the first few characters were not captured in the boundary, then
    // capture them now.
    if (ret.first().start > 0) {
        ScriptBoundary first;
        first.start = 0;
        first.end = ret.first().start;
        first.text = paragraph.mid(first.start, first.end - first.start);
        first.script = determineScript(first.text);
        ret.prepend(first);
    }

    // If the last few characters were not captured in the boundary, then
    // capture them now.
    if (ret.last().end < paragraph.length()) {
        ScriptBoundary last;
        last.start = ret.last().end;
        last.end = paragraph.length();
        last.text = paragraph.mid(last.start, last.end - last.start);
        last.script = determineScript(last.text);
        ret.append(last);
    }

    // If there is only one boundary, then nothing else to do.
    if (ret.size() == 1)
        return ret;

    // Ensure that there are no missing parts inbetween boundaries.
    for (int i = 0; i < ret.size() - 1; i++) {
        ScriptBoundary &a = ret[i];
        const ScriptBoundary &b = ret[i + 1];
        if (a.end != b.start) {
            a.end = b.start;
            a.text = paragraph.mid(a.start, a.end - a.start);
        }
    }

    // Merge boundaries if they belong to the same script.
    for (int i = ret.length() - 2; i >= 0; i--) {
        const ScriptBoundary &b = ret[i + 1];
        ScriptBoundary &a = ret[i];

        if (a.script == b.script) {
            a.end = b.end;
            a.text = paragraph.mid(a.start, a.end - a.start);
            ret.removeAt(i + 1);
        }
    }

    return ret;
}

void LanguageEngine::determineBoundariesAndInsertText(QTextCursor &cursor, const QString &paragraph)
{
    const QTextCharFormat defaultFormat = cursor.charFormat();

    const QList<ScriptBoundary> items = determineBoundaries(paragraph);
    for (const ScriptBoundary &item : items) {
        QTextCharFormat format = defaultFormat;
        format.setFontFamily(LanguageEngine::instance()->scriptFontFamily(item.script));
        cursor.insertText(item.text, format);
    }
}

QString LanguageEngine::formattedInHtml(const QString &paragraph)
{
    QString html;
    QTextStream ts(&html, QIODevice::WriteOnly);

    const QList<ScriptBoundary> items = determineBoundaries(paragraph);
    for (const ScriptBoundary &item : items) {
        if (item.script == QChar::Script_Latin)
            ts << item.text;
        else {
            const QString fontFamily = LanguageEngine::instance()->scriptFontFamily(item.script);
            ts << "<font family=\"" << fontFamily << "\">" << item.text << "</font>";
        }
    }

    ts.flush();

    return html;
}

int LanguageEngine::wordCount(const QString &paragraph)
{
    int wordCount = 0;

    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, paragraph);
    while (boundaryFinder.position() < paragraph.length()) {
        QTextBoundaryFinder::BoundaryReasons reasons = boundaryFinder.boundaryReasons();
        if (!(reasons.testFlag(QTextBoundaryFinder::StartOfItem))
            || reasons.testFlag(QTextBoundaryFinder::SoftHyphen)) {
            if (boundaryFinder.toNextBoundary() == -1)
                break;
            continue;
        }

        ++wordCount;
        boundaryFinder.toNextBoundary();
    }

    return wordCount;
}

QVector<QTextLayout::FormatRange>
LanguageEngine::mergeTextFormats(const QList<ScriptBoundary> &boundaries,
                                 const QVector<QTextLayout::FormatRange> &formats)
{
    const int length = [=]() {
        int ret = -1;
        for (const ScriptBoundary &boundary : boundaries)
            ret = qMax(boundary.end, ret);
        for (const QTextLayout::FormatRange &format : formats)
            ret = qMax(format.start + format.length - 1, ret);
        return ret + 1;
    }();
    const QString dummyText = QString(length, QChar('X'));

    QTextDocument doc;

    QTextCursor cursor(&doc);
    cursor.insertText(dummyText);
    cursor.setPosition(0);

    const bool ignoreBoundaries = !boundaries.isEmpty() && boundaries.length() == 1
            && boundaries.first().script == QChar::Script_Latin;
    if (!ignoreBoundaries) {
        for (const ScriptBoundary &boundary : boundaries) {
            cursor.setPosition(boundary.start);
            cursor.setPosition(boundary.end, QTextCursor::KeepAnchor);

            QTextCharFormat charFormat;
            charFormat.setFontFamily(LanguageEngine::instance()->scriptFontFamily(boundary.script));
            charFormat.setProperty(QTextCharFormat::UserProperty, boundary.script);
            cursor.setCharFormat(charFormat);
            cursor.clearSelection();
        }
    }

    for (const QTextLayout::FormatRange &format : formats) {
        cursor.setPosition(format.start);
        cursor.setPosition(format.start + format.length, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(format.format);
        cursor.clearSelection();
    }

    cursor.setPosition(0);

    const QTextBlock block = cursor.block();
    return block.textFormats();
}

void LanguageEngine::polishFontsAndInsertTextAtCursor(
        QTextCursor &cursor, const QString &text, const QVector<QTextLayout::FormatRange> &formats)
{
    const int startPos = cursor.position();
    determineBoundariesAndInsertText(cursor, text);
    const int endPos = cursor.position();

    if (!formats.isEmpty()) {
        cursor.setPosition(startPos);
        for (const QTextLayout::FormatRange &formatRange : formats) {
            const int length = qMin(startPos + formatRange.start + formatRange.length, endPos)
                    - (startPos + formatRange.start);
            cursor.setPosition(startPos + formatRange.start);
            if (length > 0) {
                cursor.setPosition(startPos + formatRange.start + length, QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(formatRange.format);
            }
        }
        cursor.setPosition(endPos);
    }
}

void LanguageEngine::init(const char *uri, QQmlEngine *qmlEngine)
{
    Q_UNUSED(qmlEngine)

    static bool initedOnce = false;
    if (initedOnce)
        return;

    const char *reason = "Instantiation from QML not allowed.";

    // @uri io.scrite.components
    // @reason Instantiation from QML not allowed.
    qmlRegisterSingletonInstance(uri, 1, 0, "LanguageEngine", LanguageEngine::instance());
    qmlRegisterUncreatableMetaObject(QLocale::staticMetaObject, uri, 1, 0, "QtLocale", reason);
    qmlRegisterUncreatableMetaObject(QFontDatabase::staticMetaObject, uri, 1, 0, "QtFontDatabase",
                                     reason);

    initedOnce = true;
}

bool LanguageEngine::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn && (object->isWindowType() || object->isWidgetType())) {
        this->activateTransliterationOptionOnActiveLanguage();
    }

    return false;
}

void LanguageEngine::loadConfiguration()
{
    m_configFileName = QFileInfo(Application::instance()->settingsFilePath())
                               .absoluteDir()
                               .absoluteFilePath(QStringLiteral("language-engine.json"));

    QFile file(m_configFileName);
    if (file.open(QFile::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        const QJsonObject config = doc.object();

        const QJsonArray scriptFonts = config.value("script-fonts").toArray();
        if (!scriptFonts.isEmpty()) {
            const QMetaEnum scriptEnum = QMetaEnum::fromType<Language::CharScript>();

            for (const QJsonValue &scriptFontsItem : scriptFonts) {
                const QJsonObject scriptFont = scriptFontsItem.toObject();

                const QString fontFamily = scriptFont.value("font-family").toString();
                if (fontFamily.isEmpty() || !Application::fontDatabase().hasFamily(fontFamily))
                    continue;

                const int key = scriptFont.value("key").toInt();
                const char *value = scriptEnum.valueToKey(key);
                if (value != nullptr
                    && scriptFont.value("name").toString() == QString::fromLatin1(value)) {
                    m_scriptFontFamily[QChar::Script(key)] = fontFamily;
                }
            }
        }

        m_availableLanguages->initialize();
        m_supportedLanguages->initialize();

        const QJsonValue supportedLanguagesConfig = config.value("supported-languages");
        m_supportedLanguages->fromJson(supportedLanguagesConfig);
    } else {
        m_availableLanguages->initialize();
        m_supportedLanguages->initialize();
        m_supportedLanguages->fromJson(QJsonValue());
    }

    qApp->installEventFilter(this);
}

void LanguageEngine::saveConfiguration()
{
    if (!m_configFileName.isEmpty()) {
        QFile file(m_configFileName);
        if (file.open(QFile::WriteOnly)) {
            QJsonObject config;

            // Save script font associations.
            if (!m_scriptFontFamily.isEmpty()) {
                const QMetaEnum scriptEnum = QMetaEnum::fromType<Language::CharScript>();

                QJsonArray scriptFonts;

                auto it = m_scriptFontFamily.begin();
                auto end = m_scriptFontFamily.end();
                while (it != end) {
                    const char *value = scriptEnum.valueToKey(it.key());
                    if (value != nullptr) {
                        QJsonObject scriptFont;
                        scriptFont.insert("key", it.key());
                        scriptFont.insert("name", QString::fromLatin1(value));
                        scriptFont.insert("font-family", it.value());
                        scriptFonts.append(scriptFont);
                    }
                    ++it;
                }

                if (!scriptFonts.isEmpty())
                    config.insert("script-fonts", scriptFonts);
            }

            const QJsonValue supportedLanguagesConfig = m_supportedLanguages->toJson();
            if (!supportedLanguagesConfig.isNull())
                config.insert("supported-languages", supportedLanguagesConfig);

            file.write(QJsonDocument(config).toJson(QJsonDocument::Indented));
        }
    }
}

void LanguageEngine::activateTransliterationOptionOnActiveLanguage()
{
    const Language activeLanguage =
            m_supportedLanguages ? m_supportedLanguages->activeLanguage() : Language();
    if (activeLanguage.isValid()) {
        const TransliterationOption activeOption = activeLanguage.preferredTransliterationOption();
        if (activeLanguage.isValid() && !activeOption.transliteratorObject.isNull()) {
            activeOption.transliterator()->activate(activeOption);
        }
    }
}
