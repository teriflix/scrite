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

#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QUrl>
#include <QObject>
#include <QQmlEngine>
#include <QJsonObject>

#include "execlatertimer.h"

class AutoUpdate : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static AutoUpdate *instance();
    ~AutoUpdate();

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    void setUrl(const QUrl &val);
    QUrl url() const { return m_url; }
    Q_SIGNAL void urlChanged();

    Q_PROPERTY(bool updateAvailable READ isUpdateAvailable NOTIFY updateInfoChanged)
    bool isUpdateAvailable() const { return !m_updateInfo.isEmpty(); }

    Q_PROPERTY(QUrl updateDownloadUrl READ updateDownloadUrl NOTIFY updateInfoChanged)
    QUrl updateDownloadUrl() const;

    Q_PROPERTY(QJsonObject updateInfo READ updateInfo WRITE setUpdateInfo NOTIFY updateInfoChanged)
    QJsonObject updateInfo() const { return m_updateInfo; }
    Q_SIGNAL void updateInfoChanged();

    Q_PROPERTY(bool surveyAvailable READ surveyAvailable NOTIFY surveyInfoChanged)
    bool surveyAvailable() const { return !m_surveyInfo.isEmpty(); }

    Q_PROPERTY(QJsonObject surveyInfo READ surveyInfo NOTIFY surveyInfoChanged)
    QJsonObject surveyInfo() const { return m_surveyInfo; }

    Q_PROPERTY(QUrl surveyUrl READ surveyUrl NOTIFY surveyInfoChanged)
    QUrl surveyUrl() const;

    Q_SIGNAL void surveyInfoChanged();

    Q_INVOKABLE void dontAskForSurveyAgain(bool val = true);

private:
    AutoUpdate(QObject *parent = nullptr);
    void setUpdateDownloadUrl(const QUrl &val);
    void setUpdateInfo(const QJsonObject &val);
    void setSurveyInfo(const QJsonObject &val);
    void checkForUpdates();
    void checkForUpdatesAfterSometime();
    void lookForUpdates(const QJsonObject &json);
    void lookForSurvey(const QJsonObject &json);
    void timerEvent(QTimerEvent *event);

    QString getClientId() const;

private:
    // URL has to be a http location always. Otherwise we will have to bundle
    // SSL libraries along with the installer and there are some legal bits
    // to consider before we are able to do that.
    QUrl m_url = QUrl("https://www.scrite.io/helpers/latest_release.json");
    QJsonObject m_updateInfo;
    QJsonObject m_surveyInfo;
    ExecLaterTimer m_updateTimer;
};

QML_DECLARE_TYPE(AutoUpdate)

#endif // AUTOUPDATE_H
