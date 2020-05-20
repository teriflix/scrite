/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "autoupdate.h"
#include "application.h"
#include "garbagecollector.h"

#include <QUuid>
#include <QSettings>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class NetworkAccessManager : public QNetworkAccessManager
{
public:
    static NetworkAccessManager *instance();
    ~NetworkAccessManager();

protected:
    // QNetworkAccessManager interface
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

private:
    static NetworkAccessManager *INSTANCE;
    NetworkAccessManager(QObject *parent=nullptr);
    void onReplyFinished(QNetworkReply *reply);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    QList<QNetworkReply*> m_replies;
};

NetworkAccessManager *NetworkAccessManager::INSTANCE = nullptr;

NetworkAccessManager *NetworkAccessManager::instance()
{
    if(NetworkAccessManager::INSTANCE)
        return NetworkAccessManager::INSTANCE;

    return new NetworkAccessManager(qApp);
}

NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
    NetworkAccessManager::INSTANCE = this;
    connect(this, &QNetworkAccessManager::finished, this, &NetworkAccessManager::onReplyFinished);
    connect(this, &QNetworkAccessManager::sslErrors, this, &NetworkAccessManager::onSslErrors);
}

NetworkAccessManager::~NetworkAccessManager()
{
    NetworkAccessManager::INSTANCE = nullptr;
}

QNetworkReply *NetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QNetworkReply *reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
    if(reply)
        m_replies.append(reply);

    return reply;
}

void NetworkAccessManager::onReplyFinished(QNetworkReply *reply)
{
    if( reply != nullptr && m_replies.removeOne(reply) )
    {
        GarbageCollector::instance()->add(reply);

        if(m_replies.isEmpty())
        {
            NetworkAccessManager::INSTANCE = nullptr;
            GarbageCollector::instance()->add(this);
        }
    }
}

void NetworkAccessManager::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    reply->ignoreSslErrors(errors);
}

AutoUpdate *AutoUpdate::instance()
{
    static AutoUpdate *theInstance = new AutoUpdate(Application::instance());
    return theInstance;
}

AutoUpdate::AutoUpdate(QObject *parent)
    : QObject(parent),
      m_updateTimer("AutoUpdate.m_updateTimer")
{
    m_updateTimer.start(1000, this);
}

AutoUpdate::~AutoUpdate()
{

}

void AutoUpdate::setUrl(const QUrl &val)
{
    if(m_url == val)
        return;

    m_url = val;
    emit urlChanged();
}

QUrl AutoUpdate::updateDownloadUrl() const
{
    return QUrl(m_updateInfo.value("link").toString());
}

void AutoUpdate::setUpdateInfo(const QJsonObject &val)
{
    if(m_updateInfo == val)
        return;

    m_updateInfo = val;

    const QString link = m_updateInfo.value("link").toString();
    if(!link.isEmpty())
    {
        QUrl url(link);
        QUrlQuery uq;
        uq.addQueryItem("client", this->getClientId());
        url.setQuery(uq);
        m_updateInfo.insert("link", url.toString());
    }

    emit updateInfoChanged();
}

void AutoUpdate::checkForUpdates()
{
    NetworkAccessManager &nam = *(NetworkAccessManager::instance());
    if(m_url.isEmpty() || !m_url.isValid())
        return;

    static QString userAgentString = this->getClientId();

    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgentString);
    QNetworkReply *reply = nam.get(request);
    if(reply == nullptr)
        return;

    connect(reply, &QNetworkReply::finished, [reply,this]() {
        if(reply->error() != QNetworkReply::NoError) {
            this->checkForUpdatesAfterSometime();
            return;
        }

        const QByteArray bytes = reply->readAll();
        if(bytes.isEmpty()) {
            this->checkForUpdatesAfterSometime();
            return;
        }

        const QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes);
        if(jsonDoc.isNull() || jsonDoc.isEmpty()) {
            this->checkForUpdatesAfterSometime();
            return;
        }

        const QJsonObject json = jsonDoc.object();
        this->lookForUpdates(json);
    });
}

void AutoUpdate::checkForUpdatesAfterSometime()
{
    // Check for updates after 1 hour
    m_updateTimer.start(60*60*1000, this);
}

void AutoUpdate::lookForUpdates(const QJsonObject &json)
{
    /**
      {
        "macos": {
            "version": "0.2.7",
            "versionString": "0.2.7-beta",
            "releaseDate": "....",
            "changeLog": "....",
            "link": "...."
        },
        "windows": {

        },
        "linux": {

        }
      }
      */

    QJsonObject info;
    switch(Application::instance()->platform())
    {
    case Application::MacOS:
        info = json.value("macos").toObject();
        break;
    case Application::LinuxDesktop:
        info = json.value("linux").toObject();
        break;
    case Application::WindowsDesktop:
        info = json.value("windows").toObject();
        break;
    }

    if(info.isEmpty())
    {
        this->checkForUpdatesAfterSometime();
        return;
    }

    const QVersionNumber updateVersion = QVersionNumber::fromString(info.value("version").toString());
    if(updateVersion.isNull())
    {
        this->checkForUpdatesAfterSometime();
        return;
    }

    if(updateVersion <= Application::instance()->versionNumber())
    {
        this->checkForUpdatesAfterSometime();
        return;
    }

    this->setUpdateInfo(info);
    // Dont check for updates until this update is used up.
}

void AutoUpdate::timerEvent(QTimerEvent *event)
{
    if(m_updateTimer.timerId() == event->timerId())
    {
        m_updateTimer.stop();
        this->checkForUpdates();
        return;
    }

    QObject::timerEvent(event);
}

QString AutoUpdate::getClientId() const
{
    static QString ret;
    if(ret.isEmpty())
    {
        ret = "scrite-";
        ret += Application::instance()->applicationVersion() + " ";

        QString prodName = QSysInfo::prettyProductName();
        prodName.replace(" ", "_");
        ret += prodName + " ";

        QSettings *settings = Application::instance()->settings();
        QString clientID = settings->value("Installation/ClientID").toString();
        if(clientID.isEmpty())
        {
            clientID = QUuid::createUuid().toString();
            settings->setValue("Installation/ClientID", clientID);
        }

        ret += clientID;
    }

    return ret;
}


