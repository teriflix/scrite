/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "dummyclient.h"

DummyClient::DummyClient(QObject *parent) : Sonnet::Client(parent) { }

DummyClient::~DummyClient() { }

int DummyClient::reliability() const
{
    return qEnvironmentVariableIsSet("SONNET_PREFER_NSSPELLCHECKER") ? 9999 : 30;
}

Sonnet::SpellerPlugin *DummyClient::createSpeller(const QString &language)
{
    return new DummySpellerPlugin(language);
}

QStringList DummyClient::languages() const
{
    return QStringList() << "dummy";
}

///////////////////////////////////////////////////////////////////////////////

DummySpellerPlugin::DummySpellerPlugin(const QString &language)
    : Sonnet::SpellerPlugin(language) { }

DummySpellerPlugin::~DummySpellerPlugin() { }

bool DummySpellerPlugin::isCorrect(const QString &word) const
{
    Q_UNUSED(word)
    return true;
}

QStringList DummySpellerPlugin::suggest(const QString &word) const
{
    Q_UNUSED(word)
    return QStringList();
}

bool DummySpellerPlugin::checkAndSuggest(const QString &word, QStringList &suggestions) const
{
    Q_UNUSED(word)
    Q_UNUSED(suggestions)
    return true;
}

bool DummySpellerPlugin::storeReplacement(const QString &bad, const QString &good)
{
    Q_UNUSED(bad)
    Q_UNUSED(good)
    return true;
}

bool DummySpellerPlugin::addToPersonal(const QString &word)
{
    Q_UNUSED(word)
    return true;
}

bool DummySpellerPlugin::addToSession(const QString &word)
{
    Q_UNUSED(word)
    return true;
}
