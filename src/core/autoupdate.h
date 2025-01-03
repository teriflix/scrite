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

    Q_PROPERTY(bool updateAvailable READ isUpdateAvailable NOTIFY updateInfoChanged)
    bool isUpdateAvailable() const { return !m_updateInfo.isEmpty(); }

    Q_PROPERTY(QUrl updateDownloadUrl READ updateDownloadUrl NOTIFY updateInfoChanged)
    QUrl updateDownloadUrl() const;

    Q_PROPERTY(QJsonObject updateInfo READ updateInfo WRITE setUpdateInfo NOTIFY updateInfoChanged)
    QJsonObject updateInfo() const { return m_updateInfo; }
    Q_SIGNAL void updateInfoChanged();

private:
    AutoUpdate(QObject *parent = nullptr);

    void setUpdateInfo(const QJsonObject &val);

    void checkForUpdates();
    void checkForUpdatesAfterSometime();

private:
    QUrl m_url;
    QJsonObject m_updateInfo;
    QJsonObject m_surveyInfo;
};

QML_DECLARE_TYPE(AutoUpdate)

#endif // AUTOUPDATE_H
