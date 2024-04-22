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
    explicit StructureExporterScene(const StructureExporter *exporter, QObject *parent = nullptr);
    ~StructureExporterScene();
};

class StructureIndexCard : public QGraphicsRectItem
{
public:
    explicit StructureIndexCard(const StructureExporter *exporter, const StructureElement *element);
    ~StructureIndexCard();
};

class StructureIndexCardFields : public QGraphicsRectItem
{
public:
    explicit StructureIndexCardFields(const StructureElement *element, const qreal availableWidth);
    ~StructureIndexCardFields();
};

class StructureIndexCardFieldsLegend : public QGraphicsRectItem
{
public:
    explicit StructureIndexCardFieldsLegend(const Structure *structure);
    ~StructureIndexCardFieldsLegend();
};

class StructureIndexCardConnector : public QGraphicsPathItem
{
public:
    explicit StructureIndexCardConnector(const StructureIndexCard *from,
                                         const StructureIndexCard *to, const QString &label);
    ~StructureIndexCardConnector();
};

class StructureEpisodeBox : public QGraphicsRectItem
{
public:
    explicit StructureEpisodeBox(const QJsonObject &data, const Structure *structure);
    ~StructureEpisodeBox();
};

class StructureIndexCardGroup : public QGraphicsRectItem
{
public:
    explicit StructureIndexCardGroup(const QJsonObject &data, const Structure *structure);
    ~StructureIndexCardGroup();
};

class StructureIndexCardStack : public QGraphicsItem
{
public:
    explicit StructureIndexCardStack(const StructureElementStack *stack);
    ~StructureIndexCardStack();

    // QGraphicsItem interface
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class StructureRectAnnotation : public QGraphicsRectItem
{
public:
    explicit StructureRectAnnotation(const Annotation *annotation,
                                     const QString &bgColorAttr = QStringLiteral("color"));
    ~StructureRectAnnotation();
};

class StructureTextAnnotation : public StructureRectAnnotation
{
public:
    explicit StructureTextAnnotation(const Annotation *annotation);
    ~StructureTextAnnotation();
};

class StructureUrlAnnotation : public QGraphicsRectItem
{
public:
    explicit StructureUrlAnnotation(const Annotation *annotation);
    ~StructureUrlAnnotation();
};

class StructureImageAnnotation : public StructureRectAnnotation
{
public:
    explicit StructureImageAnnotation(const Annotation *annotation);
    ~StructureImageAnnotation();
};

class StructureLineAnnotation : public QGraphicsLineItem
{
public:
    explicit StructureLineAnnotation(const Annotation *annotation);
    ~StructureLineAnnotation();
};

class StructureOvalAnnotation : public QGraphicsEllipseItem
{
public:
    explicit StructureOvalAnnotation(const Annotation *annotation);
    ~StructureOvalAnnotation();
};

class StructureUnknownAnnotation : public QGraphicsRectItem
{
public:
    explicit StructureUnknownAnnotation(const Annotation *annotation);
    ~StructureUnknownAnnotation();
};

class StructureTitleCard : public QGraphicsRectItem
{
public:
    explicit StructureTitleCard(const Structure *structure, const QString &comment);
    ~StructureTitleCard();
};

#endif // STRUCTUREEXPORTER_P_H
