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

#ifndef ABSTRACTSHAPEITEM_H
#define ABSTRACTSHAPEITEM_H

#include <QPainterPath>
#include <QQuickPaintedItem>
#include <QMetaObject>
#include <QBasicTimer>

class AbstractShapeItem : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit AbstractShapeItem(QQuickItem *parent = nullptr);
    ~AbstractShapeItem();

    enum RenderType {
        OutlineOnly = 1,
        OutlineAlso = OutlineOnly,
        FillOnly = 2,
        FillAlso = FillOnly,
        OutlineAndFill = 3
    };
    Q_ENUM(RenderType)
    Q_PROPERTY(RenderType renderType READ renderType WRITE setRenderType NOTIFY renderTypeChanged)
    void setRenderType(RenderType val);
    RenderType renderType() const { return m_renderType; }
    Q_SIGNAL void renderTypeChanged();

    enum RenderingMechanism { UseOpenGL, UseQPainter, UseAntialiasedQPainter };
    Q_ENUM(RenderingMechanism)
    Q_PROPERTY(RenderingMechanism renderingMechanism READ renderingMechanism WRITE setRenderingMechanism NOTIFY renderingMechanismChanged)
    void setRenderingMechanism(RenderingMechanism val);
    RenderingMechanism renderingMechanism() const { return m_renderingMechanism; }
    Q_SIGNAL void renderingMechanismChanged();

    Q_PROPERTY(QColor outlineColor READ outlineColor WRITE setOutlineColor NOTIFY outlineColorChanged)
    void setOutlineColor(const QColor &val);
    QColor outlineColor() const { return m_outlineColor; }
    Q_SIGNAL void outlineColorChanged();

    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    void setFillColor(const QColor &val);
    QColor fillColor() const { return m_fillColor; }
    Q_SIGNAL void fillColorChanged();

    Q_PROPERTY(qreal outlineWidth READ outlineWidth WRITE setOutlineWidth NOTIFY outlineWidthChanged)
    void setOutlineWidth(const qreal &val);
    qreal outlineWidth() const { return m_outlineWidth; }
    Q_SIGNAL void outlineWidthChanged();

    enum OutlineStyle {
        SolidLine = Qt::SolidLine,
        DashLine,
        DotLine,
        DashDotLine,
        DashDotDotLine,
    };
    Q_ENUM(OutlineStyle)
    Q_PROPERTY(OutlineStyle outlineStyle READ outlineStyle WRITE setOutlineStyle NOTIFY outlineStyleChanged)
    void setOutlineStyle(OutlineStyle val);
    OutlineStyle outlineStyle() const { return m_outlineStyle; }
    Q_SIGNAL void outlineStyleChanged();

    Q_PROPERTY(QRectF contentRect READ contentRect NOTIFY contentRectChanged)
    QRectF contentRect() const;
    Q_SIGNAL void contentRectChanged();

    QPainterPath currentShape() const { return m_path; }

    virtual QPainterPath shape() const = 0;

protected:
    bool updateShape();

    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    QSGNode *constructSceneGraph() const;
    QSGNode *polishSceneGraph(QSGNode *rootNode) const;

    void paint(QPainter *paint);

private:
    QPainterPath m_path;
    QColor m_fillColor = QColor(Qt::white);
    qreal m_outlineWidth = 1.0;
    QColor m_outlineColor = QColor(Qt::black);
    RenderType m_renderType = OutlineAndFill;
    OutlineStyle m_outlineStyle = SolidLine;
    RenderingMechanism m_renderingMechanism = UseOpenGL;
};

#endif // ABSTRACTSHAPEITEM_H
