/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "eventfilter.h"

#include <QEvent>
#include <QtDebug>
#include <QMetaEnum>
#include <QMetaObject>

EventFilterResult::EventFilterResult(QObject *parent)
                  :QObject(parent),
                   m_filter(false),
                   m_acceptEvent(true)
{

}

EventFilterResult::~EventFilterResult()
{

}

void EventFilterResult::setFilter(bool val)
{
    if(m_filter == val)
        return;

    m_filter = val;
    emit filterChanged();
}

void EventFilterResult::setAcceptEvent(bool val)
{
    if(m_acceptEvent == val)
        return;

    m_acceptEvent = val;
    emit acceptEventChanged();
}

///////////////////////////////////////////////////////////////////////////////

EventFilter::EventFilter(QObject *parent)
    :QObject(parent)
{
    parent->installEventFilter(this);
}

EventFilter::~EventFilter()
{

}

EventFilter *EventFilter::qmlAttachedProperties(QObject *object)
{
    return new EventFilter(object);
}

void EventFilter::setEvents(const QList<int> &val)
{
    if(m_events == val)
        return;

    m_events = val;
    emit eventsChanged();
}

inline bool eventToJson(QEvent *event, QJsonObject &object)
{
    const QMetaObject *eventMetaObject = &QEvent::staticMetaObject;
    auto eventTypeAsString = [eventMetaObject](int value) {
        const int ei = eventMetaObject->indexOfEnumerator("Type");
        if(ei < 0)
            return QString::number(value);

        const QMetaEnum e = eventMetaObject->enumerator(ei);
        return QString::fromLatin1(e.valueToKey(value));
    };

    QString eventName = eventTypeAsString(event->type());
    if(eventName.isEmpty())
        eventName = QString::number(event->type()) + "-Event";

    object.insert("type", int(event->type()));
    object.insert("typeName", eventName);
    object.insert("spontaneous", event->spontaneous());
    return true;
}

bool EventFilter::eventFilter(QObject *watched, QEvent *event)
{
    const bool doFilter = m_events.isEmpty() || m_events.contains(event->type());

    if(doFilter)
    {
        EventFilterResult result;

        QJsonObject eventJson;
        eventToJson(event, eventJson);

        emit filter(watched, eventJson, &result);

        if(result.filter())
        {
            event->setAccepted(result.acceptEvent());
            return true;
        }
    }

    return false;
}

