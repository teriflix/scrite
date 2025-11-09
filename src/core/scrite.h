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

#ifndef SCRITE_NAMESPACE_H
#define SCRITE_NAMESPACE_H

#include "user.h"
#include "appwindow.h"
#include "restapicall.h"
#include "application.h"
#include "scritedocument.h"
#include "notificationmanager.h"
#include "scritedocumentvault.h"

#include <QFileInfo>
#include <QQmlEngine>
#include <QTemporaryDir>

#define RUPEE_SYMBOL "₹"

struct Country
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_PROPERTY(QString code MEMBER code)
    QString code;

    Q_PROPERTY(QString name MEMBER name)
    QString name;

    Country() { }
    Country(const Country &other)
    {
        code = other.code;
        name = other.name;
    }
    bool operator==(const Country &other) const { return name == other.name && code == other.code; }
    bool operator!=(const Country &other) const { return name != other.name && code != other.code; }
    Country &operator=(const Country &other)
    {
        name = other.name;
        code = other.code;
        return *this;
    }
};
Q_DECLARE_METATYPE(Country)

struct Currency
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_PROPERTY(QString code MEMBER code)
    QString code;

    Q_PROPERTY(QString symbol MEMBER symbol)
    QString symbol;

    Currency() { }
    Currency(const Currency &other)
    {
        code = other.code;
        symbol = other.symbol;
    }
    bool operator==(const Currency &other) const
    {
        return code == other.code && symbol == other.symbol;
    }
    bool operator!=(const Currency &other) const
    {
        return code != other.code || symbol != other.symbol;
    }
    Currency &operator=(const Currency &other)
    {
        code = other.code;
        symbol = other.symbol;
        return *this;
    }
};
Q_DECLARE_METATYPE(Currency)

struct Locale
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_PROPERTY(Country country MEMBER country)
    Country country;

    Q_PROPERTY(Currency currency MEMBER currency)
    Currency currency;

    Locale() { }
    Locale(const Locale &other)
    {
        country = other.country;
        currency = other.currency;
    }
    bool operator==(const Locale &other) const
    {
        return country == other.country && currency == other.currency;
    }
    bool operator!=(const Locale &other) const
    {
        return country != other.country || currency != other.currency;
    }
    Locale &operator=(const Locale &other)
    {
        country = other.country;
        currency = other.currency;
        return *this;
    }
};
Q_DECLARE_METATYPE(Locale)

class Scrite : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Scrite(QObject *parent = nullptr);
    ~Scrite();

    enum AppFeature {
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
        WatermarkFeature,
        RecentFilesFeature,
        VaultFilesFeature,
        MinFeature = ScreenplayFeature,
        MaxFeature = VaultFilesFeature
    };
    Q_ENUM(AppFeature)

    enum ApplicationState {
        ApplicationSuspended = Qt::ApplicationSuspended,
        ApplicationHidden = Qt::ApplicationHidden,
        ApplicationInactive = Qt::ApplicationInactive,
        ApplicationActive = Qt::ApplicationActive
    };
    Q_ENUM(ApplicationState)

    Q_PROPERTY(Application *app READ app CONSTANT)
    static Application *app();

    Q_PROPERTY(AppWindow *window READ window CONSTANT)
    static AppWindow *window();

    Q_PROPERTY(User *user READ user CONSTANT)
    static User *user();

    Q_PROPERTY(RestApi *restApi READ restApi CONSTANT)
    static RestApi *restApi();

    Q_PROPERTY(ScriteDocument *document READ document CONSTANT)
    static ScriteDocument *document();

    Q_PROPERTY(ScriteDocumentVault *vault READ vault CONSTANT)
    static ScriteDocumentVault *vault();

    Q_PROPERTY(NotificationManager *notifications READ notifications CONSTANT)
    static NotificationManager *notifications();

    Q_PROPERTY(QString fileNameToOpen READ fileNameToOpen CONSTANT)
    static QString fileNameToOpen() { return m_fileNameToOpen; }
    static void setFileNameToOpen(const QString &val);

    Q_PROPERTY(QStringList defaultTransitions READ defaultTransitions CONSTANT)
    static QStringList defaultTransitions();

    Q_PROPERTY(QStringList defaultShots READ defaultShots CONSTANT)
    static QStringList defaultShots();

    Q_PROPERTY(Locale locale READ locale CONSTANT)
    static Locale locale();

    Q_INVOKABLE static QString currencySymbol(const QString &code);

    Q_INVOKABLE static bool isFeatureEnabled(Scrite::AppFeature feature,
                                             const QStringList &features);
    Q_INVOKABLE static bool isFeatureNameEnabled(const QString &feature,
                                                 const QStringList &features);

    static bool doZip(const QFileInfo &zipFileInfo, const QDir &sourceDir,
                      const QList<QPair<QString, int>> &files);
    static bool doZip(const QFileInfo &zipFileInfo, const QDir &rootDir);
    static bool doUnzip(const QFileInfo &zipFileInfo, const QTemporaryDir &dstDir);

    Q_INVOKABLE static bool isNetworkAvailable();
    static bool blockingMinimumVersionCheck();

private:
    static QString m_fileNameToOpen;
};

#endif // SCRITE_NAMESPACE_H
