/*
 * SPDX-FileCopyrightText: 2006 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "spellerplugin_p.h"

namespace Sonnet
{
class SpellerPluginPrivate
{
public:
    QString language;
};

SpellerPlugin::SpellerPlugin(const QString &lang)
    : d(new SpellerPluginPrivate)
{
    d->language = lang;
}

SpellerPlugin::~SpellerPlugin() = default;

QString SpellerPlugin::language() const
{
    return d->language;
}

bool SpellerPlugin::isMisspelled(const QString &word) const
{
    return !isCorrect(word);
}

bool SpellerPlugin::checkAndSuggest(const QString &word, QStringList &suggestions) const
{
    bool c = isCorrect(word);
    if (!c) {
        suggestions = suggest(word);
    }
    return c;
}
}
