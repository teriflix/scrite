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

#ifndef GENERICARRAYMODEL_H
#define GENERICARRAYMODEL_H

#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "booleanresult.h"
#include "qobjectproperty.h"

class GenericArrayModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GenericArrayModel(QObject *parent = nullptr);
    ~GenericArrayModel();

    // clang-format off
    Q_PROPERTY(QJsonArray array
               READ array
               WRITE setArray
               NOTIFY arrayChanged)
    // clang-format on
    void setArray(const QJsonArray &val);
    QJsonArray array() const { return m_array; }
    Q_SIGNAL void arrayChanged();

    // clang-format off
    Q_PROPERTY(bool arrayHasObjects
               READ arrayHasObjects
               NOTIFY arrayChanged)
    // clang-format on
    bool arrayHasObjects() const;

    // clang-format off
    Q_PROPERTY(QStringList objectMembers
               READ objectMembers
               WRITE setObjectMembers
               NOTIFY objectMembersChanged)
    // clang-format on
    void setObjectMembers(const QStringList &val);
    QStringList objectMembers() const { return m_objectMembers; }
    Q_SIGNAL void objectMembersChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject objectMemberRoles
               READ objectMemberRoles
               NOTIFY objectMembersChanged)
    // clang-format on
    QJsonObject objectMemberRoles() const;

    Q_INVOKABLE QString roleNameOf(int role) const;
    Q_INVOKABLE int objectMemberRole(const QString &objectMember) const;

    Q_INVOKABLE QJsonArray stringListArray(const QStringList &list) const;
    Q_INVOKABLE QJsonArray arrayFromCsv(const QString &text) const;

    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    // clang-format on
    int count() const { return m_array.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE QJsonValue at(int row) const;
    Q_INVOKABLE void clear() { this->setArray(QJsonArray()); }
    Q_INVOKABLE QJsonValue get(int row) const { return this->at(row); }
    Q_INVOKABLE bool append(const QJsonValue &value);
    Q_INVOKABLE bool insert(int row, const QJsonValue &value);
    Q_INVOKABLE bool remove(int row, int count);
    Q_INVOKABLE bool set(int row, const QJsonValue &value);
    Q_INVOKABLE bool setProperty(int row, const QString &member, const QVariant &value);

    Q_INVOKABLE int firstIndexOf(const QString &member, const QVariant &value) const;

    // clang-format off
    Q_PROPERTY(bool editable
               READ isEditable
               WRITE setEditable
               NOTIFY editableChanged)
    // clang-format on
    void setEditable(bool val);
    bool isEditable() const { return m_editable; }
    Q_SIGNAL void editableChanged();

    // QAbstractItemModel interface
    enum { ArrayItemRole = Qt::UserRole, FirstMemberRole };
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    QJsonArray &internalArray() { return m_array; }
    const QJsonArray &internalArray() const { return m_array; }

private:
    bool m_editable = false;
    QJsonArray m_array;
    QStringList m_objectMembers;
};

class GenericArraySortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GenericArraySortFilterProxyModel(QObject *parent = nullptr);
    ~GenericArraySortFilterProxyModel();

    // clang-format off
    Q_PROPERTY(GenericArrayModel *arrayModel
               READ arrayModel
               WRITE setArrayModel
               NOTIFY arrayModelChanged
               RESET resetArrayModel)
    // clang-format on
    void setArrayModel(GenericArrayModel *val);
    GenericArrayModel *arrayModel() const { return m_arrayModel; }
    Q_SIGNAL void arrayModelChanged();

    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void refilter() { this->invalidateFilter(); }
    Q_INVOKABLE void resort() { this->invalidate(); }

signals:
    void filterRow(int source_row, BooleanResult *result);
    void compare(int source_left, int source_right, BooleanResult *result);

protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private:
    void resetArrayModel();

private:
    QObjectProperty<GenericArrayModel> m_arrayModel;
};

class ModelDataChangedTracker
{
public:
    ModelDataChangedTracker(QAbstractItemModel *model) : m_model(model) { }
    ~ModelDataChangedTracker() { this->notify(); }

    int startRowIndex() const { return m_startRow; }
    int endRowIndex() const { return m_endRow; }

    void changeRow(int row)
    {
        if (m_startRow < 0 || m_endRow < 0) {
            m_startRow = row;
            m_endRow = row;
        } else {
            if (row - m_endRow > 1) {
                this->notify();
                m_startRow = row;
                m_endRow = row;
            } else
                m_endRow = row;
        }
    }

private:
    void notify()
    {
        if (m_startRow >= 0 && m_endRow >= 0) {
            const int r1 = qMin(m_startRow, m_endRow);
            const int r2 = qMax(m_startRow, m_endRow);
            const QModelIndex start = m_model->index(r1, 0);
            const QModelIndex end = m_model->index(r2, 0);
            emit m_model->dataChanged(start, end);
        }
        m_startRow = -1;
        m_endRow = -1;
    }

private:
    QAbstractItemModel *m_model = nullptr;
    int m_startRow = -1;
    int m_endRow = -1;
};

#endif // GENERICARRAYMODEL_H
