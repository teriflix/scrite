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

#ifndef FILELOCKER_H
#define FILELOCKER_H

#include <QObject>
#include <QJsonObject>

class QTimer;
class QFileSystemWatcher;

/**
 * QLockFile doesn't cut it for us. The main reason why we need a lock-file is to let
 * users create Scrite documents direclty on OneDrive or GoogleDrive synced folder and
 * open it from another location.
 *
 * Here are the scenarios it needs to support.
 *
 * Scenario #1: Singe user on local file system (most likely usecase)
 * ------------------------------------------------
 * User opens a Scrite document. Reads, writes, saves, closes the document.
 *
 * Scenario #2: Single user on cloud linked file system (like OneDrive, Google Drive, iCloud etc..)
 * ------------------------------------------------
 * User opens (or saves) a Scrite document on a folder that is linked to their
 * cloud storage. Reads, writes, saves, closes the document. Changes are syned
 * by the cloud-syncing software.
 *
 * User opens the same Scrite document from another computer, which is synced
 * to the same folder as before. All changes from the previous operation are
 * synced by the cloud software.
 *
 * Scenario #3: Multiple users cloud linked file system
 * ------------------------------------------------
 * Two or more users share a single Scrite document. As long as one of the users
 * has it opened, other user(s) are informed that they cannot open the file.
 *
 * Plus we need a mechanism to auto-update claim status when it changes AND
 * force-claim a file when the owner is done.
 */
class FileLocker : public QObject
{
    Q_OBJECT

public:
    explicit FileLocker(QObject *parent = nullptr);
    ~FileLocker();

    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    void setFilePath(const QString &val);
    QString filePath() const { return m_filePath; }
    Q_SIGNAL void filePathChanged();

    QString lockFilePath() const { return m_lockFilePath; }

    enum Strategy { MultipleReadSingleWrite, SingleReadSingleWrite };
    Q_PROPERTY(Strategy strategy READ strategy WRITE setStrategy NOTIFY strategyChanged)
    void setStrategy(Strategy val);
    Strategy strategy() const { return m_strategy; }
    Q_SIGNAL void strategyChanged();

    Q_PROPERTY(bool canRead READ canRead NOTIFY canReadChanged)
    bool canRead() const { return m_canRead; }
    Q_SIGNAL void canReadChanged();

    Q_PROPERTY(bool canWrite READ canWrite NOTIFY canWriteChanged)
    bool canWrite() const { return m_canWrite; }
    Q_SIGNAL void canWriteChanged();

    Q_PROPERTY(QJsonObject lockInfo READ lockInfo NOTIFY lockInfoChanged)
    QJsonObject lockInfo() const { return m_lockInfo; }
    Q_SIGNAL void lockInfoChanged();

    Q_PROPERTY(bool claimed READ isClaimed NOTIFY claimedChanged)
    bool isClaimed() const { return m_claimed; }
    Q_SIGNAL void claimedChanged();

    Q_INVOKABLE bool claim();

    Q_SIGNAL void modified();

private:
    void cleanup();
    void initialize();
    void updateStatus();
    void setCanRead(bool val);
    void setCanWrite(bool val);
    void setLockInfo(const QJsonObject &val);
    void setClaimed(bool val);

private:
    QString m_filePath;
    QString m_lockFilePath;
    QString m_uniqueId;
    bool m_canRead = false;
    bool m_canWrite = false;
    bool m_claimed = false;
    QJsonObject m_lockInfo;
    Strategy m_strategy = SingleReadSingleWrite;
    QFileSystemWatcher *m_fsWatcher = nullptr;
    QTimer *m_modifiedTimer = nullptr;
};

#endif // FILELOCKER_H
