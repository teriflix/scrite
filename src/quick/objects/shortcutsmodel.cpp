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

#include "application.h"
#include "shortcutsmodel.h"

ShortcutsModel *ShortcutsModel::instance()
{
    static ShortcutsModel *theInstance = new ShortcutsModel(qApp);
    return theInstance;
}

ShortcutsModel::ShortcutsModel(QObject *parent) : QAbstractListModel(parent)
{
    setGroups({ QStringLiteral("Application"), QStringLiteral("Formatting"),
                QStringLiteral("Settings"), QStringLiteral("Language"), QStringLiteral("File"),
                QStringLiteral("Edit") });
}

ShortcutsModel::~ShortcutsModel() { }

void ShortcutsModel::add(ShortcutsModelItem *item)
{
    if (m_items.contains(item) || item == nullptr)
        return;

    QList<ShortcutsModelItem *> items = m_items;
    items.append(item);
    this->sortItems(items);

    const int index = items.indexOf(item);
    this->beginInsertRows(QModelIndex(), index, index);
    m_items = items;
    this->endInsertRows();
}

void ShortcutsModel::remove(ShortcutsModelItem *item)
{
    const int index = item == nullptr ? -1 : m_items.indexOf(item);
    if (index < 0)
        return;

    this->beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    this->endRemoveRows();
}

void ShortcutsModel::update(ShortcutsModelItem *item, bool removeAndInsert)
{
    if (removeAndInsert) {
        this->remove(item);
        this->add(item);
    } else {
        const int row = m_items.indexOf(item);
        if (row < 0)
            return;

        const QModelIndex modelIndex = this->index(row);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void ShortcutsModel::sortItems(QList<ShortcutsModelItem *> &items)
{
    std::sort(items.begin(), items.end(),
              [](const ShortcutsModelItem *item1, const ShortcutsModelItem *item2) {
                  const QStringList groups = ShortcutsModel::instance()->groups();
                  if (item1->group() != item2->group()) {
                      const int group1Index = groups.indexOf(item1->group());
                      const int group2Index = groups.indexOf(item2->group());
                      if (group1Index < 0 && group2Index < 0)
                          return item1->group() < item2->group();
                      if (group1Index < 0)
                          return false;
                      if (group2Index < 0)
                          return true;
                      return group1Index < group2Index;
                  }
                  return item1->priority() > item2->priority();
              });
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant ShortcutsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const ShortcutsModelItem *item = m_items.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return item->title();
    case ShortcutRole:
        return item->shortcut();
    case GroupRole:
        return item->group();
    case EnabledRole:
        return item->isEnabled();
    case VisibleRole:
        return item->isVisible();
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[TitleRole] = "itemTitle";
        roles[ShortcutRole] = "itemShortcut";
        roles[GroupRole] = "itemGroup";
        roles[EnabledRole] = "itemEnabled";
        roles[VisibleRole] = "itemVisible";
    }

    return roles;
}

void ShortcutsModel::setGroups(const QStringList &val)
{
    if (m_groups == val)
        return;

    m_groups = val;
    emit groupsChanged();
}

////////////////////////////////////////////////////////////////////////////////////////

ShortcutsModelItem::ShortcutsModelItem(QObject *parent) : QObject(parent)
{
    ShortcutsModel::instance()->add(this);
}

ShortcutsModelItem::~ShortcutsModelItem()
{
    ShortcutsModel::instance()->remove(this);
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

    ShortcutsModel::instance()->update(this);
}

void ShortcutsModelItem::setShortcut(const QString &val)
{
    if (m_shortcut == val)
        return;

    m_shortcut = val;
    emit shortcutChanged();

    ShortcutsModel::instance()->update(this);
}

void ShortcutsModelItem::setGroup(const QString &val)
{
    if (m_group == val)
        return;

    m_group = val;
    emit groupChanged();

    ShortcutsModel::instance()->update(this, true);
}

void ShortcutsModelItem::setPriority(int val)
{
    if (m_priority == val)
        return;

    m_priority = val;
    emit priorityChanged();

    ShortcutsModel::instance()->update(this, true);
}

void ShortcutsModelItem::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    ShortcutsModel::instance()->update(this);
}

void ShortcutsModelItem::setVisible(bool val)
{
    if (m_visible == val)
        return;

    m_visible = val;
    emit visibleChanged();

    ShortcutsModel::instance()->update(this);
}
