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

    this->load();
}

void CharacterRelationshipsGraph::resetStructure()
{
    m_structure = nullptr;
    this->load();
    emit structureChanged();
}

void CharacterRelationshipsGraph::load()
{
    auto nodes = m_nodes.list();
    m_nodes.clear();
    qDeleteAll(nodes);
    nodes.clear();

    auto edges = m_edges.list();
    m_edges.clear();
    qDeleteAll(edges);
    edges.clear();

    if(m_structure.isNull())
        return;

    QMap<Character*,CharacterRelationshipsGraphNode*> nodeMap;

    // For the moment, we ignore the filter-by-character-names list.

    // First we collect all nodes and edges
    for(int i=0; i<m_structure->characterCount(); i++)
    {
        Character *character = m_structure->characterAt(i);

        CharacterRelationshipsGraphNode *node = new CharacterRelationshipsGraphNode(this);
        node->setCharacter(character);
        nodes.append(node);

        nodeMap[character] = node;

        for(int j=0; j<character->relationshipCount(); j++)
        {
            Relationship *relationship = character->relationshipAt(j);

            CharacterRelationshipsGraphEdge *edge = new CharacterRelationshipsGraphEdge(this);
            edge->setRelationship(relationship);
            edges.append(edge);
        }
    }

    // Now we layout the nodes and prepare edge-paths to give a beautiful graph appearance.

    m_nodes.assign(nodes);
    m_edges.assign(edges);
}


