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

#include "enumerationmodel.h"

EnumerationModel::EnumerationModel(QObject *parent) : QAbstractListModel(parent) { }

EnumerationModel::~EnumerationModel() { }

void EnumerationModel::setObject(QObject *val)
{
    if (m_object == val)
        return;

    if (m_object)
        QObject::disconnect(m_object, &QObject::destroyed, this, &EnumerationModel::resetObject);

    if (val)
        QObject::connect(val, &QObject::destroyed, this, &EnumerationModel::resetObject);

    m_object = val;
    emit objectChanged();
}

void EnumerationModel::setEnumeration(const QString &val)
{
    if (m_enumeration == val)
        return;

    m_enumeration = val;
    emit enumerationChanged();
}

QString EnumerationModel::valueToKey(int value) const
{
    return m_metaEnum.isValid() ? QString::fromLatin1(m_metaEnum.valueToKey(value)) : QString();
}

int EnumerationModel::keyToValue(const QString &key) const
{
    if (!m_metaEnum.isValid())
        return -1;

    bool ok = false;
    const int value = m_metaEnum.keyToValue(qPrintable(key), &ok);

    if (ok)
        return value;

    return -1;
}

QHash<int, QByteArray> EnumerationModel::roleNames() const
{
    return { { KeyRole, QByteArrayLiteral("enumerationKey") },
             { ValueRole, QByteArrayLiteral("enumerationValue") } };
}

int EnumerationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant EnumerationModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    switch (role) {
    case KeyRole:
        return m_items[index.row()].key;
    case ValueRole:
        return m_items[index.row()].value;
    }

    return QVariant();
}

void EnumerationModel::resetObject()
{
    m_object = nullptr;
    m_enumeration.clear();

    this->clearModel();

    emit objectChanged();
    emit enumerationChanged();
}

void EnumerationModel::loadModel()
{
    if (m_object == nullptr || m_enumeration.isEmpty()) {
        this->clearModel();
        return;
    }

    const QMetaObject *mo = m_object->metaObject();
    m_metaEnum = mo->enumerator(mo->indexOfEnumerator(qPrintable(m_enumeration)));
    if (!m_metaEnum.isValid()) {
        this->clearModel();
        return;
    }

    this->beginResetModel();

    m_items.clear();

    for (int i = 0; i < m_metaEnum.keyCount(); i++) {
        Item item { QString::fromLatin1(m_metaEnum.key(i)), m_metaEnum.value(i) };
        m_items.append(item);
    }

    this->endResetModel();
}

void EnumerationModel::clearModel()
{
    if (!m_items.isEmpty()) {
        this->beginResetModel();
        m_items.clear();
        this->endResetModel();
    }
}
