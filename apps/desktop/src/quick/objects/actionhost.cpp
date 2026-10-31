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

#include "actionhost.h"

#include <QSet>

typedef QHash<QString, QByteArray> ActionHostContextClassMap;

Q_GLOBAL_STATIC(QList<AbstractActionHost *>, ActionHosts)
Q_GLOBAL_STATIC(QList<AbstractActionGroup *>, ActionGroups)
Q_GLOBAL_STATIC(ActionHostContextClassMap, ActionHostContextClasses)
Q_GLOBAL_STATIC(QSet<QString>, ActionHostPendingHostUris)

/**
 * Keeps track of every group and host, and works out which groups are merged into which, for each
 * hostUri (of menu groups, and of toolbar groups).
 */
class ActionHostRegistry
{
public:
    static QString hostUriKey(AbstractActionGroup::Kind kind, const QString &hostUri)
    {
        return QString::number(int(kind)) + QStringLiteral(":") + hostUri;
    }

    static void add(AbstractActionGroup *group) { ::ActionGroups->append(group); }

    static void remove(AbstractActionGroup *group)
    {
        if (::ActionGroups.isDestroyed())
            return;

        ::ActionGroups->removeOne(group);
        resolve(group->kind(), group->hostUri());
    }

    static void add(AbstractActionHost *host) { ::ActionHosts->append(host); }

    static void remove(AbstractActionHost *host)
    {
        if (::ActionHosts.isDestroyed())
            return;

        ::ActionHosts->removeOne(host);

        if (!::ActionHostContextClasses.isDestroyed()) {
            const QString key = hostUriKey(host->kind(), host->uri());
            if (hostsOf(host->kind(), host->uri()).isEmpty())
                ::ActionHostContextClasses->remove(key);
        }
    }

    static QList<AbstractActionHost *> hostsOf(AbstractActionGroup::Kind kind, const QString &uri)
    {
        QList<AbstractActionHost *> ret;
        if (::ActionHosts.isDestroyed() || uri.isEmpty())
            return ret;

        for (AbstractActionHost *host : std::as_const(*::ActionHosts)) {
            if (host->kind() == kind && host->uri() == uri)
                ret.append(host);
        }
        return ret;
    }

    // The context kind of a hostUri is that of the first host with that uri that has a context.
    static void checkContext(AbstractActionHost *host)
    {
        if (host->uri().isEmpty() || host->context() == nullptr)
            return;

        const QString key = hostUriKey(host->kind(), host->uri());
        const QByteArray kind = host->context()->kind().toUtf8();
        const QByteArray known = ::ActionHostContextClasses->value(key);
        if (known.isEmpty())
            ::ActionHostContextClasses->insert(key, kind);
        else if (known != kind)
            qWarning("ActionHost: %s expects a context of kind %s, not %s.",
                     qPrintable(host->uri()), known.constData(), kind.constData());
    }

    static void scheduleResolve(AbstractActionGroup::Kind kind, const QString &hostUri)
    {
        if (hostUri.isEmpty() || ::ActionHostPendingHostUris.isDestroyed())
            return;

        const bool wasEmpty = ::ActionHostPendingHostUris->isEmpty();
        ::ActionHostPendingHostUris->insert(hostUriKey(kind, hostUri));
        if (wasEmpty)
            QTimer::singleShot(0, []() { flushPendingHostUris(); });
    }

    static void flushPendingHostUris()
    {
        if (::ActionHostPendingHostUris.isDestroyed())
            return;

        const QSet<QString> keys = *::ActionHostPendingHostUris;
        ::ActionHostPendingHostUris->clear();
        for (const QString &key : keys) {
            const int colon = key.indexOf(QStringLiteral(":"));
            resolve(AbstractActionGroup::Kind(key.left(colon).toInt()), key.mid(colon + 1));
        }
    }

    // Selection sort that picks the first of equals, so that groups of the same order stay in the
    // sequence in which they were made. std::sort is not used on QLists.
    static QList<AbstractActionGroup *> sortedByOrder(QList<AbstractActionGroup *> groups)
    {
        QList<AbstractActionGroup *> ret;
        ret.reserve(groups.size());
        while (!groups.isEmpty()) {
            int minIndex = 0;
            for (int i = 1; i < groups.size(); i++) {
                if (groups.at(i)->order() < groups.at(minIndex)->order())
                    minIndex = i;
            }
            ret.append(groups.takeAt(minIndex));
        }
        return ret;
    }

    static AbstractActionGroup *findByUri(const QList<AbstractActionGroup *> &groups,
                                          const QString &uri)
    {
        if (uri.isEmpty())
            return nullptr;

        for (AbstractActionGroup *group : groups) {
            if (group->uri() == uri)
                return group;
        }
        return nullptr;
    }

    static void resolve(AbstractActionGroup::Kind kind, const QString &hostUri)
    {
        if (hostUri.isEmpty() || ::ActionGroups.isDestroyed())
            return;

        QList<AbstractActionGroup *> groups;
        QList<AbstractActionGroup *> others; // Same kind, with other host uris
        for (AbstractActionGroup *group : std::as_const(*::ActionGroups)) {
            if (group->kind() != kind)
                continue;
            if (group->hostUri() == hostUri)
                groups.append(group);
            else
                others.append(group);
        }

        QHash<AbstractActionGroup *, AbstractActionGroup *> parents;
        QHash<AbstractActionGroup *, QList<AbstractActionGroup *>> merged;

        for (AbstractActionGroup *group : groups) {
            if (!group->uri().isEmpty()) {
                for (AbstractActionGroup *other : groups) {
                    if (other != group && other->uri() == group->uri()) {
                        qWarning("ActionHost: uri %s is used by more than one group in %s.",
                                 qPrintable(group->uri()), qPrintable(hostUri));
                        break;
                    }
                }
            }

            const QString target = group->mergeIntoUri();
            if (target.isEmpty())
                continue;

            if (target == group->uri()) {
                qWarning("ActionHost: group %s in %s cannot merge into itself.",
                         qPrintable(target), qPrintable(hostUri));
                continue;
            }

            AbstractActionGroup *targetGroup = findByUri(groups, target);
            if (targetGroup == nullptr) {
                if (findByUri(others, target) != nullptr)
                    qWarning("ActionHost: %s cannot merge into %s, which has another host uri.",
                             qPrintable(hostUri), qPrintable(target));
                continue; // Falls back to the top level of the hostUri.
            }

            bool cycle = false;
            AbstractActionGroup *ancestor = targetGroup;
            for (int steps = 0; ancestor != nullptr && steps <= groups.size(); steps++) {
                if (ancestor == group) {
                    cycle = true;
                    break;
                }
                ancestor = findByUri(groups, ancestor->mergeIntoUri());
            }
            if (cycle) {
                qWarning("ActionHost: merging %s into %s makes a cycle in %s.",
                         qPrintable(group->uri()), qPrintable(target), qPrintable(hostUri));
                continue;
            }

            parents.insert(group, targetGroup);
            if (group->isVisible())
                merged[targetGroup].append(group);
        }

        for (AbstractActionGroup *group : groups) {
            group->m_mergeParent = parents.value(group, nullptr);

            const QList<AbstractActionGroup *> list = sortedByOrder(merged.value(group));
            if (list != group->m_merged) {
                group->m_merged = list;
                emit group->mergedChanged();
            }
        }

        const QList<AbstractActionHost *> hosts = hostsOf(kind, hostUri);
        for (AbstractActionHost *host : hosts)
            host->reload();
    }

    static QList<AbstractActionGroup *> topLevelGroups(AbstractActionGroup::Kind kind,
                                                       const QString &hostUri)
    {
        QList<AbstractActionGroup *> ret;
        if (hostUri.isEmpty() || ::ActionGroups.isDestroyed())
            return ret;

        for (AbstractActionGroup *group : std::as_const(*::ActionGroups)) {
            if (group->kind() == kind && group->hostUri() == hostUri && group->isVisible()
                && group->mergeParent() == nullptr)
                ret.append(group);
        }
        return sortedByOrder(ret);
    }
};

///////////////////////////////////////////////////////////////////////////////

AbstractActionContext::AbstractActionContext(const QString &kind, QObject *parent)
    : QObject(parent), m_kind(kind)
{
}

AbstractActionContext::~AbstractActionContext() { }

///////////////////////////////////////////////////////////////////////////////

AbstractActionGroup::AbstractActionGroup(Kind kind, QObject *parent)
    : QObject(parent), m_kind(kind)
{
    ActionHostRegistry::add(this);
}

AbstractActionGroup::~AbstractActionGroup()
{
    ActionHostRegistry::remove(this);
}

void AbstractActionGroup::setUri(const QString &val)
{
    if (!m_uri.isEmpty()) {
        qWarning("ActionGroup: uri can only be set once in %s", qPrintable(m_uri));
        return;
    }

    const QString val2 = val.trimmed();
    if (val2.isEmpty())
        return;

    m_uri = val2;
    emit uriChanged();

    scheduleResolve();
}

void AbstractActionGroup::setHostUri(const QString &val)
{
    // Slot names are set-once
    const QString val2 = val.trimmed();
    if (!m_hostUri.isEmpty() || val2.isEmpty())
        return;

    m_hostUri = val2;
    emit hostUriChanged();

    scheduleResolve();
}

void AbstractActionGroup::setMergeIntoUri(const QString &val)
{
    const QString val2 = val.trimmed();
    if (m_mergeIntoUri == val2)
        return;

    m_mergeIntoUri = val2;
    emit mergeIntoUriChanged();

    scheduleResolve();
}

void AbstractActionGroup::setTitle(const QString &val)
{
    if (m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

QString AbstractActionGroup::title() const
{
    if (!m_title.isEmpty() || m_actionManager == nullptr)
        return m_title;

    return m_actionManager->title();
}

void AbstractActionGroup::setIconSource(const QString &val)
{
    if (m_iconSource == val)
        return;

    m_iconSource = val;
    emit iconSourceChanged();
}

QString AbstractActionGroup::iconSource() const
{
    if (!m_iconSource.isEmpty() || m_actionManager == nullptr)
        return m_iconSource;

    // iconSource is an optional property of the action managers that are made in QML.
    return m_actionManager->property("iconSource").toString();
}

void AbstractActionGroup::setActionManager(ActionManager *val)
{
    if (m_actionManager == val)
        return;

    if (m_actionManager != nullptr) {
        disconnect(m_actionManager, &QObject::destroyed, this,
                   &AbstractActionGroup::onActionManagerDestroyed);
        disconnect(m_actionManager, &ActionManager::titleChanged, this,
                   &AbstractActionGroup::titleChanged);
    }

    m_actionManager = val;

    if (m_actionManager != nullptr) {
        connect(m_actionManager, &QObject::destroyed, this,
                &AbstractActionGroup::onActionManagerDestroyed);
        connect(m_actionManager, &ActionManager::titleChanged, this,
                &AbstractActionGroup::titleChanged);
    }

    emit actionManagerChanged();
    emit titleChanged();
    emit iconSourceChanged();
}

void AbstractActionGroup::setEntries(const QVariantList &val)
{
    m_entries = val;
    emit entriesChanged();
}

void AbstractActionGroup::setDelegate(QQmlComponent *val)
{
    if (m_delegate == val)
        return;

    if (m_delegate != nullptr)
        disconnect(m_delegate, &QObject::destroyed, this, &AbstractActionGroup::onDelegateDestroyed);

    m_delegate = val;

    if (m_delegate != nullptr)
        connect(m_delegate, &QObject::destroyed, this, &AbstractActionGroup::onDelegateDestroyed);

    emit delegateChanged();
}

void AbstractActionGroup::setOrder(int val)
{
    if (m_order == val)
        return;

    m_order = val;
    emit orderChanged();

    scheduleResolve();
}

void AbstractActionGroup::setVisible(bool val)
{
    if (m_visible == val)
        return;

    m_visible = val;
    emit visibleChanged();

    scheduleResolve();
}

void AbstractActionGroup::scheduleResolve()
{
    ActionHostRegistry::scheduleResolve(m_kind, m_hostUri);
}

void AbstractActionGroup::onActionManagerDestroyed()
{
    m_actionManager = nullptr;
    emit actionManagerChanged();
    emit titleChanged();
    emit iconSourceChanged();
}

void AbstractActionGroup::onDelegateDestroyed()
{
    m_delegate = nullptr;
    emit delegateChanged();
}

///////////////////////////////////////////////////////////////////////////////

MenuGroup::MenuGroup(QObject *parent) : AbstractActionGroup(MenuKind, parent) { }

MenuGroup::~MenuGroup() { }

void MenuGroup::setInline(bool val)
{
    if (m_inline == val)
        return;

    m_inline = val;
    emit inlineChanged();
}

///////////////////////////////////////////////////////////////////////////////

MenuItemsDelegate::MenuItemsDelegate(QObject *parent) : QObject(parent) { }

MenuItemsDelegate::~MenuItemsDelegate() { }

QQmlListProperty<QObject> MenuItemsDelegate::objects()
{
    return QQmlListProperty<QObject>(this, &m_objects);
}

void MenuItemsDelegate::setContext(AbstractActionContext *val)
{
    if (m_context == val)
        return;

    if (m_context != nullptr)
        disconnect(m_context, &QObject::destroyed, this, &MenuItemsDelegate::onContextDestroyed);

    m_context = val;

    if (m_context != nullptr)
        connect(m_context, &QObject::destroyed, this, &MenuItemsDelegate::onContextDestroyed);

    emit contextChanged();
}

void MenuItemsDelegate::onContextDestroyed()
{
    m_context = nullptr;
    emit contextChanged();
}

///////////////////////////////////////////////////////////////////////////////

ToolBarGroup::ToolBarGroup(QObject *parent) : AbstractActionGroup(ToolBarKind, parent) { }

ToolBarGroup::~ToolBarGroup() { }

void ToolBarGroup::setPriority(int val)
{
    if (m_priority == val)
        return;

    m_priority = val;
    emit priorityChanged();
}

void ToolBarGroup::setCollapsible(bool val)
{
    if (m_collapsible == val)
        return;

    m_collapsible = val;
    emit collapsibleChanged();
}

void ToolBarGroup::setSeparator(bool val)
{
    if (m_separator == val)
        return;

    m_separator = val;
    emit separatorChanged();
}

void ToolBarGroup::setBackdrop(QQmlComponent *val)
{
    if (m_backdrop == val)
        return;

    if (m_backdrop != nullptr)
        disconnect(m_backdrop, &QObject::destroyed, this, &ToolBarGroup::onBackdropDestroyed);

    m_backdrop = val;

    if (m_backdrop != nullptr)
        connect(m_backdrop, &QObject::destroyed, this, &ToolBarGroup::onBackdropDestroyed);

    emit backdropChanged();
}

void ToolBarGroup::onBackdropDestroyed()
{
    m_backdrop = nullptr;
    emit backdropChanged();
}

///////////////////////////////////////////////////////////////////////////////

AbstractActionHost::AbstractActionHost(AbstractActionGroup::Kind kind, QObject *parent)
    : QAbstractListModel(parent), m_kind(kind)
{
    ActionHostRegistry::add(this);
}

AbstractActionHost::~AbstractActionHost()
{
    ActionHostRegistry::remove(this);
}

void AbstractActionHost::setUri(const QString &val)
{
    if (!m_uri.isEmpty()) {
        qWarning("ActionHost: uri can only be set once in %s", qPrintable(m_uri));
        return;
    }

    const QString val2 = val.trimmed();
    if (val2.isEmpty())
        return;

    m_uri = val2;
    emit uriChanged();

    ActionHostRegistry::checkContext(this);
    ActionHostRegistry::resolve(m_kind, m_uri);
}

void AbstractActionHost::setContext(AbstractActionContext *val)
{
    if (m_context == val)
        return;

    if (m_context != nullptr)
        disconnect(m_context, &QObject::destroyed, this, &AbstractActionHost::onContextDestroyed);

    m_context = val;

    if (m_context != nullptr)
        connect(m_context, &QObject::destroyed, this, &AbstractActionHost::onContextDestroyed);

    ActionHostRegistry::checkContext(this);
    emit contextChanged();
}

int AbstractActionHost::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_groups.size();
}

QVariant AbstractActionHost::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_groups.size())
        return QVariant();

    AbstractActionGroup *group = m_groups.at(index.row());
    switch (role) {
    case GroupRole:
        return QVariant::fromValue<QObject *>(group);
    case LevelRole:
        return this->levelOf(group);
    }

    return QVariant();
}

QHash<int, QByteArray> AbstractActionHost::roleNames() const
{
    return { { GroupRole, QByteArrayLiteral("group") }, { LevelRole, QByteArrayLiteral("level") } };
}

QVariant AbstractActionHost::levelOf(AbstractActionGroup *group) const
{
    Q_UNUSED(group);
    return QVariant();
}

void AbstractActionHost::reload()
{
    const QList<AbstractActionGroup *> groups =
            ActionHostRegistry::topLevelGroups(m_kind, m_uri);
    if (groups == m_groups)
        return;

    this->beginResetModel();
    m_groups = groups;
    this->endResetModel();

    emit countChanged();

    this->groupsReloaded();
}

void AbstractActionHost::onContextDestroyed()
{
    m_context = nullptr;
    emit contextChanged();
}

///////////////////////////////////////////////////////////////////////////////

MenuHost::MenuHost(QObject *parent) : AbstractActionHost(AbstractActionGroup::MenuKind, parent) { }

MenuHost::~MenuHost() { }

///////////////////////////////////////////////////////////////////////////////

ToolBarHost::ToolBarHost(QObject *parent)
    : AbstractActionHost(AbstractActionGroup::ToolBarKind, parent)
{
    m_fitTimer.setSingleShot(true);
    m_fitTimer.setInterval(16);
    connect(&m_fitTimer, &QTimer::timeout, this, &ToolBarHost::fit);
}

ToolBarHost::~ToolBarHost() { }

void ToolBarHost::setAvailableWidth(qreal val)
{
    if (qFuzzyCompare(m_availableWidth, val))
        return;

    m_availableWidth = val;
    emit availableWidthChanged();

    this->scheduleFit();
}

void ToolBarHost::setRoomyWidth(qreal val)
{
    if (qFuzzyCompare(m_roomyWidth, val))
        return;

    m_roomyWidth = val;
    emit roomyWidthChanged();

    this->scheduleFit();
}

void ToolBarHost::setGroupWidths(int row, qreal expanded, qreal menu)
{
    AbstractActionGroup *group = this->groupAt(row);
    if (group == nullptr)
        return;

    FitInfo &info = m_fitInfo[group];
    if (qFuzzyCompare(info.expandedWidth, expanded) && qFuzzyCompare(info.menuWidth, menu))
        return;

    info.expandedWidth = expanded;
    info.menuWidth = menu;

    this->scheduleFit();
}

void ToolBarHost::groupsReloaded()
{
    // Forget the groups that are gone.
    const QList<AbstractActionGroup *> groups = this->groups();
    QHash<const AbstractActionGroup *, FitInfo> fitInfo;
    for (AbstractActionGroup *group : groups) {
        if (m_fitInfo.contains(group))
            fitInfo.insert(group, m_fitInfo.value(group));
    }
    m_fitInfo = fitInfo;

    this->scheduleFit();
}

QVariant ToolBarHost::levelOf(AbstractActionGroup *group) const
{
    return int(m_fitInfo.value(group).level);
}

void ToolBarHost::scheduleFit()
{
    m_fitTimer.start();
}

void ToolBarHost::fit()
{
    const QList<AbstractActionGroup *> groups = this->groups();

    QList<Level> levels;
    levels.fill(Expanded, groups.size());

    auto widthOf = [&](int row) -> qreal {
        const FitInfo info = m_fitInfo.value(groups.at(row));
        switch (levels.at(row)) {
        case Expanded:
            return info.expandedWidth;
        case Menu:
            return info.menuWidth;
        case Hidden:
            break;
        }
        return 0;
    };

    auto totalWidth = [&]() -> qreal {
        qreal total = 0;
        for (int row = 0; row < groups.size(); row++)
            total += widthOf(row);
        return total;
    };

    qreal total = totalWidth();

    // The row of the group with the lowest priority, among those that are at the given level. Of
    // equals, the rightmost.
    auto lowestAt = [&](Level level) -> int {
        int victim = -1;
        for (int row = 0; row < groups.size(); row++) {
            if (levels.at(row) != level)
                continue;

            if (victim < 0) {
                victim = row;
                continue;
            }

            const ToolBarGroup *a = qobject_cast<ToolBarGroup *>(groups.at(row));
            const ToolBarGroup *b = qobject_cast<ToolBarGroup *>(groups.at(victim));
            if ((a ? a->priority() : 0) <= (b ? b->priority() : 0))
                victim = row;
        }
        return victim;
    };

    if (m_availableWidth > 0) {
        while (total > m_availableWidth) {
            // The group with the lowest priority steps down, but one that has folded into a menu is
            // hidden only when every group is folded or hidden, so that menus outlast buttons.
            int victim = lowestAt(Expanded);
            if (victim < 0)
                victim = lowestAt(Menu);

            if (victim < 0)
                break;

            const ToolBarGroup *group = qobject_cast<ToolBarGroup *>(groups.at(victim));
            if (levels.at(victim) == Expanded && group != nullptr && group->isCollapsible())
                levels[victim] = Menu;
            else
                levels[victim] = Hidden;

            total = totalWidth();
        }
    }

    bool allExpanded = true;
    for (int row = 0; row < groups.size(); row++) {
        if (levels.at(row) != Expanded)
            allExpanded = false;

        FitInfo &info = m_fitInfo[groups.at(row)];
        if (info.level != levels.at(row)) {
            info.level = levels.at(row);
            const QModelIndex index = this->index(row, 0, QModelIndex());
            emit this->dataChanged(index, index, { LevelRole });
        }
    }

    const bool roomy = allExpanded && m_availableWidth > 0 && (m_availableWidth - total) >= m_roomyWidth;
    if (m_roomy != roomy) {
        m_roomy = roomy;
        emit roomyChanged();
    }
}
