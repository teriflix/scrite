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

#ifndef TIGHTBOUNDINGBOX_H
#define TIGHTBOUNDINGBOX_H

#include "simpletimer.h"

#include <QRectF>
#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QPointer>

/**
 * QQuickItem::childrenRect() doesn't ever shrink, even though items have moved
 * inside the previously know childrenRect(). It only always expands. We need
 * a bounding box that can shrink for use with StructureView -> canvas. Tha'ts why this class.
 */
class TightBoundingBoxItem;
class TightBoundingBoxEvaluator : public QObject
{
    Q_OBJECT

public:
    TightBoundingBoxEvaluator(QObject *parent = nullptr);
    ~TightBoundingBoxEvaluator();

    Q_PROPERTY(QRectF boundingBox READ boundingBox NOTIFY boundingBoxChanged)
    QRectF boundingBox() const { return m_boundingBox; }
    Q_SIGNAL void boundingBoxChanged();

    Q_PROPERTY(qreal x READ x NOTIFY boundingBoxChanged)
    qreal x() const { return m_boundingBox.x(); }

    Q_PROPERTY(qreal y READ y NOTIFY boundingBoxChanged)
    qreal y() const { return m_boundingBox.y(); }

    Q_PROPERTY(qreal width READ width NOTIFY boundingBoxChanged)
    qreal width() const { return m_boundingBox.width(); }

    Q_PROPERTY(qreal height READ height NOTIFY boundingBoxChanged)
    qreal height() const { return m_boundingBox.height(); }

    Q_PROPERTY(qreal left READ left NOTIFY boundingBoxChanged)
    qreal left() const { return m_boundingBox.left(); }

    Q_PROPERTY(qreal top READ top NOTIFY boundingBoxChanged)
    qreal top() const { return m_boundingBox.top(); }

    Q_PROPERTY(qreal right READ right NOTIFY boundingBoxChanged)
    qreal right() const { return m_boundingBox.right(); }

    Q_PROPERTY(qreal bottom READ bottom NOTIFY boundingBoxChanged)
    qreal bottom() const { return m_boundingBox.bottom(); }

    Q_PROPERTY(QPointF center READ center NOTIFY boundingBoxChanged)
    QPointF center() const { return m_boundingBox.center(); }

protected:
    void timerEvent(QTimerEvent *event);

private:
    void setBoundingBox(const QRectF &val);

    void addItem(TightBoundingBoxItem *item);
    void removeItem(TightBoundingBoxItem* item);
    void markDirty(TightBoundingBoxItem *) { this->evaluateLater(); }
    void evaluateLater() { m_evaluationTimer.start(100, this); }
    void evaluateNow();

private:
    friend class TightBoundingBoxItem;
    QRectF m_boundingBox;
    SimpleTimer m_evaluationTimer;
    QList<TightBoundingBoxItem*> m_items;
};

class TightBoundingBoxItem : public QObject
{
    Q_OBJECT

public:
    TightBoundingBoxItem(QObject *parent=nullptr);
    ~TightBoundingBoxItem();
    Q_SIGNAL void aboutToDestroy(TightBoundingBoxItem *ptr);

    static TightBoundingBoxItem *qmlAttachedProperties(QObject *object);

    QQuickItem *item() const { return m_item; }
    QRectF itemRect() const { return m_item ? QRectF(m_item->x(), m_item->y(), m_item->width(), m_item->height()) : QRectF(); }

    Q_PROPERTY(TightBoundingBoxEvaluator* evaluator READ evaluator WRITE setEvaluator NOTIFY evaluatorChanged)
    void setEvaluator(TightBoundingBoxEvaluator* val);
    TightBoundingBoxEvaluator* evaluator() const { return m_evaluator; }
    Q_SIGNAL void evaluatorChanged();

private:
    void requestReevaluation();

private:
    QPointer<QQuickItem> m_item;
    QPointer<TightBoundingBoxEvaluator> m_evaluator;
};

Q_DECLARE_METATYPE(TightBoundingBoxItem*)
QML_DECLARE_TYPEINFO(TightBoundingBoxItem, QML_HAS_ATTACHED_PROPERTIES)

#endif // ITEMSBOUNDINGBOX_H
