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

#ifndef FILEMODIFICATIONTRACKER_H
#define FILEMODIFICATIONTRACKER_H

#include <QDateTime>
#include <QObject>

class QTimer;
class QFileSystemWatcher;

class FileModificationTracker : public QObject
{
    Q_OBJECT

public:
    FileModificationTracker(QObject *parent = nullptr);
    ~FileModificationTracker();

    // clang-format off
    Q_PROPERTY(QString filePath
               READ filePath
               WRITE setFilePath
               NOTIFY filePathChanged)
    // clang-format on
    void setFilePath(const QString &val);
    QString filePath() const { return m_filePath; }
    Q_SIGNAL void filePathChanged();

    Q_INVOKABLE void pauseTracking(int timeout = 500);
    Q_INVOKABLE void resumeTracking();

signals:
    void fileModified(const QString &filePath);

private:
    void checkForModifications();

private:
    QString m_filePath;
    bool m_paused = false;
    QTimer *m_checkTimer = nullptr;

    bool m_fileAvailable = false;
    QDateTime m_fileModificationTime;
};

#endif // FILEMODIFICATIONTRACKER_H
