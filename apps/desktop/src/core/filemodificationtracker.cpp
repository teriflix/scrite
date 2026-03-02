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

#include "filemodificationtracker.h"

#include <QTimer>
#include <QFileInfo>
#include <QFileSystemWatcher>

/**
First I tried tracking file changes using QFileSystemWatcher. QFileSystemWatcher is supposed to
emit fileChanged() signal anytime the file at the stated path changes.

This seems to happen only when the file at the path was modified. It doesn't happen, if the file was
deleted and replaced with a new one by the same name. This is because QFileSystemWatcher uses
file-inode to track changes, even if what we ask of it is to watch a path.

This is a deal-breaker for us, because there are instances where cloud-sync-software (like OneDrive,
GoogleDrive etc) delete the previously synced file, only to replace it with an new one after sync.
In such cases the watcher doesnt emit the fileChanged() signal and the user cannot be notified of
the file being changed in the background by another process.

Then, I tried using QFileSystemWatcher on the folder in which the file existed and track changes to
that folder instead. That wasn't reliable as well. In some odd cases, the notification would never
come. So, I had to junk the idea of using QFileSystemWatcher all together.

For that reason, we use a timer to periodically check for file changes. Its a brute force way to
monitor for changes to the file, but it looks like we have no way out. If anybody has a better idea,
please do suggest.
 */

FileModificationTracker::FileModificationTracker(QObject *parent) : QObject(parent) { }

FileModificationTracker::~FileModificationTracker() { }

void FileModificationTracker::setFilePath(const QString &val)
{
    if (m_filePath == val)
        return;

    m_filePath = val;
    m_paused = false;
    delete m_checkTimer;

    m_checkTimer = new QTimer(this);
    m_checkTimer->setInterval(500);
    m_checkTimer->setSingleShot(false);
    connect(m_checkTimer, &QTimer::timeout, this, &FileModificationTracker::checkForModifications);

    if (!m_filePath.isEmpty()) {
        const QFileInfo fi(m_filePath);
        m_fileAvailable = fi.exists();
        if (m_fileAvailable)
            m_fileModificationTime = fi.fileTime(QFile::FileModificationTime);
    }

    m_checkTimer->start();

    emit filePathChanged();
}

void FileModificationTracker::pauseTracking(int timeout)
{
    if (m_paused)
        return;

    m_paused = true;

    if (m_checkTimer)
        m_checkTimer->stop();

    QTimer::singleShot(timeout, this, &FileModificationTracker::resumeTracking);
}

void FileModificationTracker::resumeTracking()
{
    if (m_paused)
        this->checkForModifications();

    m_paused = false;

    if (m_checkTimer)
        m_checkTimer->start();
}

void FileModificationTracker::checkForModifications()
{
    const QFileInfo fi(m_filePath);

    const bool fa = fi.exists();
    if (fa != m_fileAvailable) {
        m_fileAvailable = fa;
        if (m_fileAvailable)
            m_fileModificationTime = fi.fileTime(QFile::FileModificationTime);
        if (!m_paused)
            emit fileModified(m_filePath);
        return;
    }

    if (m_fileAvailable) {
        const QDateTime dt = fi.fileTime(QFile::FileModificationTime);
        if (dt != m_fileModificationTime) {
            m_fileModificationTime = dt;
            if (!m_paused)
                emit fileModified(m_filePath);
        }
    }
}
