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

#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>
#include <QQmlEngine>
#include <QJsonObject>

#include "qobjectproperty.h"

class EventFilter;

class EventFilterResult : public QObject
{
    Q_OBJECT

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
    EventFilterResult(QObject *parent=nullptr);

private:
    bool m_filter = false;
    bool m_acceptEvent = false;
};

class EventFilter : public QObject
{
    Q_OBJECT

public:
    EventFilter(QObject *parent=nullptr);
    ~EventFilter();

    static EventFilter *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool isActive() const { return m_active; }
    Q_SIGNAL void activeChanged();

    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged RESET resetTarget)
    void setTarget(QObject* val);
    QObject* target() const { return m_target; }
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

Q_DECLARE_METATYPE(EventFilter*)
QML_DECLARE_TYPEINFO(EventFilter, QML_HAS_ATTACHED_PROPERTIES)

#endif // EVENTFILTER_H
