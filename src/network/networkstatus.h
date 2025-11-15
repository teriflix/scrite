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

#ifndef NETWORKSTATUS_H
#define NETWORKSTATUS_H

#include <QObject>
#include <QQmlEngine>
#include <QNetworkAccessManager>

class QTimer;
class NetworkStatus : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit NetworkStatus(QObject *parent = nullptr);
    virtual ~NetworkStatus();

    // clang-format off
    Q_PROPERTY(bool busy
               READ isBusy
               NOTIFY busyChanged)
    // clang-format on
    bool isBusy() const { return m_reply != nullptr; }
    Q_SIGNAL void busyChanged();

    // clang-format off
    Q_PROPERTY(bool online
               READ isOnline
               NOTIFY onlineChanged)
    // clang-format on
    bool isOnline() const { return m_online; }
    Q_SIGNAL void onlineChanged();

    // clang-format off
    Q_PROPERTY(int interval
               READ interval
               WRITE setInterval
               NOTIFY intervalChanged)
    // clang-format on
    void setInterval(int val);
    int interval() const { return m_interval; }
    Q_SIGNAL void intervalChanged();

    // clang-format off
    Q_PROPERTY(QUrl url
               READ url
               WRITE setUrl
               NOTIFY urlChanged)
    // clang-format on
    void setUrl(const QUrl &val);
    QUrl url() const { return m_url; }
    Q_SIGNAL void urlChanged();

    // clang-format off
    Q_PROPERTY(QNetworkAccessManager *networkAccessManager
               READ networkAccessManager
               WRITE setNetworkAccessManager
               NOTIFY networkAccessManagerChanged)
    // clang-format on
    void setNetworkAccessManager(QNetworkAccessManager *val);
    QNetworkAccessManager *networkAccessManager() const { return m_networkAccessManager; }
    Q_SIGNAL void networkAccessManagerChanged();

private:
    void setOnline(bool val);

    void pingUrl();
    void onReplyFinished();

private:
    int m_interval = 5 * 60 * 1000; // default is 5 min
    bool m_online = false;
    QUrl m_url = QUrl(QStringLiteral("https://1.1.1.1"));
    QTimer *m_timer = nullptr;
    QNetworkReply *m_reply = nullptr;
    QNetworkAccessManager *m_networkAccessManager = nullptr;
};

#endif // NETWORKSTATUS_H
