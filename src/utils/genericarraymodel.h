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

#ifndef GENERICARRAYMODEL_H
#define GENERICARRAYMODEL_H

#include <QJsonArray>
#include <QJsonObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "qobjectproperty.h"

class GenericArrayModel : public QAbstractListModel
{
    Q_OBJECT

public:
    GenericArrayModel(QObject *parent=nullptr);
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

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    QJsonArray &internalArray() { return m_array; }

private:
    void processArray();

private:
    QJsonArray m_array;
    QStringList m_objectMembers;
};

class GenericArraySortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    GenericArraySortFilterProxyModel(QObject *parent=nullptr);
    ~GenericArraySortFilterProxyModel();

    Q_PROPERTY(GenericArrayModel* arrayModel READ arrayModel WRITE setArrayModel NOTIFY arrayModelChanged RESET resetArrayModel)
    void setArrayModel(GenericArrayModel* val);
    GenericArrayModel* arrayModel() const { return m_arrayModel; }
    Q_SIGNAL void arrayModelChanged();

    QHash<int, QByteArray> roleNames() const;

private:
    void resetArrayModel();

private:
    QObjectProperty<GenericArrayModel> m_arrayModel;
};

#endif // GENERICARRAYMODEL_H
