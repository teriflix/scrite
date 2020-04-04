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
#include <QKeyEvent>
#include <QMetaObject>
#include <QMouseEvent>

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
    :QObject(parent), m_target(parent)
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

void EventFilter::setTarget(QObject *val)
{
    if(m_target == val || val == nullptr)
        return;

    if(m_target)
        m_target->removeEventFilter(this);

    m_target = val;

    if(m_target)
        m_target->installEventFilter(this);

    emit targetChanged();
}

void EventFilter::setEvents(const QList<int> &val)
{
    if(m_events == val)
        return;

    m_events = val;
    emit eventsChanged();
}

inline void packIntoJson(QMouseEvent *event, QJsonObject &object)
{
    auto pointToJson = [](const QPointF &pos) {
        QJsonObject ret;
        ret.insert("x", pos.x());
        ret.insert("y", pos.y());
        return ret;
    };

    object.insert("button", int(event->button()));
    object.insert("buttons", int(event->buttons()));
    object.insert("flags", int(event->flags()));
    object.insert("globalPos", pointToJson(event->globalPos()));
    object.insert("localPos", pointToJson(event->localPos()));
    object.insert("screenPos", pointToJson(event->screenPos()));
    object.insert("pos", pointToJson(event->pos()));
    object.insert("windowPos", pointToJson(event->windowPos()));
    object.insert("source", int(event->source()));
}

inline void packIntoJson(QKeyEvent *event, QJsonObject &object)
{
    object.insert("count", event->count());
    object.insert("isAutoRepeat", event->isAutoRepeat());
    object.insert("key", event->key());
    object.insert("modifiers", int(event->modifiers()));
    object.insert("text", event->text());
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

    switch(event->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        packIntoJson(static_cast<QMouseEvent*>(event), object);
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        packIntoJson(static_cast<QKeyEvent*>(event), object);
        break;
    default:
        break;
    }

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

