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

#ifndef GRIDBACKGROUNDITEM_H
#define GRIDBACKGROUNDITEM_H

#include <QQuickItem>

class GridBackgroundItem;

class GridBackgroundItemBorder : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~GridBackgroundItemBorder();

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
    Q_PROPERTY(qreal width
               READ width
               WRITE setWidth
               NOTIFY widthChanged)
    // clang-format on
    void setWidth(qreal val);
    qreal width() const { return m_width; }
    Q_SIGNAL void widthChanged();

private:
    friend class GridBackgroundItem;
    GridBackgroundItemBorder(QObject *parent = nullptr);

private:
    qreal m_width = 2.0;
    QColor m_color = QColor("blue");
};

class GridBackgroundItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(GridBackground)

public:
    explicit GridBackgroundItem(QQuickItem *parent = nullptr);
    ~GridBackgroundItem();

    // clang-format off
    Q_PROPERTY(qreal tickDistance
               READ tickDistance
               WRITE setTickDistance
               NOTIFY tickDistanceChanged)
    // clang-format on
    void setTickDistance(qreal val);
    qreal tickDistance() const { return m_tickDistance; }
    Q_SIGNAL void tickDistanceChanged();

    // clang-format off
    Q_PROPERTY(int majorTickStride
               READ majorTickStride
               WRITE setMajorTickStride
               NOTIFY majorTickStrideChanged)
    // clang-format on
    void setMajorTickStride(int val);
    int majorTickStride() const { return m_majorTickStride; }
    Q_SIGNAL void majorTickStrideChanged();

    // clang-format off
    Q_PROPERTY(qreal minorTickLineWidth
               READ minorTickLineWidth
               WRITE setMinorTickLineWidth
               NOTIFY minorTickLineWidthChanged)
    // clang-format on
    void setMinorTickLineWidth(qreal val);
    qreal minorTickLineWidth() const { return m_minorTickLineWidth; }
    Q_SIGNAL void minorTickLineWidthChanged();

    // clang-format off
    Q_PROPERTY(qreal majorTickLineWidth
               READ majorTickLineWidth
               WRITE setMajorTickLineWidth
               NOTIFY majorTickLineWidthChanged)
    // clang-format on
    void setMajorTickLineWidth(qreal val);
    qreal majorTickLineWidth() const { return m_majorTickLineWidth; }
    Q_SIGNAL void majorTickLineWidthChanged();

    // clang-format off
    Q_PROPERTY(QColor minorTickColor
               READ minorTickColor
               WRITE setMinorTickColor
               NOTIFY minorTickColorChanged)
    // clang-format on
    void setMinorTickColor(const QColor &val);
    QColor minorTickColor() const { return m_minorTickColor; }
    Q_SIGNAL void minorTickColorChanged();

    // clang-format off
    Q_PROPERTY(QColor majorTickColor
               READ majorTickColor
               WRITE setMajorTickColor
               NOTIFY majorTickColorChanged)
    // clang-format on
    void setMajorTickColor(const QColor &val);
    QColor majorTickColor() const { return m_majorTickColor; }
    Q_SIGNAL void majorTickColorChanged();

    // clang-format off
    Q_PROPERTY(qreal tickColorOpacity
               READ tickColorOpacity
               WRITE setTickColorOpacity
               NOTIFY tickColorOpacityChanged)
    // clang-format on
    void setTickColorOpacity(qreal val);
    qreal tickColorOpacity() const { return m_tickColorOpacity; }
    Q_SIGNAL void tickColorOpacityChanged();

    // clang-format off
    Q_PROPERTY(bool gridIsVisible
               READ gridIsVisible
               WRITE setGridIsVisible
               NOTIFY gridIsVisibleChanged)
    // clang-format on
    void setGridIsVisible(bool val);
    bool gridIsVisible() const { return m_gridIsVisible; }
    Q_SIGNAL void gridIsVisibleChanged();

    // clang-format off
    Q_PROPERTY(GridBackgroundItemBorder *border
               READ border
               CONSTANT )
    // clang-format on
    GridBackgroundItemBorder *border() const { return m_border; }
    Q_SIGNAL void borderChanged();

    // clang-format off
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

protected:
    // QQuickItem interface
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *nodeData);

private:
    bool m_gridIsVisible = true;
    qreal m_tickDistance = 10;
    int m_majorTickStride = 10;
    qreal m_tickColorOpacity = 1.0;
    qreal m_minorTickLineWidth = 1;
    qreal m_majorTickLineWidth = 2;
    QColor m_minorTickColor = QColor("lightsteelblue");
    QColor m_majorTickColor = QColor("blue");
    QColor m_backgroundColor = QColor(Qt::transparent);
    GridBackgroundItemBorder *m_border = new GridBackgroundItemBorder(this);
};

#endif // GRIDBACKGROUNDITEM_H
