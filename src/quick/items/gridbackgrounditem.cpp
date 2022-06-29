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

#include "gridbackgrounditem.h"

#include <QtQuick/QSGFlatColorMaterial>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QQuickWindow>

#include <QtMath>

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

    if (oldNode)
        delete oldNode;
    Q_UNUSED(nodeData)

    QSGNode *rootNode = new QSGNode;
    const qreal w = this->width();
    const qreal h = this->height();

    {
        if (!qFuzzyIsNull(m_backgroundColor.alphaF())) {
            QSGOpacityNode *backgroundNode = new QSGOpacityNode;
            backgroundNode->setFlag(QSGGeometryNode::OwnedByParent);
            backgroundNode->setOpacity(this->opacity());
            rootNode->appendChildNode(backgroundNode);

            QSGGeometryNode *geometryNode = new QSGGeometryNode;
            geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial
                                   | QSGNode::OwnedByParent);

            QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 6);
            geometry->setDrawingMode(QSGGeometry::DrawTriangles);
            geometryNode->setGeometry(geometry);

            QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

            points[0].x = 0.0f;
            points[0].y = 0.0f;

            points[1].x = float(w) - 1.0f;
            points[1].y = 0.0f;

            points[2].x = float(w) - 1.0f;
            points[2].y = float(h) - 1.0f;

            points[3].x = 0.0f;
            points[3].y = 0.0f;

            points[4].x = float(w) - 1.0f;
            points[4].y = float(h) - 1.0f;

            points[5].x = 0.0f;
            points[5].y = float(h) - 1.0f;

            QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
            geometryNode->setMaterial(material);
            material->setFlag(QSGMaterial::Blending);
            material->setColor(m_backgroundColor);

            backgroundNode->appendChildNode(geometryNode);
        }
    }

    if (!m_gridIsVisible || qFuzzyIsNull(m_tickColorOpacity))
        return rootNode;

    const int nrXTicks = int(qCeil(w / m_tickDistance)) - 1;
    const int nrYTicks = int(qCeil(h / m_tickDistance)) - 1;
    const int nrXMajorTicks = int(qCeil(w / (m_tickDistance * m_majorTickStride))) - 1;
    const int nrYMajorTicks = int(qCeil(h / (m_tickDistance * m_majorTickStride))) - 1;
    const int nrXMinorTicks = nrXTicks - nrXMajorTicks;
    const int nrYMinorTicks = nrYTicks - nrYMajorTicks;
    const int nrMinorTicks = nrXMinorTicks + nrYMinorTicks;
    const int nrMajorTicks = nrXMajorTicks + nrYMajorTicks;

    {
        QSGNode *minorTicksNode = new QSGOpacityNode;
        minorTicksNode->setFlags(QSGNode::OwnedByParent);
        rootNode->appendChildNode(minorTicksNode);

        QSGGeometryNode *geometryNode = new QSGGeometryNode;
        geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial
                               | QSGNode::OwnedByParent);

        QSGGeometry *geometry =
                new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), nrMinorTicks * 2);
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometry->setLineWidth(float(m_minorTickLineWidth));
        geometryNode->setGeometry(geometry);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

        qreal x = m_tickDistance, y = m_tickDistance;
        int pointIndex = 0;
        int xLineIndex = 1, yLineIndex = 1;

        while (x <= w) {
            points[pointIndex].x = float(x);
            points[pointIndex].y = 0.f;
            ++pointIndex;

            points[pointIndex].x = float(x);
            points[pointIndex].y = float(h);
            ++pointIndex;

            x += m_tickDistance;
            ++xLineIndex;

            if (xLineIndex % m_majorTickStride == 0) {
                x += m_tickDistance;
                ++xLineIndex;
            }
        }

        while (y <= h) {
            points[pointIndex].x = 0.f;
            points[pointIndex].y = float(y);
            ++pointIndex;

            points[pointIndex].x = float(w);
            points[pointIndex].y = float(y);
            ++pointIndex;

            y += m_tickDistance;
            ++yLineIndex;

            if (yLineIndex % m_majorTickStride == 0) {
                y += m_tickDistance;
                ++yLineIndex;
            }
        }

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

        QSGGeometry *geometry =
                new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), nrMajorTicks * 2);
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometry->setLineWidth(float(m_majorTickLineWidth));
        geometryNode->setGeometry(geometry);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

        qreal x = m_tickDistance * m_majorTickStride, y = x;
        int pointIndex = 0;

        while (x < w) {
            points[pointIndex].x = float(x);
            points[pointIndex].y = 0.f;
            ++pointIndex;

            points[pointIndex].x = float(x);
            points[pointIndex].y = float(h);
            ++pointIndex;

            x += m_tickDistance * m_majorTickStride;
        }

        while (y < h) {
            points[pointIndex].x = 0.f;
            points[pointIndex].y = float(y);
            ++pointIndex;

            points[pointIndex].x = float(w);
            points[pointIndex].y = float(y);
            ++pointIndex;

            y += m_tickDistance * m_majorTickStride;
        }

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

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
        geometry->setDrawingMode(QSGGeometry::DrawLineLoop);
        geometry->setLineWidth(float(m_majorTickLineWidth));
        geometryNode->setGeometry(geometry);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

        points[0].x = 0.0f;
        points[0].y = 0.0f;

        points[1].x = float(w) - 1.0f;
        points[1].y = 0.0f;

        points[2].x = float(w) - 1.0f;
        points[2].y = float(h) - 1.0f;

        points[3].x = 0.0f;
        points[3].y = float(h) - 1.0f;

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
        geometryNode->setMaterial(material);

        QColor color = m_majorTickColor;
        color.setAlphaF(color.alphaF() * m_tickColorOpacity * this->opacity());
        material->setFlag(QSGMaterial::Blending);
        material->setColor(color);

        borderNode->appendChildNode(geometryNode);
    }

    return rootNode;
}
