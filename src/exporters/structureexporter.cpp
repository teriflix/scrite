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
#include "application.h"

#include <QPainter>
#include <QFileInfo>
#include <QPdfWriter>
#include <QPainterPath>
#include <QAbstractTextDocumentLayout>

StructureExporter::StructureExporter(QObject *parent)
    :AbstractExporter(parent)
{

}

StructureExporter::~StructureExporter()
{

}

QPainterPath evaluateConnectorPath(const QRectF &r1, const QRectF &r2, QPointF *labelPos=nullptr)
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

    if(labelPos)
    {
        const qreal labelT = 0.45 - (30.0 / path.length());
        if( labelT < 0 || labelT >= 1 )
            *labelPos = lineCenter;
        else
            *labelPos = path.pointAtPercent(labelT);
    }

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

    for(int i=0; i<structure->annotationCount(); i++)
    {
        const Annotation *annotation = structure->annotationAt(i);
        structureRect |= annotation->geometry();
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

    QFont font = qApp->font();
    font.setPixelSize( Application::instance()->idealFontPointSize() );
    paint.setFont(font);

    for(int i=0; i<structure->annotationCount(); i++)
    {
        const Annotation *annotation = structure->annotationAt(i);
        paint.save();
        this->paintAnnotation(&paint, annotation);
        paint.restore();
    }

    font.setPixelSize(12);
    paint.setFont(font);

    // Draw connector lines in the first pass
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

        if(previousElement != nullptr)
        {
            QPointF labelPos;
            const QPainterPath path = evaluateConnectorPath(previousElementRect, currentElementRect, &labelPos);
            const QColor pathColor = QColor::fromRgbF( (previousElementColor.redF()+currentElementColor.redF())/2.0,
                                                 (previousElementColor.greenF()+currentElementColor.greenF())/2.0,
                                                 (previousElementColor.blueF()+currentElementColor.blueF())/2.0 );
            paint.setPen( QPen(pathColor, 2.0) );
            paint.setBrush( Qt::NoBrush );
            paint.drawPath(path);

            paint.setPen( QPen(pathColor, 1.0) );
            paint.setBrush(Qt::white);
            const QString label = QString::number(i);
            QRectF labelRect = paint.fontMetrics().boundingRect(label);
            labelRect.setWidth( qMax(labelRect.width(),labelRect.height()) );
            labelRect.setHeight(labelRect.width());
            labelRect.adjust(-5, -5, 5, 5);
            labelRect.moveCenter(labelPos);
            paint.drawRoundedRect(labelRect, 50, 50, Qt::RelativeSize);
            paint.setPen(Qt::black);
            paint.drawText(labelRect, Qt::AlignCenter, label);
        }

        previousElement = currentElement;
        previousElementRect = currentElementRect;
    }

    // Draw elements in second pass.
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

        const qreal radius = qMin(currentElement->width(), currentElement->height())*0.1;
        paint.setBrush(Qt::white);
        paint.setPen(Qt::NoPen);
        paint.drawRoundedRect(currentElementRect, radius, radius, Qt::AbsoluteSize);

        QColor fillColor = currentElementColor;
        fillColor.setAlphaF(0.2);
        paint.setBrush( QBrush(fillColor) );

        QColor outlineColor = currentElementColor;
        if(outlineColor == Qt::white || outlineColor == Qt::yellow)
            outlineColor = Qt::black;
        paint.setPen( QPen(outlineColor,2.0) );
        paint.drawRoundedRect(currentElementRect, radius, radius, Qt::AbsoluteSize);

        paint.setPen(Qt::black);
        paint.drawText( currentElementRect.adjusted(10,10,-10,-10),
                        Qt::AlignCenter|Qt::TextWordWrap, currentElement->scene()->title() );
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

void StructureExporter::paintAnnotation(QPainter *painter, const Annotation *annotation)
{
    const QJsonObject attributes = annotation->attributes();
    const QRectF geometry = annotation->geometry();

    auto paintRectangleAnnotation = [=](const QString &bgColorAttr = QStringLiteral("color")) {
        const QColor backgroundColor = QColor(attributes.value(bgColorAttr).toString());
        const QColor borderColor = QColor(attributes.value(QStringLiteral("borderColor")).toString());
        const qreal borderWidth = attributes.value(QStringLiteral("borderWidth")).toDouble();
        const bool fillBackground = attributes.value(QStringLiteral("fillBackground")).toBool();
        const qreal opacity = qBound(0.0, attributes.value(QStringLiteral("opacity")).toDouble(), 100.0)/100.0;

        QBrush brush(Qt::NoBrush);
        if(fillBackground)
            brush = QBrush(backgroundColor);

        QPen pen(Qt::NoPen);
        if(borderWidth > 0)
            pen = QPen(borderColor, borderWidth);

        painter->setOpacity(opacity);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRect(geometry);
        painter->setOpacity(1);
    };

    auto paintTextAnnotation = [=]() {
        paintRectangleAnnotation( QStringLiteral("backgroundColor") );

        const QString fontFamily = attributes.value(QStringLiteral("fontFamily")).toString();
        const int fontSize = attributes.value(QStringLiteral("fontSize")).toInt();
        const QJsonArray fontStyles = attributes.value(QStringLiteral("fontStyle")).toArray();
        const QString hAlign = attributes.value(QStringLiteral("hAlign")).toString();
        const QString vAlign = attributes.value(QStringLiteral("vAlign")).toString();
        const QString text = attributes.value(QStringLiteral("text")).toString();
        const QColor textColor = QColor(attributes.value(QStringLiteral("textColor")).toString());

        if(text.isEmpty())
            return;

        QFont font(fontFamily);
        font.setPixelSize(fontSize);
        for(int i=0; i<fontStyles.size(); i++) {
            const QString fontStyle = fontStyles.at(i).toString();
            if(!font.bold())
                font.setBold(fontStyle == QStringLiteral("bold"));
            if(!font.italic())
                font.setItalic(fontStyle == QStringLiteral("italic"));
            if(!font.underline())
                font.setUnderline(fontStyle == QStringLiteral("underline"));
        }

        painter->setFont(font);

        int flags = Qt::TextWordWrap;
        if(hAlign == QStringLiteral("center"))
            flags |= Qt::AlignHCenter;
        else if(hAlign == QStringLiteral("left"))
            flags |= Qt::AlignLeft;
        else if(hAlign == QStringLiteral("right"))
            flags |= Qt::AlignRight;

        if(vAlign == QStringLiteral("center"))
            flags |= Qt::AlignVCenter;
        else if(vAlign == QStringLiteral("top"))
            flags |= Qt::AlignTop;
        else if(vAlign == QStringLiteral("bottom"))
            flags |= Qt::AlignBottom;

        painter->setPen(textColor);

        const QRectF textRect = geometry.adjusted(10, 10, -10, -10);
        painter->drawText( textRect, flags, text );
    };

    auto paintUrlAnnotation = [=]() {
        const QString imageName = attributes.value(QStringLiteral("imageName")).toString();
        const QString imagePath = imageName.isEmpty() ? QString() : this->document()->fileSystem()->absolutePath(imageName);
        const QString url = attributes.value(QStringLiteral("url")).toString();
        const QString title = attributes.value(QStringLiteral("title")).toString();
        const QString description = attributes.value(QStringLiteral("description")).toString();

        painter->setBrush(Qt::white);
        painter->setPen(QPen(Qt::black,0));
        painter->drawRect(geometry);

        QFont font = qApp->font();
        font.setPixelSize( Application::instance()->idealFontPointSize() );
        painter->setFont(font);

        if(url.isEmpty()) {
            painter->drawText(geometry.adjusted(10,10,-10,-10), Qt::TextWordWrap|Qt::AlignCenter, QStringLiteral("No URL was set."));
            return;
        }

        QRectF rect = geometry.adjusted(8,8,-8,-8);
        rect.setHeight( rect.width()*9.0/16.0 );
        if(imagePath.isEmpty())
            painter->fillRect(rect, Qt::lightGray);
        else {
            const QImage image(imagePath);
            painter->drawImage(rect, image);
        }

        font.setPixelSize( Application::instance()->idealFontPointSize()+2 );
        font.setBold(true);
        painter->setFont(font);

        rect.moveTop(rect.bottom() + 8);
        rect.setHeight(painter->fontMetrics().lineSpacing()*2);
        painter->drawText(rect, Qt::TextWordWrap, title);

        font.setPixelSize( Application::instance()->idealFontPointSize() );
        font.setBold(false);
        painter->setFont(font);

        rect.moveTop(rect.bottom() + 8);
        rect.setHeight(painter->fontMetrics().lineSpacing()*4);
        painter->drawText(rect, Qt::TextWordWrap, description);

        font.setPixelSize( Application::instance()->idealFontPointSize()-2 );
        font.setBold(false);
        painter->setFont(font);

        rect.moveTop(rect.bottom() + 8);
        rect.setHeight(painter->fontMetrics().lineSpacing()*1);

        QTextDocument doc;
        doc.setDefaultFont(font);
        doc.setHtml( QStringLiteral("<a href=\"") + url + QStringLiteral("\">Click here to open link.</a>") );
        QAbstractTextDocumentLayout::PaintContext context;
        painter->save();
        painter->translate(rect.left(), rect.top());
        doc.documentLayout()->draw(painter, context);
        painter->restore();
    };

    auto paintImageAnnotation = [=]() {
        paintRectangleAnnotation(QStringLiteral("backgroundColor"));

        const QString image = attributes.value(QStringLiteral("image")).toString();
        const QString imagePath = image.isEmpty() ? QString() : this->document()->fileSystem()->absolutePath(image);
        const QString caption = attributes.value(QStringLiteral("caption")).toString();
        const QString captionAlignment = attributes.value(QStringLiteral("captionAlignment")).toString();
        const QColor captionColor = QColor(attributes.value(QStringLiteral("captionColor")).toString());

        QRectF rect = geometry.adjusted(5,5,-5,-5);
        if(!imagePath.isEmpty()) {
            QImage image(imagePath);
            QSizeF size = QSizeF(image.size()).scaled(rect.size(), Qt::KeepAspectRatio);
            rect.setSize(size);
            rect.moveCenter(geometry.center());
            rect.moveTop(geometry.top()+5);
            painter->drawImage(rect, image);
        }

        if(!caption.isEmpty()) {
            rect.moveTop(rect.bottom() + 5);
            rect.setBottom(geometry.bottom()-5);

            int flags = Qt::TextWordWrap;
            if(captionAlignment == QStringLiteral("center"))
                flags |= Qt::AlignHCenter;
            else if(captionAlignment == QStringLiteral("left"))
                flags |= Qt::AlignLeft;
            else if(captionAlignment == QStringLiteral("right"))
                flags |= Qt::AlignRight;

            painter->setPen(captionColor);
            painter->drawText(rect, flags, caption);
        }
    };

    auto paintLineAnnotation = [=]() {
        const QColor lineColor = QColor(attributes.value(QStringLiteral("lineColor")).toString());
        const int lineWidth = attributes.value(QStringLiteral("lineWidth")).toInt();
        const qreal opacity = attributes.value(QStringLiteral("opacity")).toDouble();
        const QString orientation = attributes.value(QStringLiteral("orientation")).toString();

        const QPointF center = geometry.center();
        QLineF line;

        if(orientation == QStringLiteral("Vertical"))
            line = QLineF(center.x(), geometry.top(), center.x(), geometry.bottom());
        else
            line = QLineF(geometry.left(), center.y(), geometry.right(), center.y());

        QPen pen(lineColor, lineWidth);
        painter->setPen(pen);
        painter->setOpacity(opacity);
        painter->drawLine(line);
        painter->setOpacity(1);
    };

    const QString type = annotation->type();
    if(type == QStringLiteral("rectangle"))
        paintRectangleAnnotation();
    else if(type == QStringLiteral("text"))
        paintTextAnnotation();
    else if(type == QStringLiteral("url"))
        paintUrlAnnotation();
    else if(type == QStringLiteral("image"))
        paintImageAnnotation();
    else if(type == QStringLiteral("line"))
        paintLineAnnotation();
    else
    {
        QPen pen(Qt::black, 1, Qt::DashDotDotLine);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(annotation->geometry());
    }
}
