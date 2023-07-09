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

#include <QTimer>
#include <QObject>
#include <QPointer>
#include <QJsonArray>
#include <QJsonObject>
#include <QQuickImageProvider>

#include "errorreport.h"
#include "jsonhttprequest.h"

class User : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static User *instance();
    ~User();

    Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY loggedInChanged)
    bool isLoggedIn() const;
    Q_SIGNAL void loggedInChanged();

    Q_PROPERTY(QString email READ email NOTIFY infoChanged)
    QString email() const;

    Q_PROPERTY(QString firstName READ firstName NOTIFY infoChanged)
    QString firstName() const;

    Q_PROPERTY(QString lastName READ lastName NOTIFY infoChanged)
    QString lastName() const;

    Q_PROPERTY(QString fullName READ fullName NOTIFY infoChanged)
    QString fullName() const;

    Q_PROPERTY(QString location READ location NOTIFY infoChanged)
    QString location() const;

    Q_PROPERTY(QString experience READ experience NOTIFY infoChanged)
    QString experience() const;

    // Where did you hear about Scrite?
    Q_PROPERTY(QString wdyhas READ wdyhas NOTIFY infoChanged)
    QString wdyhas() const;

    Q_PROPERTY(QJsonObject info READ info NOTIFY infoChanged)
    QJsonObject info() const { return m_info; }
    Q_SIGNAL void infoChanged();

    Q_PROPERTY(QJsonArray installations READ installations NOTIFY installationsChanged)
    QJsonArray installations() const { return m_installations; }
    Q_SIGNAL void installationsChanged();

    Q_PROPERTY(QJsonObject helpTips READ helpTips NOTIFY helpTipsChanged)
    QJsonObject helpTips() const { return m_helpTips; }
    Q_SIGNAL void helpTipsChanged();

    Q_PROPERTY(QStringList locations READ locations CONSTANT)
    Q_INVOKABLE static QStringList locations();

    Q_PROPERTY(int currentInstallationIndex READ currentInstallationIndex NOTIFY installationsChanged)
    int currentInstallationIndex() const { return m_currentInstallationIndex; }

    QList<int> enabledFeatures() const { return m_enabledFeatures; }
    // here feature is anything from Scrite::AppFeature enum
    bool isFeatureEnabled(int feature) const { return m_enabledFeatures.contains(feature); }
    bool isFeatureNameEnabled(const QString &featureName) const;

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    bool busy() const { return m_call != nullptr; }
    Q_SIGNAL void busyChanged();

    Q_SLOT void refresh();
    Q_SLOT void reload();
    Q_SLOT void logout();
    Q_SLOT void update(const QJsonObject &newInfo);
    Q_SLOT void deactivateInstallation(const QString &id);
    Q_SLOT void refreshInstallations();

    Q_SLOT void logActivity1(const QString &activity)
    {
        this->logActivity2(activity, QJsonValue());
    }
    Q_SLOT void logActivity2(const QString &activity, const QJsonValue &data);

signals:
    void forceLoginRequest();

private:
    User(QObject *parent = nullptr);
    void setInfo(const QJsonObject &val);
    void setInstallations(const QJsonArray &val);
    void setHelpTips(const QJsonObject &val);
    void loadStoredHelpTips();

    Q_SLOT void firstReload(bool loadStoredUserInfoAlso = true);

    void fetchHelpTips();
    void reset();
    void activateCallDone();
    void userInfoCallDone();
    void installationsCallDone();

    void loadStoredUserInformation();

    JsonHttpRequest *newCall();
    void onCallDestroyed();
    void onLogActivityCallFinished();
    void onDeactivateInstallationFinished();

    void storeUserInfo();
    void storeInstallations();

private:
    bool m_busy = false;
    QJsonObject m_info;
    QJsonObject m_helpTips;
    QTimer m_touchLogTimer;
    QList<int> m_enabledFeatures;
    QJsonArray m_installations;
    bool m_analyticsConsent = false;
    int m_currentInstallationIndex = -1;
    JsonHttpRequest *m_call = nullptr;
    ErrorReport *m_errorReport = new ErrorReport(this);
};

class UserIconProvider : public QQuickImageProvider
{
public:
    explicit UserIconProvider();
    ~UserIconProvider();

    // QQuickImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

class AppFeature : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AppFeature(QObject *parent = nullptr);
    ~AppFeature();

    Q_PROPERTY(QString featureName READ featureName WRITE setFeatureName NOTIFY featureNameChanged)
    void setFeatureName(const QString &val);
    QString featureName() const { return m_featureName; }
    Q_SIGNAL void featureNameChanged();

    Q_PROPERTY(int feature READ feature WRITE setFeature NOTIFY featureChanged)
    void setFeature(int val);
    int feature() const { return m_feature; }
    Q_SIGNAL void featureChanged();

    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
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
