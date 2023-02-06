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

#ifndef JSONHTTPREQUEST_H
#define JSONHTTPREQUEST_H

#include <QUrl>
#include <QVariant>
#include <QQmlEngine>
#include <QJsonObject>
#include <QQmlParserStatus>

#include "restapikey/restapikey.h"

class QNetworkReply;

class JsonHttpRequest : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit JsonHttpRequest(QObject *parent = nullptr);
    ~JsonHttpRequest();

    enum Type { GET, POST };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QUrl host READ host WRITE setHost NOTIFY hostChanged)
    void setHost(const QUrl &val);
    QUrl host() const { return m_host; }
    Q_SIGNAL void hostChanged();

    Q_PROPERTY(QString root READ root WRITE setRoot NOTIFY rootChanged)
    void setRoot(const QString &val);
    QString root() const { return m_root; }
    Q_SIGNAL void rootChanged();

    Q_PROPERTY(QString api READ api WRITE setApi NOTIFY apiChanged)
    void setApi(const QString &val);
    QString api() const { return m_api; }
    Q_SIGNAL void apiChanged();

    Q_PROPERTY(QJsonObject data READ data WRITE setData NOTIFY dataChanged)
    void setData(const QJsonObject &val);
    QJsonObject data() const { return m_data; }
    Q_SIGNAL void dataChanged();

    Q_INVOKABLE static QString defaultKey();

    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
    void setKey(const QString &val);
    QString key() const { return m_key; }
    Q_SIGNAL void keyChanged();

    Q_INVOKABLE static QString loginToken();
    Q_INVOKABLE static QString sessionToken();
    Q_INVOKABLE static void store(const QString &key, const QVariant &value);
    Q_INVOKABLE static QVariant fetch(const QString &key);

    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
    void setToken(const QString &val);
    QString token() const { return m_token; }
    Q_SIGNAL void tokenChanged();

    Q_INVOKABLE static QString email();
    Q_INVOKABLE static QString clientId();
    Q_INVOKABLE static QString deviceId();
    Q_INVOKABLE static QString platform();
    Q_INVOKABLE static QString platformVersion();
    Q_INVOKABLE static QString platformType();
    Q_INVOKABLE static QString appVersion();
    static QString encrypt(const QString &text);
    static QString decrypt(const QString &text);

    Q_PROPERTY(QJsonObject response READ response NOTIFY responseChanged)
    QJsonObject response() const { return m_response; }
    Q_SIGNAL void responseChanged();

    Q_PROPERTY(QString responseCode READ responseCode NOTIFY responseChanged)
    QString responseCode() const;

    Q_PROPERTY(QString responseText READ responseText NOTIFY responseChanged)
    QString responseText() const;

    Q_PROPERTY(QJsonObject responseData READ responseData NOTIFY responseChanged)
    QJsonObject responseData() const;

    Q_PROPERTY(bool hasResponse READ hasResponse NOTIFY responseChanged)
    bool hasResponse() const { return !m_response.isEmpty(); }

    Q_PROPERTY(QJsonObject error READ error NOTIFY errorChanged)
    QJsonObject error() const { return m_error; }
    Q_SIGNAL void errorChanged();

    Q_PROPERTY(QString errorCode READ errorCode NOTIFY errorChanged)
    QString errorCode() const;

    Q_PROPERTY(QString errorText READ errorText NOTIFY errorChanged)
    QString errorText() const;

    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    QString errorMessage() const;

    Q_PROPERTY(QJsonObject errorData READ errorData NOTIFY errorChanged)
    QJsonObject errorData() const;

    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    bool hasError() const { return !m_error.isEmpty(); }

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    bool isBusy() const { return m_reply != nullptr; }
    Q_SIGNAL void busyChanged();

    Q_PROPERTY(bool reportNetworkErrors READ isReportNetworkErrors WRITE setReportNetworkErrors NOTIFY reportNetworkErrorsChanged)
    void setReportNetworkErrors(bool val);
    bool isReportNetworkErrors() const { return m_reportNetworkErrors; }
    Q_SIGNAL void reportNetworkErrorsChanged();

    Q_INVOKABLE bool call();

    bool autoDelete() const;
    void setAutoDelete(bool val);

    // QQmlParserStatus interface
    void classBegin() { m_isQmlInstance = true; }
    void componentComplete() { m_isQmlInstance = true; }

signals:
    void aboutToCall();
    void justIssuedCall();
    void finished();
    void networkError(const QString &code, const QString &message);

private:
    void setError(const QJsonObject &val);
    void setResponse(const QJsonObject &val);
    void clearError() { this->setError(QJsonObject()); }
    void clearResponse() { this->setResponse(QJsonObject()); }
    void onNetworkReplyError();
    void onNetworkReplyFinished();

private:
#ifdef REST_API_LOCALHOST
    QUrl m_host = QUrl(QStringLiteral("http://localhost:8934"));
    QString m_root;
#else
    QUrl m_host = QUrl(QStringLiteral("https://www.scrite.io"));
    QString m_root = QStringLiteral("api");
#endif // REST_API_LOCALHOST
    Type m_type = POST;
    QString m_api;
    QString m_key;
    QString m_token;
    QJsonObject m_data;
    QJsonObject m_error;
    QJsonObject m_response;
    bool m_autoDelete = true;
    bool m_isQmlInstance = false;
    QNetworkReply *m_reply = nullptr;
    bool m_reportNetworkErrors =
            false; // when false, networkError() signal is emited to report
                   // network errors. When true, they are reported via error() also.
};

#endif // JSONHTTPREQUEST_H
