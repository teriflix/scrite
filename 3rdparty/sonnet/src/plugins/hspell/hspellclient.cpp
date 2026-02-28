/*
 * kspell_hspellclient.cpp
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2005 Mashrab Kuvatov <kmashrab@uni-bremen.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "hspellclient.h"

#include "hspell.h"
#include "hspelldict.h"

#include <QFileInfo>
#include <QUrl>

using namespace Sonnet;

HSpellClient::HSpellClient(QObject *parent)
    : Client(parent)
{
}

HSpellClient::~HSpellClient()
{
}

SpellerPlugin *HSpellClient::createSpeller(const QString &language)
{
    HSpellDict *ad = new HSpellDict(language);
    return ad;
}

QStringList HSpellClient::languages() const
{
    QString dictPath(QString::fromUtf8(hspell_get_dictionary_path()));
    if (QUrl(dictPath).isLocalFile() && QFileInfo::exists(dictPath)) {
        return {QStringLiteral("he")};
    }
    return {};
}

#include "moc_hspellclient.cpp"
