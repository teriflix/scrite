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

#include "structureexporter.h"

#include <QPainter>
#include <QFileInfo>
#include <QPdfWriter>
#include <QPainterPath>

StructureExporter::StructureExporter(QObject *parent)
    :AbstractExporter(parent)
{

}

StructureExporter::~StructureExporter()
{

}

QPainterPath evaluateConnectorPath(const QRectF &r1, const QRectF &r2)
{
    const QLineF line(r1.center(), r2.center());
    QPointF p1, p2;
    Qt::Edge e1, e2;
    QPainterPath path;

    if(r2.center().x() < r1.left())
    {
        p1 = QLineF(r1.topLeft(), r1.bottomLeft()).center();
        e1 = Qt::LeftEdge;

        if(r2.top() > r1.bottom())
        {
            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else if(r1.top() > r2.bottom())
        {
            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
        else
        {
            p2 = QLineF(r2.topRight(), r2.bottomRight()).center();
            e2 = Qt::RightEdge;
        }
    }
    else if(r2.center().x() > r1.right())
    {
        p1 = QLineF(r1.topRight(), r1.bottomRight()).center();
        e1 = Qt::RightEdge;

        if(r2.top() > r1.bottom())
        {
            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else if(r1.top() > r2.bottom())
        {
            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
        else
        {
            p2 = QLineF(r2.topLeft(), r2.bottomLeft()).center();
            e2 = Qt::LeftEdge;
        }
    }
    else
    {
        if(r2.top() > r1.bottom())
        {
            p1 = QLineF(r1.bottomLeft(), r1.bottomRight()).center();
            e1 = Qt::BottomEdge;

            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else
        {
            p1 = QLineF(r1.topLeft(), r1.topRight()).center();
            e1 = Qt::TopEdge;

            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
    }

    QPointF cp = p1;
    switch(e1)
    {
    case Qt::LeftEdge:
    case Qt::RightEdge:
        cp = (e2 == Qt::BottomEdge || e2 == Qt::TopEdge) ? QPointF(p2.x(), p1.y()) : p1;
        break;
    default:
        cp = p1;
        break;
    }

    if(cp == p1)
    {
        path.moveTo(p1);
        path.lineTo(p2);
    }
    else
    {
        const qreal length = line.length();
        const qreal dist = 20.0;
        const qreal dt = dist / length;
        const qreal maxdt = 1.0 - dt;
        const QLineF l1(p1, cp);
        const QLineF l2(cp, p2);
        qreal t = dt;

        path.moveTo(p1);
        while(t < maxdt)
        {
            const QLineF l( l1.pointAt(t), l2.pointAt(t) );
            const QPointF p = l.pointAt(t);
            path.lineTo(p);
            t += dt;
        }
        path.lineTo(p2);
    }

    static const QList<QPointF> arrowPoints = QList<QPointF>()
            << QPointF(-10,-5) << QPointF(0, 0) << QPointF(-10,5);

    const qreal angle = path.angleAtPercent(0.5);
    const QPointF lineCenter = path.pointAtPercent(0.5);

    QTransform tx;
    tx.translate(lineCenter.x(), lineCenter.y());
    tx.rotate(-angle);
    path.moveTo( tx.map(arrowPoints.at(0)) );
    path.lineTo( tx.map(arrowPoints.at(1)) );
    path.lineTo( tx.map(arrowPoints.at(2)) );

    return path;
}

bool StructureExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const Structure *structure = this->document()->structure();
    QRectF structureRect;

    for(int i=0; i<structure->elementCount(); i++)
    {
        const StructureElement *element = structure->elementAt(i);
        structureRect |= QRectF(element->x(), element->y(), element->width(), element->height());
    }

    QPdfWriter pdfWriter(device);
    pdfWriter.setTitle(screenplay->title() + " - Structure");
    pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());

    QSizeF pageSize = structureRect.size();
    pageSize /= pdfWriter.resolution();
    pageSize += QSizeF(0.4, 0.2);
    pdfWriter.setPageSize( QPageSize(pageSize,QPageSize::Inch) );

    pdfWriter.setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

    const qreal scale = qMin( qreal(pdfWriter.width())/structureRect.width(),
                              qreal(pdfWriter.height())/structureRect.height() );

    QPainter paint(&pdfWriter);
    paint.translate(-structureRect.left(), -structureRect.top());
    paint.scale(scale, scale);

    QFont font;
    font.setPixelSize(20);
    paint.setFont(font);

    const StructureElement *previousElement = nullptr;
    QRectF previousElementRect;
    QColor previousElementColor;
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        const ScreenplayElement *element = screenplay->elementAt(i);
        if(element == nullptr || element->scene() == nullptr)
            continue;

        const int elementIndex = structure->indexOfScene(element->scene());
        const StructureElement *currentElement = structure->elementAt(elementIndex);
        if(currentElement->scene() == nullptr)
            continue;

        const QRectF currentElementRect( currentElement->x(), currentElement->y(), currentElement->width(), currentElement->height() );
        const QColor currentElementColor( currentElement->scene()->color() );

        QColor fillColor = currentElementColor;
        fillColor.setAlphaF(0.2);
        paint.setBrush( QBrush(fillColor) );

        QColor outlineColor = currentElementColor;
        if(outlineColor == Qt::white || outlineColor == Qt::yellow)
            outlineColor = Qt::black;
        paint.setPen( QPen(outlineColor,2.0) );

        const qreal radius = qMin(currentElement->width(), currentElement->height())*0.1;
        paint.drawRoundedRect(currentElementRect, radius, radius, Qt::AbsoluteSize);

        paint.setPen(Qt::black);
        paint.drawText( currentElementRect.adjusted(radius,radius,-radius,-radius),
                        Qt::AlignCenter|Qt::TextWordWrap, currentElement->scene()->title() );

        if(previousElement != nullptr)
        {
            const QPainterPath path = evaluateConnectorPath(previousElementRect, currentElementRect);
            const QColor pathColor = QColor::fromRgbF( (previousElementColor.redF()+currentElementColor.redF())/2.0,
                                                 (previousElementColor.greenF()+currentElementColor.greenF())/2.0,
                                                 (previousElementColor.blueF()+currentElementColor.blueF())/2.0 );
            paint.setPen( QPen(pathColor, 4) );
            paint.setBrush( Qt::NoBrush );
            paint.drawPath(path);
        }

        previousElement = currentElement;
        previousElementRect = currentElementRect;
    }

    paint.end();

    return true;
}

QString StructureExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix().toLower() != "pdf")
        return fileName + ".pdf";
    return fileName;
}
