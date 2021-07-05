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

SortFilterObjectListModel::SortFilterObjectListModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &QSortFilterProxyModel::rowsInserted, this, &SortFilterObjectListModel::objectCountChanged);
    connect(this, &QSortFilterProxyModel::rowsRemoved, this, &SortFilterObjectListModel::objectCountChanged);
    connect(this, &QSortFilterProxyModel::modelReset, this, &SortFilterObjectListModel::objectCountChanged);

    this->setDynamicSortFilter(true);
}

void SortFilterObjectListModel::setSortByProperty(const QByteArray &val)
{
    if(m_sortByProperty == val)
        return;

    m_sortByProperty = val;
    emit sortByPropertyChanged();

    this->sort(0);
}

void SortFilterObjectListModel::setFilterByProperty(const QByteArray &val)
{
    if(m_filterByProperty == val)
        return;

    m_filterByProperty = val;
    emit filterByPropertyChanged();

    this->invalidateFilter();
}

void SortFilterObjectListModel::setFilterValues(const QVariantList &val)
{
    if(m_filterValues == val)
        return;

    m_filterValues = val;
    emit filterValuesChanged();

    this->invalidateFilter();
}

void SortFilterObjectListModel::setFilterMode(FilterMode val)
{
    if(m_filterMode == val)
        return;

    m_filterMode = val;
    emit filterModeChanged();

    this->invalidateFilter();
}

QHash<int, QByteArray> SortFilterObjectListModel::roleNames() const
{
    return {
        { ObjectListPropertyModelBase::ObjectItemRole, QByteArrayLiteral("objectItem") },
        { ObjectListPropertyModelBase::ModelDataRole, QByteArrayLiteral("modelData") }
    };
}

bool SortFilterObjectListModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
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

bool SortFilterObjectListModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(m_filterByProperty.isEmpty() || m_filterValues.isEmpty())
        return true;

    const QMetaObject *mo = this->sourceModel()->metaObject();
    if(!mo->inherits(&ObjectListPropertyModelBase::staticMetaObject))
        return true;

    const QModelIndex source_index = this->sourceModel()->index(source_row, 0, source_parent);
    if(!source_index.isValid())
        return true;

    QObject *source_object = source_index.data(ObjectListPropertyModelBase::ObjectItemRole).value<QObject*>();
    if(source_object == nullptr)
        return true;

    const QVariant value = source_object->property(m_filterByProperty);
    const bool flag = m_filterValues.contains(value);

    if(m_filterMode == IncludeFilterValues)
        return flag;

    return !flag;
}
