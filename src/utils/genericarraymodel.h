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

#ifndef GENERICARRAYMODEL_H
#define GENERICARRAYMODEL_H

#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "qobjectproperty.h"

class GenericArrayModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GenericArrayModel(QObject *parent = nullptr);
    ~GenericArrayModel();

    Q_PROPERTY(QJsonArray array READ array WRITE setArray NOTIFY arrayChanged)
    void setArray(const QJsonArray &val);
    QJsonArray array() const { return m_array; }
    Q_SIGNAL void arrayChanged();

    Q_PROPERTY(bool arrayHasObjects READ arrayHasObjects NOTIFY arrayChanged)
    bool arrayHasObjects() const;

    Q_PROPERTY(QStringList objectMembers READ objectMembers WRITE setObjectMembers NOTIFY objectMembersChanged)
    void setObjectMembers(const QStringList &val);
    QStringList objectMembers() const { return m_objectMembers; }
    Q_SIGNAL void objectMembersChanged();

    Q_PROPERTY(QJsonObject objectMemberRoles READ objectMemberRoles NOTIFY objectMembersChanged)
    QJsonObject objectMemberRoles() const;

    Q_INVOKABLE QString roleNameOf(int role) const;
    Q_INVOKABLE int objectMemberRole(const QString &objectMember) const;

    Q_INVOKABLE QJsonArray stringListArray(const QStringList &list) const;
    Q_INVOKABLE QJsonArray arrayFromCsv(const QString &text) const;

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_array.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE QJsonValue at(int row) const;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    QJsonArray &internalArray() { return m_array; }
    const QJsonArray &internalArray() const { return m_array; }

private:
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

    Q_PROPERTY(GenericArrayModel* arrayModel READ arrayModel WRITE setArrayModel NOTIFY arrayModelChanged RESET resetArrayModel)
    void setArrayModel(GenericArrayModel *val);
    GenericArrayModel *arrayModel() const { return m_arrayModel; }
    Q_SIGNAL void arrayModelChanged();

    QHash<int, QByteArray> roleNames() const;

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
