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

#include "transliteration.h"
#include "application.h"

#include <QMetaEnum>
#include <QSettings>
#include <QMetaObject>
#include <QJsonObject>
#include <QTextCursor>
#include <QTextDocument>
#include <QFontDatabase>
#include <QQuickTextDocument>

#include <PhTranslateLib>

static QStringList getCustomFontFilePaths()
{
    const QStringList customFonts = QStringList() <<
         ":/font/Gujarati/HindVadodara-Regular.ttf" <<
         ":/font/Gujarati/HindVadodara-Bold.ttf" <<
         ":/font/Oriya/BalooBhaina2-Regular.ttf" <<
         ":/font/Oriya/BalooBhaina2-Bold.ttf" <<
         ":/font/Punjabi/BalooPaaji2-Regular.ttf" <<
         ":/font/Punjabi/BalooPaaji2-Bold.ttf" <<
         ":/font/Malayalam/BalooChettan2-Regular.ttf" <<
         ":/font/Malayalam/BalooChettan2-Bold.ttf" <<
         ":/font/Hindi/Mukta-Regular.ttf" <<
         ":/font/Hindi/Mukta-Bold.ttf" <<
         ":/font/Telugu/HindGuntur-Regular.ttf" <<
         ":/font/Telugu/HindGuntur-Bold.ttf" <<
         ":/font/Sanskrit/Mukta-Regular.ttf" <<
         ":/font/Sanskrit/Mukta-Bold.ttf" <<
         ":/font/English/CourierPrime-BoldItalic.ttf" <<
         ":/font/English/CourierPrime-Bold.ttf" <<
         ":/font/English/CourierPrime-Italic.ttf" <<
         ":/font/English/CourierPrime-Regular.ttf" <<
         ":/font/Kannada/BalooTamma2-Regular.ttf" <<
         ":/font/Kannada/BalooTamma2-Bold.ttf" <<
         ":/font/Tamil/HindMadurai-Regular.ttf" <<
         ":/font/Tamil/HindMadurai-Bold.ttf" <<
         ":/font/Bengali/HindSiliguri-Regular.ttf" <<
         ":/font/Bengali/HindSiliguri-Bold.ttf";
    return customFonts;
}

TransliterationEngine *TransliterationEngine::instance(QCoreApplication *app)
{
    static TransliterationEngine *newInstance = new TransliterationEngine(app ? app : qApp);
    return newInstance;
}

TransliterationEngine::TransliterationEngine(QObject *parent)
    : QObject(parent)
{
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    Q_FOREACH(QString customFont, getCustomFontFilePaths())
    {
        const int id = QFontDatabase::addApplicationFont(customFont);
        const QString language = customFont.split("/", QString::SkipEmptyParts).at(2);
        Language lang = Language(metaEnum.keyToValue(qPrintable(language)));
        m_languageFontIdMap[lang] = id;
        m_languageFontFilePaths[lang].append(customFont);
    }

    for(int i=0; i<metaEnum.keyCount(); i++)
    {
        Language lang = Language(metaEnum.value(i));
        m_activeLanguages[lang] = false;
    }

    const QSettings *settings = Application::instance()->settings();
    const QStringList activeLanguages = settings->value("Transliteration/activeLanguages").toStringList();

    if(activeLanguages.isEmpty())
    {
        m_activeLanguages[English] = true;
        m_activeLanguages[Kannada] = true;
    }
    else
    {
        Q_FOREACH(QString lang, activeLanguages)
        {
            bool ok = false;
            int val = metaEnum.keyToValue(qPrintable(lang.trimmed()), &ok);
            m_activeLanguages[Language(val)] = true;
        }
    }

    const QString currentLanguage = settings->value("Transliteration/currentLanguage").toString();
    Language lang = English;
    if(!currentLanguage.isEmpty())
    {
        bool ok = false;
        int val = metaEnum.keyToValue(qPrintable(currentLanguage), &ok);
        lang = Language(val);
    }
    this->setLanguage(lang);
}

TransliterationEngine::~TransliterationEngine()
{

}

void TransliterationEngine::setLanguage(TransliterationEngine::Language val)
{
    if(m_language == val)
        return;

    m_language = val;
    m_transliterator = transliteratorFor(m_language);

    QSettings *settings = Application::instance()->settings();
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    settings->setValue("Transliteration/currentLanguage", QString::fromLatin1(metaEnum.valueToKey(m_language)));

    emit languageChanged();
}

QString TransliterationEngine::languageAsString() const
{
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    return QString::fromLatin1(metaEnum.valueToKey(m_language));
}

QString TransliterationEngine::shortcutLetter(TransliterationEngine::Language val)
{
    if(val == Tamil)
        return QString("L");

    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    return QChar(metaEnum.valueToKey(val)[0]).toUpper();
}

void TransliterationEngine::cycleLanguage()
{
    QMap<Language,bool>::const_iterator it = m_activeLanguages.constFind(m_language);
    QMap<Language,bool>::const_iterator end = m_activeLanguages.constEnd();

    ++it;
    while(it != end)
    {
        if(it.value())
        {
            this->setLanguage(it.key());
            return;
        }

        ++it;
    }

    it = m_activeLanguages.constBegin();
    while(it != end)
    {
        if(it.value())
        {
            this->setLanguage(it.key());
            return;
        }

        ++it;
    }
}

void TransliterationEngine::markLanguage(TransliterationEngine::Language language, bool active)
{
    if(m_activeLanguages.value(language,false) == active)
        return;

    m_activeLanguages[language] = active;

    QSettings *settings = Application::instance()->settings();
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    QStringList activeLanguages;
    for(int i=0; i<metaEnum.keyCount(); i++)
    {
        Language lang = Language(metaEnum.value(i));
        if(m_activeLanguages.value(lang))
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
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    QJsonArray ret;
    for(int i=0; i<metaEnum.keyCount(); i++)
    {
        QJsonObject item;
        item.insert("key", QString::fromLatin1(metaEnum.key(i)));
        item.insert("value", metaEnum.value(i));
        item.insert("active", m_activeLanguages.value(Language(metaEnum.value(i))));
        item.insert("current", m_language == metaEnum.value(i));
        ret.append(item);
    }

    return ret;
}

void *TransliterationEngine::transliteratorFor(TransliterationEngine::Language language) const
{
    switch(language)
    {
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

TransliterationEngine::Language TransliterationEngine::languageOf(void *transliterator) const
{
#define CHECK(x) if(transliterator == transliteratorFor(TransliterationEngine::x)) return TransliterationEngine::x
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

QString TransliterationEngine::transliteratedWord(const QString &word) const
{
    if(m_transliterator == nullptr)
        return word;

    return QString::fromStdWString(Translate(m_transliterator, word.toStdWString().c_str()));
}

QString TransliterationEngine::transliteratedSentence(const QString &sentence, bool includingLastWord) const
{
    if(m_transliterator == nullptr)
        return sentence;

    QStringList words = sentence.split(" ");
    for(int i=0; i<words.length(); i++)
    {
        if(i < words.length()-1 || includingLastWord)
            words[i] = this->transliteratedWord(words[i]);
    }

    return words.join(" ");
}

QFont TransliterationEngine::languageFont(TransliterationEngine::Language language) const
{
    static QMap<Language,QFontDatabase::WritingSystem> languageSystemMap;
    if(languageSystemMap.isEmpty())
    {
        languageSystemMap[English] = QFontDatabase::Latin;
        languageSystemMap[Bengali] = QFontDatabase::Bengali;
        languageSystemMap[Gujarati] = QFontDatabase::Gujarati;
        languageSystemMap[Hindi] = QFontDatabase::Devanagari;
        languageSystemMap[Kannada] = QFontDatabase::Kannada;
        languageSystemMap[Malayalam] = QFontDatabase::Malayalam;
        languageSystemMap[Oriya] = QFontDatabase::Oriya;
        languageSystemMap[Punjabi] = QFontDatabase::Gurmukhi;
        languageSystemMap[Sanskrit] = QFontDatabase::Devanagari;
        languageSystemMap[Tamil] = QFontDatabase::Tamil;
        languageSystemMap[Telugu] = QFontDatabase::Telugu;
    }

    QFontDatabase fontDb;
    const int id = m_languageFontIdMap.value(language, -1);
    const QStringList families = id < 0 ? fontDb.families(languageSystemMap.value(language)) : fontDb.applicationFontFamilies(id);
    if(families.isEmpty())
        return Application::instance()->font();

    return QFont( families.first() );
}

QStringList TransliterationEngine::languageFontFilePaths(TransliterationEngine::Language language) const
{
    return m_languageFontFilePaths.value(language, QStringList());
}

QList<TransliterationEngine::Breakup> TransliterationEngine::breakupText(const QString &text) const
{
    QList<Breakup> ret;

    static QMap<QChar::Script,Language> scriptLanguageMap;
    if(scriptLanguageMap.isEmpty())
    {
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

    QChar::Script lastScript = QChar::Script_Latin;

    Breakup item;
    for(int i=0; i<text.length(); i++)
    {
        const QChar ch = text.at(i);
        if( (ch.script() != lastScript && ch.isLetterOrNumber()) ||
            (ch.category() == QChar::Separator_Line || ch.category() == QChar::Separator_Paragraph) )
        {
            if(!item.string.isEmpty())
            {
                item.language = scriptLanguageMap.value(lastScript, English);
                item.font = this->languageFont(item.language);
                ret.append(item);
            }

            item = Breakup();
        }

        item.string.append(ch);

        if(ch.isLetterOrNumber())
            lastScript = ch.script();
    }

    if(!item.string.isEmpty())
    {
        item.language = scriptLanguageMap.value(lastScript, English);
        item.font = this->languageFont(item.language);
        ret.append(item);
    }

    return ret;
}

void TransliterationEngine::insertBreakupText(QTextCursor &cursor, const QString &text) const
{
    const QTextCharFormat givenFormat = cursor.charFormat();
    const QList<Breakup> breakup = this->breakupText(text);
    Q_FOREACH(Breakup item, breakup)
    {
        QTextCharFormat charFormat;
        if(item.language == English)
            charFormat.setFontFamily(givenFormat.fontFamily());
        else
            charFormat.setFontFamily(item.font.family());
        cursor.insertText(item.string);
    }
    cursor.setCharFormat(givenFormat);
}

///////////////////////////////////////////////////////////////////////////////

Transliterator::Transliterator(QObject *parent)
    : QObject(parent)
{

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
    if(m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void Transliterator::setTextDocument(QQuickTextDocument *val)
{
    if(m_textDocument == val)
        return;

    if(m_textDocument != nullptr)
    {
        QTextDocument *doc = m_textDocument->textDocument();
        doc->setUndoRedoEnabled(true);

        if(doc != nullptr)
        {
            disconnect( doc, &QQuickTextDocument::destroyed,
                        this, &Transliterator::onTextDocumentDestroyed);
            disconnect( doc, &QTextDocument::contentsChange,
                        this, &Transliterator::processTransliteration);
        }

        this->setCursorPosition(-1);
        this->setHasActiveFocus(false);
    }

    m_textDocument = val;

    if(m_textDocument != nullptr)
    {
        QTextDocument *doc = m_textDocument->textDocument();
        doc->setUndoRedoEnabled(false);

        if(doc != nullptr)
        {
            connect( doc, &QQuickTextDocument::destroyed,
                     this, &Transliterator::onTextDocumentDestroyed);
            connect( doc, &QTextDocument::contentsChange,
                     this, &Transliterator::processTransliteration);
        }
    }

    emit textDocumentChanged();
}

void Transliterator::setCursorPosition(int val)
{
    if(m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    emit cursorPositionChanged();
}

void Transliterator::setHasActiveFocus(bool val)
{
    if(m_hasActiveFocus == val)
        return;

    m_hasActiveFocus = val;

    if(!m_hasActiveFocus)
    {
        if(m_enabled)
            this->transliterateLastWord();
        this->setCursorPosition(-1);
    }

    emit hasActiveFocusChanged();
}

void Transliterator::setMode(Transliterator::Mode val)
{
    if(m_mode == val)
        return;

    m_mode = val;
    emit modeChanged();
}

void Transliterator::transliterateLastWord()
{
    if(this->document() != nullptr && m_cursorPosition >= 0)
    {
        // Transliterate the last word
        QTextCursor cursor(this->document());
        cursor.setPosition(m_cursorPosition);
        cursor.select(QTextCursor::WordUnderCursor);
        this->transliterate(cursor);
    }
}

void Transliterator::transliterate(int from, int to)
{
    if(this->document() == nullptr || to-from <= 0)
        return;

    void *transliterator = TransliterationEngine::instance()->transliterator();
    if(transliterator == nullptr)
        return;

    QTextCursor cursor(this->document());
    cursor.setPosition(from);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, to-from);
    this->transliterate(cursor);
}

void Transliterator::transliterateToLanguage(int from, int to, int language)
{
    if(this->document() == nullptr || to-from <= 0)
        return;

    void *transliterator = TransliterationEngine::instance()->transliteratorFor(TransliterationEngine::Language(language));
    if(transliterator == nullptr)
        return;

    QTextCursor cursor(this->document());
    cursor.setPosition(from);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, to-from);
    this->transliterate(cursor, transliterator);
}

QTextDocument *Transliterator::document() const
{
    return m_textDocument ? m_textDocument->textDocument() : nullptr;
}

void Transliterator::onTextDocumentDestroyed()
{
    m_textDocument = nullptr;
    emit textDocumentChanged();
}

void Transliterator::processTransliteration(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved)
    if(this->document() == nullptr || !m_hasActiveFocus || !m_enabled)
        return;

    if(m_enableFromNextWord == true)
    {
        m_enableFromNextWord = false;
        return;
    }

    void *transliterator = TransliterationEngine::instance()->transliterator();
    if(transliterator == nullptr)
        return;

    QTextCursor cursor(this->document());
    cursor.setPosition(from);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, charsAdded);

    const QString original = cursor.selectedText();

    if(charsAdded == 1)
    {
        // Check if the char that was added was a space.
        if(original.length() == 1 && !original.at(0).isLetter())
        {
            // Select the word that is just before the cursor.
            cursor.setPosition(from);
            cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor, 1);
            cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor, 1);
            this->transliterate(cursor);
            return;
        }
    }
    else
    {
        // Transliterate all the words that was just added.
        this->transliterate(cursor);
    }
}

void Transliterator::transliterate(QTextCursor &cursor, void *transliterator)
{
    if(this->document() == nullptr || cursor.document() != this->document() || !m_hasActiveFocus || !m_enabled)
        return;

    if(transliterator == nullptr)
        transliterator = TransliterationEngine::instance()->transliterator();
    if(transliterator == nullptr)
        return;

    const QString original = cursor.selectedText();
    QString replacement;
    QString word;

    for(int i=0; i<original.length(); i++)
    {
        const QChar ch = original.at(i);
        if(ch.isLetter())
            word += ch;
        else
        {
            replacement += TransliterationEngine::instance()->transliteratedWord(word);
            replacement += ch;
            word.clear();
        }
    }

    if(!word.isEmpty())
        replacement += TransliterationEngine::instance()->transliteratedWord(word);

    if(m_mode == AutomaticMode)
    {
        emit aboutToTransliterate(cursor.selectionStart(), cursor.selectionEnd(), replacement, original);
        cursor.insertText(replacement);
        emit finishedTransliterating(cursor.selectionStart(), cursor.selectionEnd(), replacement, original);
    }
    else
        emit transliterationSuggestion(cursor.selectionStart(), cursor.selectionEnd(), replacement, original);
}
