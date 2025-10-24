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
    Q_CLASSINFO("DefaultProperty", "actions")
    QML_ATTACHED(ActionManagerAttached)

public:
    explicit ActionManager(QObject *parent = nullptr);
    virtual ~ActionManager();

    static ActionManagerAttached *qmlAttachedProperties(QObject *object);

    Q_INVOKABLE static QString shortcut(int k1, int k2 = 0, int k3 = 0, int k4 = 0);
    Q_INVOKABLE static QKeySequence keySequence(int k1, int k2 = 0, int k3 = 0, int k4 = 0);

    static ActionManager *findManager(QObject *action);
    static bool changeActionShortcut(QObject *action, const QString &shortcut);
    static QKeySequence defaultActionShortcut(QObject *action);
    Q_INVOKABLE static bool restoreActionShortcut(QObject *action);

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(int sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    void setSortOrder(int val);
    int sortOrder() const { return m_sortOrder; }
    Q_SIGNAL void sortOrderChanged();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_actions.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE bool add(QObject *action);
    Q_INVOKABLE bool remove(QObject *action);
    Q_INVOKABLE QObject *at(int index) const { return m_actions.at(index); }
    Q_INVOKABLE QObject *find(const QString &actionName) const;

    QList<QObject *> actions() const { return m_actions; }

    Q_PROPERTY(QQmlListProperty<QObject> actions READ qmlActionsList)
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
    void onVisibilityChanged();
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

    Q_PROPERTY(ActionManager *target READ target WRITE setTarget NOTIFY targetChanged)
    void setTarget(ActionManager *val);
    ActionManager *target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    Q_INVOKABLE static QString shortcut(int k1, int k2 = 0, int k3 = 0, int k4 = 0);
    Q_INVOKABLE static QKeySequence keySequence(int k1, int k2 = 0, int k3 = 0, int k4 = 0);

    Q_INVOKABLE ActionManager *find(const QString &name) const;

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

    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
    void setPriority(int val);
    int priority() const { return m_priority; }
    Q_SIGNAL void priorityChanged();

    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    void setChecked(bool val);
    bool isChecked() const { return m_checked; }
    Q_SIGNAL void checkedChanged();

    Q_PROPERTY(bool down READ isDown WRITE setDown NOTIFY downChanged)
    void setDown(bool val);
    bool isDown() const { return m_down; }
    Q_SIGNAL void downChanged();

    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged)
    void setIconSource(const QString &val);
    QString iconSource() const { return m_iconSource; }
    Q_SIGNAL void iconSourceChanged();

    Q_PROPERTY(QObject *action READ action WRITE setAction NOTIFY actionChanged)
    void setAction(QObject *val);
    QObject *action() const { return m_action; }
    Q_SIGNAL void actionChanged();

    Q_INVOKABLE QObject *findAction(const QString &managerName, const QString &actionName) const;

signals:
    void actionAboutToChange();

signals:
    void toggled(QObject *source = nullptr);
    void triggered(QObject *source = nullptr);

private:
    void onObjectDestroyed(QObject *ptr);

private:
    int m_priority = 0;
    bool m_down = false;
    bool m_checked = false;
    QString m_iconSource;
    QObject *m_action = nullptr;
};

class ActionHandlerAttached : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ANONYMOUS

public:
    virtual ~ActionHandlerAttached();

    Q_PROPERTY(bool canHandle READ canHandle NOTIFY canHandleChanged)
    bool canHandle() const;
    Q_SIGNAL void canHandleChanged();

    Q_PROPERTY(ActionHandler* active READ active NOTIFY canHandleChanged)
    ActionHandler *active() const;

    Q_PROPERTY(QList<ActionHandler*> all READ all NOTIFY canHandleChanged)
    QList<ActionHandler *> all() const;

    Q_INVOKABLE bool trigger();
    Q_INVOKABLE bool triggerAll();

protected:
    explicit ActionHandlerAttached(QObject *parent = nullptr);

private:
    void onHandlerAvailabilityChanged(QObject *action);

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

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_items.size(); }
    Q_SIGNAL void countChanged();

    QString groupNameAt(int row) const;
    ActionManager *actionManagerAt(int row) const;
    QObject *actionAt(int row) const;

    // QAbstractItemModel interface
    enum { GroupNameRole, ActionManagerRole, ActionRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void reload();

    void onActionManagerReset();
    void onActionManagerNameChanged();
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
};

class ActionsModelFilter : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ActionsModelFilter(QObject *parent = nullptr);
    virtual ~ActionsModelFilter();

    void setSourceModel(QAbstractItemModel *model);

    enum Filter {
        AllActions = 0,
        ActionsWithText = 1,
        ActionsWithShortcut = 2,
        ActionsWithObjectName = 4,
        VisibleActions = 8,
        EnabledActions = 16,
        CustomFilter = -1,
        ShortcutsDockFilters = ActionsWithText | ActionsWithShortcut | VisibleActions,
        ShortcutsEditorFilters = ActionsWithText | ActionsWithShortcut | ActionsWithObjectName
    };
    Q_DECLARE_FLAGS(Filters, Filter)
    Q_FLAG(Filters)

    Q_PROPERTY(QString actionTextStartsWith READ actionTextStartsWith WRITE setActionTextStartsWith NOTIFY actionTextStartsWithChanged)
    void setActionTextStartsWith(const QString &val);
    QString actionTextStartsWith() const { return m_actionTextStartsWith; }
    Q_SIGNAL void actionTextStartsWithChanged();

    Q_PROPERTY(Filters filters READ filters WRITE setFilters NOTIFY filtersChanged)
    void setFilters(Filters val);
    Filters filters() const { return m_filters; }
    Q_SIGNAL void filtersChanged();

    // QQmlParserStatus interface
    void classBegin() { }
    void componentComplete();

signals:
    void filterRequest(QObject *qmlAction, ActionManager *actionManager, BooleanResult *result);

protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    Filters m_filters = AllActions;
    QString m_actionTextStartsWith;
};

#endif // ACTIONMANAGER_H
