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

#include "scrite.h"

#include "user.h"
#include "appwindow.h"
#include "application.h"
#include "scritedocument.h"
#include "shortcutsmodel.h"
#include "notificationmanager.h"
#include "scritedocumentvault.h"

Scrite::Scrite(QObject *parent) : QObject(parent)
{
    qDebug() << "Warning: Scrite namespace being created.";
}

Scrite::~Scrite()
{
    qDebug() << "Warning: Scrite namespace being destroyed.";
}

Application *Scrite::app()
{
    return Application::instance();
}

AppWindow *Scrite::window()
{
    return AppWindow::instance();
}

User *Scrite::user()
{
    return User::instance();
}

RestApi *Scrite::restApi()
{
    return RestApi::instance();
}

ScriteDocument *Scrite::document()
{
    return ScriteDocument::instance();
}

ScriteDocumentVault *Scrite::vault()
{
    return ScriteDocumentVault::instance();
}

ShortcutsModel *Scrite::shortcuts()
{
    return ShortcutsModel::instance();
}

NotificationManager *Scrite::notifications()
{
    return NotificationManager::instance();
}

QString Scrite::m_fileNameToOpen;
void Scrite::setFileNameToOpen(const QString &val)
{
    if (m_fileNameToOpen.isEmpty())
        m_fileNameToOpen = val;
}

QStringList Scrite::defaultTransitions()
{
    return QStringList(
            { QStringLiteral("CUT TO"), QStringLiteral("DISSOLVE TO"), QStringLiteral("FADE IN"),
              QStringLiteral("FADE OUT"), QStringLiteral("FADE TO"), QStringLiteral("FLASHBACK"),
              QStringLiteral("FLASH CUT TO"), QStringLiteral("FREEZE FRAME"),
              QStringLiteral("IRIS IN"), QStringLiteral("IRIS OUT"), QStringLiteral("JUMP CUT TO"),
              QStringLiteral("MATCH CUT TO"), QStringLiteral("MATCH DISSOLVE TO"),
              QStringLiteral("SMASH CUT TO"), QStringLiteral("STOCK SHOT"),
              QStringLiteral("TIME CUT"), QStringLiteral("WIPE TO") });
}

QStringList Scrite::defaultShots()
{
    return QStringList({ QStringLiteral("AIR"), QStringLiteral("CLOSE ON"),
                         QStringLiteral("CLOSER ON"), QStringLiteral("CLOSEUP"),
                         QStringLiteral("ESTABLISHING"), QStringLiteral("EXTREME CLOSEUP"),
                         QStringLiteral("INSERT"), QStringLiteral("POV"), QStringLiteral("SURFACE"),
                         QStringLiteral("THREE SHOT"), QStringLiteral("TWO SHOT"),
                         QStringLiteral("UNDERWATER"), QStringLiteral("WIDE"),
                         QStringLiteral("WIDE ON"), QStringLiteral("WIDER ANGLE") });
}

Locale Scrite::locale()
{
    Locale ret;

    const QLocale locale = QLocale::system();

    ret.currency.code = locale.currencySymbol(QLocale::CurrencyIsoCode).toLower();
    ret.currency.symbol = locale.currencySymbol(QLocale::CurrencySymbol);
    ret.country.code = locale.countryToString(locale.country());
    ret.country.name = locale.nativeCountryName();

    return ret;
}

QString Scrite::currencySymbol(const QString &code)
{
    const QString code2 = code.toLower();
    static const QMap<QString, QString> knownValues = { { "inr", "₹" }, { "usd", "$" } };
    if (knownValues.contains(code2))
        return knownValues.value(code2);

    for (int i = 1; i <= QLocale::LastCountry; i++) {
        const QLocale locale(QLocale::AnyLanguage, QLocale::Country(i));
        if (locale.currencySymbol(QLocale::CurrencyIsoCode).toLower() == code2)
            return locale.currencySymbol(QLocale::CurrencySymbol);
    }

    return code.toUpper() + " ";
}

bool Scrite::isFeatureEnabled(AppFeature feature, const QStringList &features)
{
    static const QMap<AppFeature, QString> featureNameMap = {
        { Scrite::ScreenplayFeature, "screenplay" },
        { Scrite::StructureFeature, "structure" },
        { Scrite::NotebookFeature, "notebook" },
        { Scrite::RelationshipGraphFeature, "relationshipgraph" },
        { Scrite::ScriptalayFeature, "scriptalay" },
        { Scrite::TemplateFeature, "template" },
        { Scrite::ReportFeature, "report" },
        { Scrite::ImportFeature, "import" },
        { Scrite::ExportFeature, "export" },
        { Scrite::ScritedFeature, "scrited" },
        { Scrite::WatermarkFeature, "watermark" },
        { Scrite::RecentFilesFeature, "recentfiles" },
        { Scrite::VaultFilesFeature, "vaultfiles" }
    };

    return Scrite::isFeatureNameEnabled(featureNameMap.value(feature), features);
}

bool Scrite::isFeatureNameEnabled(const QString &featureName, const QStringList &features)
{
    const QString lfeatureName = featureName.toLower();
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
