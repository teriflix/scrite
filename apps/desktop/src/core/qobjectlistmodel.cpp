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

#include "qobjectlistmodel.h"

#include <QJSEngine>

// Some duplication of code from Utils::Object, but that's acceptable.
class TypeInfo : public QObject
{
public:
    static bool check(QObject *ptr, const QString &typeName);
    static QString check(QObject *ptr, const QStringList &typeNames);
    static QString of(QObject *ptr);
    static QStringList hierarchy(QObject *ptr);
};

bool TypeInfo::check(QObject *ptr, const QString &typeName)
{
    return ptr ? ptr->inherits(qPrintable(typeName)) : false;
}

QString TypeInfo::check(QObject *ptr, const QStringList &typeNames)
{
    if (ptr && !typeNames.isEmpty()) {
        for (const QString &type : typeNames) {
            if (ptr->inherits(qPrintable(type)))
                return type;
        }
    }

    return QString();
}

QString TypeInfo::of(QObject *ptr)
{
    return ptr ? ptr->metaObject()->className() : QString();
}

QStringList TypeInfo::hierarchy(QObject *ptr)
{
    QStringList ret;
    if (ptr) {
        const QMetaObject *mo = ptr->metaObject();
        while (mo) {
            ret << QString::fromLatin1(mo->className());
            mo = mo->superClass();
        }
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

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

QObject *AbstractQObjectListModel::findByName(const QString &name, FindOption option) const
{
    if (name.isEmpty())
        return nullptr;

    for (int i = 0; i < this->objectCount(); i++) {
        QObject *ptr = this->objectAt(i);
        if (ptr->objectName() == name)
            return ptr;

        if (option == RecursiveFind) {
            if (ptr->metaObject()->inherits(&AbstractQObjectListModel::staticMetaObject)) {
                AbstractQObjectListModel *model = qobject_cast<AbstractQObjectListModel *>(ptr);
                ptr = model->findByName(name);
                if (ptr)
                    return ptr;
            }

            ptr = ptr->findChild<QObject *>(name, Qt::FindChildrenRecursively);
            if (ptr)
                return ptr;
        }
    }

    return nullptr;
}

void AbstractQObjectListModel::setObjectKinds(const QStringList &val)
{
    if (m_objectKinds == val)
        return;

    m_objectKinds = val;
    emit objectKindsChanged();
}

QHash<int, QByteArray> AbstractQObjectListModel::roleNames() const
{
    return { { ObjectItemRole, QByteArrayLiteral("objectItem") },
             { ObjectTypeRole, QByteArrayLiteral("objectType") },
             { ObjectTypeHierarchyRole, QByteArrayLiteral("objectTypeHierarchy") },
             { ModelDataRole, QByteArrayLiteral("modelData") },
             { ObjectKindRole, QByteArrayLiteral("objectKind") } };
}

int AbstractQObjectListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : this->objectCount();
}

QVariant AbstractQObjectListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() >= 0 && index.row() < this->objectCount()) {
        QObject *ptr = this->objectAt(index.row());
        switch (role) {
        case ObjectItemRole:
        case ModelDataRole:
            return QVariant::fromValue<QObject *>(ptr);
        case ObjectTypeRole:
            return TypeInfo::of(ptr);
        case ObjectTypeHierarchyRole:
            return TypeInfo::hierarchy(ptr);
        case ObjectKindRole:
            return TypeInfo::check(ptr, m_objectKinds);
        default:
            break;
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////

ObjectListModelAttached *ObjectListModel::qmlAttachedProperties(QObject *object)
{
    return new ObjectListModelAttached(object);
}

ObjectListModel::ObjectListModel(QObject *parent) : QObjectListModel<QObject *>(parent)
{
    connect(this, &ObjectListModel::objectCountChanged, this, &ObjectListModel::objectsChanged);
    connect(this, &ObjectListModel::rowsMoved, this, &ObjectListModel::objectsChanged);
}

///////////////////////////////////////////////////////////////////////////////

ObjectListModelAttached::ObjectListModelAttached(QObject *parent) : QObject(parent) { }

ObjectListModelAttached::~ObjectListModelAttached() { }

void ObjectListModelAttached::setTarget(ObjectListModel *val)
{
    if (m_target == val)
        return;

    if (m_target != nullptr)
        m_target->remove(this->parent());

    m_target = val;

    if (m_target != nullptr)
        m_target->insert(m_index, this->parent());

    emit targetChanged();
}

void ObjectListModelAttached::setIndex(int val)
{
    if (m_index == val)
        return;

    m_index = val;

    if (m_target != nullptr) {
        int row = m_target->indexOf(this->parent());
        if (row != m_index && m_index >= 0 && m_index < m_target->size()) {
            m_target->move(row, m_index);
        }
    }

    emit indexChanged();
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

    this->beginFilterChange();

    m_filterByProperty = val;
    emit filterByPropertyChanged();

    this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void SortFilterObjectListModel::setFilterValues(const QVariantList &val)
{
    if (m_filterValues == val)
        return;

    this->beginFilterChange();

    m_filterValues = val;
    emit filterValuesChanged();

    this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void SortFilterObjectListModel::setFilterMode(FilterMode val)
{
    if (m_filterMode == val)
        return;

    this->beginFilterChange();

    m_filterMode = val;
    emit filterModeChanged();

    this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void SortFilterObjectListModel::setSortFunction(const QJSValue &val)
{
    if (m_sortFunction.equals(val))
        return;

    if (!val.isCallable())
        return;

    m_sortFunction = val;
    emit sortFunctionChanged();

    this->sort(0, this->sortOrder());
}

void SortFilterObjectListModel::setFilterFunction(const QJSValue &val)
{
    if (m_filterFunction.equals(val))
        return;

    if (!val.isCallable())
        return;

    this->beginFilterChange();

    m_filterFunction = val;
    emit filterFunctionChanged();

    this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void SortFilterObjectListModel::setJsEngine(QJSEngine *val)
{
    if (m_jsEngine == val)
        return;

    m_jsEngine = val;
    emit jsEngineChanged();
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

    QJSEngine *engine = m_jsEngine == nullptr
            ? (m_sortFunction.isCallable() ? qjsEngine(this) : nullptr)
            : m_jsEngine;
    if (engine != nullptr) {
        QJSValueList args;
        args.append(engine->newQObject(left_object));
        args.append(engine->newQObject(right_object));
        return m_sortFunction.call(args).toBool();
    }

    const QVariant left = left_object->property(m_sortByProperty);
    const QVariant right = right_object->property(m_sortByProperty);
    return QVariant::compare(left, right) == QPartialOrdering::Less;
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

    QJSEngine *engine = m_jsEngine == nullptr
            ? (m_filterFunction.isCallable() ? qjsEngine(this) : nullptr)
            : m_jsEngine;
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
