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

#include "trackobject.h"

#include <QMetaProperty>
#include <QTimerEvent>

AbstractObjectTracker::AbstractObjectTracker(QObject *parent)
    : QObject(parent), m_target(this, "target")
{
    connect(this, &AbstractObjectTracker::emitTracked, this, &AbstractObjectTracker::onEmitTracked);
}

AbstractObjectTracker::~AbstractObjectTracker() { }

void AbstractObjectTracker::setTarget(QObject *val)
{
    if (m_target == val)
        return;

    if (m_target != nullptr)
        disconnect(m_target, nullptr, this, nullptr);

    m_target = val;
    emit targetChanged();

    this->init();
}

void AbstractObjectTracker::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void AbstractObjectTracker::classBegin()
{
    m_initialized = false;
}

void AbstractObjectTracker::componentComplete()
{
    m_initialized = true;
    this->init();
}

void AbstractObjectTracker::onEmitTracked()
{
    if (m_enabled)
        emit tracked();
}

void AbstractObjectTracker::resetTarget()
{
    this->setTarget(nullptr);
}

///////////////////////////////////////////////////////////////////////////////

TrackProperty::TrackProperty(QObject *parent) : AbstractObjectTracker(parent) { }

TrackProperty::~TrackProperty() { }

void TrackProperty::setProperty(const QString &val)
{
    if (m_property == val)
        return;

    if (m_target != nullptr)
        disconnect(m_target, nullptr, this, nullptr);

    m_property = val;
    emit propertyChanged();

    this->init();
}

void TrackProperty::init()
{
    if (m_target == nullptr || m_property.isEmpty() || !this->isInitialized())
        return;

    static QMetaMethod trackedSignalMethod =
            QMetaMethod::fromSignal(&AbstractObjectTracker::emitTracked);

    const QMetaObject *mo = m_target->metaObject();
    const int propIndex = mo->indexOfProperty(qPrintable(m_property));
    if (propIndex >= 0) {
        const QMetaProperty prop = mo->property(propIndex);
        if (prop.hasNotifySignal())
            connect(m_target, prop.notifySignal(), this, trackedSignalMethod);
    }
}

///////////////////////////////////////////////////////////////////////////////

TrackModelRow::TrackModelRow(QObject *parent) : AbstractObjectTracker(parent) { }

TrackModelRow::~TrackModelRow() { }

void TrackModelRow::setRow(int val)
{
    if (m_row == val)
        return;

    m_row = val;
    emit rowChanged();
}

void TrackModelRow::setRootIndex(const QModelIndex &val)
{
    if (m_rootIndex == val)
        return;

    m_rootIndex = val;
    emit rootIndexChanged();
}

void TrackModelRow::setRowEvent(TrackModelRow::Event val)
{
    if (m_event == val)
        return;

    m_event = val;
    emit eventChanged();
}

void TrackModelRow::init()
{
    if (m_target == nullptr || !this->isInitialized())
        return;

    QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(m_target);
    if (model == nullptr)
        return;

    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this,
            &TrackModelRow::onRowsAboutToInsert);
    connect(model, &QAbstractItemModel::rowsInserted, this, &TrackModelRow::onRowsInserted);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            &TrackModelRow::onRowsAboutToDelete);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &TrackModelRow::onRowsDeleted);
    connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &TrackModelRow::onAboutToReset);
    connect(model, &QAbstractItemModel::modelReset, this, &TrackModelRow::onReset);
}

void TrackModelRow::onRowsAboutToInsert(const QModelIndex &parent, int start, int end)
{
    if (parent != m_rootIndex)
        return;

    if (m_event == RowAboutToInsert && m_row >= start && m_row <= end)
        emitTracked();
    else {
        m_operation = Insert;
        m_start = start;
        m_end = end;
    }
}

void TrackModelRow::onRowsInserted()
{
    if (m_event == RowInserted && m_operation == Insert && m_row >= m_start && m_row <= m_end)
        emitTracked();

    this->resetOperation();
}

void TrackModelRow::onRowsAboutToDelete(const QModelIndex &parent, int start, int end)
{
    if (parent != m_rootIndex)
        return;

    if (m_event == RowAboutToRemove && m_row >= start && m_row <= end)
        emitTracked();
    else {
        m_operation = Remove;
        m_start = start;
        m_end = end;
    }
}

void TrackModelRow::onRowsDeleted()
{
    if (m_event == RowRemoved && m_operation == Remove && m_row >= m_start && m_row <= m_end)
        emitTracked();

    this->resetOperation();
}

void TrackModelRow::onAboutToReset()
{
    if (m_event == RowAboutToRemove && m_row >= 0)
        emitTracked();
}

void TrackModelRow::onReset()
{
    if (m_event == RowRemoved && m_row >= 0)
        emitTracked();

    this->resetOperation();
}

///////////////////////////////////////////////////////////////////////////////

TrackSignal::TrackSignal(QObject *parent) : AbstractObjectTracker(parent) { }

TrackSignal::~TrackSignal() { }

void TrackSignal::setSignal(const QString &val)
{
    if (m_signal == val)
        return;

    if (m_target != nullptr)
        disconnect(m_target, nullptr, this, nullptr);

    m_signal = val;
    emit signalChanged();

    this->init();
}

void TrackSignal::init()
{
    if (m_target == nullptr || m_signal.isEmpty() || !this->isInitialized())
        return;

    static QMetaMethod trackedSignalMethod =
            QMetaMethod::fromSignal(&AbstractObjectTracker::emitTracked);

    const QMetaObject *mo = m_target->metaObject();
    const int signalIndex = mo->indexOfSignal(qPrintable(m_signal));
    if (signalIndex >= 0) {
        const QMetaMethod signalMethod = mo->method(signalIndex);
        if (signalMethod.methodType() == QMetaMethod::Signal)
            connect(m_target, signalMethod, this, trackedSignalMethod);
    }
}

///////////////////////////////////////////////////////////////////////////////

TrackerPack::TrackerPack(QObject *parent) : QObject(parent) { }

TrackerPack::~TrackerPack() { }

void TrackerPack::setDelay(int val)
{
    if (m_delay == val)
        return;

    m_delay = val;
    emit delayChanged();
}

void TrackerPack::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    if (m_enabled && m_emitCallsWhileDisabled) {
        m_emitCallsWhileDisabled = false;
        emitChangesTrackedSignal();
    }
}

QQmlListProperty<AbstractObjectTracker> TrackerPack::trackers()
{
    return QQmlListProperty<AbstractObjectTracker>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &TrackerPack::staticAppendTracker, &TrackerPack::staticTrackerCount,
            &TrackerPack::staticTrackerAt, &TrackerPack::staticClearTrackers);
}

void TrackerPack::addTracker(AbstractObjectTracker *ptr)
{
    if (ptr == nullptr || m_trackers.indexOf(ptr) >= 0)
        return;

    connect(ptr, &AbstractObjectTracker::tracked, this, &TrackerPack::emitChangesTrackedSignal);
    m_trackers.append(ptr);
    emit trackerCountChanged();
}

void TrackerPack::removeTracker(AbstractObjectTracker *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_trackers.indexOf(ptr);
    if (index < 0)
        return;

    disconnect(ptr, &AbstractObjectTracker::tracked, this, &TrackerPack::emitChangesTrackedSignal);
    m_trackers.removeAt(index);
    emit trackerCountChanged();
}

AbstractObjectTracker *TrackerPack::trackerAt(int index) const
{
    return index < 0 || index >= m_trackers.size() ? nullptr : m_trackers.at(index);
}

void TrackerPack::clearTrackers()
{
    while (m_trackers.size())
        this->removeTracker(m_trackers.first());
}

void TrackerPack::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();
        emit tracked();
    }
}

void TrackerPack::emitChangesTrackedSignal()
{
    if (m_enabled)
        m_timer.start(m_delay, this);
    else
        m_emitCallsWhileDisabled = true;
}

void TrackerPack::staticAppendTracker(QQmlListProperty<AbstractObjectTracker> *list,
                                      AbstractObjectTracker *ptr)
{
    reinterpret_cast<TrackerPack *>(list->data)->addTracker(ptr);
}

void TrackerPack::staticClearTrackers(QQmlListProperty<AbstractObjectTracker> *list)
{
    reinterpret_cast<TrackerPack *>(list->data)->clearTrackers();
}

AbstractObjectTracker *TrackerPack::staticTrackerAt(QQmlListProperty<AbstractObjectTracker> *list,
                                                    int index)
{
    return reinterpret_cast<TrackerPack *>(list->data)->trackerAt(index);
}

int TrackerPack::staticTrackerCount(QQmlListProperty<AbstractObjectTracker> *list)
{
    return reinterpret_cast<TrackerPack *>(list->data)->trackerCount();
}
