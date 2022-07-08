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

#ifndef CHARACTERRELATIONSHIPGRAPH_H
#define CHARACTERRELATIONSHIPGRAPH_H

#include <QQmlEngine>

#include "structure.h"
#include "graphlayout.h"
#include "errorreport.h"
#include "qobjectproperty.h"
#include "qobjectlistmodel.h"

class CharacterRelationshipGraph;
class CharacterRelationshipsGraphExporter;

class CharacterRelationshipGraphNode : public QObject, public GraphLayout::AbstractNode
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~CharacterRelationshipGraphNode();

    Q_PROPERTY(Character *character READ character NOTIFY characterChanged RESET resetCharacter)
    Character *character() const { return m_character; }
    Q_SIGNAL void characterChanged();

    Q_PROPERTY(bool marked READ isMarked WRITE setMarked NOTIFY markedChanged)
    void setMarked(bool val);
    bool isMarked() const { return m_marked; }
    Q_SIGNAL void markedChanged();

    Q_PROPERTY(QRectF rect READ rect NOTIFY rectChanged)
    QRectF rect() const { return m_rect; }
    Q_SIGNAL void rectChanged();

    Q_PROPERTY(QQuickItem *item READ item WRITE setItem NOTIFY itemChanged RESET resetItem)
    void setItem(QQuickItem *val);
    QQuickItem *item() const { return m_item; }
    Q_SIGNAL void itemChanged();

    bool isPlaced() const { return !m_rect.isNull(); }
    bool isPlacedByUser() const { return m_placedByUser; }

    // GraphLayout::AbstractNode interface
    bool canBeMoved() const { return !m_placedByUser; }
    QSizeF size() const { return m_rect.size(); }
    QObject *containerObject() { return this; }
    const QObject *containerObject() const { return this; }

protected:
    // GraphLayout::AbstractNode interface
    void move(const QPointF &pos);

protected:
    friend class CharacterRelationshipGraph;
    CharacterRelationshipGraphNode(QObject *parent = nullptr);
    void setCharacter(Character *val);
    void resetCharacter();
    void resetItem();
    void updateRectFromItem();
    void updateRectFromItemLater();
    void setRect(const QRectF &val);

    void timerEvent(QTimerEvent *te);

private:
    QRectF m_rect;
    bool m_marked = false;
    bool m_placedByUser = false;
    ExecLaterTimer m_updateRectTimer;
    QObjectProperty<QQuickItem> m_item;
    QObjectProperty<Character> m_character;
};

class CharacterRelationshipGraphEdge : public QObject, public GraphLayout::AbstractEdge
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~CharacterRelationshipGraphEdge();

    Q_PROPERTY(Relationship *relationship READ relationship NOTIFY relationshipChanged RESET resetRelationship)
    Relationship *relationship() const { return m_relationship; }
    Q_SIGNAL void relationshipChanged();

    Q_PROPERTY(QString forwardLabel READ forwardLabel NOTIFY forwardLabelChanged)
    QString forwardLabel() const { return m_forwardLabel; }
    Q_SIGNAL void forwardLabelChanged();

    Q_PROPERTY(QString reverseLabel READ reverseLabel NOTIFY reverseLabelChanged)
    QString reverseLabel() const { return m_reverseLabel; }
    Q_SIGNAL void reverseLabelChanged();

    Q_PROPERTY(QPainterPath path READ path NOTIFY pathChanged)
    QPainterPath path() const { return m_path; }
    Q_SIGNAL void pathChanged();

    Q_PROPERTY(QString pathString READ pathString NOTIFY pathChanged)
    QString pathString() const;

    Q_PROPERTY(QPointF labelPosition READ labelPosition NOTIFY pathChanged)
    QPointF labelPosition() const { return m_labelPos; }

    Q_PROPERTY(qreal labelAngle READ labelAngle NOTIFY pathChanged)
    qreal labelAngle() const { return m_labelAngle; }

    // GraphLayout::AbstractEdge interface
    GraphLayout::AbstractNode *node1() const { return m_fromNode; }
    GraphLayout::AbstractNode *node2() const { return m_toNode; }
    void evaluateEdge() { this->evaluatePath(); }
    QObject *containerObject() { return this; }
    const QObject *containerObject() const { return this; }

    void setEvaluatePathAllowed(bool val);
    bool isEvaluatePathAllowed() const { return m_evaluatePathAllowed; }

    void evaluatePath();

protected:
    friend class CharacterRelationshipGraph;
    CharacterRelationshipGraphEdge(CharacterRelationshipGraphNode *from,
                                   CharacterRelationshipGraphNode *to, QObject *parent = nullptr);
    void setRelationship(Relationship *val);
    void resetRelationship();
    void setPath(const QPainterPath &val);
    void setForwardLabel(const QString &val);
    void setReverseLabel(const QString &val);

private:
    QPointF m_labelPos;
    qreal m_labelAngle = 0;
    QPainterPath m_path;
    QString m_forwardLabel;
    QString m_reverseLabel;
    bool m_evaluatePathAllowed = false;
    QObjectProperty<Relationship> m_relationship;
    QPointer<CharacterRelationshipGraphNode> m_toNode;
    QPointer<CharacterRelationshipGraphNode> m_fromNode;
};

class CharacterRelationshipGraph : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit CharacterRelationshipGraph(QObject *parent = nullptr);
    ~CharacterRelationshipGraph();

    Q_PROPERTY(QAbstractListModel *nodes READ nodes CONSTANT STORED false)
    QAbstractListModel *nodes() const
    {
        return &((const_cast<CharacterRelationshipGraph *>(this))->m_nodes);
    }

    Q_PROPERTY(QAbstractListModel *edges READ edges CONSTANT STORED false)
    QAbstractListModel *edges() const
    {
        return &(const_cast<CharacterRelationshipGraph *>(this))->m_edges;
    }

    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
    bool isEmpty() const;
    Q_SIGNAL void emptyChanged();

    Q_PROPERTY(QSizeF nodeSize READ nodeSize WRITE setNodeSize NOTIFY nodeSizeChanged)
    void setNodeSize(const QSizeF &val);
    QSizeF nodeSize() const { return m_nodeSize; }
    Q_SIGNAL void nodeSizeChanged();

    Q_PROPERTY(Structure *structure READ structure WRITE setStructure NOTIFY structureChanged RESET resetStructure)
    void setStructure(Structure *val);
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(Scene *scene READ scene WRITE setScene NOTIFY sceneChanged RESET resetScene)
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(Character *character READ character WRITE setCharacter NOTIFY characterChanged RESET resetCharacter)
    void setCharacter(Character *val);
    Character *character() const { return m_character; }
    Q_SIGNAL void characterChanged();

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(int maxTime READ maxTime WRITE setMaxTime NOTIFY maxTimeChanged)
    void setMaxTime(int val);
    int maxTime() const { return m_maxTime; }
    Q_SIGNAL void maxTimeChanged();

    Q_PROPERTY(
            int maxIterations READ maxIterations WRITE setMaxIterations NOTIFY maxIterationsChanged)
    void setMaxIterations(int val);
    int maxIterations() const { return m_maxIterations; }
    Q_SIGNAL void maxIterationsChanged();

    Q_PROPERTY(QRectF graphBoundingRect READ graphBoundingRect NOTIFY graphBoundingRectChanged)
    QRectF graphBoundingRect() const { return m_graphBoundingRect; }
    Q_SIGNAL void graphBoundingRectChanged();

    Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    void setLeftMargin(qreal val);
    qreal leftMargin() const { return m_leftMargin; }
    Q_SIGNAL void leftMarginChanged();

    Q_PROPERTY(qreal topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
    void setTopMargin(qreal val);
    qreal topMargin() const { return m_topMargin; }
    Q_SIGNAL void topMarginChanged();

    Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    void setRightMargin(qreal val);
    qreal rightMargin() const { return m_rightMargin; }
    Q_SIGNAL void rightMarginChanged();

    Q_PROPERTY(
            qreal bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
    void setBottomMargin(qreal val);
    qreal bottomMargin() const { return m_bottomMargin; }
    Q_SIGNAL void bottomMarginChanged();

    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)
    bool isDirty() const { return m_dirty; }
    Q_SIGNAL void dirtyChanged();

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    Q_INVOKABLE void reload();
    Q_INVOKABLE void reset();

    Q_SIGNAL void updated();

    CharacterRelationshipsGraphExporter *createExporter();
    Q_INVOKABLE QObject *createExporterObject();

    QObject *graphJsonObject() const;

    void updateGraphJsonFromNode(CharacterRelationshipGraphNode *node);

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

protected:
    void timerEvent(QTimerEvent *te);

private:
    void setGraphBoundingRect(const QRectF &val);
    void resetStructure();
    void resetScene();
    void resetCharacter();
    void load();
    void loadLater();
    void evaluateTitle();
    void markDirty() { this->setDirty(true); }
    void setDirty(bool val);
    void setBusy(bool val);

private:
    bool m_busy = false;
    bool m_dirty = false;
    int m_maxTime = 100;
    QString m_title;
    QSizeF m_nodeSize = QSizeF(100, 100);
    qreal m_topMargin = 0;
    qreal m_leftMargin = 0;
    qreal m_rightMargin = 0;
    qreal m_bottomMargin = 0;
    int m_maxIterations = -1;
    QObjectProperty<Scene> m_scene;
    bool m_componentLoaded = false;
    QRectF m_graphBoundingRect = QRectF(0, 0, 500, 500);
    ExecLaterTimer m_loadTimer;
    ErrorReport *m_errorReport = new ErrorReport(this);
    QObjectProperty<Character> m_character;
    QObjectProperty<Structure> m_structure;
    QObjectListModel<CharacterRelationshipGraphNode *> m_nodes;
    QObjectListModel<CharacterRelationshipGraphEdge *> m_edges;
};

#endif // CHARACTERRELATIONSHIPGRAPH_H
