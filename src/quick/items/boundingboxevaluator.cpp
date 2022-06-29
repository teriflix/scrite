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

#include "application.h"
#include "timeprofiler.h"
#include "boundingboxevaluator.h"
#include "boundingboxevaluator.h"

#include <QBuffer>
#include <QPainter>
#include <QJsonDocument>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QtConcurrentMap>
#include <QQuickItemGrabResult>

BoundingBoxEvaluator::BoundingBoxEvaluator(QObject *parent) : QObject(parent)
{
    /**
      Preview updates are time consumping operations and hence are done
      on a background thread. Since we need such operations to run on
      a background thread, but sequentially, we need to dump them all
      into one single thread. That's why we are going to configure
      the thread pool to contain exactly one thread.
      */
    m_threadPool.setMaxThreadCount(1);
}

BoundingBoxEvaluator::~BoundingBoxEvaluator() { }

void BoundingBoxEvaluator::setMargin(qreal val)
{
    if (qFuzzyCompare(m_margin, val))
        return;

    m_margin = val;
    emit marginChanged();

    this->evaluateLater();
}

void BoundingBoxEvaluator::setInitialRect(const QRectF &val)
{
    if (m_initialRect == val)
        return;

    m_initialRect = val;
    emit initialRectChanged();

    this->evaluateLater();
}

void BoundingBoxEvaluator::setPreviewScale(qreal val)
{
    if (qFuzzyCompare(m_previewScale, val))
        return;

    m_previewScale = val;
    emit previewScaleChanged();
}

QPicture BoundingBoxEvaluator::preview() const
{
    QMutexLocker locker(&m_previewLock);
    return m_preview;
}

void BoundingBoxEvaluator::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_evaluationTimer.timerId()) {
        m_evaluationTimer.stop();
        this->evaluateNow();
    } else if (event->timerId() == m_updatePreviewTimer.timerId()) {
        m_updatePreviewTimer.stop();
        this->updatePreview();
    }
}

void BoundingBoxEvaluator::setBoundingBox(const QRectF &val)
{
    if (m_boundingBox == val)
        return;

    m_boundingBox = val;
    emit boundingBoxChanged();
}

void BoundingBoxEvaluator::addItem(BoundingBoxItem *item)
{
    connect(item, &BoundingBoxItem::aboutToDestroy, this, &BoundingBoxEvaluator::removeItem);
    connect(item, &BoundingBoxItem::previewUpdated, this, &BoundingBoxEvaluator::markPreviewDirty);
    m_items.append(item);
    this->evaluateLater();

    emit itemCountChanged();
}

void BoundingBoxEvaluator::removeItem(BoundingBoxItem *item)
{
    disconnect(item, &BoundingBoxItem::aboutToDestroy, this, &BoundingBoxEvaluator::removeItem);
    disconnect(item, &BoundingBoxItem::previewUpdated, this,
               &BoundingBoxEvaluator::markPreviewDirty);
    m_items.removeOne(item);
    this->evaluateLater();

    emit itemCountChanged();
}

void BoundingBoxEvaluator::evaluateNow()
{
    QRectF rect = m_initialRect;

    for (BoundingBoxItem *item : qAsConst(m_items)) {
        if (item->item())
            rect |= item->boundingRect();
    }

    rect.adjust(-m_margin, -m_margin, m_margin, m_margin);

    this->setBoundingBox(rect);
    this->markPreviewDirty();
}

void BoundingBoxEvaluator::updatePreview()
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug("BoundingBoxEvaluator is updating preview picture");
#endif
    if (!m_preview.isNull())
        return;

    const QString futureWatcherName = QStringLiteral("CreatePreviewPictureFuture");
    if (this->findChild<QFutureWatcherBase *>(futureWatcherName) != nullptr)
        return;

    /**
     * Previously we were spawning a thread to run a function which created the preview picture.
     * That seems like a rather bad idea, because the function (which runs in a thread) will
     * have to dereference BoundingBoxItem instances in m_items, which could have gotten
     * deleted by the time the preview is evaluated.
     *
     * To address that issue, we now serialize BoundingBoxItems into a JSON array and then
     * get a function to create preview-picture in a thread.
     */
    QList<QJsonObject> itemInfoArray;
    for (BoundingBoxItem *item : qAsConst(m_items))
        itemInfoArray << item->asJson();

    QFuture<QPicture> future = QtConcurrent::run(
            &m_threadPool,
            [=](const QList<QJsonObject> &items, const QRectF &bbox,
                const qreal scale) -> QPicture {
                QMutexLocker locker(&m_previewLock);
                return BoundingBoxEvaluator::createPreviewPicture(items, bbox, scale);
            },
            itemInfoArray, m_boundingBox, m_previewScale);
    QFutureWatcher<QPicture> *futureWatcher = new QFutureWatcher<QPicture>(this);
    futureWatcher->setObjectName(futureWatcherName);
    connect(futureWatcher, &QFutureWatcher<QPicture>::finished, this, [=]() {
        m_preview = future.result();
        futureWatcher->deleteLater();
        emit previewUpdated();
    });
    futureWatcher->setFuture(future);
}

QPicture BoundingBoxEvaluator::createPreviewPicture(const QList<QJsonObject> &items,
                                                    const QRectF &bbox, const qreal scale)
{
    // Make a copy, so we can sort the copy by stack order.
    QList<QJsonObject> itemsCopy = items;

    QSizeF boxSize = bbox.size();
    boxSize *= scale;
    if (boxSize.isEmpty())
        return QPicture();

    const QString stackOrderAttrib = QStringLiteral("stackOrder");

    auto lessThan = [stackOrderAttrib](const QJsonObject &e1, const QJsonObject &e2) -> bool {
        return e1.value(stackOrderAttrib).toDouble() < e2.value(stackOrderAttrib).toDouble();
    };
    std::sort(itemsCopy.begin(), itemsCopy.end(), lessThan);

    QPicture picture;
    picture.setBoundingRect(QRectF(QPointF(0, 0), boxSize).toRect());

    QTransform tx;
    tx.translate(-bbox.left(), -bbox.top());
    tx.scale(scale, scale);

    QPainter paint(&picture);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.setRenderHint(QPainter::SmoothPixmapTransform);
    paint.setTransform(tx);

    auto rectFromJson = [](const QJsonObject &jsRect) {
        return QRectF(jsRect.value(QStringLiteral("x")).toDouble(),
                      jsRect.value(QStringLiteral("y")).toDouble(),
                      jsRect.value(QStringLiteral("width")).toDouble(),
                      jsRect.value(QStringLiteral("height")).toDouble());
    };

    auto imageFromJson = [](const QJsonValue &value) {
        if (value.isNull() || value.isUndefined())
            return QImage();

        const QByteArray base64 = value.toString().toLatin1();
        QByteArray bytes = QByteArray::fromBase64(base64);
        QBuffer buffer(&bytes);
        buffer.open(QFile::ReadOnly);

        QImage ret;
        ret.load(&buffer, "PNG");

        return ret;
    };

    for (const QJsonObject &item : qAsConst(itemsCopy)) {
        const QRectF itemRect = rectFromJson(item.value(QStringLiteral("boundingRect")).toObject());
        const bool isLivePreview = item.value(QStringLiteral("livePreview")).toBool();
        const QImage itemPreview =
                isLivePreview ? imageFromJson(item.value(QStringLiteral("preview"))) : QImage();
        const QColor previewBorderColor(
                item.value(QStringLiteral("previewBorderColor")).toString());
        const QColor previewFillColor(item.value(QStringLiteral("previewFillColor")).toString());
        const qreal previewBorderWidth =
                item.value(QStringLiteral("previewBorderWidth")).toDouble();

        if (isLivePreview && !itemPreview.isNull())
            paint.drawImage(itemRect, itemPreview);
        else if (previewBorderColor.alpha() > 0 || previewFillColor.alpha() > 0) {
            QPen pen(previewBorderColor);
            pen.setCosmetic(true);
            pen.setWidthF(previewBorderWidth);

            paint.setPen(pen);
            paint.setBrush(QBrush(previewFillColor));
            paint.drawRect(itemRect);
        }
    }

    paint.end();

    return picture;
}

void BoundingBoxEvaluator::markPreviewDirty()
{
    m_preview = QPicture();
    m_updatePreviewTimer.start(100, this);
}

///////////////////////////////////////////////////////////////////////////////

BoundingBoxItem::BoundingBoxItem(QObject *parent)
    : QObject(parent),
      m_item(qobject_cast<QQuickItem *>(parent)),
      m_updatePreviewTimer("BoundingBoxItem.updatePreviewTimer"),
      m_viewportItem(this, "viewportItem"),
      m_evaluator(this, "evaluator")
{
    if (m_item) {
        connect(m_item, &QQuickItem::xChanged, this, &BoundingBoxItem::requestReevaluation);
        connect(m_item, &QQuickItem::yChanged, this, &BoundingBoxItem::requestReevaluation);
        connect(m_item, &QQuickItem::widthChanged, this, &BoundingBoxItem::requestReevaluation);
        connect(m_item, &QQuickItem::heightChanged, this, &BoundingBoxItem::requestReevaluation);

        connect(m_item, &QQuickItem::xChanged, &m_jsonUpdateTimer, QOverload<>::of(&QTimer::start));
        connect(m_item, &QQuickItem::yChanged, &m_jsonUpdateTimer, QOverload<>::of(&QTimer::start));
        connect(m_item, &QQuickItem::widthChanged, &m_jsonUpdateTimer,
                QOverload<>::of(&QTimer::start));
        connect(m_item, &QQuickItem::heightChanged, &m_jsonUpdateTimer,
                QOverload<>::of(&QTimer::start));

        connect(m_item, &QQuickItem::xChanged, this, &BoundingBoxItem::determineVisibility);
        connect(m_item, &QQuickItem::yChanged, this, &BoundingBoxItem::determineVisibility);
        connect(m_item, &QQuickItem::widthChanged, this, &BoundingBoxItem::determineVisibility);
        connect(m_item, &QQuickItem::heightChanged, this, &BoundingBoxItem::determineVisibility);
    }

    connect(this, &BoundingBoxItem::stackOrderChanged, &m_jsonUpdateTimer,
            QOverload<>::of(&QTimer::start));
    connect(this, &BoundingBoxItem::previewFillColorChanged, &m_jsonUpdateTimer,
            QOverload<>::of(&QTimer::start));
    connect(this, &BoundingBoxItem::previewBorderColorChanged, &m_jsonUpdateTimer,
            QOverload<>::of(&QTimer::start));
    connect(this, &BoundingBoxItem::previewBorderWidthChanged, &m_jsonUpdateTimer,
            QOverload<>::of(&QTimer::start));
    connect(this, &BoundingBoxItem::livePreviewChanged, &m_jsonUpdateTimer,
            QOverload<>::of(&QTimer::start));
    connect(this, &BoundingBoxItem::previewUpdated, &m_jsonUpdateTimer,
            QOverload<>::of(&QTimer::start));

    m_jsonUpdateTimer.setInterval(0);
    m_jsonUpdateTimer.setSingleShot(true);
    connect(&m_jsonUpdateTimer, &QTimer::timeout, this, [=]() { m_json = this->toJson(); });
}

BoundingBoxItem::~BoundingBoxItem()
{
    m_item = nullptr;
    m_evaluator = nullptr;
    emit aboutToDestroy(this);
}

BoundingBoxItem *BoundingBoxItem::qmlAttachedProperties(QObject *object)
{
    return new BoundingBoxItem(object);
}

QRectF BoundingBoxItem::boundingRect() const
{
    return m_item ? QRectF(m_item->x(), m_item->y(), m_item->width(), m_item->height()) : QRectF();
}

void BoundingBoxItem::setEvaluator(BoundingBoxEvaluator *val)
{
    if (m_evaluator == val)
        return;

    if (m_evaluator)
        m_evaluator->removeItem(this);

    m_evaluator = val;

    if (m_evaluator)
        m_evaluator->addItem(this);

    this->updatePreviewLater();

    emit evaluatorChanged();
}

void BoundingBoxItem::setStackOrder(qreal val)
{
    if (qFuzzyCompare(m_stackOrder, val))
        return;

    m_stackOrder = val;
    emit stackOrderChanged();

    this->updatePreviewLater();
}

void BoundingBoxItem::setPreviewFillColor(const QColor &val)
{
    if (m_previewFillColor == val)
        return;

    m_previewFillColor = val;
    emit previewFillColorChanged();

    this->updatePreviewLater();
}

void BoundingBoxItem::setPreviewBorderColor(const QColor &val)
{
    if (m_previewBorderColor == val)
        return;

    m_previewBorderColor = val;
    emit previewBorderColorChanged();

    this->updatePreviewLater();
}

void BoundingBoxItem::setPreviewBorderWidth(qreal val)
{
    if (qFuzzyCompare(m_previewBorderWidth, val))
        return;

    m_previewBorderWidth = val;
    emit previewBorderWidthChanged();

    this->updatePreviewLater();
}

void BoundingBoxItem::setLivePreview(bool val)
{
    if (m_livePreview == val)
        return;

    m_livePreview = val;
    emit livePreviewChanged();

    this->updatePreviewLater();
}

void BoundingBoxItem::setVisibilityMode(BoundingBoxItem::VisibilityMode val)
{
    if (m_visibilityMode == val)
        return;

    m_visibilityMode = val;
    emit visibilityModeChanged();

    this->determineVisibility();
}

void BoundingBoxItem::setViewportItem(QQuickItem *val)
{
    if (m_viewportItem == val)
        return;

    m_viewportItem = val;
    emit viewportItemChanged();

    this->determineVisibility();
}

void BoundingBoxItem::setViewportRect(const QRectF &val)
{
    if (m_viewportRect == val)
        return;

    m_viewportRect = val;
    emit viewportRectChanged();

    this->determineVisibility();
}

void BoundingBoxItem::setVisibilityProperty(const QByteArray &val)
{
    if (m_visibilityProperty == val)
        return;

    m_visibilityProperty = val;
    emit visibilityPropertyChanged();

    this->determineVisibility();
}

void BoundingBoxItem::markPreviewDirty()
{
    this->updatePreviewLater();
}

QJsonObject BoundingBoxItem::toJson() const
{
    QJsonObject ret;

    ret.insert(QStringLiteral("stackOrder"), m_stackOrder);
    ret.insert(QStringLiteral("previewFillColor"), m_previewFillColor.name(QColor::HexArgb));
    ret.insert(QStringLiteral("previewBorderColor"), m_previewBorderColor.name(QColor::HexArgb));
    ret.insert(QStringLiteral("previewBorderWidth"), m_previewBorderWidth);
    ret.insert(QStringLiteral("livePreview"), m_livePreview);

    const QRectF bRect = this->boundingRect();

    QJsonObject jsRect;
    jsRect.insert(QStringLiteral("x"), bRect.x());
    jsRect.insert(QStringLiteral("y"), bRect.y());
    jsRect.insert(QStringLiteral("width"), bRect.width());
    jsRect.insert(QStringLiteral("height"), bRect.height());
    ret.insert(QStringLiteral("boundingRect"), jsRect);

    if (m_livePreview) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        m_preview.save(&buffer, "PNG"); // writes image into ba in PNG format
        bytes = bytes.toBase64();
        ret.insert(QStringLiteral("preview"), QString::fromLatin1(bytes));
    }

    return ret;
}

void BoundingBoxItem::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updatePreviewTimer.timerId()) {
        m_updatePreviewTimer.stop();
        this->updatePreview();
    }
}

void BoundingBoxItem::requestReevaluation()
{
    if (m_evaluator)
        m_evaluator->markDirty(this);

    this->updatePreviewLater();
}

void BoundingBoxItem::resetEvaluator()
{
    m_evaluator = nullptr;
    emit evaluatorChanged();
}

void BoundingBoxItem::resetViewportItem()
{
    m_viewportItem = nullptr;
    emit viewportItemChanged();

    this->determineVisibility();
}

void BoundingBoxItem::updatePreview()
{
    if (m_item == nullptr) {
        this->setPreview(QImage());
        return;
    }

    if (m_livePreview) {
        QSizeF previewSize(m_item->width(), m_item->height());
        if (m_evaluator != nullptr)
            previewSize *= m_evaluator->previewScale();

        if (previewSize.isNull()) {
            this->setPreview(QImage());
            return;
        }

        m_itemGrabResult = m_item->grabToImage(previewSize.toSize());
        connect(m_itemGrabResult.get(), &QQuickItemGrabResult::ready, this, [=]() {
            this->setPreview(m_itemGrabResult->image());
            m_itemGrabResult.clear();
        });
    } else {
        if (!m_preview.isNull())
            m_preview = QImage();

        emit previewUpdated();
    }
}

void BoundingBoxItem::updatePreviewLater()
{
    if (m_item != nullptr) {
        if (!m_livePreview && m_preview.isNull()) {
            m_updatePreviewTimer.stop();
            emit previewUpdated();
        } else
            m_updatePreviewTimer.start(500, this);
    }
}

void BoundingBoxItem::setPreview(const QImage &image)
{
    if (image.isNull() && m_preview.isNull())
        return;

    m_preview = image;
    emit previewUpdated();
}

void BoundingBoxItem::determineVisibility()
{
    if (m_item == nullptr || m_visibilityMode == IgnoreVisibility)
        return;

    if (m_visibilityMode == AlwaysVisible || m_visibilityMode == AlwaysInvisible) {
        if (m_visibilityProperty.isEmpty())
            m_item->setVisible(m_visibilityMode == AlwaysVisible);
        else
            m_item->setProperty(m_visibilityProperty, m_visibilityMode == AlwaysVisible);
        return;
    }

    QRectF itemRect(m_item->x(), m_item->y(), m_item->width(), m_item->height());
    if (!m_viewportItem.isNull()) {
        /**
          QQuickItem::mapToItem() is slightly time-consuming function call. Maybe a good idea
          to avoid calling it if we can avoid. If the items parent is same as viewport, then
          item's position is its position with respect to the parent.
          */
        if (m_item->parentItem() != m_viewportItem) {
            const QPointF pos = m_item->mapToItem(m_viewportItem, QPointF(0, 0));
            itemRect.moveTopLeft(pos);
        }
    }

    const bool wasVisible = m_visibilityProperty.isEmpty()
            ? m_item->isVisible()
            : m_item->property(m_visibilityProperty).toBool();
    bool visible = true;

    switch (m_visibilityMode) {
    case IgnoreVisibility:
        visible = wasVisible;
        break;
    case AlwaysVisible:
        visible = true;
        break;
    case AlwaysInvisible:
        visible = false;
        break;
    case VisibleUponViewportIntersection:
        visible = m_viewportRect.isValid() && itemRect.isValid()
                ? m_viewportRect.intersects(itemRect)
                : true;
        break;
    case VisibleUponViewportContains:
        visible = m_viewportRect.isValid() && itemRect.isValid() ? m_viewportRect.contains(itemRect)
                                                                 : true;
        break;
    }

    if (wasVisible == visible)
        return;

    if (m_visibilityProperty.isEmpty())
        m_item->setVisible(visible);
    else
        m_item->setProperty(m_visibilityProperty, visible);

    emit itemVisibilityChanged();
}

void BoundingBoxItem::updateJson()
{
    m_json = this->toJson();
}

///////////////////////////////////////////////////////////////////////////////

BoundingBoxPreview::BoundingBoxPreview(QQuickItem *parent)
    : QQuickPaintedItem(parent), m_evaluator(this, "evaluator")
{
}

BoundingBoxPreview::~BoundingBoxPreview() { }

void BoundingBoxPreview::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();
}

void BoundingBoxPreview::setBackgroundOpacity(qreal val)
{
    if (qFuzzyCompare(m_backgroundOpacity, val))
        return;

    m_backgroundOpacity = val;
    emit backgroundOpacityChanged();
}

void BoundingBoxPreview::setEvaluator(BoundingBoxEvaluator *val)
{
    if (m_evaluator == val)
        return;

    if (m_evaluator != nullptr)
        disconnect(m_evaluator, &BoundingBoxEvaluator::previewUpdated, this,
                   &BoundingBoxPreview::updatePreviewImage);

    m_evaluator = val;

    if (m_evaluator != nullptr)
        connect(m_evaluator, &BoundingBoxEvaluator::previewUpdated, this,
                &BoundingBoxPreview::updatePreviewImage);

    emit evaluatorChanged();

    this->update();
}

void BoundingBoxPreview::paint(QPainter *painter)
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug("BoundingBoxPreview is painting");
#endif

    if (m_evaluator == nullptr || !this->isVisible() || qFuzzyIsNull(this->opacity()))
        return;

    painter->drawImage(0, 0, m_previewImage);
}

void BoundingBoxPreview::updatePreviewImage()
{
    if (m_evaluator == nullptr)
        return;

    auto capturePreviewAsPicture = [=]() -> QImage {
        const QRectF pictureRect(0, 0, this->width(), this->height());

        /**
         * Why capture the preview in a QImage now? Why not QPicture?
         * ----------------------------------------------------------
         *
         * Painting a QPicture on screen takes more time than painting a
         * QImage. When measured on a MacBook Pro, I noticed that painting
         * a QPicture takes 19ms per paint, whereas painting QImage
         * takes only 328us. So painting a QImage is ~60 times faster than
         * painting an image.
         *
         * If we take 19ms to paint a QPicture in BoundingBoxPreview::paint()
         * we are using that time along with drawing of other times on the
         * screen. Bad idea right?
         */

        QImage image((pictureRect.size() * 2.0).toSize(), QImage::Format_ARGB32);
        image.setDevicePixelRatio(2.0);
        image.fill(Qt::transparent);

        QPicture preview = m_evaluator->preview();
        if (preview.isNull())
            return image;

        QPainter painter(&image);
        painter.setOpacity(m_backgroundOpacity);
        painter.fillRect(pictureRect, m_backgroundColor);
        painter.setOpacity(1.0);

        QSizeF previewSize = preview.boundingRect().size();
        previewSize.scale(pictureRect.size(), Qt::KeepAspectRatio);

        QRectF previewRect(QPointF(0, 0), previewSize);
        // previewRect.moveCenter(itemRect.center());

        painter.translate(previewRect.topLeft());

        const qreal sx = previewSize.width() / preview.boundingRect().width();
        const qreal sy = previewSize.height() / preview.boundingRect().height();
        painter.scale(sx, sy);

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        preview.play(&painter);

        return image;
    };

    const QString futureWatcherName = QStringLiteral("RedrawFutureWatcher");
    if (this->findChild<QFutureWatcherBase *>(futureWatcherName) != nullptr)
        return;

    QFuture<QImage> future = QtConcurrent::run(&m_evaluator->m_threadPool, capturePreviewAsPicture);
    QFutureWatcher<QImage> *futureWatcher = new QFutureWatcher<QImage>(this);
    futureWatcher->setObjectName(futureWatcherName);
    connect(futureWatcher, &QFutureWatcher<QImage>::finished, this, [=]() {
        m_previewImage = future.result();
        futureWatcher->deleteLater();
        this->update();
    });
    futureWatcher->setFuture(future);
}

void BoundingBoxPreview::resetEvaluator()
{
    m_evaluator = nullptr;
    emit evaluatorChanged();
}
