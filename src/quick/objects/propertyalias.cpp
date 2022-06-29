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

#include "propertyalias.h"

PropertyAlias::PropertyAlias(QObject *parent)
    : QObject(parent), m_sourceObject(this, "sourceObject")
{
}

PropertyAlias::~PropertyAlias() { }

void PropertyAlias::setSourceObject(QObject *val)
{
    if (m_sourceObject == val)
        return;

    this->disconnectFromSource();
    m_sourceObject = val;
    this->connectToSource();

    emit sourceObjectChanged();
    emit valueChanged();
}

void PropertyAlias::setSourceProperty(const QByteArray &val)
{
    if (m_sourceProperty == val)
        return;

    this->disconnectFromSource();
    m_sourceProperty = val;
    this->connectToSource();

    emit sourcePropertyChanged();
    emit valueChanged();
}

void PropertyAlias::setValue(const QVariant &val)
{
    if (!m_sourceObject.isNull() && !m_sourceProperty.isEmpty())
        m_sourceObject->setProperty(m_sourceProperty, val);
}

QVariant PropertyAlias::value() const
{
    if (!m_sourceObject.isNull() && !m_sourceProperty.isEmpty())
        return m_sourceObject->property(m_sourceProperty);

    return QVariant();
}

void PropertyAlias::connectToSource()
{
    if (m_sourceObject.isNull())
        return;

    if (m_sourceProperty.isEmpty())
        return;

    int propIndex = m_sourceObject->metaObject()->indexOfProperty(m_sourceProperty);
    if (propIndex < 0)
        return;

    const QMetaProperty prop = m_sourceObject->metaObject()->property(propIndex);
    m_sourcePropertyNotify = prop.notifySignal();
    if (m_sourcePropertyNotify.isValid()) {
        const QMetaMethod signal = QMetaMethod::fromSignal(&PropertyAlias::valueChanged);
        m_sourceConnection = connect(m_sourceObject, m_sourcePropertyNotify, this, signal);
    }
}

void PropertyAlias::disconnectFromSource()
{
    if (m_sourceConnection)
        disconnect(m_sourceConnection);
    m_sourceConnection = QMetaObject::Connection();
    m_sourcePropertyNotify = QMetaMethod();
}
