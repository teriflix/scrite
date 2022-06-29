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

#include "qobjectlistmodel.h"

#include <QJSEngine>

AbstractQObjectListModel::AbstractQObjectListModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(this, &QAbstractListModel::rowsInserted, this,
            &AbstractQObjectListModel::objectCountChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this,
            &AbstractQObjectListModel::objectCountChanged);
    connect(this, &QAbstractListModel::modelReset, this,
            &AbstractQObjectListModel::objectCountChanged);
    connect(this, &QAbstractListModel::dataChanged, this, &AbstractQObjectListModel::dataChanged2);
}

QHash<int, QByteArray> AbstractQObjectListModel::roleNames() const
{
    return { { ObjectItemRole, QByteArrayLiteral("objectItem") },
             { ModelDataRole, QByteArrayLiteral("modelData") } };
}

///////////////////////////////////////////////////////////////////////////////

SortFilterObjectListModel::SortFilterObjectListModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &QSortFilterProxyModel::rowsInserted, this,
            &SortFilterObjectListModel::objectCountChanged);
    connect(this, &QSortFilterProxyModel::rowsRemoved, this,
            &SortFilterObjectListModel::objectCountChanged);
    connect(this, &QSortFilterProxyModel::modelReset, this,
            &SortFilterObjectListModel::objectCountChanged);

    this->setDynamicSortFilter(true);
}

void SortFilterObjectListModel::setSortByProperty(const QByteArray &val)
{
    if (m_sortByProperty == val)
        return;

    m_sortByProperty = val;
    emit sortByPropertyChanged();

    this->sort(0);
}

void SortFilterObjectListModel::setFilterByProperty(const QByteArray &val)
{
    if (m_filterByProperty == val)
        return;

    m_filterByProperty = val;
    emit filterByPropertyChanged();

    this->invalidateFilter();
}

void SortFilterObjectListModel::setFilterValues(const QVariantList &val)
{
    if (m_filterValues == val)
        return;

    m_filterValues = val;
    emit filterValuesChanged();

    this->invalidateFilter();
}

void SortFilterObjectListModel::setFilterMode(FilterMode val)
{
    if (m_filterMode == val)
        return;

    m_filterMode = val;
    emit filterModeChanged();

    this->invalidateFilter();
}

void SortFilterObjectListModel::setSortFunction(const QJSValue &val)
{
    if (m_sortFunction.equals(val))
        return;

    if (!val.isCallable())
        return;

    m_sortFunction = val;
    emit sortFunctionChanged();
}

void SortFilterObjectListModel::setFilterFunction(const QJSValue &val)
{
    if (m_filterFunction.equals(val))
        return;

    if (!val.isCallable())
        return;

    m_filterFunction = val;
    emit filterFunctionChanged();
}

QHash<int, QByteArray> SortFilterObjectListModel::roleNames() const
{
    return { { AbstractQObjectListModel::ObjectItemRole, QByteArrayLiteral("objectItem") },
             { AbstractQObjectListModel::ModelDataRole, QByteArrayLiteral("modelData") } };
}

bool SortFilterObjectListModel::lessThan(const QModelIndex &source_left,
                                         const QModelIndex &source_right) const
{
    if (m_sortByProperty.isEmpty() && !m_sortFunction.isCallable())
        return false;

    const QMetaObject *mo = this->sourceModel()->metaObject();
    if (!mo->inherits(&AbstractQObjectListModel::staticMetaObject))
        return false;

    QObject *left_object =
            source_left.data(AbstractQObjectListModel::ObjectItemRole).value<QObject *>();
    QObject *right_object =
            source_right.data(AbstractQObjectListModel::ObjectItemRole).value<QObject *>();
    if (left_object == nullptr || right_object == nullptr)
        return false;

    QJSEngine *engine = m_sortFunction.isCallable() ? qjsEngine(this) : nullptr;
    if (engine != nullptr) {
        QJSValueList args;
        args.append(engine->newQObject(left_object));
        args.append(engine->newQObject(right_object));
        return m_sortFunction.call(args).toBool();
    }

    const QVariant left = left_object->property(m_sortByProperty);
    const QVariant right = right_object->property(m_sortByProperty);
    return left < right;
}

bool SortFilterObjectListModel::filterAcceptsRow(int source_row,
                                                 const QModelIndex &source_parent) const
{
    if ((m_filterByProperty.isEmpty() || m_filterValues.isEmpty())
        && !m_filterFunction.isCallable())
        return true;

    const QMetaObject *mo = this->sourceModel()->metaObject();
    if (!mo->inherits(&AbstractQObjectListModel::staticMetaObject))
        return true;

    const QModelIndex source_index = this->sourceModel()->index(source_row, 0, source_parent);
    if (!source_index.isValid())
        return true;

    QObject *source_object =
            source_index.data(AbstractQObjectListModel::ObjectItemRole).value<QObject *>();
    if (source_object == nullptr)
        return true;

    QJSEngine *engine = m_filterFunction.isCallable() ? qjsEngine(this) : nullptr;
    if (engine != nullptr) {
        QJSValueList args;
        args.append(engine->newQObject(source_object));
        return m_filterFunction.call(args).toBool();
    }

    const QVariant value = source_object->property(m_filterByProperty);
    const bool flag = m_filterValues.contains(value);

    if (m_filterMode == IncludeFilterValues)
        return flag;

    return !flag;
}
