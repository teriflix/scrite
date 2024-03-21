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

#include <QQmlEngine>

class User;
class AppWindow;
class Application;
class ScriteDocument;
class ShortcutsModel;
class NotificationManager;
class ScriteDocumentVault;

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

    Q_PROPERTY(QObject *app READ appObject CONSTANT)
    static QObject *appObject();
    static Application *app();

    Q_PROPERTY(QObject *window READ windowObject CONSTANT)
    static QObject *windowObject();
    static AppWindow *window();

    Q_PROPERTY(QObject *user READ userObject CONSTANT)
    static QObject *userObject();
    static User *user();

    Q_PROPERTY(QObject *document READ documentObject CONSTANT)
    static QObject *documentObject();
    static ScriteDocument *document();

    Q_PROPERTY(QObject *vault READ vaultObject CONSTANT)
    static QObject *vaultObject();
    static ScriteDocumentVault *vault();

    Q_PROPERTY(QObject *shortcuts READ shortcutsObject CONSTANT)
    static QObject *shortcutsObject();
    static ShortcutsModel *shortcuts();

    Q_PROPERTY(QObject *notifications READ notificationsObject CONSTANT)
    static QObject *notificationsObject();
    static NotificationManager *notifications();

    Q_PROPERTY(QString fileNameToOpen READ fileNameToOpen CONSTANT)
    static QString fileNameToOpen() { return m_fileNameToOpen; }
    static void setFileNameToOpen(const QString &val);

private:
    static QString m_fileNameToOpen;
};

#endif // SCRITE_NAMESPACE_H
