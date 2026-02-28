/*
 * kspell_aspelldict.h
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KSPELL_ASPELLDICT_H
#define KSPELL_ASPELLDICT_H

#include "spellerplugin_p.h"

#include "aspell.h"

class ASpellDict : public Sonnet::SpellerPlugin
{
public:
    explicit ASpellDict(const QString &lang);
    ~ASpellDict() override;
    bool isCorrect(const QString &word) const override;

    QStringList suggest(const QString &word) const override;

    bool storeReplacement(const QString &bad, const QString &good) override;

    bool addToPersonal(const QString &word) override;
    bool addToSession(const QString &word) override;

private:
    AspellConfig *m_config = nullptr;
    AspellSpeller *m_speller = nullptr;
};

#endif
