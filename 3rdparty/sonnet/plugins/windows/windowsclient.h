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

#ifndef WINDOWS_CLIENT_H
#define WINDOWS_CLIENT_H

#include "client_p.h"
#include "spellerplugin_p.h"

#include <QLoggingCategory>

namespace Sonnet {
class SpellerPlugin;
}
using Sonnet::SpellerPlugin;

struct WindowsClientData;
class WindowsClient : public Sonnet::Client
{
    Q_OBJECT
    Q_INTERFACES(Sonnet::Client)
    // Q_PLUGIN_METADATA(IID "in.scrite.Sonnet.WindowsClient")

public:
    WindowsClient(QObject *parent = nullptr);
    ~WindowsClient();

    // Sonnet::Client interface
    int reliability() const;
    Sonnet::SpellerPlugin *createSpeller(const QString &language);
    QStringList languages() const;
    QString name() const { return QStringLiteral("Windows"); }

    QString defaultEnglishLanguage() const;

private:
    WindowsClientData *d = nullptr;
};

struct WindowsSpellerPluginData;
class WindowsSpellerPlugin : public Sonnet::SpellerPlugin
{
public:
    WindowsSpellerPlugin(const QString &language, WindowsClientData *d);
    ~WindowsSpellerPlugin();

    // Sonnet::SpellerPlugin interface
    bool isCorrect(const QString &word) const;
    QStringList suggest(const QString &word) const;
    bool checkAndSuggest(const QString &word, QStringList &suggestions) const;
    bool storeReplacement(const QString &bad, const QString &good);
    bool addToPersonal(const QString &word);
    bool addToSession(const QString &word);

private:
    QStringList suggest(const QString &word, int atMost) const;

private:
    WindowsSpellerPluginData *d;
};

Q_DECLARE_LOGGING_CATEGORY(SONNET_WINDOWS_ISPELLCHECKER)

#endif
