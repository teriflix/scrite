/*
 * voikkoclient.cpp
 *
 * SPDX-FileCopyrightText: 2015 Jesse Jaara <jesse.jaara@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "voikkoclient.h"
#include "voikkodebug.h"
#include "voikkodict.h"

VoikkoClient::VoikkoClient(QObject *parent)
    : Sonnet::Client(parent)
{
    qCDebug(SONNET_VOIKKO) << "Initializing Voikko spell checker plugin.";

    char **dictionaries = voikkoListSupportedSpellingLanguages(nullptr);

    if (!dictionaries) {
        return;
    }

    for (int i = 0; dictionaries[i] != nullptr; ++i) {
        QString language = QString::fromUtf8(dictionaries[i]);
        m_supportedLanguages.append(language);
        qCDebug(SONNET_VOIKKO) << "Found dictionary for language:" << language;
    }

    voikkoFreeCstrArray(dictionaries);
}

VoikkoClient::~VoikkoClient()
{
}

int VoikkoClient::reliability() const
{
    return 50;
}

Sonnet::SpellerPlugin *VoikkoClient::createSpeller(const QString &language)
{
    VoikkoDict *speller = new VoikkoDict(language);
    if (speller->initFailed()) {
        delete speller;
        return nullptr;
    }

    return speller;
}

QStringList VoikkoClient::languages() const
{
    return m_supportedLanguages;
}

QString VoikkoClient::name() const
{
    return QStringLiteral("Voikko");
}

#include "moc_voikkoclient.cpp"
