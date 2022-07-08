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

#include "abstractshapeitem.h"
#include "polygontesselator.h"
#include "application.h"

#include <QtQuick/QSGFlatColorMaterial>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QQuickWindow>

#include <QPainter>

AbstractShapeItem::AbstractShapeItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    this->setFlag(ItemHasContents);

    connect(this, SIGNAL(opacityChanged()), this, SLOT(update()));
}

AbstractShapeItem::~AbstractShapeItem() { }

void AbstractShapeItem::setRenderType(AbstractShapeItem::RenderType val)
{
    if (m_renderType == val)
        return;

    m_renderType = val;
    emit renderTypeChanged();

    this->update();
}

void AbstractShapeItem::setRenderingMechanism(AbstractShapeItem::RenderingMechanism val)
{
    if (m_renderingMechanism == val)
        return;

    m_renderingMechanism = val;
    emit renderingMechanismChanged();

    this->update();
}

void AbstractShapeItem::setOutlineColor(const QColor &val)
{
    if (m_outlineColor == val)
        return;

    m_outlineColor = val;
    emit outlineColorChanged();

    this->update();
}

void AbstractShapeItem::setFillColor(const QColor &val)
{
    if (m_fillColor == val)
        return;

    m_fillColor = val;
    emit fillColorChanged();

    this->update();
}

void AbstractShapeItem::setOutlineWidth(const qreal &val)
{
    if (qFuzzyCompare(m_outlineWidth, val))
        return;

    if (qIsNaN(val)) {
        qDebug("%s was given NaN as parameter", Q_FUNC_INFO);
        return;
    }

    m_outlineWidth = val;
    emit outlineWidthChanged();

    this->update();
}

void AbstractShapeItem::setOutlineStyle(AbstractShapeItem::OutlineStyle val)
{
    if (m_outlineStyle == val)
        return;

    m_outlineStyle = val;
    emit outlineStyleChanged();

    this->update();
}

QRectF AbstractShapeItem::contentRect() const
{
    return m_path.boundingRect();
}

bool AbstractShapeItem::updateShape()
{
    QPainterPath path = this->shape();

#if 0
    QRectF pathRect = path.boundingRect();

    if( pathRect.topLeft() != QPointF(0,0) )
    {
        QTransform tx;
        tx.translate( -pathRect.left(), -pathRect.top() );
        path = tx.map(path).simplified();
    }
    else
        path = path.simplified();
#endif

    if (path == m_path)
        return false;

    m_path = path;
    emit contentRectChanged();
    return true;
}

QSGNode *AbstractShapeItem::updatePaintNode(QSGNode *oldNode,
                                            QQuickItem::UpdatePaintNodeData *nodeData)
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug("AbstractShapeItem is painting.");
#endif

    const bool pathUpdated = this->updateShape();
    static const bool isSoftwareContext =
            qgetenv("QMLSCENE_DEVICE") == QByteArray("softwarecontext");
    if (isSoftwareContext || m_renderingMechanism == UseQPainter
        || m_renderingMechanism == UseAntialiasedQPainter)
        return QQuickPaintedItem::updatePaintNode(oldNode, nodeData);

    QQuickWindow *qmlWindow = this->window();
    if (qmlWindow
        && qmlWindow->rendererInterface()->graphicsApi() == QSGRendererInterface::Software)
        return QQuickPaintedItem::updatePaintNode(oldNode, nodeData);

    if (pathUpdated) {
        if (oldNode)
            delete oldNode;

        oldNode = nullptr;
    }

    QSGNode *node = pathUpdated ? constructSceneGraph() : oldNode;
    return this->polishSceneGraph(node);
}

QSGNode *AbstractShapeItem::constructSceneGraph() const
{
    if (m_path.isEmpty())
        return nullptr;

    const QList<QPolygonF> subpaths =
            m_renderType & OutlineAlso ? m_path.toSubpathPolygons() : QList<QPolygonF>();

    // Triangulate all fillable polygons in the path
    const QVector<QPointF> triangles =
            m_renderType & FillAlso ? PolygonTessellator::tessellate(subpaths) : QVector<QPointF>();
    // I am not using QPolygonF here on purpose
    // even though QPolygonF is a QVector<QPointF>.
    // This is because, QPolygonF implies that all
    // points in it make a single polygon. Here
    // we want for the variable to imply a vector
    // of points such that each set of 3 points make
    // makes one triangle.

    // Extract all outline polygons
    const QList<QPolygonF> &outlines = subpaths;

    // Construct the scene graph branch for this node.
    QSGNode *rootNode = new QSGNode;

    // Construct one opacity node for outline, one more for filled.
    QSGNode *trianglesNode = new QSGOpacityNode;
    trianglesNode->setFlags(QSGNode::OwnedByParent);
    rootNode->appendChildNode(trianglesNode);

    QSGNode *outlinesNode = new QSGOpacityNode;
    outlinesNode->setFlags(QSGNode::OwnedByParent);
    rootNode->appendChildNode(outlinesNode);

    // Construct one geometry node for each fill-polygon with fill color.
    if (m_renderType & FillAlso) {
        QSGGeometryNode *fillNode = new QSGGeometryNode;
        fillNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial | QSGNode::OwnedByParent);

        QSGGeometry *fillGeometry =
                new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), triangles.size());
        fillGeometry->setDrawingMode(QSGGeometry::DrawTriangles);
        fillNode->setGeometry(fillGeometry);

        QSGGeometry::Point2D *fillPoints = fillGeometry->vertexDataAsPoint2D();
        for (int i = 0; i < triangles.size(); i++) {
            fillPoints[i].x = float(triangles.at(i).x());
            fillPoints[i].y = float(triangles.at(i).y());
        }

        QSGFlatColorMaterial *fillMaterial = new QSGFlatColorMaterial();
        fillNode->setMaterial(fillMaterial);

        QColor fillColor = m_fillColor;
        fillColor.setAlphaF(fillColor.alphaF() * this->opacity());
        fillMaterial->setFlag(QSGMaterial::Blending);
        fillMaterial->setColor(fillColor);

        trianglesNode->appendChildNode(fillNode);
    }

    if (!(m_renderType & OutlineAlso) || qFuzzyIsNull(m_outlineWidth))
        return rootNode;

    auto createLineGeometry = [=](const QPolygonF &polygon) {
        QSGGeometry *outlineGeometry =
                new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), polygon.size());
        outlineGeometry->setDrawingMode(m_renderType == OutlineOnly ? QSGGeometry::DrawLineStrip
                                                                    : QSGGeometry::DrawLineLoop);
        QSGGeometry::Point2D *outlinePoints = outlineGeometry->vertexDataAsPoint2D();
        for (int i = 0; i < polygon.size(); i++) {
            outlinePoints[i].x = float(polygon.at(i).x());
            outlinePoints[i].y = float(polygon.at(i).y());
        }

        return outlineGeometry;
    };

    auto createStrokeGeometry = [=](const QPolygonF &polygon) {
        QPainterPath path;
        for (int i = 0; i < polygon.size(); i++) {
            if (i)
                path.lineTo(polygon.at(i));
            else
                path.moveTo(polygon.at(i));
        }
        if (m_renderType & FillAlso)
            path.lineTo(polygon.at(0));

        QPainterPathStroker stroker;
        stroker.setWidth(m_outlineWidth);
        stroker.setJoinStyle(Qt::MiterJoin);
        stroker.setCapStyle(Qt::SquareCap);

        path = stroker.createStroke(path).simplified();
        path.setFillRule(Qt::WindingFill);

        const QVector<QPointF> triangles = PolygonTessellator::tessellate(path.toSubpathPolygons());

        QSGGeometry *fillGeometry =
                new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), triangles.size());
        fillGeometry->setDrawingMode(QSGGeometry::DrawTriangles);

        QSGGeometry::Point2D *fillPoints = fillGeometry->vertexDataAsPoint2D();
        for (int i = 0; i < triangles.size(); i++) {
            fillPoints[i].x = float(triangles.at(i).x());
            fillPoints[i].y = float(triangles.at(i).y());
        }

        return fillGeometry;
    };

    // Construct one geometry node for each outline will outline color
    for (const QPolygonF &polygon : outlines) {
        if (polygon.isEmpty())
            continue;

        QSGGeometryNode *outlineNode = new QSGGeometryNode;
        outlineNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial
                              | QSGNode::OwnedByParent);

        QSGGeometry *outlineGeometry = qFuzzyCompare(m_outlineWidth, 1)
                ? createLineGeometry(polygon)
                : createStrokeGeometry(polygon);
        //        QSGGeometry *outlineGeometry = createLineGeometry(polygon);
        outlineNode->setGeometry(outlineGeometry);

        QSGFlatColorMaterial *outlineMaterial = new QSGFlatColorMaterial();
        outlineNode->setMaterial(outlineMaterial);

        QColor outlineColor = m_outlineColor;
        outlineColor.setAlphaF(outlineColor.alphaF() * this->opacity());
        outlineMaterial->setFlag(QSGMaterial::Blending);
        outlineMaterial->setFlag(QSGMaterial::RequiresFullMatrixExceptTranslate);
        outlineMaterial->setColor(outlineColor);

        outlinesNode->appendChildNode(outlineNode);
    }

    return rootNode;
}

QSGNode *AbstractShapeItem::polishSceneGraph(QSGNode *rootNode) const
{
    if (rootNode == nullptr)
        return nullptr;

    QSGOpacityNode *trianglesNode = static_cast<QSGOpacityNode *>(rootNode->childAtIndex(0));
    trianglesNode->setOpacity(m_renderType & FillAlso ? 1 : 0);

    QSGGeometryNode *fillNode = static_cast<QSGGeometryNode *>(trianglesNode->firstChild());
    if (fillNode != nullptr) {
        QSGFlatColorMaterial *fillMaterial =
                static_cast<QSGFlatColorMaterial *>(fillNode->material());

        if (fillMaterial != nullptr) {
            QColor fillColor = m_fillColor;
            fillColor.setAlphaF(fillColor.alphaF() * this->opacity());
            fillMaterial->setColor(fillColor);
        }
    }

    QSGOpacityNode *outlinesNode = static_cast<QSGOpacityNode *>(rootNode->childAtIndex(1));
    outlinesNode->setOpacity(m_renderType & OutlineAlso ? 1 : 0);

    for (int i = 0; i < outlinesNode->childCount(); i++) {
        QSGGeometryNode *outlineNode =
                static_cast<QSGGeometryNode *>(outlinesNode->childAtIndex(i));
        if (outlineNode != nullptr) {
            QSGGeometry *outlineGeometry = outlineNode->geometry();
            if (outlineGeometry != nullptr)
                outlineGeometry->setLineWidth(float(m_outlineWidth));

            QSGFlatColorMaterial *outlineMaterial =
                    static_cast<QSGFlatColorMaterial *>(outlineNode->material());
            if (outlineMaterial != nullptr) {
                QColor outlineColor = m_outlineColor;
                outlineColor.setAlphaF(outlineColor.alphaF() * this->opacity());
                outlineMaterial->setColor(outlineColor);
            }
        }
    }

    return rootNode;
}

void AbstractShapeItem::paint(QPainter *paint)
{
    if (m_renderType & FillAlso)
        paint->setBrush(m_fillColor);
    else
        paint->setBrush(Qt::NoBrush);

    if (m_renderType & OutlineAlso) {
        QPen pen(m_outlineColor, m_outlineWidth);
        pen.setStyle(Qt::PenStyle(int(m_outlineStyle)));
        paint->setPen(pen);
    } else
        paint->setPen(Qt::NoPen);

    paint->setRenderHint(QPainter::Antialiasing, m_renderingMechanism == UseAntialiasedQPainter);
    paint->drawPath(m_path);
}
