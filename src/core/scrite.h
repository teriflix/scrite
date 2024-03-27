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
#include "application.h"
#include "scritedocument.h"
#include "shortcutsmodel.h"
#include "notificationmanager.h"
#include "scritedocumentvault.h"

#include <QQmlEngine>

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

    Q_PROPERTY(Application *app READ app CONSTANT)
    static Application *app();

    Q_PROPERTY(AppWindow *window READ window CONSTANT)
    static AppWindow *window();

    Q_PROPERTY(User *user READ user CONSTANT)
    static User *user();

    Q_PROPERTY(ScriteDocument *document READ document CONSTANT)
    static ScriteDocument *document();

    Q_PROPERTY(ScriteDocumentVault *vault READ vault CONSTANT)
    static ScriteDocumentVault *vault();

    Q_PROPERTY(ShortcutsModel *shortcuts READ shortcuts CONSTANT)
    static ShortcutsModel *shortcuts();

    Q_PROPERTY(NotificationManager *notifications READ notifications CONSTANT)
    static NotificationManager *notifications();

    Q_PROPERTY(QString fileNameToOpen READ fileNameToOpen CONSTANT)
    static QString fileNameToOpen() { return m_fileNameToOpen; }
    static void setFileNameToOpen(const QString &val);

private:
    static QString m_fileNameToOpen;
};

#endif // SCRITE_NAMESPACE_H
