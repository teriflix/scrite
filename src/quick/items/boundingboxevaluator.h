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

#ifndef BOUNDINGBOXEVALUATOR_H
#define BOUNDINGBOXEVALUATOR_H

#include "execlatertimer.h"

#include <QRectF>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QPicture>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QThreadPool>
#include <QJsonObject>
#include <QQuickPaintedItem>
#include <QFutureWatcherBase>

#include "qobjectproperty.h"

/**
 * QQuickItem::childrenRect() doesn't ever shrink, even though items have moved
 * inside the previously know childrenRect(). It only always expands. We need
 * a bounding box that can shrink for use with StructureView -> canvas. That's
 * why this class.
 */
class BoundingBoxItem;
class BoundingBoxEvaluator : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit BoundingBoxEvaluator(QObject *parent = nullptr);
    ~BoundingBoxEvaluator();

    // clang-format off
    Q_PROPERTY(qreal margin
               READ margin
               WRITE setMargin
               NOTIFY marginChanged)
    // clang-format on
    void setMargin(qreal val);
    qreal margin() const { return m_margin; }
    Q_SIGNAL void marginChanged();

    // clang-format off
    Q_PROPERTY(QRectF boundingBox
               READ boundingBox
               NOTIFY boundingBoxChanged)
    // clang-format on
    QRectF boundingBox() const { return m_boundingBox; }
    Q_SIGNAL void boundingBoxChanged();

    // Bounding box without initialRect consideration
    // clang-format off
    Q_PROPERTY(QRectF tightBoundingBox
               READ tightBoundingBox
               NOTIFY tightBoundingBoxChanged)
    // clang-format on
    QRectF tightBoundingBox() const { return m_tightBoundingBox; }
    Q_SIGNAL void tightBoundingBoxChanged();

    // clang-format off
    Q_PROPERTY(qreal x
               READ x
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal x() const { return m_boundingBox.x(); }

    // clang-format off
    Q_PROPERTY(qreal y
               READ y
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal y() const { return m_boundingBox.y(); }

    // clang-format off
    Q_PROPERTY(qreal width
               READ width
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal width() const { return m_boundingBox.width(); }

    // clang-format off
    Q_PROPERTY(qreal height
               READ height
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal height() const { return m_boundingBox.height(); }

    // clang-format off
    Q_PROPERTY(qreal left
               READ left
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal left() const { return m_boundingBox.left(); }

    // clang-format off
    Q_PROPERTY(qreal top
               READ top
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal top() const { return m_boundingBox.top(); }

    // clang-format off
    Q_PROPERTY(qreal right
               READ right
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal right() const { return m_boundingBox.right(); }

    // clang-format off
    Q_PROPERTY(qreal bottom
               READ bottom
               NOTIFY boundingBoxChanged)
    // clang-format on
    qreal bottom() const { return m_boundingBox.bottom(); }

    // clang-format off
    Q_PROPERTY(QPointF center
               READ center
               NOTIFY boundingBoxChanged)
    // clang-format on
    QPointF center() const { return m_boundingBox.center(); }

    // clang-format off
    Q_PROPERTY(QRectF initialRect
               READ initialRect
               WRITE setInitialRect
               NOTIFY initialRectChanged)
    // clang-format on
    void setInitialRect(const QRectF &val);
    QRectF initialRect() const { return m_initialRect; }
    Q_SIGNAL void initialRectChanged();

    // clang-format off
    Q_PROPERTY(qreal previewScale
               READ previewScale
               WRITE setPreviewScale
               NOTIFY previewScaleChanged)
    // clang-format on
    void setPreviewScale(qreal val);
    qreal previewScale() const { return m_previewScale; }
    Q_SIGNAL void previewScaleChanged();

    // clang-format off
    Q_PROPERTY(int itemCount
               READ itemCount
               NOTIFY itemCountChanged)
    // clang-format on
    int itemCount() const { return m_items.size(); }
    Q_SIGNAL void itemCountChanged();

    QPicture preview() const;
    Q_INVOKABLE void markPreviewDirty();
    Q_SIGNAL void previewUpdated();

    Q_INVOKABLE void recomputeBoundingBox() { this->evaluateNow(); }

protected:
    void timerEvent(QTimerEvent *event);
    void setBoundingBox(const QRectF &val);
    void setTightBoundingBox(const QRectF &val);
    void evaluateLater() { m_evaluationTimer.start(100, this); }
    void evaluateNow();

    void updatePreview();
    static QPicture createPreviewPicture(const QList<QJsonObject> &items, const QRectF &bbox,
                                         const qreal scale);

private:
    void addItem(BoundingBoxItem *item);
    void removeItem(BoundingBoxItem *item);
    void markDirty(BoundingBoxItem *) { this->evaluateLater(); }

private:
    friend class BoundingBoxItem;
    friend class BoundingBoxPreview;

    qreal m_margin = 0;
    QPicture m_preview;
    qreal m_previewScale = 1.0;
    QRectF m_initialRect;
    QRectF m_boundingBox;
    QRectF m_tightBoundingBox;
    QThreadPool m_threadPool;
    mutable QMutex m_previewLock;
    ExecLaterTimer m_evaluationTimer;
    ExecLaterTimer m_updatePreviewTimer;
    QList<BoundingBoxItem *> m_items;
};

class BoundingBoxItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")
    QML_ATTACHED(BoundingBoxItem)

public:
    explicit BoundingBoxItem(QObject *parent = nullptr);
    ~BoundingBoxItem();

    Q_SIGNAL void aboutToDestroy(BoundingBoxItem *ptr);

    static BoundingBoxItem *qmlAttachedProperties(QObject *object);

    QQuickItem *item() const { return m_item; }

    QRectF boundingRect() const;

    // clang-format off
    Q_PROPERTY(BoundingBoxEvaluator *evaluator
               READ evaluator
               WRITE setEvaluator
               NOTIFY evaluatorChanged
               RESET resetEvaluator
               STORED false)
    // clang-format on
    void setEvaluator(BoundingBoxEvaluator *val);
    BoundingBoxEvaluator *evaluator() const { return m_evaluator; }
    Q_SIGNAL void evaluatorChanged();

    // clang-format off
    Q_PROPERTY(QVariant itemRect
               READ itemRect
               WRITE setItemRect
               NOTIFY itemRectChanged)
    // clang-format on
    void setItemRect(const QVariant &val);
    QVariant itemRect() const { return m_itemRect; }
    Q_SIGNAL void itemRectChanged();

    // clang-format off
    Q_PROPERTY(qreal stackOrder
               READ stackOrder
               WRITE setStackOrder
               NOTIFY stackOrderChanged)
    // clang-format on
    void setStackOrder(qreal val);
    qreal stackOrder() const { return m_stackOrder; }
    Q_SIGNAL void stackOrderChanged();

    // clang-format off
    Q_PROPERTY(bool previewEnabled
               READ isPreviewEnabled
               WRITE setPreviewEnabled
               NOTIFY previewEnabledChanged)
    // clang-format on
    void setPreviewEnabled(bool val);
    bool isPreviewEnabled() const { return m_previewEnabled; }
    Q_SIGNAL void previewEnabledChanged();

    // clang-format off
    Q_PROPERTY(QColor previewFillColor
               READ previewFillColor
               WRITE setPreviewFillColor
               NOTIFY previewFillColorChanged)
    // clang-format on
    void setPreviewFillColor(const QColor &val);
    QColor previewFillColor() const { return m_previewFillColor; }
    Q_SIGNAL void previewFillColorChanged();

    // clang-format off
    Q_PROPERTY(QString previewImageSource
               READ previewImageSource
               WRITE setPreviewImageSource
               NOTIFY previewImageSourceChanged)
    // clang-format on
    void setPreviewImageSource(const QString &val);
    QString previewImageSource() const { return m_previewImageSource; }
    Q_SIGNAL void previewImageSourceChanged();

    // clang-format off
    Q_PROPERTY(QColor previewBorderColor
               READ previewBorderColor
               WRITE setPreviewBorderColor
               NOTIFY previewBorderColorChanged)
    // clang-format on
    void setPreviewBorderColor(const QColor &val);
    QColor previewBorderColor() const { return m_previewBorderColor; }
    Q_SIGNAL void previewBorderColorChanged();

    // clang-format off
    Q_PROPERTY(qreal previewBorderWidth
               READ previewBorderWidth
               WRITE setPreviewBorderWidth
               NOTIFY previewBorderWidthChanged)
    // clang-format on
    void setPreviewBorderWidth(qreal val);
    qreal previewBorderWidth() const { return m_previewBorderWidth; }
    Q_SIGNAL void previewBorderWidthChanged();

    // clang-format off
    Q_PROPERTY(bool livePreview
               READ isLivePreview
               WRITE setLivePreview
               NOTIFY livePreviewChanged)
    // clang-format on
    void setLivePreview(bool val);
    bool isLivePreview() const { return m_livePreview; }
    Q_SIGNAL void livePreviewChanged();

    enum VisibilityMode {
        IgnoreVisibility,
        AlwaysVisible,
        AlwaysInvisible,
        VisibleUponViewportIntersection,
        VisibleUponViewportContains
    };
    Q_ENUM(VisibilityMode)
    // clang-format off
    Q_PROPERTY(VisibilityMode visibilityMode
               READ visibilityMode
               WRITE setVisibilityMode
               NOTIFY visibilityModeChanged)
    // clang-format on
    void setVisibilityMode(VisibilityMode val);
    VisibilityMode visibilityMode() const { return m_visibilityMode; }
    Q_SIGNAL void visibilityModeChanged();

    // clang-format off
    Q_PROPERTY(QQuickItem *viewportItem
               READ viewportItem
               WRITE setViewportItem
               NOTIFY viewportItemChanged
               RESET resetViewportItem
               STORED false)
    // clang-format on
    void setViewportItem(QQuickItem *val);
    QQuickItem *viewportItem() const { return m_viewportItem; }
    Q_SIGNAL void viewportItemChanged();

    // clang-format off
    Q_PROPERTY(QRectF viewportRect
               READ viewportRect
               WRITE setViewportRect
               NOTIFY viewportRectChanged)
    // clang-format on
    void setViewportRect(const QRectF &val);
    QRectF viewportRect() const { return m_viewportRect; }
    Q_SIGNAL void viewportRectChanged();

    // clang-format off
    Q_PROPERTY(QByteArray visibilityProperty
               READ visibilityProperty
               WRITE setVisibilityProperty
               NOTIFY visibilityPropertyChanged)
    // clang-format on
    void setVisibilityProperty(const QByteArray &val);
    QByteArray visibilityProperty() const { return m_visibilityProperty; }
    Q_SIGNAL void visibilityPropertyChanged();

    Q_INVOKABLE void markPreviewDirty();

    QImage preview() const { return m_preview; }
    Q_SIGNAL void previewUpdated();

    Q_SIGNAL void itemVisibilityChanged();

    QJsonObject asJson() const { return m_json; }

protected:
    void timerEvent(QTimerEvent *event);

private:
    void requestReevaluation();
    void resetEvaluator();
    void resetViewportItem();
    void updatePreview();
    void updatePreviewLater();
    void setPreview(const QImage &image);
    void determineVisibility();
    void updateJson();
    QJsonObject toJson() const;

private:
    QImage m_preview;
    QImage m_staticPreview; // incase previewImageSource is set.
    qreal m_stackOrder = 0;
    QVariant m_itemRect;
    bool m_livePreview = true;
    bool m_previewEnabled = true;
    QRectF m_viewportRect;
    QJsonObject m_json;
    QTimer m_jsonUpdateTimer;
    QPointer<QQuickItem> m_item;
    QString m_previewImageSource;
    qreal m_previewBorderWidth = 1;
    QColor m_previewFillColor = Qt::white;
    QColor m_previewBorderColor = Qt::black;
    VisibilityMode m_visibilityMode = AlwaysVisible;
    QByteArray m_visibilityProperty;
    ExecLaterTimer m_updatePreviewTimer;
    QObjectProperty<QQuickItem> m_viewportItem;
    QSharedPointer<QQuickItemGrabResult> m_itemGrabResult;
    QObjectProperty<BoundingBoxEvaluator> m_evaluator;
};

class BoundingBoxPreview : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit BoundingBoxPreview(QQuickItem *parent = nullptr);
    ~BoundingBoxPreview();

    // clang-format off
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    // clang-format off
    Q_PROPERTY(qreal backgroundOpacity
               READ backgroundOpacity
               WRITE setBackgroundOpacity
               NOTIFY backgroundOpacityChanged)
    // clang-format on
    void setBackgroundOpacity(qreal val);
    qreal backgroundOpacity() const { return m_backgroundOpacity; }
    Q_SIGNAL void backgroundOpacityChanged();

    // clang-format off
    Q_PROPERTY(BoundingBoxEvaluator *evaluator
               READ evaluator
               WRITE setEvaluator
               NOTIFY evaluatorChanged
               RESET resetEvaluator)
    // clang-format on
    void setEvaluator(BoundingBoxEvaluator *val);
    BoundingBoxEvaluator *evaluator() const { return m_evaluator; }
    Q_SIGNAL void evaluatorChanged();

    // clang-format off
    Q_PROPERTY(bool isUpdatingPreview
               READ isUpdatingPreview
               NOTIFY isUpdatingPreviewChanged)
    // clang-format on
    bool isUpdatingPreview() const { return !m_updatePreviewFutureWatcher.isNull(); }
    Q_SIGNAL void isUpdatingPreviewChanged();

    // QQuickPaintedItem interface
    void paint(QPainter *painter);

private:
    void updatePreviewImage();
    void resetEvaluator();

private:
    QImage m_previewImage;
    QColor m_backgroundColor = Qt::white;
    qreal m_backgroundOpacity = 1.0;
    QPointer<QFutureWatcherBase> m_updatePreviewFutureWatcher;
    QObjectProperty<BoundingBoxEvaluator> m_evaluator;
};

#endif // ITEMSBOUNDINGBOX_H
