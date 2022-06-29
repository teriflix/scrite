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

#include "application.h"
#include "simplecrypt.h"
#include "jsonhttprequest.h"
#include "networkaccessmanager.h"

#include <QUuid>
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

static QString &SessionToken()
{
    static QString TheSessionToken;
    return TheSessionToken;
}

JsonHttpRequest::JsonHttpRequest(QObject *parent) : QObject(parent)
{
    this->setKey(this->defaultKey());
    this->setToken(this->sessionToken());

    connect(this, &JsonHttpRequest::finished, [=]() {
        if (!m_isQmlInstance && m_autoDelete)
            this->deleteLater();
    });

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
    qDebug() << "PA: ";
#endif
}

JsonHttpRequest::~JsonHttpRequest()
{
#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
    qDebug() << "PA: " << m_type << m_api;
#endif
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
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void JsonHttpRequest::setHost(const QUrl &val)
{
    if (m_host == val)
        return;

    m_host = val;
    emit hostChanged();
}

void JsonHttpRequest::setRoot(const QString &val)
{
    if (m_root == val)
        return;

    m_root = val;
    emit rootChanged();
}

void JsonHttpRequest::setApi(const QString &val)
{
    if (m_api == val)
        return;

    m_api = val;
    emit apiChanged();
}

void JsonHttpRequest::setData(const QJsonObject &val)
{
    if (m_data == val)
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
    if (m_key == val)
        return;

    m_key = val;
    emit keyChanged();
}

QString JsonHttpRequest::loginToken()
{
    return fetch(QStringLiteral("loginToken")).toString();
}

QString JsonHttpRequest::sessionToken()
{
    return ::SessionToken();
}

void JsonHttpRequest::store(const QString &key, const QVariant &value)
{
    if (key == QStringLiteral("sessionToken"))
        ::SessionToken() = value.toString();
    else {
        QSettings *settings = Application::instance()->settings();
        const QString key2 = QStringLiteral("Registration/") + key;
        if (value.isValid())
            settings->setValue(key2, value);
        else
            settings->remove(key2);
    }
}

QVariant JsonHttpRequest::fetch(const QString &key)
{
    if (key == QStringLiteral("sessionToken"))
        return ::SessionToken();

    const QSettings *settings = Application::instance()->settings();
    return settings->value(QStringLiteral("Registration/") + key);
}

void JsonHttpRequest::setToken(const QString &val)
{
    if (m_token == val)
        return;

    m_token = val;
    emit tokenChanged();
}

QString JsonHttpRequest::email()
{
    return fetch(QStringLiteral("email")).toString().toLower();
}

QString JsonHttpRequest::clientId()
{
    return Application::instance()->installationId();
}

QString JsonHttpRequest::deviceId()
{
    static QString ret;
    if (ret.isEmpty()) {
        ret = QString::fromLatin1(QSysInfo::machineUniqueId().toHex());
        if (!ret.isEmpty())
            return ret;

        const QString deviceIdKey = QStringLiteral("deviceId");
        ret = fetch(deviceIdKey).toString();
        if (!ret.isEmpty())
            return ret;

        ret = QUuid::createUuid().toString();
        store(deviceIdKey, ret);
    }

    return ret;
}

QString JsonHttpRequest::platform()
{
    switch (Application::instance()->platform()) {
    case Application::WindowsDesktop:
        return QStringLiteral("Windows");
    case Application::LinuxDesktop:
        return QStringLiteral("Linux");
    case Application::MacOS:
        return QStringLiteral("macOS");
    }

    return QStringLiteral("Unknown");
}

QString JsonHttpRequest::platformVersion()
{
#ifdef Q_OS_MAC
    const auto osvername = QSysInfo::productVersion();
#else
#ifdef Q_OS_WIN
    const auto osvername = QSysInfo::productVersion();
#else
    const auto osvername = QSysInfo::prettyProductName();
#endif
#endif

    if (!osvername.isEmpty()) {
#ifdef Q_OS_MAC
        static QHash<QString, QString> macOSVersionMap = {
            { QStringLiteral("10.12"), QStringLiteral("Sierra") },
            { QStringLiteral("10.13"), QStringLiteral("High Sierra") },
            { QStringLiteral("10.14"), QStringLiteral("Mojave") },
            { QStringLiteral("10.15"), QStringLiteral("Catalina") },
            { QStringLiteral("10.16"), QStringLiteral("Big Sur or Monterey") },
            { QStringLiteral("10.17"), QStringLiteral("Monterey") },
        };
        return macOSVersionMap.value(osvername, osvername);
#else
        return osvername;
#endif
    }

    const auto osver = QOperatingSystemVersion::current();
    return QVersionNumber(osver.majorVersion(), osver.minorVersion(), osver.microVersion())
            .toString();
}

QString JsonHttpRequest::platformType()
{
    if (QSysInfo::WordSize == 32)
        return QStringLiteral("x86");

    return QStringLiteral("x64");
}

QString JsonHttpRequest::appVersion()
{
    return Application::instance()->applicationVersion();
}

QString JsonHttpRequest::encrypt(const QString &text)
{
    SimpleCrypt sc(REST_CRYPT_KEY);
    return sc.encryptToString(text);
}

QString JsonHttpRequest::decrypt(const QString &text)
{
    SimpleCrypt sc(REST_CRYPT_KEY);
    return sc.decryptToString(text);
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

QString JsonHttpRequest::errorMessage() const
{
    return QStringLiteral("%1: %2").arg(this->errorCode(), this->errorText());
}

QJsonObject JsonHttpRequest::errorData() const
{
    return ::jsonFetch(m_error, QStringLiteral("data")).toObject();
}

void JsonHttpRequest::setReportNetworkErrors(bool val)
{
    if (m_reportNetworkErrors == val)
        return;

    m_reportNetworkErrors = val;
    emit reportNetworkErrorsChanged();
}

bool JsonHttpRequest::call()
{
    if (m_api.isEmpty() || m_reply != nullptr)
        return false;

    this->clearError();
    this->clearResponse();

    emit aboutToCall();

    QString path = QStringLiteral("/") + m_root + QStringLiteral("/") + m_api;
    path = path.replace(QRegExp(QStringLiteral("/+")), QStringLiteral("/"));

    QUrl url = m_host;
    url.setPath(path);
    if (m_type == GET && !m_data.isEmpty()) {
        QUrlQuery uq;
        QJsonObject::iterator it = m_data.begin();
        QJsonObject::iterator end = m_data.end();
        while (it != end) {
            uq.addQueryItem(it.key(), it.value().toString());
            ++it;
        }

        url.setQuery(uq);
    }

    QNetworkRequest req(url);
    if (!m_key.isEmpty())
        req.setRawHeader(QByteArrayLiteral("key"), m_key.toLatin1());
    if (!m_token.isEmpty()) {
        req.setRawHeader(QByteArrayLiteral("token"), m_token.toLatin1());
        req.setRawHeader(QByteArrayLiteral("client-id"), clientId().toLatin1());
        req.setRawHeader(QByteArrayLiteral("device-id"), deviceId().toLatin1());
    }

    static const QString userAgentString = []() {
        const QString space = QStringLiteral(" ");
        const QString ret = QStringLiteral("scrite-") + JsonHttpRequest::appVersion() + space
                + JsonHttpRequest::platform() + space + JsonHttpRequest::platformVersion() + space
                + JsonHttpRequest::platformType() + space + JsonHttpRequest::clientId();
        return ret;
    }();

    if (!userAgentString.isEmpty())
        req.setHeader(QNetworkRequest::UserAgentHeader, userAgentString);

    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, 0);
    req.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json"));
    req.setRawHeader(QByteArrayLiteral("Accept-Encoding"), QByteArrayLiteral("identity"));

    NetworkAccessManager *nam = NetworkAccessManager::instance();
    if (m_type == GET)
        m_reply = nam->get(req);
    else if (m_type == POST) {
        QByteArray bytes;
        if (!m_data.isEmpty()) {
            bytes = QJsonDocument(m_data).toJson(QJsonDocument::Compact);
            req.setHeader(QNetworkRequest::ContentLengthHeader, bytes.length());
        }
        m_reply = nam->post(req, bytes);
    }

    if (m_reply) {
#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
        qDebug() << "PA: Call Issued - " << m_type << m_api;
#endif

        emit justIssuedCall();
        emit busyChanged();

        connect(m_reply, &QNetworkReply::finished, this, &JsonHttpRequest::onNetworkReplyFinished);
        connect(m_reply, &QNetworkReply::errorOccurred, this,
                &JsonHttpRequest::onNetworkReplyError);
    }

    return true;
}

void JsonHttpRequest::setError(const QJsonObject &val)
{
    if (m_error == val)
        return;

    m_error = val;
    emit errorChanged();
}

void JsonHttpRequest::setResponse(const QJsonObject &val)
{
    if (m_response == val)
        return;

    m_response = val;
    emit responseChanged();
}

void JsonHttpRequest::onNetworkReplyError()
{
    if (m_reply->error() == QNetworkReply::NoError)
        return;

    disconnect(m_reply, &QNetworkReply::finished, this, &JsonHttpRequest::onNetworkReplyFinished);

    const QString code =
            Application::instance()->enumerationKey(m_reply, "NetworkError", m_reply->error());
    const QString msg = m_reply->errorString();

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
    qDebug() << "PA: Network Error - " << m_type << m_api << code << msg;
#endif

    emit networkError(code, msg);

    if (m_reportNetworkErrors) {
        QJsonObject errObject;
        errObject.insert(QStringLiteral("code"), code);
        errObject.insert(QStringLiteral("text"), msg);
        this->setError(errObject);
    }

    m_reply->deleteLater();
    m_reply = nullptr;
    emit busyChanged();

    emit finished();
}

void JsonHttpRequest::onNetworkReplyFinished()
{
    if (m_reply->error() == QNetworkReply::NoError) {
        const QByteArray bytes = m_reply->readAll();
        const QJsonObject json = QJsonDocument::fromJson(bytes).object();
        const QString errorAttr = QStringLiteral("error");
        const QString responseAttr = QStringLiteral("response");

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
        qDebug() << "PA: Response Received - " << m_type << m_api;
#endif

        if (json.contains(errorAttr))
            this->setError(json.value(errorAttr).toObject());
        else if (json.contains(responseAttr))
            this->setResponse(json.value(responseAttr).toObject());

        m_reply->deleteLater();
        m_reply = nullptr;
        emit busyChanged();

        emit finished();
    }
}
