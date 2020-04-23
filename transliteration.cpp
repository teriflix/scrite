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
#include <QQuickTextDocument>

#include <PhTranslateLib>

TransliterationSettings *TransliterationSettings::instance(QCoreApplication *app)
{
    static TransliterationSettings *newInstance = new TransliterationSettings(app ? app : qApp);
    return newInstance;
}

TransliterationSettings::TransliterationSettings(QObject *parent)
    : QObject(parent)
{
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    for(int i=0; i<metaEnum.keyCount(); i++)
        m_activeLanguages[Language(metaEnum.value(i))] = false;

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

TransliterationSettings::~TransliterationSettings()
{

}

void TransliterationSettings::setLanguage(TransliterationSettings::Language val)
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

QString TransliterationSettings::languageAsString() const
{
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    return QString::fromLatin1(metaEnum.valueToKey(m_language));
}

QString TransliterationSettings::shortcutLetter(TransliterationSettings::Language val)
{
    if(val == Tamil)
        return QString("L");

    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    return QChar(metaEnum.valueToKey(val)[0]).toUpper();
}

void TransliterationSettings::cycleLanguage()
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

void TransliterationSettings::markLanguage(TransliterationSettings::Language language, bool active)
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

bool TransliterationSettings::queryLanguage(TransliterationSettings::Language language) const
{
    return m_activeLanguages.value(language, false);
}

QJsonArray TransliterationSettings::languages() const
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

void *TransliterationSettings::transliteratorFor(TransliterationSettings::Language language) const
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

QString TransliterationSettings::transliteratedWord(const QString &word) const
{
    if(m_transliterator == nullptr)
        return word;

    return QString::fromStdWString(Translate(m_transliterator, word.toStdWString().c_str()));
}

QString TransliterationSettings::transliteratedSentence(const QString &sentence, bool includingLastWord) const
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
        cursor.insertText( TransliterationSettings::instance()->transliteratedWord(cursor.selectedText()) );
    }
}

void Transliterator::transliterate(int from, int to)
{
    if(this->document() == nullptr || to-from <= 0)
        return;

    void *transliterator = TransliterationSettings::instance()->transliterator();
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

    void *transliterator = TransliterationSettings::instance()->transliteratorFor(TransliterationSettings::Language(language));
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
    if(this->document() == nullptr)
        return;

    void *transliterator = TransliterationSettings::instance()->transliterator();
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
        transliterator = TransliterationSettings::instance()->transliterator();
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
            replacement += TransliterationSettings::instance()->transliteratedWord(word);
            replacement += ch;
            word.clear();
        }
    }

    if(!word.isEmpty())
        replacement += TransliterationSettings::instance()->transliteratedWord(word);

    if(m_mode == AutomaticMode)
        cursor.insertText(replacement);
    else
        emit transliterationSuggestion(cursor.selectionStart(), cursor.selectionEnd(), replacement, original);
}
