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

/** This file is compiled only when crashpad is available for Windows platform **/

#include "crashpadmodule.h"

#include <QDir>
#include <QtDebug>

#include "client/crashpad_client.h"

#define NOMINMAX
#include <windows.h>

namespace CrashpadModule {
QString applicationDirPath()
{
    static QString ret;

    if (ret.isEmpty()) {
        HMODULE hModule = GetModuleHandleW(NULL);
        WCHAR path[MAX_PATH];
        DWORD retVal = GetModuleFileNameW(hModule, path, MAX_PATH);
        if (retVal == 0)
            return NULL;

        wchar_t *lastBackslash = wcsrchr(path, '\\');
        if (lastBackslash == NULL)
            return NULL;
        *lastBackslash = 0;

        ret = QString::fromWCharArray(path);
        ret = QDir::fromNativeSeparators(ret);
    }

    return ret;
}
}

bool CrashpadModule::isAvailable()
{
    return true;
}

bool CrashpadModule::initialize()
{
    static bool invokedOnce = false;
    static bool initStatus = false;

    if (invokedOnce)
        return initStatus;

    invokedOnce = true;

    const QString crashpadHandler = QDir::toNativeSeparators(CrashpadModule::handlerPath());
    if (crashpadHandler.isEmpty())
        return false;

    const QString dataPath = QDir::toNativeSeparators(CrashpadModule::dataPath());

    const base::FilePath _handlerPath(crashpadHandler.toStdWString());
    const base::FilePath _crashpadPath(dataPath.toStdWString());
    const base::FilePath _attachmentPath((dataPath + "\\attachment.txt").toStdWString());
    const std::vector<base::FilePath> _attachments = { _attachmentPath };
    const std::map<std::string, std::string> _annotations = { { "format", "minidump" } };
    const std::vector<std::string> _args = { "--no-rate-limit" };
    const std::string _url;

    static crashpad::CrashpadClient client;
    initStatus = client.StartHandler(_handlerPath, _crashpadPath, _crashpadPath, _url, _annotations,
                                     _args, true, false, _attachments);

    return initStatus;
}

QString CrashpadModule::handlerPath()
{
    // crashpad_handler.exe is either in $SCRITE_CRASHPAD_ROOT\bin
    // or at $APPLICATION_DIR_PATH\bin

    static bool invokedOnce = false;
    static QString ret;

    if (!invokedOnce) {
        invokedOnce = true;

        const QString crashpadHandler = QStringLiteral("crashpad_handler.exe");

        const QDir dir(CrashpadModule::applicationDirPath());
        if (dir.exists(crashpadHandler)) {
            ret = dir.absoluteFilePath(crashpadHandler);
            return ret;
        }

        const QString crashpadSdkRoot = QString::fromLatin1(qgetenv("SCRITE_CRASHPAD_ROOT"));
        if (!crashpadSdkRoot.isEmpty()) {
            const QDir dir(crashpadSdkRoot);
            const QString subPath = "bin/" + crashpadHandler;
            if (dir.exists(subPath)) {
                ret = dir.absoluteFilePath(subPath);
                return ret;
            }
        }
    }

    return ret;
}

QString CrashpadModule::dataPath()
{
    // This would be %APPDATA%\Scrite\Scrite\Crashpad

    static bool invokedOnce = false;
    static QString ret;

    if (!invokedOnce) {
        invokedOnce = true;
        const QString appData = QDir::fromNativeSeparators(QString::fromLatin1(qgetenv("APPDATA")));

        QDir appDataDir(appData);
        if (!appDataDir.exists())
            return QString();

        ret = appData + "/Scrite/Scrite/Crashpad";
        QDir().mkpath(ret);
    }

    return ret;
}
