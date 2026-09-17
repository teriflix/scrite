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

#ifndef OBJECTLISTMODEL_H
#define OBJECTLISTMODEL_H

#include <QSet>
#include <QList>
#include <QJSValue>
#include <QQmlEngine>
#include <QMetaMethod>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class AbstractQObjectListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Base class")

public:
    explicit AbstractQObjectListModel(QObject *parent = nullptr);
    ~AbstractQObjectListModel() { }

    // clang-format off
    Q_PROPERTY(int objectCount
                       READ objectCount
                               NOTIFY objectCountChanged)
    // clang-format on
    virtual int objectCount() const = 0;
    Q_SIGNAL void objectCountChanged();

    /**
     * Handling dataChanged() signal in QML is confusing, because Connections
     * has its own dataChanged() signal.
     */
    Q_SIGNAL void dataChanged2(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    Q_INVOKABLE virtual QObject *objectAt(int row) const = 0;

    enum FindOption { SimpleFind, RecursiveFind };
    Q_ENUM(FindOption)

    Q_INVOKABLE QObject *findByName(const QString &name, FindOption = SimpleFind) const;

    // clang-format off
    Q_PROPERTY(QStringList objectKinds
               READ objectKinds
               WRITE setObjectKinds
               NOTIFY objectKindsChanged)
    // clang-format on
    void setObjectKinds(const QStringList &val);
    QStringList objectKinds() const { return m_objectKinds; }
    Q_SIGNAL void objectKindsChanged();

    // QAbstractListModel implementation
    enum {
        ObjectItemRole = Qt::UserRole + 1,
        ModelDataRole,
        ObjectTypeRole,
        ObjectTypeHierarchyRole,
        ObjectKindRole
    };
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

private:
    QStringList m_objectKinds;
};

template<class T>
class QObjectListModel : public AbstractQObjectListModel
{
    QML_ANONYMOUS

public:
    explicit QObjectListModel(QObject *parent = nullptr) : AbstractQObjectListModel(parent) { }
    ~QObjectListModel() { }

    operator QList<T>() { return m_list; }
    QList<T> &list() { return m_list; }
    const QList<T> &list() const { return m_list; }
    const QList<T> &constList() const { return m_list; }

    bool empty() const { return m_list.empty(); }
    bool isEmpty() const { return m_list.isEmpty(); }

    void append(T ptr) { this->insert(-1, ptr); }

    void prepend(T ptr)
    {
        if (m_list.contains(ptr) || ptr == nullptr)
            return;
        this->beginInsertRows(QModelIndex(), 0, 0);
        m_list.prepend(ptr);
        this->itemInsertEvent(ptr);
        this->endInsertRows();
    }

    int indexOf(T ptr) const { return m_list.indexOf(ptr); }

    void remove(T ptr)
    {
        const int index = this->indexOf(ptr);
        this->removeAt(index);
    }

    void removeAt(int row)
    {
        if (row < 0 || row >= m_list.size())
            return;
        this->beginRemoveRows(QModelIndex(), row, row);
        T ptr = m_list.at(row);
        this->itemRemoveEvent(ptr);
        ptr->disconnect(this);
        m_list.removeAt(row);
        this->endRemoveRows();
    }

    void insert(int row, T ptr)
    {
        if (m_list.contains(ptr) || ptr == nullptr)
            return;
        int iidx = row < 0 || row >= m_list.size() ? m_list.size() : row;
        this->beginInsertRows(QModelIndex(), iidx, iidx);
        m_list.insert(iidx, ptr);
        this->itemInsertEvent(ptr);
        this->endInsertRows();
    }

    void move(int fromRow, int toRow)
    {
        if (fromRow == toRow)
            return;

        if (fromRow < 0 || fromRow >= m_list.size())
            return;

        if (toRow < 0 || toRow >= m_list.size())
            return;

        this->beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(),
                            toRow < fromRow ? toRow : toRow + 1);
        m_list.move(fromRow, toRow);
        this->endMoveRows();
    }

    void assign(const QList<T> &list)
    {
        this->beginResetModel();
        while (!m_list.isEmpty()) {
            T ptr = m_list.first();
            this->itemRemoveEvent(ptr);
            ptr->disconnect(this);
            m_list.takeFirst();
        }
        if (!list.isEmpty()) {
            for (T ptr : list) {
                if (m_list.contains(ptr))
                    continue;
                this->itemInsertEvent(ptr);
                m_list.append(ptr);
            }
        }
        this->endResetModel();
    }

    void clear()
    {
        this->beginResetModel();
        while (!m_list.isEmpty()) {
            T ptr = m_list.first();
            this->itemRemoveEvent(ptr);
            ptr->disconnect(this);
            m_list.takeFirst();
        }
        this->endResetModel();
    }

    int size() const { return m_list.size(); }
    T at(int row) const { return row < 0 || row >= m_list.size() ? nullptr : m_list.at(row); }

    T first() const { return m_list.isEmpty() ? nullptr : m_list.first(); }
    T takeFirst()
    {
        T ptr = this->first();
        if (ptr == nullptr)
            return ptr;
        this->removeAt(0);
        return ptr;
    }

    T last() const { return m_list.isEmpty() ? nullptr : m_list.last(); }
    T takeLast()
    {
        T ptr = this->last();
        if (ptr == nullptr)
            return ptr;
        this->removeAt(m_list.size() - 1);
        return ptr;
    }

    T takeAt(int row)
    {
        T ptr = this->at(row);
        if (ptr == nullptr)
            return ptr;
        this->removeAt(row);
        return ptr;
    }

    void sortList(const std::function<bool(T, T)> &sortFunction)
    {
        bool shuffled = false;
        QList<T> copy = m_list;
        std::sort(copy.begin(), copy.end(), [sortFunction, &shuffled](T a, T b) {
            bool ret = sortFunction(a, b);
            if (ret)
                shuffled = true;
            return ret;
        });
        if (shuffled) {
            this->beginResetModel();
            m_list = copy;
            this->endResetModel();
        }
    }

    const QList<T> sortedList(std::function<bool(T, T)> lessThanFunc) const
    {
        QList<T> ret = m_list;
        std::sort(ret.begin(), ret.end(), lessThanFunc);
        return ret;
    }

    const QList<T> filteredList(std::function<bool(T)> filterFunc) const
    {
        QList<T> ret;
        for (T item : m_list) {
            if (filterFunc(item))
                ret.append(item);
        }
        return ret;
    }

    // ObjectListPropertyModelBase interface
    int objectCount() const { return m_list.size(); }
    QObject *objectAt(int row) const { return this->at(row); }

public:
    void objectChanged()
    {
        T ptr = qobject_cast<T>(this->sender());
        if (ptr == nullptr)
            return;
        const int row = m_list.indexOf(ptr);
        if (row < 0)
            return;
        const QModelIndex index = this->index(row, 0, QModelIndex());
        emit dataChanged(index, index);
    }

    void objectDestroyed(T ptr)
    {
        if (ptr == nullptr)
            return;
        const int row = m_list.indexOf(ptr);
        if (row < 0)
            return;
        this->removeAt(row);
    }

protected:
    virtual void itemInsertEvent(T ptr) { Q_UNUSED(ptr); }
    virtual void itemRemoveEvent(T ptr) { Q_UNUSED(ptr); }

protected:
    QList<T> m_list;
};

class ObjectListModelAttached;

class ObjectListModel : public QObjectListModel<QObject *>
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(ObjectListModelAttached)

public:
    static ObjectListModelAttached *qmlAttachedProperties(QObject *object);

    ObjectListModel(QObject *parent = nullptr);
    ~ObjectListModel() { }

    Q_INVOKABLE void include(QObject *ptr) { this->append(ptr); }
    Q_INVOKABLE void exclude(QObject *ptr) { this->remove(ptr); }
    Q_INVOKABLE void reset() { this->clear(); }

    Q_CLASSINFO("DefaultProperty", "objects")
    Q_PROPERTY(QQmlListProperty<QObject> objects READ objects NOTIFY objectsChanged)
    QQmlListProperty<QObject> objects()
    {
        return QQmlListProperty<QObject>(reinterpret_cast<QObject *>(this), &m_list);
    }
    Q_SIGNAL void objectsChanged();

protected:
    void itemInsertEvent(QObject *ptr)
    {
        connect(ptr, &QObject::destroyed, this, &ObjectListModel::objectDestroyed);
    }
    void itemRemoveEvent(QObject *ptr)
    {
        disconnect(ptr, &QObject::destroyed, this, &ObjectListModel::objectDestroyed);
    }
};

class ObjectListModelAttached : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit ObjectListModelAttached(QObject *parent = nullptr);
    ~ObjectListModelAttached();

    // clang-format off
    Q_PROPERTY(ObjectListModel* target
               READ target
               WRITE setTarget
               NOTIFY targetChanged)
    // clang-format on
    void setTarget(ObjectListModel *val);
    ObjectListModel *target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    // clang-format off
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY indexChanged)
    // clang-format on
    void setIndex(int val);
    int index() const { return m_index; }
    Q_SIGNAL void indexChanged();

private:
    int m_index = -1;
    ObjectListModel *m_target = nullptr;
};

class SortFilterObjectListModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SortFilterObjectListModel(QObject *parent = nullptr);
    ~SortFilterObjectListModel() { }

    // clang-format off
    Q_PROPERTY(int objectCount
               READ objectCount
               NOTIFY objectCountChanged)
    // clang-format on
    int objectCount() const { return this->rowCount(QModelIndex()); }
    Q_SIGNAL void objectCountChanged();

    // clang-format off
    Q_PROPERTY(QByteArray sortByProperty
               READ sortByProperty
               WRITE setSortByProperty
               NOTIFY sortByPropertyChanged)
    // clang-format on
    void setSortByProperty(const QByteArray &val);
    QByteArray sortByProperty() const { return m_sortByProperty; }
    Q_SIGNAL void sortByPropertyChanged();

    // clang-format off
    Q_PROPERTY(QByteArray filterByProperty
               READ filterByProperty
               WRITE setFilterByProperty
               NOTIFY filterByPropertyChanged)
    // clang-format on
    void setFilterByProperty(const QByteArray &val);
    QByteArray filterByProperty() const { return m_filterByProperty; }
    Q_SIGNAL void filterByPropertyChanged();

    // clang-format off
    Q_PROPERTY(QVariantList filterValues
               READ filterValues
               WRITE setFilterValues
               NOTIFY filterValuesChanged)
    // clang-format on
    void setFilterValues(const QVariantList &val);
    QVariantList filterValues() const { return m_filterValues; }
    Q_SIGNAL void filterValuesChanged();

    enum FilterMode { IncludeFilterValues, ExcludeFilterValues };
    Q_ENUM(FilterMode)
    // clang-format off
    Q_PROPERTY(FilterMode filterMode
               READ filterMode
               WRITE setFilterMode
               NOTIFY filterModeChanged)
    // clang-format on
    void setFilterMode(FilterMode val);
    FilterMode filterMode() const { return m_filterMode; }
    Q_SIGNAL void filterModeChanged();

    // clang-format off
    Q_PROPERTY(QJSValue sortFunction
               READ sortFunction
               WRITE setSortFunction
               NOTIFY sortFunctionChanged)
    // clang-format on
    void setSortFunction(const QJSValue &val);
    QJSValue sortFunction() const { return m_sortFunction; }
    Q_SIGNAL void sortFunctionChanged();

    // clang-format off
    Q_PROPERTY(QJSValue filterFunction
               READ filterFunction
               WRITE setFilterFunction
               NOTIFY filterFunctionChanged)
    // clang-format on
    void setFilterFunction(const QJSValue &val);
    QJSValue filterFunction() const { return m_filterFunction; }
    Q_SIGNAL void filterFunctionChanged();

    // clang-format off
    Q_PROPERTY(QJSEngine* jsEngine
               READ jsEngine
               WRITE setJsEngine
               NOTIFY jsEngineChanged)
    // clang-format on
    void setJsEngine(QJSEngine *val);
    QJSEngine *jsEngine() const { return m_jsEngine; }
    Q_SIGNAL void jsEngineChanged();

    QHash<int, QByteArray> roleNames() const;

protected:
    // QSortFilterProxyModel interface
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    mutable QJSValue m_sortFunction;
    mutable QJSValue m_filterFunction;
    QJSEngine *m_jsEngine = nullptr;
    QVariantList m_filterValues;
    QByteArray m_sortByProperty;
    QByteArray m_filterByProperty;
    FilterMode m_filterMode = IncludeFilterValues;
};

template<class T>
inline QList<T> qobject_list_cast(const QList<QObject *> &list, bool deleteUncasedObjects = true)
{
    QList<T> ret;
    ret.reserve(list.size());
    for (QObject *ptr : list) {
        T item = qobject_cast<T>(ptr);
        if (item != nullptr)
            ret.append(item);
        else if (deleteUncasedObjects)
            ptr->deleteLater();
    }

    return ret;
}

#endif // OBJECTLISTMODEL_H
