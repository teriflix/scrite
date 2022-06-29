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

#include "graphlayout.h"
#include "timeprofiler.h"

#include <QMap>
#include <QHash>
#include <QtMath>
#include <QLineF>
#include <QTransform>
#include <QElapsedTimer>

using namespace GraphLayout;

static const qreal fdg_constant = 0.0001;

ForceDirectedLayout::ForceDirectedLayout() { }

ForceDirectedLayout::~ForceDirectedLayout() { }

bool ForceDirectedLayout::layout(const Graph &graph)
{
    // Sanity checks
    if (graph.nodes.isEmpty() || graph.edges.isEmpty())
        return false;

    // If the graph contains nodes that are not part of edges within it,
    // then we must not even bother laying it out.
    QHash<AbstractNode *, int> refCountMap;
    for (AbstractEdge *edge : qAsConst(graph.edges)) {
        const int i1 = graph.nodes.indexOf(edge->node1());
        const int i2 = graph.nodes.indexOf(edge->node2());
        if (i1 < 0 || i2 < 0)
            return false;
        refCountMap[edge->node1()]++;
        refCountMap[edge->node2()]++;
    }

    for (AbstractNode *node : qAsConst(graph.nodes)) {
        if (refCountMap.value(node, 0) == 0)
            return false;
    }

    // If we are here, then graph consists of only those nodes that are connected
    // to each other with edges. No zombie nodes and no edges that connect to nodes
    // outside the given graph.

    // Place the nodes in a circle and figure out maximum size of nodes
    const qreal angleStep = 2 * M_PI / qreal(graph.nodes.size());
    QSizeF maxSize(0, 0);
    qreal angle = 0;
    for (AbstractNode *node : qAsConst(graph.nodes)) {
        if (node->canBeMoved())
            node->setPosition(QPointF(qCos(angle), qSin(angle)));

        angle += angleStep;

        const QSizeF nodeSize = node->size();
        maxSize.setWidth(qMax(nodeSize.width(), maxSize.width()));
        maxSize.setHeight(qMax(nodeSize.height(), maxSize.height()));
    }

    // Perform force directed graph layout
    int nrIterations = 0;

    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < this->maxTime()) {
        QVector<QPointF> forces(graph.nodes.size(), QPointF(0, 0));
        calculateRepulsion(forces, graph);
        calculateAttraction(forces, graph);
        bool moved = placeNodes(forces, graph);

        ++nrIterations;
        if (!moved || (maxIterations() > 0 && nrIterations >= maxIterations()))
            break;
    }

    // Scale the placement of nodes such that we consider the node sizes.

    // First, lets compute the minimum space in pixels that should be present between
    // any two nodes in our graph.
    const qreal minNodeSpacingPx = this->minimumEdgeLength()
            + QLineF(QPointF(0, 0), QPointF(maxSize.width(), maxSize.height())).length();

    // Now, lets find out the least space between any two nodes in the layed out
    // graph.
    qreal minNodeSpacing = 240000.0;
    for (AbstractNode *n1 : qAsConst(graph.nodes)) {
        for (AbstractNode *n2 : qAsConst(graph.nodes)) {
            if (n1 == n2)
                continue;
            const qreal nodeSpacing = QLineF(n1->position(), n2->position()).length();
            minNodeSpacing = qMin(nodeSpacing, minNodeSpacing);
        }
    }

    // Compute the scaling factor based on the above.
    const qreal scale = minNodeSpacingPx / minNodeSpacing;

    // Apply the scaling
    for (AbstractNode *node : qAsConst(graph.nodes))
        node->setPosition(node->position() * scale);

    // Get the edges to compute their paths
    for (AbstractEdge *edge : qAsConst(graph.edges))
        edge->evaluateEdge();

    return true;
}

void ForceDirectedLayout::calculateRepulsion(QVector<QPointF> &forces, const Graph &graph)
{
    const qreal k = fdg_constant;
    for (int i = 0; i <= graph.nodes.size() - 2; i++) {
        for (int j = i + 1; j <= graph.nodes.size() - 1; j++) {
            const AbstractNode *n1 = graph.nodes.at(i);
            const AbstractNode *n2 = graph.nodes.at(j);
            const QPointF dp = n2->position() - n1->position();
            const qreal force = k / qSqrt((qPow(dp.x(), 2.0) + qPow(dp.y(), 2.0)));
            const qreal angle = qAtan2(dp.y(), dp.x());
            const QPointF delta(force * qCos(angle), force * qSin(angle));
            forces[i] -= delta;
            forces[j] += delta;
        }
    }
}

void ForceDirectedLayout::calculateAttraction(QVector<QPointF> &forces, const Graph &graph)
{
    const qreal k = fdg_constant;
    for (AbstractEdge *edge : graph.edges) {
        AbstractNode *n1 = edge->node1();
        AbstractNode *n2 = edge->node2();
        const QPointF dp = n2->position() - n1->position();
        const int i = graph.nodes.indexOf(n1);
        const int j = graph.nodes.indexOf(n2);
        const qreal force = k * (qPow(dp.x(), 2.0) + qPow(dp.y(), 2.0));
        const qreal angle = qAtan2(dp.y(), dp.x());
        const QPointF delta(force * qCos(angle), force * qSin(angle));
        forces[i] += delta;
        forces[j] -= delta;
    }
}

bool ForceDirectedLayout::placeNodes(const QVector<QPointF> &forces, const Graph &graph)
{
    bool moved = false;
    for (int i = 0; i < graph.nodes.size(); i++) {
        AbstractNode *node = graph.nodes.at(i);
        const QPointF force = forces.at(i);
        if (qFuzzyIsNull(force.x()) && qFuzzyIsNull(force.y()))
            continue;

        const QPointF pos = node->position() + force;
        node->setPosition(pos);
        moved = true;
    }

    return moved;
}
