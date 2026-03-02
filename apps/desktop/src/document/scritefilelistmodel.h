/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SCRITEFILELISTMODEL_H
#define SCRITEFILELISTMODEL_H

#include <QQmlEngine>
#include <QAbstractListModel>

#include "scritefileinfo.h"

/**
 * Simply provide a list of ScriteFiles and extract information about the files
 * via a model interface.
 */
class QFileSystemWatcher;

class ScriteFileListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    ScriteFileListModel(QObject *parent = nullptr);
    ~ScriteFileListModel();

    enum Source { RecentFiles, Custom };
    Q_ENUM(Source)

    // clang-format off
    Q_PROPERTY(Source source
               READ source
               WRITE setSource
               NOTIFY sourceChanged)
    // clang-format on
    void setSource(Source val);
    Source source() const { return m_source; }
    Q_SIGNAL void sourceChanged();

    // clang-format off
    Q_PROPERTY(bool notifyMissingFiles
               READ isNotifyMissingFiles
               WRITE setNotifyMissingFiles
               NOTIFY notifyMissingFilesChanged)
    // clang-format on
    void setNotifyMissingFiles(bool val);
    bool isNotifyMissingFiles() const { return m_notifyMissingFiles; }
    Q_SIGNAL void notifyMissingFilesChanged();

    // This method returns a list of all .scrite files in a given folder
    Q_INVOKABLE static QStringList filesInFolder(const QString &folder);

    // Through this property we set the list of .scrite files to be accessed via this model
    // clang-format off
    Q_PROPERTY(QStringList files
               READ files
               WRITE setFiles
               NOTIFY filesChanged)
    // clang-format on
    void setFiles(const QStringList &val);
    QStringList files() const;
    Q_SIGNAL void filesChanged();

    // Here we specify the maximum number of files this model should maintain a list of
    // clang-format off
    Q_PROPERTY(int maxCount
               READ maxCount
               WRITE setMaxCount
               NOTIFY maxCountChanged)
    // clang-format on
    void setMaxCount(int val);
    int maxCount() const { return m_maxCount; }
    Q_SIGNAL void maxCountChanged();

    // Using this method we can add a new file to list, as the first item in the list
    Q_INVOKABLE void add(const QString &filePath);

    // Using this method we can update information about the file at filePath. If the
    // file is deleted, the corresponding row will be removed from the model.
    Q_INVOKABLE void update(const QString &filePath);

    // Remove file at a specific index in the list
    Q_INVOKABLE bool removeAt(int index);

    // Clear all files from the list
    Q_INVOKABLE void clear();

    // This property returns the number of items in the model
    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY filesChanged)
    // clang-format on
    int count() const { return m_files.size(); }

    // FileInfoRole returns an instance of ScriteFileInfo
    // CoverPageUrlRole returns a URL for the cover page image
    enum Roles { FileInfoRole = Qt::UserRole };

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : m_files.size(); }
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const
    {
        return { { FileInfoRole, QByteArrayLiteral("fileInfo") } };
    }

signals:
    void filesMissing(const QStringList &files);

private:
    void loadRecentFiles();
    void setFilesInternal(const QStringList &files);
    void updateFromScriteFileInfo(const ScriteFileInfo &sfi);

    void reportMissingFiles(const QStringList &files);

private:
    int m_maxCount = 10;
    Source m_source = Custom;
    bool m_notifyMissingFiles = false;
    QList<ScriteFileInfo> m_files;
    QFileSystemWatcher *m_watcher = nullptr;
};

#endif // SCRITEFILELISTMODEL_H
