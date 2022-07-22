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

#ifndef QOBJECTLISTMODEL_H
#define QOBJECTLISTMODEL_H

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

public:
    explicit AbstractQObjectListModel(QObject *parent = nullptr);
    ~AbstractQObjectListModel() { }

    Q_PROPERTY(int objectCount READ objectCount NOTIFY objectCountChanged)
    virtual int objectCount() const = 0;
    Q_SIGNAL void objectCountChanged();

    /**
     * Handling dataChanged() signal in QML is confusing, because Connections
     * has its own dataChanged() signal.
     */
    Q_SIGNAL void dataChanged2(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    Q_INVOKABLE virtual QObject *objectAt(int row) const = 0;

    // QAbstractListModel implementation
    enum { ObjectItemRole = Qt::UserRole + 1, ModelDataRole };
    QHash<int, QByteArray> roleNames() const;
};

template<class T>
class QObjectListModel : public AbstractQObjectListModel
{
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
    T takeLast() const
    {
        T ptr = this->first();
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

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : m_list.size(); }
    QVariant data(const QModelIndex &index, int role) const
    {
        if (role == ObjectItemRole || role == ModelDataRole) {
            QObject *ptr = index.row() < 0 || index.row() >= m_list.size() ? nullptr
                                                                           : m_list.at(index.row());
            return QVariant::fromValue<QObject *>(ptr);
        }
        return QVariant();
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

private:
    QList<T> m_list;
};

class SortFilterObjectListModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SortFilterObjectListModel(QObject *parent = nullptr);
    ~SortFilterObjectListModel() { }

    Q_PROPERTY(int objectCount READ objectCount NOTIFY objectCountChanged)
    int objectCount() const { return this->rowCount(QModelIndex()); }
    Q_SIGNAL void objectCountChanged();

    Q_PROPERTY(QByteArray sortByProperty READ sortByProperty WRITE setSortByProperty NOTIFY
                       sortByPropertyChanged)
    void setSortByProperty(const QByteArray &val);
    QByteArray sortByProperty() const { return m_sortByProperty; }
    Q_SIGNAL void sortByPropertyChanged();

    Q_PROPERTY(QByteArray filterByProperty READ filterByProperty WRITE setFilterByProperty NOTIFY
                       filterByPropertyChanged)
    void setFilterByProperty(const QByteArray &val);
    QByteArray filterByProperty() const { return m_filterByProperty; }
    Q_SIGNAL void filterByPropertyChanged();

    Q_PROPERTY(QVariantList filterValues READ filterValues WRITE setFilterValues NOTIFY
                       filterValuesChanged)
    void setFilterValues(const QVariantList &val);
    QVariantList filterValues() const { return m_filterValues; }
    Q_SIGNAL void filterValuesChanged();

    enum FilterMode { IncludeFilterValues, ExcludeFilterValues };
    Q_ENUM(FilterMode)
    Q_PROPERTY(FilterMode filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)
    void setFilterMode(FilterMode val);
    FilterMode filterMode() const { return m_filterMode; }
    Q_SIGNAL void filterModeChanged();

    Q_PROPERTY(QJSValue sortFunction READ sortFunction WRITE setSortFunction NOTIFY
                       sortFunctionChanged)
    void setSortFunction(const QJSValue &val);
    QJSValue sortFunction() const { return m_sortFunction; }
    Q_SIGNAL void sortFunctionChanged();

    Q_PROPERTY(QJSValue filterFunction READ filterFunction WRITE setFilterFunction NOTIFY
                       filterFunctionChanged)
    void setFilterFunction(const QJSValue &val);
    QJSValue filterFunction() const { return m_filterFunction; }
    Q_SIGNAL void filterFunctionChanged();

    QHash<int, QByteArray> roleNames() const;

protected:
    // QSortFilterProxyModel interface
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    mutable QJSValue m_sortFunction;
    mutable QJSValue m_filterFunction;
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

#endif // QOBJECTLISTMODEL_H
