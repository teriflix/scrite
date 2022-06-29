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

#include "qobjectproperty.h"

#include <QMetaObject>

QObjectPropertyBase::QObjectPropertyBase(QObject *notify, const char *resettablePropertyName)
    : m_notify(notify)
{
    if (!m_notify.isNull()) {
        const QMetaObject *mo = m_notify->metaObject();
        const int resetPropertyIndex = mo->indexOfProperty(resettablePropertyName);
        if (resetPropertyIndex < 0)
            qWarning("Resettable property %s not found in %s", resettablePropertyName,
                     mo->className());
        else {
            const QMetaProperty prop = mo->property(resetPropertyIndex);
            m_resettableProperty = prop;
        }
    }
}

QObjectPropertyBase::~QObjectPropertyBase()
{
    m_notify = nullptr;
}

void QObjectPropertyBase::setPointer(QObject *pointer)
{
    if (m_pointer == pointer)
        return;

    if (m_pointer != nullptr)
        disconnect(m_pointer, &QObject::destroyed, this, &QObjectPropertyBase::objectDestroyed);

    m_pointer = pointer;

    if (m_pointer != nullptr)
        connect(m_pointer, &QObject::destroyed, this, &QObjectPropertyBase::objectDestroyed);
}

void QObjectPropertyBase::objectDestroyed(QObject *ptr)
{
    if (ptr == m_pointer) {
        if (!m_notify.isNull() && m_resettableProperty.isValid()) {
            if (m_resettableProperty.isResettable())
                m_resettableProperty.reset(m_notify);
            else if (m_resettableProperty.isWritable())
                m_resettableProperty.write(m_notify, QVariant(QMetaType::Nullptr, nullptr));
        }

        this->resetPointer();
        m_pointer = nullptr;

        if (!m_notify.isNull() && m_resettableProperty.isValid()) {
            if (!m_resettableProperty.isResettable() && !m_resettableProperty.isWritable()) {
                const QMetaMethod notifySignal = m_resettableProperty.notifySignal();
                if (notifySignal.parameterCount() == 0)
                    notifySignal.invoke(m_notify);
            }
        }
    }
}
