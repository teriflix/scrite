/*
 * voikkoclient.h
 *
 * SPDX-FileCopyrightText: 2015 Jesse Jaara <jesse.jaara@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SONNET_VOIKKOCLIENT_H
#define SONNET_VOIKKOCLIENT_H

#include "client_p.h"

class VoikkoClient : public Sonnet::Client
{
    Q_OBJECT
    Q_INTERFACES(Sonnet::Client)
    Q_PLUGIN_METADATA(IID "org.kde.sonnet.Client")

public:
    explicit VoikkoClient(QObject *parent = nullptr);
    ~VoikkoClient();

    int reliability() const override;

    Sonnet::SpellerPlugin *createSpeller(const QString &language) override;

    QStringList languages() const override;

    QString name() const override;

private:
    QStringList m_supportedLanguages;
};

#endif // SONNET_VOIKKOCLIENT_H
