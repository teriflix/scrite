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

#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QEvent>
#include <QObject>
#include <QQmlEngine>
#include <QJsonObject>

#include "qobjectproperty.h"

class EventFilter;

class EventFilterResult : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~EventFilterResult();

    Q_PROPERTY(bool filter READ filter WRITE setFilter NOTIFY filterChanged)
    void setFilter(bool val);
    bool filter() const { return m_filter; }
    Q_SIGNAL void filterChanged();

    Q_PROPERTY(bool acceptEvent READ acceptEvent WRITE setAcceptEvent NOTIFY acceptEventChanged)
    void setAcceptEvent(bool val);
    bool acceptEvent() const { return m_acceptEvent; }
    Q_SIGNAL void acceptEventChanged();

private:
    friend class EventFilter;
    EventFilterResult(QObject *parent = nullptr);

private:
    bool m_filter = false;
    bool m_acceptEvent = false;
};

class EventFilter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(EventFilter)

public:
    explicit EventFilter(QObject *parent = nullptr);
    ~EventFilter();

    static EventFilter *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool isActive() const { return m_active; }
    Q_SIGNAL void activeChanged();

    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged RESET resetTarget)
    void setTarget(QObject *val);
    QObject *target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    Q_PROPERTY(QList<int> events READ events WRITE setEvents NOTIFY eventsChanged)
    void setEvents(const QList<int> &val);
    QList<int> events() const { return m_events; }
    Q_SIGNAL void eventsChanged();

    Q_PROPERTY(bool acceptHoverEvents READ isAcceptHoverEvents WRITE setAcceptHoverEvents NOTIFY acceptHoverEventsChanged)
    void setAcceptHoverEvents(bool val);
    bool isAcceptHoverEvents() const;
    Q_SIGNAL void acceptHoverEventsChanged();

    // Forwards the event currently being filtered
    Q_INVOKABLE bool forwardEventTo(QObject *object);

    Q_SIGNAL void filter(QObject *object, const QJsonObject &event, EventFilterResult *result);

    enum Event {
        MouseButtonPress = QEvent::MouseButtonPress,
        MouseButtonRelease = QEvent::MouseButtonRelease,
        MouseButtonDblClick = QEvent::MouseButtonDblClick,
        MouseMove = QEvent::MouseMove,
        KeyPress = QEvent::KeyPress,
        KeyRelease = QEvent::KeyRelease,
        Shortcut = QEvent::Shortcut,
        ShortcutOverride = QEvent::ShortcutOverride,
        Wheel = QEvent::Wheel,
        HoverEnter = QEvent::HoverEnter,
        HoverLeave = QEvent::HoverLeave,
        HoverMove = QEvent::HoverMove,
        DragEnter = QEvent::DragEnter,
        DragMove = QEvent::DragMove,
        DragLeave = QEvent::DragLeave,
        Drop = QEvent::Drop
    };
    Q_ENUM(Event)

protected:
    void resetTarget();
    bool eventFilter(QObject *watched, QEvent *event);
    QEvent *cloneCurrentEvent() const;

private:
    bool m_active = true;
    QList<int> m_events;
    QEvent *m_currentEvent = nullptr;
    QObjectProperty<QObject> m_target;
};

#endif // EVENTFILTER_H
