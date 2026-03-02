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

#ifndef ITEMPOSITIONMAPPER_H
#define ITEMPOSITIONMAPPER_H

#include <QTimer>
#include <QObject>
#include <QQuickItem>

#include "qobjectproperty.h"

class ItemPositionMapper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ItemPositionMapper(QObject *parent = nullptr);
    ~ItemPositionMapper();

    // clang-format off
    Q_PROPERTY(QPointF position
               READ position
               WRITE setPosition
               NOTIFY positionChanged)
    // clang-format on
    void setPosition(const QPointF &val);
    QPointF position() const { return m_position; }
    Q_SIGNAL void positionChanged();

    // clang-format off
    Q_PROPERTY(QQuickItem *from
               READ from
               WRITE setFrom
               NOTIFY fromChanged)
    // clang-format on
    void setFrom(QQuickItem *val);
    QQuickItem *from() const { return m_from; }
    Q_SIGNAL void fromChanged();

    // clang-format off
    Q_PROPERTY(QQuickItem *to
               READ to
               WRITE setTo
               NOTIFY toChanged)
    // clang-format on
    void setTo(QQuickItem *val);
    QQuickItem *to() const { return m_to; }
    Q_SIGNAL void toChanged();

    // clang-format off
    Q_PROPERTY(QPointF mappedPosition
               READ mappedPosition
               NOTIFY mappedPositionChanged)
    // clang-format on
    QPointF mappedPosition() const { return m_mappedPosition; }
    Q_SIGNAL void mappedPositionChanged();

private:
    void setMappedPosition(const QPointF &val);
    void resetTo();
    void resetFrom();

private:
    void trackFromItemMovement();
    void trackToItemMovement();
    void trackMovement(QQuickItem *item, QList<QObject *> &list);
    Q_SLOT void trackedObjectDestroyed(QObject *ptr);
    void recomputeMappedPosition();

private:
    QObjectProperty<QQuickItem> m_to;
    QObjectProperty<QQuickItem> m_from;
    QList<QObject *> m_toItemsBeingTracked;
    QList<QObject *> m_fromItemsBeingTracked;
    QPointF m_position;
    QPointF m_mappedPosition;
    QTimer m_recomputePositionTimer;
};

#endif // ITEMPOSITIONMAPPER_H
