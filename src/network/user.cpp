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

bool UserInfo::isFeatureEnabled(int feature) const
{
    return Scrite::isFeatureEnabled(Scrite::AppFeature(feature), this->availableFeatures);
}

bool UserInfo::isFeatureNameEnabled(const QString &featureName) const
{
    return Scrite::isFeatureNameEnabled(featureName, this->availableFeatures);
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
            if (!newSession->call()) {
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

void User::setInfo(const UserInfo &val)
{
    if (m_info == val)
        return;

    m_info = val;
    emit infoChanged();

    QTimer::singleShot(100, this, &User::checkIfSubscriptionIsAboutToExpire);
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
    if (nrDays < subscriptionTreshold)
        emit subscriptionAboutToExpire(nrDays);
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
    connect(apiCall, &UserMeRestApiCall::destroyed, this, &User::busyChanged, Qt::QueuedConnection);

    if (!apiCall->call())
        apiCall->deleteLater();
}

void User::childEvent(QChildEvent *e)
{
    QTimer::singleShot(0, this, &User::busyChanged);

    QObject::childEvent(e);
}

///////////////////////////////////////////////////////////////////////////////

UserIconProvider::UserIconProvider() : QQuickImageProvider(QQmlImageProviderBase::Image) { }

UserIconProvider::~UserIconProvider() { }

QImage UserIconProvider::requestImage(const QString &, QSize *size, const QSize &requestedSize)
{
    const int dim = qMax(100, qMin(requestedSize.width(), requestedSize.height()));

    QImage image(QSize(dim, dim), QImage::Format_ARGB32);
    image.setDevicePixelRatio(2.0);
    image.fill(Qt::transparent);

    QPainter paint(&image);
    paint.scale(0.5, 0.5);
    paint.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QColor baseColor("#65318f");
    enum BadgeShape { CircleShape, DiamondShape, StarShape };

    if (User::instance()->isLoggedIn()) {
        const QColor shapeColor = [baseColor]() {
            const UserInfo userInfo = User::instance()->info();
            const QList<UserSubscriptionInfo> subs = User::instance()->info().subscriptions;
            const QMap<QString, QColor> colorMap = { { "paid", QColor("#e9bf5a") },
                                                     { "internal", QColor(Qt::white) },
                                                     { "trial", QColor("#8dc63f") } };

            if (userInfo.hasUpcomingSubscription)
                return colorMap.value(subs[1].kind, baseColor);

            if (userInfo.hasActiveSubscription)
                return colorMap.value(subs[0].kind, baseColor);

            return QColor(Qt::lightGray);
        }();
        const QColor ribbonColor = [baseColor, shapeColor]() {
            const UserInfo userInfo = User::instance()->info();
            const QList<UserSubscriptionInfo> subs = User::instance()->info().subscriptions;
            if (userInfo.hasActiveSubscription
                && QStringList({ "trial", "paid" }).contains(subs[0].kind))
                return shapeColor;
            return baseColor;
        }();
        const QColor containerColor = baseColor;
        const QString daysLeft = []() {
            const UserInfo userInfo = User::instance()->info();
            const QList<UserSubscriptionInfo> subs = User::instance()->info().subscriptions;
            if (userInfo.hasActiveSubscription && subs[0].kind == "trial")
                return QString::number(QDateTime::currentDateTime().daysTo(subs[0].until) + 1);
            return QString();
        }();

        const QString initials = []() {
            QString ret;
            const UserInfo userInfo = User::instance()->info();

            if (!userInfo.firstName.isEmpty())
                ret = userInfo.firstName.at(0);
            else if (!userInfo.lastName.isEmpty())
                ret = userInfo.lastName.at(0);
            else
                ret = userInfo.email.at(0);

            return ret.toUpper();
        }();

        const BadgeShape shape = []() -> BadgeShape {
            const UserInfo userInfo = User::instance()->info();
            const QList<UserSubscriptionInfo> subs = User::instance()->info().subscriptions;
            const QMap<QString, BadgeShape> shapeMap = { { "sponsored", StarShape },
                                                         { "internal", StarShape },
                                                         { "paid", DiamondShape } };

            if (userInfo.hasUpcomingSubscription)
                return shapeMap.value(subs[1].kind, CircleShape);

            if (userInfo.hasActiveSubscription)
                return shapeMap.value(subs[0].kind, CircleShape);

            return CircleShape;
        }();

        const QSizeF badgeSize = image.size();
        const QRectF badgeRect = image.rect();

        const qreal outerCirclePenSize = badgeSize.width() * 0.03;

        paint.setPen(QPen(ribbonColor, outerCirclePenSize));
        paint.setBrush(Qt::NoBrush);
        paint.drawArc(QRectF(badgeRect.x(), badgeRect.y(), badgeRect.width(), badgeRect.width())
                              .adjusted(outerCirclePenSize / 2, outerCirclePenSize / 2,
                                        -outerCirclePenSize / 2, -outerCirclePenSize / 2),
                      0, 5760);

        paint.setPen(Qt::NoPen);
        paint.setBrush(QBrush(containerColor));
        paint.drawPie(QRectF(badgeRect.x(), badgeRect.y(), badgeRect.width(), badgeRect.width())
                              .adjusted(outerCirclePenSize * 2, outerCirclePenSize * 2,
                                        -outerCirclePenSize * 2, -outerCirclePenSize * 2),
                      0, 5760);

        QPainterPath shapePath;
        switch (shape) {
        case CircleShape: {
            const qreal hbw = badgeSize.width() * 0.5;
            QRectF circleRect(0, 0, hbw, hbw);
            circleRect.moveCenter(badgeRect.topLeft() + QPointF(hbw, hbw));
            shapePath.addEllipse(circleRect);
        } break;
        case DiamondShape: {
            const qreal hbw = badgeSize.width() * 0.5;
            QRectF rRect(0, 0, hbw, hbw);
            rRect.moveCenter(badgeRect.topLeft() + QPointF(hbw, hbw));

            QTransform tx;
            tx.translate(rRect.center().x(), rRect.center().y());
            tx.rotate(45);
            tx.translate(-rRect.center().x(), -rRect.center().y());

            shapePath.addRoundedRect(rRect, 20, 20, Qt::RelativeSize);
            shapePath = tx.map(shapePath);
        } break;
        case StarShape: {
            const qreal hbw = badgeSize.width() * 0.5;
            QRectF rRect(0, 0, hbw, hbw);
            rRect.moveCenter(badgeRect.topLeft() + QPointF(hbw, hbw));

            const int nr = 5;
            const qreal angleStep = 360.0 / qreal(nr * 2);
            const qreal outerRadius = badgeSize.width() * 0.4 - outerCirclePenSize * 2;
            const qreal innerRadius = badgeSize.width() * 0.25 - outerCirclePenSize * 2;

            QTransform tx;
            for (int i = 0; i < nr * 2; i++) {
                const QPointF p = tx.map(QPointF(0, i % 2 ? outerRadius : innerRadius));
                if (i)
                    shapePath.lineTo(p);
                else
                    shapePath.moveTo(p);

                tx.rotate(angleStep);
            }
            shapePath.closeSubpath();

            tx = QTransform();
            tx.translate(rRect.center().x(), rRect.center().y());
            shapePath = tx.map(shapePath);
        } break;
        default:
            break;
        }

        paint.setPen(QPen(shapeColor, outerCirclePenSize * 2, Qt::SolidLine, Qt::RoundCap,
                          Qt::RoundJoin));
        paint.setBrush(QBrush(shapeColor));
        paint.drawPath(shapePath);

        if (!initials.isEmpty()) {
            QFont font = qApp->font();
            font.setBold(true);
            font.setPixelSize(badgeSize.width() * 0.4);
            paint.setPen(containerColor);
            paint.setFont(font);
            paint.setBrush(Qt::NoBrush);

            QFontMetricsF fm(font);
            QRectF textRect = fm.tightBoundingRect(initials);
            textRect.moveCenter(badgeRect.topLeft()
                                + QPointF(badgeSize.width() / 2, badgeSize.width() / 2));

            const QColor textColor = Application::textColorFor(shapeColor);
            paint.setPen(textColor);
            paint.drawText(textRect.bottomLeft(), initials);
        }

        if (!daysLeft.isEmpty()) {
            QFont font = qApp->font();
            font.setBold(true);
            font.setPixelSize(badgeSize.width() * 0.15);

            QFontMetricsF fm(font);
            QRectF textRect = fm.tightBoundingRect(daysLeft);
            textRect.moveCenter(badgeRect.topLeft()
                                + QPointF(badgeSize.width() / 2, badgeSize.width() / 2));
            textRect.moveBottom(badgeRect.top() + badgeSize.width() - outerCirclePenSize * 3);

            paint.setFont(font);

            const QColor textColor = Application::textColorFor(containerColor);
            paint.setPen(textColor);

            paint.drawText(textRect.bottomLeft(), daysLeft);
        }
    } else {
        paint.setBrush(QBrush(baseColor));
        paint.drawEllipse(image.rect().adjusted(2, 2, -2, -2));

        QRectF iconRect = image.rect();
        const qreal iconMargin = iconRect.width() * 0.1;
        iconRect.adjust(iconMargin, iconMargin, -iconMargin, -iconMargin);
        paint.setRenderHint(QPainter::SmoothPixmapTransform);
        paint.drawImage(iconRect, QImage(":/icons/content/person_outline_inverted.png"));
    }

    paint.end();

    if (size)
        *size = image.size();

    return image;
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
