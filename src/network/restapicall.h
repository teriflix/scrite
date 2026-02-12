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

#ifndef RESTAPICALL_H
#define RESTAPICALL_H

#include <QUrl>
#include <QQueue>
#include <QVariant>
#include <QJsonArray>
#include <QQmlEngine>
#include <QJsonObject>
#include <QVersionNumber>
#include <QQmlParserStatus>

#include "qobjectlistmodel.h"

class QTimer;
class RestApiCall;
class QNetworkReply;
class RestApiCallQueue;

class RestApi : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Access it via Scrite.restApi")

public:
    static const QString E_API_KEY;
    static const QString E_SESSION;
    static const QString E_NO_SESSION;
    static const QString E_NETWORK;

    static RestApi *instance();
    ~RestApi();

    // clang-format off
    Q_PROPERTY(QObject *sessionApiQueue
               READ sessionApiQueueObject
               CONSTANT )
    // clang-format on
    QObject *sessionApiQueueObject() const;
    RestApiCallQueue *sessionApiQueue() const { return m_sessionApiQueue; }

    Q_INVOKABLE bool isSessionTokenAvailable();

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
    QDateTime m_lastSessionTokenRequestTimestamp;
    QTimer *m_sessionTokenTimer = nullptr;
    RestApiCallQueue *m_sessionApiQueue = nullptr;
};

class RestApiCall : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit RestApiCall(QObject *parent = nullptr);
    ~RestApiCall();
    Q_SIGNAL void aboutToDelete(RestApiCall *call);

    enum Type { GET, POST };
    Q_ENUM(Type)
    // clang-format off
    Q_PROPERTY(Type type
               READ type
               WRITE setType
               NOTIFY typeChanged)
    // clang-format on
    void setType(Type val);
    virtual Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    // clang-format off
    Q_PROPERTY(bool useSessionToken
               READ useSessionToken
               WRITE setUseSessionToken
               NOTIFY useSessionTokenChanged)
    // clang-format on
    void setUseSessionToken(bool val);
    virtual bool useSessionToken() const { return m_useSessionToken; }
    Q_SIGNAL void useSessionTokenChanged();

    // clang-format off
    Q_PROPERTY(QString api
               READ api
               WRITE setApi
               NOTIFY apiChanged)
    // clang-format on
    void setApi(const QString &val);
    virtual QString api() const { return m_api; }
    Q_SIGNAL void apiChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject data
               READ data
               WRITE setData
               NOTIFY dataChanged)
    // clang-format on
    void setData(const QJsonObject &val);
    virtual QJsonObject data() const { return m_data; }
    Q_SIGNAL void dataChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject response
               READ response
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject response() const { return m_response; }
    Q_SIGNAL void responseChanged();

    // clang-format off
    Q_PROPERTY(QString responseCode
               READ responseCode
               NOTIFY responseChanged)
    // clang-format on
    QString responseCode() const;

    // clang-format off
    Q_PROPERTY(QString responseText
               READ responseText
               NOTIFY responseChanged)
    // clang-format on
    QString responseText() const;

    // clang-format off
    Q_PROPERTY(QJsonObject responseData
               READ responseData
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject responseData() const;

    // clang-format off
    Q_PROPERTY(bool hasResponse
               READ hasResponse
               NOTIFY responseChanged)
    // clang-format on
    bool hasResponse() const { return !m_response.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QJsonObject error
               READ error
               NOTIFY errorChanged)
    // clang-format on
    QJsonObject error() const { return m_error; }
    Q_SIGNAL void errorChanged();

    // clang-format off
    Q_PROPERTY(QString errorCode
               READ errorCode
               NOTIFY errorChanged)
    // clang-format on
    QString errorCode() const;

    // clang-format off
    Q_PROPERTY(QString errorText
               READ errorText
               NOTIFY errorChanged)
    // clang-format on
    QString errorText() const;

    // clang-format off
    Q_PROPERTY(QString errorMessage
               READ errorMessage
               NOTIFY errorChanged)
    // clang-format on
    QString errorMessage() const;

    // clang-format off
    Q_PROPERTY(QJsonObject errorData
               READ errorData
               NOTIFY errorChanged)
    // clang-format on
    QJsonObject errorData() const;

    // clang-format off
    Q_PROPERTY(bool hasError
               READ hasError
               NOTIFY errorChanged)
    // clang-format on
    bool hasError() const { return !m_error.isEmpty(); }

    // clang-format off
    Q_PROPERTY(bool busy
               READ isBusy
               NOTIFY busyChanged)
    // clang-format on
    bool isBusy() const { return m_reply != nullptr; }
    Q_SIGNAL void busyChanged();

    // clang-format off
    Q_PROPERTY(bool reportNetworkErrors
               READ isReportNetworkErrors
               WRITE setReportNetworkErrors
               NOTIFY reportNetworkErrorsChanged)
    // clang-format on
    void setReportNetworkErrors(bool val);
    bool isReportNetworkErrors() const { return m_reportNetworkErrors; }
    Q_SIGNAL void reportNetworkErrorsChanged();

    Q_INVOKABLE bool queue(RestApiCallQueue *queue);

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
    QByteArray m_sessionTokenUsed;
};

class RestApiCallQueue : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    RestApiCallQueue(QObject *parent = nullptr);
    ~RestApiCallQueue();

    static RestApiCallQueue *find(RestApiCall *call);

    // clang-format off
    Q_PROPERTY(RestApiCall *current
               READ current
               NOTIFY currentChanged
               FINAL )
    // clang-format on
    RestApiCall *current() const { return m_current; }
    Q_SIGNAL void currentChanged();

    // clang-format off
    Q_PROPERTY(bool busy
               READ isBusy
               NOTIFY currentChanged
               FINAL )
    // clang-format on
    bool isBusy() const { return m_current != nullptr; }

    // clang-format off
    Q_PROPERTY(int size
               READ size
               NOTIFY sizeChanged
               FINAL )
    // clang-format on
    int size() const { return m_queue.size(); }
    Q_SIGNAL void sizeChanged();

    // clang-format off
    Q_PROPERTY(bool empty
               READ isEmpty
               NOTIFY sizeChanged
               FINAL )
    // clang-format on
    int isEmpty() const { return m_queue.isEmpty(); }

    Q_INVOKABLE bool enqueue(RestApiCall *call);
    Q_INVOKABLE bool remove(RestApiCall *call);
    Q_INVOKABLE bool contains(RestApiCall *call) const { return m_queue.contains(call); }

signals:
    void called(RestApiCall *call, bool success);
    void done(RestApiCall *call, bool success);

private:
    void onCallDone();
    void onCallDestroyed(RestApiCall *call);
    void callNext();

private:
    RestApiCall *m_current = nullptr;
    QQueue<RestApiCall *> m_queue;
};

class RestApiCallList : public QObjectListModel<RestApiCall *>
{
    Q_OBJECT
    QML_ELEMENT

public:
    RestApiCallList(QObject *parent = nullptr);
    ~RestApiCallList();

    // clang-format off
    Q_PROPERTY(int busyCount
               READ busyCount
               NOTIFY busyCountChanged)
    // clang-format on
    int busyCount() const { return m_busyCount; }
    Q_SIGNAL void busyCountChanged();

    // clang-format off
    Q_PROPERTY(bool busy
               READ isBusy
               NOTIFY busyCountChanged)
    // clang-format on
    bool isBusy() const { return m_busyCount > 0; }

    // clang-format off
    Q_PROPERTY(QQmlListProperty<RestApiCall> calls
               READ calls)
    // clang-format on
    QQmlListProperty<RestApiCall> calls();
    Q_INVOKABLE void addCall(RestApiCall *ptr);
    Q_INVOKABLE void removeCall(RestApiCall *ptr);
    Q_INVOKABLE RestApiCall *callAt(int index) const;
    // clang-format off
    Q_PROPERTY(int callCount
               READ callCount
               NOTIFY callCountChanged)
    // clang-format on
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
class AppMinimumVersionRestApiCall : public RestApiCall
{
    Q_OBJECT

public:
    AppMinimumVersionRestApiCall(QObject *parent = nullptr);
    ~AppMinimumVersionRestApiCall();

    // clang-format off
    Q_PROPERTY(QVersionNumber minimumVersion
               READ minimumVersion
               NOTIFY responseChanged)
    // clang-format on
    QVersionNumber minimumVersion() const;

    // clang-format off
    Q_PROPERTY(QVersionNumber currentVersion
               READ currentVersion
               NOTIFY responseChanged)
    // clang-format on
    QVersionNumber currentVersion() const;

    // clang-format off
    Q_PROPERTY(bool isVersionSupported
               READ isVersionSupported
               NOTIFY responseChanged)
    // clang-format on
    bool isVersionSupported() const;

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/minimumVersion"; }
};

class AppUserGuideSearchIndexRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppUserGuideSearchIndexRestApiCall(QObject *parent = nullptr);
    ~AppUserGuideSearchIndexRestApiCall();

    // clang-format off
    Q_PROPERTY(QUrl userGuideBaseUrl
               READ userGuideBaseUrl
               NOTIFY responseChanged)
    // clang-format on
    QUrl userGuideBaseUrl() const;

    // clang-format off
    Q_PROPERTY(QUrl userGuideIndexUrl
               READ userGuideIndexUrl
               NOTIFY responseChanged)
    // clang-format on
    QUrl userGuideIndexUrl() const;

    // clang-format off
    Q_PROPERTY(QStringList userGuideSortOrder
               READ userGuideSortOrder
               NOTIFY responseChanged)
    // clang-format on
    QStringList userGuideSortOrder() const;

    // clang-format off
    Q_PROPERTY(bool isUpdateRequired
               READ isUpdateRequired
               NOTIFY responseChanged)
    // clang-format on
    bool isUpdateRequired() const;

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/userGuideSearchIndex"; }
    QJsonObject data() const;

protected:
    void setResponse(const QJsonObject &val);
};

class AppWelcomeTextApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppWelcomeTextApiCall(QObject *parent = nullptr);
    ~AppWelcomeTextApiCall();

    // clang-format off
    Q_PROPERTY(QJsonObject welcomeText
               READ welcomeText
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject welcomeText() const { return this->responseData(); }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/welcomeText"; }
};

class AppCheckUserRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppCheckUserRestApiCall(QObject *parent = nullptr);
    ~AppCheckUserRestApiCall();

    // clang-format off
    Q_PROPERTY(QString email
               READ email
               WRITE setEmail
               NOTIFY emailChanged)
    // clang-format on
    void setEmail(const QString &val);
    QString email() const { return m_email; }
    Q_SIGNAL void emailChanged();

    // clang-format off
    Q_PROPERTY(QString firstName
               READ firstName
               WRITE setFirstName
               NOTIFY firstNameChanged)
    // clang-format on
    void setFirstName(const QString &val);
    QString firstName() const { return m_firstName; }
    Q_SIGNAL void firstNameChanged();

    // clang-format off
    Q_PROPERTY(QString lastName
               READ lastName
               WRITE setLastName
               NOTIFY lastNameChanged)
    // clang-format on
    void setLastName(const QString &val);
    QString lastName() const { return m_lastName; }
    Q_SIGNAL void lastNameChanged();

    // clang-format off
    Q_PROPERTY(QString experience
               READ experience
               WRITE setExperience
               NOTIFY experienceChanged)
    // clang-format on
    void setExperience(const QString &val);
    QString experience() const { return m_experience; }
    Q_SIGNAL void experienceChanged();

    // clang-format off
    Q_PROPERTY(QString phone
               READ phone
               WRITE setPhone
               NOTIFY phoneChanged)
    // clang-format on
    void setPhone(const QString &val);
    QString phone() const { return m_phone; }
    Q_SIGNAL void phoneChanged();

    // clang-format off
    Q_PROPERTY(QString wdyhas
               READ wdyhas
               WRITE setWdyhas
               NOTIFY wdyhasChanged)
    // clang-format on
    void setWdyhas(const QString &val);
    QString wdyhas() const { return m_wdyhas; }
    Q_SIGNAL void wdyhasChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject userInfo
               READ userInfo
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject userInfo() const { return this->responseData(); }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/checkUser"; }
    QJsonObject data() const;

private:
    QString m_email;
    QString m_phone;
    QString m_wdyhas;
    QString m_lastName;
    QString m_firstName;
    QString m_experience;
};

class AppLatestReleaseRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    AppLatestReleaseRestApiCall(QObject *parent = nullptr);
    ~AppLatestReleaseRestApiCall();

    // clang-format off
    Q_PROPERTY(QJsonObject latestRelease
               READ latestRelease
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject latestRelease() const { return this->responseData(); }

    // clang-format off
    Q_PROPERTY(QJsonObject update
               READ update
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject update() const { return m_update; }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return false; }
    QString api() const { return "app/latestRelease"; }
    QJsonObject data() const;

protected:
    void setResponse(const QJsonObject &val);

private:
    QJsonObject m_update;
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

    // clang-format off
    Q_PROPERTY(QString activationCode
               READ activationCode
               WRITE setActivationCode
               NOTIFY activationCodeChanged)
    // clang-format on
    void setActivationCode(const QString &val);
    QString activationCode() const { return m_activationCode; }
    Q_SIGNAL void activationCodeChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject tokens
               READ tokens
               NOTIFY responseChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QJsonObject taxonomy
               READ taxonomy
               NOTIFY responseChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QJsonObject userInfo
               READ userInfo
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject userInfo() const { return this->responseData(); }

    // clang-format off
    Q_PROPERTY(QJsonObject updatedFields
               READ updatedFields
               WRITE setUpdatedFields
               NOTIFY updatedFieldsChanged)
    // clang-format on
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

class UserOnboardingFormApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserOnboardingFormApiCall(QObject *parent = nullptr);
    ~UserOnboardingFormApiCall();

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/onboardingForm"; }
};

class UserSubmitOnboardingFormApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserSubmitOnboardingFormApiCall(QObject *parent = nullptr);
    ~UserSubmitOnboardingFormApiCall();

    // clang-format off
    Q_PROPERTY(QJsonObject formData
               READ formData
               WRITE setFormData
               NOTIFY formDataChanged)
    // clang-format on
    void setFormData(const QJsonObject &val);
    QJsonObject formData() const { return m_formData; }
    Q_SIGNAL void formDataChanged();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/submitOnboardingForm"; }
    QJsonObject data() const { return { { "formData", m_formData } }; }

protected:
    void setResponse(const QJsonObject &val);

private:
    QJsonObject m_formData;
};

class UserRequestVersionTypeApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserRequestVersionTypeApiCall(QObject *parent = nullptr);
    ~UserRequestVersionTypeApiCall();

    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/requestVersionType"; }
    QJsonObject data() const;

    bool call();

protected:
    void setResponse(const QJsonObject &val);
};

class UserMessagesRestApiCall : public RestApiCall
{
    Q_OBJECT

private:
    UserMessagesRestApiCall(QObject *parent = nullptr);

public:
    ~UserMessagesRestApiCall();

    // clang-format off
    Q_PROPERTY(QJsonArray messages
               READ messages
               NOTIFY responseChanged)
    // clang-format on
    QJsonArray messages() const { return this->response().value("data").toArray(); }

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "user/messages"; }

private:
    friend class User;
};

class UserHelpTipsRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    UserHelpTipsRestApiCall(QObject *parent = nullptr);
    ~UserHelpTipsRestApiCall();

    // clang-format off
    Q_PROPERTY(QJsonObject helpTips
               READ helpTips
               NOTIFY responseChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QStringList emails
               READ emails
               WRITE setEmails
               NOTIFY emailsChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QString activity
               READ activity
               WRITE setActivity
               NOTIFY activityChanged)
    // clang-format on
    void setActivity(const QString &val);
    QString activity() const { return m_activity; }
    Q_SIGNAL void activityChanged();

    // clang-format off
    Q_PROPERTY(QJsonValue activityData
               READ activityData
               WRITE setActivityData
               NOTIFY activityDataChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QJsonObject installationInfo
               READ installationInfo
               NOTIFY responseChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(int activeInstallationCount
               READ activeInstallationCount
               NOTIFY responseChanged)
    // clang-format on
    int activeInstallationCount() const;

    // clang-format off
    Q_PROPERTY(int allowedInstallationCount
               READ allowedInstallationCount
               NOTIFY responseChanged)
    // clang-format on
    int allowedInstallationCount() const;

    // clang-format off
    Q_PROPERTY(QJsonArray installationsInfo
               READ installationsInfo
               NOTIFY responseChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QString installationId
               READ installationId
               WRITE setInstallationId
               NOTIFY installationIdChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QJsonObject user
               READ user
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject user() const;

    // clang-format off
    Q_PROPERTY(QJsonObject installation
               READ installation
               NOTIFY responseChanged)
    // clang-format on
    QJsonObject installation() const;

    // clang-format off
    Q_PROPERTY(QDateTime since
               READ since
               NOTIFY responseChanged)
    // clang-format on
    QDateTime since() const;

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "session/current"; }

    bool call();

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

    // clang-format off
    Q_PROPERTY(Status status
               READ status
               NOTIFY statusChanged)
    // clang-format on
    Status status() const;
    Q_SIGNAL void statusChanged();

    // RestApiCall interface
    Type type() const { return GET; }
    bool useSessionToken() const { return true; }
    QString api() const { return "session/status"; }

    bool call();

protected:
    void setResponse(const QJsonObject &val);
};

class SessionNewRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    static bool isCallUnderway();

    SessionNewRestApiCall(QObject *parent = nullptr);
    ~SessionNewRestApiCall();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return false; }
    QString api() const { return "session/new"; }
    QJsonObject data() const;

    bool call();

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

    // clang-format off
    Q_PROPERTY(QJsonArray plans
               READ plans
               NOTIFY responseChanged)
    // clang-format on
    QJsonArray plans() const;

    // clang-format off
    Q_PROPERTY(QJsonArray subscriptionHistory
               READ subscriptionHistory
               NOTIFY responseChanged)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(QString code
               READ code
               WRITE setCode
               NOTIFY codeChanged)
    // clang-format on
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

class SubscriptionTrialDeclineReasonApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SubscriptionTrialDeclineReasonApiCall(QObject *parent = nullptr);
    ~SubscriptionTrialDeclineReasonApiCall();

    // clang-format off
    Q_PROPERTY(QString reason
               READ reason
               WRITE setReason
               NOTIFY reasonChanged)
    // clang-format on
    void setReason(const QString &val);
    QString reason() const { return m_reason; }
    Q_SIGNAL void reasonChanged();

    // RestApiCall interface
    Type type() const { return POST; }
    bool useSessionToken() const { return true; }
    QString api() const { return "subscription/trialDeclineReason"; }
    QJsonObject data() const;

private:
    QString m_reason;
};

class SubscriptionPlanActivationRestApiCall : public RestApiCall
{
    Q_OBJECT
    QML_ELEMENT

public:
    SubscriptionPlanActivationRestApiCall(QObject *parent = nullptr);
    ~SubscriptionPlanActivationRestApiCall();

    // clang-format off
    Q_PROPERTY(QString activationApi
               READ api
               WRITE setApi
               NOTIFY apiChanged)
    // clang-format on

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

    // clang-format off
    Q_PROPERTY(QUrl baseUrl
               READ baseUrl
               NOTIFY baseUrlChanged)
    // clang-format on
    QUrl baseUrl() const { return m_baseUrl; }
    Q_SIGNAL void baseUrlChanged();

    // clang-format off
    Q_PROPERTY(QJsonArray records
               READ records
               NOTIFY recordsChanged)
    // clang-format on
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
