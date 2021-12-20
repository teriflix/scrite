/**
 * nsspellcheckerclient.mm
 *
 * Copyright (C)  2015  Nick Shaforostoff <shaforostoff@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "nsspellcheckerclient.h"
#include "nsspellcheckerdict.h"

#import <AppKit/AppKit.h>

#include <QtDebug>
#include <QThread>

using namespace Sonnet;

NSSpellCheckerClient::NSSpellCheckerClient(QObject *parent) : Client(parent) { }

NSSpellCheckerClient::~NSSpellCheckerClient() { }

/**
 * In Qt 5.15.7, for whatever reason, [NSSpellChecker sharedSpellChecker]
 * doesnt create or return a shared spell checker, if we ask for it in a
 * background thread. So, we call this function in the main thread once
 * before we query for availableLanguages() and so on in the background thread.
 */
bool NSSpellCheckerClient::ensureSpellCheckerAvailability()
{
    const bool spellCheckerExists = [NSSpellChecker sharedSpellCheckerExists];
    if (!spellCheckerExists) {
        NSSpellChecker *spellChecker = [NSSpellChecker sharedSpellChecker];
        return spellChecker != nullptr;
    }
    return spellCheckerExists;
}

int NSSpellCheckerClient::reliability() const
{
    return qEnvironmentVariableIsSet("SONNET_PREFER_NSSPELLCHECKER") ? 9999 : 30;
}

SpellerPlugin *NSSpellCheckerClient::createSpeller(const QString &language)
{
    return new NSSpellCheckerDict(language);
}

QStringList NSSpellCheckerClient::languages() const
{
    QStringList lst;

    const bool spellCheckerExists = [NSSpellChecker sharedSpellCheckerExists];
    if (!spellCheckerExists)
        return lst;

    NSSpellChecker *spellChecker = [NSSpellChecker sharedSpellChecker];
    NSArray *availableLanguages = [spellChecker availableLanguages];
    for (NSString *lang_code in availableLanguages) {
        lst.append(QString::fromNSString(lang_code));
    }
    return lst;
}
