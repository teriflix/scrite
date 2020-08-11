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

#include <QPainter>
#include <QQuickItemGrabResult>

TightBoundingBoxEvaluator::TightBoundingBoxEvaluator(QObject *parent)
    : QObject(parent)
{

}

TightBoundingBoxEvaluator::~TightBoundingBoxEvaluator()
{

}

void TightBoundingBoxEvaluator::setPreviewScale(qreal val)
{
    if( qFuzzyCompare(m_previewScale, val) )
        return;

    m_previewScale = val;
    emit previewScaleChanged();
}

void TightBoundingBoxEvaluator::updatePreview()
{
    if(!m_preview.isNull())
        return;

    QSizeF boxSize = m_boundingBox.size();
    boxSize *= m_previewScale;
    if(boxSize.isEmpty())
        return;

    auto lessThan = [](TightBoundingBoxItem *e1, TightBoundingBoxItem *e2) -> bool {
        return e1->stackOrder() < e2->stackOrder();
    };
    std::sort(m_items.begin(), m_items.end(), lessThan);

    m_preview = QImage(boxSize.toSize(), QImage::Format_ARGB32);
    m_preview.fill(Qt::transparent);

    QTransform tx;
    tx.translate(-m_boundingBox.left(), -m_boundingBox.top());
    tx.scale(m_previewScale, m_previewScale);

    QPainter paint(&m_preview);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.setRenderHint(QPainter::SmoothPixmapTransform);
    paint.setTransform(tx);

    Q_FOREACH(TightBoundingBoxItem *item, m_items)
    {
        if(item->item() == nullptr)
            continue;

        const QRectF itemRect = item->itemRect();
        const QImage itemPreview = item->preview();

        if(item->isLivePreview() && !itemPreview.isNull())
            paint.drawImage(itemRect, itemPreview);
        else
        {
            paint.setPen( QPen(item->previewBorderColor()) );
            paint.setBrush( QBrush(item->previewFillColor()) );
            paint.drawRect(itemRect);
        }
    }

    paint.end();
}

void TightBoundingBoxEvaluator::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_evaluationTimer.timerId())
    {
        m_evaluationTimer.stop();
        this->evaluateNow();
    }
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
    connect(item, &TightBoundingBoxItem::previewUpdated, this, &TightBoundingBoxEvaluator::markPreviewDirty);
    m_items.append(item);
    this->evaluateLater();
}

void TightBoundingBoxEvaluator::removeItem(TightBoundingBoxItem *item)
{
    disconnect(item, &TightBoundingBoxItem::aboutToDestroy, this, &TightBoundingBoxEvaluator::removeItem);
    disconnect(item, &TightBoundingBoxItem::previewUpdated, this, &TightBoundingBoxEvaluator::markPreviewDirty);
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

void TightBoundingBoxEvaluator::markPreviewDirty()
{
    m_preview = QImage();
    emit previewNeedsUpdate();
}

///////////////////////////////////////////////////////////////////////////////

TightBoundingBoxItem::TightBoundingBoxItem(QObject *parent)
    : QObject(parent),
      m_item(qobject_cast<QQuickItem*>(parent)),
      m_updatePreviewTimer("TightBoundingBoxItem.updatePreviewTimer"),
      m_viewportItem(this, "viewportItem"),
      m_evaluator(this, "evaluator")
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

    this->updatePreviewLater();

    emit evaluatorChanged();
}

void TightBoundingBoxItem::setStackOrder(qreal val)
{
    if( qFuzzyCompare(m_stackOrder, val) )
        return;

    m_stackOrder = val;
    emit stackOrderChanged();

    this->updatePreviewLater();
}

void TightBoundingBoxItem::setPreviewFillColor(const QColor &val)
{
    if(m_previewFillColor == val)
        return;

    m_previewFillColor = val;
    emit previewFillColorChanged();

    this->updatePreviewLater();
}

void TightBoundingBoxItem::setPreviewBorderColor(const QColor &val)
{
    if(m_previewBorderColor == val)
        return;

    m_previewBorderColor = val;
    emit previewBorderColorChanged();

    this->updatePreviewLater();
}

void TightBoundingBoxItem::setLivePreview(bool val)
{
    if(m_livePreview == val)
        return;

    m_livePreview = val;
    emit livePreviewChanged();

    this->updatePreviewLater();
}

void TightBoundingBoxItem::setVisibilityMode(TightBoundingBoxItem::VisibilityMode val)
{
    if(m_visibilityMode == val)
        return;

    m_visibilityMode = val;
    emit visibilityModeChanged();

    this->determineVisibility();
}

void TightBoundingBoxItem::setViewportItem(QQuickItem *val)
{
    if(m_viewportItem == val)
        return;

    m_viewportItem = val;
    emit viewportItemChanged();

    this->determineVisibility();
}

void TightBoundingBoxItem::setViewportRect(const QRectF &val)
{
    if(m_viewportRect == val)
        return;

    m_viewportRect = val;
    emit viewportRectChanged();

    this->determineVisibility();
}

void TightBoundingBoxItem::markPreviewDirty()
{
    this->updatePreviewLater();
}

void TightBoundingBoxItem::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_updatePreviewTimer.timerId())
    {
        m_updatePreviewTimer.stop();
        this->updatePreview();
    }
}

void TightBoundingBoxItem::requestReevaluation()
{
    if(m_evaluator)
        m_evaluator->markDirty(this);

    this->updatePreviewLater();
}

void TightBoundingBoxItem::resetEvaluator()
{
    m_evaluator = nullptr;
    emit evaluatorChanged();
}

void TightBoundingBoxItem::resetViewportItem()
{
    m_viewportItem = nullptr;
    emit viewportItemChanged();

    this->determineVisibility();
}

void TightBoundingBoxItem::updatePreview()
{
    if(m_item == nullptr)
    {
        this->setPreview(QImage());
        return;
    }

    if(m_livePreview)
    {
        QSizeF previewSize( m_item->width(), m_item->height() );
        if(m_evaluator != nullptr)
            previewSize *= m_evaluator->previewScale();

        if(previewSize.isNull())
        {
            this->setPreview(QImage());
            return;
        }

        m_itemGrabResult = m_item->grabToImage(previewSize.toSize());
        connect(m_itemGrabResult.get(), &QQuickItemGrabResult::ready, [=]() {
            this->setPreview(m_itemGrabResult->image());
             m_itemGrabResult.clear();
        });
    }
    else
    {
        m_preview = QImage();
        emit previewUpdated();
    }
}

void TightBoundingBoxItem::updatePreviewLater()
{
    if(m_item != nullptr)
        m_updatePreviewTimer.start(500, this);
}

void TightBoundingBoxItem::setPreview(const QImage &image)
{
    if(image.isNull() && m_preview.isNull())
        return;

    m_preview = image;
    emit previewUpdated();
}

void TightBoundingBoxItem::determineVisibility()
{
    if(m_item == nullptr)
        return;

    QRectF itemRect(m_item->x(), m_item->y(), m_item->width(), m_item->height());
    if(!m_viewportItem.isNull())
    {
        const QPointF pos = m_item->mapToItem(m_viewportItem, QPointF(0,0));
        itemRect.moveTopLeft(pos);
    }

    bool visible = m_item->isVisible();

    switch(m_visibilityMode)
    {
    case AlwaysVisible:
        visible = true;
        break;
    case VisibleUponViewportIntersection:
        visible = m_viewportRect.isValid() && itemRect.isValid() ? m_viewportRect.intersects(itemRect) : true;
        break;
    case VisibleUponViewportContains:
        visible = m_viewportRect.isValid() && itemRect.isValid() ? m_viewportRect.contains(itemRect) : true;
        break;
    }

    m_item->setVisible(visible);
}

///////////////////////////////////////////////////////////////////////////////

TightBoundingBoxPreview::TightBoundingBoxPreview(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_evaluator(this, "evaluator")
{

}

TightBoundingBoxPreview::~TightBoundingBoxPreview()
{

}

void TightBoundingBoxPreview::setBackgroundColor(const QColor &val)
{
    if(m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();
}

void TightBoundingBoxPreview::setBackgroundOpacity(qreal val)
{
    if( qFuzzyCompare(m_backgroundOpacity, val) )
        return;

    m_backgroundOpacity = val;
    emit backgroundOpacityChanged();
}

void TightBoundingBoxPreview::setEvaluator(TightBoundingBoxEvaluator *val)
{
    if(m_evaluator == val)
        return;

    if(m_evaluator != nullptr)
        disconnect(m_evaluator, &TightBoundingBoxEvaluator::previewNeedsUpdate, this, &TightBoundingBoxPreview::redraw);

    m_evaluator = val;

    if(m_evaluator != nullptr)
        connect(m_evaluator, &TightBoundingBoxEvaluator::previewNeedsUpdate, this, &TightBoundingBoxPreview::redraw);

    emit evaluatorChanged();

    this->update();
}

void TightBoundingBoxPreview::paint(QPainter *painter)
{
#ifndef QT_NO_DEBUG
    qDebug("TightBoundingBoxPreview is painting");
#endif

    const QRectF itemRect(0, 0, this->width(), this->height());
    painter->setOpacity(m_backgroundOpacity);
    painter->fillRect(itemRect, m_backgroundColor);
    painter->setOpacity(1.0);

    if(m_evaluator != nullptr)
    {
        m_evaluator->updatePreview();

        const QImage preview = m_evaluator->preview();
        if(preview.isNull())
            return;

        QSizeF previewSize = preview.size();
        previewSize.scale(itemRect.size(), Qt::KeepAspectRatio);

        QRectF previewRect( QPointF(0,0), previewSize );
        // previewRect.moveCenter(itemRect.center());

        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawImage(previewRect, preview );
    }
}

void TightBoundingBoxPreview::resetEvaluator()
{
    m_evaluator = nullptr;
    emit evaluatorChanged();
}
