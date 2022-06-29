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

#include "garbagecollector.h"
#include "networkaccessmanager.h"

#include <QNetworkReply>
#include <QCoreApplication>

NetworkAccessManager *NetworkAccessManager::INSTANCE = nullptr;

NetworkAccessManager *NetworkAccessManager::instance()
{
    if (NetworkAccessManager::INSTANCE)
        return NetworkAccessManager::INSTANCE;

    return new NetworkAccessManager(qApp);
}

NetworkAccessManager::NetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent)
{
    NetworkAccessManager::INSTANCE = this;
    connect(this, &QNetworkAccessManager::finished, this, &NetworkAccessManager::onReplyFinished);
    connect(this, &QNetworkAccessManager::sslErrors, this, &NetworkAccessManager::onSslErrors);
}

NetworkAccessManager::~NetworkAccessManager()
{
    NetworkAccessManager::INSTANCE = nullptr;
}

QNetworkReply *NetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                   const QNetworkRequest &request,
                                                   QIODevice *outgoingData)
{
    QNetworkReply *reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
    if (reply)
        m_replies.append(reply);

    return reply;
}

void NetworkAccessManager::onReplyFinished(QNetworkReply *reply)
{
    if (reply != nullptr && m_replies.removeOne(reply)) {
        GarbageCollector::instance()->add(reply);

        if (m_replies.isEmpty()) {
            NetworkAccessManager::INSTANCE = nullptr;
            GarbageCollector::instance()->add(this);
        }
    }
}

void NetworkAccessManager::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    reply->ignoreSslErrors(errors);
}
