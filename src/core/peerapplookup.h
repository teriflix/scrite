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

#ifndef PEERAPPLOOKUP_H
#define PEERAPPLOOKUP_H

#include <QMap>
#include <QObject>

class QTimer;
class QUdpSocket;
class QJsonObject;

/**
 * The purpose of this class is to provide a simple, reliable and crossplatform way
 * to detect if there are other instances of Scrite running on the local computer.
 * This way we can decide whether we want to reset sessionToken or reuse the existing
 * one.
 */

class PeerAppLookup : public QObject
{
    Q_OBJECT

public:
    static PeerAppLookup *instance();
    ~PeerAppLookup();

    // clang-format off
    Q_PROPERTY(QString instanceId
               READ isInstanceId
               CONSTANT )
    // clang-format on
    QString isInstanceId() const { return m_instanceId; }

    // clang-format off
    Q_PROPERTY(int peerCount
               READ peerCount
               NOTIFY peerCountChanged)
    // clang-format on
    int peerCount() const { return m_peerCount; }
    Q_SIGNAL void peerCountChanged();

    // clang-format off
    Q_PROPERTY(int lookupCount
               READ lookupCount
               NOTIFY lookupCountChanged)
    // clang-format on
    int lookupCount() const { return m_lookupCount; }
    Q_SIGNAL void lookupCountChanged();

private:
    explicit PeerAppLookup(QObject *parent = nullptr);

    enum UpdateMode { BlockingUpdate, NonBlockingUpdate, BlockingCleanup };
    void update(UpdateMode mode);
    void nonBlockingUpdate() { this->update(NonBlockingUpdate); }

    static bool readPeersInfo(QJsonObject &info);
    static bool writePeersInfo(const QJsonObject &info);

private:
    QString m_instanceId;
    QTimer *m_updateTimer = nullptr;
    int m_peerCount = 0;
    int m_lookupCount = 0;
};

#endif // PEERAPPLOOKUP_H
