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

#ifndef GRAPHLAYOUT_H
#define GRAPHLAYOUT_H

#include <QSizeF>
#include <QPointF>
#include <QVector>
#include <QVector2D>

namespace GraphLayout {

class AbstractNode
{
public:
    void setPosition(const QPointF &pos)
    {
        if (m_position == pos)
            return;
        m_position = pos;
        this->move(m_position);
    }
    QPointF position() const { return m_position; }

    virtual bool canBeMoved() const { return true; }
    virtual QSizeF size() const = 0;

    virtual QObject *containerObject() { return nullptr; }
    virtual const QObject *containerObject() const { return nullptr; }

protected:
    virtual void move(const QPointF &pos) = 0;

private:
    QPointF m_position;
};

class AbstractEdge
{
public:
    virtual AbstractNode *node1() const = 0;
    virtual AbstractNode *node2() const = 0;
    virtual void evaluateEdge() = 0;

    virtual QObject *containerObject() { return nullptr; }
    virtual const QObject *containerObject() const { return nullptr; }
};

struct Graph
{
    QVector<AbstractNode *> nodes;
    QVector<AbstractEdge *> edges;
};

class AbstractLayout
{
public:
    void setMaxTime(qint32 time) { m_maxtime = time; }
    qint32 maxTime() const { return m_maxtime; }

    void setMaxIterations(int val) { m_maxIterations = val; }
    int maxIterations() const { return m_maxIterations; }

    void setMinimumEdgeLength(qreal val) { m_minimumEdgeLength = val; }
    qreal minimumEdgeLength() const { return m_minimumEdgeLength; }

    virtual bool layout(const Graph &graph) = 0;

private:
    qint32 m_maxtime = 1000;
    int m_maxIterations = -1;
    qreal m_minimumEdgeLength = 0;
};

// https://en.wikipedia.org/wiki/Force-directed_graph_drawing
//
// Stress majorization layout (as in Graphviz's neato), which works directly in pixel
// space. Connected nodes are placed such that the gap between their boundaries is
// roughly the minimum edge length (for example, to accommodate edge labels), and
// other nodes are placed proportional to the number of edges between them.
//
// The layout is attempted multiple times from different starting positions (a radial
// placement around the most connected node, and variations of it), and the attempt
// with the least edge crossings, overlaps and spread is picked. A fixed random seed
// is used, so that the same graph always produces the same layout.
class ForceDirectedLayout : public AbstractLayout
{
public:
    explicit ForceDirectedLayout();
    ~ForceDirectedLayout();

    // AbstractGraphLayout interface
    bool layout(const Graph &graph) override;
};

}

#endif // GRAPHLAYOUT_H
