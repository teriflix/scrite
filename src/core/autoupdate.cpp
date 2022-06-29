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

#include "autoupdate.h"
#include "application.h"
#include "garbagecollector.h"
#include "networkaccessmanager.h"

#include <QSettings>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QNetworkReply>

AutoUpdate *AutoUpdate::instance()
{
    static AutoUpdate *theInstance = new AutoUpdate(Application::instance());
    return theInstance;
}

AutoUpdate::AutoUpdate(QObject *parent) : QObject(parent), m_updateTimer("AutoUpdate.m_updateTimer")
{
    m_updateTimer.start(1000, this);
}

AutoUpdate::~AutoUpdate() { }

void AutoUpdate::setUrl(const QUrl &val)
{
    if (m_url == val)
        return;

    m_url = val;
    emit urlChanged();
}

QUrl AutoUpdate::updateDownloadUrl() const
{
    return QUrl(m_updateInfo.value("link").toString());
}

QUrl AutoUpdate::surveyUrl() const
{
    return QUrl(m_surveyInfo.value("link").toString());
}

void AutoUpdate::dontAskForSurveyAgain(bool val)
{
    Application::instance()->settings()->setValue("Survey/dontAskAgain", val);
}

void AutoUpdate::setUpdateInfo(const QJsonObject &val)
{
    if (m_updateInfo == val)
        return;

    m_updateInfo = val;

    const QString link = m_updateInfo.value("link").toString();
    if (!link.isEmpty()) {
        QUrl url(link);
        QUrlQuery uq(link);
        uq.addQueryItem("client", this->getClientId());
        url.setQuery(uq);
        m_updateInfo.insert("link", url.toString());
    }

    emit updateInfoChanged();
}

void AutoUpdate::setSurveyInfo(const QJsonObject &val)
{
    if (m_surveyInfo == val)
        return;

    QSettings *settings = Application::instance()->settings();

    /**
     * It would be a big turn off if the first thing that the user gets after
     * installing and running the app for the very first time is a survey
     * invite. Lets allow the user to use the app for alteast a few days
     * before we ask for survey participation.
     *
     * So, we give the user 2 days of 5 launches which ever is later.
     */

    static const int minDaysBeforeSurvey = 2;
    static const int minLaunchesBeforeSurvey = 5;

    const QDateTime now = QDateTime::currentDateTime();
    const bool allowSurvey =
            Application::instance()->installationTimestamp().daysTo(now) >= minDaysBeforeSurvey
            && Application::instance()->launchCounter() > minLaunchesBeforeSurvey;
    if (!allowSurvey)
        return;

    QString link = val.value("link").toString();
    if (link.isEmpty())
        return;

    QUrl url(link);
    QUrlQuery uq(url);
    uq.addQueryItem("client", this->getClientId());
    url.setQuery(uq);
    link = url.toString();

    const int surveyCounter = val.value("counter").toInt();
    const int lastSurveyCounter = settings->value("Survey/counter").toInt();
    const bool dontAskAgain = settings->value("Survey/dontAskAgain").toBool();
    if (lastSurveyCounter < surveyCounter || !dontAskAgain) {
        settings->setValue("Survey/counter", surveyCounter);
        settings->setValue("Survey/dontAskAgain", false);

        m_surveyInfo = val;
        m_surveyInfo.insert("link", link);

        emit surveyInfoChanged();
    }
}

void AutoUpdate::checkForUpdates()
{
    NetworkAccessManager &nam = *(NetworkAccessManager::instance());
    if (m_url.isEmpty() || !m_url.isValid())
        return;

    static QString userAgentString = this->getClientId();

    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgentString);
    QNetworkReply *reply = nam.get(request);
    if (reply == nullptr)
        return;

    connect(reply, &QNetworkReply::finished, this, [reply, this]() {
        if (reply->error() != QNetworkReply::NoError) {
            this->checkForUpdatesAfterSometime();
            return;
        }

        const QByteArray bytes = reply->readAll();
        if (bytes.isEmpty()) {
            this->checkForUpdatesAfterSometime();
            return;
        }

        const QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes);
        if (jsonDoc.isNull() || jsonDoc.isEmpty()) {
            this->checkForUpdatesAfterSometime();
            return;
        }

        const QJsonObject json = jsonDoc.object();
        this->lookForUpdates(json);
        this->lookForSurvey(json);
    });
}

void AutoUpdate::checkForUpdatesAfterSometime()
{
    // Check for updates after 1 hour
    m_updateTimer.start(60 * 60 * 1000, this);
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
    switch (Application::instance()->platform()) {
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

    if (info.isEmpty()) {
        this->checkForUpdatesAfterSometime();
        return;
    }

    const QVersionNumber updateVersion =
            QVersionNumber::fromString(info.value("version").toString());
    if (updateVersion.isNull()) {
        this->checkForUpdatesAfterSometime();
        return;
    }

    if (updateVersion <= Application::instance()->versionNumber()) {
        this->checkForUpdatesAfterSometime();
        return;
    }

    this->setUpdateInfo(info);
    // Dont check for updates until this update is used up.
}

void AutoUpdate::lookForSurvey(const QJsonObject &json)
{
    const QJsonObject info = json.value("survey").toObject();
    if (info.isEmpty())
        return;

    this->setSurveyInfo(info);
}

void AutoUpdate::timerEvent(QTimerEvent *event)
{
    if (m_updateTimer.timerId() == event->timerId()) {
        m_updateTimer.stop();
        this->checkForUpdates();
        return;
    }

    QObject::timerEvent(event);
}

QString AutoUpdate::getClientId() const
{
    static QString ret;
    if (ret.isEmpty()) {
        ret = "scrite-";
        ret += Application::instance()->applicationVersion() + "["
                + Application::instance()->buildTimestamp() + "] ";
        QString prodName = QSysInfo::prettyProductName() + "-" + QSysInfo::currentCpuArchitecture();
        prodName.replace(" ", "_");
        ret += prodName + " ";
        ret += Application::instance()->installationId();
    }

    return ret;
}
