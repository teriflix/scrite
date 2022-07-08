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
        MinFeature = ScreenplayFeature,
        MaxFeature = ScritedFeature
    };
    Q_ENUM(AppFeature)

    Q_PROPERTY(QObject *app READ appObject CONSTANT)
    QObject *appObject() const;
    Application *app() const;

    Q_PROPERTY(QObject *window READ windowObject CONSTANT)
    QObject *windowObject() const;
    AppWindow *window() const;

    Q_PROPERTY(QObject *user READ userObject CONSTANT)
    QObject *userObject() const;
    User *user() const;

    Q_PROPERTY(QObject *document READ documentObject CONSTANT)
    QObject *documentObject() const;
    ScriteDocument *document() const;

    Q_PROPERTY(QObject *vault READ vaultObject CONSTANT)
    QObject *vaultObject() const;
    ScriteDocumentVault *vault() const;

    Q_PROPERTY(QObject *shortcuts READ shortcutsObject CONSTANT)
    QObject *shortcutsObject() const;
    ShortcutsModel *shortcuts() const;

    Q_PROPERTY(QObject *notifications READ notificationsObject CONSTANT)
    QObject *notificationsObject() const;
    NotificationManager *notifications() const;
};

#endif // SCRITE_NAMESPACE_H
