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
#include "callgraph.h"
#include "application.h"
#include "timeprofiler.h"
#include "peerapplookup.h"
#include "scritedocument.h"
#include "jsonhttprequest.h"

#include <QtDebug>
#include <QPainter>
#include <QLocale>
#include <QSettings>
#include <QDateTime>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QScopedValueRollback>

static QString GetSessionExpiredErrorMessage(const QString &context)
{
    if (context == "E_ACTIVE_SUBSCRIPTION")
        return context
                + QStringLiteral(": No active subscription was found. Please subscribe to a plan "
                                 "to continue using Scrite.");

    return context
            + QStringLiteral(": Your login session has expired and therefore your device is "
                             "deactivated. Please connect to the Internet and "
                             "login/reactivate your installation of Scrite.");
}

User *User::instance()
{
    // CAPTURE_FIRST_CALL_GRAPH;

    static bool firstTime = true;
    if (firstTime) {
        User::locations();
        PeerAppLookup::instance();

        const QStringList appArgs = qApp->arguments();
        const QString starg = QStringLiteral("--sessionToken");
        const int stargPos = appArgs.indexOf(starg);
        if (stargPos >= 0 && appArgs.size() >= stargPos + 2) {
            const QString stok = appArgs.at(stargPos + 1);
            JsonHttpRequest::store(QStringLiteral("sessionToken"), stok);
        }
    }

    static User *theUser = new User(qApp);

    if (firstTime) {
        theUser->loadStoredInformation();
        theUser->firstReload(false);
    }

    firstTime = false;

    return theUser;
}

User::User(QObject *parent) : QObject(parent)
{
    // CAPTURE_CALL_GRAPH;

    connect(this, &User::infoChanged, this, &User::loggedInChanged);
    connect(this, &User::installationsChanged, this, &User::loggedInChanged);

    m_touchLogTimer.setSingleShot(false);
    m_touchLogTimer.setInterval(10 * 60 * 1000); // 10 minutes
    connect(&m_touchLogTimer, &QTimer::timeout, this,
            [=]() { this->logActivity1(QStringLiteral("touch")); });
    connect(this, &User::loggedInChanged, this, [=]() {
        if (this->isLoggedIn())
            m_touchLogTimer.start();
        else
            m_touchLogTimer.stop();
    });
}

User::~User() { }

bool User::isLoggedIn() const
{
    return !m_info.isEmpty() && !m_installations.isEmpty();
}

QString User::email() const
{
    return m_info.value(QStringLiteral("email")).toString().toLower();
}

QString User::firstName() const
{
    return m_info.value(QStringLiteral("firstName")).toString();
}

QString User::lastName() const
{
    return m_info.value(QStringLiteral("lastName")).toString();
}

QString User::fullName() const
{
    return QStringList({ this->firstName(), this->lastName() }).join(QStringLiteral(" ")).trimmed();
}

QString User::location() const
{
    return m_info.value(QStringLiteral("location")).toString();
}

QString User::experience() const
{
    return m_info.value(QStringLiteral("experience")).toString();
}

QString User::wdyhas() const
{
    return m_info.value(QStringLiteral("wdyhas")).toString();
}

QString User::country() const
{
    return m_info.value(QStringLiteral("country")).toString();
}

QString User::currency() const
{
    return m_info.value(QStringLiteral("currency")).toString();
}

static QJsonObject findActiveSubscription(const QJsonArray &subscriptions)
{
    for (const QJsonValue &subValue : subscriptions) {
        const QJsonObject sub = subValue.toObject();
        if (sub.value("active").toBool())
            return sub;
    }

    return QJsonObject();
}

QJsonObject User::activeSubscription() const
{
    return findActiveSubscription(m_subscriptions);
}

bool User::hasActiveSubscription() const
{
    const QJsonObject activeSub = this->activeSubscription();
    return !activeSub.isEmpty();
}

QString User::activeSubscriptionDescription() const
{
    QString ret;

    if (this->hasActiveSubscription()) {
        const QJsonObject activeSub = this->activeSubscription();
        const QString planType = activeSub.value("plan_kind").toString();
        const QString planName = activeSub.value("plan_name").toString();
        const QDate validUntil =
                QDate::fromString(activeSub.value("end_date").toString(), Qt::ISODate);
        const int nrDays = QDate::currentDate().daysTo(validUntil) + 1;

        ret += planName;
        if (nrDays <= 30 || planType == "trial") {
            if (nrDays == 1)
                ret += " Expires Tomorrow";
            else if (nrDays == 0)
                ret += " Expires Today!";
            else
                ret += " - " + QString::number(nrDays) + " Days Left";
        }
    }

    return ret;
}

static QJsonObject findUpcomingSubscription(const QJsonArray &subscriptions)
{
    for (const QJsonValue &subValue : subscriptions) {
        const QJsonObject sub = subValue.toObject();
        if (sub.value("pending").toBool())
            return sub;
    }

    return QJsonObject();
}

QJsonObject User::upcomingSubscription() const
{
    return findUpcomingSubscription(m_subscriptions);
}

bool User::hasUpcomingSubscription() const
{
    const QJsonObject upcomingSub = this->upcomingSubscription();
    return !upcomingSub.isEmpty();
}

QString User::upcomingSubscriptionDescription() const
{
    QString ret;

    if (this->hasUpcomingSubscription()) {
        const QJsonObject upcomingSub = this->upcomingSubscription();
        const QString planType = upcomingSub.value("plan_kind").toString();
        const QString planName = upcomingSub.value("plan_name").toString();
        const QDate validUntil =
                QDate::fromString(upcomingSub.value("end_date").toString(), Qt::ISODate);
        const int nrDays = QDate::currentDate().daysTo(validUntil) + 1;

        ret += planName;
        if (nrDays <= 30 || planType == "trial") {
            if (nrDays == 1)
                ret += " Expires Tomorrow";
            else if (nrDays == 0)
                ret += " Expires Today!";
            else
                ret += " - " + QString::number(nrDays) + " Days Left";
        }
    }

    return ret;
}

QStringList User::locations()
{
    static QStringList ret;
    if (ret.isEmpty()) {
        // CAPTURE_CALL_GRAPH;
        QFile ccdb(QStringLiteral(":/misc/city-country-map.json.compressed"));
        if (ccdb.open(QFile::ReadOnly)) {
            const QByteArray json = qUncompress(ccdb.readAll());
            const QJsonObject jsonMap = QJsonDocument::fromJson(json).object();
            QJsonObject::const_iterator it = jsonMap.constBegin();
            QJsonObject::const_iterator end = jsonMap.constEnd();
            QSet<QString> locs;
            while (it != end) {
                const QString cityCountry = it.key() + QStringLiteral(", ") + it.value().toString();
                locs << cityCountry;
                ++it;
            }

            ret = locs.values();
        }
    }

    return ret;
}

bool User::isFeatureNameEnabled(const QString &featureName) const
{
    if (m_info.isEmpty())
        return false;

    const QString lfeatureName = featureName.toLower();
    const QJsonArray features = m_info.value(QStringLiteral("enabledAppFeatures")).toArray();
    const auto featurePredicate = [lfeatureName](const QJsonValue &item) -> bool {
        const QString istring = item.toString().toLower();
        return (istring == lfeatureName);
    };
    const auto wildCardPredicate = [](const QJsonValue &item) -> bool {
        return item.toString() == QStringLiteral("*");
    };
    const auto notFeaturePredicate = [lfeatureName](const QJsonValue &item) -> bool {
        const QString istring = item.toString().toLower();
        return istring.startsWith(QChar('!')) && (istring.mid(1) == lfeatureName);
    };

    const bool featureEnabled =
            std::find_if(features.constBegin(), features.constEnd(), featurePredicate)
            != features.constEnd();
    const bool allFeaturesEnabled =
            std::find_if(features.constBegin(), features.constEnd(), wildCardPredicate)
            != features.constEnd();
    const bool featureDisabled =
            std::find_if(features.constBegin(), features.constEnd(), notFeaturePredicate)
            != features.constEnd();
    return (allFeaturesEnabled || featureEnabled) && !featureDisabled;
}

void User::refresh()
{
    emit infoChanged();
    emit installationsChanged();
}

void User::setInfo(const QJsonObject &val)
{
    m_info = val;
    m_enabledFeatures.clear();
    m_analyticsConsent = false;

    if (!m_info.isEmpty()) {
        // Must be in sync with Scrite.AppFeatures enumeration
        static const QStringList availableFeatures = {
            QStringLiteral("screenplay"), QStringLiteral("structure"),
            QStringLiteral("notebook"),   QStringLiteral("relationshipgraph"),
            QStringLiteral("scriptalay"), QStringLiteral("template"),
            QStringLiteral("report"),     QStringLiteral("import"),
            QStringLiteral("export"),     QStringLiteral("scrited"),
            QStringLiteral("watermark")
        };
        const QJsonArray features = m_info.value(QStringLiteral("enabledAppFeatures")).toArray();
        QSet<int> ifeatures;
        for (const QJsonValue &featureItem : features) {
            const QString feature = featureItem.toString().toLower();
            if (feature.isEmpty())
                continue;

            if (feature == QStringLiteral("*")) {
                for (int i = Scrite::MinFeature; i <= Scrite::MaxFeature; i++)
                    ifeatures += i;
            } else {
                const bool invert = feature.startsWith(QChar('!'));
                const int index = availableFeatures.indexOf(invert ? feature.mid(1) : feature);
                if (index >= 0) {
                    if (invert)
                        ifeatures -= index;
                    else
                        ifeatures += index;
                }
            }
        }

        m_enabledFeatures = ifeatures.values();
        std::sort(m_enabledFeatures.begin(), m_enabledFeatures.end());

        const QJsonObject consentObj = m_info.value(QStringLiteral("consent")).toObject();
        m_analyticsConsent = consentObj.value(QStringLiteral("activity")).toBool(false);

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
        qDebug() << "PA: " << m_enabledFeatures << m_info;
#endif

        QTimer::singleShot(2000, this, &User::updateUserCountryAndCurrency);
    }

    emit infoChanged();
}

void User::setInstallations(const QJsonArray &val)
{
    if (m_installations == val)
        return;

    m_installations = val;
    m_currentInstallationIndex = -1;

    auto sortVersionsArray = [](QJsonObject &object, const QString &key) {
        QJsonArray array = object.value(key).toArray();
        if (array.isEmpty())
            return;

        QVariantList varList = array.toVariantList();
        std::sort(varList.begin(), varList.end(), [](const QVariant &va, const QVariant &vb) {
            const QString vas = va.toString();
            const QString vbs = vb.toString();
            return QVersionNumber::fromString(vas) > QVersionNumber::fromString(vbs);
        });

        array = QJsonArray::fromVariantList(varList);
        object.insert(key, array);
    };

    // Sort version fields appVersions and pastVersions
    for (QJsonValueRef item : m_installations) {
        QJsonObject installation = item.toObject();
        sortVersionsArray(installation, QStringLiteral("appVersions"));
        sortVersionsArray(installation, QStringLiteral("pastVersions"));
        item = installation;
    }

    int index = -1;
    bool activationTimeout = false;
    for (const QJsonValue &item : qAsConst(m_installations)) {
        ++index;
        const QJsonObject installation = item.toObject();
        if (installation.value(QStringLiteral("deviceId")).toString()
            == JsonHttpRequest::deviceId()) {
            const QString dtFormat = QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz");
            const QString lastActivatedDateString =
                    installation.value(QStringLiteral("lastActivationDate")).toString();
            if (!lastActivatedDateString.isEmpty()) {
                const QDateTime lastActivateDate = QDateTime::fromString(
                        lastActivatedDateString.left(lastActivatedDateString.length() - 1),
                        dtFormat);
                const int nrDaysFromLastActivation =
                        lastActivateDate.daysTo(QDateTime::currentDateTime());
                if (nrDaysFromLastActivation > 28) {
                    activationTimeout = true;
                    break;
                }
            }

            m_currentInstallationIndex = index;
            break;
        }
    }

    if (m_currentInstallationIndex < 0 && !m_installations.isEmpty()) {
        if (m_loadingStoredUserInformation)
            this->reset();
        else {
            const QString errContext = m_currentInstallationIndex < 0
                    ? (activationTimeout ? QStringLiteral("E_ACT_TIMEOUT")
                                         : QStringLiteral("E_INSTALL"))
                    : QStringLiteral("E_INSTALLS");
            m_errorReport->setErrorMessage(GetSessionExpiredErrorMessage(errContext));
            m_installations = QJsonArray();
            this->logout();
        }
    }

    emit installationsChanged();
}

void User::setSubscriptions(const QJsonArray &val)
{
    if (m_subscriptions == val)
        return;

    m_subscriptions = val;
    emit subscriptionsChanged();
}

void User::setHelpTips(const QJsonObject &val)
{
    if (m_helpTips == val)
        return;

    m_helpTips = val;
    emit helpTipsChanged();

    const QByteArray json = QJsonDocument(val).toJson();
    JsonHttpRequest::store(QStringLiteral("helpTips"), json.toBase64());
}

void User::loadStoredHelpTips()
{
    const QByteArray base64 = JsonHttpRequest::fetch(QStringLiteral("helpTips")).toByteArray();
    const QByteArray json = QByteArray::fromBase64(base64);

    m_helpTips = QJsonDocument::fromJson(json).object();
    emit helpTipsChanged();
}

void User::firstReload(bool loadStoredUserInfoAlso)
{
    if (loadStoredUserInfoAlso)
        this->loadStoredInformation();
    this->fetchHelpTips();
    this->reload();
}

void User::fetchHelpTips()
{
    JsonHttpRequest *call = new JsonHttpRequest(this);
    call->setAutoDelete(true);

    call->setApi(QStringLiteral("user/helpTips"));
    call->setType(JsonHttpRequest::GET);
    connect(call, &JsonHttpRequest::finished, this, [=]() {
        if (call->hasError() || !call->hasResponse()) {
            this->loadStoredHelpTips();
            return; // Use stored credentials
        }
        this->setHelpTips(call->responseData());
    });
    call->call();
}

void User::reset()
{
    if (!m_loadingStoredUserInformation) {
        ScriteDocument *document = ScriteDocument::instance();
        if (document)
            document->reset();
    }

    this->setSubscriptions(QJsonArray());
    this->setInstallations(QJsonArray());
    this->setInfo(QJsonObject());
    JsonHttpRequest::store(QStringLiteral("devices"), QVariant());
    JsonHttpRequest::store(QStringLiteral("subscriptions"), QVariant());
    JsonHttpRequest::store(QStringLiteral("userInfo"), QVariant());
    JsonHttpRequest::store(QStringLiteral("loginToken"), QVariant());
    JsonHttpRequest::store(QStringLiteral("sessionToken"), QVariant());
}

void User::activateCallDone()
{
    if (m_call) {
        if (m_call->hasError()) {
            m_errorReport->setErrorMessage(m_call->errorText(), m_call->error());
            this->reset();
            emit forceLoginRequest();
            return;
        }

        if (!m_call->hasResponse())
            return; // Use stored credentials

        const QJsonObject tokens = m_call->responseData();
        const QString sessionTokenKey = QStringLiteral("sessionToken");
        const QString sessionToken = tokens.value(sessionTokenKey).toString();
        m_call->store(sessionTokenKey, sessionToken);
    }

    // Get user information
    m_call = this->newCall();
    connect(m_call, &JsonHttpRequest::finished, this, &User::userInfoCallDone);
    m_call->setApi(QStringLiteral("user/me"));
    m_call->setType(JsonHttpRequest::GET);
    m_call->call();
}

void User::userInfoCallDone()
{
    if (m_call) {
        if (m_call->hasError()) {
            m_errorReport->setErrorMessage(m_call->errorText(), m_call->error());
            this->reset();
            emit forceLoginRequest();
            return;
        }

        if (!m_call->hasResponse())
            return; // Use stored credentials

        QJsonObject userInfo = m_call->responseData();

        const QString ikey = QStringLiteral("installations");
        const QJsonValue installationsValue = userInfo.value(ikey);
        userInfo.remove(ikey);

        const QString skey = QStringLiteral("subscriptions");
        const QJsonValue subscriptionsValue = userInfo.value(skey);
        userInfo.remove(skey);

        this->setInfo(userInfo);
        this->storeUserInfo();

        if (subscriptionsValue.isArray()) {
            const QJsonArray subscriptions = subscriptionsValue.toArray();
            this->setSubscriptions(subscriptions);
            this->storeSubscriptions();
        }

        if (installationsValue.isArray()) {
            const QJsonArray installations = installationsValue.toArray();
            this->setInstallations(installations);
            this->storeInstallations();
            return;
        }
    }
}

void User::installationsCallDone()
{
    if (m_call) {
        if (m_call->hasError()) {
            m_errorReport->setErrorMessage(m_call->errorText(), m_call->error());
            this->reset();
            emit forceLoginRequest();
            return;
        }

        if (!m_call->hasResponse())
            return; // Use stored credentials

        const QJsonObject installationsInfo = m_call->responseData();
        const QJsonArray installations = installationsInfo.value(QStringLiteral("list")).toArray();
        this->setInstallations(installations);
        this->storeInstallations();
    }
}

void User::subscriptionsCallDone()
{
    if (m_call) {
        if (m_call->hasError()) {
            m_errorReport->setErrorMessage(m_call->errorText(), m_call->error());
            this->reset();
            emit forceLoginRequest();
            return;
        }

        if (!m_call->hasResponse())
            return; // Use stored credentials

        const QJsonObject subscriptionsInfo = m_call->responseData();
        const QJsonArray subscriptions = subscriptionsInfo.value(QStringLiteral("list")).toArray();
        this->setSubscriptions(subscriptions);
        this->storeSubscriptions();
    }
}

void User::loadStoredInformation()
{
    QScopedValueRollback<bool> rollback(m_loadingStoredUserInformation, true);

    QJsonParseError parseError;

    auto parse = [&parseError](const QVariant &variant) -> QJsonValue {
        parseError.error = QJsonParseError::NoError;
        parseError.offset = -1;

        if (variant.isValid() && !variant.isNull() && variant.canConvert(QMetaType::QString)) {
            QJsonParseError parseError;
            const QString cryptText = variant.toString();
            const QString crypt = JsonHttpRequest::decrypt(cryptText);
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(crypt.toUtf8(), &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isArray())
                    return jsonDoc.array();
                if (jsonDoc.isObject())
                    return jsonDoc.object();
            }
        }

        return QJsonValue(QJsonValue::Undefined);
    };

    // Load information stored in the previous session
    const QJsonObject userInfo =
            parse(JsonHttpRequest::fetch(QStringLiteral("userInfo"))).toObject();
    if (userInfo.isEmpty()) {
        this->reset();
        return;
    } else
        this->setInfo(userInfo);

    const QJsonArray devices = parse(JsonHttpRequest::fetch(QStringLiteral("devices"))).toArray();
    if (devices.isEmpty()) {
        this->reset();
        return;
    } else
        this->setInstallations(devices);

    const QJsonArray subscriptions =
            parse(JsonHttpRequest::fetch(QStringLiteral("subscriptions"))).toArray();
    const QJsonObject activeSub = findActiveSubscription(subscriptions);
    if (activeSub.isEmpty()) {
        this->reset();
        m_errorReport->setErrorMessage(
                GetSessionExpiredErrorMessage(QStringLiteral("E_ACTIVE_SUBSCRIPTION")));
        return;
    } else
        this->setSubscriptions(subscriptions);
}

JsonHttpRequest *User::newCall()
{
    if (m_call) {
        disconnect(m_call, &JsonHttpRequest::destroyed, this, &User::onCallDestroyed);
        m_call->deleteLater();
        m_call = nullptr;
    }

    m_errorReport->clear();

    m_call = new JsonHttpRequest(this);
    m_call->setAutoDelete(true);
    connect(m_call, &JsonHttpRequest::destroyed, this, &User::onCallDestroyed);
    connect(m_call, &JsonHttpRequest::justIssuedCall, m_errorReport, &ErrorReport::clear);
    emit busyChanged();
    return m_call;
}

void User::onCallDestroyed()
{
    m_call = nullptr;
    emit busyChanged();

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
    qDebug() << "PA: ";
#endif
}

void User::onLogActivityCallFinished()
{
    JsonHttpRequest *call = qobject_cast<JsonHttpRequest *>(this->sender());
    if (call == nullptr)
        return;

    if (call->hasError()) {
        const QStringList errorCodes({ QStringLiteral("E_NO_ACTIVATION"),
                                       QStringLiteral("E_NO_SESSION"),
                                       QStringLiteral("E_NO_USER") });
        if (errorCodes.contains(call->errorCode())) {
            m_errorReport->setErrorMessage(
                    ::GetSessionExpiredErrorMessage(QStringLiteral("E_LOG")));
            this->logout();
            return;
        }

        // Other error codes are fine, no issues
    }
}

void User::onDeactivateInstallationFinished()
{
    JsonHttpRequest *call = qobject_cast<JsonHttpRequest *>(this->sender());
    if (call == nullptr)
        return;

    if (call->hasError() || !call->hasResponse()) {
        m_errorReport->setErrorMessage(QStringLiteral("Could not deactivate installation."),
                                       call->error());
        return;
    }

    const QJsonObject response = call->responseData();
    const QJsonArray installations = response.value(QStringLiteral("list")).toArray();
    this->setInstallations(installations);
    this->storeInstallations();
}

void User::storeUserInfo()
{
    const QString text = QJsonDocument(m_info).toJson(QJsonDocument::Compact);
    const QString cryptText = JsonHttpRequest::encrypt(text);
    JsonHttpRequest::store(QStringLiteral("userInfo"), cryptText);
}

void User::storeInstallations()
{
    const QString text = QJsonDocument(m_installations).toJson(QJsonDocument::Compact);
    const QString cryptText = JsonHttpRequest::encrypt(text);
    JsonHttpRequest::store(QStringLiteral("devices"), cryptText);
}

void User::storeSubscriptions()
{
    const QString text = QJsonDocument(m_subscriptions).toJson(QJsonDocument::Compact);
    const QString cryptText = JsonHttpRequest::encrypt(text);
    JsonHttpRequest::store(QStringLiteral("subscriptions"), cryptText);
}

void User::updateUserCountryAndCurrency()
{
    const QString countryAttrib = QStringLiteral("country");
    const QString currencyAttrib = QStringLiteral("currency");
    if (!m_info.contains(countryAttrib) || m_info.value(countryAttrib).toString().isEmpty()) {
        const QString country = QLocale::countryToString(QLocale::system().country());
        m_info.insert(countryAttrib, country);

        const QString currency = QLocale::system().currencySymbol(QLocale::CurrencyIsoCode);
        m_info.insert(currencyAttrib, currency);

        QJsonObject updatedInfo;
        updatedInfo.insert(countryAttrib, country);
        updatedInfo.insert(currencyAttrib, currency);

        this->update(updatedInfo);
    }
}

void User::reload()
{
    if (m_call != nullptr)
        return;

    // User should have logged in once.
    if (JsonHttpRequest::loginToken().isEmpty() || JsonHttpRequest::email().isEmpty()) {
        this->setInfo(QJsonObject());
        this->setInstallations(QJsonArray());
        emit forceLoginRequest();
        return;
    }

    if (JsonHttpRequest::sessionToken().isEmpty()) {
        const bool resetSessionToken = PeerAppLookup::instance()->peerCount() == 0;

        // Activate device to get session token
        m_call = this->newCall();
        connect(m_call, &JsonHttpRequest::finished, this, &User::activateCallDone);
        m_call->setApi(QStringLiteral("app/activate"));
        m_call->setData(QJsonObject(
                { { QStringLiteral("email"), JsonHttpRequest::email() },
                  { QStringLiteral("clientId"), JsonHttpRequest::clientId() },
                  { QStringLiteral("deviceId"), JsonHttpRequest::deviceId() },
                  { QStringLiteral("appVersion"), JsonHttpRequest::appVersion() },
                  { QStringLiteral("loginToken"), JsonHttpRequest::loginToken() },
                  { QStringLiteral("platform"), JsonHttpRequest::platform() },
                  { QStringLiteral("platformType"), JsonHttpRequest::platformType() },
                  { QStringLiteral("platformVersion"), JsonHttpRequest::platformVersion() },
                  { QStringLiteral("resetSessionToken"), QJsonValue(resetSessionToken) } }));
        m_call->call();
    } else {
        // Since we have session token, we can reload user information and
        // installation info.
        this->activateCallDone();
    }
}

void User::logout()
{
    ScriteDocument *document = ScriteDocument::instance();
    if (document && document->isModified() && !document->isEmpty()) {
        m_errorReport->setErrorMessage(QStringLiteral(
                "Current document is not saved. Please save the document before logging out."));
        return;
    }

    JsonHttpRequest *call = new JsonHttpRequest(this);
    call->setAutoDelete(true);
    call->setType(JsonHttpRequest::POST);
    call->setApi(QStringLiteral("app/deactivate"));
    call->call(); // Fire and forget.

    this->reset();

    emit forceLoginRequest();
}

void User::update(const QJsonObject &newInfo)
{
    if (JsonHttpRequest::sessionToken().isEmpty())
        return;

    JsonHttpRequest *call = this->newCall();
    call->setAutoDelete(true);
    call->setType(JsonHttpRequest::POST);
    call->setApi(QStringLiteral("user/me"));
    call->setData(newInfo);
    connect(call, &JsonHttpRequest::finished, this, [=]() {
        if (call->hasError())
            m_errorReport->setErrorMessage(call->errorText(), call->error());
        else if (call->hasResponse())
            this->setInfo(call->responseData());
        else {
            const QString errMsg = QStringLiteral("Couldn't update user information.");
            m_errorReport->setErrorMessage(
                    errMsg,
                    QJsonObject({ { QStringLiteral("code"), QStringLiteral("E_USERINFO") },
                                  { QStringLiteral("text"), errMsg } }));
        }
    });
    call->call();
}

void User::deactivateInstallation(const QString &id)
{
    if (id.isEmpty() || !this->isLoggedIn())
        return;

    JsonHttpRequest *call = this->newCall();
    call->setType(JsonHttpRequest::POST);
    call->setApi(QStringLiteral("user/deactivateInstallation"));
    call->setData(QJsonObject({ { QStringLiteral("installationId"), id } }));
    connect(call, &JsonHttpRequest::finished, this, &User::onDeactivateInstallationFinished);
    call->call();
}

void User::refreshInstallations()
{
    if (m_call) {
        connect(m_call, &JsonHttpRequest::finished, this, &User::refreshInstallations,
                Qt::UniqueConnection);
        return;
    }

    m_call = this->newCall();
    m_call->setAutoDelete(true);
    m_call->setType(JsonHttpRequest::GET);
    m_call->setApi(QStringLiteral("user/installations"));
    connect(m_call, &JsonHttpRequest::finished, this, &User::installationsCallDone);
    m_call->call();
}

void User::refreshSubscriptions()
{
    if (m_call) {
        connect(m_call, &JsonHttpRequest::finished, this, &User::refreshSubscriptions,
                Qt::UniqueConnection);
        return;
    }

    m_call = this->newCall();
    m_call->setAutoDelete(true);
    m_call->setType(JsonHttpRequest::GET);
    m_call->setApi(QStringLiteral("user/subscriptions"));
    connect(m_call, &JsonHttpRequest::finished, this, &User::subscriptionsCallDone);
    m_call->call();
}

void User::logActivity2(const QString &givenActivity, const QJsonValue &data)
{
    if (JsonHttpRequest::sessionToken().isEmpty() || !m_analyticsConsent)
        return;

    const QString activity = givenActivity.isEmpty() ? QStringLiteral("touch")
                                                     : givenActivity.toLower().simplified();

    // !!!NOT CALLING newCall() on PURPOSE!!!!
    // While logging activity, we do not need User.busy to become true
    JsonHttpRequest *call = new JsonHttpRequest(this);
    call->setAutoDelete(true);
    call->setType(JsonHttpRequest::POST);
    call->setApi(QStringLiteral("activity/log"));
    const QJsonObject callData = {
        { QStringLiteral("appVersion"), Application::instance()->applicationVersion() },
        { QStringLiteral("activity"), activity },
        { QStringLiteral("data"), data },
    };
    call->setData(callData);
    connect(call, &JsonHttpRequest::finished, this, &User::onLogActivityCallFinished);
    call->call(); // Fire and Forget

    // Trigger a touch log after 10 minutes
    m_touchLogTimer.stop();
    m_touchLogTimer.start();
}

///////////////////////////////////////////////////////////////////////////////

UserIconProvider::UserIconProvider() : QQuickImageProvider(QQmlImageProviderBase::Image) { }

UserIconProvider::~UserIconProvider() { }

QImage UserIconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);

    const int dim = qMax(100, qMin(requestedSize.width(), requestedSize.height()));

    QImage image(QSize(dim, dim), QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter paint(&image);

    QColor appPurple("#65318f");
    paint.setPen(QPen(appPurple, 2.0));

    appPurple.setAlphaF(0.9);
    paint.setBrush(appPurple);

    paint.setRenderHint(QPainter::Antialiasing);
    paint.drawEllipse(image.rect().adjusted(2, 2, -2, -2));

    if (User::instance()->isLoggedIn()) {
        const QString email = JsonHttpRequest::email();
        const QString firstName =
                User::instance()->info().value(QStringLiteral("firstName")).toString();
        const QString lastName =
                User::instance()->info().value(QStringLiteral("lastName")).toString();

        QString initials;

        if (firstName.isEmpty() && lastName.isEmpty()) {
            if (!email.isEmpty())
                initials = email.at(0).toUpper();
        } else {
            initials = !firstName.isEmpty() ? firstName.at(0).toUpper() : QString();
            initials += !lastName.isEmpty() ? lastName.at(0).toUpper() : QString();
        }

        if (!initials.isEmpty()) {
            const QFontMetricsF fm = paint.fontMetrics();
            QRectF brect = fm.boundingRect(initials);
            brect.moveTopLeft(QPointF(0, 0));
            qreal scale = qreal(dim) * 0.65 / qMax(brect.width(), brect.height());

            paint.save();
            paint.translate(image.rect().center());
            paint.scale(scale, scale);
            paint.translate(-brect.center() + QPointF(0.5, 0.5));
            paint.setPen(Qt::white);
            paint.drawText(0, 0, brect.width(), brect.height(), Qt::AlignCenter | Qt::TextDontClip,
                           initials);
            paint.restore();
        }
    } else {
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
    connect(User::instance(), &User::loggedInChanged, this, &AppFeature::reevaluate);
}

AppFeature::~AppFeature() { }

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
    if (User::instance()->isLoggedIn()) {
        const bool flag1 = m_feature < 0
                ? true
                : User::instance()->isFeatureEnabled(Scrite::AppFeature(m_feature));
        const bool flag2 = m_featureName.isEmpty()
                ? true
                : User::instance()->isFeatureNameEnabled(m_featureName);
        this->setEnabled(flag1 && flag2);
    } else
        this->setEnabled(false);

#ifndef QT_NO_DEBUG_OUTPUT_OUTPUT
    qDebug() << "PA: " << m_featureName << "/" << m_feature << " = " << m_enabled;
#endif
}

void AppFeature::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}
