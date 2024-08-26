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

#include "scritefilelistmodel.h"
#include "application.h"

#include <QDir>
#include <QTimer>
#include <QFuture>
#include <QSettings>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QtConcurrentMap>
#include <QFileSystemWatcher>

ScriteFileListModel::ScriteFileListModel(QObject *parent) : QAbstractListModel(parent)
{
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &ScriteFileListModel::update);

    connect(this, &QAbstractListModel::rowsInserted, this, &ScriteFileListModel::filesChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &ScriteFileListModel::filesChanged);
    connect(this, &QAbstractListModel::modelReset, this, &ScriteFileListModel::filesChanged);
    connect(this, &QAbstractListModel::dataChanged, this, &ScriteFileListModel::filesChanged);
}

ScriteFileListModel::~ScriteFileListModel() { }

void ScriteFileListModel::setSource(Source val)
{
    if (m_source == val)
        return;

    m_source = val;
    emit sourceChanged();

    QTimer::singleShot(500, this, &ScriteFileListModel::loadRecentFiles);
}

QStringList ScriteFileListModel::filesInFolder(const QString &folder)
{
    const QDir dir(folder);
    if (!dir.exists())
        return QStringList();

    const QFileInfoList fiList =
            dir.entryInfoList(QStringList { "*.scrite" }, QDir::Files, QDir::Time | QDir::Reversed);
    QStringList ret;
    std::transform(fiList.begin(), fiList.end(), std::back_inserter(ret),
                   [](const QFileInfo &fi) { return fi.absoluteFilePath(); });

    return ret;
}

inline static ScriteFileInfo loadScriteFileInfo(const QString &filePath)
{
    ScriteFileInfo ret = ScriteFileInfo::load(filePath);
    if (!ret.isValid())
        ret.filePath = filePath;

    return ret;
};

void ScriteFileListModel::setFiles(const QStringList &filePaths)
{
    if (m_source == Custom) {
        this->setFilesInternal(filePaths);
        return;
    }
}

QStringList ScriteFileListModel::files() const
{
    QStringList ret;
    std::transform(m_files.begin(), m_files.end(), std::back_inserter(ret),
                   [](const ScriteFileInfo &sfi) { return sfi.filePath; });
    return ret;
}

void ScriteFileListModel::setMaxCount(int val)
{
    if (m_maxCount == val)
        return;

    m_maxCount = val;
    emit maxCountChanged();

    if (m_maxCount > m_files.size()) {
        const QList<ScriteFileInfo> prunedFiles = m_files.mid(m_maxCount);
        for (const ScriteFileInfo &prunedFile : prunedFiles)
            m_watcher->removePath(prunedFile.filePath);

        this->beginRemoveRows(QModelIndex(), m_maxCount, m_files.size() - 1);
        m_files = m_files.mid(0, m_maxCount);
        this->endRemoveRows();
    }
}

void ScriteFileListModel::add(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    // Right now, we are only doing a quick load of ScriteFileInfo object for the given filePath
    ScriteFileInfo sfi = ScriteFileInfo::quickLoad(filePath);
    if (!sfi.fileInfo.exists())
        return;

    const int idx = m_files.indexOf(sfi);
    if (idx != 0) {
        if (idx > 0) {
            this->beginRemoveRows(QModelIndex(), idx, idx);
            sfi = m_files.takeAt(idx);
            this->endRemoveRows();
        }

        this->beginInsertRows(QModelIndex(), 0, 0);
        m_files.prepend(sfi);
        this->endInsertRows();

        m_watcher->addPath(sfi.filePath);
    }

    // At this point, we can afford to schedule a complete load of ScriteFileInfo
    // object in a separate thread and update this model when its done.
    QFutureWatcher<ScriteFileInfo> *futureWatcher = new QFutureWatcher<ScriteFileInfo>(this);
    connect(futureWatcher, &QFutureWatcher<ScriteFileInfo>::finished, this, [=]() {
        const ScriteFileInfo sfi = futureWatcher->result();
        this->updateFromScriteFileInfo(sfi);
        futureWatcher->deleteLater();
    });
    QFuture<ScriteFileInfo> future = QtConcurrent::run(loadScriteFileInfo, filePath);
    futureWatcher->setFuture(future);
}

void ScriteFileListModel::update(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    // Check if this path already existed in the model
    int idx = -1;
    for (int i = 0; i < m_files.size(); i++) {
        if (m_files[i].filePath == filePath) {
            idx = i;
            break;
        }
    }

    // If we did not already have the path in this model, then there is nothing to update.
    if (idx < 0)
        return;

    const QModelIndex index = this->index(idx);

    // If the file at filePath no longer exists, then we simply need to remove it.
    if (!QFile::exists(filePath)) {
        m_watcher->removePath(filePath);

        this->beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_files.removeAt(index.row());
        this->endRemoveRows();

        return;
    }

    // At this point, we can afford to schedule a complete load of ScriteFileInfo
    // object in a separate thread and update this model when its done.
    QFutureWatcher<ScriteFileInfo> *futureWatcher = new QFutureWatcher<ScriteFileInfo>(this);
    connect(futureWatcher, &QFutureWatcher<ScriteFileInfo>::finished, this, [=]() {
        const ScriteFileInfo sfi = futureWatcher->result();
        this->updateFromScriteFileInfo(sfi);
        futureWatcher->deleteLater();
    });
    QFuture<ScriteFileInfo> future = QtConcurrent::run(loadScriteFileInfo, filePath);
    futureWatcher->setFuture(future);
}

void ScriteFileListModel::updateFromScriteFileInfo(const ScriteFileInfo &sfi)
{
    if (sfi.filePath.isEmpty())
        return;

    const int idx = m_files.indexOf(sfi);
    if (idx < 0)
        return;

    const QModelIndex index = this->index(idx);

    if (sfi.isValid()) {
        m_files.replace(idx, sfi);
        emit dataChanged(index, index);
    } else {
        m_watcher->removePath(sfi.filePath);

        this->beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_files.removeAt(index.row());
        this->endRemoveRows();
    }
}

QVariant ScriteFileListModel::data(const QModelIndex &index, int role) const
{
    QVariant none;
    if (!index.isValid() || role != FileInfoRole)
        return none;

    const ScriteFileInfo sfi = m_files.at(index.row());
    return QVariant::fromValue<ScriteFileInfo>(sfi);
}

void ScriteFileListModel::loadRecentFiles()
{
    if (m_source == RecentFiles) {
        const QString recentFilesKey = QStringLiteral("RecentFiles/files");

        QSettings *settings = Application::instance()->settings();
        const QStringList filePaths = settings->value(recentFilesKey, QStringList()).toStringList();
        this->setFilesInternal(filePaths);

        settings->setValue(recentFilesKey, this->files());
    }
}

void ScriteFileListModel::setFilesInternal(const QStringList &filePaths)
{
    // Right now, we are only doing a quick load of ScriteFileInfo objects
    QList<ScriteFileInfo> newList;
    for (const QString &filePath : filePaths) {
        const ScriteFileInfo sfi = ScriteFileInfo::quickLoad(filePath);
        if (sfi.fileInfo.exists())
            newList.append(sfi);
    }

    if (newList == m_files)
        return;

    const QStringList files = m_watcher->files();
    if (!files.isEmpty())
        m_watcher->removePaths(files);

    this->beginResetModel();
    m_files = newList.mid(0, m_maxCount);
    for (const ScriteFileInfo &sfi : qAsConst(m_files))
        m_watcher->addPath(sfi.filePath);
    this->endResetModel();

    // At this point, we can afford to schedule a complete load of ScriteFileInfo
    // objects in a separate thread. As and when we get results, we can update
    // them in the model.
    QFutureWatcher<ScriteFileInfo> *futureWatcher = new QFutureWatcher<ScriteFileInfo>(this);
    connect(futureWatcher, &QFutureWatcher<ScriteFileInfo>::resultReadyAt, this, [=](int index) {
        const ScriteFileInfo sfi = futureWatcher->resultAt(index);
        this->updateFromScriteFileInfo(sfi);
    });
    connect(futureWatcher, &QFutureWatcher<ScriteFileInfo>::finished, futureWatcher,
            &QObject::deleteLater);
    QFuture<ScriteFileInfo> future = QtConcurrent::mapped(filePaths, loadScriteFileInfo);
    futureWatcher->setFuture(future);
}
