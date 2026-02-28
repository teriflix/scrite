/*
 * kspell_hspelldict.h
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2005 Mashrab Kuvatov <kmashrab@uni-bremen.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KSPELL_HSPELLDICT_H
#define KSPELL_HSPELLDICT_H

#include <QSet>
#include <QStringDecoder>
#include <QStringEncoder>

#include "spellerplugin_p.h"
/* libhspell is a C library and it does not have #ifdef __cplusplus */
extern "C" {
#include "hspell.h"
}

class HSpellDict : public Sonnet::SpellerPlugin
{
public:
    explicit HSpellDict(const QString &lang);
    ~HSpellDict();
    bool isCorrect(const QString &word) const override;

    QStringList suggest(const QString &word) const override;

    bool storeReplacement(const QString &bad, const QString &good) override;

    bool addToPersonal(const QString &word) override;
    bool addToSession(const QString &word) override;
    inline bool isInitialized() const
    {
        return initialized;
    }

private:
    void storePersonalWords();

    struct dict_radix *m_speller;
    mutable QStringDecoder m_decoder;
    mutable QStringEncoder m_encoder;
    bool initialized;
    QSet<QString> m_sessionWords;
    QSet<QString> m_personalWords;
    QHash<QString, QString> m_replacements;
};

#endif
