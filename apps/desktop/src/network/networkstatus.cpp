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

#include "networkstatus.h"
#include "networkaccessmanager.h"

#include <QTimer>
#include <QNetworkReply>

NetworkStatus::NetworkStatus(QObject *parent) : QObject { parent }
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &NetworkStatus::pingUrl);
    m_timer->start(m_interval);

    m_networkAccessManager = NetworkAccessManager::instance();
}

NetworkStatus::~NetworkStatus() { }

void NetworkStatus::setOnline(bool val)
{
    if (m_online == val)
        return;

    m_online = val;
    emit onlineChanged();
}

void NetworkStatus::setInterval(int val)
{
    if (m_interval == val)
        return;

    m_interval = val;
    emit intervalChanged();
}

void NetworkStatus::setUrl(const QUrl &val)
{
    if (m_url == val)
        return;

    m_url = val;
    emit urlChanged();
}

void NetworkStatus::pingUrl()
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QNetworkAccessManager *nam = m_networkAccessManager;
    if (nam == nullptr)
        nam = NetworkAccessManager::instance();

    QNetworkRequest request(m_url);
    request.setTransferTimeout(5000); // 5 seconds timeout

    m_reply = nam->get(request);

    connect(m_reply, &QNetworkReply::finished, this, &NetworkStatus::onReplyFinished);

    emit busyChanged();
}

void NetworkStatus::onReplyFinished()
{
    this->setOnline((m_reply->error() == QNetworkReply::NoError));

    m_reply->deleteLater();

    emit busyChanged();

    m_timer->start(m_interval);
}

void NetworkStatus::setNetworkAccessManager(QNetworkAccessManager *val)
{
    if (m_networkAccessManager == val)
        return;

    m_networkAccessManager = val;
    emit networkAccessManagerChanged();
}
