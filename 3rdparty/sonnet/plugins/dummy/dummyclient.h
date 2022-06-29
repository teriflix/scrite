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

#ifndef DUMMY_CLIENT_H
#define DUMMY_CLIENT_H

#include "client_p.h"
#include "spellerplugin_p.h"

namespace Sonnet {
class SpellerPlugin;
}
using Sonnet::SpellerPlugin;

class DummyClient : public Sonnet::Client
{
    Q_OBJECT
    Q_INTERFACES(Sonnet::Client)
    // Q_PLUGIN_METADATA(IID "in.scrite.Sonnet.DummyClient")

public:
    DummyClient(QObject *parent = nullptr);
    ~DummyClient();

    // Sonnet::Client interface
    int reliability() const;
    Sonnet::SpellerPlugin *createSpeller(const QString &language);
    QStringList languages() const;
    QString name() const { return QStringLiteral("Dummy"); }
};

class DummySpellerPlugin : public Sonnet::SpellerPlugin
{
public:
    DummySpellerPlugin(const QString &language);
    ~DummySpellerPlugin();

    // Sonnet::SpellerPlugin interface
    bool isCorrect(const QString &word) const;
    QStringList suggest(const QString &word) const;
    bool checkAndSuggest(const QString &word, QStringList &suggestions) const;
    bool storeReplacement(const QString &bad, const QString &good);
    bool addToPersonal(const QString &word);
    bool addToSession(const QString &word);
};

#endif
