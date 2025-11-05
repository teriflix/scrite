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

EnumerationModel::EnumerationModel(QObject *parent) : QAbstractListModel(parent)
{
    connect(this, &EnumerationModel::objectChanged, this, &EnumerationModel::loadModel);
    connect(this, &EnumerationModel::enumerationChanged, this, &EnumerationModel::loadModel);
    connect(this, &QAbstractListModel::modelReset, this, &EnumerationModel::countChanged);
}

EnumerationModel::EnumerationModel(const QMetaObject *mo, const QString &enumName, QObject *parent)
    : QAbstractListModel(parent)
{
    m_metaObject = mo;
    m_enumeration = enumName;
    this->loadModel();

    connect(this, &EnumerationModel::objectChanged, this, &EnumerationModel::loadModel);
    connect(this, &EnumerationModel::enumerationChanged, this, &EnumerationModel::loadModel);
    connect(this, &EnumerationModel::ignoreListChanged, this, &EnumerationModel::loadModel);
    connect(this, &EnumerationModel::objectChanged, this, &EnumerationModel::classNameChanged);
    connect(this, &QAbstractListModel::modelReset, this, &EnumerationModel::countChanged);
}

EnumerationModel::~EnumerationModel() { }

void EnumerationModel::setIgnoreList(QList<int> val)
{
    if (m_ignoreList == val)
        return;

    m_ignoreList = val;
    emit ignoreListChanged();
}

void EnumerationModel::setClassName(const QString &val)
{
    const int typeId = QMetaType::type(qPrintable(val + "*"));
    const QMetaObject *mo =
            typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    if (mo != m_metaObject || m_object != nullptr) {
        if (m_object)
            QObject::disconnect(m_object, &QObject::destroyed, this,
                                &EnumerationModel::resetObject);

        m_metaObject = mo;
        emit objectChanged();
    }
}

QString EnumerationModel::className() const
{
    return m_metaObject ? QString::fromLatin1(m_metaObject->className()) : QString();
}

void EnumerationModel::setObject(QObject *val)
{
    if (m_object == val)
        return;

    if (m_object)
        QObject::disconnect(m_object, &QObject::destroyed, this, &EnumerationModel::resetObject);

    if (val)
        QObject::connect(val, &QObject::destroyed, this, &EnumerationModel::resetObject);

    m_object = val;
    m_metaObject = m_object ? m_object->metaObject() : nullptr;
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

QString EnumerationModel::valueToIcon(int value) const
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
                           [value](const Item &item) { return item.value == value; });
    if (it != m_items.end())
        return it->icon;

    return QString();
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

int EnumerationModel::indexOfValue(int value) const
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
                           [value](const Item &item) { return item.value == value; });
    return it != m_items.end() ? std::distance(m_items.begin(), it) : -1;
}

QHash<int, QByteArray> EnumerationModel::roleNames() const
{
    return { { KeyRole, QByteArrayLiteral("enumKey") },
             { ValueRole, QByteArrayLiteral("enumValue") },
             { IconRole, QByteArrayLiteral("enumIcon") } };
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
    case IconRole:
        return m_items[index.row()].icon;
    }

    return QVariant();
}

void EnumerationModel::componentComplete()
{
    m_componentComplete = true;
    this->loadModel();
}

void EnumerationModel::resetObject()
{
    m_object = nullptr;
    m_metaObject = nullptr;
    m_enumeration.clear();

    emit objectChanged();
    emit enumerationChanged();
}

void EnumerationModel::loadModel()
{
    if (!m_componentComplete)
        return;

    if (m_metaObject == nullptr || m_enumeration.isEmpty()) {
        this->clearModel();
        return;
    }

    const QMetaObject *mo = m_metaObject;
    m_metaEnum = mo->enumerator(mo->indexOfEnumerator(qPrintable(m_enumeration)));
    if (!m_metaEnum.isValid()) {
        this->clearModel();
        return;
    }

    this->beginResetModel();

    m_items.clear();

    auto queryEnumIcon = [=](const char *key) {
        const QString iconKey = QStringLiteral("%1.%2:icon").arg(m_metaEnum.name(), key);
        const QByteArray ciKey = iconKey.toLatin1();
        const int ciIndex = mo->indexOfClassInfo(ciKey.constData());
        if (ciIndex < 0)
            return QString();

        const QMetaClassInfo ci = mo->classInfo(ciIndex);
        return QString::fromLatin1(ci.value());
    };

    for (int i = 0; i < m_metaEnum.keyCount(); i++) {
        if (m_ignoreList.contains(m_metaEnum.value(i))
            || this->indexOfValue(m_metaEnum.value(i)) >= 0)
            continue;

        Item item { QString::fromLatin1(m_metaEnum.key(i)), m_metaEnum.value(i),
                    queryEnumIcon(m_metaEnum.key(i)) };
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
