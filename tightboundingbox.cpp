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

#include "tightboundingbox.h"
#include "tightboundingbox.h"

TightBoundingBoxEvaluator::TightBoundingBoxEvaluator(QObject *parent)
    : QObject(parent)
{

}

TightBoundingBoxEvaluator::~TightBoundingBoxEvaluator()
{

}

void TightBoundingBoxEvaluator::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_evaluationTimer.timerId())
        this->evaluateNow();
}

void TightBoundingBoxEvaluator::setBoundingBox(const QRectF &val)
{
    if(m_boundingBox == val)
        return;

    m_boundingBox = val;
    emit boundingBoxChanged();
}

void TightBoundingBoxEvaluator::addItem(TightBoundingBoxItem *item)
{
    connect(item, &TightBoundingBoxItem::aboutToDestroy, this, &TightBoundingBoxEvaluator::removeItem);
    m_items.append(item);
    this->evaluateLater();
}

void TightBoundingBoxEvaluator::removeItem(TightBoundingBoxItem *item)
{
    disconnect(item, &TightBoundingBoxItem::aboutToDestroy, this, &TightBoundingBoxEvaluator::removeItem);
    m_items.removeOne(item);
    this->evaluateLater();
}

void TightBoundingBoxEvaluator::evaluateNow()
{
    QRectF rect;
    Q_FOREACH(TightBoundingBoxItem *item, m_items)
    {
        if(item->item())
            rect |= item->itemRect();
    }

    this->setBoundingBox(rect);
}

TightBoundingBoxItem::TightBoundingBoxItem(QObject *parent)
    : QObject(parent),
      m_item(qobject_cast<QQuickItem*>(parent))
{
    if(m_item)
    {
        connect(m_item, &QQuickItem::xChanged, this, &TightBoundingBoxItem::requestReevaluation);
        connect(m_item, &QQuickItem::yChanged, this, &TightBoundingBoxItem::requestReevaluation);
        connect(m_item, &QQuickItem::widthChanged, this, &TightBoundingBoxItem::requestReevaluation);
        connect(m_item, &QQuickItem::heightChanged, this, &TightBoundingBoxItem::requestReevaluation);
    }
}

TightBoundingBoxItem::~TightBoundingBoxItem()
{
    m_item = nullptr;
    m_evaluator = nullptr;
    emit aboutToDestroy(this);
}

TightBoundingBoxItem *TightBoundingBoxItem::qmlAttachedProperties(QObject *object)
{
    return new TightBoundingBoxItem(object);
}

void TightBoundingBoxItem::setEvaluator(TightBoundingBoxEvaluator *val)
{
    if(m_evaluator == val)
        return;

    if(m_evaluator)
        m_evaluator->removeItem(this);

    m_evaluator = val;

    if(m_evaluator)
        m_evaluator->addItem(this);

    emit evaluatorChanged();
}

void TightBoundingBoxItem::requestReevaluation()
{
    if(m_evaluator)
        m_evaluator->markDirty(this);
}
