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

#include "qimageitem.h"
#include "application.h"

#include <QSGNode>
#include <QSGTexture>
#include <QQuickWindow>
#include <QSGOpaqueTextureMaterial>
#include <QPainter>

QImageItem::QImageItem(QQuickItem *parentItem) : QQuickPaintedItem(parentItem)
{
    connect(this, &QImageItem::imageChanged, this, &QQuickItem::update);
    connect(this, &QImageItem::fillModeChanged, this, &QQuickItem::update);
    connect(this, &QImageItem::useSoftwareRendererChanged, this, &QQuickItem::update);
}

QImageItem::~QImageItem() { }

void QImageItem::setUseSoftwareRenderer(bool val)
{
    if (m_useSoftwareRenderer == val)
        return;

    m_useSoftwareRenderer = val;
    emit useSoftwareRendererChanged();
}

void QImageItem::setFillMode(FillMode val)
{
    if (m_fillMode == val)
        return;

    m_fillMode = val;

    this->setClip(m_fillMode == PreserveAspectCrop);

    emit fillModeChanged();
}

QImage QImageItem::fromIcon(const QIcon &icon, const QSize &size)
{
    return icon.pixmap(size).toImage();
}

void QImageItem::setImage(const QImage &val)
{
    if (m_image == val)
        return;

    m_image = val;
    emit imageChanged();
}

void QImageItem::paint(QPainter *painter)
{
    if (m_image.isNull())
        return;

    QRectF sourceRect, targetRect;

    switch (m_fillMode) {
    case Stretch:
        sourceRect = m_image.rect();
        targetRect = this->boundingRect();
        break;
    case PreserveAspectFit:
        sourceRect = m_image.rect();
        targetRect =
                QRectF(QPointF(0, 0), sourceRect.size().scaled(this->size(), Qt::KeepAspectRatio));
        targetRect.moveCenter(this->boundingRect().center());
        break;
    case PreserveAspectCrop:
        sourceRect = m_image.rect();
        targetRect = QRectF(QPointF(0, 0),
                            sourceRect.size().scaled(this->size(), Qt::KeepAspectRatioByExpanding));
        targetRect.moveCenter(this->boundingRect().center());
        break;
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawImage(targetRect, m_image, sourceRect);
}

QSGNode *QImageItem::updatePaintNode(QSGNode *oldRoot, UpdatePaintNodeData *data)
{
    if (m_useSoftwareRenderer) {
        if (lastPaintMode == SceneGraphPaintMode) {
            if (oldRoot)
                delete oldRoot;

            oldRoot = nullptr;
        }

        lastPaintMode = PainterPaintMode;
        return QQuickPaintedItem::updatePaintNode(oldRoot, data);
    }

    lastPaintMode = SceneGraphPaintMode;

    if (oldRoot)
        delete oldRoot;

    QSGNode *rootNode = new QSGNode;
    if (m_image.isNull())
        return rootNode;

    QSGOpacityNode *opacityNode = new QSGOpacityNode;
    opacityNode->setFlag(QSGNode::OwnedByParent);
    opacityNode->setOpacity(this->opacity());
    rootNode->appendChildNode(opacityNode);

    QSGGeometryNode *geoNode = new QSGGeometryNode;
    geoNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial | QSGNode::OwnedByParent);
    opacityNode->appendChildNode(geoNode);

    QSGGeometry *rectGeo = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 6);
    rectGeo->setDrawingMode(QSGGeometry::DrawTriangles);
    geoNode->setGeometry(rectGeo);

    QSGGeometry::TexturedPoint2D *rectPoints = rectGeo->vertexDataAsTexturedPoint2D();

    QRectF imageRect = this->boundingRect();

    switch (m_fillMode) {
    case Stretch:
        break;
    case PreserveAspectFit:
        imageRect = QRectF(QPointF(0, 0),
                           QSizeF(m_image.size()).scaled(this->size(), Qt::KeepAspectRatio));
        imageRect.moveCenter(this->boundingRect().center());
        break;
    case PreserveAspectCrop:
        imageRect =
                QRectF(QPointF(0, 0),
                       QSizeF(m_image.size()).scaled(this->size(), Qt::KeepAspectRatioByExpanding));
        imageRect.moveCenter(this->boundingRect().center());
        break;
    }

    const float x1 = (float)imageRect.left();
    const float y1 = (float)imageRect.top();
    const float x2 = (float)imageRect.right();
    const float y2 = (float)imageRect.bottom();

    rectPoints[0].set(x1, y1, 0.f, 0.f);
    rectPoints[1].set(x2, y1, 1.f, 0.f);
    rectPoints[2].set(x2, y2, 1.f, 1.f);

    rectPoints[3].set(x1, y1, 0.f, 0.f);
    rectPoints[4].set(x2, y2, 1.f, 1.f);
    rectPoints[5].set(x1, y2, 0.f, 1.f);

    QSGTexture *texture = this->window()->createTextureFromImage(m_image);
    texture->setFiltering(QSGTexture::Linear);
    texture->setMipmapFiltering(QSGTexture::Linear);

    QSGOpaqueTextureMaterial *textureMaterial = new QSGOpaqueTextureMaterial;
    textureMaterial->setFlag(QSGMaterial::Blending);
    textureMaterial->setFiltering(QSGTexture::Linear);
    textureMaterial->setTexture(texture);
    geoNode->setMaterial(textureMaterial);

    return rootNode;
}
