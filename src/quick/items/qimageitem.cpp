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

#include <QSGNode>
#include <QSGTexture>
#include <QQuickWindow>
#include <QSGOpaqueTextureMaterial>

QImageItem::QImageItem(QQuickItem *parentItem) : QQuickItem(parentItem)
{
    this->setFlag(QQuickItem::ItemHasContents);
    connect(this, &QImageItem::imageChanged, this, &QQuickItem::update);
}

QImageItem::~QImageItem()
{

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

QSGNode *QImageItem::updatePaintNode(QSGNode *oldRoot, UpdatePaintNodeData *)
{
    if (oldRoot)
        delete oldRoot;

    QSGNode *rootNode = new QSGNode;
    if (m_image.isNull())
        return rootNode;

    const qreal w = this->width();
    const qreal h = this->height();

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

    rectPoints[0].set(0.f, 0.f, 0.f, 0.f);
    rectPoints[1].set(float(w), 0.f, 1.f, 0.f);
    rectPoints[2].set(float(w), float(h), 1.f, 1.f);

    rectPoints[3].set(0.f, 0.f, 0.f, 0.f);
    rectPoints[4].set(float(w), float(h), 1.f, 1.f);
    rectPoints[5].set(0.f, float(h), 0.f, 1.f);

    QSGTexture *texture = this->window()->createTextureFromImage(m_image);
    QSGOpaqueTextureMaterial *textureMaterial = new QSGOpaqueTextureMaterial;
    textureMaterial->setFlag(QSGMaterial::Blending);
    textureMaterial->setFiltering(QSGTexture::Nearest);
    textureMaterial->setTexture(texture);
    geoNode->setMaterial(textureMaterial);

    return rootNode;
}
