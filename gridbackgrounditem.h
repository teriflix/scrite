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

#ifndef GRIDBACKGROUNDITEM_H
#define GRIDBACKGROUNDITEM_H

#include <QQuickItem>

class GridBackgroundItem : public QQuickItem
{
    Q_OBJECT

public:
    GridBackgroundItem(QQuickItem *parent=nullptr);
    ~GridBackgroundItem();

    Q_PROPERTY(qreal tickDistance READ tickDistance WRITE setTickDistance NOTIFY tickDistanceChanged)
    void setTickDistance(qreal val);
    qreal tickDistance() const { return m_tickDistance; }
    Q_SIGNAL void tickDistanceChanged();

    Q_PROPERTY(int majorTickStride READ majorTickStride WRITE setMajorTickStride NOTIFY majorTickStrideChanged)
    void setMajorTickStride(int val);
    int majorTickStride() const { return m_majorTickStride; }
    Q_SIGNAL void majorTickStrideChanged();

    Q_PROPERTY(qreal minorTickLineWidth READ minorTickLineWidth WRITE setMinorTickLineWidth NOTIFY minorTickLineWidthChanged)
    void setMinorTickLineWidth(qreal val);
    qreal minorTickLineWidth() const { return m_minorTickLineWidth; }
    Q_SIGNAL void minorTickLineWidthChanged();

    Q_PROPERTY(qreal majorTickLineWidth READ majorTickLineWidth WRITE setMajorTickLineWidth NOTIFY majorTickLineWidthChanged)
    void setMajorTickLineWidth(qreal val);
    qreal majorTickLineWidth() const { return m_majorTickLineWidth; }
    Q_SIGNAL void majorTickLineWidthChanged();

    Q_PROPERTY(QColor minorTickColor READ minorTickColor WRITE setMinorTickColor NOTIFY minorTickColorChanged)
    void setMinorTickColor(const QColor &val);
    QColor minorTickColor() const { return m_minorTickColor; }
    Q_SIGNAL void minorTickColorChanged();

    Q_PROPERTY(QColor majorTickColor READ majorTickColor WRITE setMajorTickColor NOTIFY majorTickColorChanged)
    void setMajorTickColor(const QColor &val);
    QColor majorTickColor() const { return m_majorTickColor; }
    Q_SIGNAL void majorTickColorChanged();

    Q_PROPERTY(qreal tickColorOpacity READ tickColorOpacity WRITE setTickColorOpacity NOTIFY tickColorOpacityChanged)
    void setTickColorOpacity(qreal val);
    qreal tickColorOpacity() const { return m_tickColorOpacity; }
    Q_SIGNAL void tickColorOpacityChanged();

protected:
    // QQuickItem interface
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *nodeData);

private:
    qreal m_tickDistance = 10;
    int   m_majorTickStride = 10;
    qreal m_tickColorOpacity = 1.0;
    qreal m_minorTickLineWidth = 1;
    qreal m_majorTickLineWidth = 1;
    QColor m_minorTickColor = QColor("lightsteelblue");
    QColor m_majorTickColor = QColor("blue");
};

#endif // GRIDBACKGROUNDITEM_H
