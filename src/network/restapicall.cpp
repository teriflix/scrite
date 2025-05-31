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

#include "restapicall.h"

#include "user.h"
#include "scrite.h"
#include "application.h"
#include "localstorage.h"
#include "networkaccessmanager.h"
#include "restapikey/restapikey.h"

#include <QUuid>
#include <QTimer>
#include <QSysInfo>
#include <QUrlQuery>
#include <QSettings>
#include <QJsonValue>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QOperatingSystemVersion>

RestApi *RestApi::instance()
{
    static RestApi *theInstance = new RestApi(qApp);
    return theInstance;
}

RestApi::~RestApi() { }

QObject *RestApi::sessionApiQueueObject() const
{
    return m_sessionApiQueue;
}

bool RestApi::isSessionTokenAvailable()
{
    const QByteArray sessionToken = LocalStorage::load("sessionToken").toByteArray();
    const QByteArray userId = LocalStorage::load("userId").toByteArray();
    if (sessionToken.isEmpty() || userId.isEmpty())
        return false;

    return true;
}

void RestApi::requestNewSessionToken()
{
    if (m_sessionTokenTimer == nullptr) {
        m_sessionTokenTimer = new QTimer(this);
        m_sessionTokenTimer->setInterval(500);
        m_sessionTokenTimer->setSingleShot(true);
        connect(m_sessionTokenTimer, &QTimer::timeout, this, &RestApi::requestNewSessionTokenNow);
        this->requestNewSessionTokenNow();
    } else if (!m_sessionTokenTimer->isActive())
        m_sessionTokenTimer->start();
}

void RestApi::requestFreshActivation()
{
    if (m_sessionTokenTimer)
        m_sessionTokenTimer->stop();

    LocalStorage::store("user", QVariant());
    LocalStorage::store("userId", QVariant());
    LocalStorage::store("loginToken", QVariant());
    LocalStorage::store("sessionToken", QVariant());
    User::instance()->loadInfoFromStorage();

    emit freshActivationRequired();
}

void RestApi::reportInvalidApiKey()
{
    if (m_sessionTokenTimer)
        m_sessionTokenTimer->stop();

    emit invalidApiKey();
}

void RestApi::requestNewSessionTokenNow()
{
    const QDateTime now = QDateTime::currentDateTime();
    if (m_lastSessionTokenRequestTimestamp.isValid()) {
        if (m_lastSessionTokenRequestTimestamp.msecsTo(now) < 1000)
            return;
    } else
        m_lastSessionTokenRequestTimestamp = now;

    LocalStorage::store("sessionToken", QVariant());
    emit newSessionTokenRequired();
}

RestApi::RestApi(QObject *parent) : QObject(parent)
{
    m_sessionApiQueue = new RestApiCallQueue(this);
}

///////////////////////////////////////////////////////////////////////////////

RestApiCall::RestApiCall(QObject *parent) : QObject(parent)
{
    connect(this, &RestApiCall::finished, this, &RestApiCall::maybeAutoDelete);
}

RestApiCall::~RestApiCall()
{
    emit aboutToDelete(this);
}

bool RestApiCall::autoDelete() const
{
    return m_autoDelete;
}

void RestApiCall::setAutoDelete(bool val)
{
    m_autoDelete = val;
}

void RestApiCall::setType(Type val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void RestApiCall::setUseSessionToken(bool val)
{
    if (m_useSessionToken == val)
        return;

    m_useSessionToken = val;
    emit useSessionTokenChanged();
}

void RestApiCall::setApi(const QString &val)
{
    if (m_api == val)
        return;

    m_api = val;
    emit apiChanged();
}

void RestApiCall::setData(const QJsonObject &val)
{
    if (m_data == val)
        return;

    m_data = val;
    emit dataChanged();
}

QString RestApiCall::responseCode() const
{
    return m_response.value("code").toString();
}

QString RestApiCall::responseText() const
{
    return m_response.value("text").toString();
}

QJsonObject RestApiCall::responseData() const
{
    return m_response.value("data").toObject();
}

QString RestApiCall::errorCode() const
{
    return m_error.value("code").toString();
}

QString RestApiCall::errorText() const
{
    return m_error.value("text").toString();
}

QString RestApiCall::errorMessage() const
{
    return QStringLiteral("%1: %2").arg(this->errorCode(), this->errorText());
}

QJsonObject RestApiCall::errorData() const
{
    return m_error.value("data").toObject();
}

void RestApiCall::setReportNetworkErrors(bool val)
{
    if (m_reportNetworkErrors == val)
        return;

    m_reportNetworkErrors = val;
    emit reportNetworkErrorsChanged();
}

bool RestApiCall::queue(RestApiCallQueue *queue)
{
    if (queue)
        return queue->enqueue(this);

    return false;
}

bool RestApiCall::call()
{
    if (this->api().isEmpty() || m_reply != nullptr || this->isBusy())
        return false;

    this->clearError();
    this->clearResponse();

    emit aboutToCall();

    const QJsonObject compiledData = LocalStorage::compile(this->data());

    QString path =
            QStringLiteral("/") + QLatin1String(REST_API_ROOT) + QStringLiteral("/") + this->api();
    path = path.replace(QRegExp(QStringLiteral("/+")), QStringLiteral("/"));

    QUrl url = QUrl(QLatin1String(REST_API_URL));
    url.setPath(path);
    if (this->type() == GET && !compiledData.isEmpty()) {
        QUrlQuery uq;
        QJsonObject::const_iterator it = compiledData.begin();
        QJsonObject::const_iterator end = compiledData.end();
        while (it != end) {
            uq.addQueryItem(it.key(), it.value().toString());
            ++it;
        }

        url.setQuery(uq);
    }

    QNetworkRequest req(url);
    req.setRawHeader(QByteArrayLiteral("key"), QByteArrayLiteral(REST_API_KEY));
    if (this->useSessionToken()) {
        const QByteArray sessionToken = LocalStorage::load("sessionToken").toByteArray();
        const QByteArray userId = LocalStorage::load("userId").toByteArray();

        if (sessionToken.isEmpty()) {
            QTimer::singleShot(0, RestApi::instance(), &RestApi::requestNewSessionToken);
            return false;
        }

        if (userId.isEmpty()) {
            QTimer::singleShot(0, RestApi::instance(), &RestApi::requestFreshActivation);
            return false;
        }

        m_sessionTokenUsed = sessionToken;

        req.setRawHeader("token", m_sessionTokenUsed);
        req.setRawHeader("did", Application::instance()->deviceId().toLatin1());
        req.setRawHeader("cid", Application::instance()->installationId().toLatin1());
        req.setRawHeader("uid", userId);
    }

    static const QString userAgentString = []() {
        Application *sapp = Application::instance();

        const QString space = QStringLiteral(" ");
        const QString ret = QStringLiteral("scrite-") + sapp->versionNumber().toString() + space
                + sapp->platformAsString() + space + sapp->platformVersion() + space
                + sapp->platformType() + space + sapp->installationId();
        return ret;
    }();

    if (!userAgentString.isEmpty())
        req.setHeader(QNetworkRequest::UserAgentHeader, userAgentString);

    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, 0);
    req.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json"));
    req.setRawHeader(QByteArrayLiteral("Accept-Encoding"), QByteArrayLiteral("identity"));
    req.setRawHeader(QByteArrayLiteral("client-type"), QByteArrayLiteral("desktop-app"));
    req.setRawHeader(QByteArrayLiteral("client-version"), QByteArrayLiteral(SCRITE_VERSION));
    req.setRawHeader(QByteArrayLiteral("client-platform"),
                     Application::instance()->platformAsString().toLatin1());

    NetworkAccessManager *nam = NetworkAccessManager::instance();
    if (this->type() == GET)
        m_reply = nam->get(req);
    else if (this->type() == POST) {
        QByteArray bytes;
        if (!compiledData.isEmpty()) {
            bytes = QJsonDocument(compiledData).toJson(QJsonDocument::Compact);
            req.setHeader(QNetworkRequest::ContentLengthHeader, bytes.length());
        }
        m_reply = nam->post(req, bytes);
    }

    if (m_reply) {
        emit justIssuedCall();
        emit busyChanged();

        connect(m_reply, &QNetworkReply::finished, this, &RestApiCall::onNetworkReplyFinished);
        connect(m_reply, &QNetworkReply::errorOccurred, this, &RestApiCall::onNetworkReplyError);
        return true;
    }

    return false;
}

void RestApiCall::clearError()
{
    if (m_error.isEmpty())
        return;

    m_error = QJsonObject();
    emit errorChanged();
}

void RestApiCall::clearResponse()
{
    if (m_response.isEmpty())
        return;

    m_response = QJsonObject();
    emit responseChanged();
}

void RestApiCall::setError(const QJsonObject &val)
{
    if (m_error == val)
        return;

    m_error = val;

    const QString errorCode = val.value("code").toString();

    const bool noApiKey = errorCode == "E_API_KEY";
    const bool noSession = this->useSessionToken() && errorCode == "E_NO_SESSION"
            && m_sessionTokenUsed == LocalStorage::load("sessionToken").toByteArray();

    if (noSession || noApiKey) {
        LocalStorage::store("sessionToken", QVariant());

        if (noApiKey) {
            LocalStorage::store("userId", QVariant());
            LocalStorage::store("userInfo", QVariant());
            LocalStorage::store("loginToken", QVariant());
        }

        QTimer::singleShot(0, User::instance(), &User::loadInfoFromStorage);

        if (noSession) {
            if (LocalStorage::load("loginToken").isValid()
                && LocalStorage::load("userId").isValid()) {
                QTimer::singleShot(0, RestApi::instance(), &RestApi::requestNewSessionToken);
            } else
                QTimer::singleShot(0, RestApi::instance(), &RestApi::requestFreshActivation);
        } else if (noApiKey)
            QTimer::singleShot(0, RestApi::instance(), &RestApi::reportInvalidApiKey);
    }

    emit errorChanged();
}

void RestApiCall::setResponse(const QJsonObject &val)
{
    if (m_response == val)
        return;

    m_response = val;
    emit responseChanged();
}

void RestApiCall::onNetworkReplyError()
{
    if (m_reply->error() == QNetworkReply::NoError)
        return;

    disconnect(m_reply, &QNetworkReply::finished, this, &RestApiCall::onNetworkReplyFinished);

    const QString code = "E_NETWORK_"
            + Application::instance()
                      ->enumerationKey(m_reply, "NetworkError", m_reply->error())
                      .toUpper();
    const QString msg = m_reply->errorString();

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

void RestApiCall::onNetworkReplyFinished()
{
    if (m_reply->error() == QNetworkReply::NoError) {
        const QByteArray bytes = m_reply->readAll();
        const QJsonObject json = QJsonDocument::fromJson(bytes).object();
        const QString errorAttr = QStringLiteral("error");
        const QString responseAttr = QStringLiteral("response");

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

void RestApiCall::maybeAutoDelete()
{
    if (!m_isQmlInstance && m_autoDelete)
        this->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////

Q_GLOBAL_STATIC(QList<RestApiCallQueue *>, RestApiCallQueues)

RestApiCallQueue::RestApiCallQueue(QObject *parent) : QObject(parent)
{
    ::RestApiCallQueues->append(this);
}

RestApiCallQueue::~RestApiCallQueue()
{
    ::RestApiCallQueues->removeOne(this);
}

RestApiCallQueue *RestApiCallQueue::find(RestApiCall *call)
{
    for (RestApiCallQueue *queue : qAsConst(*::RestApiCallQueues)) {
        if (queue->contains(call) || queue->current() == call)
            return queue;
    }

    return nullptr;
}

bool RestApiCallQueue::enqueue(RestApiCall *call)
{
    auto callQueuedEslewhere = [](RestApiCall *call, RestApiCallQueue *except) -> bool {
        RestApiCallQueue *queue = RestApiCallQueue::find(call);
        if (queue != nullptr && queue != except)
            return true;
        return false;
    };

    if (call && !callQueuedEslewhere(call, this)) {
        if (m_queue.contains(call))
            return true;

        connect(call, &RestApiCall::aboutToDelete, this, &RestApiCallQueue::onCallDestroyed);

        m_queue.enqueue(call);
        emit sizeChanged();

        if (m_current == nullptr)
            this->callNext();

        return true;
    }

    return false;
}

bool RestApiCallQueue::remove(RestApiCall *call)
{
    if (call) {
        int index = m_queue.indexOf(call);
        if (index >= 0) {
            m_queue.removeAt(index);
            emit sizeChanged();

            disconnect(call, 0, this, 0);

            return true;
        }
    }

    return false;
}

void RestApiCallQueue::onCallDone()
{
    if (this->sender() == m_current) {
        disconnect(m_current, 0, this, 0);
        emit done(m_current, m_current->hasResponse());

        if (m_queue.isEmpty()) {
            m_current = nullptr;
            emit currentChanged();
        } else
            this->callNext();
    }
}

void RestApiCallQueue::onCallDestroyed(RestApiCall *call)
{
    if (call == nullptr)
        return;

    if (m_current == call)
        this->onCallDone();
    else
        remove(call);
}

void RestApiCallQueue::callNext()
{
    if (m_queue.isEmpty())
        return;

    while (!m_queue.isEmpty()) {
        m_current = m_queue.dequeue();

        connect(m_current, &RestApiCall::finished, this, &RestApiCallQueue::onCallDone);

        if (m_current->call())
            break;

        disconnect(m_current, 0, this, 0);
        m_current = nullptr;
    }

    emit sizeChanged();
    emit currentChanged();
}

///////////////////////////////////////////////////////////////////////////////

RestApiCallList::RestApiCallList(QObject *parent) : QObjectListModel<RestApiCall *>(parent) { }

RestApiCallList::~RestApiCallList() { }

QQmlListProperty<RestApiCall> RestApiCallList::calls()
{
    return QQmlListProperty<RestApiCall>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &RestApiCallList::staticAppendCall, &RestApiCallList::staticCallCount,
            &RestApiCallList::staticCallAt, &RestApiCallList::staticClearCalls);
}

void RestApiCallList::addCall(RestApiCall *ptr)
{
    if (ptr == nullptr || this->indexOf(ptr) >= 0)
        return;

    this->append(ptr);
    emit callCountChanged();
}

void RestApiCallList::removeCall(RestApiCall *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = this->indexOf(ptr);
    if (index < 0)
        return;

    this->removeAt(index);
    emit callCountChanged();
}

RestApiCall *RestApiCallList::callAt(int index) const
{
    return index < 0 || index >= this->size() ? nullptr : this->at(index);
}

void RestApiCallList::clearCalls()
{
    while (this->size())
        this->removeCall(this->first());
}

void RestApiCallList::staticAppendCall(QQmlListProperty<RestApiCall> *list, RestApiCall *ptr)
{
    reinterpret_cast<RestApiCallList *>(list->data)->addCall(ptr);
}

void RestApiCallList::staticClearCalls(QQmlListProperty<RestApiCall> *list)
{
    reinterpret_cast<RestApiCallList *>(list->data)->clearCalls();
}

RestApiCall *RestApiCallList::staticCallAt(QQmlListProperty<RestApiCall> *list, int index)
{
    return reinterpret_cast<RestApiCallList *>(list->data)->callAt(index);
}

int RestApiCallList::staticCallCount(QQmlListProperty<RestApiCall> *list)
{
    return reinterpret_cast<RestApiCallList *>(list->data)->callCount();
}

void RestApiCallList::itemInsertEvent(RestApiCall *ptr)
{
    connect(ptr, &RestApiCall::busyChanged, this, &RestApiCallList::evaluateBusyCount);
}

void RestApiCallList::itemRemoveEvent(RestApiCall *ptr)
{
    disconnect(ptr, &RestApiCall::busyChanged, this, &RestApiCallList::evaluateBusyCount);
}

void RestApiCallList::setBusyCount(int val)
{
    if (m_busyCount == val)
        return;

    m_busyCount = val;
    emit busyCountChanged();
}

void RestApiCallList::evaluateBusyCount()
{
    int count = 0;

    const QList<RestApiCall *> list = this->list();
    for (const RestApiCall *call : list) {
        if (call->isBusy())
            ++count;
    }

    this->setBusyCount(count);
}

///////////////////////////////////////////////////////////////////////////////

AppMinimumVersionRestApiCall::AppMinimumVersionRestApiCall(QObject *parent)
    : RestApiCall(parent) { }

AppMinimumVersionRestApiCall::~AppMinimumVersionRestApiCall() { }

QVersionNumber AppMinimumVersionRestApiCall::minimumVersion() const
{
    const QJsonObject res = this->responseData();
    return QVersionNumber::fromString(res.value("minimumVersion").toString(SCRITE_VERSION));
}

QVersionNumber AppMinimumVersionRestApiCall::currentVersion() const
{
    const QJsonObject res = this->responseData();
    return QVersionNumber::fromString(res.value("currentVersion").toString(SCRITE_VERSION));
}

bool AppMinimumVersionRestApiCall::isVersionSupported() const
{
    const QJsonObject res = this->responseData();
    return res.value("versionSupported").toBool(true);
}

///////////////////////////////////////////////////////////////////////////////

AppCheckUserRestApiCall::AppCheckUserRestApiCall(QObject *parent) : RestApiCall(parent)
{
    m_email = LocalStorage::load("email").toString();
}

AppCheckUserRestApiCall::~AppCheckUserRestApiCall() { }

void AppCheckUserRestApiCall::setEmail(const QString &val)
{
    if (m_email == val)
        return;

    LocalStorage::store("email", val);

    m_email = val;
    emit emailChanged();
}

void AppCheckUserRestApiCall::setFirstName(const QString &val)
{
    if (m_firstName == val)
        return;

    m_firstName = val;
    emit firstNameChanged();
}

void AppCheckUserRestApiCall::setLastName(const QString &val)
{
    if (m_lastName == val)
        return;

    m_lastName = val;
    emit lastNameChanged();
}

void AppCheckUserRestApiCall::setExperience(const QString &val)
{
    if (m_experience == val)
        return;

    m_experience = val;
    emit experienceChanged();
}

void AppCheckUserRestApiCall::setPhone(const QString &val)
{
    if (m_phone == val)
        return;

    m_phone = val;
    emit phoneChanged();
}

void AppCheckUserRestApiCall::setWdyhas(const QString &val)
{
    if (m_wdyhas == val)
        return;

    m_wdyhas = val;
    emit wdyhasChanged();
}

QJsonObject AppCheckUserRestApiCall::data() const
{
    const Locale locale = Scrite::locale();
    QJsonObject ret = { { "email", "$email" },
                        { "country", locale.country.code },
                        { "currency", locale.currency.code } };

    if (!m_firstName.isEmpty())
        ret.insert("firstName", m_firstName);

    if (!m_lastName.isEmpty())
        ret.insert("lastName", m_lastName);

    if (!m_experience.isEmpty())
        ret.insert("experience", m_experience);

    if (!m_wdyhas.isEmpty())
        ret.insert("wdyhas", m_wdyhas);

    if (!m_phone.isEmpty())
        ret.insert("phone", m_phone);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AppLatestReleaseRestApiCall::AppLatestReleaseRestApiCall(QObject *parent) : RestApiCall(parent) { }

AppLatestReleaseRestApiCall::~AppLatestReleaseRestApiCall() { }

QJsonObject AppLatestReleaseRestApiCall::data() const
{
    return { { "platform", Application::instance()->platformAsString() } };
}

void AppLatestReleaseRestApiCall::setResponse(const QJsonObject &val)
{
    const QJsonObject data = val.value("data").toObject();

    const QVersionNumber updateVersion =
            QVersionNumber::fromString(data.value("version").toString());
    if (updateVersion.isNull() || updateVersion <= Application::instance()->versionNumber())
        m_update = QJsonObject();
    else
        m_update = data;

    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

AppRequestActivationCodeRestApiCall::AppRequestActivationCodeRestApiCall(QObject *parent)
    : RestApiCall(parent)
{
}

AppRequestActivationCodeRestApiCall::~AppRequestActivationCodeRestApiCall() { }

QJsonObject AppRequestActivationCodeRestApiCall::data() const
{
    return { { "email", "$email" },
             { "hostName", Application::instance()->hostName() },
             { "clientId", Application::instance()->installationId() },
             { "deviceId", Application::instance()->deviceId() },
             { "platform", Application::instance()->platformAsString() },
             { "platformVersion", Application::instance()->platformVersion() },
             { "platformType", Application::instance()->platformType() },
             { "appVersion", QStringLiteral(SCRITE_VERSION) } };
}

///////////////////////////////////////////////////////////////////////////////

AppActivateDeviceRestApiCall::AppActivateDeviceRestApiCall(QObject *parent)
    : RestApiCall(parent) { }

AppActivateDeviceRestApiCall::~AppActivateDeviceRestApiCall() { }

void AppActivateDeviceRestApiCall::setActivationCode(const QString &val)
{
    if (m_activationCode == val)
        return;

    m_activationCode = val;
    emit activationCodeChanged();
}

QJsonObject AppActivateDeviceRestApiCall::data() const
{
    return { { "email", "$email" },
             { "clientId", Application::instance()->installationId() },
             { "deviceId", Application::instance()->deviceId() },
             { "activationCode", m_activationCode } };
}

void AppActivateDeviceRestApiCall::setResponse(const QJsonObject &val)
{
    const QJsonObject data = val.value("data").toObject();
    const QStringList keys = data.keys();
    for (const QString &key : keys)
        LocalStorage::store(key, data.value(key).toString().toUtf8());

    QTimer::singleShot(0, User::instance(), &User::loadInfoUsingRestApiCall);

    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

AppPlanTaxonomyRestApiCall::AppPlanTaxonomyRestApiCall(QObject *parent) : RestApiCall(parent) { }

AppPlanTaxonomyRestApiCall::~AppPlanTaxonomyRestApiCall() { }

///////////////////////////////////////////////////////////////////////////////

UserMeRestApiCall::UserMeRestApiCall(QObject *parent) : RestApiCall(parent) { }

UserMeRestApiCall::~UserMeRestApiCall() { }

void UserMeRestApiCall::setUpdatedFields(const QJsonObject &val)
{
    if (m_updatedFields == val)
        return;

    m_updatedFields = val;
    emit updatedFieldsChanged();
}

void UserMeRestApiCall::setResponse(const QJsonObject &val)
{
    const QJsonObject data = val.value("data").toObject();
    LocalStorage::store("user", QJsonDocument(data).toJson());
    LocalStorage::store("userId", data.value("_id").toString());

    QTimer::singleShot(100, User::instance(), &User::checkForMessages);
    QTimer::singleShot(0, User::instance(), &User::loadInfoFromStorage);
    QTimer::singleShot(0, RestApi::instance(), &RestApi::sessionTokenAvailable);

    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

UserMessagesRestApiCall::UserMessagesRestApiCall(QObject *parent) : RestApiCall(parent) { }

UserMessagesRestApiCall::~UserMessagesRestApiCall() { }

///////////////////////////////////////////////////////////////////////////////

UserHelpTipsRestApiCall::UserHelpTipsRestApiCall(QObject *parent) : RestApiCall(parent) { }

UserHelpTipsRestApiCall::~UserHelpTipsRestApiCall() { }

///////////////////////////////////////////////////////////////////////////////

UserCheckRestApiCall::UserCheckRestApiCall(QObject *parent) : RestApiCall(parent) { }

UserCheckRestApiCall::~UserCheckRestApiCall() { }

void UserCheckRestApiCall::setEmails(const QStringList &val)
{
    if (m_emails == val)
        return;

    m_emails = val;
    emit emailsChanged();
}

QJsonObject UserCheckRestApiCall::data() const
{
    if (m_emails.length() == 1)
        return { { "email", m_emails.first() } };

    return { { "emails", QJsonArray::fromStringList(m_emails) } };
}

///////////////////////////////////////////////////////////////////////////////

UserActivityRestApiCall::UserActivityRestApiCall(QObject *parent) : RestApiCall(parent) { }

UserActivityRestApiCall::~UserActivityRestApiCall() { }

void UserActivityRestApiCall::setActivity(const QString &val)
{
    if (m_activity == val)
        return;

    m_activity = val;
    emit activityChanged();
}

void UserActivityRestApiCall::setActivityData(const QJsonValue &val)
{
    if (m_activityData == val)
        return;

    m_activityData = val;
    emit activityDataChanged();
}

QJsonObject UserActivityRestApiCall::data() const
{
    return { { "activity", m_activity }, { "data", m_activityData } };
}

///////////////////////////////////////////////////////////////////////////////

InstallationCurrentRestApiCall::InstallationCurrentRestApiCall(QObject *parent)
    : RestApiCall(parent)
{
}

InstallationCurrentRestApiCall::~InstallationCurrentRestApiCall() { }

///////////////////////////////////////////////////////////////////////////////

InstallationAllRestApiCall::InstallationAllRestApiCall(QObject *parent) : RestApiCall(parent) { }

InstallationAllRestApiCall::~InstallationAllRestApiCall() { }

int InstallationAllRestApiCall::activeInstallationCount() const
{
    return this->responseData().value("activeInstallationCount").toInt();
}

int InstallationAllRestApiCall::allowedInstallationCount() const
{
    return this->responseData().value("allowedInstallationCount").toInt();
}

QJsonArray InstallationAllRestApiCall::installationsInfo() const
{
    return this->responseData().value("installations").toArray();
}

///////////////////////////////////////////////////////////////////////////////

InstallationDeactivateRestApiCall::InstallationDeactivateRestApiCall(QObject *parent)
    : RestApiCall(parent)
{
    connect(this, &RestApiCall::finished, this,
            &InstallationDeactivateRestApiCall::resetEverything);
}

InstallationDeactivateRestApiCall::~InstallationDeactivateRestApiCall() { }

bool InstallationDeactivateRestApiCall::call()
{
    const bool ret = RestApiCall::call();
    if (!ret)
        this->resetEverything();

    return ret;
}

void InstallationDeactivateRestApiCall::resetEverything()
{
    LocalStorage::store("user", QVariant());
    LocalStorage::store("userId", QVariant());
    LocalStorage::store("loginToken", QVariant());
    LocalStorage::store("sessionToken", QVariant());
    User::instance()->loadInfoFromStorage();
    QTimer::singleShot(0, RestApi::instance(), &RestApi::freshActivationRequired);
}

void InstallationDeactivateRestApiCall::setResponse(const QJsonObject &val)
{
    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

InstallationUpdateRestApiCall::InstallationUpdateRestApiCall(QObject *parent) : RestApiCall(parent)
{
}

InstallationUpdateRestApiCall::~InstallationUpdateRestApiCall() { }

QJsonObject InstallationUpdateRestApiCall::data() const
{
    return { { "platform", Application::instance()->platformAsString() },
             { "platformVersion", Application::instance()->platformVersion() },
             { "platformType", Application::instance()->platformType() },
             { "appVersion", QStringLiteral(SCRITE_VERSION) },
             { "hostName", Application::instance()->hostName() } };
}

void InstallationUpdateRestApiCall::setResponse(const QJsonObject &val)
{
    QTimer::singleShot(0, User::instance(), &User::loadInfoUsingRestApiCall);

    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

InstallationDeactivateOtherRestApiCall::InstallationDeactivateOtherRestApiCall(QObject *parent)
    : RestApiCall(parent)
{
}

InstallationDeactivateOtherRestApiCall::~InstallationDeactivateOtherRestApiCall() { }

void InstallationDeactivateOtherRestApiCall::setInstallationId(const QString &val)
{
    if (m_installationId == val)
        return;

    m_installationId = val;
    emit installationIdChanged();
}

QJsonObject InstallationDeactivateOtherRestApiCall::data() const
{
    return { { "iid", m_installationId } };
}

void InstallationDeactivateOtherRestApiCall::setResponse(const QJsonObject &val)
{
    QTimer::singleShot(0, User::instance(), &User::loadInfoUsingRestApiCall);

    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

SessionCurrentRestApiCall::SessionCurrentRestApiCall(QObject *parent) : RestApiCall(parent) { }

SessionCurrentRestApiCall::~SessionCurrentRestApiCall() { }

QJsonObject SessionCurrentRestApiCall::user() const
{
    return this->responseData().value("user").toObject();
}

QJsonObject SessionCurrentRestApiCall::installation() const
{
    return this->responseData().value("installation").toObject();
}

QDateTime SessionCurrentRestApiCall::since() const
{
    return QDateTime::fromString(this->responseData().value("since").toString(), Qt::ISODateWithMs);
}

bool SessionCurrentRestApiCall::call()
{
    if (!RestApiCallQueue::find(this)) {
        qWarning("All API calls to /session/ should be queued.");
    }

    return RestApiCall::call();
}

void SessionCurrentRestApiCall::setResponse(const QJsonObject &val)
{
    const QJsonObject data = val.value("data").toObject();

    LocalStorage::store("user", QJsonDocument(data).toJson());
    LocalStorage::store("userId", data.value("_id").toString());

    QTimer::singleShot(0, User::instance(), &User::loadInfoFromStorage);
    QTimer::singleShot(100, User::instance(), &User::checkForMessages);

    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

SessionStatusRestApiCall::SessionStatusRestApiCall(QObject *parent) : RestApiCall(parent)
{
    connect(this, &RestApiCall::responseChanged, this, &SessionStatusRestApiCall::statusChanged);
    connect(this, &RestApiCall::busyChanged, this, &SessionStatusRestApiCall::statusChanged);
}

SessionStatusRestApiCall::~SessionStatusRestApiCall() { }

bool SessionStatusRestApiCall::call()
{
    if (!RestApiCallQueue::find(this)) {
        qWarning("All API calls to /session/ should be queued.");
    }

    return RestApiCall::call();
}

void SessionStatusRestApiCall::setResponse(const QJsonObject &val)
{
    RestApiCall::setResponse(val);

    QTimer::singleShot(100, User::instance(), &User::checkForMessages);
}

SessionStatusRestApiCall::Status SessionStatusRestApiCall::status() const
{
    if (this->isBusy())
        return Unknown;

    return (this->responseCode() == "SUCCESS") ? Valid : Invalid;
}

///////////////////////////////////////////////////////////////////////////////

SessionNewRestApiCall::SessionNewRestApiCall(QObject *parent) : RestApiCall(parent) { }

SessionNewRestApiCall::~SessionNewRestApiCall() { }

QJsonObject SessionNewRestApiCall::data() const
{
    return { { "token", "$loginToken" },
             { "uid", "$userId" },
             { "did", Application::instance()->deviceId() },
             { "cid", Application::instance()->installationId() } };
}

bool SessionNewRestApiCall::call()
{
    if (!RestApiCallQueue::find(this)) {
        qWarning("All API calls to /session/ should be queued.");
    }

    return RestApiCall::call();
}

void SessionNewRestApiCall::setError(const QJsonObject &val)
{
    const QString code = val.value("code").toString();
    if (code == "E_SESSION") {
        QTimer::singleShot(0, RestApi::instance(), &RestApi::requestFreshActivation);
    }

    RestApiCall::setError(val);
}

void SessionNewRestApiCall::setResponse(const QJsonObject &val)
{
    const QJsonObject data = val.value("data").toObject();
    const QStringList keys = data.keys();
    for (const QString &key : keys)
        LocalStorage::store(key, data.value(key).toString().toUtf8());

    QTimer::singleShot(100, User::instance(), &User::checkForMessages);
    QTimer::singleShot(0, User::instance(), &User::loadInfoUsingRestApiCall);
    QTimer::singleShot(0, RestApi::instance(), &RestApi::sessionTokenAvailable);

    RestApiCall::setResponse(val);
}

///////////////////////////////////////////////////////////////////////////////

SubscriptionPlansRestApiCall::SubscriptionPlansRestApiCall(QObject *parent)
    : RestApiCall(parent) { }

SubscriptionPlansRestApiCall::~SubscriptionPlansRestApiCall() { }

QJsonArray SubscriptionPlansRestApiCall::plans() const
{
    return this->responseData().value("plans").toArray();
}

QJsonArray SubscriptionPlansRestApiCall::subscriptionHistory() const
{
    return this->responseData().value("subscriptions").toArray();
}

///////////////////////////////////////////////////////////////////////////////

SubscriptionReferralCodeRestApiCall::SubscriptionReferralCodeRestApiCall(QObject *parent)
    : RestApiCall(parent)
{
}

SubscriptionReferralCodeRestApiCall::~SubscriptionReferralCodeRestApiCall() { }

void SubscriptionReferralCodeRestApiCall::setCode(const QString &val)
{
    if (m_code == val)
        return;

    m_code = val;
    emit codeChanged();
}

QJsonObject SubscriptionReferralCodeRestApiCall::data() const
{
    return { { "code", m_code } };
}

///////////////////////////////////////////////////////////////////////////////

SubscriptionTrialDeclineReasonApiCall::SubscriptionTrialDeclineReasonApiCall(QObject *parent)
    : RestApiCall(parent)
{
}

SubscriptionTrialDeclineReasonApiCall::~SubscriptionTrialDeclineReasonApiCall() { }

void SubscriptionTrialDeclineReasonApiCall::setReason(const QString &val)
{
    if (m_reason == val)
        return;

    m_reason = val.left(512);
    emit reasonChanged();
}

QJsonObject SubscriptionTrialDeclineReasonApiCall::data() const
{
    return { { "reason", m_reason } };
}

///////////////////////////////////////////////////////////////////////////////

SubscriptionPlanActivationRestApiCall::SubscriptionPlanActivationRestApiCall(QObject *parent)
    : RestApiCall(parent)
{
}

SubscriptionPlanActivationRestApiCall::~SubscriptionPlanActivationRestApiCall() { }

void SubscriptionPlanActivationRestApiCall::setResponse(const QJsonObject &val)
{
    RestApiCall::setResponse(val);

    SessionNewRestApiCall *api = new SessionNewRestApiCall(User::instance());
    if (!api->queue(RestApi::instance()->sessionApiQueue()))
        api->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////

AbstractScriptalayRestApiCall::AbstractScriptalayRestApiCall(QObject *parent) : RestApiCall(parent)
{
}

AbstractScriptalayRestApiCall::~AbstractScriptalayRestApiCall() { }

QString AbstractScriptalayRestApiCall::api() const
{
    return "scriptalay/" + this->endpoint();
}

void AbstractScriptalayRestApiCall::setResponse(const QJsonObject &val)
{
    const QJsonObject data = val.value("data").toObject();

    this->setBaseUrl(QUrl(data.value("baseUrl").toString()));
    this->setRecords(data.value("records").toArray());

    RestApiCall::setResponse(val);
}

void AbstractScriptalayRestApiCall::setBaseUrl(const QUrl &val)
{
    if (m_baseUrl == val)
        return;

    m_baseUrl = val;
    emit baseUrlChanged();
}

void AbstractScriptalayRestApiCall::setRecords(const QJsonArray &val)
{
    if (m_records == val)
        return;

    m_records = val;
    emit recordsChanged();
}

ScriptalayFormsRestApiCall::ScriptalayFormsRestApiCall(QObject *parent)
    : AbstractScriptalayRestApiCall(parent)
{
}

ScriptalayFormsRestApiCall::~ScriptalayFormsRestApiCall() { }

ScriptalayTemplatesRestApiCall::ScriptalayTemplatesRestApiCall(QObject *parent)
    : AbstractScriptalayRestApiCall(parent)
{
}

ScriptalayTemplatesRestApiCall::~ScriptalayTemplatesRestApiCall() { }

ScriptalayScreenplaysRestApiCall::ScriptalayScreenplaysRestApiCall(QObject *parent)
    : AbstractScriptalayRestApiCall(parent)
{
}

ScriptalayScreenplaysRestApiCall::~ScriptalayScreenplaysRestApiCall() { }

///////////////////////////////////////////////////////////////////////////////
