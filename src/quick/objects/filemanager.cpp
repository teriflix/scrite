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

#include "filemanager.h"
#include "standardpaths.h"

#include <QDir>
#include <QDateTime>

FileManager::FileManager(QObject *parent) : QObject(parent) { }

FileManager::~FileManager()
{
    this->removeFilesInAutoDeleteList();
}

QString FileManager::generateUniqueTemporaryFileName(const QString &ext)
{
    const QDir tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    qint64 counter = QDateTime::currentMSecsSinceEpoch();
    while (1) {
        const QString baseName = QString::number(counter) + QStringLiteral(".") + ext;
        const QString filePath = tmp.absoluteFilePath(baseName);
        if (!QFile::exists(filePath))
            return filePath;

        ++counter;
    }

    return QString();
}

void FileManager::setAutoDeleteList(const QStringList &val)
{
    if (m_autoDeleteList == val)
        return;

    m_autoDeleteList = val;
    emit autoDeleteListChanged();
}

void FileManager::removeFilesInAutoDeleteList()
{
    if (m_autoDeleteList.isEmpty())
        return;

    while (!m_autoDeleteList.isEmpty()) {
        const QString filePath = m_autoDeleteList.takeFirst();
#ifndef QT_NO_DEBUG_OUTPUT
        qDebug() << "FileManager is removing: " << filePath;
#endif
        QFile::remove(filePath);
    }

    emit autoDeleteListChanged();
}

void FileManager::addToAutoDeleteList(const QString &filePath)
{
    if (!m_autoDeleteList.contains(filePath))
        m_autoDeleteList.append(filePath);
}

void FileManager::removeFromAutoDeleteList(const QString &filePath)
{
    m_autoDeleteList.removeOne(filePath);
}
