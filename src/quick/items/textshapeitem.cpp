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

#include "textshapeitem.h"

TextShapeItem::TextShapeItem(QQuickItem *parent) : AbstractShapeItem(parent)
{
    this->setOutlineColor(QColor(0, 0, 0, 128));
}

TextShapeItem::~TextShapeItem() { }

void TextShapeItem::setText(const QString &val)
{
    if (m_text == val)
        return;

    m_text = val;
    emit textChanged();

    this->update();
}

void TextShapeItem::setFont(const QFont &val)
{
    if (m_font == val)
        return;

    m_font = val;
    emit fontChanged();

    this->update();
}

QPainterPath TextShapeItem::shape() const
{
    QPainterPath path;
    path.addText(QPointF(0, 0), m_font, m_text);
    return path;
}
