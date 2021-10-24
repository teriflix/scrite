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

#include "user.h"
#include "application.h"
#include "jsonhttprequest.h"

#include <QtDebug>
#include <QPainter>
#include <QDateTime>
#include <QJsonDocument>
#include <QCoreApplication>

static QString GetSessionExpiredErrorMessage()
{
    return QStringLiteral("Your login session has expired and therefore your device is deactivated. Please connect to the Internet and login/reactivate your installation of Scrite.");
}

User *User::instance()
{
    static User *theUser = new User(qApp);
    return theUser;
}

User::User(QObject *parent)
    :QObject(parent)
{
    const QStringList appArgs = qApp->arguments();
    const QString starg = QStringLiteral("--sessionToken");
    const int stargPos = appArgs.indexOf(starg);
    if(stargPos >= 0 && appArgs.size() >= stargPos+2)
    {
        const QString stok = appArgs.at(stargPos+1);
        JsonHttpRequest::store( QStringLiteral("sessionToken"), stok );
    }

    QMetaObject::invokeMethod(this, "reload", Qt::QueuedConnection);

    connect(this, &User::infoChanged, this, &User::loggedInChanged);
    connect(this, &User::installationsChanged, this, &User::loggedInChanged);
}

User::~User()
{

}

bool User::isLoggedIn() const
{
    return !m_info.isEmpty() && !m_installations.isEmpty();
}

bool User::isFeatureNameEnabled(const QString &featureName) const
{
    if(m_info.isEmpty())
        return false;

    const QJsonArray features = m_info.value( QStringLiteral("enabledAppFeatures") ).toArray();
    QJsonArray::const_iterator it = std::find_if(features.constBegin(), features.constEnd(), [featureName](const QJsonValue &item) {
        return (item.toString() == featureName);
    });
    return it != features.constEnd();
}

void User::setInfo(const QJsonObject &val)
{
    if(m_info == val)
        return;

    m_info = val;
    m_enabledFeatures.clear();

    if(!m_info.isEmpty())
    {
        static const QStringList availableFeatures = {
            QStringLiteral("screenplay"), QStringLiteral("structure"), QStringLiteral("notebook"),
            QStringLiteral("scriptalay"), QStringLiteral("template"), QStringLiteral("report"),
            QStringLiteral("import"), QStringLiteral("export"), QStringLiteral("scrited")
        };
        const QJsonArray features = m_info.value( QStringLiteral("enabledAppFeatures") ).toArray();
        QSet<int> ifeatures;
        for(const QJsonValue &featureItem : features)
        {
            const QString feature = featureItem.toString().toLower();
            const int index = availableFeatures.indexOf(feature);
            if(index >= 0)
                ifeatures += index;
        }

        m_enabledFeatures = ifeatures.toList();
        std::sort(m_enabledFeatures.begin(), m_enabledFeatures.end());

#ifndef QT_NODEBUG
        qDebug() << "PA: " << m_enabledFeatures << m_info;
#endif
    }

    emit infoChanged();
}

void User::setInstallations(const QJsonArray &val)
{
    if(m_installations == val)
        return;

    m_installations = val;
    m_currentInstallationIndex = -1;

    int index = -1;
    for(const QJsonValue &item : qAsConst(m_installations))
    {
        ++index;
        const QJsonObject installation = item.toObject();
        if(installation.value(QStringLiteral("deviceId")).toString() == JsonHttpRequest::deviceId())
        {
            const QString dtFormat = QStringLiteral("yyyy-MM-ddThh:mm:ss.zzz");
            const QString lastActivatedDateString = installation.value(QStringLiteral("lastActivationDate")).toString();
            if(!lastActivatedDateString.isEmpty())
            {
                const QDateTime lastActivateDate = QDateTime::fromString( lastActivatedDateString.left(lastActivatedDateString.length()-1), dtFormat );
                const int nrDaysFromLastActivation = lastActivateDate.daysTo(QDateTime::currentDateTime());
                if( nrDaysFromLastActivation > 28 )
                    break;
            }

            m_currentInstallationIndex = index;
            break;
        }
    }

    if(m_currentInstallationIndex < 0 && !m_installations.isEmpty())
    {
        m_errorReport->setErrorMessage(GetSessionExpiredErrorMessage());
        m_installations = QJsonArray();
        this->logout();
    }

    emit installationsChanged();
}

void User::activateCallDone()
{
    if(m_call)
    {
        if(m_call->hasError())
        {
            m_errorReport->setErrorMessage(m_call->errorText(), m_call->error());
            return;
        }

        if(!m_call->hasResponse())
        {
            // Load information stored in the previous session
            {
                QJsonParseError parseError;
                const QString cryptText = JsonHttpRequest::fetch( QStringLiteral("userInfo") ).toString();
                const QString crypt = JsonHttpRequest::decrypt(cryptText);
                const QJsonObject object = QJsonDocument::fromJson(crypt.toLatin1(), &parseError).object();
                if(parseError.error == QJsonParseError::NoError && !object.isEmpty())
                    this->setInfo(object);
                else
                {
                    m_errorReport->setErrorMessage(GetSessionExpiredErrorMessage());
                    this->logout();
                    return;
                }
            }

            {
                QJsonParseError parseError;
                const QString cryptText = JsonHttpRequest::fetch( QStringLiteral("devices") ).toString();
                const QString crypt = JsonHttpRequest::decrypt(cryptText);
                const QJsonArray array = QJsonDocument::fromJson(crypt.toLatin1(), &parseError).array();
                if(parseError.error == QJsonParseError::NoError && !array.isEmpty())
                    this->setInstallations(array);
                else
                {
                    m_errorReport->setErrorMessage(GetSessionExpiredErrorMessage());
                    this->logout();
                }
            }

            return;
        }

        const QJsonObject tokens = m_call->responseData();
        const QString sessionTokenKey = QStringLiteral("sessionToken");
        const QString sessionToken = tokens.value(sessionTokenKey).toString();
        m_call->store( sessionTokenKey, sessionToken );
    }

    // Get user information
    m_call = this->newCall();
    connect(m_call, &JsonHttpRequest::finished, this, &User::userInfoCallDone);
    m_call->setApi( QStringLiteral("user/me") );
    m_call->setType(JsonHttpRequest::GET);
    m_call->call();
}

void User::userInfoCallDone()
{
    if(m_call)
    {
        if(m_call->hasError())
        {
            m_errorReport->setErrorMessage(m_call->errorText(), m_call->error());
            return;
        }

        if(!m_call->hasResponse())
            return;

        const QJsonObject userInfo = m_call->responseData();
        this->setInfo(userInfo);

        const QString text = QJsonDocument(m_info).toJson();
        const QString cryptText = JsonHttpRequest::encrypt(text);
        JsonHttpRequest::store( QStringLiteral("userInfo"), cryptText );
    }

    // Fetch installations information
    m_call = this->newCall();
    connect(m_call, &JsonHttpRequest::finished, this, &User::installationsCallDone);
    m_call->setApi( QStringLiteral("user/installations") );
    m_call->setType(JsonHttpRequest::GET);
    m_call->call();
}

void User::installationsCallDone()
{
    if(m_call)
    {
        if(m_call->hasError())
        {
            m_errorReport->setErrorMessage(m_call->errorText(), m_call->error());
            return;
        }

        if(!m_call->hasResponse())
            return;

        const QJsonObject installationsInfo = m_call->responseData();
        const QJsonArray installations = installationsInfo.value(QStringLiteral("list")).toArray();
        this->setInstallations( installations );

        const QString text = QJsonDocument(installations).toJson();
        const QString cryptText = JsonHttpRequest::encrypt(text);
        JsonHttpRequest::store( QStringLiteral("devices"), cryptText );

        // All done.
    }
}

JsonHttpRequest *User::newCall()
{
    if(m_call)
    {
        disconnect(m_call, &JsonHttpRequest::destroyed, this, &User::onCallDestroyed);
        m_call->deleteLater();
        m_call = nullptr;
    }

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

#ifndef QT_NODEBUG
    qDebug() << "PA: ";
#endif
}

void User::reload()
{
    if(m_call != nullptr)
        return;

    // User should have logged in once.
    if(JsonHttpRequest::loginToken().isEmpty() || JsonHttpRequest::email().isEmpty())
    {
        this->setInfo(QJsonObject());
        this->setInstallations(QJsonArray());
        return;
    }

    if( JsonHttpRequest::sessionToken().isEmpty() )
    {
        // Activate device to get session token
        m_call = this->newCall();
        connect(m_call, &JsonHttpRequest::finished, this, &User::activateCallDone);
        m_call->setApi( QStringLiteral("app/activate") );
        m_call->setData( QJsonObject({
                { QStringLiteral("email"), JsonHttpRequest::email() },
                { QStringLiteral("clientId"), JsonHttpRequest::clientId() },
                { QStringLiteral("deviceId"), JsonHttpRequest::deviceId() },
                { QStringLiteral("appVersion"), JsonHttpRequest::appVersion() },
                { QStringLiteral("loginToken"), JsonHttpRequest::loginToken() }
            }));
        m_call->call();
    }
    else
    {
        // Since we have session token, we can reload user information and
        // installation info.
        this->activateCallDone();
    }
}

void User::logout()
{
    this->setInfo( QJsonObject() );
    this->setInstallations( QJsonArray() );
    JsonHttpRequest::store( QStringLiteral("sessionToken"), QVariant() );
}

void User::logActivity2(const QString &givenActivity, const QJsonValue &data)
{
    if(JsonHttpRequest::sessionToken().isEmpty())
        return;

    const QString activity = givenActivity.isEmpty() ? QStringLiteral("touch") : givenActivity.toLower().simplified();

    // !!!NOT CALLING newCall() on PURPOSE!!!!
    // While logging activity, we do not need User.busy to become true
    JsonHttpRequest *call = new JsonHttpRequest(this);
    call->setAutoDelete(true);
    call->setType(JsonHttpRequest::POST);
    call->setApi( QStringLiteral("activity/log") );
    const QJsonObject callData = {
        { QStringLiteral("appVersion"), Application::instance()->applicationVersion() },
        { QStringLiteral("activity"), activity },
        { QStringLiteral("data"), data },
    };
    call->setData(callData);
    call->call(); // Fire and Forget
}

///////////////////////////////////////////////////////////////////////////////

UserIconProvider::UserIconProvider() : QQuickImageProvider(QQmlImageProviderBase::Image) { }

UserIconProvider::~UserIconProvider() { }

QImage UserIconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);

    const int dim = qMax(100, qMin(requestedSize.width(),requestedSize.height()));

    QImage image(QSize(dim,dim), QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter paint(&image);

    QColor appPurple("#65318f");
    paint.setPen( QPen(appPurple,2.0) );

    appPurple.setAlphaF(0.9);
    paint.setBrush(appPurple);

    paint.setRenderHint(QPainter::Antialiasing);
    paint.drawEllipse(image.rect().adjusted(2, 2, -2, -2));

    if(User::instance()->isLoggedIn())
    {
        const QString email = JsonHttpRequest::email();
        const QString firstName = User::instance()->info().value(QStringLiteral("firstName")).toString();
        const QString lastName = User::instance()->info().value(QStringLiteral("lastName")).toString();

        QString initials;

        if(firstName.isEmpty() && lastName.isEmpty())
        {
            if(!email.isEmpty())
                initials = email.at(0).toUpper();
        }
        else
        {
            initials = !firstName.isEmpty() ? firstName.at(0).toUpper() : QString();
            initials += !lastName.isEmpty() ? lastName.at(0).toUpper() : QString();
        }

        if(!initials.isEmpty())
        {
            const QFontMetricsF fm = paint.fontMetrics();
            QRectF brect = fm.boundingRect(initials); brect.moveTopLeft(QPointF(0,0));
            qreal scale = qreal(dim)*0.75/qMax(brect.width(),brect.height());

            paint.save();
            paint.translate(image.rect().center());
            paint.scale(scale, scale);
            paint.translate(-brect.center() + QPointF(0.5,0.5));
            paint.setPen(Qt::white);
            paint.drawText(0, 0, brect.width(), brect.height(), Qt::AlignCenter|Qt::TextDontClip, initials);
            paint.restore();
        }
    }
    else
    {
        QRectF iconRect = image.rect();
        const qreal iconMargin = iconRect.width()*0.1;
        iconRect.adjust(iconMargin, iconMargin, -iconMargin, -iconMargin);
        paint.setRenderHint(QPainter::SmoothPixmapTransform);
        paint.drawImage(iconRect, QImage(":/icons/content/person_outline_inverted.png"));
    }

    paint.end();

    if(size)
        *size = image.size();

    return image;
}

///////////////////////////////////////////////////////////////////////////////

AppFeature::AppFeature(QObject *parent)
    :QObject(parent)
{
    connect(User::instance(), &User::infoChanged, this, &AppFeature::reevaluate);
    connect(User::instance(), &User::loggedInChanged, this, &AppFeature::reevaluate);
}

AppFeature::~AppFeature()
{

}

void AppFeature::setFeatureName(const QString &val)
{
    if(m_featureName == val)
        return;

    m_featureName = val;
    emit featureNameChanged();
    this->reevaluate();
}

void AppFeature::setFeature(int val)
{
    if(m_feature == val)
        return;

    m_feature = val;
    emit featureChanged();
    this->reevaluate();
}

void AppFeature::reevaluate()
{
    if(User::instance()->isLoggedIn())
    {
        if(m_featureName.isEmpty())
            this->setEnabled( User::instance()->isFeatureEnabled(User::AppFeature(m_feature)) );
        else
            this->setEnabled( User::instance()->isFeatureNameEnabled(m_featureName) );
    }
    else
        this->setEnabled(false);

#ifndef QT_NODEBUG
    qDebug() << "PA: " << m_featureName << "/" << m_feature << " = " << m_enabled;
#endif
}

void AppFeature::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}
