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

#include "actionmanager.h"
#include "application.h"
#include "qobjectlistmodel.h"

#include <QTimer>
#include <QGuiApplication>
#include <QDynamicPropertyChangeEvent>

static const char *_QQuickAction = "QQuickAction";
static const QByteArray _QQuickActionSortOrderProperty = QByteArrayLiteral("sortOrder");
static const char *_QQuickActionSortOrderChanged = SIGNAL(sortOrderChanged());
static const QByteArray _QQuickActionVisibleProperty = QByteArrayLiteral("visible");
static const char *_QQuickActionVisibilityChanged = SIGNAL(visibleChanged());

Q_GLOBAL_STATIC(QObjectListModel<ActionManager *>, ActionManagerModel)

static bool ActionManagerModelSortFunction(ActionManager *a, ActionManager *b)
{
    if (a->sortOrder() == b->sortOrder())
        return QString::localeAwareCompare(a->objectName(), b->objectName()) < 0;

    return a->sortOrder() - b->sortOrder() < 0;
}

static ActionManager *findActionManager(const QString &name)
{
    const QList<ActionManager *> list = ::ActionManagerModel->constList();
    auto it = std::find_if(list.begin(), list.end(),
                           [name](ActionManager *am) { return am->objectName() == name; });

    if (it != list.end())
        return *it;

    return nullptr;
}

ActionManager::ActionManager(QObject *parent) : QAbstractListModel(parent)
{
    ActionManagerModel->append(this);

    connect(this, &ActionManager::modelReset, this, &ActionManager::countChanged);
    connect(this, &ActionManager::rowsRemoved, this, &ActionManager::countChanged);
    connect(this, &ActionManager::rowsInserted, this, &ActionManager::countChanged);
}

ActionManager::~ActionManager()
{
    ActionManagerModel->remove(this);
}

ActionManagerAttached *ActionManager::qmlAttachedProperties(QObject *object)
{
    return new ActionManagerAttached(object);
}

QString ActionManager::shortcut(int k1, int k2, int k3, int k4)
{
    return keySequence(k1, k2, k3, k4).toString();
}

QKeySequence ActionManager::keySequence(int k1, int k2, int k3, int k4)
{
    return QKeySequence(k1, k2, k3, k4);
}

void ActionManager::setTitle(const QString &val)
{
    if (m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void ActionManager::setSortOrder(int val)
{
    if (m_sortOrder == val)
        return;

    m_sortOrder = val;
    emit sortOrderChanged();
}

bool ActionManager::add(QObject *action)
{
    return this->addInternal(action);
}

bool ActionManager::remove(QObject *action)
{
    return this->removeInternal(action);
}

QObject *ActionManager::find(const QString &actionName) const
{
    auto it = std::find_if(m_actions.begin(), m_actions.end(), [actionName](QObject *action) {
        return (action->objectName() == actionName);
    });
    if (it != m_actions.end())
        return *it;

    return nullptr;
}

QQmlListProperty<QObject> ActionManager::qmlActionsList()
{
    return QQmlListProperty<QObject>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &ActionManager::staticAppendAction, &ActionManager::staticActionCount,
            &ActionManager::staticActionAt, &ActionManager::staticClearActions);
}

void ActionManager::clearActions()
{
    while (m_actions.size())
        this->removeAction(m_actions.first());
}

void ActionManager::staticAppendAction(QQmlListProperty<QObject> *list, QObject *ptr)
{
    reinterpret_cast<ActionManager *>(list->data)->addAction(ptr);
}

void ActionManager::staticClearActions(QQmlListProperty<QObject> *list)
{
    reinterpret_cast<ActionManager *>(list->data)->clearActions();
}

QObject *ActionManager::staticActionAt(QQmlListProperty<QObject> *list, int index)
{
    return reinterpret_cast<ActionManager *>(list->data)->actionAt(index);
}

int ActionManager::staticActionCount(QQmlListProperty<QObject> *list)
{
    return reinterpret_cast<ActionManager *>(list->data)->actionCount();
}

int ActionManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_actions.size();
}

QVariant ActionManager::data(const QModelIndex &index, int role) const
{
    if (role == ActionRole && index.row() >= 0 && index.row() < m_actions.size())
        return QVariant::fromValue<QObject *>(m_actions.at(index.row()));

    return QVariant();
}

QHash<int, QByteArray> ActionManager::roleNames() const
{
    return { { ActionRole, QByteArrayLiteral("qmlAction") } };
}

bool ActionManager::addInternal(QObject *action)
{
    if (action != nullptr && !m_actions.contains(action) && action->inherits(_QQuickAction)) {

        connect(action, &QObject::destroyed, this, &ActionManager::onObjectDestroyed);

        const QMetaProperty sortOrderProperty = action->metaObject()->property(
                action->metaObject()->indexOfProperty(_QQuickActionSortOrderProperty));
        if (sortOrderProperty.isValid()) {
            if (sortOrderProperty.isWritable() && !sortOrderProperty.isConstant()
                && sortOrderProperty.hasNotifySignal()) {
                connect(action, _QQuickActionSortOrderChanged, this, SLOT(onSortOrderChanged()));
            }
        }

        const QMetaProperty visibleProperty = action->metaObject()->property(
                action->metaObject()->indexOfProperty(_QQuickActionVisibleProperty));
        if (visibleProperty.isValid()) {
            if (visibleProperty.isWritable() && !visibleProperty.isConstant()
                && visibleProperty.hasNotifySignal())
                connect(action, _QQuickActionVisibilityChanged, this, SLOT(onVisibilityChanged()));
        }

        QList<QObject *> actions = m_actions;
        actions.append(action);
        sortActions(actions);

        int insertIndex = actions.indexOf(action);

        this->beginInsertRows(QModelIndex(), insertIndex, insertIndex);
        m_actions = actions;
        this->endInsertRows();

        return true;
    }

    return false;
}

bool ActionManager::removeInternal(QObject *action)
{
    const int row = m_actions.indexOf(action);
    if (row < 0)
        return false;

    this->beginRemoveRows(QModelIndex(), row, row);

    m_actions.removeAt(row);
    disconnect(action, nullptr, this, nullptr);

    this->endRemoveRows();

    return false;
}

void ActionManager::onObjectDestroyed(QObject *action)
{
    this->removeInternal(action);
}

void ActionManager::sortActions(QList<QObject *> &actions)
{
    QMap<int, QList<QObject *>> map;
    while (!actions.isEmpty()) {
        QObject *action = actions.takeFirst();
        const QMetaObject *mo = action->metaObject();
        const QMetaProperty sortOrderProp =
                mo->property(mo->indexOfProperty(_QQuickActionSortOrderProperty));
        const int sortOrder = sortOrderProp.isValid() ? sortOrderProp.read(action).toInt() : 0;
        map[sortOrder].append(action);
    }

    auto it = map.begin();
    auto end = map.end();
    while (it != end) {
        actions += it.value();
        ++it;
    }
}

void ActionManager::onSortOrderChanged()
{
    if (m_sortActionsTimer == nullptr) {
        m_sortActionsTimer = new QTimer(this);
        m_sortActionsTimer->setInterval(0);
        m_sortActionsTimer->setSingleShot(true);
        connect(m_sortActionsTimer, &QTimer::timeout, this, [=]() {
            this->beginResetModel();
            sortActions(m_actions);
            this->endResetModel();
        });
    }

    m_sortActionsTimer->start();
}

void ActionManager::onVisibilityChanged()
{
    QObject *action = this->sender();

    int row = m_actions.indexOf(action);
    if (row >= 0) {
        const QModelIndex index = this->index(row, 0);
        emit dataChanged(index, index);
    }
}

///////////////////////////////////////////////////////////////////////////////

ActionManagerAttached::ActionManagerAttached(QObject *parent) : QObject(parent)
{
    if (parent && parent->inherits(_QQuickAction)) {
        m_action = parent;
        if (m_action != nullptr)
            connect(ActionHandlers::instance(), &ActionHandlers::handlerAvailabilityChanged, this,
                    &ActionManagerAttached::onHandlerAvailabilityChanged);
    }
}

ActionManagerAttached::~ActionManagerAttached() { }

void ActionManagerAttached::setTarget(ActionManager *val)
{
    if (m_target == val)
        return;

    if (m_target != nullptr)
        m_target->remove(m_action);

    m_target = val;

    if (m_target != nullptr)
        m_target->add(m_action);

    emit targetChanged();
}

bool ActionManagerAttached::canHandle() const
{
    return ActionHandlers::instance()->findFirst(m_action, true) != nullptr;
}

QString ActionManagerAttached::shortcut(int k1, int k2, int k3, int k4)
{
    return ActionManager::shortcut(k1, k2, k3, k4);
}

QKeySequence ActionManagerAttached::keySequence(int k1, int k2, int k3, int k4)
{
    return ActionManager::keySequence(k1, k2, k3, k4);
}

bool ActionManagerAttached::trigger()
{
    ActionHandler *handler = ActionHandlers::instance()->findFirst(m_action, true);
    if (handler) {
        emit handler->triggered(this->parent());
        return true;
    }

    return false;
}

bool ActionManagerAttached::triggerAll()
{
    QList<ActionHandler *> handlers = ActionHandlers::instance()->findAll(m_action, true);

    for (ActionHandler *handler : qAsConst(handlers))
        emit handler->triggered(this->parent());

    return !handlers.isEmpty();
}

ActionManager *ActionManagerAttached::find(const QString &name) const
{
    return ::findActionManager(name);
}

void ActionManagerAttached::onHandlerAvailabilityChanged(QObject *action)
{
    if (m_action == action)
        emit canHandleChanged();
}

///////////////////////////////////////////////////////////////////////////////

ActionHandler::ActionHandler(QQuickItem *parent) : QQuickItem(parent)
{

    this->setFlag(QQuickItem::ItemHasContents, false);

    ActionHandlers::instance()->add(this);
}

ActionHandler::~ActionHandler()
{
    ActionHandlers::instance()->remove(this);
}

ActionHandlerAttached *ActionHandler::qmlAttachedProperties(QObject *parent)
{
    return new ActionHandlerAttached(parent);
}

void ActionHandler::setPriority(int val)
{
    if (m_priority == val)
        return;

    m_priority = val;
    emit priorityChanged();
}

void ActionHandler::setAction(QObject *val)
{
    if (m_action == val)
        return;

    if (m_action != nullptr) {
        emit actionAboutToChange();
        disconnect(m_action, nullptr, this, nullptr);
    }

    m_action = val && val->inherits(_QQuickAction) ? val : nullptr;

    if (m_action != nullptr) {
        connect(m_action, &QObject::destroyed, this, &ActionHandler::onObjectDestroyed);

        // clang-format off
        connect(m_action, SIGNAL(toggled(QObject*)), this, SIGNAL(toggled(QObject*)));
        connect(m_action, SIGNAL(triggered(QObject*)), this, SIGNAL(triggered(QObject*)));
        // clang-format on
    }

    emit actionChanged();
}

QObject *ActionHandler::findAction(const QString &managerName, const QString &actionName) const
{
    ActionManager *actionManager = ::findActionManager(managerName);
    return actionManager ? actionManager->find(actionName) : nullptr;
}

void ActionHandler::onObjectDestroyed(QObject *ptr)
{
    if (m_action == ptr && m_action != nullptr) {
        emit actionAboutToChange();
        m_action = nullptr;
        emit actionChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////

ActionHandlerAttached::ActionHandlerAttached(QObject *parent) : QObject(parent)
{
    if (parent && parent->inherits(_QQuickAction)) {
        m_action = parent;
        if (m_action != nullptr)
            connect(ActionHandlers::instance(), &ActionHandlers::handlerAvailabilityChanged, this,
                    &ActionHandlerAttached::onHandlerAvailabilityChanged);
    }
}

ActionHandlerAttached::~ActionHandlerAttached() { }

bool ActionHandlerAttached::canHandle() const
{
    return ActionHandlers::instance()->findFirst(m_action, true) != nullptr;
}

bool ActionHandlerAttached::trigger()
{
    ActionHandler *handler = ActionHandlers::instance()->findFirst(m_action, true);
    if (handler) {
        emit handler->triggered(this->parent());
        return true;
    }

    return false;
}

bool ActionHandlerAttached::triggerAll()
{
    QList<ActionHandler *> handlers = ActionHandlers::instance()->findAll(m_action, true);

    for (ActionHandler *handler : qAsConst(handlers))
        emit handler->triggered(this->parent());

    return !handlers.isEmpty();
}

void ActionHandlerAttached::onHandlerAvailabilityChanged(QObject *action)
{
    if (m_action == action)
        emit canHandleChanged();
}

///////////////////////////////////////////////////////////////////////////////

ActionHandlers *ActionHandlers::instance()
{
    static ActionHandlers actionHandlers;
    return &actionHandlers;
}

ActionHandlers::ActionHandlers(QObject *parent) : QObject(parent)
{
    connect(qApp, &QCoreApplication::aboutToQuit, this, [=]() { m_appAboutToQuit = true; });
}

ActionHandlers::~ActionHandlers() { }

ActionHandler *ActionHandlers::findFirst(QObject *object, bool enabledOnly) const
{
    if (object == nullptr || m_appAboutToQuit)
        return nullptr;

    auto it = std::find_if(m_actionHandlers.constBegin(), m_actionHandlers.constEnd(),
                           [object, enabledOnly](ActionHandler *handler) {
                               return handler->action() == object
                                       && (enabledOnly ? handler->isEnabled() : true);
                           });
    if (it != m_actionHandlers.end())
        return *it;

    return nullptr;
}

QList<ActionHandler *> ActionHandlers::findAll(QObject *object, bool enabledOnly) const
{
    QList<ActionHandler *> ret;

    if (object == nullptr || m_appAboutToQuit)
        return ret;

    std::copy_if(m_actionHandlers.constBegin(), m_actionHandlers.constEnd(),
                 std::back_inserter(ret), [object, enabledOnly](ActionHandler *handler) {
                     return handler->action() == object
                             && (enabledOnly ? handler->isEnabled() : true);
                 });

    return ret;
}

void ActionHandlers::add(ActionHandler *handler)
{
    if (m_appAboutToQuit)
        return;

    if (handler && !m_actionHandlers.contains(handler)) {
        m_actionHandlers.append(handler);

        if (handler->action())
            emit handlerAvailabilityChanged(handler->action());

        connect(handler, &ActionHandler::actionChanged, this,
                &ActionHandlers::notifyHandlerAvailability);
        connect(handler, &ActionHandler::priorityChanged, this,
                &ActionHandlers::onHandlerPriorityChanged);
        connect(handler, &ActionHandler::priorityChanged, this,
                &ActionHandlers::notifyHandlerAvailability);
        connect(handler, &ActionHandler::enabledChanged, this,
                &ActionHandlers::notifyHandlerAvailability);
        connect(handler, &ActionHandler::actionAboutToChange, this,
                &ActionHandlers::onHanlderActionAboutToChange);
    }
}

void ActionHandlers::remove(ActionHandler *handler)
{
    if (handler) {
        const int index = m_actionHandlers.indexOf(handler);
        if (index < 0)
            return;

        disconnect(handler, nullptr, this, nullptr);

        m_actionHandlers.removeAt(index);

        if (!m_appAboutToQuit && handler->action())
            emit handlerAvailabilityChanged(handler->action());
    }
}

void ActionHandlers::notifyHandlerAvailability()
{
    if (m_appAboutToQuit)
        return;

    ActionHandler *handler = qobject_cast<ActionHandler *>(this->sender());
    if (handler && handler->action()) {
        emit handlerAvailabilityChanged(handler->action());
    }
}

void ActionHandlers::onHandlerPriorityChanged()
{
    if (m_appAboutToQuit)
        return;

    std::sort(m_actionHandlers.begin(), m_actionHandlers.end(),
              [](ActionHandler *a, ActionHandler *b) { return b->priority() - a->priority() < 0; });
}

void ActionHandlers::onHanlderActionAboutToChange()
{
    if (m_appAboutToQuit)
        return;

    ActionHandler *handler = qobject_cast<ActionHandler *>(this->sender());
    if (handler && handler->action()) {
        QObject *oldAction = handler->action();

        // clang-format off
        QMetaObject::invokeMethod(this, "handlerAvailabilityChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(QObject*, oldAction));
        // clang-format on
    }
}

///////////////////////////////////////////////////////////////////////////////

ActionsModel::ActionsModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(this, &ActionsModel::modelReset, this, &ActionsModel::countChanged);
    connect(this, &ActionsModel::rowsRemoved, this, &ActionsModel::countChanged);
    connect(this, &ActionsModel::rowsInserted, this, &ActionsModel::countChanged);

    this->reload();

    connect(::ActionManagerModel, &QAbstractListModel::modelReset, this,
            &ActionsModel::onActionManagerModelReset);
    connect(::ActionManagerModel, &QAbstractListModel::rowsRemoved, this,
            &ActionsModel::onActionManagerModelRowsRemoved);
    connect(::ActionManagerModel, &QAbstractListModel::rowsInserted, this,
            &ActionsModel::onActionManagerModelRowsInserted);
}

ActionsModel::~ActionsModel() { }

int ActionsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant ActionsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    switch (role) {
    case GroupNameRole:
        return m_items[index.row()].actionManager->objectName();
    case ActionManagerRole:
        return QVariant::fromValue<QObject *>(m_items[index.row()].actionManager);
    case ActionRole:
        return QVariant::fromValue<QObject *>(m_items[index.row()].action);
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> ActionsModel::roleNames() const
{
    return { { GroupNameRole, QByteArrayLiteral("groupName") },
             { ActionManagerRole, QByteArray("actionManager") },
             { ActionRole, QByteArray("qmlAction") } };
}

void ActionsModel::reload()
{
    this->beginResetModel();

    for (const Item &item : qAsConst(m_items)) {
        if (!item.actionManager.isNull())
            disconnect(item.actionManager, nullptr, this, nullptr);
    }
    m_items.clear();

    const QList<ActionManager *> sortedManagers =
            ::ActionManagerModel->sortedList(::ActionManagerModelSortFunction);

    for (ActionManager *actionManager : sortedManagers) {
        connect(actionManager, &ActionManager::modelReset, this,
                &ActionsModel::onActionManagerReset);
        connect(actionManager, &ActionManager::objectNameChanged, this,
                &ActionsModel::onActionManagerNameChanged);
        connect(actionManager, &ActionManager::titleChanged, this,
                &ActionsModel::onActionManagerNameChanged);
        connect(actionManager, &ActionManager::rowsAboutToBeRemoved, this,
                &ActionsModel::onActionManagerRowsRemoved);
        connect(actionManager, &ActionManager::rowsInserted, this,
                &ActionsModel::onActionManagerRowsInserted);

        const QList<QObject *> actions = actionManager->actions();
        for (QObject *action : actions) {
            m_items << Item({ actionManager, action });
        }
    }

    this->endResetModel();
}

void ActionsModel::onActionManagerReset()
{
    ActionManager *actionManager = qobject_cast<ActionManager *>(this->sender());
    if (actionManager == nullptr)
        return;

    const QPair<int, int> rowRange = this->findRowRange(actionManager);
    const int removeStart = rowRange.first, removeEnd = rowRange.second;
    if (removeStart < 0 || removeEnd < 0)
        return;

    this->beginRemoveRows(QModelIndex(), removeStart, removeEnd);
    for (int i = removeEnd; i >= removeStart; i--)
        m_items.removeAt(i);
    this->endRemoveRows();

    const QList<QObject *> actions = actionManager->actions();
    const int insertStart = removeStart, insertEnd = removeStart + actions.size() - 1;

    this->beginInsertRows(QModelIndex(), insertStart, insertEnd);
    for (int i = 0; i < actions.size(); i++)
        m_items.insert(insertStart + i, Item({ actionManager, actions.at(i) }));
    this->endInsertRows();
}

void ActionsModel::onActionManagerNameChanged()
{
    ActionManager *actionManager = qobject_cast<ActionManager *>(this->sender());
    if (actionManager == nullptr)
        return;

    const QPair<int, int> rowRange = this->findRowRange(actionManager);
    if (rowRange.first < 0 || rowRange.second < 0)
        return;

    const QModelIndex start = this->index(rowRange.first, 0);
    const QModelIndex end = this->index(rowRange.second, 0);
    emit dataChanged(start, end);
}

void ActionsModel::onActionManagerRowsRemoved(const QModelIndex &index, int start, int end)
{
    if (!index.isValid())
        return;

    ActionManager *actionManager = qobject_cast<ActionManager *>(this->sender());
    if (actionManager == nullptr)
        return;

    auto it = std::find_if(m_items.begin(), m_items.end(), [actionManager](const Item &item) {
        return item.actionManager == actionManager;
    });

    if (it == m_items.end())
        return;

    const int offset = std::distance(m_items.begin(), it);
    this->beginRemoveRows(QModelIndex(), start + offset, end + offset);
    for (int i = end + offset; i >= start + offset; i--)
        m_items.removeAt(i);
    this->endRemoveRows();
}

void ActionsModel::onActionManagerRowsInserted(const QModelIndex &index, int start, int end)
{
    if (!index.isValid())
        return;

    ActionManager *actionManager = qobject_cast<ActionManager *>(this->sender());
    if (actionManager == nullptr)
        return;

    auto it = std::find_if(m_items.begin(), m_items.end(), [actionManager](const Item &item) {
        return item.actionManager == actionManager;
    });

    if (it == m_items.end())
        return;

    const int offset = std::distance(m_items.begin(), it);
    this->beginInsertRows(QModelIndex(), start + offset, end + offset);
    for (int i = start + offset; i <= end + offset; i--) {
        QObject *action = actionManager->at(i - offset);
        m_items.insert(i, Item({ actionManager, action }));
    }
    this->endInsertRows();
}

void ActionsModel::onActionManagerModelReset()
{
    this->reload();
}

void ActionsModel::onActionManagerModelRowsRemoved(const QModelIndex &index, int start, int end)
{
    if (!index.isValid())
        return;

    for (int i = start; i <= end; i++) {
        QObject *actionManager = ::ActionManagerModel->objectAt(i);

        const QPair<int, int> rowRange = this->findRowRange(actionManager);
        if (rowRange.first < 0 || rowRange.second < 0)
            continue;

        this->beginRemoveRows(QModelIndex(), rowRange.first, rowRange.second);
        for (int r = rowRange.second; r >= rowRange.first; r--)
            m_items.removeAt(i);
        this->endRemoveRows();
    }
}

void ActionsModel::onActionManagerModelRowsInserted(const QModelIndex &index, int start, int end)
{
    if (!index.isValid())
        return;

    const QList<ActionManager *> sortedManagers =
            ::ActionManagerModel->sortedList(::ActionManagerModelSortFunction);

    for (int i = start; i <= end; i++) {
        ActionManager *actionManager = ::ActionManagerModel->at(i);

        const QList<QObject *> actions = actionManager->actions();
        const int managerIndex = sortedManagers.indexOf(actionManager);

        const int insertStart = managerIndex == 0
                ? 0
                : this->findRowRange(sortedManagers.at(managerIndex - 1)).second + 1;
        if (insertStart < 0) {
            this->reload();
            return;
        }

        const int insertEnd = insertStart + actions.size() - 1;

        this->beginInsertRows(QModelIndex(), insertStart, insertEnd);
        for (int i = insertStart; i <= insertEnd; i++) {
            m_items.insert(i, Item({ actionManager, actions.at(i - insertStart) }));
        }
        this->endInsertRows();
    }
}

QPair<int, int> ActionsModel::findRowRange(QObject *actionManager) const
{
    int removeStart = -1, removeEnd = -2;
    for (int i = 0; i < m_items.size(); i++) {
        const Item item = m_items.at(i);
        if (item.actionManager == actionManager) {
            if (removeStart < 0)
                removeStart = i;
            removeEnd = i;
        } else {
            if (removeEnd >= 0)
                break;
        }
    }

    return qMakePair(removeStart, removeEnd);
}
