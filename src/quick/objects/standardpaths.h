/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
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
#include <QObject>
#include <QStandardPaths>

class StandardPaths : public QObject
{
    Q_OBJECT

public:
    StandardPaths(QObject *parent=nullptr);
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

    Q_INVOKABLE QString writableLocation(StandardLocation type) const;
    Q_INVOKABLE QString displayName(StandardLocation type) const;
    Q_INVOKABLE QString findExecutable(const QString &executableName, const QStringList &paths = QStringList());
    Q_INVOKABLE QString locateFile(StandardLocation type, const QString &fileName) const;
    Q_INVOKABLE QStringList locateAllFiles(StandardLocation type, const QString &fileName) const;
    Q_INVOKABLE QString locateFolder(StandardLocation type, const QString &fileName) const;
    Q_INVOKABLE QStringList locateAllFolders(StandardLocation type, const QString &fileName) const;
    Q_INVOKABLE QUrl fromLocalFile(const QString &file) const;
    Q_INVOKABLE QString resolvePath(const QString &path) const;

    static QString resolvedPath(const QString &path);
};

#endif // STANDARDPATHS_H
