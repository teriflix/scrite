/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "characterrelationshipsgraph.h"

#include "hourglass.h"
#include "application.h"

#include <QtMath>
#include <QFontMetrics>
#include <QElapsedTimer>

CharacterRelationshipsGraphNode::CharacterRelationshipsGraphNode(QObject *parent)
    : QObject(parent),
      m_character(this, "character")
{

}


CharacterRelationshipsGraphNode::~CharacterRelationshipsGraphNode()
{

}

void CharacterRelationshipsGraphNode::setCharacter(Character *val)
{
    if(m_character == val)
        return;

    m_character = val;
    emit characterChanged();
}

void CharacterRelationshipsGraphNode::resetCharacter()
{
    m_character = nullptr;
    emit characterChanged();

    m_rect = QRectF();
    emit rectChanged();
}

void CharacterRelationshipsGraphNode::setRect(const QRectF &val)
{
    if(m_rect == val)
        return;

    m_rect = val;
    emit rectChanged();
}

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipsGraphEdge::CharacterRelationshipsGraphEdge(QObject *parent)
    : QObject(parent),
      m_relationship(this, "relationship")
{

}

CharacterRelationshipsGraphEdge::~CharacterRelationshipsGraphEdge()
{

}

QString CharacterRelationshipsGraphEdge::pathString() const
{
    return Application::instance()->painterPathToString(m_path);
}

void CharacterRelationshipsGraphEdge::setRelationship(Relationship *val)
{
    if(m_relationship == val)
        return;

    m_relationship = val;
    emit relationshipChanged();
}

void CharacterRelationshipsGraphEdge::resetRelationship()
{
    m_relationship = nullptr;
    emit relationshipChanged();

    m_path = QPainterPath();
    emit pathChanged();
}

void CharacterRelationshipsGraphEdge::setPath(const QPainterPath &val)
{
    if(m_path == val)
        return;

    m_path = val;

    if(m_path.isEmpty())
    {
        m_labelPos = QPointF(0,0);
        m_labelAngle = 0;
    }
    else
    {
        m_labelPos = m_path.pointAtPercent(0.5);
        m_labelAngle = qRadiansToDegrees( qAtan(m_path.slopeAtPercent(0.5)) );
    }

    emit pathChanged();
}

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipsGraph::CharacterRelationshipsGraph(QObject *parent)
    : QObject(parent),
      m_structure(this, "structure")
{

}

CharacterRelationshipsGraph::~CharacterRelationshipsGraph()
{

}

void CharacterRelationshipsGraph::setNodeSize(const QSizeF &val)
{
    if(m_nodeSize == val)
        return;

    m_nodeSize = val;
    emit nodeSizeChanged();

    this->load();
}

void CharacterRelationshipsGraph::setStructure(Structure *val)
{
    if(m_structure == val)
        return;

    m_structure = val;
    emit structureChanged();

    this->load();
}

void CharacterRelationshipsGraph::setFilterByCharacterNames(const QStringList &val)
{
    if(m_filterByCharacterNames == val)
        return;

    m_filterByCharacterNames = val;
    emit filterByCharacterNamesChanged();

    // this->load();
}

void CharacterRelationshipsGraph::setMaxTime(int val)
{
    const int val2 = qBound(10, val, 5000);
    if(m_maxTime == val2)
        return;

    m_maxTime = val2;
    emit maxTimeChanged();
}

void CharacterRelationshipsGraph::setMaxIterations(int val)
{
    if(m_maxIterations == val)
        return;

    m_maxIterations = val;
    emit maxIterationsChanged();
}

void CharacterRelationshipsGraph::reload()
{
    this->load();
}

void CharacterRelationshipsGraph::classBegin()
{
    m_componentLoaded = false;
}

void CharacterRelationshipsGraph::componentComplete()
{
    m_componentLoaded = true;
    this->load();
}

void CharacterRelationshipsGraph::setGraphBoundingRect(const QRectF &val)
{
    if(m_graphBoundingRect == val)
        return;

    m_graphBoundingRect = val;
    if(m_graphBoundingRect.isEmpty())
        m_graphBoundingRect.setSize(QSizeF(500,500));
    emit graphBoundingRectChanged();
}

void CharacterRelationshipsGraph::resetStructure()
{
    m_structure = nullptr;
    this->load();
    emit structureChanged();
}

void CharacterRelationshipsGraph::load()
{
    HourGlass hourGlass;

    QList<CharacterRelationshipsGraphNode*> nodes = m_nodes.list();
    m_nodes.clear();
    qDeleteAll(nodes);
    nodes.clear();

    QList<CharacterRelationshipsGraphEdge*> edges = m_edges.list();
    m_edges.clear();
    qDeleteAll(edges);
    edges.clear();

    if(m_structure.isNull() || !m_componentLoaded)
    {
        this->setGraphBoundingRect( QRectF(0,0,0,0) );
        emit updated();
        return;
    }

    // For the moment, we ignore the filter-by-character-names list.

    // First we collect all nodes and edges
    QString longestRelationshipName;
    QList<Character*> loneCharacters;
    QMap<Character*,CharacterRelationshipsGraphNode*> nodeMap;
    for(int i=0; i<m_structure->characterCount(); i++)
    {
        Character *character = m_structure->characterAt(i);
        if(character->relationshipCount() == 0)
        {
            loneCharacters.append(character);
            continue;
        }

        CharacterRelationshipsGraphNode *node = new CharacterRelationshipsGraphNode(this);
        node->setCharacter(character);
        nodes.append(node);

        nodeMap[character] = node;

        for(int j=0; j<character->relationshipCount(); j++)
        {
            Relationship *relationship = character->relationshipAt(j);
            if(relationship->direction() != Relationship::OfWith)
                continue;

            if(relationship->of() != character || relationship->with() == nullptr || relationship->with() == character)
                continue;

            CharacterRelationshipsGraphEdge *edge = new CharacterRelationshipsGraphEdge(this);
            edge->setRelationship(relationship);
            edges.append(edge);

            if( relationship->name().length() > longestRelationshipName.length() )
                longestRelationshipName = relationship->name();
        }
    }

    // Initialize positions of all nodes so that we place them in a circle.
    const qreal angleStep = 2*M_PI / qreal(nodes.size());
    qreal angle = 0;
    for(CharacterRelationshipsGraphNode *node : nodes)
    {
        QRectF rect( QPointF(0,0), m_nodeSize );
        rect.moveCenter( QPointF(qCos(angle), qSin(angle)) );
        node->setRect(rect);
        angle += angleStep;
    }

    int nrIterations = 0;

    // https://en.wikipedia.org/wiki/Force-directed_graph_drawing
    QElapsedTimer timer;
    if(!nodes.isEmpty())
        timer.start();
    while(timer.elapsed() < m_maxTime && !nodes.isEmpty())
    {
        // Initialize Force Vector
        QVector<QPointF> forces(nodes.size(), QPointF(0,0));

        // Calculate Repulsion
        const qreal k = 0.001;
        for(int i=0; i<=nodes.size()-2; i++)
        {
            for(int j=i+1; j<=nodes.size()-1; j++)
            {
                const CharacterRelationshipsGraphNode *n1 = nodes.at(i);
                const CharacterRelationshipsGraphNode *n2 = nodes.at(j);
                const QPointF dp = n2->rect().center() - n1->rect().center();
                const qreal force = k / qSqrt((qPow(dp.x(), 2.0) + qPow(dp.y(), 2.0)));
                const qreal angle = qAtan2(dp.y(), dp.x());
                const QPointF delta( force*qCos(angle), force*qSin(angle) );
                forces[i] -= delta;
                forces[j] += delta;
            }
        }

        // Calculate Attraction
        for(CharacterRelationshipsGraphEdge *edge : edges)
        {
            CharacterRelationshipsGraphNode *n1 = nodeMap.value(edge->relationship()->of());
            CharacterRelationshipsGraphNode *n2 = nodeMap.value(edge->relationship()->with());;
            const QPointF dp = n2->rect().center() - n1->rect().center();
            const int i = nodes.indexOf(n1);
            const int j = nodes.indexOf(n2);
            const qreal force = k * (qPow(dp.x(), 2.0) + qPow(dp.y(), 2.0));
            const qreal angle = qAtan2(dp.y(), dp.x());
            const QPointF delta( force*qCos(angle), force*qSin(angle) );
            forces[i] += delta;
            forces[j] -= delta;
        }

        // Place nodes
        bool moved = false;
        for(int i=0; i<nodes.size(); i++)
        {
            CharacterRelationshipsGraphNode *node = nodes.at(i);

            const QPointF force = forces.at(i);
            if( qFuzzyIsNull(force.x()) && qFuzzyIsNull(force.y()) )
                continue;

            QRectF rect = node->rect();
            rect.moveCenter(rect.center() + force);
            node->setRect(rect);
            moved = true;
        }

        ++nrIterations;
        if(!moved || (m_maxIterations > 0 && nrIterations >= m_maxIterations))
            break;
    }

    // Scale the placement of nodes such that we consider the node sizes.
    qreal minNodeSpacing = MAXFLOAT;
    for(CharacterRelationshipsGraphNode *n1 : nodes)
    {
        for(CharacterRelationshipsGraphNode *n2 : nodes)
        {
            if(n1 == n2)
                continue;
            const qreal nodeSpacing = QLineF( n1->rect().center(), n2->rect().center() ).length();
            minNodeSpacing = qMin(nodeSpacing, minNodeSpacing);
        }
    }

    const QFontMetricsF fm(qApp->font());

    const qreal minNodeSpacingPx = QLineF( QPointF(0,0), QPointF(m_nodeSize.width(),m_nodeSize.height()) ).length() + fm.horizontalAdvance(longestRelationshipName);
    const qreal scale = minNodeSpacingPx / minNodeSpacing;

    QRectF boundingRect;
    for(CharacterRelationshipsGraphNode *node : nodes)
    {
        QRectF rect = node->rect();
        rect.moveCenter(rect.center()*scale);
        node->setRect(rect);

        boundingRect |= rect;
    }

    // Place lone characters along the left side of the bounding rect.
    QRectF rect( QPointF(0,0), m_nodeSize );
    rect.setX( boundingRect.left() - m_nodeSize.width() * 1.25 );
    rect.setY( boundingRect.center().y() - ((m_nodeSize.height() * loneCharacters.size()) + (loneCharacters.size()-1)*m_nodeSize.height()*0.5) );
    for(Character *character : loneCharacters)
    {
        CharacterRelationshipsGraphNode *node = new CharacterRelationshipsGraphNode(this);
        node->setCharacter(character);
        node->setRect(rect);
        m_nodes.append(node);

        nodeMap[character] = node;

        rect.setY( rect.y() + m_nodeSize.height()*1.5 );
    }

    // Move all the nodes so that the top-left is from 0,0
    boundingRect = QRectF();
    for(CharacterRelationshipsGraphNode *node : nodes)
        boundingRect |= node->rect();
    const qreal margin = qMin(qMax(boundingRect.width(),boundingRect.height())*0.1, 50.0);
    boundingRect.adjust(-margin, -margin, margin, margin);

    const QPointF dp = -boundingRect.topLeft();

    for(CharacterRelationshipsGraphNode *node : nodes)
    {
        QRectF rect = node->rect();
        rect.moveTopLeft( rect.topLeft() + dp );
        node->setRect(rect);
    }

    // Get all edges to evaluate their nodes.
    for(CharacterRelationshipsGraphEdge *edge : edges)
    {
        CharacterRelationshipsGraphNode *n1 = nodeMap.value(edge->relationship()->of());
        CharacterRelationshipsGraphNode *n2 = nodeMap.value(edge->relationship()->with());;

        QPainterPath path;
        path.moveTo( n1->rect().center() );
        path.lineTo( n2->rect().center() );
        edge->setPath(path);
    }

    // Update the nodes and edges properties
    m_nodes.assign(nodes);
    m_edges.assign(edges);

    // Announce changes to the bounding rectangle of the whole graph
    boundingRect.moveTopLeft( QPointF(0,0) );
    this->setGraphBoundingRect(boundingRect);

#ifndef QT_NO_DEBUG
    qDebug() << "PA: Layout Iterations: " << nrIterations;
#endif

    emit updated();
}
