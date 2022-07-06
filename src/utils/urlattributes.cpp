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

#include "hourglass.h"
#include "application.h"
#include "urlattributes.h"
#include "garbagecollector.h"
#include "networkaccessmanager.h"

#include <QFile>
#include <QTimer>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

UrlAttributes::UrlAttributes(QObject *parent) : QObject(parent) { }

UrlAttributes::~UrlAttributes() { }

void UrlAttributes::setUrl(const QUrl &val)
{
    if (m_url == val)
        return;

    m_url = val;
    emit urlChanged();

    NetworkAccessManager &nam = *NetworkAccessManager::instance();
    if (!m_reply.isNull())
        delete m_reply;
    m_reply = nullptr;

    if (!m_url.isEmpty() && m_url.isValid()) {
        this->setAttributes(this->createDefaultAttributes());
        this->setStatus(Loading);

        static const QUrl url(
                QStringLiteral("https://www.scrite.io/helpers/urlattribs/urlattribs.php"));
        const QNetworkRequest request(url);

        QUrlQuery postData;
        postData.addQueryItem("url", m_url.toString());

        const QByteArray postDataBytes = postData.toString(QUrl::FullyEncoded).toLatin1();

        m_reply = nam.post(request, postDataBytes);
        if (m_reply != nullptr) {
            connect(m_reply, &QNetworkReply::finished, this, &UrlAttributes::onHttpRequestFinished);
            connect(m_reply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError) {
                m_reply->deleteLater();
                m_reply = nullptr;
                this->setStatus(Error);
            });
        }
    } else {
        this->setAttributes(QJsonObject());
        this->setStatus(Ready);
    }
}

void UrlAttributes::setStatus(UrlAttributes::Status val)
{
    if (m_status == val)
        return;

    m_status = val;
    emit statusChanged();
}

void UrlAttributes::onHttpRequestFinished()
{
    if (m_reply == nullptr)
        return;

    const QByteArray bytes = m_reply->readAll();
    m_reply->deleteLater();
    m_reply = nullptr;

    QJsonParseError parseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes, &parseError);
    QJsonObject jsonObj =
            parseError.error == QJsonParseError::NoError ? jsonDoc.object() : QJsonObject();
    if (jsonObj.isEmpty())
        jsonObj = this->createDefaultAttributes();
    this->setAttributes(jsonObj);
    this->setStatus(Ready);
}

void UrlAttributes::setAttributes(const QJsonObject &val)
{
    if (m_attributes == val)
        return;

    m_attributes = val;
    if (m_attributes.value(QStringLiteral("url")).toString() != m_url.toString())
        m_attributes.insert(QStringLiteral("url"), m_url.toString());
    emit attributesChanged();
}

QJsonObject UrlAttributes::createDefaultAttributes() const
{
    QJsonObject defaultAttrs;
    defaultAttrs.insert(QStringLiteral("url"), m_url.toString());
    defaultAttrs.insert(QStringLiteral("type"), QStringLiteral("website"));
    defaultAttrs.insert(QStringLiteral("title"), QStringLiteral("Website URL"));
    defaultAttrs.insert(QStringLiteral("image"), QLatin1String(""));
    defaultAttrs.insert(QStringLiteral("description"), QLatin1String(""));
    return defaultAttrs;
}
