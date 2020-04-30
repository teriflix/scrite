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

#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QUrl>
#include <QObject>
#include <QBasicTimer>
#include <QJsonObject>

class AutoUpdate : public QObject
{
    Q_OBJECT

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

private:
    AutoUpdate(QObject *parent=nullptr);
    void setUpdateDownloadUrl(const QUrl &val);
    void setUpdateInfo(const QJsonObject &val);
    void checkForUpdates();
    void checkForUpdatesAfterSometime();
    void lookForUpdates(const QJsonObject &json);
    void timerEvent(QTimerEvent *event);

    QString getClientId() const;

private:
    // URL has to be a http location always. Otherwise we will have to bundle
    // SSL libraries along with the installer and there are some legal bits
    // to consider before we are able to do that.
    QUrl m_url = QUrl("http://www.teriflix.in/scrite/latest_release.json");
    QJsonObject m_updateInfo;
    QBasicTimer m_updateTimer;
};

#endif // AUTOUPDATE_H
