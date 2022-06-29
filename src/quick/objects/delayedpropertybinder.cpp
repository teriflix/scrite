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

#include "delayedpropertybinder.h"

DelayedPropertyBinder::DelayedPropertyBinder(QQuickItem *parent) : QQuickItem(parent)
{
    this->setFlag(ItemHasContents, false);
    this->setVisible(false);
    connect(this, &QQuickItem::enabledChanged, this, &DelayedPropertyBinder::schedule);

#ifndef QT_NO_DEBUG_OUTPUT
    connect(this, &QQuickItem::parentChanged, this, &DelayedPropertyBinder::parentHasChanged);
    this->parentHasChanged();
#else
    m_timer.setName("DelayedPropertyBinder.m_timer");
#endif
}

DelayedPropertyBinder::~DelayedPropertyBinder() { }

void DelayedPropertyBinder::setName(const QString &val)
{
    if (m_name == val)
        return;

    m_name = val;

#ifndef QT_NO_DEBUG_OUTPUT
    if (m_name.isEmpty()) {
        connect(this, &QQuickItem::parentChanged, this, &DelayedPropertyBinder::parentHasChanged);
        this->parentHasChanged();
    } else {
        m_timer.setName("DelayedPropertyBinder.m_timer[" + m_name + "]");
        disconnect(this, &QQuickItem::parentChanged, this,
                   &DelayedPropertyBinder::parentHasChanged);
    }
#else
    if (m_name.isEmpty())
        m_timer.setName("DelayedPropertyBinder.m_timer");
    else
        m_timer.setName("DelayedPropertyBinder.m_timer[" + m_name + "]");
#endif

    emit nameChanged();
}

void DelayedPropertyBinder::setSet(const QVariant &val)
{
    if (m_set == val)
        return;

    m_set = val;
    emit setChanged();

    this->schedule();
}

void DelayedPropertyBinder::setInitial(const QVariant &val)
{
    if (m_initial == val || m_initial.isValid())
        return;

    m_initial = val;
    emit initialChanged();

    this->setGet(val);
}

void DelayedPropertyBinder::setGet(const QVariant &val)
{
    if (m_get == val)
        return;

    m_get = val;
    emit getChanged();
}

void DelayedPropertyBinder::setDelay(int val)
{
    if (m_delay == val || val < 0 || val >= 10000)
        return;

    m_delay = val;
    emit delayChanged();

    this->schedule();
}

void DelayedPropertyBinder::schedule()
{
    if (!this->isEnabled())
        return;

    m_timer.start(m_delay, this);
}

void DelayedPropertyBinder::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timer.timerId()) {
        m_timer.stop();

        if (this->isEnabled())
            this->setGet(this->set());
    }
}

void DelayedPropertyBinder::parentHasChanged()
{
#ifndef QT_NO_DEBUG_OUTPUT
    if (m_name.isEmpty()) {
        const QQuickItem *parentItem = this->parentItem();
        const QMetaObject *parentMO = parentItem ? parentItem->metaObject() : nullptr;
        const QString name =
                parentMO ? QString::fromLatin1(parentMO->className()) : QStringLiteral("Unknown");
        m_timer.setName("DelayedPropertyBinder.m_timer[" + name + "]");
    }
#endif
}
