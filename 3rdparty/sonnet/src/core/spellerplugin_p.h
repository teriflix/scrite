/*
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_SPELLERPLUGIN_P_H
#define SONNET_SPELLERPLUGIN_P_H

#include <QString>
#include <QStringList>

#include "sonnetcore_export.h"

#include <memory>

namespace Sonnet
{
class SpellerPluginPrivate;
/*!
 * \internal
 * \brief class used for actual spell checking.
 *
 * Class is returned by from Loader. It acts
 * as the actual spellchecker.
 */
class SONNETCORE_EXPORT SpellerPlugin
{
public:
    virtual ~SpellerPlugin();

    /*!
     * Checks the given word.
     * Returns false if the word is misspelled. true otherwise
     */
    virtual bool isCorrect(const QString &word) const = 0;

    /*!
     * Checks the given word.
     * Returns true if the word is misspelled. false otherwise
     */
    bool isMisspelled(const QString &word) const;

    /*!
     * Fetches suggestions for the word.
     *
     * Returns list of all suggestions for the word
     */
    virtual QStringList suggest(const QString &word) const = 0;

    /*!
     * Convenient method calling isCorrect() and suggest()
     * if the word isn't correct.
     */
    virtual bool checkAndSuggest(const QString &word, QStringList &suggestions) const;

    /*!
     * Stores user defined good replacement for the bad word.
     * Returns true on success
     */
    virtual bool storeReplacement(const QString &bad, const QString &good) = 0;

    /*!
     * Adds word to the list of of personal words.
     * Returns true on success
     */
    virtual bool addToPersonal(const QString &word) = 0;

    /*!
     * Adds word to the words recognizable in the current session.
     * Returns true on success
     */
    virtual bool addToSession(const QString &word) = 0;

    /*!
     * Returns language supported by this dictionary.
     */
    QString language() const;

protected:
    /*!
     */
    SpellerPlugin(const QString &lang);

private:
    std::unique_ptr<SpellerPluginPrivate> const d;
};
}

#endif
