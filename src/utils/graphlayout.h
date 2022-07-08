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
class ForceDirectedLayout : public AbstractLayout
{
public:
    explicit ForceDirectedLayout();
    ~ForceDirectedLayout();

    // AbstractGraphLayout interface
    bool layout(const Graph &graph);

private:
    void calculateRepulsion(QVector<QPointF> &forces, const Graph &graph);
    void calculateAttraction(QVector<QPointF> &forces, const Graph &graph);
    bool placeNodes(const QVector<QPointF> &forces, const Graph &graph);
};

}

#endif // GRAPHLAYOUT_H
