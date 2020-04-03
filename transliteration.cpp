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
    : QObject(parent), m_transliterator(nullptr), m_language(English)
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
            int val = metaEnum.keyToValue(qPrintable(lang), &ok);
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
    switch(m_language)
    {
    case English:
        m_transliterator = nullptr;
        break;
    case Bengali:
        m_transliterator = GetBengaliTranslator();
        break;
    case Gujarati:
        m_transliterator = GetGujaratiTranslator();
        break;
    case Hindi:
        m_transliterator = GetHindiTranslator();
        break;
    case Kannada:
        m_transliterator = GetKannadaTranslator();
        break;
    case Malayalam:
        m_transliterator = GetMalayalamTranslator();
        break;
    case Oriya:
        m_transliterator = GetOriyaTranslator();
        break;
    case Punjabi:
        m_transliterator = GetPunjabiTranslator();
        break;
    case Sanskrit:
        m_transliterator = GetSanskritTranslator();
        break;
    case Tamil:
        m_transliterator = GetTamilTranslator();
        break;
    case Telugu:
        m_transliterator = GetTeluguTranslator();
        break;
    }

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
    if(m_activeLanguages.value(m_language,false) == active)
        return;

    m_activeLanguages[language] = active;

    QSettings *settings = Application::instance()->settings();
    const QMetaObject *mo = this->metaObject();
    const QMetaEnum metaEnum = mo->enumerator( mo->indexOfEnumerator("Language") );
    QStringList activeLanguages = settings->value("Transliteration/activeLanguages").toStringList();
    if(active)
        activeLanguages.append(QString::fromLatin1(metaEnum.valueToKey(m_language)));
    else
        activeLanguages.removeOne(QString::fromLatin1(metaEnum.valueToKey(m_language)));
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

QString TransliterationSettings::transliteratedWord(const QString &word) const
{
    if(m_transliterator == nullptr)
        return word;

    return QString::fromStdWString(Translate(m_transliterator, word.toStdWString().c_str()));
}

///////////////////////////////////////////////////////////////////////////////

Transliterator::Transliterator(QObject *parent)
    : QObject(parent),
      m_mode(AutomaticMode),
      m_cursorPosition(-1),
      m_hasActiveFocus(false),
      m_textDocument(nullptr)
{

}

Transliterator::~Transliterator()
{

}

Transliterator *Transliterator::qmlAttachedProperties(QObject *object)
{
    return new Transliterator(object);
}

void Transliterator::setTextDocument(QQuickTextDocument *val)
{
    if(m_textDocument == val)
        return;

    if(m_textDocument != nullptr)
    {
        QTextDocument *doc = m_textDocument->textDocument();
        if(doc != nullptr)
        {
            disconnect( doc, &QQuickTextDocument::destroyed,
                        this, &Transliterator::onTextDocumentDestroyed);
            disconnect( doc, &QTextDocument::contentsChange,
                        this, &Transliterator::processTransliteration);
        }
    }

    m_textDocument = val;

    if(m_textDocument != nullptr)
    {
        QTextDocument *doc = m_textDocument->textDocument();
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
        if(this->document() != nullptr)
        {
            // Transliterate the last word
            QTextCursor cursor(this->document());
            cursor.setPosition(m_cursorPosition);
            cursor.select(QTextCursor::WordUnderCursor);
            cursor.insertText( TransliterationSettings::instance()->transliteratedWord(cursor.selectedText()) );
            this->setCursorPosition(-1);
        }
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

void Transliterator::transliterate(QTextCursor &cursor)
{
    if(this->document() == nullptr || cursor.document() != this->document())
        return;

    void *transliterator = TransliterationSettings::instance()->transliterator();
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
