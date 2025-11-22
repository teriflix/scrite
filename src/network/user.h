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

#ifndef USER_H
#define USER_H

#include <QUrl>
#include <QDateTime>
#include <QQmlEngine>
#include <QJsonValue>
#include <QQuickImageProvider>

struct UserInstallationInfo
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    UserInstallationInfo() { }
    UserInstallationInfo(const QJsonObject &object);
    UserInstallationInfo(const UserInstallationInfo &other);

    bool operator==(const UserInstallationInfo &other) const;
    bool operator!=(const UserInstallationInfo &other) const { return !(*this == other); }
    UserInstallationInfo &operator=(const UserInstallationInfo &other);

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const { return !id.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString id
               MEMBER id)
    // clang-format on
    QString id;

    // clang-format off
    Q_PROPERTY(QString clientId
               MEMBER clientId)
    // clang-format on
    QString clientId;

    // clang-format off
    Q_PROPERTY(QString deviceId
               MEMBER deviceId)
    // clang-format on
    QString deviceId;

    // clang-format off
    Q_PROPERTY(QString platform
               MEMBER platform)
    // clang-format on
    QString platform;

    // clang-format off
    Q_PROPERTY(QString platformVersion
               MEMBER platformVersion)
    // clang-format on
    QString platformVersion;

    // clang-format off
    Q_PROPERTY(QString platformType
               MEMBER platformType)
    // clang-format on
    QString platformType;

    // clang-format off
    Q_PROPERTY(QString hostName
               MEMBER hostName)
    // clang-format on
    QString hostName;

    // clang-format off
    Q_PROPERTY(QString appVersion
               MEMBER appVersion)
    // clang-format on
    QString appVersion;

    // clang-format off
    Q_PROPERTY(QStringList pastAppVersions
               MEMBER pastAppVersions)
    // clang-format on
    QStringList pastAppVersions;

    // clang-format off
    Q_PROPERTY(QDateTime creationDate
               MEMBER creationDate)
    // clang-format on
    QDateTime creationDate;

    // clang-format off
    Q_PROPERTY(QDateTime lastActivationDate
               MEMBER lastActivationDate)
    // clang-format on
    QDateTime lastActivationDate;

    // clang-format off
    Q_PROPERTY(QDateTime lastSessionDate
               MEMBER lastSessionDate)
    // clang-format on
    QDateTime lastSessionDate;

    // clang-format off
    Q_PROPERTY(bool activated
               MEMBER activated)
    // clang-format on
    bool activated = false;

    // clang-format off
    Q_PROPERTY(bool isCurrent
               READ isCurrent)
    // clang-format on
    bool isCurrent() const; // TODO
};
Q_DECLARE_METATYPE(UserInstallationInfo)
Q_DECLARE_METATYPE(QList<UserInstallationInfo>)

struct UserSubscriptionPlanInfo
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    UserSubscriptionPlanInfo() { }
    UserSubscriptionPlanInfo(const QJsonObject &object);
    UserSubscriptionPlanInfo(const UserSubscriptionPlanInfo &other);

    bool operator==(const UserSubscriptionPlanInfo &other) const;
    bool operator!=(const UserSubscriptionPlanInfo &other) const { return !(*this == other); }
    UserSubscriptionPlanInfo &operator=(const UserSubscriptionPlanInfo &other);

    // clang-format off
    Q_PROPERTY(QString name
               MEMBER name)
    // clang-format on
    QString name;

    // clang-format off
    Q_PROPERTY(QString kind
               MEMBER kind)
    // clang-format on
    QString kind;

    // clang-format off
    Q_PROPERTY(QString title
               MEMBER title)
    // clang-format on
    QString title;

    // clang-format off
    Q_PROPERTY(QString subtitle
               MEMBER subtitle)
    // clang-format on
    QString subtitle;

    // clang-format off
    Q_PROPERTY(int duration
               MEMBER duration)
    // clang-format on
    int duration = 0;

    // clang-format off
    Q_PROPERTY(bool exclusive
               MEMBER exclusive)
    // clang-format on
    bool exclusive = false;

    // clang-format off
    Q_PROPERTY(QString currency
               MEMBER currency)
    // clang-format on
    QString currency;

    // clang-format off
    Q_PROPERTY(qreal price
               MEMBER price)
    // clang-format on
    qreal price = 0;

    // clang-format off
    Q_PROPERTY(QStringList features
               MEMBER features)
    // clang-format on
    QStringList features;

    // clang-format off
    Q_PROPERTY(QString featureNote
               MEMBER featureNote)
    // clang-format on
    QString featureNote;

    // clang-format off
    Q_PROPERTY(int devices
               MEMBER devices)
    // clang-format on
    int devices = 1;
};
Q_DECLARE_METATYPE(UserSubscriptionPlanInfo)
Q_DECLARE_METATYPE(QList<UserSubscriptionPlanInfo>)

struct UserSubscriptionInfo
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    UserSubscriptionInfo() { }
    UserSubscriptionInfo(const QJsonObject &object);
    UserSubscriptionInfo(const UserSubscriptionInfo &other);

    bool operator==(const UserSubscriptionInfo &other) const;
    bool operator!=(const UserSubscriptionInfo &other) const { return !(*this == other); }
    UserSubscriptionInfo &operator=(const UserSubscriptionInfo &other);

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const { return !id.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString id
               MEMBER id)
    // clang-format on
    QString id;

    // clang-format off
    Q_PROPERTY(QString kind
               MEMBER kind)
    // clang-format on
    QString kind;

    // clang-format off
    Q_PROPERTY(UserSubscriptionPlanInfo plan
               MEMBER plan)
    // clang-format on
    UserSubscriptionPlanInfo plan;

    // clang-format off
    Q_PROPERTY(QDateTime from
               MEMBER from)
    // clang-format on
    QDateTime from;

    // clang-format off
    Q_PROPERTY(QDateTime until
               MEMBER until)
    // clang-format on
    QDateTime until;

    // clang-format off
    Q_PROPERTY(QString wc_order_id
               MEMBER orderId)
    Q_PROPERTY(QString orderId
               MEMBER orderId)
    // clang-format on
    QString orderId;

    // clang-format off
    Q_PROPERTY(QUrl detailsUrl
               MEMBER detailsUrl)
    // clang-format on
    QUrl detailsUrl;

    // clang-format off
    Q_PROPERTY(bool isActive
               MEMBER isActive)
    // clang-format on
    bool isActive = false;

    // clang-format off
    Q_PROPERTY(bool isUpcoming
               MEMBER isUpcoming)
    // clang-format on
    bool isUpcoming = false;

    // clang-format off
    Q_PROPERTY(bool hasExpired
               MEMBER hasExpired)
    // clang-format on
    bool hasExpired = false;

    // clang-format off
    Q_PROPERTY(int daysToUntil
               READ daysToUntil)
    // clang-format on
    int daysToUntil() const { return QDateTime::currentDateTime().daysTo(this->until) + 1; }

    // clang-format off
    Q_PROPERTY(int daysToFrom
               READ daysToFrom)
    // clang-format on
    int daysToFrom() const { return QDateTime::currentDateTime().daysTo(this->from) + 1; }

    // clang-format off
    Q_PROPERTY(QString description
               READ description)
    // clang-format on
    QString description() const;

    // feature is anything from Scrite::AppFeature
    Q_INVOKABLE bool isFeatureEnabled(int feature) const;
    Q_INVOKABLE bool isFeatureNameEnabled(const QString &featureName) const;
};
Q_DECLARE_METATYPE(UserSubscriptionInfo)
Q_DECLARE_METATYPE(QList<UserSubscriptionInfo>)

struct UserInfo
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    UserInfo() { }
    UserInfo(const QJsonObject &object);
    UserInfo(const UserInfo &other);

    bool operator==(const UserInfo &other) const;
    bool operator!=(const UserInfo &other) const { return !(*this == other); }
    UserInfo &operator=(const UserInfo &other);

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const { return !id.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString id
               MEMBER id)
    // clang-format on
    QString id;

    // clang-format off
    Q_PROPERTY(QString email
               MEMBER email)
    // clang-format on
    QString email;

    // clang-format off
    Q_PROPERTY(QDateTime signUpDate
               MEMBER signUpDate)
    // clang-format on
    QDateTime signUpDate;

    // clang-format off
    Q_PROPERTY(QDateTime timestamp
               MEMBER timestamp)
    // clang-format on
    QDateTime timestamp;

    // clang-format off
    Q_PROPERTY(QString firstName
               MEMBER firstName)
    // clang-format on
    QString firstName;

    // clang-format off
    Q_PROPERTY(QString lastName
               MEMBER lastName)
    // clang-format on
    QString lastName;

    // clang-format off
    Q_PROPERTY(QString fullName
               MEMBER fullName)
    // clang-format on
    QString fullName;

    // clang-format off
    Q_PROPERTY(QString experience
               MEMBER experience)
    // clang-format on
    QString experience;

    // clang-format off
    Q_PROPERTY(QString phone
               MEMBER phone)
    // clang-format on
    QString phone;

    // clang-format off
    Q_PROPERTY(QString city
               MEMBER city)
    // clang-format on
    QString city;

    // clang-format off
    Q_PROPERTY(QString country
               MEMBER country)
    // clang-format on
    QString country;

    // clang-format off
    Q_PROPERTY(QString wdyhas
               MEMBER wdyhas)
    // clang-format on
    QString wdyhas;

    // clang-format off
    Q_PROPERTY(bool consentToActivityLog
               MEMBER consentToActivityLog)
    // clang-format on
    bool consentToActivityLog = false;

    // clang-format off
    Q_PROPERTY(bool consentToEmail
               MEMBER consentToEmail)
    // clang-format on
    bool consentToEmail = false;

    // clang-format off
    Q_PROPERTY(QStringList allowedVersionTypes
               MEMBER allowedVersionTypes)
    // clang-format on
    QStringList allowedVersionTypes;

    // clang-format off
    Q_PROPERTY(QList<UserInstallationInfo> installations
               MEMBER installations)
    // clang-format on
    QList<UserInstallationInfo> installations;

    // clang-format off
    Q_PROPERTY(QList<UserSubscriptionInfo> subscriptions
               MEMBER subscriptions)
    // clang-format on
    QList<UserSubscriptionInfo> subscriptions;

    // clang-format off
    Q_PROPERTY(UserSubscriptionInfo publicBetaSubscription
               MEMBER publicBetaSubscription)
    // clang-format on
    UserSubscriptionInfo publicBetaSubscription;

    // clang-format off
    Q_PROPERTY(int activeInstallationCount
               MEMBER activeInstallationCount)
    // clang-format on
    int activeInstallationCount = 0;

    // clang-format off
    Q_PROPERTY(bool hasActiveSubscription
               MEMBER hasActiveSubscription)
    // clang-format on
    bool hasActiveSubscription = false;

    // clang-format off
    Q_PROPERTY(bool hasUpcomingSubscription
               MEMBER hasUpcomingSubscription)
    // clang-format on
    bool hasUpcomingSubscription = false;

    // clang-format off
    Q_PROPERTY(bool hasTrialSubscription
               MEMBER hasTrialSubscription)
    // clang-format on
    bool hasTrialSubscription = false;

    // clang-format off
    Q_PROPERTY(int paidSubscriptionCount
               MEMBER paidSubscriptionCount)
    // clang-format on
    int paidSubscriptionCount = 0;

    // clang-format off
    Q_PROPERTY(QDateTime subscribedUntil
               MEMBER subscribedUntil)
    // clang-format on
    QDateTime subscribedUntil;

    // clang-format off
    Q_PROPERTY(bool isEarlyAdopter
               MEMBER isEarlyAdopter)
    // clang-format on
    bool isEarlyAdopter = false;

    // clang-format off
    Q_PROPERTY(QStringList availableFeatures
               MEMBER availableFeatures)
    // clang-format on
    QStringList availableFeatures;

    // clang-format off
    Q_PROPERTY(QUrl badgeImageUrl
               MEMBER badgeImageUrl)
    // clang-format on
    QUrl badgeImageUrl;

    // clang-format off
    Q_PROPERTY(QColor badgeTextColor
               MEMBER badgeTextColor)
    // clang-format on
    QColor badgeTextColor = Qt::white;

    Q_INVOKABLE int daysToSubscribedUntil() const;
    Q_INVOKABLE bool isFeatureEnabled(int feature) const;
    Q_INVOKABLE bool isFeatureNameEnabled(const QString &featureName) const;
    Q_INVOKABLE QString initials() const;
};
Q_DECLARE_METATYPE(UserInfo)

struct UserMessageButton
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    UserMessageButton() { }
    UserMessageButton(const QJsonObject &object);
    UserMessageButton(const UserMessageButton &other);

    bool operator==(const UserMessageButton &other) const;
    bool operator!=(const UserMessageButton &other) const { return !(*this == other); }
    UserMessageButton &operator=(const UserMessageButton &other);

    enum Type { LinkType, OtherType };
    Q_ENUM(Type)

    // clang-format off
    Q_PROPERTY(Type type
               MEMBER type)
    // clang-format on
    Type type = LinkType;

    // clang-format off
    Q_PROPERTY(QString text
               MEMBER text)
    // clang-format on
    QString text;

    enum Action { UrlAction, CommandAction, OtherAction };
    Q_ENUM(Action)

    // clang-format off
    Q_PROPERTY(Action action
               MEMBER action)
    // clang-format on
    Action action = UrlAction;

    // clang-format off
    Q_PROPERTY(QString endpoint
               MEMBER endpoint)
    // clang-format on
    QString endpoint;

    // clang-format off
    Q_PROPERTY(QJsonValue params
               MEMBER params)
    // clang-format on
    QJsonValue params;
};
Q_DECLARE_METATYPE(UserMessageButton)
Q_DECLARE_METATYPE(QList<UserMessageButton>)

struct UserMessage
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    UserMessage() { }
    UserMessage(const QJsonObject &object);
    UserMessage(const UserMessage &other);

    bool operator==(const UserMessage &other) const;
    bool operator!=(const UserMessage &other) const { return !(*this == other); }
    UserMessage &operator=(const UserMessage &other);

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const { return !id.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString id
               MEMBER id)
    // clang-format on
    QString id;

    enum Type { DefaultType, ImportantType };
    Q_ENUM(Type)

    // clang-format off
    Q_PROPERTY(Type type
               MEMBER type)
    // clang-format on
    Type type = DefaultType;

    // clang-format off
    Q_PROPERTY(QString from
               MEMBER from)
    // clang-format on
    QString from;

    // clang-format off
    Q_PROPERTY(QDateTime timestamp
               MEMBER timestamp)
    // clang-format on
    QDateTime timestamp;

    // clang-format off
    Q_PROPERTY(QDateTime expiresOn
               MEMBER expiresOn)
    // clang-format on
    QDateTime expiresOn;

    // clang-format off
    Q_PROPERTY(bool hasExpired
               READ hasExpired)
    // clang-format on
    bool hasExpired() const
    {
        return this->isValid() && QDateTime::currentDateTime() > this->expiresOn;
    }

    // clang-format off
    Q_PROPERTY(QString subject
               MEMBER subject)
    // clang-format on
    QString subject;

    // clang-format off
    Q_PROPERTY(QUrl image
               MEMBER image)
    // clang-format on
    QUrl image;

    // clang-format off
    Q_PROPERTY(QString body
               MEMBER body)
    // clang-format on
    QString body;

    // clang-format off
    Q_PROPERTY(bool read
               MEMBER read)
    // clang-format on
    bool read = false;

    // clang-format off
    Q_PROPERTY(QList<UserMessageButton> buttons
               MEMBER buttons)
    // clang-format on
    QList<UserMessageButton> buttons;
};
Q_DECLARE_METATYPE(UserMessage)
Q_DECLARE_METATYPE(QList<UserMessage>)

class User : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static User *instance();
    ~User();

    // clang-format off
    Q_PROPERTY(bool loggedIn
               READ isLoggedIn
               NOTIFY infoChanged)
    // clang-format on
    bool isLoggedIn() const;
    Q_SIGNAL void loggedInChanged();

    // clang-format off
    Q_PROPERTY(UserInfo info
               READ info
               NOTIFY infoChanged)
    // clang-format on
    UserInfo info() const { return m_info; }
    Q_SIGNAL void infoChanged();

    // clang-format off
    Q_PROPERTY(bool canUseAppVersionType
               READ canUseAppVersionType
               NOTIFY infoChanged)
    // clang-format on
    bool canUseAppVersionType() const;

    Q_SLOT void logActivity1(const QString &activity)
    {
        this->logActivity2(activity, QJsonValue());
    }
    Q_SLOT void logActivity2(const QString &activity, const QJsonValue &data);

    // clang-format off
    Q_PROPERTY(bool busy
               READ isBusy
               NOTIFY busyChanged)
    // clang-format on
    bool isBusy() const;
    Q_SIGNAL void busyChanged();

    // clang-format off
    Q_PROPERTY(QList<UserMessage> messages
               READ messages
               WRITE setMessages
               NOTIFY messagesChanged)
    // clang-format on
    QList<UserMessage> messages() const { return m_messages; }
    Q_SIGNAL void messagesChanged();

    // clang-format off
    Q_PROPERTY(int totalMessageCount
               READ totalMessageCount
               NOTIFY messagesChanged)
    // clang-format on
    int totalMessageCount() const { return m_messages.size(); }

    // clang-format off
    Q_PROPERTY(int unreadMessageCount
               READ unreadMessageCount
               NOTIFY messagesChanged)
    // clang-format on
    int unreadMessageCount() const;

    Q_INVOKABLE void checkForMessages();
    Q_INVOKABLE void markMessagesAsRead();

    void checkIfVersionTypeUseIsAllowed();

signals:
    void subscriptionAboutToExpire(int days);
    void notifyImportantMessages(const QList<UserMessage> &messages);
    void requestVersionTypeAccess();

private:
    User(QObject *parent = nullptr);

    void setInfo(const UserInfo &val);
    void setMessages(const QList<UserMessage> &val);
    void checkIfSubscriptionIsAboutToExpire();
    void checkIfInstallationInfoNeedsUpdate();

    void checkForMessagesNow();
    void storeMessages();
    void loadStoredMessages();

public: // Don't use these methods
    void loadInfoFromStorage();
    void loadInfoUsingRestApiCall();

protected:
    void childEvent(QChildEvent *e);

private:
    UserInfo m_info;
    QList<UserMessage> m_messages;
    QTimer *m_checkForMessagesTimer = nullptr;
};

class AppFeature : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AppFeature(QObject *parent = nullptr);
    ~AppFeature();

    static bool isEnabled(int feature);
    static bool isEnabled(const QString &featureName);

    // clang-format off
    Q_PROPERTY(QString featureName
               READ featureName
               WRITE setFeatureName
               NOTIFY featureNameChanged)
    // clang-format on
    void setFeatureName(const QString &val);
    QString featureName() const { return m_featureName; }
    Q_SIGNAL void featureNameChanged();

    // clang-format off
    Q_PROPERTY(int feature
               READ feature
               WRITE setFeature
               NOTIFY featureChanged)
    // clang-format on
    void setFeature(int val);
    int feature() const { return m_feature; }
    Q_SIGNAL void featureChanged();

    // clang-format off
    Q_PROPERTY(bool enabled
               READ isEnabled
               NOTIFY enabledChanged)
    // clang-format on
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

private:
    void reevaluate();
    void setEnabled(bool val);

private:
    QString m_featureName;
    int m_feature = -1;
    bool m_enabled = false;
};

#endif // USER_H
