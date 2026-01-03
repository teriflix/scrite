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
#include "utils.h"
#include "application.h"
#include "qobjectlistmodel.h"
#include "timeprofiler.h"

#include <QDir>
#include <QTimer>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QDynamicPropertyChangeEvent>

static const char *_QQuickAction = "QQuickAction";

static const QByteArray _QQuickActionSortOrderProperty = QByteArrayLiteral("sortOrder");
static const char *_QQuickActionSortOrderChanged = SIGNAL(sortOrderChanged());

static const QByteArray _QQuickActionTextProperty = QByteArrayLiteral("text");
static const char *_QQuickActionTextChanged = SIGNAL(textChanged(QString));

static const QByteArray _QQuickActionIconProperty = QByteArrayLiteral("icon");
static const char *_QQuickActionIconChanged = SIGNAL(iconChanged(QQuickIcon));

static const QByteArray _QQuickActionTooltipProperty = QByteArrayLiteral("tooltip");
static const char *_QQuickActionTooltipChanged = SIGNAL(tooltipChanged());

static const QByteArray _QQuickActionKeywordsProperty = QByteArrayLiteral("keywords");
static const char *_QQuickActionKeywordsChanged = SIGNAL(keywordsChanged());

static const QByteArray _QQuickActionVisibleProperty = QByteArrayLiteral("visible");
static const char *_QQuickActionVisibilityChanged = SIGNAL(visibleChanged());

static const QByteArray _QQuickActionShortcutProperty = QByteArrayLiteral("shortcut");
static const char *_QQuickActionShortcutChanged = SIGNAL(shortcutChanged(QKeySequence));

static const QByteArray _QQuickActionDefaultShortcutProperty = QByteArrayLiteral("defaultShortcut");

static const QByteArray _QQuickActionAllowShortcutProperty = QByteArrayLiteral("allowShortcut");

static const QByteArray _QQuickActionEnabledProperty = QByteArrayLiteral("enabled");
static const QByteArray _QQuickActionCheckableProperty = QByteArrayLiteral("checkable");

static const QByteArray _QQuickActionTriggerCount = QByteArrayLiteral("triggerCount");
static const char *_QQuickActionTriggerCountChanged = SIGNAL(triggerCountChanged());

static const QByteArray _QQuickActionTriggerMethod = QByteArrayLiteral("triggerMethod");

#ifdef DEBUG_SHORTCUT_DELIVERY
class ActionManagers : public QObjectListModel<ActionManager *>
{
public:
    explicit ActionManagers(QObject *parent = nullptr) : QObjectListModel<ActionManager *>(parent)
    {
        qApp->installEventFilter(this);
    }
    virtual ~ActionManagers() { }

    bool eventFilter(QObject *object, QEvent *event)
    {
        if (object == qApp->focusWindow() && event->type() == QEvent::ShortcutOverride) {
            QKeyEvent *keyEvent = reinterpret_cast<QKeyEvent *>(event);
            QKeySequence keySequence(keyEvent->modifiers() + keyEvent->key());
            QObject *qmlAction = ActionManager::findActionForShortcut(keySequence.toString());
            if (qmlAction)
                this->addToPending(qmlAction, keySequence);
        } else if (event->type() == QEvent::Shortcut) {
            QShortcutEvent *shortcutEvent = reinterpret_cast<QShortcutEvent *>(event);
            this->removeFromPending(object, shortcutEvent->key());
        }

        return false;
    }

    void timerEvent(QTimerEvent *te)
    {
        if (te->timerId() == m_processPendingShortcuts.timerId()) {
            m_processPendingShortcuts.stop();

            while (!m_pendingShortcuts.isEmpty()) {
                auto item = m_pendingShortcuts.takeFirst();
                if (item.first->property(_QQuickActionEnabledProperty).toBool()) {
                    if (item.first->property(_QQuickActionCheckableProperty).toBool()) {
                        QMetaObject::invokeMethod(item.first, "toggled", Qt::DirectConnection,
                                                  Q_ARG(QObject *, item.first));
                    }
                    QMetaObject::invokeMethod(item.first, "triggered", Qt::DirectConnection,
                                              Q_ARG(QObject *, item.first));
                }
            }
        }
    }

private:
    int findPending(QObject *object, const QKeySequence &sequence) const
    {
        if (m_pendingShortcuts.isEmpty() || object == nullptr || sequence.isEmpty())
            return -1;

        auto it = std::find_if(m_pendingShortcuts.begin(), m_pendingShortcuts.end(),
                               [=](const QPair<QObject *, QKeySequence> &item) {
                                   return (item.first == object && item.second == sequence);
                               });
        if (it != m_pendingShortcuts.end())
            return std::distance(m_pendingShortcuts.begin(), it);

        return -1;
    }

    void addToPending(QObject *object, const QKeySequence &sequence)
    {
        if (object == nullptr || sequence.isEmpty())
            return;

        if (this->findPending(object, sequence) < 0) {
            connect(object, &QObject::destroyed, this, &ActionManagers::objectDestroyed);
            m_pendingShortcuts.append({ object, sequence });
            m_processPendingShortcuts.start(m_delay, this);
        }
    }

    void removeFromPending(QObject *object, const QKeySequence &sequence)
    {
        if (object == nullptr || sequence.isEmpty())
            return;

        int index = this->findPending(object, sequence);
        if (index >= 0) {
            m_pendingShortcuts.removeAt(index);
            disconnect(object, &QObject::destroyed, this, &ActionManagers::objectDestroyed);
            if (m_pendingShortcuts.isEmpty())
                m_processPendingShortcuts.stop();
            else
                m_processPendingShortcuts.start(m_delay, this);
        }
    }

    void objectDestroyed(QObject *object)
    {
        if (object == nullptr)
            return;

        for (int i = m_pendingShortcuts.size() - 1; i >= 0; i--) {
            if (m_pendingShortcuts.at(i).first == object) {
                m_pendingShortcuts.removeAt(i);
                m_processPendingShortcuts.start(m_delay, this);
            }
        }
    }

private:
    const int m_delay = 100;
    QBasicTimer m_processPendingShortcuts;
    QList<QPair<QObject *, QKeySequence>> m_pendingShortcuts;
};
#else
typedef QObjectListModel<ActionManager *> ActionManagers;
#endif

Q_GLOBAL_STATIC(ActionManagers, ActionManagerModel)

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
    connect(this, &ActionManager::countChanged, this, &ActionManager::visibleActionsChanged);

    connect(this, &QObject::objectNameChanged, this, &ActionManager::scheduleApplySavedShortcuts);
}

ActionManager::~ActionManager()
{
    this->saveShortcutMap();

    ActionManagerModel->remove(this);
}

ActionManagerAttached *ActionManager::qmlAttachedProperties(QObject *object)
{
    return new ActionManagerAttached(object);
}

ActionManager *ActionManager::findManager(QObject *action)
{
    if (action == nullptr)
        return nullptr;

    const QList<ActionManager *> managers = ::ActionManagerModel->constList();
    for (ActionManager *manager : managers) {
        if (manager->actions().contains(action))
            return manager;
    }

    return nullptr;
}

bool ActionManager::canChangeActionShortcut(QObject *action)
{
    /*
     * Actions with editable shortcuts must meet the following criteria
     *
     * 1. They should have an objectName
     * 2. They should have a readonly string property by name defaultShortcut or
     *    a readonly shortcut that allows assigning of shortcuts.
     * 3. They must belong to an ActionManager
     */

    // 3rd Condition Check
    const ActionManager *manager = findManager(action);
    if (manager == nullptr)
        return false;

    // 1st Condition Check
    if (action->objectName().isEmpty())
        return false;

    // 2nd Condition Check
    const QMetaObject *mo = action->metaObject();
    const QMetaProperty defaultShortcutProperty =
            mo->property(mo->indexOfProperty(_QQuickActionDefaultShortcutProperty));
    if (!defaultShortcutProperty.isValid() || defaultShortcutProperty.isWritable()
        || defaultShortcutProperty.userType() != QMetaType::QString) {
        const QMetaProperty allowShortcutProperty =
                mo->property(mo->indexOfProperty(_QQuickActionAllowShortcutProperty));
        if (allowShortcutProperty.isValid() && !allowShortcutProperty.isWritable()
            && allowShortcutProperty.userType() == QMetaType::Bool)
            return allowShortcutProperty.read(action).toBool();

        return false;
    }

    return true;
}

bool ActionManager::changeActionShortcut(QObject *action, const QString &shortcut)
{
    if (shortcut.isEmpty())
        return false;

    if (!canChangeActionShortcut(action))
        return false;

    return action->setProperty(_QQuickActionShortcutProperty, shortcut);
}

QKeySequence ActionManager::defaultActionShortcut(QObject *action)
{
    if (!canChangeActionShortcut(action))
        return QKeySequence();

    const QVariant defaultShortcutValue = action->property(_QQuickActionDefaultShortcutProperty);
    if (defaultShortcutValue.isValid()) {
        const QKeySequence defaultShortcut = defaultShortcutValue.userType() == QMetaType::QString
                ? QKeySequence::fromString(defaultShortcutValue.toString())
                : defaultShortcutValue.value<QKeySequence>();
        return defaultShortcut;
    }

    return QKeySequence();
}

bool ActionManager::restoreActionShortcut(QObject *action)
{
    if (!canChangeActionShortcut(action))
        return false;

    const QKeySequence defaultShortcut = defaultActionShortcut(action);
    if (defaultShortcut.isEmpty()) {
        const QVariant currentShortcut = action->property(_QQuickActionShortcutProperty);
        if (currentShortcut.isValid() && !Utils::Gui::portableShortcut(currentShortcut).isEmpty()) {
            return action->setProperty(_QQuickActionShortcutProperty, QVariant());
        }
        return false;
    }

    const QString currentSequence = action->property(_QQuickActionShortcutProperty).toString();
    const QKeySequence currentShortcut = QKeySequence::fromString(currentSequence);
    if (currentShortcut == defaultShortcut)
        return false;

    return action->setProperty(_QQuickActionShortcutProperty, defaultShortcut.toString());
}

QObject *ActionManager::findActionForShortcut(const QString &shortcut)
{
    for (const ActionManager *manager : ::ActionManagerModel->constList()) {
        QObject *action = manager->findByShortcut(shortcut);
        if (action)
            return action;
    }

    return nullptr;
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

bool ActionManager::contains(QObject *action) const
{
    return m_actions.contains(action);
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

QObject *ActionManager::findByShortcut(const QString &shortcut) const
{
    auto it = std::find_if(m_actions.begin(), m_actions.end(), [shortcut](QObject *action) {
        const QString actionShortcut = action->property(_QQuickActionShortcutProperty).toString();
        return actionShortcut == shortcut;
    });
    if (it != m_actions.end())
        return *it;

    return nullptr;
}

QList<QObject *> ActionManager::visibleActions() const
{
    QList<QObject *> ret;

    std::copy_if(m_actions.begin(), m_actions.end(), std::back_inserter(ret), [](QObject *action) {
        const QMetaProperty visibleProperty = action->metaObject()->property(
                action->metaObject()->indexOfProperty(_QQuickActionVisibleProperty));
        if (visibleProperty.isValid() && visibleProperty.userType() == QMetaType::Bool) {
            const QVariant visibleFlag = action->property(_QQuickActionVisibleProperty);
            if (visibleFlag.isValid() && visibleFlag.userType() == QMetaType::Bool)
                return visibleFlag.toBool();
        }
        return true;
    });

    return ret;
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
                && visibleProperty.hasNotifySignal()) {
                connect(action, _QQuickActionVisibilityChanged, this, SLOT(onActionDataChanged()));
                connect(action, _QQuickActionVisibilityChanged, this,
                        SIGNAL(visibleActionsChanged()));
            }
        }

        const QMetaProperty tooltipProperty = action->metaObject()->property(
                action->metaObject()->indexOfProperty(_QQuickActionTooltipProperty));
        if (tooltipProperty.isValid()) {
            if (tooltipProperty.isWritable() && !tooltipProperty.isConstant()
                && tooltipProperty.hasNotifySignal())
                connect(action, _QQuickActionTooltipChanged, this, SLOT(onActionDataChanged()));
        }

        const QMetaProperty keywordsProperty = action->metaObject()->property(
                action->metaObject()->indexOfProperty(_QQuickActionKeywordsProperty));
        if (keywordsProperty.isValid()) {
            if (keywordsProperty.isWritable() && !keywordsProperty.isConstant()
                && keywordsProperty.hasNotifySignal())
                connect(action, _QQuickActionKeywordsChanged, this, SLOT(onActionDataChanged()));
        }

        connect(action, _QQuickActionTextChanged, this, SLOT(onActionDataChanged()));
        connect(action, _QQuickActionIconChanged, this, SLOT(onActionDataChanged()));
        connect(action, _QQuickActionShortcutChanged, this,
                SLOT(onActionShortcutChanged(QKeySequence)));

        QList<QObject *> actions = m_actions;
        actions.append(action);
        sortActions(actions);

        int insertIndex = actions.indexOf(action);

        this->beginInsertRows(QModelIndex(), insertIndex, insertIndex);
        m_actions = actions;
        this->endInsertRows();

        this->scheduleApplySavedShortcuts();

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

void ActionManager::onActionShortcutChanged(const QKeySequence &newShortcut)
{
    QObject *action = this->sender();

    const int row = m_actions.indexOf(action);
    if (row < 0)
        return;

    if (!ActionManager::canChangeActionShortcut(action))
        return;

    if (m_shortcutMap.value(action->objectName()).toString() == newShortcut.toString())
        return;

    const QKeySequence defaultShortcut = defaultActionShortcut(action);
    if (defaultShortcut == newShortcut)
        m_shortcutMap.remove(action->objectName());
    else
        m_shortcutMap[action->objectName()] = newShortcut.toString();
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

void ActionManager::saveShortcutMap()
{
    const QString mapFilePath = this->shortcutMapFilePath();
    if (mapFilePath.isEmpty())
        return;

    if (m_shortcutMap.isEmpty()) {
        QFile::remove(mapFilePath);
        return;
    }

    QFile mapFile(mapFilePath);
    if (!mapFile.open(QFile::WriteOnly))
        return;

    const QJsonObject shortcuts = QJsonObject::fromVariantMap(m_shortcutMap);
    mapFile.write(QJsonDocument(shortcuts).toJson(QJsonDocument::Indented));
}

void ActionManager::applySavedShortcuts()
{
    const QVariantMap shortcutMap = [=]() {
        QVariantMap theMap;

        const QString mapFilePath = this->shortcutMapFilePath();
        if (mapFilePath.isEmpty())
            return theMap;

        QFile mapFile(mapFilePath);
        if (!mapFile.open(QFile::ReadOnly))
            return theMap;

        const QJsonDocument jsonDoc = QJsonDocument::fromJson(mapFile.readAll());
        const QJsonObject shortcuts = jsonDoc.object();
        if (shortcuts.isEmpty())
            return theMap;

        return shortcuts.toVariantMap();
    }();

    if (shortcutMap.isEmpty() || m_actions.isEmpty())
        return;

    for (QObject *action : qAsConst(m_actions)) {
        if (shortcutMap.contains(action->objectName()))
            changeActionShortcut(action, shortcutMap[action->objectName()].toString());
    }
}

void ActionManager::scheduleApplySavedShortcuts()
{
    if (m_applySavedShortcutsTimer.isNull()) {
        m_applySavedShortcutsTimer = new QTimer(this);
        m_applySavedShortcutsTimer->setInterval(500);
        m_applySavedShortcutsTimer->setSingleShot(true);
        connect(m_applySavedShortcutsTimer, &QTimer::timeout, this,
                &ActionManager::applySavedShortcuts);
        connect(m_applySavedShortcutsTimer, &QTimer::timeout, m_applySavedShortcutsTimer,
                &QObject::deleteLater);
    }

    m_applySavedShortcutsTimer->start();
}

QString ActionManager::shortcutMapFilePath() const
{
    if (this->objectName().isEmpty())
        return QString();

    return Utils::Platform::configPath("shortcuts/" + this->objectName() + ".json");
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

void ActionManager::onActionDataChanged()
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
    if (parent && parent->inherits(_QQuickAction))
        m_action = parent;
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

ActionManager *ActionManagerAttached::find(const QString &name) const
{
    return ::findActionManager(name);
}

QObject *ActionManagerAttached::findActionForShortcut(const QString &shortcut) const
{
    return ActionManager::findActionForShortcut(shortcut);
}

///////////////////////////////////////////////////////////////////////////////

ActionHandler::ActionHandler(QQuickItem *parent) : QQuickItem(parent)
{
    this->setFlag(QQuickItem::ItemHasContents, false);

    connect(this, &QQuickItem::enabledChanged, this, &ActionHandler::checkTriggerCount);

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

void ActionHandler::setChecked(bool val)
{
    if (m_checked == val)
        return;

    m_checked = val;
    emit checkedChanged();
}

void ActionHandler::setDown(bool val)
{
    if (m_down == val)
        return;

    m_down = val;
    emit downChanged();
}

void ActionHandler::setIconSource(const QString &val)
{
    if (m_iconSource == val)
        return;

    m_iconSource = val;
    emit iconSourceChanged();
}

void ActionHandler::setTooltip(const QString &val)
{
    if (m_tooltip == val)
        return;

    m_tooltip = val;
    emit tooltipChanged();
}

void ActionHandler::setAction(QObject *val)
{
    if (m_action == val)
        return;

    if (m_action != nullptr) {
        emit actionAboutToChange();
        disconnect(m_action, nullptr, this, nullptr);
    }

    m_actionTriggerMethodProperty = QMetaProperty();
    m_action = val && val->inherits(_QQuickAction) ? val : nullptr;

    if (m_action != nullptr) {
        connect(m_action, &QObject::destroyed, this, &ActionHandler::onObjectDestroyed);

        connect(m_action, SIGNAL(toggled(QObject *)), this, SLOT(onToggled(QObject *)));
        connect(m_action, SIGNAL(triggered(QObject *)), this, SLOT(onTriggered(QObject *)));

        const QMetaObject *mo = m_action->metaObject();
        const QMetaProperty triggerCountProp =
                mo->property(mo->indexOfProperty(_QQuickActionTriggerCount));
        if (triggerCountProp.isValid() && triggerCountProp.userType() == QMetaType::Int)
            connect(m_action, _QQuickActionTriggerCountChanged, this, SLOT(checkTriggerCount()));

        const QMetaProperty triggerMethodProp =
                mo->property(mo->indexOfProperty(_QQuickActionTriggerMethod));
        if (triggerMethodProp.isValid() && triggerMethodProp.userType() == QMetaType::Int) {
            m_actionTriggerMethodProperty = triggerMethodProp;
        }
    }

    emit actionChanged();
}

QObject *ActionHandler::findAction(const QString &managerName, const QString &actionName) const
{
    ActionManager *actionManager = ::findActionManager(managerName);
    return actionManager ? actionManager->find(actionName) : nullptr;
}

void ActionHandler::componentComplete()
{
    QQuickItem::componentComplete();

    if (m_action != nullptr) {
        const QMetaObject *mo = m_action->metaObject();
        const QMetaProperty triggerCountProp =
                mo->property(mo->indexOfProperty(_QQuickActionTriggerCount));
        if (triggerCountProp.isValid() && triggerCountProp.userType() == QMetaType::Int)
            checkTriggerCount();
    }
}

void ActionHandler::onToggled(QObject *source)
{
    if (this->isEnabled()) {
        switch (triggerMethod()) {
        case TriggerNone:
            return;
        case TriggerFirst:
            if (ActionHandlers::instance()->findFirst(m_action) != this)
                return;
        default:
            break;
        }

        emit toggled(source);
    }
}

void ActionHandler::onTriggered(QObject *source)
{
    if (this->isEnabled()) {
        switch (triggerMethod()) {
        case TriggerNone:
            return;
        case TriggerFirst:
            if (ActionHandlers::instance()->findFirst(m_action) != this)
                return;
        default:
            break;
        }

        emit triggered(source);
    }
}

void ActionHandler::onObjectDestroyed(QObject *ptr)
{
    if (m_action == ptr && m_action != nullptr) {
        emit actionAboutToChange();
        m_action = nullptr;
        emit actionChanged();
    }
}

void ActionHandler::checkTriggerCount()
{
    if (m_action != nullptr && this->isComponentComplete() && this->isEnabled()) {
        const QVariant tc = m_action->property(_QQuickActionTriggerCount);
        if (tc.userType() == QMetaType::Int && tc.toInt() > 0) {
            emit triggerCountChanged(tc.toInt());
            m_action->setProperty(_QQuickActionTriggerCount, QVariant::fromValue<int>(0));
        }
    }
}

ActionHandler::TriggerMethod ActionHandler::triggerMethod() const
{
    if (m_action != nullptr && m_actionTriggerMethodProperty.isValid()) {
        const int propValue = m_actionTriggerMethodProperty.read(m_action).toInt();
        static const QMetaEnum triggerMethodEnum =
                QMetaEnum::fromType<ActionHandler::TriggerMethod>();
        if (triggerMethodEnum.value(propValue))
            return TriggerMethod(propValue);
    }

    return TriggerAll;
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

ActionHandler *ActionHandlerAttached::active() const
{
    return ActionHandlers::instance()->findFirst(m_action, true);
}

QList<ActionHandler *> ActionHandlerAttached::all() const
{
    return ActionHandlers::instance()->findAll(m_action, true);
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

void ActionHandlerAttached::onHandlerAvailabilityChanged(QObject *action, ActionHandler *handler)
{
    Q_UNUSED(handler);

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
                                       && (!enabledOnly || handler->isEnabled());
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
                     return handler->action() == object && (!enabledOnly || handler->isEnabled());
                 });

    return ret;
}

void ActionHandlers::add(ActionHandler *handler)
{
    if (m_appAboutToQuit)
        return;

    if (handler && !m_actionHandlers.contains(handler)) {
        m_actionHandlers.append(handler);
        sortHandlersByPriority();

        if (handler->action())
            emit handlerAvailabilityChanged(handler->action(), handler);
        emit handlerCheckedChanged(handler->action(), handler);
        emit handlerDownChanged(handler->action(), handler);

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
        connect(handler, &ActionHandler::checkedChanged, this,
                &ActionHandlers::onHandlerCheckedChanged);
        connect(handler, &ActionHandler::downChanged, this, &ActionHandlers::onHandlerDownChanged);
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
            emit handlerAvailabilityChanged(handler->action(), handler);
    }
}

void ActionHandlers::sortHandlersByPriority()
{
    if (m_appAboutToQuit)
        return;

    std::sort(m_actionHandlers.begin(), m_actionHandlers.end(),
              [](ActionHandler *a, ActionHandler *b) { return b->priority() - a->priority() < 0; });
}

void ActionHandlers::notifyHandlerAvailability()
{
    if (m_appAboutToQuit)
        return;

    ActionHandler *handler = qobject_cast<ActionHandler *>(this->sender());
    if (handler && handler->action())
        emit handlerAvailabilityChanged(handler->action(), handler);
}

void ActionHandlers::onHandlerPriorityChanged()
{
    this->sortHandlersByPriority();
}

void ActionHandlers::onHandlerCheckedChanged()
{
    if (m_appAboutToQuit)
        return;

    ActionHandler *handler = qobject_cast<ActionHandler *>(this->sender());
    if (handler && handler->action())
        emit handlerCheckedChanged(handler->action(), handler);
}

void ActionHandlers::onHandlerDownChanged()
{
    if (m_appAboutToQuit)
        return;

    ActionHandler *handler = qobject_cast<ActionHandler *>(this->sender());
    if (handler && handler->action())
        emit handlerCheckedChanged(handler->action(), handler);
}

void ActionHandlers::onHanlderActionAboutToChange()
{
    if (m_appAboutToQuit)
        return;

    ActionHandler *handler = qobject_cast<ActionHandler *>(this->sender());
    if (handler && handler->action()) {
        QObject *oldAction = handler->action();

        QMetaObject::invokeMethod(this, "handlerAvailabilityChanged", Qt::QueuedConnection,
                                  Q_ARG(QObject *, oldAction), Q_ARG(ActionHandler *, nullptr));
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
            &ActionsModel::reloadLater);
    connect(::ActionManagerModel, &QAbstractListModel::rowsRemoved, this,
            &ActionsModel::reloadLater);
    connect(::ActionManagerModel, &QAbstractListModel::rowsInserted, this,
            &ActionsModel::reloadLater);
}

ActionsModel::~ActionsModel() { }

void ActionsModel::setActionManagers(const QList<ActionManager *> &val)
{
    if (m_actionManagers == val)
        return;

    disconnect(::ActionManagerModel, &QAbstractListModel::modelReset, this,
               &ActionsModel::reloadLater);
    disconnect(::ActionManagerModel, &QAbstractListModel::rowsRemoved, this,
               &ActionsModel::reloadLater);
    disconnect(::ActionManagerModel, &QAbstractListModel::rowsInserted, this,
               &ActionsModel::reloadLater);

    m_actionManagers = val;

    if (m_actionManagers.isEmpty()) {
        connect(::ActionManagerModel, &QAbstractListModel::modelReset, this,
                &ActionsModel::reloadLater);
        connect(::ActionManagerModel, &QAbstractListModel::rowsRemoved, this,
                &ActionsModel::reloadLater);
        connect(::ActionManagerModel, &QAbstractListModel::rowsInserted, this,
                &ActionsModel::reloadLater);
    }

    emit actionManagersChanged();

    this->reloadLater();
}

QString ActionsModel::groupNameAt(int row) const
{
    const QVariant data = this->data(this->index(row, 0), GroupNameRole);
    return data.toString();
}

ActionManager *ActionsModel::actionManagerAt(int row) const
{
    const QVariant data = this->data(this->index(row, 0), ActionManagerRole);
    return qobject_cast<ActionManager *>(data.value<QObject *>());
}

QObject *ActionsModel::actionAt(int row) const
{
    const QVariant data = this->data(this->index(row, 0), ActionRole);
    return data.value<QObject *>();
}

int ActionsModel::indexOfAction(QObject *action) const
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
                           [action](const Item &item) { return item.action == action; });

    if (it != m_items.end())
        return std::distance(m_items.begin(), it);

    return -1;
}

QObject *ActionsModel::findActionForShortcut(const QString &shortcut) const
{
    return ActionManager::findActionForShortcut(shortcut);
}

bool ActionsModel::restoreActionShortcut(QObject *action) const
{
    return ActionManager::restoreActionShortcut(action);
}

bool ActionsModel::isActionShortcutEditable(QObject *action) const
{
    return ActionManager::canChangeActionShortcut(action);
}

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
        return qobject_cast<ActionManager *>(m_items[index.row()].actionManager)->title();
    case ActionManagerRole:
        return QVariant::fromValue<QObject *>(m_items[index.row()].actionManager);
    case ActionRole:
        return QVariant::fromValue<QObject *>(m_items[index.row()].action);
    case ShortcutIsEditableRole:
        return this->isActionShortcutEditable(m_items[index.row()].action);
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> ActionsModel::roleNames() const
{
    return { { GroupNameRole, QByteArrayLiteral("groupName") },
             { ActionManagerRole, QByteArray("actionManager") },
             { ActionRole, QByteArray("qmlAction") },
             { ShortcutIsEditableRole, QByteArray("shortcutIsEditable") } };
}

void ActionsModel::clear()
{
    if (m_items.isEmpty())
        return;

    this->beginResetModel();

    QSet<QObject *> actionManagers;

    for (const Item &item : qAsConst(m_items)) {
        if (!item.actionManager.isNull())
            actionManagers.insert(item.actionManager);
    }

    for (QObject *actionManager : actionManagers)
        actionManager->disconnect(this);

    m_items.clear();

    this->endResetModel();
}

void ActionsModel::reload()
{
    this->clear();

    this->beginResetModel();

    const QList<ActionManager *> sortedManagers = m_actionManagers.isEmpty()
            ? ::ActionManagerModel->sortedList(::ActionManagerModelSortFunction)
            : m_actionManagers;

    for (ActionManager *actionManager : sortedManagers) {
        connect(actionManager, &ActionManager::objectNameChanged, this,
                &ActionsModel::onActionManagerNameChanged);
        connect(actionManager, &ActionManager::titleChanged, this,
                &ActionsModel::onActionManagerNameChanged);
        connect(actionManager, &ActionManager::dataChanged, this,
                &ActionsModel::onActionManagerDataChanged);

        connect(actionManager, &ActionManager::modelReset, this, &ActionsModel::reloadLater);
        connect(actionManager, &ActionManager::rowsAboutToBeRemoved, this,
                &ActionsModel::reloadLater);
        connect(actionManager, &ActionManager::rowsInserted, this, &ActionsModel::reloadLater);

        const QList<QObject *> actions = actionManager->actions();
        for (QObject *action : actions) {
            m_items << Item({ actionManager, action });
        }
    }

    this->endResetModel();
}

void ActionsModel::reloadLater()
{
    if (m_reloadTimer == nullptr) {
        m_reloadTimer = new QTimer(this);
        m_reloadTimer->setInterval(500);
        m_reloadTimer->setSingleShot(true);
        connect(m_reloadTimer, &QTimer::timeout, this, &ActionsModel::reload);
    }

    m_reloadTimer->start();
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

void ActionsModel::onActionManagerDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    if (!start.isValid() || !end.isValid())
        return;

    ActionManager *actionManager = qobject_cast<ActionManager *>(this->sender());
    if (actionManager == nullptr)
        return;

    // Ensure that all the changed actions actually exist
    for (int i = start.row(); i <= end.row(); i++) {
        QObject *changedAction = start.data(ActionManager::ActionRole).value<QObject *>();
        if (changedAction) {
            auto it = std::find_if(m_items.begin(), m_items.end(),
                                   [actionManager, changedAction](const Item &item) {
                                       return item.actionManager == actionManager
                                               && item.action == changedAction;
                                   });
            if (it == m_items.end()) {
                this->reloadLater();
                return;
            }
        }
    }

    auto it = std::find_if(m_items.begin(), m_items.end(), [actionManager](const Item &item) {
        return item.actionManager == actionManager;
    });

    if (it == m_items.end()) {
        /** we could have inserted actions from this action-manager at the end, but then
         *  we would have a hard time figuring out the insert indexes based on the sort
         *  order. Its better off to simply reload the whole thing in such a case */
        this->reloadLater();
        return;
    }

    const int offset = std::distance(m_items.begin(), it);
    const QModelIndex start2 = this->index(start.row() + offset);
    const QModelIndex end2 = this->index(end.row() + offset);
    emit dataChanged(start2, end2);
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

///////////////////////////////////////////////////////////////////////////////

ActionsModelFilter::ActionsModelFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
    this->setDynamicSortFilter(true);
}

ActionsModelFilter::~ActionsModelFilter() { }

void ActionsModelFilter::setSourceModel(QAbstractItemModel *model)
{
    if (model->metaObject()->inherits(&ActionsModel::staticMetaObject))
        QSortFilterProxyModel::setSourceModel(model);
}

void ActionsModelFilter::setActionManagerTitle(const QString &val)
{
    if (m_actionManagerTitleFilter == val)
        return;

    m_actionManagerTitleFilter = val;
    emit actionManagerTitleChanged();

    this->invalidateFilter();
}

void ActionsModelFilter::setActionText(const QString &val)
{
    if (m_actionTextFilter == val)
        return;

    m_actionTextFilter = val;
    emit actionTextChanged();

    this->invalidateFilter();
}

void ActionsModelFilter::setFilters(Filters val)
{
    if (m_filters == val)
        return;

    m_filters = val;
    emit filtersChanged();

    this->invalidateFilter();
}

void ActionsModelFilter::setCustomFilterMode(bool val)
{
    if (m_customFilterMode == val)
        return;

    m_customFilterMode = val;
    emit customFilterModeChanged();
}

QObject *ActionsModelFilter::findActionForShortcut(const QString &shortcut) const
{
    // NOTE: This function returns from the source model, which means the returned action
    // may not be present as a part of the filtered set returned by this model.
    ActionsModel *srcModel = qobject_cast<ActionsModel *>(this->sourceModel());
    if (srcModel)
        return srcModel->findActionForShortcut(shortcut);

    return nullptr;
}

bool ActionsModelFilter::restoreActionShortcut(QObject *action)
{
    // NOTE: This works only for actions avaialable as filtered through this model
    ActionsModel *srcModel = qobject_cast<ActionsModel *>(this->sourceModel());
    if (srcModel) {
        const int row = srcModel->indexOfAction(action);
        if (row < 0)
            return false;

        const QModelIndex index = this->mapFromSource(srcModel->index(row));
        if (index.isValid()) {
            bool success = srcModel->restoreActionShortcut(action);
            if (success) {
                emit dataChanged(index, index);
                emit actionShortcutRestored(action);
            }
            return success;
        }
    }

    return false;
}

int ActionsModelFilter::restoreAllActionShortcuts()
{
    int restoreCount = 0;

    // NOTE: This works only for actions avaialable as filtered through this model
    for (int i = 0; i < this->rowCount(); i++) {
        const QModelIndex index = this->index(i, 0);
        QObject *action = index.data(ActionsModel::ActionRole).value<QObject *>();
        if (this->restoreActionShortcut(action))
            ++restoreCount;
    }

    return restoreCount;
}

void ActionsModelFilter::filter()
{
    this->invalidateFilter();
}

ActionManager *ActionsModelFilter::actionManagerOf(QObject *action) const
{
    return ActionManager::findManager(action);
}

void ActionsModelFilter::componentComplete()
{
    if (this->sourceModel() == nullptr)
        this->setSourceModel(new ActionsModel(this));
}

bool ActionsModelFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent.isValid() || m_filters == NoActions)
        return false;

    if (m_filters == AllActions)
        return true;

    const ActionsModel *sourceActionModel = qobject_cast<ActionsModel *>(this->sourceModel());
    if (sourceActionModel == nullptr)
        return false;

    ActionManager *manager = sourceActionModel->actionManagerAt(source_row);
    QObject *action = sourceActionModel->actionAt(source_row);
    if (manager == nullptr || action == nullptr)
        return false;

    bool accept = true;

    if (accept && m_filters.testFlag(HideCommandCenterActions)) {
        const QMetaProperty hideInCmdCenter = action->metaObject()->property(
                action->metaObject()->indexOfProperty("hideInCommandCenter"));
        if (hideInCmdCenter.isValid() && !hideInCmdCenter.isWritable()
            && hideInCmdCenter.userType() == QMetaType::Bool) {
            const bool hide = hideInCmdCenter.read(action).toBool();
            if (hide)
                accept = false;
        }
    }

    if (accept && m_filters.testFlag(ActionsWithText)) {
        const QString actionText = action->property(_QQuickActionTextProperty).toString();
        accept &= !actionText.isEmpty();

        if (accept) {
            const QString actionManagerTitle = manager->title();
            bool textAccept = m_actionManagerTitleFilter.isEmpty()
                    ? true
                    : actionManagerTitle.contains(m_actionManagerTitleFilter, Qt::CaseInsensitive);
            textAccept &= m_actionTextFilter.isEmpty()
                    ? true
                    : actionText.contains(m_actionTextFilter, Qt::CaseInsensitive);
            accept &= textAccept;
        }
    }

    if (accept && m_filters.testFlag(ActionsWithShortcut)) {
        const QKeySequence shortcut =
                action->property(_QQuickActionShortcutProperty).value<QKeySequence>();
        accept &= !shortcut.isEmpty();
    }

    if (accept && m_filters.testFlag(ActionsWithDefaultShortcut)) {
        const QKeySequence shortcut =
                action->property(_QQuickActionDefaultShortcutProperty).value<QKeySequence>();
        accept &= !shortcut.isEmpty();
    }

    if (accept & m_filters.testFlag(ActionsWithObjectName)) {
        accept &= !action->objectName().isEmpty();
    }

    if (accept & m_filters.testFlag(VisibleActions)) {
        const QMetaProperty visibleProperty = action->metaObject()->property(
                action->metaObject()->indexOfProperty(_QQuickActionVisibleProperty));
        if (visibleProperty.isValid() && visibleProperty.userType() == QMetaType::Bool) {
            const QVariant visibleFlag = action->property(_QQuickActionVisibleProperty);
            if (visibleFlag.isValid() && visibleFlag.userType() == QMetaType::Bool)
                accept &= visibleFlag.toBool();
        } else
            accept &= true;
    }

    if (accept & m_filters.testFlag(EnabledActions)) {
        const QVariant enabledFlag = action->property(_QQuickActionEnabledProperty);
        accept &= enabledFlag.toBool();
    }

    if (accept && m_customFilterMode) {
        BooleanResult result;
        emit const_cast<ActionsModelFilter *>(this)->filterRequest(action, manager, &result);
        accept = result.value();
    }

    return accept;
}
