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

#include "shortcutsmodel.h"
#include "qobjectlistmodel.h"
#include "application.h"

#include <QTimer>
#include <QShortcut>
#include <QCoreApplication>

class ShortcutsModelItemListModel : public QObjectListModel<ShortcutsModelItem *>
{
public:
    ShortcutsModelItemListModel(QObject *parent = nullptr)
        : QObjectListModel<ShortcutsModelItem *>(parent)
    {
    }
    ~ShortcutsModelItemListModel() { }

    void itemInsertEvent(ShortcutsModelItem *ptr)
    {
        connect(ptr, &ShortcutsModelItem::changed, this, &ShortcutsModelItemListModel::itemChanged);
    }

    void itemRemoveEvent(ShortcutsModelItem *ptr)
    {
        disconnect(ptr, &ShortcutsModelItem::changed, this,
                   &ShortcutsModelItemListModel::itemChanged);
    }

    void itemChanged(ShortcutsModelItem *ptr)
    {
        const int row = this->indexOf(ptr);
        if (row >= 0) {
            const QModelIndex index = this->index(row, 0);
            emit dataChanged(index, index);
        }
    }
};

Q_GLOBAL_STATIC(ShortcutsModelItemListModel, GlobalShortcutItemsModel);

ShortcutsModel::ShortcutsModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    this->setSourceModel(::GlobalShortcutItemsModel);
    this->setFilterKeyColumn(0);
    this->setFilterRole(ShortcutsModel::VisibleRole);
    this->setDynamicSortFilter(true);
    this->sort(0);
    this->setSortRole(ShortcutsModel::GroupRole);

    connect(::GlobalShortcutItemsModel, &QAbstractItemModel::dataChanged, this,
            [=]() { this->invalidate(); });
}

ShortcutsModel::~ShortcutsModel() { }

void ShortcutsModel::setGroups(const QStringList &val)
{
    if (m_groups == val)
        return;

    m_groups = val;
    for (QString &group : m_groups)
        group = group.toLower();

    emit groupsChanged();
}

void ShortcutsModel::setTitleFilter(const QString &val)
{
    if (m_titleFilter == val)
        return;

    m_titleFilter = val;
    emit titleFilterChanged();

    this->invalidateFilter();
}

QVariant ShortcutsModel::data(const QModelIndex &index, int role) const
{
    const QModelIndex sourceIndex = this->mapToSource(index);

    const ShortcutsModelItem *item = qobject_cast<ShortcutsModelItem *>(
            sourceIndex.data(AbstractQObjectListModel::ObjectItemRole).value<QObject *>());
    switch (role) {
    case GroupRole:
        return item->group();
    case TitleRole:
        return item->title();
    case EnabledRole:
        return item->isEnabled();
    case VisibleRole:
        return item->isVisible() && !item->title().isEmpty() && !item->group().isEmpty();
    case ShortcutRole:
        return item->shortcut();
    case CanActivateRole:
        return item->canActivate();
    default:
        break;
    }

    return QVariant();
}

void ShortcutsModel::activateShortcutAt(int row)
{
    const QModelIndex index = this->index(row, 0);
    const QModelIndex sourceIndex = this->mapToSource(index);
    ShortcutsModelItem *item = qobject_cast<ShortcutsModelItem *>(
            sourceIndex.data(AbstractQObjectListModel::ObjectItemRole).value<QObject *>());
    if (item)
        item->activate();
}

void ShortcutsModel::printWholeThing()
{
    Application::log("===========");
    const int nrRows = this->rowCount();
    for (int i = 0; i < nrRows; i++) {
        const QModelIndex index = this->index(i, 0);
        Application::log(QString::number(i) + ": "
                         + index.data(ShortcutsModel::GroupRole).toString() + "/"
                         + index.data(ShortcutsModel::TitleRole).toString());
    }
    Application::log("===========");
}

bool ShortcutsModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const ShortcutsModelItem *left = ::GlobalShortcutItemsModel->at(source_left.row());
    const ShortcutsModelItem *right = ::GlobalShortcutItemsModel->at(source_right.row());

    const QString leftGroup = left->group().toLower();
    const QString rightGroup = right->group().toLower();
    const int leftGroupIndex = m_groups.indexOf(leftGroup);
    const int rightGroupIndex = m_groups.indexOf(rightGroup);
    if (leftGroupIndex < 0 && rightGroupIndex < 0)
        return QString::localeAwareCompare(leftGroup, rightGroup) < 0;
    if (leftGroupIndex < 0)
        return false;
    if (rightGroupIndex < 0)
        return true;
    if (leftGroupIndex == rightGroupIndex) {
        if (left->priority() != right->priority())
            return left->priority() < right->priority();
        return QString::localeAwareCompare(left->title(), right->title()) < 0;
    }
    return leftGroupIndex < rightGroupIndex;
}

bool ShortcutsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    const ShortcutsModelItem *item = ::GlobalShortcutItemsModel->at(source_row);
    return item->isVisible() && !item->title().isEmpty() && !item->group().isEmpty()
            && item->title().startsWith(m_titleFilter, Qt::CaseInsensitive);
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[TitleRole] = "itemTitle";
        roles[GroupRole] = "itemGroup";
        roles[EnabledRole] = "itemEnabled";
        roles[VisibleRole] = "itemVisible";
        roles[ShortcutRole] = "itemShortcut";
        roles[CanActivateRole] = "itemCanBeActivated";
    }

    return roles;
}

////////////////////////////////////////////////////////////////////////////////////////

ShortcutsModelItem::ShortcutsModelItem(QObject *parent) : QObject(parent)
{
    ::GlobalShortcutItemsModel->append(this);
}

ShortcutsModelItem::~ShortcutsModelItem()
{
    ::GlobalShortcutItemsModel->remove(this);
}

ShortcutsModelItem *ShortcutsModelItem::qmlAttachedProperties(QObject *object)
{
    return new ShortcutsModelItem(object);
}

void ShortcutsModelItem::setTitle(const QString &val)
{
    if (m_title == val)
        return;

    m_title = val;
    emit titleChanged();
    emit changed(this);
}

void ShortcutsModelItem::setShortcut(const QString &val)
{
    if (m_shortcut == val)
        return;

    m_shortcut = val;
    emit shortcutChanged();
    emit changed(this);
}

void ShortcutsModelItem::setGroup(const QString &val)
{
    if (m_group == val)
        return;

    m_group = val;
    emit groupChanged();
    emit changed(this);
}

void ShortcutsModelItem::setPriority(int val)
{
    if (m_priority == val)
        return;

    m_priority = val;
    emit priorityChanged();
    emit changed(this);
}

void ShortcutsModelItem::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
    emit changed(this);
}

void ShortcutsModelItem::setVisible(bool val)
{
    if (m_visible == val)
        return;

    m_visible = val;
    emit visibleChanged();
    emit changed(this);
}

void ShortcutsModelItem::setCanActivate(bool val)
{
    if (m_canActivate == val)
        return;

    m_canActivate = val;
    emit canActivateChanged();
    emit changed(this);
}

void ShortcutsModelItem::activate()
{
    if (m_canActivate)
        emit activated();
}
