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

#include <QDir>
#include <QTimer>
#include <QFileSystemWatcher>

ScriteFileListModel::ScriteFileListModel(QObject *parent) : QAbstractListModel(parent)
{
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &ScriteFileListModel::update);
}

ScriteFileListModel::~ScriteFileListModel() { }

QStringList ScriteFileListModel::filesInFolder(const QString &folder)
{
    const QDir dir(folder);
    if (!dir.exists())
        return QStringList();

    const QFileInfoList fiList =
            dir.entryInfoList(QStringList { "*.scrite" }, QDir::Files, QDir::Time | QDir::Reversed);
    QStringList ret;
    std::transform(fiList.begin(), fiList.end(), std::back_inserter(ret), [](const QFileInfo &fi) {
        // TODO: load() could be time-consuming, perhaps it should be done in a separate thread.
        const ScriteFileInfo sfi = ScriteFileInfo::load(fi);
        if (sfi.isValid())
            return sfi.filePath;
        return QString();
    });

    return ret;
}

void ScriteFileListModel::setFiles(const QStringList &val)
{
    QList<ScriteFileInfo> newList;
    for (const QString &filePath : val) {
        // TODO: load() could be time-consuming, perhaps it should be done in a separate thread.
        const ScriteFileInfo sfi = ScriteFileInfo::load(filePath);
        if (sfi.isValid())
            newList.append(sfi);
    }

    if (newList == m_files)
        return;

    m_watcher->removePaths(m_watcher->files());

    this->beginResetModel();
    m_files = newList.mid(0, m_maxCount);
    for (const ScriteFileInfo &sfi : qAsConst(m_files))
        m_watcher->addPath(sfi.filePath);
    this->endResetModel();

    emit filesChanged();
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
    // TODO: load() could be time-consuming, perhaps it should be done in a separate thread.
    const ScriteFileInfo sfi = ScriteFileInfo::load(filePath);
    if (!sfi.isValid())
        return;

    const int idx = m_files.indexOf(sfi);
    if (idx == 0)
        return;

    if (idx > 0) {
        this->beginRemoveRows(QModelIndex(), idx, idx);
        m_files.removeAt(idx);
        this->endRemoveRows();
    }

    this->beginInsertRows(QModelIndex(), 0, 0);
    m_files.prepend(sfi);
    this->endInsertRows();

    m_watcher->addPath(sfi.filePath);

    emit filesChanged();
}

void ScriteFileListModel::update(const QString &filePath)
{
    const int idx = m_files.indexOf(ScriteFileInfo { filePath });
    if (idx < 0)
        return;

    const QModelIndex index = this->index(idx);

    const ScriteFileInfo sfi = ScriteFileInfo::load(filePath);
    if (sfi.isValid()) {
        m_files.replace(idx, sfi);
        emit dataChanged(index, index);
    } else {
        m_watcher->removePath(filePath);

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
