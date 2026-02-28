/*
 * kspell_hspellclient.h
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2005 Mashrab Kuvatov <kmashrab@uni-bremen.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KSPELL_HSPELLCLIENT_H
#define KSPELL_HSPELLCLIENT_H

#include "client_p.h"

/* libhspell is a C library and it does not have #ifdef __cplusplus */
extern "C" {
#include "hspell.h"
}

namespace Sonnet
{
class SpellerPlugin;
}
using Sonnet::SpellerPlugin;

class HSpellClient : public Sonnet::Client
{
    Q_OBJECT
    Q_INTERFACES(Sonnet::Client)
    Q_PLUGIN_METADATA(IID "org.kde.sonnet.Client")
public:
    explicit HSpellClient(QObject *parent = nullptr);
    ~HSpellClient();

    int reliability() const override
    {
        return 20;
    }

    SpellerPlugin *createSpeller(const QString &language) override;

    QStringList languages() const override;

    QString name() const override
    {
        return QString::fromLatin1("HSpell");
    }

private:
};

#endif
