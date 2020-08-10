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

#include "hourglass.h"
#include "urlattributes.h"
#include "garbagecollector.h"

#include <QFile>
#include <QTimer>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QJsonDocument>

UrlAttributes::UrlAttributes(QObject *parent)
    : QObject(parent)
{
}

UrlAttributes::~UrlAttributes()
{

}

void UrlAttributes::setUrl(const QUrl &val)
{
    if(m_url == val)
        return;

    m_url = val;
    emit urlChanged();

    static QNetworkAccessManager nam;
    if(m_reply != nullptr)
        delete m_reply;
    m_reply = nullptr;

    if(!m_url.isEmpty() && m_url.isValid())
    {
        this->setAttributes(this->createDefaultAttributes());
        this->setStatus(Loading);

        static const QUrl url( QStringLiteral("http://www.teriflix.in/scrite/urlattribs/urlattribs.php") );
        const QNetworkRequest request(url);

        QUrlQuery postData;
        postData.addQueryItem("url", m_url.toString());

        const QByteArray postDataBytes = postData.toString(QUrl::FullyEncoded).toLatin1();

        m_reply = nam.post(request, postDataBytes);
        if(m_reply != nullptr)
        {
            connect(m_reply, &QNetworkReply::finished, this, &UrlAttributes::onHttpRequestFinished);
            connect(m_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                [=](QNetworkReply::NetworkError){
                m_reply->deleteLater();
                m_reply = nullptr;
                this->setStatus(Error);
            });
        }
    }
    else
    {
        this->setAttributes(QJsonObject());
        this->setStatus(Ready);
    }
}

void UrlAttributes::setStatus(UrlAttributes::Status val)
{
    if(m_status == val)
        return;

    m_status = val;
    emit statusChanged();
}

void UrlAttributes::onHttpRequestFinished()
{
    if(m_reply == nullptr)
        return;

    const QByteArray bytes = m_reply->readAll();
    m_reply->deleteLater();
    m_reply = nullptr;

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes);
    const QJsonObject jsonObj = jsonDoc.object();
    this->setAttributes(jsonObj);
    this->setStatus(Ready);
}

void UrlAttributes::setAttributes(const QJsonObject &val)
{
    if(m_attributes == val)
        return;

    m_attributes = val;
    emit attributesChanged();
}

QJsonObject UrlAttributes::createDefaultAttributes() const
{
    QJsonObject defaultAttrs;
    defaultAttrs.insert(QStringLiteral("url"), m_url.toString());
    defaultAttrs.insert(QStringLiteral("type"), QStringLiteral("website"));
    defaultAttrs.insert(QStringLiteral("title"), QStringLiteral("Website URL"));
    defaultAttrs.insert(QStringLiteral("image"), QStringLiteral(""));
    defaultAttrs.insert(QStringLiteral("description"), QStringLiteral(""));
    return defaultAttrs;
}


