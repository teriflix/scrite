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
#include <QtDebug>
#include <QQuickItem>
#include <QFontMetrics>
#include <QElapsedTimer>

CharacterRelationshipsGraphNode::CharacterRelationshipsGraphNode(QObject *parent)
    : QObject(parent),
      m_item(this, "item"),
      m_character(this, "character")
{

}

CharacterRelationshipsGraphNode::~CharacterRelationshipsGraphNode()
{

}

void CharacterRelationshipsGraphNode::setMarked(bool val)
{
    if(m_marked == val)
        return;

    m_marked = val;
    emit markedChanged();
}

void CharacterRelationshipsGraphNode::setItem(QQuickItem *val)
{
    if(m_item == val)
        return;

    if(!m_item.isNull())
    {
        disconnect(m_item, &QQuickItem::xChanged, this, &CharacterRelationshipsGraphNode::updateRectFromItemLater);
        disconnect(m_item, &QQuickItem::yChanged, this, &CharacterRelationshipsGraphNode::updateRectFromItemLater);
    }

    m_item = val;

    if(!m_item.isNull())
    {
        connect(m_item, &QQuickItem::xChanged, this, &CharacterRelationshipsGraphNode::updateRectFromItemLater);
        connect(m_item, &QQuickItem::yChanged, this, &CharacterRelationshipsGraphNode::updateRectFromItemLater);

        if(m_character->relationshipCount() > 0)
        {
            m_placedByUser = true;
            CharacterRelationshipsGraph *graph = qobject_cast<CharacterRelationshipsGraph*>(this->parent());
            if(graph)
                graph->updateGraphJsonFromNode(this);
        }
    }

    emit itemChanged();
}

void CharacterRelationshipsGraphNode::move(const QPointF &pos)
{
    QRectF rect = m_rect;
    rect.moveCenter(pos);
    this->setRect(rect);
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

void CharacterRelationshipsGraphNode::resetItem()
{
    m_item = nullptr;
    emit itemChanged();
}

void CharacterRelationshipsGraphNode::updateRectFromItem()
{
    if(m_item.isNull())
        return;

    const QRectF rect(m_item->x(), m_item->y(), m_item->width(), m_item->height());
    if(rect != m_rect)
    {
        this->setRect(rect);

        if(m_placedByUser)
        {
            CharacterRelationshipsGraph *graph = qobject_cast<CharacterRelationshipsGraph*>(this->parent());
            if(graph)
                graph->updateGraphJsonFromNode(this);
        }
    }
}

void CharacterRelationshipsGraphNode::updateRectFromItemLater()
{
    m_updateRectTimer.start(0, this);
}

void CharacterRelationshipsGraphNode::setRect(const QRectF &val)
{
    if(m_rect == val)
        return;

    m_rect = val;
    emit rectChanged();
}

void CharacterRelationshipsGraphNode::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == m_updateRectTimer.timerId())
    {
        m_updateRectTimer.stop();
        this->updateRectFromItem();
    }
    else
        QObject::timerEvent(te);
}

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipsGraphEdge::CharacterRelationshipsGraphEdge(CharacterRelationshipsGraphNode *from, CharacterRelationshipsGraphNode *to, QObject *parent)
    : QObject(parent),
      m_relationship(this, "relationship")
{
    m_fromNode = from;
    if(!m_fromNode.isNull())
        connect(m_fromNode, &CharacterRelationshipsGraphNode::rectChanged,
                this, &CharacterRelationshipsGraphEdge::evaluatePath);

    m_toNode = to;
    if(!m_toNode.isNull())
        connect(m_toNode, &CharacterRelationshipsGraphNode::rectChanged,
                this, &CharacterRelationshipsGraphEdge::evaluatePath);
}

CharacterRelationshipsGraphEdge::~CharacterRelationshipsGraphEdge()
{

}

QString CharacterRelationshipsGraphEdge::pathString() const
{
    return Application::instance()->painterPathToString(m_path);
}

void CharacterRelationshipsGraphEdge::setEvaluatePathAllowed(bool val)
{
    if(m_evaluatePathAllowed == val)
        return;

    m_evaluatePathAllowed = val;
    if(val)
        this->evaluatePath();
}

void CharacterRelationshipsGraphEdge::evaluatePath()
{
    if(!m_evaluatePathAllowed)
        return;

    QPainterPath path;

    if(!m_fromNode.isNull() && !m_toNode.isNull())
    {
#if 0
        path.moveTo( m_fromNode->rect().center() );
        path.lineTo( m_toNode->rect().center() );
#else
        auto pickIntersectionEdge = [](const QRectF &rect, const QLineF &line, int *type=nullptr) {
            if(!rect.contains(line.p1()) && !rect.contains(line.p2()))
                return QLineF();
            const QList<QLineF> edges = QList<QLineF>()
                    << QLineF( rect.topLeft(), rect.bottomLeft() )
                    << QLineF( rect.topLeft(), rect.topRight() )
                    << QLineF( rect.topRight(), rect.bottomRight() )
                    << QLineF( rect.bottomRight(), rect.bottomLeft() );
            const QList<int> types = QList<int>()
                    << Qt::LeftEdge << Qt::TopEdge << Qt::RightEdge << Qt::BottomEdge;
            for(int i=0; i<edges.size(); i++) {
                const QLineF edge = edges.at(i);
                if(line.intersect(edge, nullptr) == QLineF::BoundedIntersection) {
                    if(type)
                        *type = types[i];
                    return edge;
                }
            }
            return QLineF();
        };

        const QRectF r1 = m_fromNode->rect();
        const QRectF r2 = m_toNode->rect();
        const QLineF centerLine(r1.center(), r2.center());
        int edgeType1 = 0, edgeType2 = 0;
        const QLineF edge1 = pickIntersectionEdge(r1, centerLine, &edgeType1);
        const QLineF edge2 = pickIntersectionEdge(r2, centerLine, &edgeType2);
        const QPointF p1 = edge1.center();
        const QPointF p2 = edge2.center();
        const QLineF line(p1, p2);
        const QPointF cp = line.center();

        const QPointF cp1 = (edgeType1 == Qt::LeftEdge || edgeType1 == Qt::RightEdge) ? QPointF(cp.x(), p1.y()) : QPointF(p1.x(), cp.y());
        const QPointF cp2 = (edgeType2 == Qt::LeftEdge || edgeType2 == Qt::RightEdge) ? QPointF(cp.x(), p2.y()) : QPointF(p2.x(), cp.y());

        path.moveTo(p1);
        path.quadTo(cp1, cp);
        path.quadTo(cp2, p2);
#endif
    }

    this->setPath(path);
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
        m_labelAngle = 0; // qRadiansToDegrees( qAtan(m_path.slopeAtPercent(0.5)) );
    }

    emit pathChanged();
}

void CharacterRelationshipsGraphEdge::setForwardLabel(const QString &val)
{
    if(m_forwardLabel == val)
        return;

    m_forwardLabel = val;
    emit forwardLabelChanged();
}

void CharacterRelationshipsGraphEdge::setReverseLabel(const QString &val)
{
    if(m_reverseLabel == val)
        return;

    m_reverseLabel = val;
    emit reverseLabelChanged();
}

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipsGraph::CharacterRelationshipsGraph(QObject *parent)
    : QObject(parent),
      m_scene(this, "scene"),
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

    this->loadLater();
}

void CharacterRelationshipsGraph::setStructure(Structure *val)
{
    if(m_structure == val)
        return;

    m_structure = val;
    emit structureChanged();

    this->loadLater();
}

void CharacterRelationshipsGraph::setScene(Scene *val)
{
    if(m_scene == val)
        return;

    m_scene = val;
    emit sceneChanged();

    this->loadLater();
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

void CharacterRelationshipsGraph::setLeftMargin(qreal val)
{
    if( qFuzzyCompare(m_leftMargin, val) )
        return;

    m_leftMargin = val;
    emit leftMarginChanged();

    this->loadLater();
}

void CharacterRelationshipsGraph::setTopMargin(qreal val)
{
    if( qFuzzyCompare(m_topMargin, val) )
        return;

    m_topMargin = val;
    emit topMarginChanged();

    this->loadLater();
}

void CharacterRelationshipsGraph::setRightMargin(qreal val)
{
    if( qFuzzyCompare(m_rightMargin, val) )
        return;

    m_rightMargin = val;
    emit rightMarginChanged();

    this->loadLater();
}

void CharacterRelationshipsGraph::setBottomMargin(qreal val)
{
    if( qFuzzyCompare(m_bottomMargin, val) )
        return;

    m_bottomMargin = val;
    emit bottomMarginChanged();

    this->loadLater();
}

void CharacterRelationshipsGraph::reload()
{
    this->loadLater();
}

void CharacterRelationshipsGraph::reset()
{
    if(!m_structure.isNull())
    {
        if(m_scene.isNull())
            m_structure->setCharacterRelationshipGraph(QJsonObject());
        else
            m_scene->setCharacterRelationshipGraph(QJsonObject());
    }

    this->reload();
}

void CharacterRelationshipsGraph::updateGraphJsonFromNode(CharacterRelationshipsGraphNode *node)
{
    if(m_structure.isNull() || m_nodes.indexOf(node) < 0)
        return;

    const QRectF rect = node->rect();

    QJsonObject graphJson = m_scene.isNull() ? m_structure->characterRelationshipGraph() : m_scene->characterRelationshipGraph();
    QJsonObject rectJson;
    rectJson.insert("x", rect.x());
    rectJson.insert("y", rect.y());
    rectJson.insert("width", rect.width());
    rectJson.insert("height", rect.height());
    graphJson.insert(node->character()->name(), rectJson);
    if(m_scene.isNull())
        m_structure->setCharacterRelationshipGraph(graphJson);
    else
        m_scene->setCharacterRelationshipGraph(graphJson);
}

void CharacterRelationshipsGraph::classBegin()
{
    m_componentLoaded = false;
}

void CharacterRelationshipsGraph::componentComplete()
{
    m_componentLoaded = true;
    this->loadLater();
}

void CharacterRelationshipsGraph::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == m_loadTimer.timerId())
    {
        m_loadTimer.stop();
        this->load();
    }
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
    this->loadLater();
    emit structureChanged();
}

void CharacterRelationshipsGraph::resetScene()
{
    m_scene = nullptr;
    this->loadLater();
    emit sceneChanged();
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

    // If the graph is being requested for a specific scene, then we will have
    // to consider this character, only and only if it shows up in the scene
    // or is related to one of the characters in the scene.
    const QStringList sceneCharacterNames = m_scene.isNull() ? QStringList() : m_scene->characterNames();
    const QList<Character*> sceneCharacters = m_structure->findCharacters(sceneCharacterNames);

    // Lets fetch information about the graph as previously placed by the user.
    const QJsonObject previousGraphJson = m_scene.isNull() ?
                 m_structure->characterRelationshipGraph() :
                 m_scene->characterRelationshipGraph();

    QMap<Character*,CharacterRelationshipsGraphNode*> nodeMap;

    // Lets begin by create graph groups. Each group consists of nodes and edges
    // of characters related to each other.
    QList< GraphLayout::Graph > graphs;

    // The first graph consists of zombie characters. As in those characters who
    // dont have any relationship with anybody else in the screenplay.
    graphs.append( GraphLayout::Graph() );

    for(int i=0; i<m_structure->characterCount(); i++)
    {
        Character *character = m_structure->characterAt(i);

        // If the graph is being requested for a specific scene, then we will have
        // to consider this character, only and only if it shows up in the scene
        // or is related to one of the characters in the scene.
        if(!m_scene.isNull())
        {
            bool include = sceneCharacters.contains(character);
            if(!include)
            {
                for(Character *sceneCharacter : sceneCharacters)
                {
                    include = character->isRelatedTo(sceneCharacter);
                    if(include)
                        break;
                }
            }

            if(!include)
                continue;
        }

        CharacterRelationshipsGraphNode *node = new CharacterRelationshipsGraphNode(this);
        node->setCharacter(character);
        node->setRect( QRectF( QPointF(0,0), m_nodeSize) );
        if(!m_scene.isNull())
            node->setMarked( sceneCharacters.contains(node->character()) );
        nodes.append(node);
        nodeMap[character] = node;

        if(character->relationshipCount() == 0)
        {
            // This is a lone character with no relationship to other
            // characters in the screenplay. We will lay them out in one corner
            // of the graph at the end.
            QRectF nodeRect = node->rect();
            nodeRect.moveTop( graphs.first().nodes.size()*m_nodeSize.height()*1.5 );
            node->setRect(nodeRect);
            graphs.first().nodes.append(node);
            continue;
        }

        bool grouped = false;

        // This is a character which has a relationship. We group it into a graph/group
        // in which this character has relationships. Otherwise, we create a new group.
        for(int j=1; j<graphs.size(); j++)
        {
            GraphLayout::Graph &graph = graphs[j];

            for(GraphLayout::AbstractNode *agnode : graph.nodes)
            {
                CharacterRelationshipsGraphNode *gnode =
                        qobject_cast<CharacterRelationshipsGraphNode*>(agnode->containerObject());
                if(character->isRelatedTo(gnode->character()))
                {
                    graph.nodes.append(node);
                    grouped = true;
                    break;
                }
            }

            if(grouped)
                break;
        }

        if(grouped)
            continue;

        // Since we did not find a graph to which this node can belong, we are adding
        // this to a new graph all together.
        GraphLayout::Graph newGraph;
        newGraph.nodes.append(node);
        graphs.append(newGraph);
    }

    // Lets now loop over all nodes within each graph (except for the first one, which only
    // constains lone character nodes) and bundle relationships.
    for(int i=1; i<graphs.size(); i++)
    {
        GraphLayout::Graph &graph = graphs[i];

        for(GraphLayout::AbstractNode *agnode : graph.nodes)
        {
            CharacterRelationshipsGraphNode *node1 =
                    qobject_cast<CharacterRelationshipsGraphNode*>(agnode->containerObject());
            Character *character = node1->character();

            for(int j=0; j<character->relationshipCount(); j++)
            {
                Relationship *relationship = character->relationshipAt(j);
                if(relationship->direction() != Relationship::OfWith)
                    continue;

                Character *with = relationship->with();
                CharacterRelationshipsGraphNode *node2 = nodeMap.value(with);
                if(node2 == nullptr)
                    continue;

                CharacterRelationshipsGraphEdge *edge = new CharacterRelationshipsGraphEdge(node1, node2, this);
                edge->setRelationship(relationship);
                edge->setForwardLabel(relationship->name());
                const Relationship *reverseRelationship = with->findRelationship(character);
                if(reverseRelationship != nullptr)
                    edge->setReverseLabel(reverseRelationship->name());
                edges.append(edge);
                graph.edges.append(edge);
            }
        }
    }

    // Now lets layout all the graphs and arrange them in a row
    auto longerText = [](const QString &s1, const QString &s2) {
        return s1.length() > s2.length() ? s1 : s2;
    };
    QRectF boundingRect(m_leftMargin,m_topMargin,0,0);
    for(int i=0; i<graphs.size(); i++)
    {
        const GraphLayout::Graph &graph = graphs[i];
        if(graph.nodes.isEmpty())
            continue;

        if(i >= 1)
        {
            QString longestRelationshipName;
            for(GraphLayout::AbstractEdge *agedge : graph.edges)
            {
                CharacterRelationshipsGraphEdge *gedge =
                        qobject_cast<CharacterRelationshipsGraphEdge*>(agedge->containerObject());
                const QString relationshipName = gedge->relationship()->name();
                longestRelationshipName = longerText(gedge->forwardLabel(), longestRelationshipName);
                longestRelationshipName = longerText(gedge->reverseLabel(), longestRelationshipName);
            }

            const QFontMetricsF fm(qApp->font());

            GraphLayout::ForceDirectedLayout layout;
            layout.setMaxTime(m_maxTime);
            layout.setMaxIterations(m_maxIterations);
            layout.setMinimumEdgeLength(fm.horizontalAdvance(longestRelationshipName));
            layout.layout(graph);
        }

        // Compute bounding rect of the nodes.
        QRectF graphRect;
        for(GraphLayout::AbstractNode *agnode : graph.nodes)
        {
            CharacterRelationshipsGraphNode *gnode =
                    qobject_cast<CharacterRelationshipsGraphNode*>(agnode->containerObject());

            const Character *character = gnode->character();

            const QJsonValue rectJsonValue = previousGraphJson.value(character->name());
            if(!rectJsonValue.isUndefined() && rectJsonValue.isObject())
            {
                const QJsonObject rectJson = rectJsonValue.toObject();
                const QRectF rect( rectJson.value("x").toDouble(),
                                   rectJson.value("y").toDouble(),
                                   rectJson.value("width").toDouble(),
                                   rectJson.value("height").toDouble() );
                if(rect.isValid())
                {
                    gnode->setRect(rect);
                    gnode->m_placedByUser = true;
                }
            }

            graphRect |= gnode->rect();
        }

        // Move the nodes such that they are layed out in a row.
        const QPointF dp = -graphRect.topLeft() + boundingRect.topRight();
        for(GraphLayout::AbstractNode *agnode : graph.nodes)
        {
            CharacterRelationshipsGraphNode *gnode =
                    qobject_cast<CharacterRelationshipsGraphNode*>(agnode->containerObject());

            QRectF rect = gnode->rect();
            rect.moveTopLeft( rect.topLeft() + dp );
            gnode->setRect(rect);
        }
        graphRect.moveTopLeft( graphRect.topLeft() + dp );

        boundingRect |= graphRect;
        if(i < graphs.size()-1)
            boundingRect.setRight( boundingRect.right() + 100 );
    }

    for(CharacterRelationshipsGraphEdge *edge : edges)
        edge->setEvaluatePathAllowed(true);

    // Update the models and bounding rectangle
    m_nodes.assign(nodes);
    m_edges.assign(edges);

    boundingRect.setRight( boundingRect.right() + m_rightMargin );
    boundingRect.setBottom( boundingRect.bottom() + m_bottomMargin );
    this->setGraphBoundingRect(boundingRect);

    emit updated();
}

void CharacterRelationshipsGraph::loadLater()
{
    m_loadTimer.start(0, this);
}

