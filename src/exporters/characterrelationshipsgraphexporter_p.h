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

#ifndef CHARACTERRELATIONSHIPSGRAPHEXPORTER_P_H
#define CHARACTERRELATIONSHIPSGRAPHEXPORTER_P_H

#include <QGraphicsRectItem>

#include "pdfexportablegraphicsscene.h"
#include "characterrelationshipgraph.h"

class CharacterRelationshipsGraphScene : public PdfExportableGraphicsScene
{
public:
    explicit CharacterRelationshipsGraphScene(const CharacterRelationshipGraph *graph,
                                              QObject *parent = nullptr);
    ~CharacterRelationshipsGraphScene();
};

class CharacterRelationshipsGraphNodeItem : public QGraphicsRectItem
{
public:
    explicit CharacterRelationshipsGraphNodeItem(const CharacterRelationshipGraphNode *node);
    ~CharacterRelationshipsGraphNodeItem();

    // QGraphicsItem interface
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const CharacterRelationshipGraphNode *m_node;
};

class CharacterRelationshipsGraphEdgeItem : public QGraphicsPathItem
{
public:
    explicit CharacterRelationshipsGraphEdgeItem(const CharacterRelationshipGraphEdge *edge);
    ~CharacterRelationshipsGraphEdgeItem();
};

#endif // CHARACTERRELATIONSHIPSGRAPHEXPORTER_P_H
