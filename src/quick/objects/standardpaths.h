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

#ifndef STANDARDPATHS_H
#define STANDARDPATHS_H

#include <QUrl>
#include <QQmlEngine>
#include <QStandardPaths>

class StandardPaths : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")
    QML_SINGLETON

public:
    explicit StandardPaths(QObject *parent = nullptr);
    ~StandardPaths();

    // Copied from QStandardPaths
    enum StandardLocation {
        DesktopLocation,
        DocumentsLocation,
        FontsLocation,
        ApplicationsLocation,
        MusicLocation,
        MoviesLocation,
        PicturesLocation,
        TempLocation,
        HomeLocation,
        DataLocation,
        CacheLocation,
        GenericDataLocation,
        RuntimeLocation,
        ConfigLocation,
        DownloadLocation,
        GenericCacheLocation,
        GenericConfigLocation,
        AppDataLocation,
        AppConfigLocation,
        AppLocalDataLocation = DataLocation,
        ApplicationBinaryLocation = 500,
    };
    Q_ENUM(StandardLocation)

    Q_INVOKABLE static QString writableLocation(StandardPaths::StandardLocation type);
    Q_INVOKABLE static QString displayName(StandardPaths::StandardLocation type);
    Q_INVOKABLE static QString findExecutable(const QString &executableName,
                                              const QStringList &paths = QStringList());
    Q_INVOKABLE static QString locateFile(StandardPaths::StandardLocation type,
                                          const QString &fileName);
    Q_INVOKABLE static QStringList locateAllFiles(StandardPaths::StandardLocation type,
                                                  const QString &fileName);
    Q_INVOKABLE static QString locateFolder(StandardPaths::StandardLocation type,
                                            const QString &fileName);
    Q_INVOKABLE static QStringList locateAllFolders(StandardPaths::StandardLocation type,
                                                    const QString &fileName);
    Q_INVOKABLE static QUrl fromLocalFile(const QString &file);
    Q_INVOKABLE static QString resolvePath(const QString &path);

    static QString resolvedPath(const QString &path);
};

#endif // STANDARDPATHS_H
