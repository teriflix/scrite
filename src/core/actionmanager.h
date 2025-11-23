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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QTimer>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "booleanresult.h"

class ActionManagerAttached;
class ActionManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    // clang-format off
    Q_CLASSINFO("DefaultProperty", "actions")
    // clang-format on
    QML_ATTACHED(ActionManagerAttached)

public:
    explicit ActionManager(QObject *parent = nullptr);
    virtual ~ActionManager();

    static ActionManagerAttached *qmlAttachedProperties(QObject *object);

    static ActionManager *findManager(QObject *action);
    static bool canChangeActionShortcut(QObject *action);
    static bool changeActionShortcut(QObject *action, const QString &shortcut);
    static QKeySequence defaultActionShortcut(QObject *action);
    static bool restoreActionShortcut(QObject *action);
    static QObject *findActionForShortcut(const QString &shortcut);

    // clang-format off
    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)
    // clang-format on
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    // clang-format off
    Q_PROPERTY(int sortOrder
               READ sortOrder
               WRITE setSortOrder
               NOTIFY sortOrderChanged)
    // clang-format on
    void setSortOrder(int val);
    int sortOrder() const { return m_sortOrder; }
    Q_SIGNAL void sortOrderChanged();

    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    // clang-format on
    int count() const { return m_actions.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE bool add(QObject *action);
    Q_INVOKABLE bool remove(QObject *action);
    Q_INVOKABLE bool contains(QObject *action) const;
    Q_INVOKABLE QObject *at(int index) const { return m_actions.at(index); }
    Q_INVOKABLE QObject *find(const QString &actionName) const;
    Q_INVOKABLE QObject *findByShortcut(const QString &shortcut) const;

    QList<QObject *> actions() const { return m_actions; }

    // clang-format off
    Q_PROPERTY(QQmlListProperty<QObject> actions
               READ qmlActionsList)
    // clang-format on
    QQmlListProperty<QObject> qmlActionsList();
    void addAction(QObject *ptr) { this->add(ptr); }
    void removeAction(QObject *ptr) { this->remove(ptr); }
    QObject *actionAt(int index) const { return this->at(index); }
    int actionCount() const { return m_actions.size(); }
    void clearActions();

private:
    static void staticAppendAction(QQmlListProperty<QObject> *list, QObject *ptr);
    static void staticClearActions(QQmlListProperty<QObject> *list);
    static QObject *staticActionAt(QQmlListProperty<QObject> *list, int index);
    static int staticActionCount(QQmlListProperty<QObject> *list);

public:
    // QAbstractItemModel interface
    enum RoleNames { ActionRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    bool addInternal(QObject *action);
    bool removeInternal(QObject *action);
    void onObjectDestroyed(QObject *action);
    static void sortActions(QList<QObject *> &actions);

    void saveShortcutMap();
    void applySavedShortcuts();
    void scheduleApplySavedShortcuts();
    QString shortcutMapFilePath() const;

private slots:
    void onSortOrderChanged();
    void onActionDataChanged();
    void onActionShortcutChanged(const QKeySequence &newShortcut);

private:
    int m_sortOrder = 0;
    QTimer *m_sortActionsTimer = nullptr;
    QString m_title;
    QList<QObject *> m_actions;
    QVariantMap m_shortcutMap;
    QPointer<QTimer> m_applySavedShortcutsTimer;
};

class ActionManagerAttached : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ANONYMOUS

public:
    virtual ~ActionManagerAttached();

    // clang-format off
    Q_PROPERTY(ActionManager *target
               READ target
               WRITE setTarget
               NOTIFY targetChanged)
    // clang-format on
    void setTarget(ActionManager *val);
    ActionManager *target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    Q_INVOKABLE ActionManager *find(const QString &name) const;
    Q_INVOKABLE QObject *findActionForShortcut(const QString &shortcut) const;

protected:
    explicit ActionManagerAttached(QObject *parent = nullptr);

private:
    void onHandlerAvailabilityChanged(QObject *action);

private:
    friend class ActionManager;

    QObject *m_action = nullptr;
    ActionManager *m_target = nullptr;
};

class ActionHandlerAttached;
class ActionHandler : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(ActionHandlerAttached)

public:
    explicit ActionHandler(QQuickItem *parent = nullptr);
    virtual ~ActionHandler();

    static ActionHandlerAttached *qmlAttachedProperties(QObject *parent);

    // clang-format off
    Q_PROPERTY(int priority
               READ priority
               WRITE setPriority
               NOTIFY priorityChanged)
    // clang-format on
    void setPriority(int val);
    int priority() const { return m_priority; }
    Q_SIGNAL void priorityChanged();

    // clang-format off
    Q_PROPERTY(bool checked
               READ isChecked
               WRITE setChecked
               NOTIFY checkedChanged)
    // clang-format on
    void setChecked(bool val);
    bool isChecked() const { return m_checked; }
    Q_SIGNAL void checkedChanged();

    // clang-format off
    Q_PROPERTY(bool down
               READ isDown
               WRITE setDown
               NOTIFY downChanged)
    // clang-format on
    void setDown(bool val);
    bool isDown() const { return m_down; }
    Q_SIGNAL void downChanged();

    // clang-format off
    Q_PROPERTY(QString iconSource
               READ iconSource
               WRITE setIconSource
               NOTIFY iconSourceChanged)
    // clang-format on
    void setIconSource(const QString &val);
    QString iconSource() const { return m_iconSource; }
    Q_SIGNAL void iconSourceChanged();

    // clang-format off
    Q_PROPERTY(QString tooltip
               READ tooltip
               WRITE setTooltip
               NOTIFY tooltipChanged)
    // clang-format on
    void setTooltip(const QString &val);
    QString tooltip() const { return m_tooltip; }
    Q_SIGNAL void tooltipChanged();

    // clang-format off
    Q_PROPERTY(QObject *action
               READ action
               WRITE setAction
               NOTIFY actionChanged)
    // clang-format on
    void setAction(QObject *val);
    QObject *action() const { return m_action; }
    Q_SIGNAL void actionChanged();

    Q_INVOKABLE QObject *findAction(const QString &managerName, const QString &actionName) const;

    void componentComplete();

signals:
    void actionAboutToChange();
    void toggled(QObject *source = nullptr);
    void triggered(QObject *source = nullptr);
    void triggerCountChanged(int count);

private:
    Q_SLOT void onToggled(QObject *source = nullptr);
    Q_SLOT void onTriggered(QObject *source = nullptr);
    void onObjectDestroyed(QObject *ptr);
    Q_SLOT void checkTriggerCount();

private:
    int m_priority = 0;
    bool m_down = false;
    bool m_checked = false;
    QString m_iconSource;
    QString m_tooltip;
    QObject *m_action = nullptr;
};

class ActionHandlerAttached : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ANONYMOUS

public:
    virtual ~ActionHandlerAttached();

    // clang-format off
    Q_PROPERTY(bool canHandle
               READ canHandle
               NOTIFY canHandleChanged)
    // clang-format on
    bool canHandle() const;
    Q_SIGNAL void canHandleChanged();

    // clang-format off
    Q_PROPERTY(ActionHandler *active
               READ active
               NOTIFY canHandleChanged)
    // clang-format on
    ActionHandler *active() const;

    // clang-format off
    Q_PROPERTY(QList<ActionHandler *>
               all READ
               all NOTIFY
               canHandleChanged )
    // clang-format on
    QList<ActionHandler *> all() const;

    Q_INVOKABLE bool trigger();
    Q_INVOKABLE bool triggerAll();

protected:
    explicit ActionHandlerAttached(QObject *parent = nullptr);

private:
    void onHandlerAvailabilityChanged(QObject *action, ActionHandler *handler);

private:
    friend class ActionHandler;
    QObject *m_action = nullptr;
};

class ActionHandlers : public QObject
{
    Q_OBJECT

public:
    static ActionHandlers *instance();

    virtual ~ActionHandlers();

    ActionHandler *findFirst(QObject *object, bool enabledOnly = true) const;
    QList<ActionHandler *> findAll(QObject *object, bool enabledOnly = true) const;

signals:
    void handlerAvailabilityChanged(QObject *action, ActionHandler *handler);
    void handlerCheckedChanged(QObject *action, ActionHandler *handler);
    void handlerDownChanged(QObject *action, ActionHandler *handler);

private:
    explicit ActionHandlers(QObject *parent = nullptr);

    void add(ActionHandler *handler);
    void remove(ActionHandler *handler);
    void sortHandlersByPriority();

    void notifyHandlerAvailability();
    void onHandlerPriorityChanged();
    void onHandlerCheckedChanged();
    void onHandlerDownChanged();
    void onHanlderActionAboutToChange();

private:
    friend class ActionHandler;
    bool m_appAboutToQuit = false;
    QList<ActionHandler *> m_actionHandlers;
};

class ActionsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ActionsModel(QObject *parent = nullptr);
    virtual ~ActionsModel();

    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    // clang-format on
    int count() const { return m_items.size(); }
    Q_SIGNAL void countChanged();

    // clang-format off
    Q_PROPERTY(QList<ActionManager*> actionManagers
               READ actionManagers
               WRITE setActionManagers
               NOTIFY actionManagersChanged)
    // clang-format on
    void setActionManagers(const QList<ActionManager *> &val);
    QList<ActionManager *> actionManagers() const { return m_actionManagers; }
    Q_SIGNAL void actionManagersChanged();

    Q_INVOKABLE QString groupNameAt(int row) const;
    Q_INVOKABLE ActionManager *actionManagerAt(int row) const;
    Q_INVOKABLE QObject *actionAt(int row) const;
    Q_INVOKABLE int indexOfAction(QObject *action) const;

    Q_INVOKABLE QObject *findActionForShortcut(const QString &shortcut) const;
    Q_INVOKABLE bool restoreActionShortcut(QObject *action) const;
    Q_INVOKABLE bool isActionShortcutEditable(QObject *action) const;

    // QAbstractItemModel interface
    enum { GroupNameRole, ActionManagerRole, ActionRole, ShortcutIsEditableRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void reload();

    void onActionManagerReset();
    void onActionManagerNameChanged();
    void onActionManagerDataChanged(const QModelIndex &start, const QModelIndex &end);
    void onActionManagerRowsRemoved(const QModelIndex &index, int start, int end);
    void onActionManagerRowsInserted(const QModelIndex &index, int start, int end);

    void onActionManagerModelReset();
    void onActionManagerModelRowsRemoved(const QModelIndex &index, int start, int end);
    void onActionManagerModelRowsInserted(const QModelIndex &index, int start, int end);

private:
    QPair<int, int> findRowRange(QObject *actionManager) const;

    struct Item
    {
        QPointer<QObject> actionManager;
        QPointer<QObject> action;
    };
    QList<Item> m_items;

    QList<ActionManager *> m_actionManagers;
};

class ActionsModelFilter : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit ActionsModelFilter(QObject *parent = nullptr);
    virtual ~ActionsModelFilter();

    void setSourceModel(QAbstractItemModel *model);

    enum Filter {
        NoActions = -1,
        AllActions = 0,
        ActionsWithText = 1,
        ActionsWithShortcut = 2,
        ActionsWithDefaultShortcut = 4,
        ActionsWithObjectName = 8,
        VisibleActions = 16,
        EnabledActions = 32,
        HideCommandCenterActions = 64,
        ShortcutsEditorFilters = ActionsWithText | ActionsWithObjectName,
        CommandCenterFilters = ActionsWithText | EnabledActions | HideCommandCenterActions
    };
    Q_DECLARE_FLAGS(Filters, Filter)
    Q_FLAG(Filters)

    // clang-format off
    Q_PROPERTY(QString actionManagerTitle
               READ actionManagerTitle
               WRITE setActionManagerTitle
               NOTIFY actionManagerTitleChanged)
    // clang-format on
    void setActionManagerTitle(const QString &val);
    QString actionManagerTitle() const { return m_actionManagerTitleFilter; }
    Q_SIGNAL void actionManagerTitleChanged();

    // clang-format off
    Q_PROPERTY(QString actionText
               READ actionText
               WRITE setActionText
               NOTIFY actionTextChanged)
    // clang-format on
    void setActionText(const QString &val);
    QString actionText() const { return m_actionTextFilter; }
    Q_SIGNAL void actionTextChanged();

    // clang-format off
    Q_PROPERTY(Filters filters
               READ filters
               WRITE setFilters
               NOTIFY filtersChanged)
    // clang-format on
    void setFilters(Filters val);
    Filters filters() const { return m_filters; }
    Q_SIGNAL void filtersChanged();

    // clang-format off
    Q_PROPERTY(bool customFilterMode
               READ isCustomFilterMode
               WRITE setCustomFilterMode
               NOTIFY customFilterModeChanged)
    // clang-format on
    void setCustomFilterMode(bool val);
    bool isCustomFilterMode() const { return m_customFilterMode; }
    Q_SIGNAL void customFilterModeChanged();

    Q_INVOKABLE QObject *findActionForShortcut(const QString &shortcut) const;
    Q_INVOKABLE bool restoreActionShortcut(QObject *action) const;
    Q_INVOKABLE int restoreAllActionShortcuts();
    Q_INVOKABLE void filter();

    // QQmlParserStatus interface
    void classBegin() { }
    void componentComplete();

signals:
    void filterRequest(QObject *qmlAction, ActionManager *actionManager, BooleanResult *result);

protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    bool m_customFilterMode = false;
    Filters m_filters = AllActions;
    QString m_actionTextFilter;
    QString m_actionManagerTitleFilter;
};

#endif // ACTIONMANAGER_H
