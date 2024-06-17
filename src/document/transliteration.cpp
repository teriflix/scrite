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

#include "user.h"
#include "hourglass.h"
#include "appwindow.h"
#include "callgraph.h"
#include "application.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "transliteration.h"
#include "spellcheckservice.h"
#include "systemtextinputmanager.h"
#include "3rdparty/sonnet/sonnet/src/core/textbreaks_p.h"

#include <QTimer>
#include <QPainter>
#include <QMetaEnum>
#include <QSettings>
#include <QTextBlock>
#include <QMetaObject>
#include <QJsonObject>
#include <QTextCursor>
#include <QQuickWindow>
#include <QTextDocument>
#include <QFontDatabase>
#include <QtConcurrentRun>
#include <QQuickTextDocument>
#include <QTextBoundaryFinder>
#include <QAbstractTextDocumentLayout>

#include <PhTranslateLib>

static QStringList getCustomFontFilePaths()
{
    const QStringList customFonts = QStringList()
            << QStringLiteral(":/font/Gujarati/HindVadodara-Regular.ttf")
            << QStringLiteral(":/font/Gujarati/HindVadodara-Bold.ttf")
            << QStringLiteral(":/font/Oriya/BalooBhaina2-Regular.ttf")
            << QStringLiteral(":/font/Oriya/BalooBhaina2-Bold.ttf")
            << QStringLiteral(":/font/Punjabi/BalooPaaji2-Regular.ttf")
            << QStringLiteral(":/font/Punjabi/BalooPaaji2-Bold.ttf")
            << QStringLiteral(":/font/Malayalam/BalooChettan2-Regular.ttf")
            << QStringLiteral(":/font/Malayalam/BalooChettan2-Bold.ttf")
            << QStringLiteral(":/font/Marathi/Shusha-Normal.ttf")
            << QStringLiteral(":/font/Hindi/Mukta-Regular.ttf")
            << QStringLiteral(":/font/Hindi/Mukta-Bold.ttf")
            << QStringLiteral(":/font/Telugu/HindGuntur-Regular.ttf")
            << QStringLiteral(":/font/Telugu/HindGuntur-Bold.ttf")
            << QStringLiteral(":/font/Sanskrit/Mukta-Regular.ttf")
            << QStringLiteral(":/font/Sanskrit/Mukta-Bold.ttf")
            << QStringLiteral(":/font/English/CourierPrime-BoldItalic.ttf")
            << QStringLiteral(":/font/English/CourierPrime-Bold.ttf")
            << QStringLiteral(":/font/English/CourierPrime-Italic.ttf")
            << QStringLiteral(":/font/English/CourierPrime-Regular.ttf")
            << QStringLiteral(":/font/Kannada/BalooTamma2-Regular.ttf")
            << QStringLiteral(":/font/Kannada/BalooTamma2-Bold.ttf")
            << QStringLiteral(":/font/Tamil/HindMadurai-Regular.ttf")
            << QStringLiteral(":/font/Tamil/HindMadurai-Bold.ttf")
            << QStringLiteral(":/font/Bengali/HindSiliguri-Regular.ttf")
            << QStringLiteral(":/font/Bengali/HindSiliguri-Bold.ttf");
    return customFonts;
}

TransliterationEngine *TransliterationEngine::instance(QCoreApplication *app)
{
    // CAPTURE_FIRST_CALL_GRAPH;
    static TransliterationEngine *newInstance = new TransliterationEngine(app ? app : qApp);
    return newInstance;
}

TransliterationEngine::TransliterationEngine(QObject *parent) : QObject(parent)
{
    // CAPTURE_CALL_GRAPH;
    const QMetaObject *mo = &TransliterationEngine::staticMetaObject;
    const QMetaEnum languageEnum = mo->enumerator(mo->indexOfEnumerator("Language"));
    const QStringList customFontPaths = ::getCustomFontFilePaths();
    for (const QString &customFont : customFontPaths) {
        const int id = QFontDatabase::addApplicationFont(customFont);
        const QString language = customFont.split("/", Qt::SkipEmptyParts).at(2);
        const QStringList appFontFamilies = QFontDatabase::applicationFontFamilies(id);
        Language lang = Language(languageEnum.keyToValue(qPrintable(language)));
        m_languageBundledFontId[lang] = id;
        m_languageFontFamily[lang] = appFontFamilies.first();
        m_languageFontFilePaths[lang].append(customFont);
    }

    const QSettings *settings = Application::instance()->settings();

    for (int i = 0; i < languageEnum.keyCount(); i++) {
        Language lang = Language(languageEnum.value(i));
        m_activeLanguages[lang] = false;

        const QString langStr = QString::fromLatin1(languageEnum.key(i));

        const QString sfontKey =
                QStringLiteral("Transliteration/") + langStr + QStringLiteral("_Font");
        const QString fontFamily = settings->value(sfontKey).toString();
        if (!fontFamily.isEmpty())
            m_languageFontFamily[lang] = fontFamily;

        const QString tisIdKey =
                QStringLiteral("Transliteration/") + langStr + QStringLiteral("_tisID");
        const QVariant tisId = settings->value(tisIdKey, QVariant());
        if (tisId.isValid())
            m_tisMap[lang] = tisId.toString();
    }

    const QStringList activeLanguages =
            settings->value("Transliteration/activeLanguages").toStringList();

    if (activeLanguages.isEmpty()) {
        m_activeLanguages[English] = true;
        m_activeLanguages[Kannada] = true;
    } else {
        for (const QString &lang : activeLanguages) {
            bool ok = false;
            int val = languageEnum.keyToValue(qPrintable(lang.trimmed()), &ok);
            m_activeLanguages[Language(val)] = true;
        }
    }

    const QString currentLanguage = settings->value("Transliteration/currentLanguage").toString();
    Language lang = English;
    if (!currentLanguage.isEmpty()) {
        bool ok = false;
        int val = languageEnum.keyToValue(qPrintable(currentLanguage), &ok);
        lang = Language(val);
    }
    this->setLanguage(lang);
}

void TransliterationEngine::setEnabledLanguages(const QList<int> &val)
{
    if (m_enabledLanguages == val)
        return;

    m_enabledLanguages = val;
    emit enabledLanguagesChanged();
}

void TransliterationEngine::determineEnabledLanguages()
{
    const QMetaObject *mo = &TransliterationEngine::staticMetaObject;
    const QMetaEnum languageEnum = mo->enumerator(mo->indexOfEnumerator("Language"));

    QQuickItem *focusItem =
            AppWindow::instance() ? AppWindow::instance()->activeFocusItem() : nullptr;
    TransliterationHints *hints = TransliterationHints::find(focusItem);
    TransliterationHints::AllowedMechanisms mechanisms = TransliterationHints::AllMechanisms;
    if (hints)
        mechanisms = hints->allowedMechanisms();

    QList<int> languages;
    if (mechanisms == TransliterationHints::NoMechanism) {
        this->setEnabledLanguages(languages);
        return;
    }

    for (int i = 0; i < languageEnum.keyCount(); i++) {
        Language lang = Language(languageEnum.value(i));
        if (lang == TransliterationEngine::English
            || mechanisms.testFlag(TransliterationHints::StaticMechanism))
            languages.append(lang);
        else if (mechanisms.testFlag(TransliterationHints::TextInputSourceMechanism)) {
            const QString tisId = m_tisMap.value(lang);
            const AbstractSystemTextInputSource *tis =
                    SystemTextInputManager::instance()->findSourceById(tisId);
            if (tis != nullptr)
                languages.append(lang);
        }
    }

    this->setEnabledLanguages(languages);

    if (!languages.contains(m_language))
        this->setLanguage(English);
}

TransliterationEngine::~TransliterationEngine() { }

void TransliterationEngine::setLanguage(TransliterationEngine::Language val)
{
    if (m_language == val)
        return;

    m_language = val;
    m_transliterator = transliteratorFor(m_language);

    QSettings *settings = Application::instance()->settings();
    const QMetaObject *mo = &TransliterationEngine::staticMetaObject;
    const QMetaEnum metaEnum = mo->enumerator(mo->indexOfEnumerator("Language"));
    const QString languageName = QString::fromLatin1(metaEnum.valueToKey(m_language));
    settings->setValue("Transliteration/currentLanguage", languageName);

    SystemTextInputManager *tisManager = SystemTextInputManager::instance();
    const QString tisId = m_tisMap.value(val);
    if (tisId.isEmpty()) {
        AbstractSystemTextInputSource *fallbackSource = tisManager->fallbackInputSource();
        if (fallbackSource)
            fallbackSource->select();
    } else {
        AbstractSystemTextInputSource *tis =
                SystemTextInputManager::instance()->findSourceById(tisId);
        if (tis != nullptr)
            tis->select();
        else
            this->setTextInputSourceIdForLanguage(m_language, QString());
    }

    emit languageChanged();

    User::instance()->logActivity2(QStringLiteral("language"), languageName);
}

QString TransliterationEngine::languageAsString() const
{
    return this->languageAsString(m_language);
}

QString TransliterationEngine::languageAsString(TransliterationEngine::Language language)
{
    const QMetaEnum metaEnum = QMetaEnum::fromType<TransliterationEngine::Language>();
    return QString::fromLatin1(metaEnum.valueToKey(language));
}

QString TransliterationEngine::shortcutLetter(TransliterationEngine::Language val) const
{
    if (val == Tamil)
        return QStringLiteral("L");
    if (val == Marathi)
        return QStringLiteral("R");

    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator(mo->indexOfEnumerator("Language"));
    return QChar(metaEnum.valueToKey(val)[0]).toUpper();
}

template<class T>
class RawArray
{
public:
    RawArray() { }
    ~RawArray() { }

    void load(const T *a, int s)
    {
        m_array = a;
        m_size = s;
    }

    const T *at(int index) const
    {
        if (index < 0 || index >= m_size)
            return nullptr;
        return &m_array[index];
    }

    int size() const { return m_size; }

    bool isEmpty() const { return m_size <= 0; }

    QJsonObject toJson(int index) const
    {
        QJsonObject ret;
        const T *item = this->at(index);
        if (item == nullptr)
            return ret;
        ret.insert("latin", QString::fromLatin1(item->phRep));
        if (sizeof(T) == sizeof(PhTranslation::VowelDef)) {
            const PhTranslation::VowelDef *vitem =
                    reinterpret_cast<const PhTranslation::VowelDef *>(item);
            ret.insert("unicode",
                       QString::fromWCharArray(&(vitem->uCode), 1) + QStringLiteral(", ")
                               + QString::fromWCharArray(&(vitem->dCode), 1));
        } else
            ret.insert("unicode", QString::fromWCharArray(&(item->uCode), 1));
        return ret;
    }

    QJsonArray toJson() const
    {
        QJsonArray ret;
        for (int i = 0; i < m_size; i++)
            ret.append(this->toJson(i));
        return ret;
    }

private:
    const T *m_array = nullptr;
    int m_size = 0;
};

QJsonObject TransliterationEngine::alphabetMappingsFor(TransliterationEngine::Language lang) const
{
    static QMap<Language, QJsonObject> alphabetMappings;

    QJsonObject ret = alphabetMappings.value(lang, QJsonObject());
    if (!ret.isEmpty())
        return ret;

    RawArray<PhTranslation::VowelDef> vowels;
    RawArray<PhTranslation::ConsonantDef> consonants;
    RawArray<PhTranslation::DigitDef> digits;
    RawArray<PhTranslation::SpecialSymbolDef> symbols;

#define NUMBER_OF_ITEMS_IN(x) (sizeof(x) / sizeof(x[0]))
#define LOAD_ARRAYS(x)                                                                             \
    {                                                                                              \
        vowels.load(PhTranslation::x::Vowels, NUMBER_OF_ITEMS_IN(PhTranslation::x::Vowels));       \
        consonants.load(PhTranslation::x::Consonants,                                              \
                        NUMBER_OF_ITEMS_IN(PhTranslation::x::Consonants));                         \
        digits.load(PhTranslation::x::Digits, NUMBER_OF_ITEMS_IN(PhTranslation::x::Digits));       \
        symbols.load(PhTranslation::x::SpecialSymbols,                                             \
                     NUMBER_OF_ITEMS_IN(PhTranslation::x::SpecialSymbols));                        \
    }

    switch (lang) {
    case English:
        return ret;
    case Bengali:
        LOAD_ARRAYS(Bengali)
        break;
    case Gujarati:
        LOAD_ARRAYS(Gujarati)
        break;
    case Hindi:
        LOAD_ARRAYS(Hindi)
        break;
    case Kannada:
        LOAD_ARRAYS(Kannada)
        break;
    case Malayalam:
        LOAD_ARRAYS(Malayalam)
        break;
    case Marathi:
        LOAD_ARRAYS(Hindi)
        break;
    case Oriya:
        LOAD_ARRAYS(Oriya)
        break;
    case Punjabi:
        LOAD_ARRAYS(Punjabi)
        break;
    case Sanskrit:
        LOAD_ARRAYS(Sanskrit)
        break;
    case Tamil:
        LOAD_ARRAYS(Tamil)
        break;
    case Telugu:
        LOAD_ARRAYS(Telugu)
        break;
    }

#undef LOAD_ARRAYS
#undef NUMBER_OF_ITEMS_IN

    ret.insert("vowels", vowels.toJson());
    ret.insert("consonants", consonants.toJson());
    ret.insert("digits", digits.toJson());
    ret.insert("symbols", symbols.toJson());

    alphabetMappings.insert(lang, ret);

    return ret;
}

void TransliterationEngine::cycleLanguage()
{
    QMap<Language, bool>::const_iterator it = m_activeLanguages.constFind(m_language);
    QMap<Language, bool>::const_iterator end = m_activeLanguages.constEnd();

    ++it;
    while (it != end) {
        if (it.value()) {
            this->setLanguage(it.key());
            return;
        }

        ++it;
    }

    it = m_activeLanguages.constBegin();
    while (it != end) {
        if (it.value()) {
            this->setLanguage(it.key());
            return;
        }

        ++it;
    }
}

void TransliterationEngine::markLanguage(TransliterationEngine::Language language, bool active)
{
    if (m_activeLanguages.value(language, false) == active)
        return;

    m_activeLanguages[language] = active;

    QSettings *settings = Application::instance()->settings();
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator(mo->indexOfEnumerator("Language"));
    QStringList activeLanguages;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        Language lang = Language(metaEnum.value(i));
        if (m_activeLanguages.value(lang))
            activeLanguages.append(QString::fromLatin1(metaEnum.valueToKey(lang)));
    }
    settings->setValue("Transliteration/activeLanguages", activeLanguages);

    emit languagesChanged();
}

bool TransliterationEngine::queryLanguage(TransliterationEngine::Language language) const
{
    return m_activeLanguages.value(language, false);
}

QJsonArray TransliterationEngine::languages() const
{
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator(mo->indexOfEnumerator("Language"));
    QJsonArray ret;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        QJsonObject item;
        item.insert("key", QString::fromLatin1(metaEnum.key(i)));
        item.insert("value", metaEnum.value(i));
        item.insert("active", m_activeLanguages.value(Language(metaEnum.value(i))));
        item.insert("current", m_language == metaEnum.value(i));
        ret.append(item);
    }

    return ret;
}

void *TransliterationEngine::transliterator() const
{
    const QString tisId = m_tisMap.value(m_language);
    return tisId.isEmpty() ? m_transliterator : nullptr;
}

void *TransliterationEngine::transliteratorFor(TransliterationEngine::Language language)
{
    switch (language) {
    case English:
        return nullptr;
    case Bengali:
        return GetBengaliTranslator();
    case Gujarati:
        return GetGujaratiTranslator();
    case Hindi:
        return GetHindiTranslator();
    case Kannada:
        return GetKannadaTranslator();
    case Malayalam:
        return GetMalayalamTranslator();
    case Marathi:
        return GetMarathiTranslator();
    case Oriya:
        return GetOriyaTranslator();
    case Punjabi:
        return GetPunjabiTranslator();
    case Sanskrit:
        return GetSanskritTranslator();
    case Tamil:
        return GetTamilTranslator();
    case Telugu:
        return GetTeluguTranslator();
    }

    return nullptr;
}

TransliterationEngine::Language TransliterationEngine::languageOf(void *transliterator)
{
#define CHECK(x)                                                                                   \
    if (transliterator == transliteratorFor(TransliterationEngine::x))                             \
    return TransliterationEngine::x
    CHECK(English);
    CHECK(Bengali);
    CHECK(Gujarati);
    CHECK(Hindi);
    CHECK(Kannada);
    CHECK(Malayalam);
    CHECK(Oriya);
    CHECK(Punjabi);
    CHECK(Sanskrit);
    CHECK(Tamil);
    CHECK(Telugu);
#undef CHECK

    return English;
}

void TransliterationEngine::setTextInputSourceIdForLanguage(
        TransliterationEngine::Language language, const QString &id)
{
    if (m_tisMap.value(language) == id)
        return;

    m_tisMap[language] = id;

    QSettings *settings = Application::instance()->settings();
    settings->setValue(QStringLiteral("Transliteration/") + languageAsString(language)
                               + QStringLiteral("_tisID"),
                       id);

    if (m_language == language) {
        SystemTextInputManager *tisManager = SystemTextInputManager::instance();
        AbstractSystemTextInputSource *source =
                id.isEmpty() ? nullptr : tisManager->findSourceById(id);
        if (source)
            source->select();
        else {
            AbstractSystemTextInputSource *fallbackSource = tisManager->fallbackInputSource();
            if (fallbackSource)
                fallbackSource->select();
        }
    }
}

QString
TransliterationEngine::textInputSourceIdForLanguage(TransliterationEngine::Language language) const
{
    return m_tisMap.value(language);
}

QJsonObject TransliterationEngine::languageTextInputSourceMap() const
{
    QJsonObject ret;
    QMap<Language, QString>::const_iterator it = m_tisMap.constBegin();
    QMap<Language, QString>::const_iterator end = m_tisMap.constEnd();
    while (it != end) {
        ret.insert(this->languageAsString(it.key()), it.value());
        ++it;
    }

    return ret;
}

QString TransliterationEngine::transliteratedWord(const QString &word) const
{
    return TransliterationEngine::transliteratedWord(word, m_transliterator);
}

QString TransliterationEngine::transliteratedParagraph(const QString &paragraph,
                                                       bool includingLastWord) const
{
    return TransliterationEngine::transliteratedParagraph(paragraph, m_transliterator,
                                                          includingLastWord);
}

QString
TransliterationEngine::transliteratedWordInLanguage(const QString &word,
                                                    TransliterationEngine::Language language) const
{
    return transliteratedWord(word, transliteratorFor(language));
}

QString TransliterationEngine::transliteratedWord(const QString &word, void *transliterator)
{
    if (transliterator == nullptr)
        return word;

    Language language = languageOf(transliterator);
    const QString tisId = TransliterationEngine::instance()->textInputSourceIdForLanguage(language);
    if (tisId.isEmpty())
        return QString::fromStdWString(Translate(transliterator, word.toStdWString().c_str()));

    return word;
}

QString TransliterationEngine::transliteratedParagraph(const QString &paragraph,
                                                       void *transliterator, bool includingLastWord)
{
    if (transliterator == nullptr || paragraph.isEmpty())
        return paragraph;

    const Sonnet::TextBreaks::Positions wordPositions = Sonnet::TextBreaks::wordBreaks(paragraph);
    if (wordPositions.isEmpty())
        return paragraph;

    const QChar lastCharacter = paragraph.at(paragraph.length() - 1);
    const bool endsWithSpaceOrPunctuation =
            lastCharacter.isSpace() || lastCharacter.isPunct() || lastCharacter.isDigit();
    if (endsWithSpaceOrPunctuation)
        includingLastWord = true;

    QString ret;
    Sonnet::TextBreaks::Position wordPosition;
    int lastCharIndex = -1;
    for (int i = 0; i < wordPositions.size(); i++) {
        ++lastCharIndex;

        wordPosition = wordPositions.at(i);
        if (wordPosition.start > lastCharIndex)
            ret += paragraph.midRef(lastCharIndex, wordPosition.start - lastCharIndex);

        const QString word = paragraph.mid(wordPosition.start, wordPosition.length);
        lastCharIndex = wordPosition.start + wordPosition.length - 1;
        QString replacement;

        if (i < wordPositions.length() - 1 || includingLastWord)
            replacement = transliteratedWord(word, transliterator);
        else
            replacement = word;

        ret += replacement;
    }

    ret += paragraph.midRef(lastCharIndex + 1);

    return ret;
}

QFont TransliterationEngine::languageFont(TransliterationEngine::Language language,
                                          bool preferAppFonts) const
{
    const QFontDatabase &fontDb = ::Application::fontDatabase();
    const QString preferredFontFamily = m_languageFontFamily.value(language);

    QString fontFamily = preferAppFonts ? preferredFontFamily : QString();
    if (fontFamily.isEmpty()) {
        const QStringList languageFontFamilies =
                fontDb.families(writingSystemForLanguage(language));
        fontFamily = languageFontFamilies.first();
    }

    return fontFamily.isEmpty() ? Application::instance()->font() : QFont(fontFamily);
}

QStringList
TransliterationEngine::languageFontFilePaths(TransliterationEngine::Language language) const
{
    return m_languageFontFilePaths.value(language, QStringList());
}

QJsonObject
TransliterationEngine::availableLanguageFontFamilies(TransliterationEngine::Language language) const
{
    QJsonObject ret;

    const QString preferredFontFamily = m_languageFontFamily.value(language);
    QStringList filteredLanguageFontFamilies = m_availableLanguageFontFamilies.value(language);

    if (filteredLanguageFontFamilies.isEmpty()) {
        HourGlass hourGlass;

        const QFontDatabase &fontDb = ::Application::fontDatabase();
        const QStringList languageFontFamilies =
                fontDb.families(writingSystemForLanguage(language));
        std::copy_if(languageFontFamilies.begin(), languageFontFamilies.end(),
                     std::back_inserter(filteredLanguageFontFamilies),
                     [fontDb, language](const QString &family) {
                         return fontDb.isPrivateFamily(family)
                                 ? false
                                 : (language == TransliterationEngine::English
                                            ? fontDb.isFixedPitch(family)
                                            : true);
                     });

        const int builtInFontId = m_languageBundledFontId.value(language);
        if (builtInFontId >= 0) {
            const QString builtInFont =
                    QFontDatabase::applicationFontFamilies(builtInFontId).first();
            filteredLanguageFontFamilies.removeOne(builtInFont);
            filteredLanguageFontFamilies.append(builtInFont);
            std::sort(filteredLanguageFontFamilies.begin(), filteredLanguageFontFamilies.end());
        }

        m_availableLanguageFontFamilies[language] = filteredLanguageFontFamilies;
    }

    if (!filteredLanguageFontFamilies.contains(preferredFontFamily))
        filteredLanguageFontFamilies.append(preferredFontFamily);

    ret.insert("families", QJsonArray::fromStringList(filteredLanguageFontFamilies));
    ret.insert("preferredFamily", preferredFontFamily);
    ret.insert("preferredFamilyIndex", filteredLanguageFontFamilies.indexOf(preferredFontFamily));
    return ret;
}

QString
TransliterationEngine::preferredFontFamilyForLanguage(TransliterationEngine::Language language)
{
    return m_languageFontFamily.value(language);
}

void TransliterationEngine::setPreferredFontFamilyForLanguage(
        TransliterationEngine::Language language, const QString &fontFamily)
{
    const QString before = m_languageFontFamily.value(language);

    const int builtInFontId = m_languageBundledFontId.value(language);
    const QStringList appFontFamilies = QFontDatabase::applicationFontFamilies(builtInFontId);
    const QString builtInFontFamily = builtInFontId < 0 ? QString() : appFontFamilies.first();
    if (fontFamily.isEmpty()
        || (!fontFamily.isEmpty() && !builtInFontFamily.isEmpty()
            && fontFamily == builtInFontFamily))
        m_languageFontFamily[language] = builtInFontFamily;
    else {
#if 0
        const QFontDatabase &fontDb = ::Application::fontDatabase();
        const QList<QFontDatabase::WritingSystem> writingSystems =
                fontDb.writingSystems(fontFamily);
        if (writingSystems.contains(writingSystemForLanguage(language)))
#endif
        m_languageFontFamily[language] = fontFamily;
    }

    const QString after = m_languageFontFamily.value(language);
    if (before != after) {
        QSettings *settings = Application::instance()->settings();
        settings->setValue(QStringLiteral("Transliteration/") + languageAsString(language)
                                   + QStringLiteral("_Font"),
                           after);
        emit preferredFontFamilyForLanguageChanged(language, after);
    }
}

TransliterationEngine::Language TransliterationEngine::languageForScript(QChar::Script script)
{
    static QMap<QChar::Script, Language> scriptLanguageMap;
    if (scriptLanguageMap.isEmpty()) {
        scriptLanguageMap[QChar::Script_Latin] = English;
        scriptLanguageMap[QChar::Script_Devanagari] = Hindi;
        scriptLanguageMap[QChar::Script_Bengali] = Bengali;
        scriptLanguageMap[QChar::Script_Gurmukhi] = Punjabi;
        scriptLanguageMap[QChar::Script_Gujarati] = Gujarati;
        scriptLanguageMap[QChar::Script_Oriya] = Oriya;
        scriptLanguageMap[QChar::Script_Tamil] = Tamil;
        scriptLanguageMap[QChar::Script_Telugu] = Telugu;
        scriptLanguageMap[QChar::Script_Kannada] = Kannada;
        scriptLanguageMap[QChar::Script_Malayalam] = Malayalam;
    }

    return scriptLanguageMap.value(script, English);
}

QChar::Script TransliterationEngine::scriptForLanguage(Language language)
{
    static QMap<Language, QChar::Script> languageScriptMap;
    if (languageScriptMap.isEmpty()) {
        languageScriptMap[English] = QChar::Script_Latin;
        languageScriptMap[Hindi] = QChar::Script_Devanagari;
        languageScriptMap[Marathi] = QChar::Script_Devanagari;
        languageScriptMap[Sanskrit] = QChar::Script_Devanagari;
        languageScriptMap[Bengali] = QChar::Script_Bengali;
        languageScriptMap[Punjabi] = QChar::Script_Gurmukhi;
        languageScriptMap[Gujarati] = QChar::Script_Gujarati;
        languageScriptMap[Oriya] = QChar::Script_Oriya;
        languageScriptMap[Tamil] = QChar::Script_Tamil;
        languageScriptMap[Telugu] = QChar::Script_Telugu;
        languageScriptMap[Kannada] = QChar::Script_Kannada;
        languageScriptMap[Malayalam] = QChar::Script_Malayalam;
    }

    return languageScriptMap.value(language, QChar::Script_Latin);
}

QFontDatabase::WritingSystem
TransliterationEngine::writingSystemForLanguage(TransliterationEngine::Language language)
{
    static QMap<Language, QFontDatabase::WritingSystem> languageWritingSystemMap;
    if (languageWritingSystemMap.isEmpty()) {
        languageWritingSystemMap[English] = QFontDatabase::Latin;
        languageWritingSystemMap[Bengali] = QFontDatabase::Bengali;
        languageWritingSystemMap[Gujarati] = QFontDatabase::Gujarati;
        languageWritingSystemMap[Hindi] = QFontDatabase::Devanagari;
        languageWritingSystemMap[Kannada] = QFontDatabase::Kannada;
        languageWritingSystemMap[Malayalam] = QFontDatabase::Malayalam;
        languageWritingSystemMap[Marathi] = QFontDatabase::Devanagari;
        languageWritingSystemMap[Oriya] = QFontDatabase::Oriya;
        languageWritingSystemMap[Punjabi] = QFontDatabase::Gurmukhi;
        languageWritingSystemMap[Sanskrit] = QFontDatabase::Devanagari;
        languageWritingSystemMap[Tamil] = QFontDatabase::Tamil;
        languageWritingSystemMap[Telugu] = QFontDatabase::Telugu;
    }

    return languageWritingSystemMap.value(language);
}

void TransliterationEngine::Boundary::evalStringLanguageAndFont(const QString &sourceText)
{
    if (this->isEmpty())
        return;

    this->string = sourceText.mid(this->start, (this->end - this->start + 1));
    this->language = TransliterationEngine::instance()->languageForScript(
            TransliterationEngine::determineScript(this->string));
    this->font = TransliterationEngine::instance()->languageFont(this->language);
}

void TransliterationEngine::Boundary::append(const QChar &ch, int pos)
{
    string += ch;
    if (start < 0)
        start = pos;
    end = pos;
}

bool TransliterationEngine::Boundary::isEmpty() const
{
    return end < 0 || start < 0 || (end - start + 1) == 0;
}

QList<TransliterationEngine::Boundary>
TransliterationEngine::evaluateBoundaries(const QString &text,
                                          bool /*bundleCommonScriptChars*/) const
{
    QList<Boundary> ret;
    if (text.isEmpty())
        return ret;

    // Create a boundary item for each word found in the given text
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, text);
    while (boundaryFinder.position() < text.length()) {
        if (!(boundaryFinder.boundaryReasons().testFlag(QTextBoundaryFinder::StartOfItem))) {
            if (boundaryFinder.toNextBoundary() == -1)
                break;
            continue;
        }

        Boundary item;
        item.start = boundaryFinder.position();
        item.end = boundaryFinder.toNextBoundary();
        if (item.end < 0)
            item.end = text.length() - 1;

        if (item.isEmpty())
            continue;

        item.evalStringLanguageAndFont(text);
        ret.append(item);
    }

    // If no boundaries were found, then the whole text is one boundary.
    if (ret.isEmpty()) {
        Boundary item;
        item.start = 0;
        item.end = text.length() - 1;
        if (!item.isEmpty()) {
            item.evalStringLanguageAndFont(text);
            ret.append(item);
        }

        return ret;
    }

    // If the first few characters were not captured in the boundary, then
    // capture them now.
    if (ret.first().start > 0) {
        Boundary firstItem;
        firstItem.start = 0;
        firstItem.end = ret.first().start - 1;
        if (!firstItem.isEmpty()) {
            firstItem.evalStringLanguageAndFont(text);
            ret.prepend(firstItem);
        }
    }

    // If the last few characters were not captured in the boundary, then
    // capture them now.
    if (ret.last().end < text.length() - 1) {
        Boundary lastItem;
        lastItem.start = ret.last().end + 1;
        lastItem.end = text.length() - 1;
        if (!lastItem.isEmpty()) {
            lastItem.evalStringLanguageAndFont(text);
            ret.append(lastItem);
        }
    }

    // Merge boundaries if they belong to the same language.
    if (ret.size() >= 2) {
        for (int i = ret.size() - 2; i >= 0; i--) {
            const Boundary left = ret.at(i);
            const Boundary right = ret.at(i + 1);
            if (left.end + 1 != right.start) {
                Boundary inbetween;
                inbetween.start = left.end + 1;
                inbetween.end = right.start - 1;
                inbetween.evalStringLanguageAndFont(text);
                ret.insert(i + 1, inbetween);
            }
        }

        for (int i = ret.size() - 2; i >= 0; i--) {
            Boundary &left = ret[i];
            const Boundary right = ret.at(i + 1);
            if (left.language == right.language) {
                left.end = right.end;
                left.string += right.string;
                ret.removeAt(i + 1);
            }
        }
    }

    for (int i = ret.size() - 1; i >= 0; i--) {
        Boundary &b = ret[i];
        const QSet<QChar::Script> scripts = [](const QString &text) -> QSet<QChar::Script> {
            QSet<QChar::Script> ret;
            for (const QChar &ch : text) {
                if (ch.script() != QChar::Script_Common)
                    ret += ch.script();
            }
            return ret;
        }(b.string);

        if (scripts.size() <= 1 || b.string.length() <= 1)
            continue;

        QChar::Script script = TransliterationEngine::scriptForLanguage(b.language);
        for (int j = b.end; j >= b.start; j--) {
            const QChar ch = b.string.at(j - b.start);
            if (ch.script() == QChar::Script_Common || ch.script() == QChar::Script_Inherited)
                continue;

            if (ch.script() != script) {
                Boundary b2;
                b2.start = j + 1;
                b2.end = b.end;
                b2.evalStringLanguageAndFont(text);
                ret.insert(i + 1, b2);
                b.end = j;
                b.evalStringLanguageAndFont(text);
                script = ch.script();
            }
        }
    }

    return ret;
}

void TransliterationEngine::evaluateBoundariesAndInsertText(QTextCursor &cursor,
                                                            const QString &text) const
{
#if 0
    const int beginPosition = cursor.position();
    cursor.insertText(text);
    const int endPosition = cursor.position();

    const QList<TransliterationEngine::Boundary> items = this->evaluateBoundaries(text);
    for (const TransliterationEngine::Boundary &item : items) {
        if (item.isEmpty())
            continue;

        cursor.setPosition(beginPosition + item.start);
        cursor.setPosition(beginPosition + item.end, QTextCursor::KeepAnchor);

        QTextCharFormat format;
        format.setFontFamily(item.font.family());
        cursor.mergeCharFormat(format);
        cursor.clearSelection();
    }

    cursor.setPosition(endPosition);
#else
    const QTextCharFormat defaultFormat = cursor.charFormat();

    const QList<TransliterationEngine::Boundary> items = this->evaluateBoundaries(text);
    for (const TransliterationEngine::Boundary &item : items) {
        if (item.isEmpty())
            continue;

        QTextCharFormat format = defaultFormat;
        if (item.language == TransliterationEngine::English)
            format.setFontFamily(cursor.document()->defaultFont().family());
        else
            format.setFontFamily(item.font.family());
        cursor.insertText(item.string, format);
    }
#endif
}

QChar::Script TransliterationEngine::determineScript(const QString &val)
{
    for (int i = 0; i < val.length(); i++) {
        const QChar ch = val.at(i);
        if (ch.script() == QChar::Script_Common || ch.script() == QChar::Script_Inherited)
            continue;
        return ch.script();
    }

    return QChar::Script_Latin;
}

QString TransliterationEngine::formattedHtmlOf(const QString &text) const
{
    QString html;
    QTextStream ts(&html, QIODevice::WriteOnly);

    const QList<TransliterationEngine::Boundary> breakup =
            TransliterationEngine::instance()->evaluateBoundaries(text);
    for (const TransliterationEngine::Boundary &item : breakup) {
        if (item.language == TransliterationEngine::English)
            ts << item.string;
        else
            ts << "<font family=\"" << item.font.family() << "\">" << item.string << "</font>";
    }

    ts.flush();

    return html;
}

int TransliterationEngine::wordCount(const QString &text)
{
    int wordCount = 0;

    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, text);
    while (boundaryFinder.position() < text.length()) {
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
TransliterationEngine::mergeTextFormats(const QList<Boundary> &boundaries,
                                        const QVector<QTextLayout::FormatRange> &formats)
{
    const int length = [=]() {
        int ret = -1;
        for (const Boundary &boundary : boundaries)
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
            && boundaries.first().language == TransliterationEngine::English;
    if (!ignoreBoundaries) {
        for (const Boundary &boundary : boundaries) {
            cursor.setPosition(boundary.start);
            cursor.setPosition(boundary.end + 1, QTextCursor::KeepAnchor);

            QTextCharFormat charFormat;
            charFormat.setFontFamily(boundary.font.family());
            charFormat.setProperty(QTextCharFormat::UserProperty, boundary.language);
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

///////////////////////////////////////////////////////////////////////////////

Transliterator::Transliterator(QObject *parent)
    : QObject(parent), m_textDocument(this, "textDocument")
{
    if (parent->inherits("QQuickTextEdit"))
        this->syncDefaultFontWithParent();
}

Transliterator::~Transliterator()
{
    this->setTextDocument(nullptr);
}

Transliterator *Transliterator::qmlAttachedProperties(QObject *object)
{
    return new Transliterator(object);
}

void Transliterator::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void Transliterator::setTextDocument(QQuickTextDocument *val)
{
    if (m_textDocument == val)
        return;

    if (m_textDocument != nullptr) {
        QTextDocument *doc = m_textDocument->textDocument();

        if (doc != nullptr) {
            doc->setUndoRedoEnabled(true);
            disconnect(doc, &QTextDocument::contentsChange, this,
                       &Transliterator::processTransliteration);
        }

        if (!m_highlighter.isNull())
            m_highlighter->setDocument(nullptr);

        this->setCursorPosition(-1);
        this->setHasActiveFocus(false);
    }

    m_textDocument = val;

    if (m_textDocument != nullptr) {
        QTextDocument *doc = m_textDocument->textDocument();

        if (doc != nullptr) {
            doc->setUndoRedoEnabled(m_textDocumentUndoRedoEnabled);
            connect(doc, &QTextDocument::contentsChange, this,
                    &Transliterator::processTransliteration);
            if (!m_highlighter.isNull())
                m_highlighter->setDocument(doc);
        }
    }

    emit textDocumentChanged();
}

void Transliterator::setTextDocumentUndoRedoEnabled(bool val)
{
    if (m_textDocumentUndoRedoEnabled == val)
        return;

    m_textDocumentUndoRedoEnabled = val;

    if (m_textDocument != nullptr) {
        QTextDocument *doc = m_textDocument->textDocument();
        if (doc != nullptr)
            doc->setUndoRedoEnabled(val);
    }

    emit textDocumentUndoRedoEnabledChanged();
}

void Transliterator::setCursorPosition(int val)
{
    if (m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    emit cursorPositionChanged();

    if (!m_highlighter.isNull()) {
        SpellCheckSyntaxHighlighterDelegate *spellCheckHighlighter =
                m_highlighter->findChild<SpellCheckSyntaxHighlighterDelegate *>();
        if (spellCheckHighlighter)
            spellCheckHighlighter->setCursorPosition(m_cursorPosition);
    }
}

void Transliterator::setHasActiveFocus(bool val)
{
    if (m_hasActiveFocus == val)
        return;

    m_hasActiveFocus = val;

    if (!m_hasActiveFocus) {
        if (m_enabled)
            this->transliterateLastWord();
        this->setCursorPosition(-1);
    }

    emit hasActiveFocusChanged();
}

void Transliterator::setApplyLanguageFonts(bool val)
{
    if (m_applyLanguageFonts == val)
        return;

    m_applyLanguageFonts = val;

    if (m_applyLanguageFonts) {
        this->createSyntaxHighlighter();
    } else {
        m_highlighter->setDocument(nullptr);
        delete m_highlighter;
    }

    emit applyLanguageFontsChanged();
    emit highlighterChanged();
}

void Transliterator::setDefaultFont(const QFont &val)
{
    if (m_defaultFont == val)
        return;

    m_defaultFont = val;
    emit defaultFontChanged();

    if (!m_highlighter.isNull()) {
        LanguageFontSyntaxHighlighterDelegate *fontHighlighter =
                m_highlighter->findChild<LanguageFontSyntaxHighlighterDelegate *>();
        fontHighlighter->setDefaultFont(m_defaultFont);
    }

    disconnect(this->parent(), SIGNAL(fontChanged(QFont)), this, SLOT(syncDefaultFontWithParent()));
}

void Transliterator::setEnforeDefaultFont(bool val)
{
    if (m_enforeDefaultFont == val)
        return;

    m_enforeDefaultFont = val;

    if (!m_highlighter.isNull()) {
        LanguageFontSyntaxHighlighterDelegate *fontHighlighter =
                m_highlighter->findChild<LanguageFontSyntaxHighlighterDelegate *>();
        if (fontHighlighter)
            fontHighlighter->setEnforceDefaultFont(val);
    }

    emit enforeDefaultFontChanged();
}

void Transliterator::setEnforceHeadingFontSize(bool val)
{
    if (m_enforceHeadingFontSize == val)
        return;

    m_enforceHeadingFontSize = val;

    if (!m_highlighter.isNull()) {
        HeadingFontSyntaxHighlighterDelegate *headingHighlighter =
                m_highlighter->findChild<HeadingFontSyntaxHighlighterDelegate *>();
        if (headingHighlighter)
            headingHighlighter->setEnabled(val);
    }

    emit enforceHeadingFontSizeChanged();
}

void Transliterator::setSpellCheckEnabled(bool val)
{
    if (m_spellCheckEnabled == val)
        return;

    m_spellCheckEnabled = val;

    if (!m_highlighter.isNull()) {
        SpellCheckSyntaxHighlighterDelegate *spellCheckHighlighter =
                m_highlighter->findChild<SpellCheckSyntaxHighlighterDelegate *>();
        if (spellCheckHighlighter)
            spellCheckHighlighter->setEnabled(val);
    }

    emit spellCheckEnabledChanged();
}

void Transliterator::setTransliterateCurrentWordOnly(bool val)
{
    if (m_transliterateCurrentWordOnly == val)
        return;

    m_transliterateCurrentWordOnly = val;
    emit transliterateCurrentWordOnlyChanged();
}

void Transliterator::setMode(Transliterator::Mode val)
{
    if (m_mode == val)
        return;

    m_mode = val;
    emit modeChanged();
}

void Transliterator::transliterateLastWord()
{
    if (this->document() != nullptr && m_cursorPosition >= 0) {
        // Transliterate the last word
        QTextCursor cursor(this->document());
        cursor.setPosition(m_cursorPosition);
        cursor.select(QTextCursor::WordUnderCursor);
        this->transliterate(cursor);
    }
}

void Transliterator::transliterate(int from, int to)
{
    if (this->document() == nullptr || to - from <= 0)
        return;

    void *transliterator = TransliterationEngine::instance()->transliterator();
    if (transliterator == nullptr)
        return;

    QTextCursor cursor(this->document());
    cursor.setPosition(from);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, to - from);
    this->transliterate(cursor);
}

void Transliterator::transliterateToLanguage(int from, int to, int language)
{
    if (this->document() == nullptr || to - from <= 0)
        return;

    void *transliterator = TransliterationEngine::instance()->transliteratorFor(
            TransliterationEngine::Language(language));
    if (transliterator == nullptr)
        return;

    const QTextBlock fromBlock = this->document()->findBlock(from);
    const QTextBlock toBlock = this->document()->findBlock(to);
    if (!fromBlock.isValid() && !toBlock.isValid())
        return;

    QTextCursor cursor(this->document());
    QTextBlock block = fromBlock;
    while (block.isValid()) {
        cursor.setPosition(qMax(from, block.position()));
        cursor.setPosition(qMin(to, block.position() + block.length() - 1),
                           QTextCursor::KeepAnchor);
        this->transliterate(cursor, transliterator, true);

        if (block == toBlock)
            break;
    }
}

QTextDocument *Transliterator::document() const
{
    return m_textDocument ? m_textDocument->textDocument() : nullptr;
}

void Transliterator::resetTextDocument()
{
    m_textDocument = nullptr;
    emit textDocumentChanged();
}

void Transliterator::processTransliteration(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved)
    if (this->document() == nullptr || !m_hasActiveFocus || !m_enabled || charsAdded == 0)
        return;

    if (m_enableFromNextWord == true) {
        m_enableFromNextWord = false;
        return;
    }

    void *transliterator = TransliterationEngine::instance()->transliterator();
    if (transliterator == nullptr)
        return;

    QTextCursor cursor(this->document());
    cursor.setPosition(from);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, charsAdded);

    const QString original = cursor.selectedText();
    if (original.isEmpty())
        return;

    if (charsAdded == 1) {
        const QChar ch = original.at(original.length() - 1);
        if (!(ch.isLetter() || ch.isDigit())) {
            // Select the word that is just before the cursor.
            cursor.setPosition(from);
            cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor, 1);
            // cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor, 1);
            cursor.setPosition(from, QTextCursor::KeepAnchor);
            this->transliterate(cursor);
            return;
        }
    } else if (!m_transliterateCurrentWordOnly) {
        // Transliterate all the words that was just added.
        this->transliterate(cursor);
    }
}

void Transliterator::transliterate(QTextCursor &cursor, void *transliterator, bool force)
{
    if (this->document() == nullptr || cursor.document() != this->document())
        return;

    if (!force && (!m_hasActiveFocus || !m_enabled))
        return;

    if (transliterator == nullptr)
        transliterator = TransliterationEngine::instance()->transliterator();
    if (transliterator == nullptr)
        return;

    const QString original = cursor.selectedText();
    const QString replacement =
            TransliterationEngine::transliteratedParagraph(original, transliterator, true);

    if (replacement == original)
        return;

    if (m_mode == AutomaticMode) {
        const int start = cursor.selectionStart();

        emit aboutToTransliterate(cursor.selectionStart(), cursor.selectionEnd(), replacement,
                                  original);
        cursor.insertText(replacement);
        emit finishedTransliterating(cursor.selectionStart(), cursor.selectionEnd(), replacement,
                                     original);

        const int end = cursor.position();

        qApp->postEvent(cursor.document(),
                        new TransliterationEvent(start, end, original,
                                                 TransliterationEngine::instance()->language(),
                                                 replacement));
    } else
        emit transliterationSuggestion(cursor.selectionStart(), cursor.selectionEnd(), replacement,
                                       original);
}

void Transliterator::createSyntaxHighlighter()
{
    if (m_highlighter.isNull())
        m_highlighter = new SyntaxHighlighter(this);

    m_highlighter->setTextDocument(this->textDocument());

    LanguageFontSyntaxHighlighterDelegate *fontHighlighter =
            new LanguageFontSyntaxHighlighterDelegate(m_highlighter);
    fontHighlighter->setDefaultFont(m_defaultFont);
    fontHighlighter->setEnforceDefaultFont(m_enforeDefaultFont);
    m_highlighter->addDelegate(fontHighlighter);

    HeadingFontSyntaxHighlighterDelegate *headingHighlighter =
            new HeadingFontSyntaxHighlighterDelegate(m_highlighter);
    headingHighlighter->setEnabled(m_enforceHeadingFontSize);
    headingHighlighter->initializeWithNormalFontAs(m_defaultFont);
    m_highlighter->addDelegate(headingHighlighter);

    SpellCheckSyntaxHighlighterDelegate *spellCheckHighlighter =
            new SpellCheckSyntaxHighlighterDelegate(m_highlighter);
    spellCheckHighlighter->setEnabled(m_spellCheckEnabled);
    spellCheckHighlighter->setCursorPosition(m_cursorPosition);
    m_highlighter->addDelegate(spellCheckHighlighter);
}

void Transliterator::syncDefaultFontWithParent()
{
    if (!this->parent() || !this->parent()->inherits("QQuickTextEdit"))
        return;

    const QFont font = this->parent()->property("font").value<QFont>();
    this->setDefaultFont(font);

    // setDefaultFont() would have disconnected the following singal, so we
    // reconnect it now.
    connect(this->parent(), SIGNAL(fontChanged(QFont)), this, SLOT(syncDefaultFontWithParent()));
}

QEvent::Type TransliterationEvent::EventType()
{
    static const int type = QEvent::registerEventType();
    return QEvent::Type(type);
}

TransliterationEvent::TransliterationEvent(int start, int end, const QString &original,
                                           TransliterationEngine::Language lang,
                                           const QString &replacement)
    : QEvent(EventType()),
      m_start(start),
      m_end(end),
      m_original(original),
      m_replacement(replacement),
      m_language(lang)
{
}

TransliterationEvent::~TransliterationEvent() { }

///////////////////////////////////////////////////////////////////////////////

TransliteratedText::TransliteratedText(QQuickItem *parent)
    : QQuickPaintedItem(parent), m_updateTimer("TransliteratedText.m_updateTimer")
{
}

TransliteratedText::~TransliteratedText()
{
    m_updateTimer.stop();
}

void TransliteratedText::setText(const QString &val)
{
    if (m_text == val)
        return;

    m_text = val;
    emit textChanged();

    this->updateStaticTextLater();
}

void TransliteratedText::setFont(const QFont &val)
{
    if (m_font == val)
        return;

    m_font = val;
    emit fontChanged();

    this->updateStaticTextLater();
}

void TransliteratedText::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    m_color = val;
    emit colorChanged();

    this->updateStaticTextLater();
}

void TransliteratedText::paint(QPainter *painter)
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug("TransliteratedText is painting %s", qPrintable(m_text));
#endif

    painter->setPen(m_color);
    painter->setFont(m_font);
    painter->drawStaticText(0, 0, m_staticText);
}

void TransliteratedText::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_updateTimer.timerId()) {
        m_updateTimer.stop();
        this->updateStaticText();
    }
}

void TransliteratedText::updateStaticText()
{
    m_staticText.setText(m_text);
    m_staticText.prepare(QTransform(), m_font);

    const QSizeF size = m_staticText.size();
    this->setContentWidth(size.width());
    this->setContentHeight(size.height());

    this->update();
}

void TransliteratedText::updateStaticTextLater()
{
    m_updateTimer.start(0, this);
}

void TransliteratedText::setContentWidth(qreal val)
{
    if (qFuzzyCompare(m_contentWidth, val))
        return;

    m_contentWidth = val;
    emit contentWidthChanged();
}

void TransliteratedText::setContentHeight(qreal val)
{
    if (qFuzzyCompare(m_contentHeight, val))
        return;

    m_contentHeight = val;
    emit contentHeightChanged();
}

///////////////////////////////////////////////////////////////////////////////

TransliterationHints::TransliterationHints(QObject *parent) : QObject(parent) { }

TransliterationHints::~TransliterationHints() { }

TransliterationHints *TransliterationHints::qmlAttachedProperties(QObject *object)
{
    return new TransliterationHints(object);
}

TransliterationHints *TransliterationHints::find(QQuickItem *item)
{
    if (item == nullptr)
        return nullptr;

    do {
        TransliterationHints *ret =
                item->findChild<TransliterationHints *>(QString(), Qt::FindDirectChildrenOnly);
        if (ret)
            return ret;

        item = item->parentItem();
    } while (item);

    return nullptr;
}

void TransliterationHints::setAllowedMechanisms(AllowedMechanisms val)
{
    if (m_allowedMechanisms == val)
        return;

    m_allowedMechanisms = val;
    emit allowedMechanismsChanged();
}

void TransliterationUtils::polishFontsAndInsertTextAtCursor(
        QTextCursor &cursor, const QString &text, const QVector<QTextLayout::FormatRange> &formats)
{
    const int startPos = cursor.position();
    TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor, text);
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
