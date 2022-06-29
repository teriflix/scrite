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

#include "characterrelationshipgraph.h"
#include "characterrelationshipsgraphexporter.h"

#include "hourglass.h"
#include "structure.h"
#include "screenplay.h"
#include "application.h"
#include "scritedocument.h"

#include <QtMath>
#include <QtDebug>
#include <QFuture>
#include <QDateTime>
#include <QPdfWriter>
#include <QQuickItem>
#include <QFontMetrics>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QStandardPaths>
#include <QtConcurrentRun>

CharacterRelationshipGraphNode::CharacterRelationshipGraphNode(QObject *parent)
    : QObject(parent), m_item(this, "item"), m_character(this, "character")
{
}

CharacterRelationshipGraphNode::~CharacterRelationshipGraphNode() { }

void CharacterRelationshipGraphNode::setMarked(bool val)
{
    if (m_marked == val)
        return;

    m_marked = val;
    emit markedChanged();
}

void CharacterRelationshipGraphNode::setItem(QQuickItem *val)
{
    if (m_item == val)
        return;

    if (!m_item.isNull()) {
        disconnect(m_item, &QQuickItem::xChanged, this,
                   &CharacterRelationshipGraphNode::updateRectFromItemLater);
        disconnect(m_item, &QQuickItem::yChanged, this,
                   &CharacterRelationshipGraphNode::updateRectFromItemLater);
    }

    m_item = val;

    if (!m_item.isNull()) {
        connect(m_item, &QQuickItem::xChanged, this,
                &CharacterRelationshipGraphNode::updateRectFromItemLater);
        connect(m_item, &QQuickItem::yChanged, this,
                &CharacterRelationshipGraphNode::updateRectFromItemLater);

        m_placedByUser = true;
        CharacterRelationshipGraph *graph =
                qobject_cast<CharacterRelationshipGraph *>(this->parent());
        if (graph)
            graph->updateGraphJsonFromNode(this);
    }

    emit itemChanged();
}

void CharacterRelationshipGraphNode::move(const QPointF &pos)
{
    QRectF rect = m_rect;
    rect.moveCenter(pos);
    this->setRect(rect);
}

void CharacterRelationshipGraphNode::setCharacter(Character *val)
{
    if (m_character == val)
        return;

    m_character = val;
    emit characterChanged();
}

void CharacterRelationshipGraphNode::resetCharacter()
{
    m_character = nullptr;
    emit characterChanged();

    m_rect = QRectF();
    emit rectChanged();
}

void CharacterRelationshipGraphNode::resetItem()
{
    m_item = nullptr;
    emit itemChanged();
}

void CharacterRelationshipGraphNode::updateRectFromItem()
{
    if (m_item.isNull())
        return;

    const QRectF rect(m_item->x(), m_item->y(), m_item->width(), m_item->height());
    if (rect != m_rect) {
        this->setRect(rect);

        if (m_placedByUser) {
            CharacterRelationshipGraph *graph =
                    qobject_cast<CharacterRelationshipGraph *>(this->parent());
            if (graph)
                graph->updateGraphJsonFromNode(this);
        }
    }
}

void CharacterRelationshipGraphNode::updateRectFromItemLater()
{
    m_updateRectTimer.start(0, this);
}

void CharacterRelationshipGraphNode::setRect(const QRectF &val)
{
    if (m_rect == val)
        return;

    m_rect = val;
    emit rectChanged();
}

void CharacterRelationshipGraphNode::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_updateRectTimer.timerId()) {
        m_updateRectTimer.stop();
        this->updateRectFromItem();
    } else
        QObject::timerEvent(te);
}

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipGraphEdge::CharacterRelationshipGraphEdge(CharacterRelationshipGraphNode *from,
                                                               CharacterRelationshipGraphNode *to,
                                                               QObject *parent)
    : QObject(parent), m_relationship(this, "relationship")
{
    m_fromNode = from;
    if (!m_fromNode.isNull())
        connect(m_fromNode, &CharacterRelationshipGraphNode::rectChanged, this,
                &CharacterRelationshipGraphEdge::evaluatePath);

    m_toNode = to;
    if (!m_toNode.isNull())
        connect(m_toNode, &CharacterRelationshipGraphNode::rectChanged, this,
                &CharacterRelationshipGraphEdge::evaluatePath);
}

CharacterRelationshipGraphEdge::~CharacterRelationshipGraphEdge() { }

QString CharacterRelationshipGraphEdge::pathString() const
{
    return Application::instance()->painterPathToString(m_path);
}

void CharacterRelationshipGraphEdge::setEvaluatePathAllowed(bool val)
{
    if (m_evaluatePathAllowed == val)
        return;

    m_evaluatePathAllowed = val;
    if (val)
        this->evaluatePath();
}

void CharacterRelationshipGraphEdge::evaluatePath()
{
    if (!m_evaluatePathAllowed)
        return;

    if (!m_fromNode.isNull() && !m_toNode.isNull() && !m_relationship.isNull()) {
        const QRectF r1 = m_fromNode->rect();
        const QRectF r2 = m_toNode->rect();
        const QRectF box1 = m_relationship->direction() == Relationship::WithOf ? r2 : r1;
        const QRectF box2 = m_relationship->direction() == Relationship::WithOf ? r1 : r2;

        const QString futureName = QStringLiteral("curvedArrowFuture");
        QFutureWatcher<QPainterPath> *futureWatcher =
                this->findChild<QFutureWatcher<QPainterPath> *>(futureName,
                                                                Qt::FindDirectChildrenOnly);
        if (futureWatcher) {
            futureWatcher->cancel();
            futureWatcher->deleteLater();
        }

        futureWatcher = new QFutureWatcher<QPainterPath>(this);
        futureWatcher->setObjectName(futureName);
        connect(futureWatcher, &QFutureWatcher<QPainterPath>::finished, this, [=]() {
            if (futureWatcher->isCanceled())
                return;

            const QPainterPath path = futureWatcher->result();
            this->setPath(path);
            futureWatcher->deleteLater();
        });
        futureWatcher->setFuture(QtConcurrent::run(&StructureElementConnector::curvedArrowPath,
                                                   box1, box2, 5, false));
    }
}

void CharacterRelationshipGraphEdge::setRelationship(Relationship *val)
{
    if (m_relationship == val)
        return;

    m_relationship = val;
    emit relationshipChanged();
}

void CharacterRelationshipGraphEdge::resetRelationship()
{
    m_relationship = nullptr;
    emit relationshipChanged();

    m_path = QPainterPath();
    emit pathChanged();
}

void CharacterRelationshipGraphEdge::setPath(const QPainterPath &val)
{
    if (m_path == val)
        return;

    m_path = val;

    if (m_path.isEmpty()) {
        m_labelPos = QPointF(0, 0);
        m_labelAngle = 0;
    } else {
        m_labelPos = m_path.pointAtPercent(0.5);
        m_labelAngle = 0; // qRadiansToDegrees( qAtan(m_path.slopeAtPercent(0.5)) );
    }

    emit pathChanged();
}

void CharacterRelationshipGraphEdge::setForwardLabel(const QString &val)
{
    if (m_forwardLabel == val)
        return;

    m_forwardLabel = val;
    emit forwardLabelChanged();
}

void CharacterRelationshipGraphEdge::setReverseLabel(const QString &val)
{
    if (m_reverseLabel == val)
        return;

    m_reverseLabel = val;
    emit reverseLabelChanged();
}

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipGraph::CharacterRelationshipGraph(QObject *parent)
    : QObject(parent),
      m_scene(this, "scene"),
      m_character(this, "character"),
      m_structure(this, "structure")
{
    connect(this, &CharacterRelationshipGraph::structureChanged, this,
            &CharacterRelationshipGraph::evaluateTitle);
    connect(this, &CharacterRelationshipGraph::sceneChanged, this,
            &CharacterRelationshipGraph::evaluateTitle);
    connect(this, &CharacterRelationshipGraph::characterChanged, this,
            &CharacterRelationshipGraph::evaluateTitle);
    connect(this, &CharacterRelationshipGraph::updated, this,
            &CharacterRelationshipGraph::emptyChanged);
}

CharacterRelationshipGraph::~CharacterRelationshipGraph()
{
    if (m_loadTimer.isActive())
        this->load();
}

bool CharacterRelationshipGraph::isEmpty() const
{
    return m_nodes.isEmpty();
}

void CharacterRelationshipGraph::setNodeSize(const QSizeF &val)
{
    if (m_nodeSize == val)
        return;

    m_nodeSize = val;
    emit nodeSizeChanged();

    this->loadLater();
}

void CharacterRelationshipGraph::setStructure(Structure *val)
{
    if (m_structure == val)
        return;

    if (!m_structure.isNull())
        disconnect(m_structure, &Structure::characterCountChanged, this,
                   &CharacterRelationshipGraph::markDirty);

    m_structure = val;
    emit structureChanged();

    if (!m_structure.isNull())
        connect(m_structure, &Structure::characterCountChanged, this,
                &CharacterRelationshipGraph::markDirty);

    this->loadLater();
}

void CharacterRelationshipGraph::setScene(Scene *val)
{
    if (m_scene == val)
        return;

    if (!m_scene.isNull())
        disconnect(m_scene, &Scene::characterNamesChanged, this,
                   &CharacterRelationshipGraph::markDirty);

    m_scene = val;
    emit sceneChanged();

    if (!m_scene.isNull())
        connect(m_scene, &Scene::characterNamesChanged, this,
                &CharacterRelationshipGraph::markDirty);

    this->loadLater();
}

void CharacterRelationshipGraph::setCharacter(Character *val)
{
    if (m_character == val)
        return;

    if (!m_character.isNull()) {
        disconnect(m_character, &Character::relationshipCountChanged, this,
                   &CharacterRelationshipGraph::loadLater);
        disconnect(m_character, &Character::nameChanged, this, &CharacterRelationshipGraph::reset);
    }

    m_character = val;
    emit characterChanged();

    if (!m_character.isNull()) {
        connect(m_character, &Character::relationshipCountChanged, this,
                &CharacterRelationshipGraph::loadLater);
        connect(m_character, &Character::nameChanged, this, &CharacterRelationshipGraph::reset);
    }

    this->loadLater();
}

void CharacterRelationshipGraph::setMaxTime(int val)
{
    const int val2 = qBound(10, val, 5000);
    if (m_maxTime == val2)
        return;

    m_maxTime = val2;
    emit maxTimeChanged();
}

void CharacterRelationshipGraph::setMaxIterations(int val)
{
    if (m_maxIterations == val)
        return;

    m_maxIterations = val;
    emit maxIterationsChanged();
}

void CharacterRelationshipGraph::setLeftMargin(qreal val)
{
    if (qFuzzyCompare(m_leftMargin, val))
        return;

    m_leftMargin = val;
    emit leftMarginChanged();

    this->loadLater();
}

void CharacterRelationshipGraph::setTopMargin(qreal val)
{
    if (qFuzzyCompare(m_topMargin, val))
        return;

    m_topMargin = val;
    emit topMarginChanged();

    this->loadLater();
}

void CharacterRelationshipGraph::setRightMargin(qreal val)
{
    if (qFuzzyCompare(m_rightMargin, val))
        return;

    m_rightMargin = val;
    emit rightMarginChanged();

    this->loadLater();
}

void CharacterRelationshipGraph::setBottomMargin(qreal val)
{
    if (qFuzzyCompare(m_bottomMargin, val))
        return;

    m_bottomMargin = val;
    emit bottomMarginChanged();

    this->loadLater();
}

void CharacterRelationshipGraph::reload()
{
    this->loadLater();
}

void CharacterRelationshipGraph::reset()
{
    QObject *gjObject = this->graphJsonObject();
    if (gjObject != nullptr)
        gjObject->setProperty("characterRelationshipGraph",
                              QVariant::fromValue<QJsonObject>(QJsonObject()));

    this->reload();
}

CharacterRelationshipsGraphExporter *CharacterRelationshipGraph::createExporter()
{
    CharacterRelationshipsGraphExporter *exporter = new CharacterRelationshipsGraphExporter(this);

    ScriteDocument *document = ScriteDocument::instance();
    document->setupExporter(exporter);
    exporter->setGraph(this);

    return exporter;
}

QObject *CharacterRelationshipGraph::createExporterObject()
{
    return this->createExporter();
}

QObject *CharacterRelationshipGraph::graphJsonObject() const
{
    if (m_structure.isNull())
        return nullptr;

    if (m_scene.isNull() && m_character.isNull())
        return m_structure;

    if (m_character.isNull())
        return m_scene;

    return m_character;
}

void CharacterRelationshipGraph::updateGraphJsonFromNode(CharacterRelationshipGraphNode *node)
{
    if (m_structure.isNull() || m_nodes.indexOf(node) < 0)
        return;

    const QRectF rect = node->rect();

    QJsonObject graphJson =
            this->graphJsonObject()->property("characterRelationshipGraph").value<QJsonObject>();
    QJsonObject rectJson;
    rectJson.insert("x", rect.x());
    rectJson.insert("y", rect.y());
    rectJson.insert("width", rect.width());
    rectJson.insert("height", rect.height());
    graphJson.insert(node->character()->name(), rectJson);
    this->graphJsonObject()->setProperty("characterRelationshipGraph",
                                         QVariant::fromValue<QJsonObject>(graphJson));
}

void CharacterRelationshipGraph::classBegin()
{
    m_componentLoaded = false;
}

void CharacterRelationshipGraph::componentComplete()
{
    m_componentLoaded = true;
    this->loadLater();
}

void CharacterRelationshipGraph::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_loadTimer.timerId()) {
        m_loadTimer.stop();
        this->load();
    }
}

void CharacterRelationshipGraph::setGraphBoundingRect(const QRectF &val)
{
    if (m_graphBoundingRect == val)
        return;

    m_graphBoundingRect = val;
    if (m_graphBoundingRect.isEmpty())
        m_graphBoundingRect.setSize(QSizeF(500, 500));
    emit graphBoundingRectChanged();
}

void CharacterRelationshipGraph::resetStructure()
{
    m_structure = nullptr;
    this->loadLater();
    emit structureChanged();
}

void CharacterRelationshipGraph::resetScene()
{
    m_scene = nullptr;
    this->loadLater();
    emit sceneChanged();
}

void CharacterRelationshipGraph::resetCharacter()
{
    m_character = nullptr;
    this->loadLater();
    emit characterChanged();
}

void CharacterRelationshipGraph::load()
{
    HourGlass hourGlass;
    this->setBusy(true);

    QList<CharacterRelationshipGraphEdge *> edges = m_edges.list();
    m_edges.clear();
    for (CharacterRelationshipGraphEdge *edge : qAsConst(edges)) {
        disconnect(edge->relationship(), &Relationship::aboutToDelete, this,
                   &CharacterRelationshipGraph::loadLater);
        GarbageCollector::instance()->add(edge);
    }
    edges.clear();

    QList<CharacterRelationshipGraphNode *> nodes = m_nodes.list();
    m_nodes.clear();
    for (CharacterRelationshipGraphNode *node : qAsConst(nodes)) {
        disconnect(node->character(), &Character::aboutToDelete, this,
                   &CharacterRelationshipGraph::loadLater);
        GarbageCollector::instance()->add(node);
    }
    nodes.clear();

    if (m_structure.isNull() || !m_componentLoaded) {
        this->setBusy(false);
        this->setDirty(false);
        this->setGraphBoundingRect(QRectF(0, 0, 0, 0));
        emit updated();
        return;
    }

    // If the graph is being requested for a specific scene, then we will have
    // to consider this character, only and only if it shows up in the scene
    // or is related to one of the characters in the scene.
    const QStringList sceneCharacterNames = m_character.isNull()
            ? (m_scene.isNull() ? QStringList() : m_scene->characterNames())
            : QStringList() << m_character->name();
    const QList<Character *> sceneCharacters = m_structure->findCharacters(sceneCharacterNames);

    // Lets fetch information about the graph as previously placed by the user.
    const QJsonObject previousGraphJson =
            this->graphJsonObject()->property("characterRelationshipGraph").value<QJsonObject>();

    QHash<Character *, CharacterRelationshipGraphNode *> nodeMap;

    // Lets begin by create graph groups. Each group consists of nodes and edges
    // of characters related to each other.
    QList<GraphLayout::Graph> graphs;

    // The first graph consists of zombie characters. As in those characters who
    // dont have any relationship with anybody else in the screenplay.
    graphs.append(GraphLayout::Graph());

    const QList<Character *> characters = m_structure->charactersModel()->list();
    for (Character *character : characters) {
        // If the graph is being requested for a specific scene, then we will have
        // to consider this character, only and only if it shows up in the scene
        // or is related to one of the characters in the scene.
        if (!m_scene.isNull()) {
            bool include = sceneCharacters.contains(character);
            if (!include) {
                for (Character *sceneCharacter : sceneCharacters) {
                    include = character->isRelatedTo(sceneCharacter);
                    if (include)
                        break;
                }
            }

            if (!include)
                continue;
        }

        // If graph is being requested for a specific character, then we have to
        // consider only those characters with whom it has a direct relationship.
        if (!m_character.isNull()) {
            const bool include = (m_character == character
                                  || m_character->findRelationship(character) != nullptr);
            if (!include)
                continue;
        }

        CharacterRelationshipGraphNode *node = new CharacterRelationshipGraphNode(this);
        node->setCharacter(character);
        node->setRect(QRectF(QPointF(0, 0), m_nodeSize));
        if (!m_scene.isNull())
            node->setMarked(sceneCharacters.contains(node->character()));
        nodes.append(node);
        nodeMap[character] = node;

        if (character->relationshipCount() == 0) {
            graphs.first().nodes.append(node);
            continue;
        }

        bool grouped = false;

        // This is a character which has a relationship. We group it into a graph/group
        // in which this character has relationships. Otherwise, we create a new group.
        for (GraphLayout::Graph &graph : graphs) {
            for (GraphLayout::AbstractNode *agnode : qAsConst(graph.nodes)) {
                CharacterRelationshipGraphNode *gnode =
                        qobject_cast<CharacterRelationshipGraphNode *>(agnode->containerObject());
                if (character->isRelatedTo(gnode->character())) {
                    graph.nodes.append(node);
                    grouped = true;
                    break;
                }
            }

            if (grouped)
                break;
        }

        if (grouped)
            continue;

        // Since we did not find a graph to which this node can belong, we are adding
        // this to a new graph all together.
        GraphLayout::Graph newGraph;
        newGraph.nodes.append(node);
        graphs.append(newGraph);
    }

    // Layout the first graph in the form of a regular grid.
    auto layoutNodesInAGrid = [](const GraphLayout::Graph &graph) {
        const int nrNodes = graph.nodes.size();
        const int nrCols = qFloor(qSqrt(qreal(nrNodes)));

        int col = 0;
        QPointF pos;
        for (GraphLayout::AbstractNode *agnode : qAsConst(graph.nodes)) {
            CharacterRelationshipGraphNode *node =
                    qobject_cast<CharacterRelationshipGraphNode *>(agnode->containerObject());
            node->move(pos);
            ++col;
            if (col < nrCols)
                pos.setX(pos.x() + node->size().width() * 1.5);
            else {
                pos.setX(0);
                pos.setY(pos.y() + node->size().height() * 1.5);
                col = 0;
            }
        }
    };
    layoutNodesInAGrid(graphs[0]);

    // Lets now loop over all nodes within each graph (except for the first one, which only
    // constains lone character nodes) and bundle relationships.
    for (GraphLayout::Graph &graph : graphs) {
        for (GraphLayout::AbstractNode *agnode : qAsConst(graph.nodes)) {
            CharacterRelationshipGraphNode *node1 =
                    qobject_cast<CharacterRelationshipGraphNode *>(agnode->containerObject());
            Character *character = node1->character();

            const QList<Relationship *> relationships = character->relationshipsModel()->list();
            for (Relationship *relationship : relationships) {
                if (relationship->direction() != Relationship::OfWith)
                    continue;

                Character *with = relationship->with();
                CharacterRelationshipGraphNode *node2 = nodeMap.value(with);
                if (node2 == nullptr)
                    continue;

                CharacterRelationshipGraphEdge *edge =
                        new CharacterRelationshipGraphEdge(node1, node2, this);
                edge->setRelationship(relationship);
                edge->setForwardLabel(relationship->name());
                const Relationship *reverseRelationship = with->findRelationship(character);
                if (reverseRelationship != nullptr)
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
    QRectF boundingRect(m_leftMargin, m_topMargin, 0, 0);
    int graphIndex = 0;
    for (const GraphLayout::Graph &graph : qAsConst(graphs)) {
        const int i = graphIndex++;
        if (graph.nodes.isEmpty())
            continue;

        if (i >= 1) {
            QString longestRelationshipName;
            for (GraphLayout::AbstractEdge *agedge : qAsConst(graph.edges)) {
                CharacterRelationshipGraphEdge *gedge =
                        qobject_cast<CharacterRelationshipGraphEdge *>(agedge->containerObject());
                longestRelationshipName =
                        longerText(gedge->forwardLabel(), longestRelationshipName);
                longestRelationshipName =
                        longerText(gedge->reverseLabel(), longestRelationshipName);
            }

            const QFontMetricsF fm(qApp->font());

            GraphLayout::ForceDirectedLayout layout;
            layout.setMaxTime(m_maxTime);
            layout.setMaxIterations(m_maxIterations);
            layout.setMinimumEdgeLength(fm.horizontalAdvance(longestRelationshipName) * 0.5);
            layout.layout(graph);
        }

        // Compute bounding rect of the nodes.
        QRectF graphRect;
        for (GraphLayout::AbstractNode *agnode : qAsConst(graph.nodes)) {
            CharacterRelationshipGraphNode *gnode =
                    qobject_cast<CharacterRelationshipGraphNode *>(agnode->containerObject());

            const Character *character = gnode->character();

            const QJsonValue rectJsonValue = previousGraphJson.value(character->name());
            if (!rectJsonValue.isUndefined() && rectJsonValue.isObject()) {
                const QJsonObject rectJson = rectJsonValue.toObject();
                const QRectF rect(rectJson.value("x").toDouble(), rectJson.value("y").toDouble(),
                                  rectJson.value("width").toDouble(),
                                  rectJson.value("height").toDouble());
                if (rect.isValid()) {
                    gnode->setRect(rect);
                    gnode->m_placedByUser = true;
                }
            }

            graphRect |= gnode->rect();
        }

        // Move the nodes such that they are layed out in a row.
        const QPointF dp = -graphRect.topLeft() + boundingRect.topRight();
        for (GraphLayout::AbstractNode *agnode : qAsConst(graph.nodes)) {
            CharacterRelationshipGraphNode *gnode =
                    qobject_cast<CharacterRelationshipGraphNode *>(agnode->containerObject());
            if (gnode->m_placedByUser)
                continue;

            QRectF rect = gnode->rect();
            rect.moveTopLeft(rect.topLeft() + dp);
            gnode->setRect(rect);
        }
        graphRect.moveTopLeft(graphRect.topLeft() + dp);

        boundingRect |= graphRect;
        if (i < graphs.size() - 1)
            boundingRect.setRight(boundingRect.right() + 100);
    }

    for (CharacterRelationshipGraphNode *node : qAsConst(nodes))
        connect(node->character(), &Character::aboutToDelete, this,
                &CharacterRelationshipGraph::loadLater, Qt::UniqueConnection);

    for (CharacterRelationshipGraphEdge *edge : qAsConst(edges)) {
        connect(edge->relationship(), &Relationship::aboutToDelete, this,
                &CharacterRelationshipGraph::loadLater, Qt::UniqueConnection);
        edge->setEvaluatePathAllowed(true);
    }

    // Update the models and bounding rectangle
    m_nodes.assign(nodes);
    m_edges.assign(edges);

    boundingRect.setRight(boundingRect.right() + m_rightMargin);
    boundingRect.setBottom(boundingRect.bottom() + m_bottomMargin);
    this->setGraphBoundingRect(boundingRect);

    this->setBusy(false);
    this->setDirty(false);

    emit updated();
}

void CharacterRelationshipGraph::loadLater()
{
    m_loadTimer.start(0, this);
}

void CharacterRelationshipGraph::evaluateTitle()
{
    const QString defaultTitle = QStringLiteral("Character Relationship Graph");

    Screenplay *screenplay = ScriteDocument::instance()->screenplay();

    if (m_character != nullptr)
        m_title = QStringLiteral("Relationships Of \"") + m_character->name()
                + QStringLiteral("\" in \"") + screenplay->title() + QStringLiteral("\"");
    else if (m_scene != nullptr)
        m_title = QStringLiteral("Characters In A Scene Of \"") + screenplay->title()
                + QStringLiteral("\"");
    else if (m_structure != nullptr)
        m_title =
                QStringLiteral("All Characters Of \"") + screenplay->title() + QStringLiteral("\"");
    else
        m_title = defaultTitle;

    emit titleChanged();
}

void CharacterRelationshipGraph::setDirty(bool val)
{
    if (m_dirty == val)
        return;

    m_dirty = val;
    emit dirtyChanged();
}

void CharacterRelationshipGraph::setBusy(bool val)
{
    if (m_busy == val)
        return;

    m_busy = val;
    emit busyChanged();
}
