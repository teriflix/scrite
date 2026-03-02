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

#include "timelinecursoritem.h"

#include <QPainter>
#include <QPainterPath>

TimelineCursorItem::TimelineCursorItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    this->setImplicitWidth(8);
    this->setImplicitHeight(20);
}

TimelineCursorItem::~TimelineCursorItem() { }

void TimelineCursorItem::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    m_color = val;
    emit colorChanged();

    this->update();
}

void TimelineCursorItem::setLineWidth(qreal val)
{
    if (qFuzzyCompare(m_lineWidth, val))
        return;

    m_lineWidth = val;
    emit lineWidthChanged();

    this->update();
}

void TimelineCursorItem::paint(QPainter *painter)
{
    const QRectF itemRect(0, 0, this->width(), this->height());
    const qreal triangleSize = itemRect.width();

    QLineF line(itemRect.center().x(), itemRect.top(), itemRect.center().x(), itemRect.bottom());
    painter->setPen(QPen(m_color, m_lineWidth));
    painter->drawLine(line);

    if (itemRect.height() < 2 * triangleSize)
        return;

    painter->setPen(Qt::NoPen);

    QPainterPath topTriangle, bottomTriangle;

    topTriangle.moveTo(itemRect.topLeft());
    topTriangle.lineTo(itemRect.topRight());
    topTriangle.lineTo(itemRect.center().x(), itemRect.top() + triangleSize);
    topTriangle.closeSubpath();

    bottomTriangle.moveTo(itemRect.bottomLeft());
    bottomTriangle.lineTo(itemRect.bottomRight());
    bottomTriangle.lineTo(itemRect.center().x(), itemRect.bottom() - triangleSize);
    bottomTriangle.closeSubpath();

    painter->setBrush(QBrush(m_color));
    painter->drawPath(topTriangle);
    painter->drawPath(bottomTriangle);
}
