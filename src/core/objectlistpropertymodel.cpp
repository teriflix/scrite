/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "objectlistpropertymodel.h"

ObjectListPropertyModelBase::ObjectListPropertyModelBase(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(this, &QAbstractListModel::rowsInserted, this, &ObjectListPropertyModelBase::objectCountChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &ObjectListPropertyModelBase::objectCountChanged);
    connect(this, &QAbstractListModel::modelReset, this, &ObjectListPropertyModelBase::objectCountChanged);
}

QHash<int, QByteArray> ObjectListPropertyModelBase::roleNames() const
{
    return {
        { ObjectItemRole, QByteArrayLiteral("objectItem") },
        { ModelDataRole, QByteArrayLiteral("modelData") }
    };
}

///////////////////////////////////////////////////////////////////////////////

SortedObjectListPropertyModel::SortedObjectListPropertyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &QSortFilterProxyModel::rowsInserted, this, &SortedObjectListPropertyModel::objectCountChanged);
    connect(this, &QSortFilterProxyModel::rowsRemoved, this, &SortedObjectListPropertyModel::objectCountChanged);
    connect(this, &QSortFilterProxyModel::modelReset, this, &SortedObjectListPropertyModel::objectCountChanged);

    this->setDynamicSortFilter(true);
}

void SortedObjectListPropertyModel::setSortByProperty(const QByteArray &val)
{
    if(m_sortByProperty == val)
        return;

    m_sortByProperty = val;
    emit sortByPropertyChanged();

    this->sort(0);
}

QHash<int, QByteArray> SortedObjectListPropertyModel::roleNames() const
{
    return {
        { ObjectListPropertyModelBase::ObjectItemRole, QByteArrayLiteral("objectItem") },
        { ObjectListPropertyModelBase::ModelDataRole, QByteArrayLiteral("modelData") }
    };
}

bool SortedObjectListPropertyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if(m_sortByProperty.isEmpty())
        return false;

    const QMetaObject *mo = this->sourceModel()->metaObject();
    if(!mo->inherits(&ObjectListPropertyModelBase::staticMetaObject))
        return false;

    QObject *left_object = source_left.data(ObjectListPropertyModelBase::ObjectItemRole).value<QObject*>();
    QObject *right_object = source_right.data(ObjectListPropertyModelBase::ObjectItemRole).value<QObject*>();
    if(left_object == nullptr || right_object == nullptr)
        return false;

    const QVariant left = left_object->property(m_sortByProperty);
    const QVariant right = right_object->property(m_sortByProperty);
    return left < right;
}
