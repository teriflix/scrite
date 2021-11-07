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

#ifndef CHARACTERRELATIONSHIPSGRAPH_P_H
#define CHARACTERRELATIONSHIPSGRAPH_P_H

#include <QGraphicsRectItem>

#include "pdfexportablegraphicsscene.h"
#include "characterrelationshipsgraph.h"

class CharacterRelationshipsGraphScene : public PdfExportableGraphicsScene
{
public:
    CharacterRelationshipsGraphScene(const CharacterRelationshipsGraph *graph, QObject *parent=nullptr);
    ~CharacterRelationshipsGraphScene();
};

class CharacterRelationshipsGraphNodeItem : public QGraphicsRectItem
{
public:
    CharacterRelationshipsGraphNodeItem(const CharacterRelationshipsGraphNode *node);
    ~CharacterRelationshipsGraphNodeItem();

    // QGraphicsItem interface
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const CharacterRelationshipsGraphNode *m_node;
};

class CharacterRelationshipsGraphEdgeItem : public QGraphicsPathItem
{
public:
    CharacterRelationshipsGraphEdgeItem(const CharacterRelationshipsGraphEdge *edge);
    ~CharacterRelationshipsGraphEdgeItem();
};

#endif // CHARACTERRELATIONSHIPSGRAPH_P_H
