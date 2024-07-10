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

QString CrashpadModule::pendingCrashReportsPath()
{
    return CrashpadModule::dataPath() + "/reports";
}
