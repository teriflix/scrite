/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "itemsboundingbox.h"
#include <QtConcurrentMap>

BoundingBoxItem::BoundingBoxItem(QObject *parent)
                :QObject(parent),
                 m_item(qobject_cast<QQuickItem*>(parent)),
                 m_boundingBox(nullptr)
{
    connect(m_item, &QQuickItem::xChanged, this, &BoundingBoxItem::itemRectChanged);
    connect(m_item, &QQuickItem::yChanged, this, &BoundingBoxItem::itemRectChanged);
    connect(m_item, &QQuickItem::widthChanged, this, &BoundingBoxItem::itemRectChanged);
    connect(m_item, &QQuickItem::heightChanged, this, &BoundingBoxItem::itemRectChanged);
}

BoundingBoxItem::~BoundingBoxItem()
{
    if(m_boundingBox != nullptr)
        m_boundingBox->removeItem(this);

    m_boundingBox = nullptr;
    m_item = nullptr;
}

BoundingBoxItem *BoundingBoxItem::qmlAttachedProperties(QObject *object)
{
    return new BoundingBoxItem(object);
}

void BoundingBoxItem::setBelongsTo(ItemsBoundingBox *val)
{
    if(m_boundingBox == val)
        return;

    if(m_boundingBox != nullptr && m_item != nullptr)
    {
        m_boundingBox->removeItem(this);
        disconnect(m_boundingBox, &ItemsBoundingBox::destroyed, this, &BoundingBoxItem::onBoundingBoxDestroyed);
    }

    m_boundingBox = val;

    if(m_boundingBox != nullptr && m_item != nullptr)
    {
        m_boundingBox->addItem(this);
        connect(m_boundingBox, &ItemsBoundingBox::destroyed, this, &BoundingBoxItem::onBoundingBoxDestroyed);
    }

    emit belongsToChanged();
}

QRectF BoundingBoxItem::itemRect() const
{
    if(m_item == nullptr)
        return QRectF();

    return QRectF(m_item->x(), m_item->y(), m_item->width(), m_item->height());
}

void BoundingBoxItem::onBoundingBoxDestroyed()
{
    m_boundingBox = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

ItemsBoundingBox::ItemsBoundingBox(QObject *parent)
                 :QObject(parent)
{
    connect(this, &ItemsBoundingBox::itemCountChanged, this, &ItemsBoundingBox::computeBoundingBoxLater);
}

ItemsBoundingBox::~ItemsBoundingBox()
{

}

void ItemsBoundingBox::setMargins(const QMarginsF &val)
{
    if(m_margins == val)
        return;
    m_margins = val;
    emit marginsChanged();
}

QQmlListProperty<BoundingBoxItem> ItemsBoundingBox::items()
{
    return QQmlListProperty<BoundingBoxItem>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &ItemsBoundingBox::staticItemCount,
                &ItemsBoundingBox::staticItemAt);
}

void ItemsBoundingBox::addItem(BoundingBoxItem *ptr)
{
    if(ptr == nullptr || m_items.indexOf(ptr) >= 0)
        return;

    connect(ptr, &BoundingBoxItem::itemRectChanged, this, &ItemsBoundingBox::computeBoundingBoxLater);
    m_items.append(ptr);
    emit itemCountChanged();
}

void ItemsBoundingBox::removeItem(BoundingBoxItem *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_items.indexOf(ptr);
    if(index < 0)
        return;

    m_items.removeAt(index);
    disconnect(ptr, &BoundingBoxItem::itemRectChanged, this, &ItemsBoundingBox::computeBoundingBoxLater);

    emit itemCountChanged();
}

BoundingBoxItem *ItemsBoundingBox::itemAt(int index) const
{
    return index < 0 || index >= m_items.size() ? nullptr : m_items.at(index);
}

void ItemsBoundingBox::clearItems()
{
    while(m_items.size())
        this->removeItem(m_items.first());
}

void ItemsBoundingBox::timerEvent(QTimerEvent *event)
{
    if(m_computeTimer.timerId() == event->timerId())
    {
        this->computeBoundingBox();
        m_computeTimer.stop();
        return;
    }

    QObject::timerEvent(event);
}

void ItemsBoundingBox::setRect(const QRectF &val)
{
    if(m_rect == val)
        return;

    m_rect = val;
    emit rectChanged();
}

inline void uniteBox(QRectF &box, const QRectF &itemRect) { box |= itemRect; }
inline QRectF itemRect(BoundingBoxItem *item) { return item->itemRect(); }

void ItemsBoundingBox::computeBoundingBox()
{
    QRectF box = QtConcurrent::blockingMappedReduced(m_items, itemRect, uniteBox,QtConcurrent::UnorderedReduce);
    box.adjust(m_margins.left(), -m_margins.top(), m_margins.right(), m_margins.bottom());

    this->setRect(box);
}

void ItemsBoundingBox::computeBoundingBoxLater()
{
    m_computeTimer.start(0, this);
}

BoundingBoxItem *ItemsBoundingBox::staticItemAt(QQmlListProperty<BoundingBoxItem> *list, int index)
{
    return reinterpret_cast< ItemsBoundingBox* >(list->data)->itemAt(index);
}

int ItemsBoundingBox::staticItemCount(QQmlListProperty<BoundingBoxItem> *list)
{
    return reinterpret_cast< ItemsBoundingBox* >(list->data)->itemCount();
}
