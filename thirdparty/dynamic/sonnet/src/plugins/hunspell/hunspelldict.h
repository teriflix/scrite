/*
 * kspell_aspelldict.h
 *
 * SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KSPELL_HUNSPELLDICT_H
#define KSPELL_HUNSPELLDICT_H

#include "hunspell.hxx"
#include "spellerplugin_p.h"

#include <QStringDecoder>
#include <QStringEncoder>

#include <memory>

class HunspellDict : public Sonnet::SpellerPlugin
{
public:
    explicit HunspellDict(const QString &name, const std::shared_ptr<Hunspell> &speller);
    ~HunspellDict() override;
    bool isCorrect(const QString &word) const override;

    QStringList suggest(const QString &word) const override;

    bool storeReplacement(const QString &bad, const QString &good) override;

    bool addToPersonal(const QString &word) override;
    bool addToSession(const QString &word) override;

    static std::shared_ptr<Hunspell> createHunspell(const QString &lang, QString path);

private:
    QByteArray toDictEncoding(const QString &word) const;

    std::shared_ptr<Hunspell> m_speller;
    mutable QStringEncoder m_encoder;
    mutable QStringDecoder m_decoder;
};

#endif
