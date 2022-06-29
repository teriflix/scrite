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

#include "peerapplookup.h"

#include <QDir>
#include <QUuid>
#include <QFile>
#include <QTimer>
#include <QLockFile>
#include <QDateTime>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFutureWatcher>
#include <QStandardPaths>
#include <QtConcurrentRun>
#include <QCoreApplication>

const int updateInterval = 1000;
static QString peerInfoFilePath()
{
    static QString ret;
    if (ret.isEmpty()) {
        const QString folderPath =
                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir folder(folderPath);
        if (!folder.exists())
            QDir().mkpath(folderPath);
        ret = folderPath + QStringLiteral("/peers.json");
    }
    return ret;
}

PeerAppLookup::PeerAppLookup(QObject *parent) : QObject(parent)
{
    m_instanceId = QUuid::createUuid().toString();

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &PeerAppLookup::nonBlockingUpdate);
    m_updateTimer->setSingleShot(true);

    this->update(BlockingUpdate);
}

PeerAppLookup *PeerAppLookup::instance()
{
    static PeerAppLookup *theInstance = new PeerAppLookup(qApp);
    return theInstance;
}

PeerAppLookup::~PeerAppLookup()
{
    this->update(BlockingCleanup);
}

void PeerAppLookup::update(UpdateMode mode)
{
    auto updateImpl = [](const QString &iid, bool remove) -> int {
        const QString filePath = ::peerInfoFilePath();
        QLockFile fileLock(filePath + QStringLiteral("_lock"));
        if (!fileLock.tryLock(100))
            return 0;

        QJsonObject info;
        PeerAppLookup::readPeersInfo(info);

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (remove)
            info.remove(iid);
        else
            info.insert(iid, QString::number(now));

        QStringList iids = info.keys();
        iids.removeAll(iid);
        for (const QString &_iid : qAsConst(iids)) {
            const qint64 timestamp = info.value(_iid).toString().toLongLong();
            if (now - timestamp > 10 * updateInterval)
                info.remove(_iid);
        }

        PeerAppLookup::writePeersInfo(info);
        return qMax(0, remove ? info.size() : info.size() - 1);
    };

    auto acceptUpdateResult = [=](int pc) {
        if (m_peerCount != pc) {
            m_peerCount = pc;
            emit peerCountChanged();
        }
        m_updateTimer->start(updateInterval);
        ++m_lookupCount;
        emit lookupCountChanged();
    };

    if (mode == NonBlockingUpdate) {
        QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);
        connect(watcher, &QFutureWatcher<int>::finished, this, [=]() {
            acceptUpdateResult(watcher->result());
            watcher->deleteLater();
        });
        watcher->setFuture(QtConcurrent::run(updateImpl, m_instanceId, false));
    } else if (mode == BlockingUpdate) {
        acceptUpdateResult(updateImpl(m_instanceId, false));
    } else if (mode == BlockingCleanup) {
        updateImpl(m_instanceId, true);
    }
}

bool PeerAppLookup::readPeersInfo(QJsonObject &info)
{
    info = QJsonObject();

    const QString filePath = ::peerInfoFilePath();
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly))
        return false;

    const QByteArray bytes = file.readAll();
    if (bytes.isEmpty())
        return false;

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError)
        return false;

    info = doc.object();
    return true;
}

bool PeerAppLookup::writePeersInfo(const QJsonObject &info)
{
    const QString filePath = ::peerInfoFilePath();
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly))
        return false;

    const QByteArray bytes = QJsonDocument(info).toJson();
    file.write(bytes);
    return true;
}
