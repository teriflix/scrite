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

#include "eventfilter.h"
#include "application.h"

#include <QEvent>
#include <QtDebug>
#include <QMetaEnum>
#include <QKeyEvent>
#include <QMetaObject>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMimeData>

// #define POST_CLONED_EVENTS_WHILE_FORWARDING

EventFilterResult::EventFilterResult(QObject *parent) : QObject(parent) { }

EventFilterResult::~EventFilterResult() { }

void EventFilterResult::setFilter(bool val)
{
    if (m_filter == val)
        return;

    m_filter = val;
    emit filterChanged();
}

void EventFilterResult::setAcceptEvent(bool val)
{
    if (m_acceptEvent == val)
        return;

    m_acceptEvent = val;
    emit acceptEventChanged();
}

///////////////////////////////////////////////////////////////////////////////

EventFilter::EventFilter(QObject *parent) : QObject(parent), m_target(this, "target")
{
    m_target = parent;

    if (m_active)
        parent->installEventFilter(this);
}

EventFilter::~EventFilter() { }

EventFilter *EventFilter::qmlAttachedProperties(QObject *object)
{
    return new EventFilter(object);
}

void EventFilter::setActive(bool val)
{
    if (m_active == val)
        return;

    m_active = val;
    emit activeChanged();

    if (m_target) {
        if (m_active)
            m_target->installEventFilter(this);
        else
            m_target->removeEventFilter(this);
    }
}

void EventFilter::setTarget(QObject *val)
{
    if (m_target == val || val == nullptr)
        return;

    if (m_target)
        m_target->removeEventFilter(this);

    m_target = val;

    if (m_target && m_active)
        m_target->installEventFilter(this);

    emit targetChanged();
}

void EventFilter::setEvents(const QList<int> &val)
{
    if (m_events == val)
        return;

    m_events = val;
    emit eventsChanged();

    QQuickItem *item = qobject_cast<QQuickItem *>(m_target);

    if (item != nullptr) {
        for (int event : { QEvent::DragEnter, QEvent::DragLeave, QEvent::DragMove, QEvent::Drop }) {
            if (val.contains(event)) {
                item->setFlag(QQuickItem::ItemAcceptsDrops);
                break;
            }
        }

        for (int event : { QEvent::HoverEnter, QEvent::HoverMove, QEvent::HoverLeave }) {
            if (val.contains(event)) {
                item->setAcceptHoverEvents(true);
                break;
            }
        }

        for (int event : { QEvent::MouseButtonPress, QEvent::MouseButtonRelease,
                           QEvent::MouseButtonDblClick }) {
            if (val.contains(event)) {
                item->setAcceptedMouseButtons(Qt::AllButtons);
                break;
            }
        }
    }
}

void EventFilter::setAcceptHoverEvents(bool val)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(m_target);
    if (item) {
        item->setAcceptHoverEvents(val);
        emit acceptHoverEventsChanged();
        return;
    }
}

bool EventFilter::isAcceptHoverEvents() const
{
    QQuickItem *item = qobject_cast<QQuickItem *>(m_target);
    if (item)
        return item->acceptHoverEvents();

    return false;
}

bool EventFilter::forwardEventTo(QObject *object)
{
    if (object == nullptr || object == m_target)
        return false;

        // We have to create a duplicate copy of the event and
        // put it into the queue.
#ifdef POST_CLONED_EVENTS_WHILE_FORWARDING
    if (m_currentEvent->type() == QEvent::NativeGesture)
        return qApp->sendEvent(object, m_currentEvent);

    QEvent *event = this->cloneCurrentEvent();
    if (event != nullptr) {
        qApp->postEvent(object, event);
        return true;
    }
#else
    return qApp->sendEvent(object, m_currentEvent);
#endif

    return false;
}

void EventFilter::resetTarget()
{
    m_target = nullptr;
    emit targetChanged();
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
    object.insert("modifiers", int(event->modifiers()));
    object.insert("controlModifier", event->modifiers() & Qt::ControlModifier ? true : false);
    object.insert("shiftModifier", event->modifiers() & Qt::ShiftModifier ? true : false);
    object.insert("altModifier", event->modifiers() & Qt::AltModifier ? true : false);
}

inline void packIntoJson(QKeyEvent *event, QJsonObject &object)
{
    object.insert("count", event->count());
    object.insert("isAutoRepeat", event->isAutoRepeat());
    object.insert("key", event->key());
    object.insert("modifiers", int(event->modifiers()));
    object.insert("text", event->text());
    object.insert("hasText", !event->text().isEmpty());
    object.insert("controlModifier", event->modifiers() & Qt::ControlModifier ? true : false);
    object.insert("shiftModifier", event->modifiers() & Qt::ShiftModifier ? true : false);
    object.insert("altModifier", event->modifiers() & Qt::AltModifier ? true : false);
}

inline void packIntoJson(QShortcutEvent *event, QJsonObject &object)
{
    object.insert("isAmbiguous", event->isAmbiguous());
    object.insert("key", event->key().toString());
    object.insert("shortcutId", event->shortcutId());
}

inline void packIntoJson(QWheelEvent *event, QJsonObject &object)
{
    auto pointToJson = [](const QPointF &pos) {
        QJsonObject ret;
        ret.insert("x", pos.x());
        ret.insert("y", pos.y());
        return ret;
    };

    const int delta = event->angleDelta().x() + event->angleDelta().y();
    const int orientation = event->angleDelta().x() > 0 ? Qt::Horizontal : Qt::Vertical;

    object.insert("modifiers", int(event->modifiers()));
    object.insert("pixelDelta", pointToJson(event->pixelDelta()));
    object.insert("angleDelta", pointToJson(event->angleDelta()));
    object.insert("delta", delta);
    object.insert("orientation", orientation);
    object.insert("pos", pointToJson(event->position()));
    object.insert("globalPos", pointToJson(event->globalPosition()));
    object.insert("buttons", int(event->buttons()));
    object.insert("phase", int(event->phase()));
    object.insert("inverted", event->inverted());
    object.insert("source", int(event->source()));
    object.insert("controlModifier", event->modifiers() & Qt::ControlModifier ? true : false);
    object.insert("shiftModifier", event->modifiers() & Qt::ShiftModifier ? true : false);
    object.insert("altModifier", event->modifiers() & Qt::AltModifier ? true : false);
}

inline void packIntoJson(QHoverEvent *event, QJsonObject &object)
{
    auto pointToJson = [](const QPointF &pos) {
        QJsonObject ret;
        ret.insert("x", pos.x());
        ret.insert("y", pos.y());
        return ret;
    };

    object.insert("pos", pointToJson(event->posF()));
    object.insert("oldPos", pointToJson(event->pos()));
}

inline void packDropEventIntoJson(QDropEvent *event, QJsonObject &object)
{
    auto pointToJson = [](const QPointF &pos) {
        QJsonObject ret;
        ret.insert("x", pos.x());
        ret.insert("y", pos.y());
        return ret;
    };

    object.insert("pos", pointToJson(event->posF()));

    const QMimeData *mimeData = event->mimeData();
    const QStringList formats = mimeData->formats();
    QJsonObject mimeDataJson;
    for (const QString &format : formats)
        mimeDataJson.insert(format, QString::fromLatin1(mimeData->data(format)));
    object.insert("mimeData", mimeDataJson);
}

inline void packIntoJson(QDragEnterEvent *event, QJsonObject &object)
{
    packDropEventIntoJson(event, object);
}

inline void packIntoJson(QDragMoveEvent *event, QJsonObject &object)
{
    packDropEventIntoJson(event, object);
}

inline void packIntoJson(QDragLeaveEvent *event, QJsonObject &object)
{
    Q_UNUSED(event)
    Q_UNUSED(object)
}

inline void packIntoJson(QDropEvent *event, QJsonObject &object)
{
    packDropEventIntoJson(event, object);
}

inline bool eventToJson(QEvent *event, QJsonObject &object)
{
    const QMetaObject *eventMetaObject = &QEvent::staticMetaObject;
    auto eventTypeAsString = [eventMetaObject](int value) {
        const int ei = eventMetaObject->indexOfEnumerator("Type");
        if (ei < 0)
            return QString::number(value);

        const QMetaEnum e = eventMetaObject->enumerator(ei);
        return QString::fromLatin1(e.valueToKey(value));
    };

    QString eventName = eventTypeAsString(event->type());
    if (eventName.isEmpty())
        eventName = QString::number(event->type()) + "-Event";

    object.insert("type", int(event->type()));
    object.insert("typeName", eventName);
    object.insert("spontaneous", event->spontaneous());

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        packIntoJson(static_cast<QMouseEvent *>(event), object);
        break;
    case QEvent::Shortcut:
        packIntoJson(static_cast<QShortcutEvent *>(event), object);
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::ShortcutOverride:
        packIntoJson(static_cast<QKeyEvent *>(event), object);
        break;
    case QEvent::Wheel:
        packIntoJson(static_cast<QWheelEvent *>(event), object);
        break;
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
        packIntoJson(static_cast<QHoverEvent *>(event), object);
        break;
    case QEvent::DragEnter:
        packIntoJson(static_cast<QDragEnterEvent *>(event), object);
        break;
    case QEvent::DragMove:
        packIntoJson(static_cast<QDragMoveEvent *>(event), object);
        break;
    case QEvent::DragLeave:
        packIntoJson(static_cast<QDragLeaveEvent *>(event), object);
        break;
    case QEvent::Drop:
        packIntoJson(static_cast<QDropEvent *>(event), object);
        break;
    default:
        break;
    }

    return true;
}

bool EventFilter::eventFilter(QObject *watched, QEvent *event)
{
    const bool doFilter = m_events.isEmpty() || m_events.contains(event->type());

    if (doFilter) {
        EventFilterResult result;

        QJsonObject eventJson;
        eventToJson(event, eventJson);

        m_currentEvent = event;
        emit filter(watched, eventJson, &result);
        m_currentEvent = nullptr;

        if (result.filter()) {
            if (QList<int>({ QEvent::DragEnter, QEvent::DragLeave, QEvent::DragMove, QEvent::Drop })
                        .contains(event->type())) {
                QDropEvent *dndEvent = static_cast<QDropEvent *>(event);
                if (result.acceptEvent())
                    dndEvent->acceptProposedAction();
                else
                    dndEvent->ignore();
            } else
                event->setAccepted(result.acceptEvent());
            return true;
        }
    }

    return false;
}

QEvent *EventFilter::cloneCurrentEvent() const
{
#ifdef POST_CLONED_EVENTS_WHILE_FORWARDING
    if (m_currentEvent == nullptr)
        return nullptr;

    // This function is called by forwardEventTo() method. This function is
    // expected to create a clone of the current event in m_currentEvent
    // and return a pointer to that.
    switch (m_currentEvent->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *me = static_cast<QMouseEvent *>(m_currentEvent);
        return new QMouseEvent(me->type(), me->localPos(), me->button(), me->buttons(),
                               me->modifiers());
    }
    case QEvent::Wheel: {
        QWheelEvent *we = static_cast<QWheelEvent *>(m_currentEvent);
        return new QWheelEvent(we->position(), we->globalPosition(), we->pixelDelta(),
                               we->angleDelta(), we->buttons(), we->modifiers(), we->phase(),
                               we->inverted(), Qt::MouseEventSynthesizedByApplication);
    }
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(m_currentEvent);
        return new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text(), ke->isAutoRepeat(),
                             ushort(ke->count()));
    }
    default:
        break;
    }
#endif

    return nullptr;
}
