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

#include "simpletimer.h"
#include "application.h"

#include <QList>
#include <QThread>

#ifndef QT_NO_DEBUG
Q_GLOBAL_STATIC(QList<SimpleTimer*>, SimpleTimerList)
#endif

SimpleTimer *SimpleTimer::get(int timerId)
{
#ifndef QT_NO_DEBUG
    Q_FOREACH(SimpleTimer *timer, *SimpleTimerList)
    {
        if(timer->timerId() == timerId)
            return timer;
    }
#else
    Q_UNUSED(timerId)
#endif

    return nullptr;
}

SimpleTimer::SimpleTimer(const QString &name, QObject *parent)
    : QObject(parent), m_name(name)
{
#ifndef QT_NO_DEBUG
    SimpleTimerList->append(this);
#endif

    m_timer.setObjectName("SimpleTimer");
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &SimpleTimer::onTimeout);
}

SimpleTimer::~SimpleTimer()
{
    this->stop();

#ifndef QT_NO_DEBUG
    SimpleTimerList->removeOne(this);
#endif
}

void SimpleTimer::setName(const QString &val)
{
    if(m_name == val)
        return;

    m_name = val;
    emit nameChanged();
}

void SimpleTimer::start(int msec, QObject *object)
{
    if(m_timer.isActive())
        this->stop();

    if(object == nullptr)
        return;

    if(object != m_object)
    {
        if(object)
            disconnect(object, &QObject::destroyed, this, &SimpleTimer::onObjectDestroyed);

        m_object = object;

        if(m_object)
            connect(object, &QObject::destroyed, this, &SimpleTimer::onObjectDestroyed);
    }

    if(this->thread()->eventDispatcher() != nullptr)
    {
        m_timer.start(msec);
        m_timerId = m_timer.timerId();
    }
    else
        m_timerId = -1;
}

void SimpleTimer::onTimeout()
{
    if(m_object != nullptr && m_timerId >= 0)
    {
#ifndef QT_NO_DEBUG
        qDebug() << "Posting Timer " << m_timerId << " to " << m_object;
#endif
        qApp->postEvent(m_object, new QTimerEvent(m_timerId));
    }
}

void SimpleTimer::onObjectDestroyed(QObject *ptr)
{
    if(m_object == ptr)
    {
        m_object = nullptr;
        this->stop();
    }
}
