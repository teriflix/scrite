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

#ifndef USER_H
#define USER_H

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

public:
    static User *instance();
    ~User();

    Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY loggedInChanged)
    bool isLoggedIn() const;
    Q_SIGNAL void loggedInChanged();

    Q_PROPERTY(QJsonObject info READ info NOTIFY infoChanged)
    QJsonObject info() const { return m_info; }
    Q_SIGNAL void infoChanged();

    Q_PROPERTY(QJsonArray installations READ installations NOTIFY installationsChanged)
    QJsonArray installations() const { return m_installations; }
    Q_SIGNAL void installationsChanged();

    Q_PROPERTY(QStringList countryNames READ countryNames CONSTANT)
    static QStringList countryNames();

    Q_PROPERTY(QStringList cityNames READ cityNames CONSTANT)
    static QStringList cityNames();

    Q_INVOKABLE static QStringList countries(const QString &cityName);

    Q_PROPERTY(int currentInstallationIndex READ currentInstallationIndex NOTIFY installationsChanged)
    int currentInstallationIndex() const { return m_currentInstallationIndex; }

    enum AppFeature
    {
        ScreenplayFeature,
        StructureFeature,
        NotebookFeature,
        RelationshipGraphFeature,
        ScriptalayFeature,
        TemplateFeature,
        ReportFeature,
        ImportFeature,
        ExportFeature,
        ScritedFeature,
        MinFeature = ScreenplayFeature,
        MaxFeature = ScritedFeature
    };
    Q_ENUM(AppFeature)

    QList<int> enabledFeatures() const { return m_enabledFeatures; }
    bool isFeatureEnabled(AppFeature feature) const {
        return m_enabledFeatures.contains(feature);
    }
    bool isFeatureNameEnabled(const QString &featureName) const;

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    bool busy() const { return m_call != nullptr; }
    Q_SIGNAL void busyChanged();

    Q_SLOT void refresh();
    Q_SLOT void reload();
    Q_SLOT void logout();
    Q_SLOT void update(const QJsonObject &newInfo);

    Q_SLOT void logActivity1(const QString &activity) { this->logActivity2(activity, QJsonValue()); }
    Q_SLOT void logActivity2(const QString &activity, const QJsonValue &data);

signals:
    void forceLoginRequest();

private:
    User(QObject *parent=nullptr);
    void setInfo(const QJsonObject &val);
    void setInstallations(const QJsonArray &val);

    void activateCallDone();
    void userInfoCallDone();
    void installationsCallDone();

    JsonHttpRequest *newCall();
    void onCallDestroyed();

private:
    bool m_busy = false;
    QJsonObject m_info;
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
    UserIconProvider();
    ~UserIconProvider();

    // QQuickImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

class AppFeature : public QObject
{
    Q_OBJECT

public:
    AppFeature(QObject *parent=nullptr);
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
