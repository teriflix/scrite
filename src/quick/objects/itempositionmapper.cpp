/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "itempositionmapper.h"
#include "timeprofiler.h"
#include "application.h"

/**
 This class helps in aligning items with each other when they dont share
 a parent child relationship and are not siblings.
 */

ItemPositionMapper::ItemPositionMapper(QObject *parent)
    : QObject(parent), m_to(this, "to"), m_from(this, "from")
{
    m_recomputePositionTimer.setSingleShot(true);
    m_recomputePositionTimer.setInterval(0);
    connect(&m_recomputePositionTimer, &QTimer::timeout, this,
            &ItemPositionMapper::recomputeMappedPosition);
}

ItemPositionMapper::~ItemPositionMapper() { }

void ItemPositionMapper::setPosition(const QPointF &val)
{
    if (m_position == val)
        return;

    m_position = val;
    emit positionChanged();
}

void ItemPositionMapper::setFrom(QQuickItem *val)
{
    if (m_from == val)
        return;

    m_from = val;
    emit fromChanged();

    this->trackFromItemMovement();
}

void ItemPositionMapper::setTo(QQuickItem *val)
{
    if (m_to == val)
        return;

    m_to = val;
    emit toChanged();

    this->trackToItemMovement();
}

void ItemPositionMapper::setMappedPosition(const QPointF &val)
{
    if (m_mappedPosition == val)
        return;

    m_mappedPosition = val;
    emit mappedPositionChanged();
}

void ItemPositionMapper::resetTo()
{
    m_to = nullptr;
    this->trackToItemMovement();
    emit toChanged();

    this->setMappedPosition(QPointF());
}

void ItemPositionMapper::resetFrom()
{
    m_from = nullptr;
    this->trackFromItemMovement();
    emit fromChanged();

    this->setMappedPosition(QPointF());
}

void ItemPositionMapper::trackFromItemMovement()
{
    trackMovement(m_from, m_fromItemsBeingTracked);
}

void ItemPositionMapper::trackToItemMovement()
{
    trackMovement(m_to, m_toItemsBeingTracked);
}

void ItemPositionMapper::trackMovement(QQuickItem *item, QList<QObject *> &list)
{
    for (QObject *ptr : qAsConst(list)) {
        disconnect(ptr, SIGNAL(destroyed(QObject *)), this,
                   SLOT(trackedObjectDestroyed(QObject *)));
        disconnect(ptr, nullptr, &m_recomputePositionTimer, nullptr);
    }
    list.clear();

    if (item == nullptr)
        return;

    QQuickItem *ptr = item;
    while (ptr) {
        list.append(ptr);
        connect(ptr, &QQuickItem::destroyed, this, &ItemPositionMapper::trackedObjectDestroyed);

        connect(ptr, &QQuickItem::xChanged, &m_recomputePositionTimer, qOverload<>(&QTimer::start),
                Qt::QueuedConnection);
        connect(ptr, &QQuickItem::yChanged, &m_recomputePositionTimer, qOverload<>(&QTimer::start),
                Qt::QueuedConnection);
        connect(ptr, &QQuickItem::widthChanged, &m_recomputePositionTimer,
                qOverload<>(&QTimer::start), Qt::QueuedConnection);
        connect(ptr, &QQuickItem::heightChanged, &m_recomputePositionTimer,
                qOverload<>(&QTimer::start), Qt::QueuedConnection);
        connect(ptr, &QQuickItem::rotationChanged, &m_recomputePositionTimer,
                qOverload<>(&QTimer::start), Qt::QueuedConnection);
        connect(ptr, &QQuickItem::scaleChanged, &m_recomputePositionTimer,
                qOverload<>(&QTimer::start), Qt::QueuedConnection);
        connect(ptr, &QQuickItem::transformOriginChanged, &m_recomputePositionTimer,
                qOverload<>(&QTimer::start), Qt::QueuedConnection);
        connect(ptr, &QQuickItem::parentChanged, &m_recomputePositionTimer,
                qOverload<>(&QTimer::start), Qt::QueuedConnection);
        ptr = ptr->parentItem();
    }

    m_recomputePositionTimer.start();
}

void ItemPositionMapper::trackedObjectDestroyed(QObject *ptr)
{
    bool success = m_fromItemsBeingTracked.removeOne(ptr);
    success |= m_toItemsBeingTracked.removeOne(ptr);
    if (success)
        m_recomputePositionTimer.start();
}

void ItemPositionMapper::recomputeMappedPosition()
{
    if (!m_to.isNull() && !m_from.isNull())
        this->setMappedPosition(m_to->mapFromItem(m_from, m_position));
    else
        this->setMappedPosition(QPointF());
}
