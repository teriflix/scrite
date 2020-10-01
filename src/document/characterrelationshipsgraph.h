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

#ifndef CHARACTERRELATIONSHIPSGRAPH_H
#define CHARACTERRELATIONSHIPSGRAPH_H

#include <QObject>

#include "structure.h"
#include "qobjectproperty.h"
#include "objectlistpropertymodel.h"

class CharacterRelationshipsGraph;

class CharacterRelationshipsGraphNode : public QObject
{
    Q_OBJECT

public:
    ~CharacterRelationshipsGraphNode();

    Q_PROPERTY(Character* character READ character NOTIFY characterChanged RESET resetCharacter)
    Character* character() const { return m_character; }
    Q_SIGNAL void characterChanged();

    Q_PROPERTY(QRectF rect READ rect NOTIFY rectChanged)
    QRectF rect() const { return m_rect; }
    Q_SIGNAL void rectChanged();

    Q_PROPERTY(QQuickItem* item READ item WRITE setItem NOTIFY itemChanged RESET resetItem)
    void setItem(QQuickItem* val);
    QQuickItem* item() const { return m_item; }
    Q_SIGNAL void itemChanged();

    bool isPlaced() const { return !m_rect.isNull(); }

protected:
    friend class CharacterRelationshipsGraph;
    CharacterRelationshipsGraphNode(QObject *parent=nullptr);
    void setCharacter(Character* val);
    void resetCharacter();
    void resetItem();
    void updateRectFromItem();
    void updateRectFromItemLater();
    void setRect(const QRectF &val);

    void timerEvent(QTimerEvent *te);

private:
    QRectF m_rect;
    ExecLaterTimer m_updateRectTimer;
    QObjectProperty<QQuickItem> m_item;
    QObjectProperty<Character> m_character;
};

class CharacterRelationshipsGraphEdge : public QObject
{
    Q_OBJECT

public:
    ~CharacterRelationshipsGraphEdge();

    Q_PROPERTY(Relationship* relationship READ relationship NOTIFY relationshipChanged RESET resetRelationship)
    Relationship* relationship() const { return m_relationship; }
    Q_SIGNAL void relationshipChanged();

    Q_PROPERTY(QPainterPath path READ path NOTIFY pathChanged)
    QPainterPath path() const { return m_path; }
    Q_SIGNAL void pathChanged();

    Q_PROPERTY(QString pathString READ pathString NOTIFY pathChanged)
    QString pathString() const;

    Q_PROPERTY(QPointF labelPosition READ labelPosition NOTIFY pathChanged)
    QPointF labelPosition() const { return m_labelPos; }

    Q_PROPERTY(qreal labelAngle READ labelAngle NOTIFY pathChanged)
    qreal labelAngle() const { return m_labelAngle; }

    void evaluatePath(CharacterRelationshipsGraphNode *from, CharacterRelationshipsGraphNode *to);

protected:
    friend class CharacterRelationshipsGraph;
    CharacterRelationshipsGraphEdge(QObject *parent=nullptr);
    void setRelationship(Relationship* val);
    void resetRelationship();
    void reevaluatePath();
    void setPath(const QPainterPath &val);

private:
    QPointF m_labelPos;
    qreal m_labelAngle = 0;
    QPainterPath m_path;
    QObjectProperty<Relationship> m_relationship;
    QPointer<CharacterRelationshipsGraphNode> m_toNode;
    QPointer<CharacterRelationshipsGraphNode> m_fromNode;
};

class CharacterRelationshipsGraph : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    CharacterRelationshipsGraph(QObject *parent = nullptr);
    ~CharacterRelationshipsGraph();

    Q_PROPERTY(QAbstractListModel* nodes READ nodes CONSTANT)
    QAbstractListModel *nodes() const {
        return &((const_cast<CharacterRelationshipsGraph*>(this))->m_nodes);
    }

    Q_PROPERTY(QAbstractListModel* edges READ edges CONSTANT)
    QAbstractListModel *edges() const {
        return &(const_cast<CharacterRelationshipsGraph*>(this))->m_edges;
    }

    Q_PROPERTY(QSizeF nodeSize READ nodeSize WRITE setNodeSize NOTIFY nodeSizeChanged)
    void setNodeSize(const QSizeF &val);
    QSizeF nodeSize() const { return m_nodeSize; }
    Q_SIGNAL void nodeSizeChanged();

    Q_PROPERTY(Structure* structure READ structure WRITE setStructure NOTIFY structureChanged RESET resetStructure)
    void setStructure(Structure* val);
    Structure* structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(Scene* scene READ scene WRITE setScene NOTIFY sceneChanged RESET resetScene)
    void setScene(Scene* val);
    Scene* scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(int maxTime READ maxTime WRITE setMaxTime NOTIFY maxTimeChanged)
    void setMaxTime(int val);
    int maxTime() const { return m_maxTime; }
    Q_SIGNAL void maxTimeChanged();

    Q_PROPERTY(int maxIterations READ maxIterations WRITE setMaxIterations NOTIFY maxIterationsChanged)
    void setMaxIterations(int val);
    int maxIterations() const { return m_maxIterations; }
    Q_SIGNAL void maxIterationsChanged();

    Q_PROPERTY(QRectF graphBoundingRect READ graphBoundingRect NOTIFY graphBoundingRectChanged)
    QRectF graphBoundingRect() const { return m_graphBoundingRect; }
    Q_SIGNAL void graphBoundingRectChanged();

    Q_INVOKABLE void reload();

    Q_SIGNAL void updated();

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

private:
    void setGraphBoundingRect(const QRectF &val);
    void resetStructure();
    void resetScene();
    void load();

private:
    int m_maxTime = 100;
    QSizeF m_nodeSize = QSizeF(100,100);
    int m_maxIterations = -1;
    QObjectProperty<Scene> m_scene;
    bool m_componentLoaded = false;
    QRectF m_graphBoundingRect = QRectF(0,0,500,500);
    QObjectProperty<Structure> m_structure;
    ObjectListPropertyModel<CharacterRelationshipsGraphNode*> m_nodes;
    ObjectListPropertyModel<CharacterRelationshipsGraphEdge*> m_edges;
};

#endif // CHARACTERRELATIONSHIPSGRAPH_H
