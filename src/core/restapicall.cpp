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

#include "restapicall.h"
#include "application.h"
#include "restapikey/restapikey.h"
#include "networkaccessmanager.h"

#include <QSysInfo>
#include <QUrlQuery>
#include <QSettings>
#include <QJsonValue>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QOperatingSystemVersion>

inline QJsonValue jsonFetch(const QJsonObject &object, const QString &attr)
{
    return object.contains(attr) ? object.value(attr) : QJsonValue();
}

RestApiCall::RestApiCall(QObject *parent) : QObject(parent)
{
    this->setKey( this->defaultKey() );
    this->setToken( this->sessionToken() );
}

RestApiCall::~RestApiCall()
{

}

void RestApiCall::setType(Type val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void RestApiCall::setHost(const QUrl &val)
{
    if(m_host == val)
        return;

    m_host = val;
    emit hostChanged();
}

void RestApiCall::setRoot(const QString &val)
{
    if(m_root == val)
        return;

    m_root = val;
    emit rootChanged();
}

void RestApiCall::setApi(const QString &val)
{
    if(m_api == val)
        return;

    m_api = val;
    emit apiChanged();
}

void RestApiCall::setData(const QJsonObject &val)
{
    if(m_data == val)
        return;

    m_data = val;
    emit dataChanged();
}

QString RestApiCall::defaultKey()
{
    return QStringLiteral(REST_API_KEY);
}

void RestApiCall::setKey(const QString &val)
{
    if(m_key == val)
        return;

    m_key = val;
    emit keyChanged();
}

QString RestApiCall::loginToken()
{
    const QSettings *settings = Application::instance()->settings();
    return settings->value( QStringLiteral("RestApi/loginToken") ).toString();
}

QString RestApiCall::sessionToken()
{
    const QSettings *settings = Application::instance()->settings();
    return settings->value( QStringLiteral("RestApi/sessionToken") ).toString();
}

void RestApiCall::updateTokensFromResponse()
{
    const QString ltoken = ::jsonFetch(this->responseData(), QStringLiteral("loginToken") ).toString();
    const QString stoken = ::jsonFetch(this->responseData(), QStringLiteral("sessionToken") ).toString();

    QSettings *settings = Application::instance()->settings();
    if(!ltoken.isEmpty())
        settings->setValue( QStringLiteral("RestApi/loginToken"), ltoken );
    if(!stoken.isEmpty())
        settings->setValue( QStringLiteral("RestApi/sessionToken"), stoken );
}

void RestApiCall::setToken(const QString &val)
{
    if(m_token == val)
        return;

    m_token = val;
    emit tokenChanged();
}

QString RestApiCall::clientId()
{
    return Application::instance()->installationId();
}

QString RestApiCall::deviceId()
{
    return QString::fromLatin1( QSysInfo::machineUniqueId().toHex() );
}

QString RestApiCall::platform()
{
    switch(Application::instance()->platform())
    {
    case Application::WindowsDesktop:
        return QStringLiteral("Windows");
    case Application::LinuxDesktop:
        return QStringLiteral("Linux");
    case Application::MacOS:
        return QStringLiteral("Mac");
    default:
        break;
    }

    return QStringLiteral("Unknown");
}

QString RestApiCall::platformVersion()
{
    return QOperatingSystemVersion::current().name();
}

QString RestApiCall::platformType()
{
    if(QSysInfo::WordSize == 32)
        return QStringLiteral("x32");

    return QStringLiteral("x64");
}

QString RestApiCall::appVersion()
{
    return Application::instance()->applicationVersion();
}

QString RestApiCall::responseCode() const
{
    return ::jsonFetch(m_response, QStringLiteral("code")).toString();
}

QString RestApiCall::responseText() const
{
    return ::jsonFetch(m_response, QStringLiteral("text")).toString();
}

QJsonObject RestApiCall::responseData() const
{
    return ::jsonFetch(m_response, QStringLiteral("data")).toObject();
}

QString RestApiCall::errorCode() const
{
    return ::jsonFetch(m_response, QStringLiteral("code")).toString();
}

QString RestApiCall::errorText() const
{
    return ::jsonFetch(m_response, QStringLiteral("text")).toString();
}

QJsonObject RestApiCall::errorData() const
{
    return ::jsonFetch(m_response, QStringLiteral("data")).toObject();
}

bool RestApiCall::call()
{
    if(m_api.isEmpty() || m_reply != nullptr)
        return false;

    const QString path = m_root.isEmpty() ? m_api : m_root + QStringLiteral("/") + m_api;

    QUrl url = m_host;
    url.setPath(path);
    if(m_type == GET && !m_data.isEmpty())
    {
        QUrlQuery uq;
        QJsonObject::iterator it = m_data.begin();
        QJsonObject::iterator end = m_data.end();
        while(it != end)
        {
            uq.addQueryItem(it.key(), it.value().toString());
            ++it;
        }

        url.setQuery(uq);
    }

    QNetworkRequest req(url);
    if( !m_key.isEmpty() )
        req.setRawHeader( QByteArrayLiteral("key"), m_key.toLatin1() );
    if( !m_token.isEmpty() )
        req.setRawHeader( QByteArrayLiteral("token"), m_token.toLatin1() );

    static const QString userAgentString = []() {
        const QString ret = QStringLiteral("scrite-") + RestApiCall::appVersion() +
                            QStringLiteral("-") + RestApiCall::platform() +
                            QStringLiteral("-") + RestApiCall::platformVersion() +
                            QStringLiteral("-") + RestApiCall::platformType();
        return ret;
    } ();
    if( !userAgentString.isEmpty() )
        req.setHeader(QNetworkRequest::UserAgentHeader, userAgentString);

    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, 0);
    req.setRawHeader( QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json") );
    req.setRawHeader( QByteArrayLiteral("Accept-Encoding"), QByteArrayLiteral("identity") );

    NetworkAccessManager *nam = NetworkAccessManager::instance();
    if(m_type == GET)
        m_reply = nam->get(req);
    else if(m_type == POST)
    {
        if(!m_data.isEmpty())
        {
            const QByteArray bytes = QJsonDocument(m_data).toJson(QJsonDocument::Compact);
            req.setHeader(QNetworkRequest::ContentLengthHeader, bytes.length());
            m_reply = nam->post(req, bytes);
        }
    }

    if(m_reply)
    {
        connect(m_reply, &QNetworkReply::finished, this, &RestApiCall::onNetworkReplyFinished);
        connect(m_reply,  QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                this, &RestApiCall::onNetworkReplyError);
        emit busyChanged();
    }

    return true;
}

void RestApiCall::setError(const QJsonObject &val)
{
    if(m_error == val)
        return;

    m_error = val;
    emit errorChanged();
}

void RestApiCall::setResponse(const QJsonObject &val)
{
    if(m_response == val)
        return;

    m_response = val;
    emit responseChanged();
}

void RestApiCall::onNetworkReplyError()
{
    if(m_reply->error() == QNetworkReply::NoError)
        return;

    QJsonObject json;
    json.insert( QStringLiteral("code"), QStringLiteral("E_NETWORK") );
    json.insert( QStringLiteral("text"), QStringLiteral("Error making network call.") );
    this->setError(json);

    m_reply->deleteLater();
    m_reply = nullptr;
    emit busyChanged();

    emit finished();
}

void RestApiCall::onNetworkReplyFinished()
{
    if(m_reply->error() == QNetworkReply::NoError)
    {
        const QByteArray bytes = m_reply->readAll();
        const QJsonObject json = QJsonDocument::fromJson(bytes).object();
        const QString errorAttr = QStringLiteral("error");
        const QString responseAttr = QStringLiteral("response");

        if( json.contains(errorAttr) )
            this->setError(json.value(errorAttr).toObject());
        else if( json.contains(responseAttr) )
            this->setResponse(json.value(responseAttr).toObject());

        m_reply->deleteLater();
        m_reply = nullptr;
        emit busyChanged();

        emit finished();
    }
}
