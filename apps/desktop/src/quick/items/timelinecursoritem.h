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

#ifndef TIMELINECURSORITEM_H
#define TIMELINECURSORITEM_H

#include <QQuickPaintedItem>

class TimelineCursorItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TimelineCursorItem(QQuickItem *parent = nullptr);
    virtual ~TimelineCursorItem();

    // clang-format off
    Q_PROPERTY(QColor color
               READ color
               WRITE setColor
               NOTIFY colorChanged)
    // clang-format on
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    // clang-format off
    Q_PROPERTY(qreal lineWidth
               READ lineWidth
               WRITE setLineWidth
               NOTIFY lineWidthChanged)
    // clang-format on
    void setLineWidth(qreal val);
    qreal lineWidth() const { return m_lineWidth; }
    Q_SIGNAL void lineWidthChanged();

    // QQuickPaintedItem interface
    void paint(QPainter *painter);

private:
    QColor m_color = Qt::black;
    qreal m_lineWidth = 2.0;
};

#endif // TIMELINECURSORITEM_H
