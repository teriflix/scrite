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

#ifndef TRACKPROPERTYCHANGES_H
#define TRACKPROPERTYCHANGES_H

#include "execlatertimer.h"
#include "qobjectproperty.h"

#include <QQmlEngine>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QAbstractListModel>

class AbstractObjectTracker : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~AbstractObjectTracker();

    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged RESET resetTarget)
    void setTarget(QObject *val);
    QObject *target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    bool isInitialized() const { return m_initialized; }

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

signals:
    void tracked();
    void emitTracked();

protected:
    void onEmitTracked();
    void resetTarget();

protected:
    AbstractObjectTracker(QObject *parent = nullptr);
    virtual void init() { }

    bool m_enabled = true;
    bool m_initialized = true;
    QObjectProperty<QObject> m_target;
};

class TrackProperty : public AbstractObjectTracker
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TrackProperty(QObject *parent = nullptr);
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

class TrackModelRow : public AbstractObjectTracker
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TrackModelRow(QObject *parent = nullptr);
    ~TrackModelRow();

    Q_PROPERTY(int row READ row WRITE setRow NOTIFY rowChanged)
    void setRow(int val);
    int row() const { return m_row; }
    Q_SIGNAL void rowChanged();

    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
    void setRootIndex(const QModelIndex &val);
    QModelIndex rootIndex() const { return m_rootIndex; }
    Q_SIGNAL void rootIndexChanged();

    enum Event { RowAboutToRemove, RowRemoved, RowAboutToInsert, RowInserted };
    Q_ENUM(Event)
    Q_PROPERTY(Event event READ rowEvent WRITE setRowEvent NOTIFY eventChanged)
    void setRowEvent(Event val);
    Event rowEvent() const { return m_event; }
    Q_SIGNAL void eventChanged();

private:
    void init();
    void onRowsAboutToInsert(const QModelIndex &parent, int start, int end);
    void onRowsInserted();
    void onRowsAboutToDelete(const QModelIndex &parent, int start, int end);
    void onRowsDeleted();
    void onAboutToReset();
    void onReset();

private:
    int m_row = -1;
    Event m_event = RowAboutToRemove;
    QModelIndex m_rootIndex;

    enum Operation { None, Insert, Remove, Move };
    int m_operation = None;
    int m_start = -1;
    int m_end = -1;
    void resetOperation()
    {
        m_operation = None;
        m_start = -1;
        m_end = -1;
    }
};

class TrackSignal : public AbstractObjectTracker
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TrackSignal(QObject *parent = nullptr);
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

class TrackerPack : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TrackerPack(QObject *parent = nullptr);
    ~TrackerPack();

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
    ExecLaterTimer m_timer;
    bool m_emitCallsWhileDisabled = false;

    static void staticAppendTracker(QQmlListProperty<AbstractObjectTracker> *list,
                                    AbstractObjectTracker *ptr);
    static void staticClearTrackers(QQmlListProperty<AbstractObjectTracker> *list);
    static AbstractObjectTracker *staticTrackerAt(QQmlListProperty<AbstractObjectTracker> *list,
                                                  int index);
    static int staticTrackerCount(QQmlListProperty<AbstractObjectTracker> *list);
    QList<AbstractObjectTracker *> m_trackers;
};

#endif // TRACKPROPERTYCHANGES_H
