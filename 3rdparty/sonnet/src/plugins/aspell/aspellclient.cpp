/*
 * kspell_aspellclient.cpp
 *
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "aspellclient.h"
#include "aspelldict.h"

#include "aspell_debug.h"
#ifdef Q_OS_WIN
#include <QCoreApplication>
#endif

using namespace Sonnet;

ASpellClient::ASpellClient(QObject *parent)
    : Client(parent)
    , m_config(new_aspell_config())
{
#ifdef Q_OS_WIN
    aspell_config_replace(m_config, "data-dir", QString::fromLatin1("%1/data/aspell").arg(QCoreApplication::applicationDirPath()).toLatin1().constData());
    aspell_config_replace(m_config, "dict-dir", QString::fromLatin1("%1/data/aspell").arg(QCoreApplication::applicationDirPath()).toLatin1().constData());
#endif
}

ASpellClient::~ASpellClient()
{
    delete_aspell_config(m_config);
}

SpellerPlugin *ASpellClient::createSpeller(const QString &language)
{
    ASpellDict *ad = new ASpellDict(language);
    return ad;
}

QStringList ASpellClient::languages() const
{
    AspellDictInfoList *l = get_aspell_dict_info_list(m_config);
    AspellDictInfoEnumeration *el = aspell_dict_info_list_elements(l);

    QStringList langs;
    const AspellDictInfo *di = nullptr;
    while ((di = aspell_dict_info_enumeration_next(el))) {
        langs.append(QString::fromLatin1(di->name));
    }

    delete_aspell_dict_info_enumeration(el);

    return langs;
}

#include "moc_aspellclient.cpp"
