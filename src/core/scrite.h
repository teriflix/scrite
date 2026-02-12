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
    // clang-format off
    Q_PROPERTY(QString code
               MEMBER code)
    // clang-format on
    QString code;

    // clang-format off
    Q_PROPERTY(QString name
               MEMBER name)
    // clang-format on
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
    // clang-format off
    Q_PROPERTY(QString code
               MEMBER code)
    // clang-format on
    QString code;

    // clang-format off
    Q_PROPERTY(QString symbol
               MEMBER symbol)
    // clang-format on
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
    // clang-format off
    Q_PROPERTY(Country country
               MEMBER country)
    // clang-format on
    Country country;

    // clang-format off
    Q_PROPERTY(Currency currency
               MEMBER currency)
    // clang-format on
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

    // clang-format off
    Q_PROPERTY(Application *app
               READ app
               CONSTANT )
    // clang-format on
    static Application *app();

    // clang-format off
    Q_PROPERTY(AppWindow *window
               READ window
               CONSTANT )
    // clang-format on
    static AppWindow *window();

    // clang-format off
    Q_PROPERTY(User *user
               READ user
               CONSTANT )
    // clang-format on
    static User *user();

    // clang-format off
    Q_PROPERTY(RestApi *restApi
               READ restApi
               CONSTANT )
    // clang-format on
    static RestApi *restApi();

    // clang-format off
    Q_PROPERTY(ScriteDocument *document
               READ document
               CONSTANT )
    // clang-format on
    static ScriteDocument *document();

    // clang-format off
    Q_PROPERTY(ScriteDocumentVault *vault
               READ vault
               CONSTANT )
    // clang-format on
    static ScriteDocumentVault *vault();

    // clang-format off
    Q_PROPERTY(NotificationManager *notifications
               READ notifications
               CONSTANT )
    // clang-format on
    static NotificationManager *notifications();

    // clang-format off
    Q_PROPERTY(QString fileNameToOpen
               READ fileNameToOpen
               CONSTANT )
    // clang-format on
    static QString fileNameToOpen() { return m_fileNameToOpen; }
    static void setFileNameToOpen(const QString &val);

    // clang-format off
    Q_PROPERTY(QStringList defaultTransitions
               READ defaultTransitions
               CONSTANT )
    // clang-format on
    static QStringList defaultTransitions();

    // clang-format off
    Q_PROPERTY(QStringList defaultShots
               READ defaultShots
               CONSTANT )
    // clang-format on
    static QStringList defaultShots();

    // clang-format off
    Q_PROPERTY(Locale locale
               READ locale
               CONSTANT )
    // clang-format on
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

private:
    static QString m_fileNameToOpen;
};

#endif // SCRITE_NAMESPACE_H
