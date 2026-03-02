/*
 * nsspellcheckerdict.h
 *
 * SPDX-FileCopyrightText: 2015 Nick Shaforostoff <shaforostoff@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KSPELL_NSSPELLDICT_H
#define KSPELL_NSSPELLDICT_H

#include "spellerplugin_p.h"

class NSSpellCheckerDict : public Sonnet::SpellerPlugin
{
public:
    explicit NSSpellCheckerDict(const QString &lang);
    ~NSSpellCheckerDict();
    virtual bool isCorrect(const QString &word) const;

    virtual QStringList suggest(const QString &word) const;

    virtual bool storeReplacement(const QString &bad, const QString &good);

    virtual bool addToPersonal(const QString &word);
    virtual bool addToSession(const QString &word);

private:
#ifdef __OBJC__
    NSString *m_langCode;
#else
    void *m_langCode;
#endif
};

#endif
