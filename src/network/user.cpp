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

#include "user.h"
#include "scrite.h"
#include "application.h"
#include "restapicall.h"
#include "localstorage.h"
#include "peerapplookup.h"

#include <QImage>
#include <QPainter>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMetaClassInfo>

inline QStringList JsonArrayToStringList(const QJsonArray &array)
{
    QStringList ret;
    for (const QJsonValue &item : array)
        ret << item.toString();
    return ret;
}

inline bool DeepCopyGadget(const QMetaObject *mo, const void *from, void *to)
{
    bool success = true;
    for (int i = 0; i < mo->propertyCount(); i++) {
        const QMetaProperty prop = mo->property(i);
        const QVariant propValue = prop.readOnGadget(from);
        success &= prop.writeOnGadget(to, propValue);
    }
    return success;
}

inline bool DeepCompareGadget(const QMetaObject *mo, const void *a, const void *b)
{
    for (int i = 0; i < mo->propertyCount(); i++) {
        const QMetaProperty prop = mo->property(i);
        const QVariant aValue = prop.readOnGadget(a);
        const QVariant bValue = prop.readOnGadget(b);
        int result = 0;
        bool compareSuccess = QMetaType::compare(aValue.constData(), bValue.constData(),
                                                 prop.userType(), &result);
        if (compareSuccess) {
            if (result != 0)
                return false;
        } else {
            if (aValue != bValue)
                return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

UserInstallationInfo::UserInstallationInfo(const QJsonObject &object)
{
    this->id = object.value("_id").toString();
    this->clientId = object.value("clientId").toString();
    this->deviceId = object.value("deviceId").toString();
    this->platform = object.value("platform").toString();
    this->platformVersion = object.value("platformVersion").toString();
    this->platformType = object.value("platformType").toString();
    this->appVersion = object.value("appVersion").toString();
    this->pastAppVersions = JsonArrayToStringList(object.value("appVersions").toArray());
    this->creationDate =
            QDateTime::fromString(object.value("creationDate").toString(), Qt::ISODateWithMs);
    this->lastActivationDate =
            QDateTime::fromString(object.value("lastActivationDate").toString(), Qt::ISODateWithMs);
    this->lastSessionDate =
            QDateTime::fromString(object.value("lastSessionDate").toString(), Qt::ISODateWithMs);
    this->activated = object.value("activated").toBool();
}

UserInstallationInfo::UserInstallationInfo(const UserInstallationInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
}

bool UserInstallationInfo::operator==(const UserInstallationInfo &other) const
{
    return DeepCompareGadget(&staticMetaObject, static_cast<const void *>(&other),
                             static_cast<const void *>(this));
}

UserInstallationInfo &UserInstallationInfo::operator=(const UserInstallationInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
    return *this;
}

bool UserInstallationInfo::isCurrent() const
{
    return this->clientId == Application::instance()->installationId()
            && this->deviceId == Application::instance()->deviceId();
}

///////////////////////////////////////////////////////////////////////////////

UserSubscriptionPlanInfo::UserSubscriptionPlanInfo(const QJsonObject &object)
{
    this->name = object.value("name").toString();
    this->kind = object.value("kind").toString();
    this->title = object.value("title").toString();
    this->subtitle = object.value("subtitle").toString();
    this->duration = object.value("duration").toInt();
    this->exclusive = object.value("exclusive").toBool(false);

    const QJsonObject pricing = object.value("pricing").toObject();
    this->currency = pricing.value("currency").toString();
    this->price = pricing.value("price").toDouble();

    this->features = JsonArrayToStringList(object.value("features").toArray());
    this->featureNote = object.value("featureNote").toString();

    this->devices = object.value("devices").toInt();
}

UserSubscriptionPlanInfo::UserSubscriptionPlanInfo(const UserSubscriptionPlanInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
}

bool UserSubscriptionPlanInfo::operator==(const UserSubscriptionPlanInfo &other) const
{
    return DeepCompareGadget(&staticMetaObject, static_cast<const void *>(&other),
                             static_cast<const void *>(this));
}

UserSubscriptionPlanInfo &UserSubscriptionPlanInfo::operator=(const UserSubscriptionPlanInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

UserSubscriptionInfo::UserSubscriptionInfo(const QJsonObject &object)
{
    this->id = object.value("_id").toString();
    this->kind = object.value("kind").toString();
    this->plan = UserSubscriptionPlanInfo(object.value("plan").toObject());
    this->from = QDateTime::fromString(object.value("from").toString(), Qt::ISODateWithMs);
    this->until = QDateTime::fromString(object.value("until").toString(), Qt::ISODateWithMs);
    this->orderId = object.value("wc_order_id").toString();
    this->detailsUrl = QUrl(object.value("detailsUrl").toString());
    this->isActive = object.value("isActive").toBool();
    this->isUpcoming = object.value("isUpcoming").toBool();
    this->hasExpired = object.value("hasExpired").toBool();
}

UserSubscriptionInfo::UserSubscriptionInfo(const UserSubscriptionInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
}

bool UserSubscriptionInfo::operator==(const UserSubscriptionInfo &other) const
{
    return DeepCompareGadget(&staticMetaObject, static_cast<const void *>(&other),
                             static_cast<const void *>(this));
}

UserSubscriptionInfo &UserSubscriptionInfo::operator=(const UserSubscriptionInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
    return *this;
}

QString UserSubscriptionInfo::description() const
{
    auto describeDays = [](int nrDays) -> QString {
        if (nrDays == 0)
            return "today";
        else if (nrDays == 1)
            return "tomorrow";
        return "in " + QString::number(nrDays) + " days";
    };

    QString ret = this->plan.title;
    if (this->isActive) {
        const int nrDays = this->daysToUntil();
        if (nrDays <= 30)
            ret += " (Expires " + describeDays(nrDays) + ")";
    } else if (this->isUpcoming) {
        const int nrDays = this->daysToFrom();
        if (nrDays <= 30)
            ret += " (Starts " + describeDays(nrDays) + ")";
        else
            ret += " (Upcoming)";
    } else if (this->hasExpired) {
        ret += " (Expired)";
    }

    return ret;
}

bool UserSubscriptionInfo::isFeatureEnabled(int feature) const
{
    return Scrite::isFeatureEnabled(Scrite::AppFeature(feature), this->plan.features);
}

bool UserSubscriptionInfo::isFeatureNameEnabled(const QString &featureName) const
{
    return Scrite::isFeatureNameEnabled(featureName, this->plan.features);
}

///////////////////////////////////////////////////////////////////////////////

UserInfo::UserInfo(const QJsonObject &object)
{
    this->id = object.value("_id").toString();
    this->email = object.value("email").toString();
    this->signUpDate =
            QDateTime::fromString(object.value("signUpDate").toString(), Qt::ISODateWithMs);
    this->timestamp =
            QDateTime::fromString(object.value("timestamp").toString(), Qt::ISODateWithMs);
    this->firstName = object.value("firstName").toString();
    this->lastName = object.value("lastName").toString();
    this->fullName = object.value("fullName").toString();
    this->experience = object.value("experience").toString();
    this->phone = object.value("phone").toString();
    this->city = object.value("city").toString();
    this->country = object.value("country").toString();
    this->wdyhas = object.value("wdyhas").toString();
    this->consentToActivityLog = object.value("consentToActivityLog").toBool();
    this->consentToEmail = object.value("consentToEmail").toBool();

    const QJsonArray _installations = object.value("installations").toArray();
    for (const QJsonValue &_installation : _installations) {
        UserInstallationInfo info(_installation.toObject());
        this->installations << info;
    }

    const QJsonArray _subscriptions = object.value("subscriptions").toArray();
    for (const QJsonValue &_subscription : _subscriptions) {
        UserSubscriptionInfo info(_subscription.toObject());
        this->subscriptions << info;
    }

    const QJsonValue pbs = object.value("publicBetaSubscription");
    if (pbs.isObject())
        this->publicBetaSubscription = UserSubscriptionInfo(pbs.toObject());

    this->activeInstallationCount = object.value("activeInstallationCount").toInt();
    this->hasActiveSubscription = object.value("hasActiveSubscription").toBool();
    this->hasUpcomingSubscription = object.value("hasUpcomingSubscription").toBool();
    this->hasTrialSubscription = object.value("hasTrialSubscription").toBool();
    this->paidSubscriptionCount = object.value("paidSubscriptionCount").toBool();
    this->subscribedUntil =
            QDateTime::fromString(object.value("subscribedUntil").toString(), Qt::ISODateWithMs);
    this->isEarlyAdopter = object.value("isEarlyAdopter").toBool();

    this->availableFeatures = JsonArrayToStringList(object.value("availableFeatures").toArray());

    this->badgeImageUrl = QUrl(object.value("badge").toString());

    const QString btc = object.value("badgeTextColor").toString();
    this->badgeTextColor = btc.isEmpty() ? QColor(Qt::white) : QColor(btc);
}

UserInfo::UserInfo(const UserInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
}

bool UserInfo::operator==(const UserInfo &other) const
{
    return DeepCompareGadget(&staticMetaObject, static_cast<const void *>(&other),
                             static_cast<const void *>(this));
}

UserInfo &UserInfo::operator=(const UserInfo &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
    return *this;
}

int UserInfo::daysToSubscribedUntil() const
{
    return QDate::currentDate().daysTo(this->subscribedUntil.date()) + 1;
}

bool UserInfo::isFeatureEnabled(int feature) const
{
    return Scrite::isFeatureEnabled(Scrite::AppFeature(feature), this->availableFeatures);
}

bool UserInfo::isFeatureNameEnabled(const QString &featureName) const
{
    return Scrite::isFeatureNameEnabled(featureName, this->availableFeatures);
}

UserMessageButton::UserMessageButton(const QJsonObject &object)
{
    this->type = QMap<QString, Type>({ { "link", LinkType } })
                         .value(object.value("type").toString(), OtherType);
    this->text = object.value("text").toString();
    this->action = QMap<QString, Action>({ { "url", UrlAction }, { "command", CommandAction } })
                           .value(object.value("action").toString(), OtherAction);
    this->endpoint = object.value("endpoint").toString();
    this->params = object.value("params");
}

UserMessageButton::UserMessageButton(const UserMessageButton &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
}

bool UserMessageButton::operator==(const UserMessageButton &other) const
{
    return DeepCompareGadget(&staticMetaObject, static_cast<const void *>(&other),
                             static_cast<const void *>(this));
}

UserMessageButton &UserMessageButton::operator=(const UserMessageButton &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
    return *this;
}

QDataStream &operator<<(QDataStream &ds, const UserMessageButton &umb)
{
    ds << umb.type << umb.text << umb.action << umb.endpoint << umb.params;
    return ds;
}

QDataStream &operator>>(QDataStream &ds, UserMessageButton &umb)
{
    ds >> umb.type >> umb.text >> umb.action >> umb.endpoint >> umb.params;
    return ds;
}

UserMessage::UserMessage(const QJsonObject &object)
{
    this->id = object.value("_id").toString();
    this->type = QMap<QString, Type>({ { "important", ImportantType } })
                         .value(object.value("type").toString(), DefaultType);
    this->timestamp = QDateTime::fromString(object.value("ts").toString(), Qt::ISODateWithMs);
    this->expiresOn = QDateTime::fromString(object.value("expiry").toString(), Qt::ISODateWithMs);
    this->from = object.value("from").toString();
    this->subject = object.value("subject").toString();
    this->body = object.value("body").toString();
    this->image = QUrl(object.value("image").toString());

    const QJsonArray buttons = object.value("buttons").toArray();
    for (const QJsonValue &button : buttons)
        this->buttons.append(UserMessageButton(button.toObject()));
}

UserMessage::UserMessage(const UserMessage &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
}

bool UserMessage::operator==(const UserMessage &other) const
{
    return this->id == other.id;
}

UserMessage &UserMessage::operator=(const UserMessage &other)
{
    DeepCopyGadget(&staticMetaObject, static_cast<const void *>(&other), static_cast<void *>(this));
    return *this;
}

QDataStream &operator<<(QDataStream &ds, const UserMessage &um)
{
    ds << um.id << um.timestamp << um.expiresOn << um.from << um.subject << um.body << um.image
       << um.read << um.type;
    ds << qint8(um.buttons.size());
    for (const UserMessageButton &button : um.buttons)
        ds << button;

    return ds;
}

QDataStream &operator>>(QDataStream &ds, UserMessage &um)
{
    ds >> um.id >> um.timestamp >> um.expiresOn >> um.from >> um.subject >> um.body >> um.image
            >> um.read >> um.type;

    qint8 nrButtons = 0;
    ds >> nrButtons;

    for (qint8 i = 0; i < nrButtons; i++) {
        UserMessageButton umb;
        ds >> umb;
        um.buttons.append(umb);
    }

    return ds;
}

///////////////////////////////////////////////////////////////////////////////

User *User::instance()
{
    static bool firstTime = true;

    bool refreshSessionToken = firstTime;

    if (firstTime) {
        PeerAppLookup::instance();

        const QStringList appArgs = qApp->arguments();
        const QString starg = QStringLiteral("--sessionToken");
        const int stargPos = appArgs.indexOf(starg);
        if (stargPos >= 0 && appArgs.size() >= stargPos + 2) {
            const QString stok = appArgs.at(stargPos + 1);
            LocalStorage::store("sessionToken", stok);
            refreshSessionToken = false;
        }
    }

    static User *theUser = new User(qApp);

    if (firstTime) {
        if (refreshSessionToken && LocalStorage::load("loginToken").isValid()) {
            SessionNewRestApiCall *newSession = new SessionNewRestApiCall(theUser);
            if (!newSession->queue(RestApi::instance()->sessionApiQueue())) {
                newSession->deleteLater();
            }
        } else if (LocalStorage::load("sessionToken").isValid()) {
            QTimer::singleShot(0, theUser, &User::loadInfoUsingRestApiCall);
        }

        theUser->loadInfoFromStorage();
    }

    firstTime = false;

    return theUser;
}

User::User(QObject *parent) : QObject(parent)
{
    connect(this, &User::infoChanged, this, &User::loggedInChanged);
    connect(this, &User::loggedInChanged, this, &User::loadStoredMessages);
    connect(this, &User::messagesChanged, this, &User::storeMessages);
}

User::~User() { }

bool User::isLoggedIn() const
{
    return m_info.isValid();
}

void User::logActivity2(const QString &activity, const QJsonValue &data)
{
    if (m_info.isValid() && !this->isBusy()) {
        UserActivityRestApiCall *call = new UserActivityRestApiCall(qApp);
        call->setActivity("desktop/" + activity);
        call->setActivityData(data);
        call->call();
    }
}

bool User::isBusy() const
{
    RestApiCall *apiCall = this->findChild<RestApiCall *>(QString(), Qt::FindDirectChildrenOnly);
    return apiCall != nullptr;
}

int User::unreadMessageCount() const
{
    return std::count_if(m_messages.begin(), m_messages.end(),
                         [](const UserMessage &item) { return item.read == false; });
}

void User::checkForMessages()
{
    if (!m_checkForMessagesTimer) {
        m_checkForMessagesTimer = new QTimer(this);
        m_checkForMessagesTimer->setInterval(30000);
        connect(m_checkForMessagesTimer, &QTimer::timeout, this, &User::checkForMessagesNow);
        this->checkForMessagesNow();
    } else if (!m_checkForMessagesTimer->isActive())
        m_checkForMessagesTimer->start();
}

void User::markMessagesAsRead()
{
    int nrMessages = 0;
    for (UserMessage &message : m_messages) {
        if (!message.read) {
            message.read = true;
            ++nrMessages;
        }
    }

    if (nrMessages > 0)
        emit messagesChanged();
}

void User::setInfo(const UserInfo &val)
{
    if (m_info == val)
        return;

    m_info = val;
    emit infoChanged();

    QTimer::singleShot(100, this, &User::checkIfSubscriptionIsAboutToExpire);
}

void User::setMessages(const QList<UserMessage> &val)
{
    if (m_messages == val)
        return;

    m_messages = val;
    emit messagesChanged();
}

void User::checkIfSubscriptionIsAboutToExpire()
{
    if (!m_info.isValid() || !m_info.hasActiveSubscription)
        return;

    const bool alreadyCheckedOnceToday = []() {
        const QString lsKey = QStringLiteral("lastSubscriptionReminderDate");
        const QVariant lastReminderDateVal = LocalStorage::load(lsKey);
        if (lastReminderDateVal.isValid()) {
            const QDate dt = lastReminderDateVal.value<QDate>();
            if (dt == QDate::currentDate())
                return true;
        }

        LocalStorage::store(lsKey, QDate::currentDate());
        return false;
    }();
    if (alreadyCheckedOnceToday)
        return;

    UserMeRestApiCall *apiCall =
            this->findChild<UserMeRestApiCall *>(QString(), Qt::FindDirectChildrenOnly);
    if (apiCall) {
        QTimer::singleShot(100, this, &User::checkIfSubscriptionIsAboutToExpire);
        return;
    }

    const int subscriptionTreshold = 15;
    const int nrDays = QDate::currentDate().daysTo(m_info.subscribedUntil.date()) + 1;
    if (nrDays >= 0 && nrDays < subscriptionTreshold)
        emit subscriptionAboutToExpire(nrDays);
}

void User::checkForMessagesNow()
{
    if (!this->isLoggedIn())
        return;

    UserMessagesRestApiCall *api =
            this->findChild<UserMessagesRestApiCall *>(QString(), Qt::FindDirectChildrenOnly);
    if (api != nullptr)
        return;

    api = new UserMessagesRestApiCall(this);
    api->setAutoDelete(true);
    connect(api, &UserMessagesRestApiCall::finished, this, [=]() {
        const QJsonArray messages = api->messages();
        if (messages.isEmpty())
            return;

        QList<UserMessage> importantMessages;
        for (const QJsonValue &message : messages) {
            const UserMessage msg(message.toObject());
            if (!m_messages.contains(msg)) {
                m_messages.prepend(UserMessage(message.toObject()));
                if (msg.type == UserMessage::ImportantType)
                    importantMessages.append(msg);
            }
        }

        if (!m_messages.isEmpty())
            m_messages.erase(
                    std::remove_if(m_messages.begin(), m_messages.end(),
                                   [](const UserMessage &msg) { return msg.hasExpired(); }),
                    m_messages.end());

        if (!importantMessages.isEmpty())
            emit notifyImportantMessages(importantMessages);

        emit messagesChanged();
    });
    if (!api->call())
        api->deleteLater();
}

void User::storeMessages()
{
    if (!this->isLoggedIn())
        return;

    QByteArray messageBytes;
    {
        QDataStream ds(&messageBytes, QIODevice::WriteOnly);
        ds << m_info.id << this->m_messages;
    }
    LocalStorage::store("userMessages", messageBytes);
}

void User::loadStoredMessages()
{
    if (!this->isLoggedIn())
        return;

    m_messages.clear();

    const QByteArray messageBytes = LocalStorage::load("userMessages", QByteArray()).toByteArray();
    if (!messageBytes.isEmpty()) {
        QString userId;

        QDataStream ds(messageBytes);
        ds >> userId;

        if (userId != m_info.id) {
            LocalStorage::store("userMessages", QVariant());
        } else {
            ds >> m_messages;

            if (!m_messages.isEmpty())
                m_messages.erase(
                        std::remove_if(m_messages.begin(), m_messages.end(),
                                       [](const UserMessage &msg) { return msg.hasExpired(); }),
                        m_messages.end());

            if (!m_messages.isEmpty()) {
                QList<UserMessage> importantMessages;
                std::copy_if(m_messages.begin(), m_messages.end(),
                             std::back_inserter(importantMessages), [](const UserMessage &msg) {
                                 return !msg.read && msg.type == UserMessage::ImportantType;
                             });
                if (!importantMessages.isEmpty())
                    emit notifyImportantMessages(importantMessages);
            }
        }
    }

    emit messagesChanged();
}

void User::loadInfoFromStorage()
{
    const QString token = LocalStorage::load("loginToken").toString();
    if (!token.isEmpty()) {
        const QByteArray userJson = LocalStorage::load("user").toByteArray();

        QJsonParseError error;
        const QJsonObject user = QJsonDocument::fromJson(userJson, &error).object();
        if (error.error == QJsonParseError::NoError) {
            this->setInfo(UserInfo(user));
        } else {
            this->setInfo(UserInfo());
        }
    } else {
        this->setInfo(UserInfo());
    }
}

void User::loadInfoUsingRestApiCall()
{
    UserMeRestApiCall *apiCall =
            this->findChild<UserMeRestApiCall *>(QString(), Qt::FindDirectChildrenOnly);
    if (apiCall)
        return;

    apiCall = new UserMeRestApiCall(this);
    if (!apiCall->call())
        apiCall->deleteLater();
}

void User::childEvent(QChildEvent *e)
{
    QTimer::singleShot(0, this, &User::busyChanged);

    QObject::childEvent(e);
}

///////////////////////////////////////////////////////////////////////////////

AppFeature::AppFeature(QObject *parent) : QObject(parent)
{
    connect(User::instance(), &User::infoChanged, this, &AppFeature::reevaluate);
}

AppFeature::~AppFeature() { }

bool AppFeature::isEnabled(int feature)
{
    if (User::instance()->isLoggedIn() && User::instance()->info().hasActiveSubscription) {
        return feature < 0 ? false
                           : User::instance()->info().isFeatureEnabled(Scrite::AppFeature(feature));
    }

    return false;
}

bool AppFeature::isEnabled(const QString &featureName)
{
    if (User::instance()->isLoggedIn() && User::instance()->info().hasActiveSubscription) {
        return featureName.isEmpty() ? false
                                     : User::instance()->info().isFeatureNameEnabled(featureName);
    }

    return false;
}

void AppFeature::setFeatureName(const QString &val)
{
    if (m_featureName == val)
        return;

    m_featureName = val;
    emit featureNameChanged();
    this->reevaluate();
}

void AppFeature::setFeature(int val)
{
    if (m_feature == val)
        return;

    m_feature = val;
    emit featureChanged();
    this->reevaluate();
}

void AppFeature::reevaluate()
{
    if (User::instance()->isLoggedIn() && User::instance()->info().hasActiveSubscription) {
        const bool flag1 = m_feature < 0
                ? true
                : User::instance()->info().isFeatureEnabled(Scrite::AppFeature(m_feature));
        const bool flag2 = m_featureName.isEmpty()
                ? true
                : User::instance()->info().isFeatureNameEnabled(m_featureName);
        this->setEnabled(flag1 && flag2);
    } else
        this->setEnabled(false);
}

void AppFeature::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}
