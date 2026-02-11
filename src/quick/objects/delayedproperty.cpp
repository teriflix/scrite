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

#include "delayedproperty.h"

DelayedProperty::DelayedProperty(QQuickItem *parent) : QQuickItem(parent)
{
    this->setFlag(ItemHasContents, false);
    this->setVisible(false);
    connect(this, &QQuickItem::enabledChanged, this, &DelayedProperty::schedule);

#ifndef QT_NO_DEBUG_OUTPUT
    connect(this, &QQuickItem::parentChanged, this, &DelayedProperty::parentHasChanged);
    this->parentHasChanged();
#else
    m_timer.setName("DelayedProperty.m_timer");
#endif
}

DelayedProperty::~DelayedProperty() { }

DelayedPropertyAttached *DelayedProperty::qmlAttachedProperties(QObject *parent)
{
    return new DelayedPropertyAttached(parent);
}

void DelayedProperty::setName(const QString &val)
{
    if (m_name == val)
        return;

    m_name = val;

#ifndef QT_NO_DEBUG_OUTPUT
    if (m_name.isEmpty()) {
        connect(this, &QQuickItem::parentChanged, this, &DelayedProperty::parentHasChanged);
        this->parentHasChanged();
    } else {
        m_timer.setName("DelayedProperty.m_timer[" + m_name + "]");
        disconnect(this, &QQuickItem::parentChanged, this, &DelayedProperty::parentHasChanged);
    }
#else
    if (m_name.isEmpty())
        m_timer.setName("DelayedProperty.m_timer");
    else
        m_timer.setName("DelayedProperty.m_timer[" + m_name + "]");
#endif

    emit nameChanged();
}

void DelayedProperty::setSet(const QVariant &val)
{
    if (m_set == val)
        return;

    m_set = val;
    emit setChanged();

    this->schedule();
}

void DelayedProperty::setInitial(const QVariant &val)
{
    if (m_initial == val || m_initial.isValid())
        return;

    m_initial = val;
    emit initialChanged();

    this->setGet(val);
}

void DelayedProperty::setGet(const QVariant &val)
{
    if (m_get == val)
        return;

    m_get = val;
    emit getChanged();
}

void DelayedProperty::setDelay(int val)
{
    if (m_delay == val || val < 0 || val >= 10000)
        return;

    m_delay = val;
    emit delayChanged();

    this->schedule();
}

void DelayedProperty::schedule()
{
    if (!this->isEnabled())
        return;

    m_timer.start(m_delay, this);
}

void DelayedProperty::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timer.timerId()) {
        m_timer.stop();

        if (this->isEnabled())
            this->setGet(this->set());
    }
}

void DelayedProperty::parentHasChanged()
{
#ifndef QT_NO_DEBUG_OUTPUT
    if (m_name.isEmpty()) {
        const QQuickItem *parentItem = this->parentItem();
        const QMetaObject *parentMO = parentItem ? parentItem->metaObject() : nullptr;
        const QString name =
                parentMO ? QString::fromLatin1(parentMO->className()) : QStringLiteral("Unknown");
        m_timer.setName("DelayedProperty.m_timer[" + name + "]");
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

DelayedPropertyAttached::DelayedPropertyAttached(QObject *parent) : QObject(parent) { }

DelayedPropertyAttached::~DelayedPropertyAttached() { }

void DelayedPropertyAttached::setName(const QString &val)
{
    if (m_name == val)
        return;

    m_name = val;
    emit nameChanged();
}

void DelayedPropertyAttached::setWatch(const QVariant &val)
{
    if (m_watch == val)
        return;

    m_watch = val;
    emit watchChanged();

    if (m_initial.isValid()) {
        this->setValue(m_initial);
    }

    m_timer.start(m_delay, this);
}

void DelayedPropertyAttached::setDelay(int val)
{
    if (m_delay == val)
        return;

    m_delay = val;
    emit delayChanged();
}

void DelayedPropertyAttached::setInitial(const QVariant &val)
{
    if (m_initial == val)
        return;

    m_initial = val;
    emit initialChanged();
}

void DelayedPropertyAttached::setValue(const QVariant &val)
{
    if (m_value == val)
        return;

    m_value = val;
    emit valueChanged();
}

void DelayedPropertyAttached::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timer.timerId()) {
#ifndef QT_NO_DEBUG_OUTPUT
        if (!m_name.isEmpty()) {
            qDebug("DelayedPropertyAttached[%s]: Timer", qPrintable(m_name));
        }
#endif
        m_timer.stop();
        this->setValue(m_watch);
        return;
    }

    QObject::timerEvent(te);
}
