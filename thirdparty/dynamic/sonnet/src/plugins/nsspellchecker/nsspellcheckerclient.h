/*
 * nsspellcheckerclient.h
 *
 * SPDX-FileCopyrightText: 2015 Nick Shaforostoff <shaforostoff@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KSPELL_NSSPELLCLIENT_H
#define KSPELL_NSSPELLCLIENT_H

#include "client_p.h"

namespace Sonnet
{
class SpellerPlugin;
}
using Sonnet::SpellerPlugin;

class NSSpellCheckerClient : public Sonnet::Client
{
    Q_OBJECT
    Q_INTERFACES(Sonnet::Client)
    Q_PLUGIN_METADATA(IID "org.kde.sonnet.Client")
public:
    explicit NSSpellCheckerClient(QObject *parent = nullptr);
    ~NSSpellCheckerClient();

    int reliability() const;

    SpellerPlugin *createSpeller(const QString &language);
    QStringList languages() const;
    QString name() const
    {
        return QStringLiteral("NSSpellChecker");
    }
};

#endif
