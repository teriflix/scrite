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

#include "trackobject.h"

#include <QMetaProperty>
#include <QTimerEvent>

AbstractObjectTracker::AbstractObjectTracker(QObject *parent)
    : QObject(parent)
{

}

AbstractObjectTracker::~AbstractObjectTracker()
{

}

void AbstractObjectTracker::setTarget(QObject *val)
{
    if(m_target == val)
        return;

    if(m_target != nullptr)
        disconnect(m_target, nullptr, this, nullptr);

    m_target = val;
    emit targetChanged();

    this->init();
}

///////////////////////////////////////////////////////////////////////////////

TrackProperty::TrackProperty(QObject *parent)
    : AbstractObjectTracker(parent)
{

}

TrackProperty::~TrackProperty()
{

}

void TrackProperty::setProperty(const QString &val)
{
    if(m_property == val)
        return;

    if(m_target != nullptr)
        disconnect(m_target, nullptr, this, nullptr);

    m_property = val;
    emit propertyChanged();

    this->init();
}

void TrackProperty::init()
{
    if(m_target == nullptr || m_property.isEmpty())
        return;

    static QMetaMethod trackedSignalMethod = QMetaMethod::fromSignal(&AbstractObjectTracker::tracked);

    const QMetaObject *mo = m_target->metaObject();
    const int propIndex = mo->indexOfProperty( qPrintable(m_property) );
    if(propIndex >= 0)
    {
        const QMetaProperty prop = mo->property(propIndex);
        if(prop.hasNotifySignal())
            connect(m_target, prop.notifySignal(), this, trackedSignalMethod);
    }
}

///////////////////////////////////////////////////////////////////////////////

TrackSignal::TrackSignal(QObject *parent)
    : AbstractObjectTracker(parent)
{

}

TrackSignal::~TrackSignal()
{

}

void TrackSignal::setSignal(const QString &val)
{
    if(m_signal == val)
        return;

    if(m_target != nullptr)
        disconnect(m_target, nullptr, this, nullptr);

    m_signal = val;
    emit signalChanged();

    this->init();
}

void TrackSignal::init()
{
    if(m_target == nullptr || m_signal.isEmpty())
        return;

    static QMetaMethod trackedSignalMethod = QMetaMethod::fromSignal(&AbstractObjectTracker::tracked);

    const QMetaObject *mo = m_target->metaObject();
    const int signalIndex = mo->indexOfSignal(qPrintable(m_signal));
    if(signalIndex >= 0)
    {
        const QMetaMethod signalMethod = mo->method(signalIndex);
        if(signalMethod.methodType() == QMetaMethod::Signal)
            connect(m_target, signalMethod, this, trackedSignalMethod);
    }
}

///////////////////////////////////////////////////////////////////////////////

TrackObject::TrackObject(QObject *parent)
    : QObject(parent)
{

}

TrackObject::~TrackObject()
{

}

void TrackObject::setDelay(int val)
{
    if(m_delay == val)
        return;

    m_delay = val;
    emit delayChanged();
}

void TrackObject::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    if(m_enabled && m_emitCallsWhileDisabled)
    {
        m_emitCallsWhileDisabled = false;
        emitChangesTrackedSignal();
    }
}

QQmlListProperty<AbstractObjectTracker> TrackObject::trackers()
{
    return QQmlListProperty<AbstractObjectTracker>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &TrackObject::staticAppendTracker,
                &TrackObject::staticTrackerCount,
                &TrackObject::staticTrackerAt,
                &TrackObject::staticClearTrackers);
}

void TrackObject::addTracker(AbstractObjectTracker *ptr)
{
    if(ptr == nullptr || m_trackers.indexOf(ptr) >= 0)
        return;

    connect(ptr, &AbstractObjectTracker::tracked, this, &TrackObject::emitChangesTrackedSignal);
    m_trackers.append(ptr);
    emit trackerCountChanged();
}

void TrackObject::removeTracker(AbstractObjectTracker *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_trackers.indexOf(ptr);
    if(index < 0)
        return ;

    disconnect(ptr, &AbstractObjectTracker::tracked, this, &TrackObject::emitChangesTrackedSignal);
    m_trackers.removeAt(index);
    emit trackerCountChanged();
}

AbstractObjectTracker *TrackObject::trackerAt(int index) const
{
    return index < 0 || index >= m_trackers.size() ? nullptr : m_trackers.at(index);
}

void TrackObject::clearTrackers()
{
    while(m_trackers.size())
        this->removeTracker(m_trackers.first());
}

void TrackObject::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_timer.timerId())
    {
        m_timer.stop();
        emit tracked();
    }
}

void TrackObject::emitChangesTrackedSignal()
{
    if(m_enabled)
        m_timer.start(m_delay, this);
    else
        m_emitCallsWhileDisabled = true;
}

void TrackObject::staticAppendTracker(QQmlListProperty<AbstractObjectTracker> *list, AbstractObjectTracker *ptr)
{
    reinterpret_cast< TrackObject* >(list->data)->addTracker(ptr);
}

void TrackObject::staticClearTrackers(QQmlListProperty<AbstractObjectTracker> *list)
{
    reinterpret_cast< TrackObject* >(list->data)->clearTrackers();
}

AbstractObjectTracker *TrackObject::staticTrackerAt(QQmlListProperty<AbstractObjectTracker> *list, int index)
{
    return reinterpret_cast< TrackObject* >(list->data)->trackerAt(index);
}

int TrackObject::staticTrackerCount(QQmlListProperty<AbstractObjectTracker> *list)
{
    return reinterpret_cast< TrackObject* >(list->data)->trackerCount();
}

