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

#include "application.h"
#include "jsonhttprequest.h"
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

JsonHttpRequest::JsonHttpRequest(QObject *parent) : QObject(parent)
{
    this->setKey( this->defaultKey() );
    this->setToken( this->sessionToken() );

    connect(this, &JsonHttpRequest::finished, [=]() {
        if(!m_isQmlInstance && m_autoDelete)
            this->deleteLater();
    });
}

JsonHttpRequest::~JsonHttpRequest()
{

}

bool JsonHttpRequest::autoDelete() const
{
    return m_autoDelete;
}

void JsonHttpRequest::setAutoDelete(bool val)
{
    m_autoDelete = val;
}

void JsonHttpRequest::setType(Type val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void JsonHttpRequest::setHost(const QUrl &val)
{
    if(m_host == val)
        return;

    m_host = val;
    emit hostChanged();
}

void JsonHttpRequest::setRoot(const QString &val)
{
    if(m_root == val)
        return;

    m_root = val;
    emit rootChanged();
}

void JsonHttpRequest::setApi(const QString &val)
{
    if(m_api == val)
        return;

    m_api = val;
    emit apiChanged();
}

void JsonHttpRequest::setData(const QJsonObject &val)
{
    if(m_data == val)
        return;

    m_data = val;
    emit dataChanged();
}

QString JsonHttpRequest::defaultKey()
{    
    return QStringLiteral(REST_API_KEY);
}

void JsonHttpRequest::setKey(const QString &val)
{
    if(m_key == val)
        return;

    m_key = val;
    emit keyChanged();
}

QString JsonHttpRequest::loginToken()
{
    return fetch( QStringLiteral("loginToken") ).toString();
}

QString JsonHttpRequest::sessionToken()
{
    return fetch( QStringLiteral("sessionToken") ).toString();
}

static QString & SessionToken()
{
    static QString TheSessionToken;
    return TheSessionToken;
}

void JsonHttpRequest::store(const QString &key, const QVariant &value)
{
    if(key == QStringLiteral("sessionToken"))
        ::SessionToken() = value.toString();
    else
    {
        QSettings *settings = Application::instance()->settings();
        settings->setValue( QStringLiteral("RestApi/") + key, value );
    }
}

QVariant JsonHttpRequest::fetch(const QString &key)
{
    if(key == QStringLiteral("sessionToken"))
        return ::SessionToken();

    const QSettings *settings = Application::instance()->settings();
    return settings->value( QStringLiteral("RestApi/") + key );
}

void JsonHttpRequest::setToken(const QString &val)
{
    if(m_token == val)
        return;

    m_token = val;
    emit tokenChanged();
}

QString JsonHttpRequest::email()
{
    return fetch( QStringLiteral("email") ).toString();
}

QString JsonHttpRequest::clientId()
{
    return Application::instance()->installationId();
}

QString JsonHttpRequest::deviceId()
{
    return QString::fromLatin1( QSysInfo::machineUniqueId().toHex() );
}

QString JsonHttpRequest::platform()
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

QString JsonHttpRequest::platformVersion()
{
    return QOperatingSystemVersion::current().name();
}

QString JsonHttpRequest::platformType()
{
    if(QSysInfo::WordSize == 32)
        return QStringLiteral("x32");

    return QStringLiteral("x64");
}

QString JsonHttpRequest::appVersion()
{
    return Application::instance()->applicationVersion();
}

QString JsonHttpRequest::responseCode() const
{
    return ::jsonFetch(m_response, QStringLiteral("code")).toString();
}

QString JsonHttpRequest::responseText() const
{
    return ::jsonFetch(m_response, QStringLiteral("text")).toString();
}

QJsonObject JsonHttpRequest::responseData() const
{
    return ::jsonFetch(m_response, QStringLiteral("data")).toObject();
}

QString JsonHttpRequest::errorCode() const
{
    return ::jsonFetch(m_error, QStringLiteral("code")).toString();
}

QString JsonHttpRequest::errorText() const
{
    return ::jsonFetch(m_error, QStringLiteral("text")).toString();
}

QJsonObject JsonHttpRequest::errorData() const
{
    return ::jsonFetch(m_error, QStringLiteral("data")).toObject();
}

bool JsonHttpRequest::call()
{
    if(m_api.isEmpty() || m_reply != nullptr)
        return false;

    this->clearError();
    this->clearResponse();

    emit aboutToCall();

    QString path = QStringLiteral("/") + m_root + QStringLiteral("/") + m_api;
    path = path.replace(QRegExp(QStringLiteral("/+")), QStringLiteral("/"));

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
        const QString ret = QStringLiteral("scrite-") + JsonHttpRequest::appVersion() +
                            QStringLiteral("-") + JsonHttpRequest::platform() +
                            QStringLiteral("-") + JsonHttpRequest::platformVersion() +
                            QStringLiteral("-") + JsonHttpRequest::platformType();
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
        QByteArray bytes;
        if(!m_data.isEmpty())
        {
            bytes = QJsonDocument(m_data).toJson(QJsonDocument::Compact);
            req.setHeader(QNetworkRequest::ContentLengthHeader, bytes.length());
        }
        m_reply = nam->post(req, bytes);
    }

    if(m_reply)
    {
        qDebug() << "PA: Call Issued - " << m_type << m_api;

        emit justIssuedCall();
        emit busyChanged();

        connect(m_reply, &QNetworkReply::finished, this, &JsonHttpRequest::onNetworkReplyFinished);
        connect(m_reply,  QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                this, &JsonHttpRequest::onNetworkReplyError);
    }

    return true;
}

void JsonHttpRequest::setError(const QJsonObject &val)
{
    if(m_error == val)
        return;

    m_error = val;
    emit errorChanged();
}

void JsonHttpRequest::setResponse(const QJsonObject &val)
{
    if(m_response == val)
        return;

    m_response = val;
    emit responseChanged();
}

void JsonHttpRequest::onNetworkReplyError()
{
    if(m_reply->error() == QNetworkReply::NoError)
        return;

    disconnect(m_reply, &QNetworkReply::finished, this, &JsonHttpRequest::onNetworkReplyFinished);

    const QString code = Application::instance()->enumerationKey(m_reply, "NetworkError", m_reply->error());
    const QString msg = m_reply->errorString();
    emit networkError(code, msg);

    m_reply->deleteLater();
    m_reply = nullptr;
    emit busyChanged();

    emit finished();
}

void JsonHttpRequest::onNetworkReplyFinished()
{
    if(m_reply->error() == QNetworkReply::NoError)
    {
        const QByteArray bytes = m_reply->readAll();
        const QJsonObject json = QJsonDocument::fromJson(bytes).object();
        const QString errorAttr = QStringLiteral("error");
        const QString responseAttr = QStringLiteral("response");

        qDebug() << "PA: Response Received - " << m_type << m_api;

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
