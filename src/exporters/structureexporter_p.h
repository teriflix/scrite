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

#ifndef STRUCTUREEXPORTER_P_H
#define STRUCTUREEXPORTER_P_H

#include <QGraphicsItem>
#include <QGraphicsScene>

#include "structure.h"
#include "pdfexportablegraphicsscene.h"

class StructureExporter;

class StructureExporterScene : public PdfExportableGraphicsScene
{
    Q_OBJECT

public:
    StructureExporterScene(const StructureExporter *exporter, QObject *parent = nullptr);
    ~StructureExporterScene();
};

class StructureIndexCard : public QGraphicsRectItem
{
public:
    StructureIndexCard(const StructureElement *element);
    ~StructureIndexCard();
};

class StructureIndexCardConnector : public QGraphicsPathItem
{
public:
    StructureIndexCardConnector(const StructureIndexCard *from, const StructureIndexCard *to,
                                const QString &label);
    ~StructureIndexCardConnector();
};

class StructureEpisodeBox : public QGraphicsRectItem
{
public:
    StructureEpisodeBox(const QJsonObject &data, const Structure *structure);
    ~StructureEpisodeBox();
};

class StructureIndexCardGroup : public QGraphicsRectItem
{
public:
    StructureIndexCardGroup(const QJsonObject &data, const Structure *structure);
    ~StructureIndexCardGroup();
};

class StructureIndexCardStack : public QGraphicsItem
{
public:
    StructureIndexCardStack(const StructureElementStack *stack);
    ~StructureIndexCardStack();

    // QGraphicsItem interface
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class StructureRectAnnotation : public QGraphicsRectItem
{
public:
    StructureRectAnnotation(const Annotation *annotation,
                            const QString &bgColorAttr = QStringLiteral("color"));
    ~StructureRectAnnotation();
};

class StructureTextAnnotation : public StructureRectAnnotation
{
public:
    StructureTextAnnotation(const Annotation *annotation);
    ~StructureTextAnnotation();
};

class StructureUrlAnnotation : public QGraphicsRectItem
{
public:
    StructureUrlAnnotation(const Annotation *annotation);
    ~StructureUrlAnnotation();
};

class StructureImageAnnotation : public StructureRectAnnotation
{
public:
    StructureImageAnnotation(const Annotation *annotation);
    ~StructureImageAnnotation();
};

class StructureLineAnnotation : public QGraphicsLineItem
{
public:
    StructureLineAnnotation(const Annotation *annotation);
    ~StructureLineAnnotation();
};

class StructureOvalAnnotation : public QGraphicsEllipseItem
{
public:
    StructureOvalAnnotation(const Annotation *annotation);
    ~StructureOvalAnnotation();
};

class StructureUnknownAnnotation : public QGraphicsRectItem
{
public:
    StructureUnknownAnnotation(const Annotation *annotation);
    ~StructureUnknownAnnotation();
};

class StructureTitleCard : public QGraphicsRectItem
{
public:
    StructureTitleCard(const Structure *structure, const QString &comment);
    ~StructureTitleCard();
};

#endif // STRUCTUREEXPORTER_P_H
