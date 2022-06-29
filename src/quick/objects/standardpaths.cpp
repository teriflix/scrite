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

#include "standardpaths.h"

#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>
#include <QtDebug>
#include <QMetaEnum>

StandardPaths::StandardPaths(QObject *parent) : QObject(parent) { }

StandardPaths::~StandardPaths() { }

QString StandardPaths::writableLocation(StandardPaths::StandardLocation type)
{
    switch (type) {
    case ApplicationBinaryLocation:
        return qApp->applicationDirPath();
    default:
        break;
    }

    return QStandardPaths::writableLocation(QStandardPaths::StandardLocation(type));
}

QString StandardPaths::displayName(StandardPaths::StandardLocation type)
{
    switch (type) {
    case ApplicationBinaryLocation:
#ifdef Q_OS_WIN
        return QString("Location of %1.exe").arg(qApp->applicationName());
#else
        return QString("Location of %1").arg(qApp->applicationName());
#endif
    default:
        break;
    }

    return QStandardPaths::displayName(QStandardPaths::StandardLocation(type));
}

QString StandardPaths::findExecutable(const QString &executableName, const QStringList &paths)
{
    {
        const QDir appDir(qApp->applicationDirPath());
        if (appDir.exists(executableName))
            return appDir.absoluteFilePath(executableName);
    }

    return QStandardPaths::findExecutable(executableName, paths);
}

QString StandardPaths::locateFile(StandardPaths::StandardLocation type, const QString &fileName)
{
    switch (type) {
    case ApplicationBinaryLocation: {
        const QDir appDir(qApp->applicationDirPath());
        if (appDir.exists(fileName))
            return appDir.absoluteFilePath(fileName);
        return QString();
    }
    default:
        break;
    }

    return QStandardPaths::locate(QStandardPaths::StandardLocation(type), fileName,
                                  QStandardPaths::LocateFile);
}

QStringList StandardPaths::locateAllFiles(StandardPaths::StandardLocation type,
                                          const QString &fileName)
{
    if (int(type) >= ApplicationBinaryLocation)
        return QStringList();

    return QStandardPaths::locateAll(QStandardPaths::StandardLocation(type), fileName,
                                     QStandardPaths::LocateFile);
}

QString StandardPaths::locateFolder(StandardPaths::StandardLocation type, const QString &fileName)
{
    if (int(type) >= ApplicationBinaryLocation)
        return StandardPaths::locateFile(type, fileName);

    return QStandardPaths::locate(QStandardPaths::StandardLocation(type), fileName,
                                  QStandardPaths::LocateDirectory);
}

QStringList StandardPaths::locateAllFolders(StandardPaths::StandardLocation type,
                                            const QString &fileName)
{
    if (int(type) >= ApplicationBinaryLocation)
        return QStringList();

    return QStandardPaths::locateAll(QStandardPaths::StandardLocation(type), fileName,
                                     QStandardPaths::LocateDirectory);
}

QUrl StandardPaths::fromLocalFile(const QString &file)
{
    return QUrl::fromLocalFile(file);
}

QString StandardPaths::resolvePath(const QString &path)
{
    const int enumIndex = StandardPaths::staticMetaObject.indexOfEnumerator("StandardLocation");
    const QMetaEnum enumerator = StandardPaths::staticMetaObject.enumerator(enumIndex);

    QString retPath = path;
    for (int i = 0; i < enumerator.keyCount(); i++) {
        const QString key = QString("$$%1").arg(enumerator.key(i));
        if (path.startsWith(key)) {
            retPath.remove(0, key.length() + 1);
            retPath = StandardPaths::locateFile(
                    StandardPaths::StandardLocation(enumerator.value(i)), retPath);
            break;
        }
    }

    return retPath;
}

QString StandardPaths::resolvedPath(const QString &path)
{
    StandardPaths paths;
    return paths.resolvePath(path);
}
