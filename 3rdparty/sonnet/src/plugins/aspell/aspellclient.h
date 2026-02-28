/*
 * kspell_aspellclient.h
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KSPELL_ASPELLCLIENT_H
#define KSPELL_ASPELLCLIENT_H

#include "client_p.h"

#include "aspell.h"

namespace Sonnet
{
class SpellerPlugin;
}
using Sonnet::SpellerPlugin;

class ASpellClient : public Sonnet::Client
{
    Q_OBJECT
    Q_INTERFACES(Sonnet::Client)
    Q_PLUGIN_METADATA(IID "org.kde.sonnet.Client")

public:
    explicit ASpellClient(QObject *parent = nullptr);
    ~ASpellClient() override;

    int reliability() const override
    {
        return 20;
    }

    SpellerPlugin *createSpeller(const QString &language) override;

    QStringList languages() const override;

    QString name() const override
    {
        return QStringLiteral("ASpell");
    }

private:
    AspellConfig *const m_config;
};

#endif
