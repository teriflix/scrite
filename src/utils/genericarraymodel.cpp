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

#include "genericarraymodel.h"

#include <QJSValue>
#include <QtDebug>

GenericArrayModel::GenericArrayModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(this, &GenericArrayModel::rowsInserted, this, &GenericArrayModel::countChanged);
    connect(this, &GenericArrayModel::rowsRemoved, this, &GenericArrayModel::countChanged);
    connect(this, &GenericArrayModel::modelReset, this, &GenericArrayModel::countChanged);
}

GenericArrayModel::~GenericArrayModel() { }

void GenericArrayModel::setArray(const QJsonArray &val)
{
    if (m_array == val)
        return;

    this->beginResetModel();
    m_array = val;
    this->endResetModel();

    emit arrayChanged();
}

bool GenericArrayModel::arrayHasObjects() const
{
    return m_array.isEmpty() ? false : m_array.at(0).isObject();
}

void GenericArrayModel::setObjectMembers(const QStringList &val)
{
    if (m_objectMembers == val)
        return;

    this->beginResetModel();
    m_objectMembers = val;
    this->endResetModel();
    emit objectMembersChanged();
}

QJsonObject GenericArrayModel::objectMemberRoles() const
{
    QJsonObject ret;

    for (const QString &objectMember : m_objectMembers)
        ret.insert(objectMember, this->objectMemberRole(objectMember));

    return ret;
}

QString GenericArrayModel::roleNameOf(int role) const
{
    return QString::fromLatin1(this->roleNames().value(role, QByteArrayLiteral("arrayItem")));
}

int GenericArrayModel::objectMemberRole(const QString &objectMember) const
{
    const int index = m_objectMembers.indexOf(objectMember);
    return index < 0 ? Qt::DisplayRole : (Qt::UserRole + index + 1);
}

QJsonArray GenericArrayModel::stringListArray(const QStringList &list) const
{
    QJsonArray ret;
    for (const QString &item : list)
        ret.append(QJsonValue(item.trimmed()));
    return ret;
}

QJsonArray GenericArrayModel::arrayFromCsv(const QString &text) const
{
    return this->stringListArray(text.split(",", Qt::SkipEmptyParts));
}

QJsonValue GenericArrayModel::at(int row) const
{
    return row < 0 || row >= m_array.size() ? QJsonValue() : m_array.at(row);
}

int GenericArrayModel::firstIndexOf(const QString &member, const QVariant &value) const
{
    for (int i = 0; i < m_array.size(); i++) {
        QVariant itemValue;

        const QJsonValue item = m_array.at(i);
        if (item.isArray()) {
            const QJsonArray array = item.toArray();

            bool ok = true;
            const int idx = member.toInt(&ok);
            if (ok && idx >= 0 && idx < array.size())
                itemValue = array.at(idx).toVariant();
            else
                itemValue = QVariant();
        } else if (item.isObject()) {
            const QJsonObject object = item.toObject();
            itemValue = object.value(member).toVariant();
        }

        if (itemValue == value)
            return i;
    }

    return -1;
}

int GenericArrayModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_array.size();
}

QVariant GenericArrayModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_array.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        return m_array.at(index.row());

    const QJsonObject item = m_array.at(index.row()).toObject();
    const int memberIndex = role - (Qt::UserRole + 1);
    if (memberIndex < 0 || memberIndex >= m_objectMembers.size())
        return QVariant();

    return item.value(m_objectMembers.at(memberIndex));
}

QHash<int, QByteArray> GenericArrayModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = QByteArrayLiteral("arrayItem");

    for (const QString &objectMember : m_objectMembers)
        roles[this->objectMemberRole(objectMember)] = objectMember.toLatin1();

    return roles;
}

///////////////////////////////////////////////////////////////////////////////

GenericArraySortFilterProxyModel::GenericArraySortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_arrayModel(this, "arrayModel")
{
}

GenericArraySortFilterProxyModel::~GenericArraySortFilterProxyModel() { }

void GenericArraySortFilterProxyModel::setArrayModel(GenericArrayModel *val)
{
    if (m_arrayModel == val)
        return;

    m_arrayModel = val;
    this->setSourceModel(val);
    emit arrayModelChanged();
}

QHash<int, QByteArray> GenericArraySortFilterProxyModel::roleNames() const
{
    if (m_arrayModel == nullptr)
        return QSortFilterProxyModel::roleNames();

    return m_arrayModel->roleNames();
}

void GenericArraySortFilterProxyModel::resetArrayModel()
{
    this->setArrayModel(nullptr);
}
