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

#ifndef BOUNDINGBOXEVALUATOR_H
#define BOUNDINGBOXEVALUATOR_H

#include "execlatertimer.h"

#include <QRectF>
#include <QImage>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickPaintedItem>

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

public:
    BoundingBoxEvaluator(QObject *parent = nullptr);
    ~BoundingBoxEvaluator();

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

    Q_PROPERTY(QRectF initialRect READ initialRect WRITE setInitialRect NOTIFY initialRectChanged)
    void setInitialRect(const QRectF &val);
    QRectF initialRect() const { return m_initialRect; }
    Q_SIGNAL void initialRectChanged();

    Q_PROPERTY(qreal previewScale READ previewScale WRITE setPreviewScale NOTIFY previewScaleChanged)
    void setPreviewScale(qreal val);
    qreal previewScale() const { return m_previewScale; }
    Q_SIGNAL void previewScaleChanged();

    void updatePreview();
    QImage preview() const { return m_preview; }
    Q_INVOKABLE void markPreviewDirty();
    Q_SIGNAL void previewNeedsUpdate();

protected:
    void timerEvent(QTimerEvent *event);
    void setBoundingBox(const QRectF &val);
    void evaluateLater() { m_evaluationTimer.start(100, this); }
    void evaluateNow();

private:
    void addItem(BoundingBoxItem *item);
    void removeItem(BoundingBoxItem* item);
    void markDirty(BoundingBoxItem *) { this->evaluateLater(); }

private:
    friend class BoundingBoxItem;
    QImage m_preview;
    QRectF m_initialRect;
    QRectF m_boundingBox;
    qreal m_previewScale = 1.0;
    ExecLaterTimer m_evaluationTimer;
    QList<BoundingBoxItem*> m_items;
};

class BoundingBoxItem : public QObject
{
    Q_OBJECT

public:
    BoundingBoxItem(QObject *parent=nullptr);
    ~BoundingBoxItem();

    Q_SIGNAL void aboutToDestroy(BoundingBoxItem *ptr);

    static BoundingBoxItem *qmlAttachedProperties(QObject *object);

    QQuickItem *item() const { return m_item; }

    QRectF boundingRect() const;

    Q_PROPERTY(BoundingBoxEvaluator* evaluator READ evaluator WRITE setEvaluator NOTIFY evaluatorChanged RESET resetEvaluator)
    void setEvaluator(BoundingBoxEvaluator* val);
    BoundingBoxEvaluator* evaluator() const { return m_evaluator; }
    Q_SIGNAL void evaluatorChanged();

    Q_PROPERTY(qreal stackOrder READ stackOrder WRITE setStackOrder NOTIFY stackOrderChanged)
    void setStackOrder(qreal val);
    qreal stackOrder() const { return m_stackOrder; }
    Q_SIGNAL void stackOrderChanged();

    Q_PROPERTY(QColor previewFillColor READ previewFillColor WRITE setPreviewFillColor NOTIFY previewFillColorChanged)
    void setPreviewFillColor(const QColor &val);
    QColor previewFillColor() const { return m_previewFillColor; }
    Q_SIGNAL void previewFillColorChanged();

    Q_PROPERTY(QColor previewBorderColor READ previewBorderColor WRITE setPreviewBorderColor NOTIFY previewBorderColorChanged)
    void setPreviewBorderColor(const QColor &val);
    QColor previewBorderColor() const { return m_previewBorderColor; }
    Q_SIGNAL void previewBorderColorChanged();

    Q_PROPERTY(bool livePreview READ isLivePreview WRITE setLivePreview NOTIFY livePreviewChanged)
    void setLivePreview(bool val);
    bool isLivePreview() const { return m_livePreview; }
    Q_SIGNAL void livePreviewChanged();

    enum VisibilityMode
    {
        AlwaysVisible,
        VisibleUponViewportIntersection,
        VisibleUponViewportContains
    };
    Q_ENUMS(VisibilityMode)
    Q_PROPERTY(VisibilityMode visibilityMode READ visibilityMode WRITE setVisibilityMode NOTIFY visibilityModeChanged)
    void setVisibilityMode(VisibilityMode val);
    VisibilityMode visibilityMode() const { return m_visibilityMode; }
    Q_SIGNAL void visibilityModeChanged();

    Q_PROPERTY(QQuickItem* viewportItem READ viewportItem WRITE setViewportItem NOTIFY viewportItemChanged RESET resetViewportItem)
    void setViewportItem(QQuickItem* val);
    QQuickItem* viewportItem() const { return m_viewportItem; }
    Q_SIGNAL void viewportItemChanged();

    Q_PROPERTY(QRectF viewportRect READ viewportRect WRITE setViewportRect NOTIFY viewportRectChanged)
    void setViewportRect(const QRectF &val);
    QRectF viewportRect() const { return m_viewportRect; }
    Q_SIGNAL void viewportRectChanged();

    Q_INVOKABLE void markPreviewDirty();

    QImage preview() const { return m_preview; }
    Q_SIGNAL void previewUpdated();

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

private:
    QImage m_preview;
    qreal m_stackOrder = 0;
    bool m_livePreview = true;
    QRectF m_viewportRect;
    QPointer<QQuickItem> m_item;
    QColor m_previewFillColor = Qt::white;
    QColor m_previewBorderColor = Qt::black;
    VisibilityMode m_visibilityMode = AlwaysVisible;
    ExecLaterTimer m_updatePreviewTimer;
    QObjectProperty<QQuickItem> m_viewportItem;
    QSharedPointer<QQuickItemGrabResult> m_itemGrabResult;
    QObjectProperty<BoundingBoxEvaluator> m_evaluator;
};

class BoundingBoxPreview : public QQuickPaintedItem
{
    Q_OBJECT

public:
    BoundingBoxPreview(QQuickItem *parent=nullptr);
    ~BoundingBoxPreview();

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(qreal backgroundOpacity READ backgroundOpacity WRITE setBackgroundOpacity NOTIFY backgroundOpacityChanged)
    void setBackgroundOpacity(qreal val);
    qreal backgroundOpacity() const { return m_backgroundOpacity; }
    Q_SIGNAL void backgroundOpacityChanged();

    Q_PROPERTY(BoundingBoxEvaluator* evaluator READ evaluator WRITE setEvaluator NOTIFY evaluatorChanged RESET resetEvaluator)
    void setEvaluator(BoundingBoxEvaluator* val);
    BoundingBoxEvaluator* evaluator() const { return m_evaluator; }
    Q_SIGNAL void evaluatorChanged();

    // QQuickPaintedItem interface
    void paint(QPainter *painter);

private:
    void redraw() { this->update(); }
    void resetEvaluator();

private:
    QColor m_backgroundColor = Qt::white;
    qreal m_backgroundOpacity = 1.0;
    QObjectProperty<BoundingBoxEvaluator> m_evaluator;
};

Q_DECLARE_METATYPE(BoundingBoxItem*)
QML_DECLARE_TYPEINFO(BoundingBoxItem, QML_HAS_ATTACHED_PROPERTIES)

#endif // ITEMSBOUNDINGBOX_H
