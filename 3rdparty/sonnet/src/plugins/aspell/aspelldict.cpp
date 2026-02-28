/*
 * kspell_aspelldict.cpp
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "aspelldict.h"

#include "aspell_debug.h"

#ifdef Q_OS_WIN
#include <QCoreApplication>
#endif

using namespace Sonnet;

ASpellDict::ASpellDict(const QString &lang)
    : SpellerPlugin(lang)
{
    m_config = new_aspell_config();
    aspell_config_replace(m_config, "lang", lang.toLatin1().constData());
    /* All communication with Aspell is done in UTF-8 */
    /* For reference, please look at BR#87250         */
    aspell_config_replace(m_config, "encoding", "utf-8");

#ifdef Q_OS_WIN
    aspell_config_replace(m_config, "data-dir", QString::fromLatin1("%1/data/aspell").arg(QCoreApplication::applicationDirPath()).toLatin1().constData());
    aspell_config_replace(m_config, "dict-dir", QString::fromLatin1("%1/data/aspell").arg(QCoreApplication::applicationDirPath()).toLatin1().constData());
#endif

    AspellCanHaveError *possible_err = new_aspell_speller(m_config);

    if (aspell_error_number(possible_err) != 0) {
        qCWarning(SONNET_LOG_ASPELL) << "aspell error: " << aspell_error_message(possible_err);
    } else {
        m_speller = to_aspell_speller(possible_err);
    }
}

ASpellDict::~ASpellDict()
{
    delete_aspell_speller(m_speller);
    delete_aspell_config(m_config);
}

bool ASpellDict::isCorrect(const QString &word) const
{
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.toUtf8().length() for nonlatin strings    */
    if (!m_speller) {
        return false;
    }
    int correct = aspell_speller_check(m_speller, word.toUtf8().constData(), word.toUtf8().length());
    return correct;
}

QStringList ASpellDict::suggest(const QString &word) const
{
    if (!m_speller) {
        return QStringList();
    }

    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.toUtf8().length() for nonlatin strings    */
    const AspellWordList *suggestions = aspell_speller_suggest(m_speller, word.toUtf8().constData(), word.toUtf8().length());

    AspellStringEnumeration *elements = aspell_word_list_elements(suggestions);

    QStringList qsug;
    const char *cword;

    while ((cword = aspell_string_enumeration_next(elements))) {
        /* Since while creating the class ASpellDict the encoding is set */
        /* to utf-8, one has to convert output from Aspell to QString's UTF-16 */
        qsug.append(QString::fromUtf8(cword));
    }

    delete_aspell_string_enumeration(elements);
    return qsug;
}

bool ASpellDict::storeReplacement(const QString &bad, const QString &good)
{
    if (!m_speller) {
        return false;
    }
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.toUtf8().length() for nonlatin strings    */
    return aspell_speller_store_replacement(m_speller, bad.toUtf8().constData(), bad.toUtf8().length(), good.toUtf8().constData(), good.toUtf8().length());
}

bool ASpellDict::addToPersonal(const QString &word)
{
    if (!m_speller) {
        return false;
    }
    qCDebug(SONNET_LOG_ASPELL) << "Adding" << word << "to aspell personal dictionary";
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.toUtf8().length() for nonlatin strings    */
    aspell_speller_add_to_personal(m_speller, word.toUtf8().constData(), word.toUtf8().length());
    /* Add is not enough, one has to save it. This is not documented */
    /* in ASpell's API manual. I found it in                         */
    /* aspell-0.60.2/example/example-c.c                             */
    return aspell_speller_save_all_word_lists(m_speller);
}

bool ASpellDict::addToSession(const QString &word)
{
    if (!m_speller) {
        return false;
    }
    /* ASpell is expecting length of a string in char representation */
    /* word.length() != word.toUtf8().length() for nonlatin strings    */
    return aspell_speller_add_to_session(m_speller, word.toUtf8().constData(), word.toUtf8().length());
}
