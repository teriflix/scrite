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

#include "callgraph.h"
#include "autoupdate.h"
#include "application.h"

#include <QSettings>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QNetworkReply>

#include "restapicall.h"

AutoUpdate *AutoUpdate::instance()
{
    // CAPTURE_FIRST_CALL_GRAPH;
    static AutoUpdate *theInstance = new AutoUpdate(Application::instance());
    return theInstance;
}

AutoUpdate::AutoUpdate(QObject *parent) : QObject(parent)
{
    // CAPTURE_CALL_GRAPH;
    QTimer::singleShot(1000, this, &AutoUpdate::checkForUpdates);
}

AutoUpdate::~AutoUpdate() { }

QUrl AutoUpdate::updateDownloadUrl() const
{
    return QUrl(m_updateInfo.value("link").toString());
}

void AutoUpdate::setUpdateInfo(const QJsonObject &val)
{
    if (m_updateInfo == val)
        return;

    m_updateInfo = val;
    emit updateInfoChanged();
}

void AutoUpdate::checkForUpdates()
{
    AppLatestReleaseRestApiCall *api = new AppLatestReleaseRestApiCall(this);
    api->setAutoDelete(true);
    api->setReportNetworkErrors(false);

    connect(api, &AppLatestReleaseRestApiCall::finished, this, [=]() {
        if (api->hasError() || !api->hasResponse())
            this->checkForUpdatesAfterSometime();
        else
            this->setUpdateInfo(api->update());
    });

    if (!api->call()) {
        api->deleteLater();
        this->checkForUpdatesAfterSometime();
    }
}

void AutoUpdate::checkForUpdatesAfterSometime()
{
    // Check for updates after 1 hour
    QTimer::singleShot(60 * 60 * 1000, this, &AutoUpdate::checkForUpdates);
}
