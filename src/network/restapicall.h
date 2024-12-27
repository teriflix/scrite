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

#ifndef RESTAPICALL_H
#define RESTAPICALL_H

#include <QUrl>
#include <QVariant>
#include <QJsonArray>
#include <QQmlEngine>
#include <QJsonObject>
#include <QQmlParserStatus>

#include "qobjectlistmodel.h"

class QTimer;
class QNetworkReply;

class RestApi : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Access it via Scrite.restApi")

public:
    static RestApi *instance();
    ~RestApi();

    void requestNewSessionToken();
    void requestFreshActivation();
    void reportInvalidApiKey();

signals:
    void newSessionTokenRequired();
    void freshActivationRequired();
    void invalidApiKey();

    void sessionTokenAvailable();

private:
    void requestNewSessionTokenNow();

protected:
    friend class RestApiCall;
    RestApi(QObject *parent = nullptr);

private:
    QTimer *m_sessionTokenTimer = nullptr;
};

class RestApiCall : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit RestApiCall(QObject *parent = nullptr);
    ~RestApiCall();

    enum Type { GET, POST };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    void setType(Type val);
    virtual Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(bool useSessionToken READ useSessionToken WRITE setUseSessionToken NOTIFY useSessionTokenChanged)
    void setUseSessionToken(bool val);
    virtual bool useSessionToken() const { return m_useSessionToken; }
    Q_SIGNAL void useSessionTokenChanged();

    Q_PROPERTY(QString api READ api WRITE setApi NOTIFY apiChanged)
    void setApi(const QString &val);
    virtual QString api() const { return m_api; }
    Q_SIGNAL void apiChanged();

    Q_PROPERTY(QJsonObject data READ data WRITE setData NOTIFY dataChanged)
    void setData(const QJsonObject &val);
    virtual QJsonObject data() const { return m_data; }
    Q_SIGNAL void dataChanged();

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

    virtual Q_INVOKABLE bool call();
    Q_INVOKABLE void reset()
    {
        this->clearError();
        this->clearResponse();
    }
    Q_INVOKABLE void clearError();
    Q_INVOKABLE void clearResponse();

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

protected:
    virtual void setError(const QJsonObject &val);
    virtual void setResponse(const QJsonObject &val);
    void onNetworkReplyError();
    void onNetworkReplyFinished();
    void maybeAutoDelete();

private:
    Type m_type = POST;
    QString m_api;
    QString m_token;
    QJsonObject m_data;
    QJsonObject m_error;
    QJsonObject m_response;
    bool m_autoDelete = true;
    bool m_isQmlInstance = false;
    bool m_useSessionToken = true;
    QNetworkReply *m_reply = nullptr;
    bool m_reportNetworkErrors =
            false; // when false, networkError() signal is emited to report
                   // network errors. When true, they are reported via error() also.
};

class RestApiCallList : public QObjectListModel<RestApiCall *>
{
    Q_OBJECT
    QML_ELEMENT

public:
    RestApiCallList(QObject *parent = nullptr);
    ~RestApiCallList();

    Q_PROPERTY(int busyCount READ busyCount NOTIFY busyCountChanged)
    int busyCount() const { return m_busyCount; }
    Q_SIGNAL void busyCountChanged();

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyCountChanged)
    bool isBusy() const { return m_busyCount > 0; }

    Q_PROPERTY(QQmlListProperty<RestApiCall> calls READ calls)
    QQmlListProperty<RestApiCall> calls();
    Q_INVOKABLE void addCall(RestApiCall *ptr);
    Q_INVOKABLE void removeCall(RestApiCall *ptr);
    Q_INVOKABLE RestApiCall *callAt(int index) const;
    Q_PROPERTY(int callCount READ callCount NOTIFY callCountChanged)
    int callCount() const { return this->size(); }
    Q_INVOKABLE void clearCalls();
    Q_SIGNAL void callCountChanged();

private:
    static void staticAppendCall(QQmlListProperty<RestApiCall> *list, RestApiCall *ptr);
    static void staticClearCalls(QQmlListProperty<RestApiCall> *list);
    static RestApiCall *staticCallAt(QQmlListProperty<RestApiCall> *list, int index);
    static int staticCallCount(QQmlListProperty<RestApiCall> *list);

protected:
    void itemInsertEvent(RestApiCall *ptr);
    void itemRemoveEvent(RestApiCall *ptr);

private:
    void setBusyCount(int val);
    void evaluateBusyCount();

private:
    int m_busyCount = 0;
};

// Known API calls.
class AppCheckUserRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppCheckUserRestApiCall(QObject *parent = nullptr);
    ~AppCheckUserRestApiCall();

    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    void setEmail(const QString &val);
    QString email() const { return m_email; }
    Q_SIGNAL void emailChanged();

    Q_PROPERTY(QJsonObject userInfo READ userInfo NOTIFY responseChanged)
    QJsonObject userInfo() const { return this->responseData(); }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/checkUser"; }
    QJsonObject data() const;

private:
    QString m_email;
};

class AppRequestActivationCodeRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppRequestActivationCodeRestApiCall(QObject *parent = nullptr);
    ~AppRequestActivationCodeRestApiCall();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/requestActivationCode"; }
    QJsonObject data() const;
};

class AppActivateDeviceRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppActivateDeviceRestApiCall(QObject *parent = nullptr);
    ~AppActivateDeviceRestApiCall();

    Q_PROPERTY(QString activationCode READ activationCode WRITE setActivationCode NOTIFY activationCodeChanged)
    void setActivationCode(const QString &val);
    QString activationCode() const { return m_activationCode; }
    Q_SIGNAL void activationCodeChanged();

    Q_PROPERTY(QJsonObject tokens READ tokens NOTIFY responseChanged)
    QJsonObject tokens() const { return this->responseData(); }

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/activateDevice"; }
    QJsonObject data() const;

protected:
    void setResponse(const QJsonObject &val);

private:
    QString m_activationCode;
};

class AppPlanTaxonomyRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppPlanTaxonomyRestApiCall(QObject *parent = nullptr);
    ~AppPlanTaxonomyRestApiCall();

    Q_PROPERTY(QJsonObject taxonomy READ taxonomy NOTIFY responseChanged)
    QJsonObject taxonomy() const { return this->responseData(); }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/planTaxonomy"; }
};

class UserMeRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserMeRestApiCall(QObject *parent = nullptr);
    ~UserMeRestApiCall();

    Q_PROPERTY(QJsonObject userInfo READ userInfo NOTIFY responseChanged)
    QJsonObject userInfo() const { return this->responseData(); }

    Q_PROPERTY(QJsonObject updatedFields READ updatedFields WRITE setUpdatedFields NOTIFY updatedFieldsChanged)
    void setUpdatedFields(const QJsonObject &val);
    QJsonObject updatedFields() const { return m_updatedFields; }
    Q_SIGNAL void updatedFieldsChanged();

    // RestApiCall interface
    Type type() const { return m_updatedFields.isEmpty() ? GET : POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/me"; }
    QJsonObject data() const { return m_updatedFields; }

protected:
    void setResponse(const QJsonObject &val);

private:
    QJsonObject m_updatedFields;
};

class UserHelpTipsRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserHelpTipsRestApiCall(QObject *parent = nullptr);
    ~UserHelpTipsRestApiCall();

    Q_PROPERTY(QJsonObject helpTips READ helpTips NOTIFY responseChanged)
    QJsonObject helpTips() const { return this->responseData(); }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/helpTips"; }
};

class UserCheckRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserCheckRestApiCall(QObject *parent = nullptr);
    ~UserCheckRestApiCall();

    Q_PROPERTY(QStringList emails READ emails WRITE setEmails NOTIFY emailsChanged)
    void setEmails(const QStringList &val);
    QStringList emails() const { return m_emails; }
    Q_SIGNAL void emailsChanged();

    // RestApiCall interface
    Type type() const { return m_emails.length() == 1 ? GET : POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/check"; }
    QJsonObject data() const;

private:
    QStringList m_emails;
};

class UserActivityRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserActivityRestApiCall(QObject *parent = nullptr);
    ~UserActivityRestApiCall();

    Q_PROPERTY(QString activity READ activity WRITE setActivity NOTIFY activityChanged)
    void setActivity(const QString &val);
    QString activity() const { return m_activity; }
    Q_SIGNAL void activityChanged();

    Q_PROPERTY(QJsonValue activityData READ activityData WRITE setActivityData NOTIFY activityDataChanged)
    void setActivityData(const QJsonValue &val);
    QJsonValue activityData() const { return m_activityData; }
    Q_SIGNAL void activityDataChanged();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/activity"; }
    QJsonObject data() const;

private:
    QString m_activity;
    QJsonValue m_activityData;
};

class InstallationCurrentRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    InstallationCurrentRestApiCall(QObject *parent = nullptr);
    ~InstallationCurrentRestApiCall();

    Q_PROPERTY(QJsonObject installationInfo READ installationInfo NOTIFY responseChanged)
    QJsonObject installationInfo() const { return this->responseData(); }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "installation/current"; }
};

class InstallationAllRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    InstallationAllRestApiCall(QObject *parent = nullptr);
    ~InstallationAllRestApiCall();

    Q_PROPERTY(int activeInstallationCount READ activeInstallationCount NOTIFY responseChanged)
    int activeInstallationCount() const;

    Q_PROPERTY(int allowedInstallationCount READ allowedInstallationCount NOTIFY responseChanged)
    int allowedInstallationCount() const;

    Q_PROPERTY(QJsonArray installationsInfo READ installationsInfo NOTIFY responseChanged)
    QJsonArray installationsInfo() const;

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "installation/all"; }
};

class InstallationDeactivateRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    InstallationDeactivateRestApiCall(QObject *parent = nullptr);
    ~InstallationDeactivateRestApiCall();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "installation/deactivate"; }

    bool call();

private:
    void resetEverything();

protected:
    void setResponse(const QJsonObject &val);
};

class InstallationUpdateRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    InstallationUpdateRestApiCall(QObject *parent = nullptr);
    ~InstallationUpdateRestApiCall();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "installation/update"; }
    QJsonObject data() const;

protected:
    void setResponse(const QJsonObject &val);
};

class InstallationDeactivateOtherRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    InstallationDeactivateOtherRestApiCall(QObject *parent = nullptr);
    ~InstallationDeactivateOtherRestApiCall();

    Q_PROPERTY(QString installationId READ installationId WRITE setInstallationId NOTIFY installationIdChanged)
    void setInstallationId(const QString &val);
    QString installationId() const { return m_installationId; }
    Q_SIGNAL void installationIdChanged();

    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "installation/deactivateOther"; }
    QJsonObject data() const;

protected:
    void setResponse(const QJsonObject &val);

private:
    QString m_installationId;
};

class SessionCurrentRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SessionCurrentRestApiCall(QObject *parent = nullptr);
    ~SessionCurrentRestApiCall();

    Q_PROPERTY(QJsonObject user READ user NOTIFY responseChanged)
    QJsonObject user() const;

    Q_PROPERTY(QJsonObject installation READ installation NOTIFY responseChanged)
    QJsonObject installation() const;

    Q_PROPERTY(QDateTime since READ since NOTIFY responseChanged)
    QDateTime since() const;

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "session/current"; }

protected:
    void setResponse(const QJsonObject &val);
};

class SessionStatusRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SessionStatusRestApiCall(QObject *parent = nullptr);
    ~SessionStatusRestApiCall();

    enum Status { Unknown = -1, Invalid, Valid };
    Q_ENUM(Status)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Status status() const;
    Q_SIGNAL void statusChanged();

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "session/status"; }
};

class SessionNewRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SessionNewRestApiCall(QObject *parent = nullptr);
    ~SessionNewRestApiCall();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return false; }
    QString api() const { return "session/new"; }
    QJsonObject data() const;

protected:
    void setError(const QJsonObject &val);
    void setResponse(const QJsonObject &val);
};

class SubscriptionPlansRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SubscriptionPlansRestApiCall(QObject *parent = nullptr);
    ~SubscriptionPlansRestApiCall();

    Q_PROPERTY(QJsonArray plans READ plans NOTIFY responseChanged)
    QJsonArray plans() const;

    Q_PROPERTY(QJsonArray subscriptionHistory READ subscriptionHistory NOTIFY responseChanged)
    QJsonArray subscriptionHistory() const;

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "subscription/plans"; }
};

class SubscriptionReferralCodeRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SubscriptionReferralCodeRestApiCall(QObject *parent = nullptr);
    ~SubscriptionReferralCodeRestApiCall();

    Q_PROPERTY(QString code READ code WRITE setCode NOTIFY codeChanged)
    void setCode(const QString &val);
    QString code() const { return m_code; }
    Q_SIGNAL void codeChanged();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "subscription/referralCode"; }
    QJsonObject data() const;

private:
    QString m_code;
};

class SubscriptionPlanActivationRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SubscriptionPlanActivationRestApiCall(QObject *parent = nullptr);
    ~SubscriptionPlanActivationRestApiCall();

    Q_PROPERTY(QString activationApi READ api WRITE setApi NOTIFY apiChanged)

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return true; }

protected:
    void setResponse(const QJsonObject &val);
};

class AbstractScriptalayRestApiCall : public RestApiCall
{
    Q_OBJECT

public:
    AbstractScriptalayRestApiCall(QObject *parent = nullptr);
    ~AbstractScriptalayRestApiCall();

    Q_PROPERTY(QUrl baseUrl READ baseUrl NOTIFY baseUrlChanged)
    QUrl baseUrl() const { return m_baseUrl; }
    Q_SIGNAL void baseUrlChanged();

    Q_PROPERTY(QJsonArray records READ records NOTIFY recordsChanged)
    QJsonArray records() const { return m_records; }
    Q_SIGNAL void recordsChanged();

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const;

protected:
    virtual QString endpoint() const = 0;
    void setResponse(const QJsonObject &val);

private:
    void setBaseUrl(const QUrl &val);
    void setRecords(const QJsonArray &val);

private:
    QUrl m_baseUrl;
    QJsonArray m_records;
};

class ScriptalayFormsRestApiCall : public AbstractScriptalayRestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    ScriptalayFormsRestApiCall(QObject *parent = nullptr);
    ~ScriptalayFormsRestApiCall();

protected:
    QString endpoint() const { return "forms"; }
};

class ScriptalayTemplatesRestApiCall : public AbstractScriptalayRestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    ScriptalayTemplatesRestApiCall(QObject *parent = nullptr);
    ~ScriptalayTemplatesRestApiCall();

protected:
    QString endpoint() const { return "templates"; }
};

class ScriptalayScreenplaysRestApiCall : public AbstractScriptalayRestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    ScriptalayScreenplaysRestApiCall(QObject *parent = nullptr);
    ~ScriptalayScreenplaysRestApiCall();

protected:
    QString endpoint() const { return "screenplays"; }
};

#endif // RESTAPICALL_H
