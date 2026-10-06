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

#include "graphlayout.h"
#include "timeprofiler.h"

#include <QSet>
#include <QHash>
#include <QRectF>
#include <QtMath>
#include <QLineF>
#include <QElapsedTimer>
#include <QRandomGenerator>

#include <limits>

using namespace GraphLayout;

namespace {

struct LayoutState
{
    int nrNodes = 0;
    QVector<QSizeF> sizes;
    QVector<bool> movable;
    QVector<QPair<int, int>> edges; // unique & undirected
    QVector<QVector<int>> adjacency;
    qreal maxDimension = 0; // largest width or height among all nodes
    qreal padding = 0; // breathing space around nodes
    qreal idealLength = 0; // ideal centre-to-centre distance between connected nodes
};

qreal vectorLength(const QPointF &p)
{
    return qSqrt(p.x() * p.x() + p.y() * p.y());
}

QRectF boxAt(const QPointF &centre, const QSizeF &size)
{
    return QRectF(centre.x() - size.width() / 2, centre.y() - size.height() / 2, size.width(),
                  size.height());
}

bool segmentIntersectsRect(const QLineF &line, const QRectF &rect)
{
    if (rect.contains(line.p1()) || rect.contains(line.p2()))
        return true;

    const QLineF sides[] = { QLineF(rect.topLeft(), rect.topRight()),
                             QLineF(rect.topRight(), rect.bottomRight()),
                             QLineF(rect.bottomRight(), rect.bottomLeft()),
                             QLineF(rect.bottomLeft(), rect.topLeft()) };
    for (const QLineF &side : sides) {
        if (line.intersects(side) == QLineF::BoundedIntersection)
            return true;
    }

    return false;
}

// Places the most connected node at the centre, and others in concentric rings around it based
// on their (BFS) distance from the centre. Each node gets an angular wedge, proportional to the
// number of leaves under it, within which its children are placed. If rng is provided, the order
// of children and the starting angle are shuffled, to produce variations of the placement.
QVector<QPointF> radialPlacement(const LayoutState &state, const QVector<QPointF> &initial,
                                 QRandomGenerator *rng)
{
    const int n = state.nrNodes;
    const qreal L = state.idealLength;
    QVector<QPointF> positions = initial;

    int root = 0;
    for (int i = 1; i < n; i++) {
        if (state.adjacency[i].size() > state.adjacency[root].size())
            root = i;
    }

    QVector<int> depth(n, -1);
    QVector<QVector<int>> children(n);
    QVector<int> bfsOrder;
    bfsOrder.reserve(n);

    depth[root] = 0;
    bfsOrder.append(root);
    for (int qi = 0; qi < bfsOrder.size(); qi++) {
        const int u = bfsOrder.at(qi);
        for (int v : state.adjacency.at(u)) {
            if (depth[v] >= 0)
                continue;
            depth[v] = depth[u] + 1;
            children[u].append(v);
            bfsOrder.append(v);
        }

        if (rng != nullptr) {
            QVector<int> &list = children[u];
            for (int i = list.size() - 1; i > 0; i--) {
                const int j = rng->bounded(i + 1);
                qSwap(list[i], list[j]);
            }
        }
    }

    // Number of leaves under each node, computed bottom-up.
    QVector<int> weight(n, 1);
    for (int qi = bfsOrder.size() - 1; qi >= 0; qi--) {
        const int u = bfsOrder.at(qi);
        if (children.at(u).isEmpty())
            continue;
        int w = 0;
        for (int c : children.at(u))
            w += weight.at(c);
        weight[u] = w;
    }

    // Radius of each ring, such that nodes in a ring don't crowd each other.
    int maxDepth = 0;
    for (int u : std::as_const(bfsOrder))
        maxDepth = qMax(maxDepth, depth.at(u));

    QVector<int> ringCounts(maxDepth + 2, 0);
    for (int u : std::as_const(bfsOrder))
        ++ringCounts[depth.at(u)];
    ringCounts[maxDepth + 1] = n - bfsOrder.size();

    QVector<qreal> ringRadius(maxDepth + 2, 0);
    for (int d = 1; d <= maxDepth + 1; d++)
        ringRadius[d] = qMax(ringRadius.at(d - 1) + L, ringCounts.at(d) * L / (2 * M_PI));

    // Assign wedges and place nodes.
    QVector<qreal> wedgeStart(n, 0), wedgeSpan(n, 0);
    wedgeStart[root] = rng ? rng->bounded(2 * M_PI) : 0;
    wedgeSpan[root] = 2 * M_PI;
    if (state.movable.at(root))
        positions[root] = QPointF(0, 0);

    for (int u : std::as_const(bfsOrder)) {
        qreal start = wedgeStart.at(u);
        for (int c : children.at(u)) {
            const qreal span = wedgeSpan.at(u) * qreal(weight.at(c)) / qreal(weight.at(u));
            wedgeStart[c] = start;
            wedgeSpan[c] = span;
            start += span;

            if (state.movable.at(c)) {
                const qreal angle = wedgeStart.at(c) + span / 2;
                const qreal radius = ringRadius.at(depth.at(c));
                positions[c] = QPointF(radius * qCos(angle), radius * qSin(angle));
            }
        }
    }

    // Nodes not reachable from the root (should not happen for a connected graph) go into
    // an outer ring.
    const int nrUnreached = ringCounts.at(maxDepth + 1);
    if (nrUnreached > 0) {
        const qreal radius = ringRadius.at(maxDepth + 1);
        int index = 0;
        for (int i = 0; i < n; i++) {
            if (depth.at(i) >= 0)
                continue;
            const qreal angle = 2 * M_PI * qreal(index++) / qreal(nrUnreached);
            if (state.movable.at(i))
                positions[i] = QPointF(radius * qCos(angle), radius * qSin(angle));
        }
    }

    // Small jitter on variations, so that symmetric placements can break out.
    if (rng != nullptr) {
        for (int i = 0; i < n; i++) {
            if (state.movable.at(i))
                positions[i] += QPointF((rng->generateDouble() - 0.5) * L * 0.2,
                                        (rng->generateDouble() - 0.5) * L * 0.2);
        }
    }

    return positions;
}

QVector<QPointF> randomPlacement(const LayoutState &state, const QVector<QPointF> &initial,
                                 QRandomGenerator &rng)
{
    QVector<QPointF> positions = initial;
    const qreal radius = qSqrt(qreal(state.nrNodes)) * state.idealLength * 0.75;
    for (int i = 0; i < state.nrNodes; i++) {
        if (!state.movable.at(i))
            continue;
        const qreal r = radius * qSqrt(rng.generateDouble());
        const qreal angle = rng.bounded(2 * M_PI);
        positions[i] = QPointF(r * qCos(angle), r * qSin(angle));
    }
    return positions;
}

// Number of edges on the shortest path between every pair of nodes. Pairs that are not
// connected are treated as being one hop farther than the farthest connected pair.
QVector<QVector<int>> hopDistances(const LayoutState &state)
{
    const int n = state.nrNodes;
    QVector<QVector<int>> hops(n, QVector<int>(n, -1));
    int maxHops = 0;
    for (int s = 0; s < n; s++) {
        QVector<int> &row = hops[s];
        QVector<int> queue;
        queue.reserve(n);
        row[s] = 0;
        queue.append(s);
        for (int qi = 0; qi < queue.size(); qi++) {
            const int u = queue.at(qi);
            for (int v : state.adjacency.at(u)) {
                if (row.at(v) >= 0)
                    continue;
                row[v] = row.at(u) + 1;
                maxHops = qMax(maxHops, row.at(v));
                queue.append(v);
            }
        }
    }

    for (QVector<int> &row : hops) {
        for (int &h : row) {
            if (h < 0)
                h = maxHops + 1;
        }
    }

    return hops;
}

// Stress majorization (as in Graphviz's neato). Every pair of nodes has a target distance of
// hops x idealLength, weighted by 1/distance^2 so that nearby pairs matter the most. Each node
// is moved, in turn, to the position that best satisfies its target distances given the current
// positions of all other nodes. Unlike spring models, this keeps edge lengths uniform instead of
// stretching edges that lead to clusters of nodes.
void runStressMajorization(const LayoutState &state, const QVector<QVector<int>> &hops,
                           QVector<QPointF> &positions, int nrIterations)
{
    const int n = state.nrNodes;

    for (int iteration = 0; iteration < nrIterations; iteration++) {
        qreal maxMovement = 0;

        for (int i = 0; i < n; i++) {
            if (!state.movable.at(i))
                continue;

            QPointF target(0, 0);
            qreal totalWeight = 0;
            for (int j = 0; j < n; j++) {
                if (i == j)
                    continue;

                const qreal targetDistance = hops.at(i).at(j) * state.idealLength;
                const qreal weight = 1.0 / (targetDistance * targetDistance);
                const QPointF delta = positions.at(i) - positions.at(j);
                const qreal d = qMax(vectorLength(delta), 1e-6);
                target += (positions.at(j) + delta * (targetDistance / d)) * weight;
                totalWeight += weight;
            }

            target /= totalWeight;
            maxMovement = qMax(maxMovement, vectorLength(target - positions.at(i)));
            positions[i] = target;
        }

        if (maxMovement < 0.1)
            break;
    }
}

// Pushes apart nodes whose (padded) boxes overlap, along the axis of least overlap.
void removeOverlaps(const LayoutState &state, QVector<QPointF> &positions)
{
    const int n = state.nrNodes;
    const qreal margin = state.padding / 2;

    for (int pass = 0; pass < 100; pass++) {
        bool overlapped = false;

        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                const bool movable1 = state.movable.at(i);
                const bool movable2 = state.movable.at(j);
                if (!movable1 && !movable2)
                    continue;

                const QRectF r1 = boxAt(positions.at(i), state.sizes.at(i))
                                          .adjusted(-margin, -margin, margin, margin);
                const QRectF r2 = boxAt(positions.at(j), state.sizes.at(j))
                                          .adjusted(-margin, -margin, margin, margin);
                if (!r1.intersects(r2))
                    continue;

                overlapped = true;

                const QRectF overlap = r1.intersected(r2);
                QPointF shift; // how much to move node i away from node j
                if (overlap.width() < overlap.height())
                    shift = QPointF(positions.at(i).x() < positions.at(j).x() ? -overlap.width()
                                                                              : overlap.width(),
                                    0);
                else
                    shift = QPointF(0,
                                    positions.at(i).y() < positions.at(j).y() ? -overlap.height()
                                                                              : overlap.height());

                if (movable1 && movable2) {
                    positions[i] += shift / 2;
                    positions[j] -= shift / 2;
                } else if (movable1)
                    positions[i] += shift;
                else
                    positions[j] -= shift;
            }
        }

        if (!overlapped)
            break;
    }
}

// Lower score is better. Edge crossings and overlaps dominate, followed by uneven edge lengths
// and the overall area occupied by the layout.
qreal evaluateLayout(const LayoutState &state, const QVector<QPointF> &positions)
{
    const int n = state.nrNodes;

    QVector<QRectF> rects(n);
    QRectF boundingRect;
    for (int i = 0; i < n; i++) {
        rects[i] = boxAt(positions.at(i), state.sizes.at(i));
        boundingRect |= rects.at(i);
    }

    QVector<QLineF> lines;
    lines.reserve(state.edges.size());
    for (const QPair<int, int> &edge : state.edges)
        lines.append(QLineF(positions.at(edge.first), positions.at(edge.second)));

    int nrCrossings = 0;
    for (int e1 = 0; e1 < state.edges.size() - 1; e1++) {
        for (int e2 = e1 + 1; e2 < state.edges.size(); e2++) {
            const QPair<int, int> &a = state.edges.at(e1);
            const QPair<int, int> &b = state.edges.at(e2);
            if (a.first == b.first || a.first == b.second || a.second == b.first
                || a.second == b.second)
                continue;
            if (lines.at(e1).intersects(lines.at(e2)) == QLineF::BoundedIntersection)
                ++nrCrossings;
        }
    }

    int nrEdgesOverNodes = 0;
    for (int e = 0; e < state.edges.size(); e++) {
        const QPair<int, int> &edge = state.edges.at(e);
        for (int k = 0; k < n; k++) {
            if (k == edge.first || k == edge.second)
                continue;
            if (segmentIntersectsRect(lines.at(e), rects.at(k)))
                ++nrEdgesOverNodes;
        }
    }

    int nrNodeOverlaps = 0;
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (rects.at(i).intersects(rects.at(j)))
                ++nrNodeOverlaps;
        }
    }

    qreal edgeLengthVariation = 0;
    if (!lines.isEmpty()) {
        qreal sum = 0, sumSquares = 0;
        for (const QLineF &line : std::as_const(lines)) {
            const qreal length = line.length();
            sum += length;
            sumSquares += length * length;
        }
        const qreal mean = sum / qreal(lines.size());
        const qreal variance = qMax(sumSquares / qreal(lines.size()) - mean * mean, 0.0);
        if (mean > 0)
            edgeLengthVariation = qSqrt(variance) / mean;
    }

    const qreal L = state.idealLength;
    const qreal areaRatio = (boundingRect.width() * boundingRect.height()) / (qreal(n) * L * L);

    return nrCrossings * 10.0 + nrEdgesOverNodes * 10.0 + nrNodeOverlaps * 20.0
            + edgeLengthVariation * 5.0 + areaRatio;
}

}

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
    for (AbstractEdge *edge : std::as_const(graph.edges)) {
        const int i1 = graph.nodes.indexOf(edge->node1());
        const int i2 = graph.nodes.indexOf(edge->node2());
        if (i1 < 0 || i2 < 0)
            return false;
        refCountMap[edge->node1()]++;
        refCountMap[edge->node2()]++;
    }

    for (AbstractNode *node : std::as_const(graph.nodes)) {
        if (refCountMap.value(node, 0) == 0)
            return false;
    }

    // If we are here, then graph consists of only those nodes that are connected
    // to each other with edges. No zombie nodes and no edges that connect to nodes
    // outside the given graph.

    // Capture the graph into a form that's quick to work with.
    LayoutState state;
    state.nrNodes = graph.nodes.size();
    state.sizes.resize(state.nrNodes);
    state.movable.resize(state.nrNodes);
    state.adjacency.resize(state.nrNodes);

    QHash<AbstractNode *, int> indexMap;
    QVector<QPointF> initialPositions(state.nrNodes);
    for (int i = 0; i < state.nrNodes; i++) {
        AbstractNode *node = graph.nodes.at(i);
        indexMap[node] = i;
        state.sizes[i] = node->size();
        state.movable[i] = node->canBeMoved();
        state.maxDimension =
                qMax(state.maxDimension, qMax(state.sizes[i].width(), state.sizes[i].height()));
        initialPositions[i] = node->position();
    }

    QSet<QPair<int, int>> edgeSet;
    for (AbstractEdge *edge : std::as_const(graph.edges)) {
        const int i1 = indexMap.value(edge->node1());
        const int i2 = indexMap.value(edge->node2());
        if (i1 == i2)
            continue;
        const QPair<int, int> key(qMin(i1, i2), qMax(i1, i2));
        if (edgeSet.contains(key))
            continue;
        edgeSet.insert(key);
        state.edges.append(key);
        state.adjacency[key.first].append(key.second);
        state.adjacency[key.second].append(key.first);
    }

    state.padding = qMax(state.maxDimension * 0.1, 10.0);
    state.idealLength =
            state.maxDimension + qMax(this->minimumEdgeLength(), 0.0) + 2 * state.padding;

    // Attempt the layout from several starting positions, and pick the best one. The first
    // attempt always runs to completion; others are attempted only within the time budget.
    const int maxAttempts = 8;
    const int nrIterationsPerAttempt =
            this->maxIterations() > 0 ? qBound(50, this->maxIterations() / maxAttempts, 1000) : 500;

    QRandomGenerator rng(20200101); // fixed seed, so that layouts are reproducible
    const QVector<QVector<int>> hops = hopDistances(state);

    QVector<QPointF> bestPositions;
    qreal bestScore = std::numeric_limits<qreal>::max();

    QElapsedTimer timer;
    timer.start();

    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        if (attempt > 0 && timer.elapsed() >= this->maxTime())
            break;

        QVector<QPointF> positions;
        if (attempt == 0)
            positions = radialPlacement(state, initialPositions, nullptr);
        else if (attempt % 3 == 0)
            positions = randomPlacement(state, initialPositions, rng);
        else
            positions = radialPlacement(state, initialPositions, &rng);

        runStressMajorization(state, hops, positions, nrIterationsPerAttempt);
        removeOverlaps(state, positions);

        const qreal score = evaluateLayout(state, positions);
        if (score < bestScore) {
            bestScore = score;
            bestPositions = positions;
        }
    }

    // Apply the best placement
    for (int i = 0; i < state.nrNodes; i++) {
        if (state.movable.at(i))
            graph.nodes.at(i)->setPosition(bestPositions.at(i));
    }

    // Get the edges to compute their paths
    for (AbstractEdge *edge : std::as_const(graph.edges))
        edge->evaluateEdge();

    return true;
}
