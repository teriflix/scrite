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

#ifndef TRACKPROPERTYCHANGES_H
#define TRACKPROPERTYCHANGES_H

#include "simpletimer.h"

#include <QObject>
#include <QQmlListProperty>

class AbstractObjectTracker : public QObject
{
    Q_OBJECT

public:
    ~AbstractObjectTracker();

    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged)
    void setTarget(QObject* val);
    QObject* target() const { return m_target; }
    Q_SIGNAL void targetChanged();

signals:
    void tracked();

protected:
    AbstractObjectTracker(QObject *parent=nullptr);
    virtual void init() { }

    QObject* m_target = nullptr;
};

class TrackProperty : public AbstractObjectTracker
{
    Q_OBJECT

public:
    TrackProperty(QObject *parent=nullptr);
    ~TrackProperty();

    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
    void setProperty(const QString &val);
    QString property() const { return m_property; }
    Q_SIGNAL void propertyChanged();

protected:
    void init();

private:
    QString m_property;
};

class TrackSignal : public AbstractObjectTracker
{
    Q_OBJECT

public:
    TrackSignal(QObject *parent=nullptr);
    ~TrackSignal();

    Q_PROPERTY(QString signal READ signal WRITE setSignal NOTIFY signalChanged)
    void setSignal(const QString &val);
    QString signal() const { return m_signal; }
    Q_SIGNAL void signalChanged();

protected:
    void init();

private:
    QString m_signal;
};

class TrackObject : public QObject
{
    Q_OBJECT

public:
    TrackObject(QObject *parent = nullptr);
    ~TrackObject();

    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    void setDelay(int val);
    int delay() const { return m_delay; }
    Q_SIGNAL void delayChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_CLASSINFO("DefaultProperty", "trackers")
    Q_PROPERTY(QQmlListProperty<AbstractObjectTracker> trackers READ trackers NOTIFY trackerCountChanged)
    QQmlListProperty<AbstractObjectTracker> trackers();
    Q_INVOKABLE void addTracker(AbstractObjectTracker *ptr);
    Q_INVOKABLE void removeTracker(AbstractObjectTracker *ptr);
    Q_INVOKABLE AbstractObjectTracker *trackerAt(int index) const;
    Q_PROPERTY(int trackerCount READ trackerCount NOTIFY trackerCountChanged)
    int trackerCount() const { return m_trackers.size(); }
    Q_INVOKABLE void clearTrackers();
    Q_SIGNAL void trackerCountChanged();

signals:
    void tracked();

protected:
    void timerEvent(QTimerEvent *event);
    void emitChangesTrackedSignal();

private:
    int m_delay = 0;
    bool m_enabled = true;
    SimpleTimer m_timer;
    bool m_emitCallsWhileDisabled = false;

    static void staticAppendTracker(QQmlListProperty<AbstractObjectTracker> *list, AbstractObjectTracker *ptr);
    static void staticClearTrackers(QQmlListProperty<AbstractObjectTracker> *list);
    static AbstractObjectTracker* staticTrackerAt(QQmlListProperty<AbstractObjectTracker> *list, int index);
    static int staticTrackerCount(QQmlListProperty<AbstractObjectTracker> *list);
    QList<AbstractObjectTracker *> m_trackers;
};

#endif // TRACKPROPERTYCHANGES_H
