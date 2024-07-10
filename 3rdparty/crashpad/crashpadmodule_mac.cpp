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

/** This file is compiled only when crashpad is available for macOS platform **/

#include "crashpadmodule.h"

#include <mach-o/dyld.h>

#include <QDir>
#include <QtDebug>

namespace CrashpadModule {
QString applicationDirPath()
{
    static QString ret;

    if (ret.isEmpty()) {
        unsigned int bufferSize = 512;
        std::vector<char> buffer(bufferSize + 1);

        if (_NSGetExecutablePath(&buffer[0], &bufferSize)) {
            buffer.resize(bufferSize);
            _NSGetExecutablePath(&buffer[0], &bufferSize);
        }

        char *lastForwardSlash = strrchr(&buffer[0], '/');
        if (lastForwardSlash == NULL)
            return NULL;
        *lastForwardSlash = 0;

        ret = QString::fromLatin1(&buffer[0]);
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
    static QString ret;

    if (ret.isEmpty()) {
        const QString appDir = CrashpadModule::applicationDirPath();
        ret = QDir(appDir).absoluteFilePath("crashpad_handler");
    }

    return ret;
}

QString CrashpadModule::dataPath()
{
    static QString ret;

    if (ret.isEmpty()) {
        const QString homePath = qgetenv("HOME");
        const QString appSupportPath = homePath + "/Library/Application Support/";
        QDir appSupportDir(appSupportPath);
        if (appSupportDir.cd("Scrite") && appSupportDir.cd("Scrite")) {
            ret = appSupportDir.absoluteFilePath("Crashpad");
            QDir().mkpath(ret);
        }
    }

    return ret;
}

QString CrashpadModule::pendingCrashReportsPath()
{
    return CrashpadModule::dataPath() + "/pending";
}
