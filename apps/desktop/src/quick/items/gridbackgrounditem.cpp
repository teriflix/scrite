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

#include "gridbackgrounditem.h"
#include "utils.h"

#include <QtQuick/QSGFlatColorMaterial>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QQuickWindow>

#include <QtMath>
#include <QVector>

GridBackgroundItemBorder::GridBackgroundItemBorder(QObject *parent) : QObject(parent) { }

GridBackgroundItemBorder::~GridBackgroundItemBorder() { }

void GridBackgroundItemBorder::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    m_color = val;
    emit colorChanged();
}

void GridBackgroundItemBorder::setWidth(qreal val)
{
    val = qBound(0.0, val, 10.0);
    if (qFuzzyCompare(m_width, val))
        return;

    m_width = val;
    emit widthChanged();
}

///////////////////////////////////////////////////////////////////////////////

GridBackgroundItem::GridBackgroundItem(QQuickItem *parent) : QQuickItem(parent)
{
    this->setFlag(ItemHasContents);

    connect(this, &GridBackgroundItem::opacityChanged, this, &GridBackgroundItem::update);
    connect(this, &GridBackgroundItem::tickColorOpacityChanged, this, &GridBackgroundItem::update);
    connect(m_border, &GridBackgroundItemBorder::colorChanged, this, &GridBackgroundItem::update);
    connect(m_border, &GridBackgroundItemBorder::widthChanged, this, &GridBackgroundItem::update);
}

GridBackgroundItem::~GridBackgroundItem() { }

void GridBackgroundItem::setTickDistance(qreal val)
{
    if (qFuzzyCompare(m_tickDistance, val))
        return;

    m_tickDistance = val;
    emit tickDistanceChanged();

    this->update();
}

void GridBackgroundItem::setMajorTickStride(int val)
{
    if (m_majorTickStride == val)
        return;

    m_majorTickStride = val;
    emit majorTickStrideChanged();

    this->update();
}

void GridBackgroundItem::setMinorTickLineWidth(qreal val)
{
    if (qFuzzyCompare(m_minorTickLineWidth, val))
        return;

    m_minorTickLineWidth = val;
    emit minorTickLineWidthChanged();

    this->update();
}

void GridBackgroundItem::setMajorTickLineWidth(qreal val)
{
    if (qFuzzyCompare(m_majorTickLineWidth, val))
        return;

    m_majorTickLineWidth = val;
    emit majorTickLineWidthChanged();

    this->update();
}

void GridBackgroundItem::setMinorTickColor(const QColor &val)
{
    if (m_minorTickColor == val)
        return;

    m_minorTickColor = val;
    emit minorTickColorChanged();

    this->update();
}

void GridBackgroundItem::setMajorTickColor(const QColor &val)
{
    if (m_majorTickColor == val)
        return;

    m_majorTickColor = val;
    emit majorTickColorChanged();

    this->update();
}

void GridBackgroundItem::setTickColorOpacity(qreal val)
{
    val = qBound(0.0, val, 1.0);
    if (qFuzzyCompare(m_tickColorOpacity, val))
        return;

    m_tickColorOpacity = val;
    emit tickColorOpacityChanged();

    this->update();
}

void GridBackgroundItem::setGridIsVisible(bool val)
{
    if (m_gridIsVisible == val)
        return;

    m_gridIsVisible = val;
    emit gridIsVisibleChanged();

    this->update();
}

void GridBackgroundItem::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();

    this->update();
}

QSGNode *GridBackgroundItem::updatePaintNode(QSGNode *oldNode,
                                             QQuickItem::UpdatePaintNodeData *nodeData)
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug("GridBackgroundItem is painting.");
#endif

    Q_UNUSED(nodeData)

    // Always recreate the scene graph to ensure proper rendering
    // when child items move or when properties change
    if (oldNode) {
        delete oldNode;
        oldNode = nullptr;
    }

    QSGNode *rootNode = new QSGNode;
    const qreal w = this->width();
    const qreal h = this->height();

    {
        if (!qFuzzyIsNull(m_backgroundColor.alphaF())) {
            QSGGeometryNode *geometryNode = new QSGGeometryNode;
            geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial
                                   | QSGNode::OwnedByParent);
            rootNode->appendChildNode(geometryNode);

            QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 6);
            geometry->setDrawingMode(QSGGeometry::DrawTriangles);
            geometryNode->setGeometry(geometry);

            QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

            points[0].x = 0.0f;
            points[0].y = 0.0f;

            points[1].x = float(w);
            points[1].y = 0.0f;

            points[2].x = 0.0f;
            points[2].y = float(h);

            points[3].x = 0.0f;
            points[3].y = float(h);

            points[4].x = float(w);
            points[4].y = 0.0f;

            points[5].x = float(w);
            points[5].y = float(h);

            QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
            geometryNode->setMaterial(material);

            QColor color = m_backgroundColor;
            color.setAlphaF(color.alphaF() * this->opacity());
            material->setFlag(QSGMaterial::Blending);
            material->setColor(color);
        }
    }

    if (!m_gridIsVisible || qFuzzyIsNull(m_tickColorOpacity))
        return rootNode;

    const int majorTickStride = qMax(1, m_majorTickStride);
    QVector<QSGGeometry::Point2D> minorTickVertices;
    QVector<QSGGeometry::Point2D> majorTickVertices;

    auto appendLine = [](QVector<QSGGeometry::Point2D> &vertices, qreal x1, qreal y1, qreal x2,
                         qreal y2) {
        QSGGeometry::Point2D p1 = { float(x1), float(y1) };
        QSGGeometry::Point2D p2 = { float(x2), float(y2) };
        vertices.append(p1);
        vertices.append(p2);
    };

    {
        qreal x = m_tickDistance;
        int xLineIndex = 1;
        while (x <= w) {
            if (xLineIndex % majorTickStride == 0)
                appendLine(majorTickVertices, x, 0.0, x, h);
            else
                appendLine(minorTickVertices, x, 0.0, x, h);

            x += m_tickDistance;
            ++xLineIndex;
        }
    }

    {
        qreal y = m_tickDistance;
        int yLineIndex = 1;
        while (y <= h) {
            if (yLineIndex % majorTickStride == 0)
                appendLine(majorTickVertices, 0.0, y, w, y);
            else
                appendLine(minorTickVertices, 0.0, y, w, y);

            y += m_tickDistance;
            ++yLineIndex;
        }
    }

    {
        QSGNode *minorTicksNode = new QSGOpacityNode;
        minorTicksNode->setFlags(QSGNode::OwnedByParent);
        rootNode->appendChildNode(minorTicksNode);

        QSGGeometryNode *geometryNode = new QSGGeometryNode;
        geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial
                               | QSGNode::OwnedByParent);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
                                                minorTickVertices.size());
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometry->setLineWidth(float(m_minorTickLineWidth));
        geometryNode->setGeometry(geometry);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
        for (int i = 0; i < minorTickVertices.size(); ++i)
            points[i] = minorTickVertices.at(i);

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
        geometryNode->setMaterial(material);

        QColor color = m_minorTickColor;
        color.setAlphaF(color.alphaF() * m_tickColorOpacity * this->opacity());
        material->setFlag(QSGMaterial::Blending);
        material->setColor(color);

        minorTicksNode->appendChildNode(geometryNode);
    }

    {
        QSGNode *majorTicksNode = new QSGOpacityNode;
        majorTicksNode->setFlags(QSGNode::OwnedByParent);
        rootNode->appendChildNode(majorTicksNode);

        QSGGeometryNode *geometryNode = new QSGGeometryNode;
        geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial
                               | QSGNode::OwnedByParent);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
                                                majorTickVertices.size());
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometry->setLineWidth(float(m_majorTickLineWidth));
        geometryNode->setGeometry(geometry);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
        for (int i = 0; i < majorTickVertices.size(); ++i)
            points[i] = majorTickVertices.at(i);

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
        geometryNode->setMaterial(material);

        QColor color = m_majorTickColor;
        color.setAlphaF(color.alphaF() * m_tickColorOpacity * this->opacity());
        material->setFlag(QSGMaterial::Blending);
        material->setColor(color);

        majorTicksNode->appendChildNode(geometryNode);
    }

    if (!qFuzzyIsNull(m_border->width())) {
        QSGNode *borderNode = new QSGOpacityNode;
        borderNode->setFlags(QSGNode::OwnedByParent);
        rootNode->appendChildNode(borderNode);

        QSGGeometryNode *geometryNode = new QSGGeometryNode;
        geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial
                               | QSGNode::OwnedByParent);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 8);
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometry->setLineWidth(float(m_border->width()));
        geometryNode->setGeometry(geometry);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

        points[0].x = 0.0f;
        points[0].y = 0.0f;
        points[1].x = float(w) - 1.0f;
        points[1].y = 0.0f;

        points[2].x = float(w) - 1.0f;
        points[2].y = 0.0f;
        points[3].x = float(w) - 1.0f;
        points[3].y = float(h) - 1.0f;

        points[4].x = float(w) - 1.0f;
        points[4].y = float(h) - 1.0f;
        points[5].x = 0.0f;
        points[5].y = float(h) - 1.0f;

        points[6].x = 0.0f;
        points[6].y = float(h) - 1.0f;
        points[7].x = 0.0f;
        points[7].y = 0.0f;

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
        geometryNode->setMaterial(material);

        QColor color = m_border->color();
        color.setAlphaF(color.alphaF() * this->opacity());
        material->setFlag(QSGMaterial::Blending);
        material->setColor(color);

        borderNode->appendChildNode(geometryNode);
    }

    return rootNode;
}
