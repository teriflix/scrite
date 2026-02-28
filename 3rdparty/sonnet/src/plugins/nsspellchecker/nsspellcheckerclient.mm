/*
 * nsspellcheckerclient.mm
 *
 * SPDX-FileCopyrightText: 2015 Nick Shaforostoff <shaforostoff@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "nsspellcheckerclient.h"
#include "nsspellcheckerdict.h"

#import <AppKit/AppKit.h>

using namespace Sonnet;

NSSpellCheckerClient::NSSpellCheckerClient(QObject *parent)
    : Client(parent)
{
}

NSSpellCheckerClient::~NSSpellCheckerClient()
{
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
    NSArray* availableLanguages = [[NSSpellChecker sharedSpellChecker]
                                   availableLanguages];
    for (NSString* lang_code in availableLanguages) {
        lst.append(QString::fromNSString(lang_code));
    }
    return lst;
}


#include "moc_nsspellcheckerclient.cpp"
