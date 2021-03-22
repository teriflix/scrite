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

#include "structureexporter_p.h"

#include <QAbstractTextDocumentLayout>

#include "application.h"
#include "scritedocument.h"

inline QFont applicationFont()
{
    QFont font = Application::instance()->font();
    font.setPointSize(Application::instance()->idealFontPointSize());
    return font;
}

StructureExporterScene::StructureExporterScene(const Structure *structure, QObject *parent)
    : QGraphicsScene(parent)
{
    this->setBackgroundBrush(Qt::white);

    // Add all elements as index cards
    QHash<StructureElement*, StructureIndexCard*> elementIndexCardMap;
    for(int i=0; i<structure->elementCount(); i++)
    {
        StructureElement *element = structure->elementAt(i);
        StructureIndexCard *indexCard = new StructureIndexCard(element);
        indexCard->setZValue(10);
        this->addItem(indexCard);

        elementIndexCardMap[element] = indexCard;
    }

    // Add all connectors
    ScriteDocument *document = structure->scriteDocument() ? structure->scriteDocument() : ScriteDocument::instance();
    QAbstractListModel *connectorsModel = document->structureElementConnectors();
    StructureElementConnectors *connectors = qobject_cast<StructureElementConnectors*>(connectorsModel);
    for(int i=0; i<connectors->count(); i++)
    {
        StructureElement *from = connectors->fromElement(i);
        StructureElement *to = connectors->toElement(i);
        const QString label = connectors->label(i);

        if(from->stackId().isEmpty() || to->stackId().isEmpty() || from->stackId() != to->stackId())
        {
            StructureIndexCard *fromIndexCard = elementIndexCardMap.value(from, nullptr);
            StructureIndexCard *toIndexCard = elementIndexCardMap.value(to, nullptr);
            if(fromIndexCard == nullptr || toIndexCard == nullptr)
                continue;

            StructureIndexCardConnector *connector = new StructureIndexCardConnector(fromIndexCard, toIndexCard, label);
            connector->setZValue(9);
            this->addItem(connector);
        }
    }

    // Add all groups
    const QJsonArray beats = structure->evaluateBeats(document->screenplay(), structure->preferredGroupCategory());
    for(int i=0; i<beats.size(); i++)
    {
        const QJsonObject beat = beats.at(i).toObject();

        StructureIndexCardGroup *group = new StructureIndexCardGroup(beat, structure);
        group->setZValue(5);
        this->addItem(group);
    }

    // Add stacks
    StructureElementStacks *stacks = structure->elementStacks();
    for(int i=0; i<stacks->objectCount(); i++)
    {
        QObject *stackObject = stacks->objectAt(i);
        StructureElementStack *stack = qobject_cast<StructureElementStack*>(stackObject);
        if(stack == nullptr)
            continue;

        StructureIndexCardStack *stackItem = new StructureIndexCardStack(stack);
        stackItem->setZValue(7);
        this->addItem(stackItem);
    }

    // Add annotations
    for(int i=0; i<structure->annotationCount(); i++)
    {
        const Annotation *annotation = structure->annotationAt(i);

        QGraphicsItem *annotationItem = nullptr;

        const QString type = annotation->type();
        if(type == QStringLiteral("rectangle"))
            annotationItem = new StructureRectAnnotation(annotation);
        else if(type == QStringLiteral("text"))
            annotationItem = new StructureTextAnnotation(annotation);
        else if(type == QStringLiteral("url"))
            annotationItem = new StructureUrlAnnotation(annotation);
        else if(type == QStringLiteral("image"))
            annotationItem = new StructureImageAnnotation(annotation);
        else if(type == QStringLiteral("line"))
            annotationItem = new StructureLineAnnotation(annotation);
        else if(type == QStringLiteral("oval"))
            annotationItem = new StructureOvalAnnotation(annotation);
        else
            annotationItem = new StructureUnknownAnnotation(annotation);

        annotationItem->setZValue(6);
        this->addItem(annotationItem);
    }
}

StructureExporterScene::~StructureExporterScene()
{

}

///////////////////////////////////////////////////////////////////////////////

StructureIndexCard::StructureIndexCard(const StructureElement *element)
{
    this->setPos( element->x(), element->y() );
    this->setRect( 0, 0, element->width(), element->height() );

    const StructureElementStacks *stacks = element->structure()->elementStacks();
    const StructureElementStack *stack = stacks->findStackById(element->stackId());
    const bool stackedOnTop = stack == nullptr || stack->topmostElement() == element;
    if(!stackedOnTop)
    {
        this->setVisible(false);
        return;
    }

    const QColor sceneColor = element->scene()->color();
    const QColor penColor = Application::instance()->isLightColor(sceneColor) ? QColor(Qt::black) : sceneColor;

    // Transparency is best achived using opacity, rather than using alpha channel in color.
    this->setBrush(sceneColor);
    this->setPen(QPen(penColor, 2.0));

    QGraphicsRectItem *bgColorItem = new QGraphicsRectItem(this);
    bgColorItem->setRect(this->rect().adjusted(2,2,-2,-2));
    bgColorItem->setBrush(Qt::white);
    bgColorItem->setOpacity(0.9);
    bgColorItem->setPen(Qt::NoPen);

    const QFont fixedWidthFont(QStringLiteral("Courier Prime"), Application::instance()->idealFontPointSize());
    const QFont normalFont = ::applicationFont();
    auto boldFont = [](const QFont &font) {
        QFont f = font;
        f.setBold(true);
        return f;
    };
    auto smallFont = [](const QFont &font) {
        QFont f = font;
        f.setPointSize(f.pointSize()-2);
        return f;
    };

    const QRectF contentRect(this->rect().adjusted(5,5,-5,-5));

    // Draw scene heading at the top
    QGraphicsTextItem *headingTextItem = new QGraphicsTextItem(this);
    headingTextItem->setTextWidth(contentRect.width());
    if(element->scene()->heading()->isEnabled())
        headingTextItem->setPlainText(element->scene()->heading()->text());
    else
        headingTextItem->setPlainText(QStringLiteral("NO SCENE HEADING"));
    headingTextItem->setFont(boldFont(fixedWidthFont));

    headingTextItem->setX(contentRect.x());
    headingTextItem->setY(contentRect.y());

    // Draw tags at the bottom
    const QString tagsText = element->structure()->presentableGroupNames(element->scene()->groups());
    QGraphicsTextItem *tagsTextItem = new QGraphicsTextItem(this);
    tagsTextItem->setTextWidth(contentRect.width());
    tagsTextItem->setHtml(tagsText);
    tagsTextItem->setFont(smallFont(normalFont));

    tagsTextItem->setX(contentRect.x());
    tagsTextItem->setY(contentRect.bottom()-tagsTextItem->boundingRect().height());

    const QRectF headingTextRect = headingTextItem->mapToParent(headingTextItem->boundingRect()).boundingRect();
    const QRectF tagsTextRect = tagsTextItem->mapToParent(tagsTextItem->boundingRect()).boundingRect();
    const QRectF synopsisTextRect = QRectF(headingTextRect.bottomLeft(), tagsTextRect.topRight()).adjusted(0, 10, 0, -10);

    // Synopsis text in between
    QGraphicsRectItem *synopsisTextRectItem = new QGraphicsRectItem(this);
    synopsisTextRectItem->setPos(synopsisTextRect.topLeft());
    synopsisTextRectItem->setRect(QRectF(0,0,synopsisTextRect.width(),synopsisTextRect.height()));
    synopsisTextRectItem->setBrush(QColor::fromRgbF(1,1,1,0.4));
    synopsisTextRectItem->setPen(Qt::NoPen);
    synopsisTextRectItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    QGraphicsTextItem *synopsisTextItem = new QGraphicsTextItem(synopsisTextRectItem);
    synopsisTextItem->setPos(0,0);
    synopsisTextItem->setTextWidth(contentRect.width());
    synopsisTextItem->setPlainText(element->scene()->title());
    synopsisTextItem->setFont(normalFont);

    if(synopsisTextItem->boundingRect().height() > synopsisTextRectItem->rect().height())
        synopsisTextRectItem->setPen( QPen(QColor::fromRgbF(0,0,0,0.5),1) );
}

StructureIndexCard::~StructureIndexCard()
{

}

///////////////////////////////////////////////////////////////////////////////

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
        *labelPos = path.pointAtPercent(0.65);

    QTransform tx;
    tx.translate(lineCenter.x(), lineCenter.y());
    tx.rotate(-angle);
    path.moveTo( tx.map(arrowPoints.at(0)) );
    path.lineTo( tx.map(arrowPoints.at(1)) );
    path.lineTo( tx.map(arrowPoints.at(2)) );

    return path;
}

StructureIndexCardConnector::StructureIndexCardConnector(const StructureIndexCard *from, const StructureIndexCard *to, const QString &label)
{
    const QRectF r1 = from->mapToScene(from->boundingRect()).boundingRect();
    const QRectF r2 = to->mapToScene(to->boundingRect()).boundingRect();
    QPointF labelPos;

    const QPainterPath path = evaluateConnectorPath(r1, r2, &labelPos);
    this->setPath(path);

    const QFont normalFont = ::applicationFont();

    QGraphicsSimpleTextItem *labelText = new QGraphicsSimpleTextItem(this);
    labelText->setFont(normalFont);
    labelText->setText(label);
    labelText->setPos(labelPos - labelText->boundingRect().center());
    labelText->setZValue(1);

    const QSizeF labelSize = labelText->boundingRect().size() * 1.2;
    const qreal size = qMax(labelSize.width(), labelSize.height());
    QRectF labelBgRect(0.0, 0.0, size, size);
    labelBgRect.moveCenter(labelPos);

    QGraphicsEllipseItem *labelBackground = new QGraphicsEllipseItem(this);
    labelBackground->setBrush(Qt::white);
    labelBackground->setPen(QPen(Qt::black));
    labelBackground->setOpacity(0.75);
    labelBackground->setRect(labelBgRect);
}

StructureIndexCardConnector::~StructureIndexCardConnector()
{

}

///////////////////////////////////////////////////////////////////////////////

StructureIndexCardGroup::StructureIndexCardGroup(const QJsonObject &data, const Structure *structure)
{
    const QJsonObject geometryJson = data.value( QStringLiteral("geometry") ).toObject();
          QRectF geometry( 0, 0,
                           geometryJson.value( QStringLiteral("width") ).toDouble(),
                           geometryJson.value( QStringLiteral("height") ).toDouble() );

    geometry.adjust(-20, -20, 20, 20);

    if(structure != nullptr && structure->elementStacks()->objectCount() > 0)
        geometry.adjust(0, -15, 0, 0);

    this->setRect(geometry);
    this->setBrush(Qt::NoBrush);
    this->setPen( QPen(Qt::gray,2) );
    this->setPos( QPointF(geometryJson.value( QStringLiteral("x") ).toDouble(),
                          geometryJson.value( QStringLiteral("y") ).toDouble()) );

    QGraphicsRectItem *bgItem = new QGraphicsRectItem(this);
    bgItem->setRect( geometry.adjusted(1,1,-1,-1) );
    bgItem->setPen(Qt::NoPen);
    bgItem->setBrush( Qt::gray );
    bgItem->setOpacity(0.1);

    const QFont normalFont = ::applicationFont();

    const QString name = data.value(QStringLiteral("name")).toString();
    const int sceneCount = data.value(QStringLiteral("sceneCount")).toInt();

    const QString title = QStringLiteral("<b>") + name + QStringLiteral("</b><font size=\"-2\">: ") + QString::number(sceneCount) + (sceneCount == 1 ? QStringLiteral(" scene"): QStringLiteral(" scenes")) + QStringLiteral("</font>");

    QGraphicsTextItem *titleText = new QGraphicsTextItem(this);
    titleText->setDefaultTextColor(Qt::black);
    titleText->setFont(normalFont);
    titleText->setHtml(title);
    titleText->setZValue(1);

    QRectF titleRect = titleText->boundingRect();
    titleRect.adjust(-10, -10, 10, 10);
    titleRect.moveBottomLeft( geometry.topLeft() );

    QGraphicsRectItem *titleBackground = new QGraphicsRectItem(this);
    titleBackground->setRect(titleRect);
    titleBackground->setBrush(Qt::NoBrush);
    titleBackground->setPen(this->pen());

    bgItem = new QGraphicsRectItem(this);
    bgItem->setRect(titleRect.adjusted(1,1,-1,-1));
    bgItem->setPen(Qt::NoPen);
    bgItem->setBrush( Qt::gray );
    bgItem->setOpacity(0.1);

    titleRect.adjust(10, 10, -10, -10);
    titleText->setPos(titleRect.topLeft());
}

StructureIndexCardGroup::~StructureIndexCardGroup()
{

}

///////////////////////////////////////////////////////////////////////////////

StructureIndexCardStack::StructureIndexCardStack(const StructureElementStack *stack)
{
    QRectF geometry1 = stack->geometry();
    this->setPos(geometry1.topLeft());
    geometry1.moveTopLeft(QPointF(0,0));

    const int nrCards = qMin(5, stack->objectCount());
    for(int i=nrCards-1; i>=0; i--)
    {
        int idx = i;
        if(idx == stack->topmostElementIndex())
            ++idx;

        const StructureElement *element = qobject_cast<StructureElement*>(stack->objectAt(i));
        const QColor lineColor = element == nullptr ? stack->topmostElement()->scene()->color() : element->scene()->color();

        const int shift = i*3;
        const QRectF geometry2 = geometry1.adjusted(shift,shift,shift,shift);
        QPainterPath shadowPath;
        shadowPath.moveTo(geometry2.bottomLeft());
        shadowPath.lineTo(geometry2.bottomRight());
        shadowPath.lineTo(geometry2.topRight());

        QGraphicsRectItem *shadow = new QGraphicsRectItem(this);
        shadow->setRect(geometry2);
        shadow->setBrush(Qt::white);
        shadow->setPen(QPen(lineColor,1));
    }
}

StructureIndexCardStack::~StructureIndexCardStack()
{

}

QRectF StructureIndexCardStack::boundingRect() const
{
    return QRectF(0,0,1,1);
}

void StructureIndexCardStack::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

///////////////////////////////////////////////////////////////////////////////

StructureRectAnnotation::StructureRectAnnotation(const Annotation *annotation, const QString &bgColorAttr)
{
    const QJsonObject attributes = annotation->attributes();

    const QColor backgroundColor = QColor(attributes.value(bgColorAttr).toString());
    const QColor borderColor = QColor(attributes.value(QStringLiteral("borderColor")).toString());
    const qreal borderWidth = attributes.value(QStringLiteral("borderWidth")).toDouble();
    const bool fillBackground = attributes.value(QStringLiteral("fillBackground")).toBool();
    const qreal opacity = qBound(0.0, attributes.value(QStringLiteral("opacity")).toDouble(), 100.0)/100.0;

    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0,0,geometry.width(),geometry.height()));

    if(fillBackground)
        this->setBrush(backgroundColor);
    else
        this->setBrush(Qt::NoBrush);
    this->setPen(QPen(borderColor,borderWidth));
    this->setOpacity(opacity);
}

StructureRectAnnotation::~StructureRectAnnotation()
{

}

StructureTextAnnotation::StructureTextAnnotation(const Annotation *annotation)
    : StructureRectAnnotation(annotation, QStringLiteral("backgroundColor"))
{
    const QJsonObject attributes = annotation->attributes();

    const QString fontFamily = attributes.value(QStringLiteral("fontFamily")).toString();
    const int fontSize = attributes.value(QStringLiteral("fontSize")).toInt();
    const QJsonArray fontStyles = attributes.value(QStringLiteral("fontStyle")).toArray();
    const QString hAlign = attributes.value(QStringLiteral("hAlign")).toString();
    const QString vAlign = attributes.value(QStringLiteral("vAlign")).toString();
    const QString text = attributes.value(QStringLiteral("text")).toString();
    const QColor textColor = QColor(attributes.value(QStringLiteral("textColor")).toString());

    if(text.isEmpty())
        return;

    QRectF textAreaRect = this->rect().adjusted(10, 10, -10, -10);

    QGraphicsRectItem *textArea = new QGraphicsRectItem(this);
    textArea->setPos(textAreaRect.topLeft());
    textAreaRect.moveTopLeft(QPointF(0,0));
    textArea->setRect(textAreaRect);
    textArea->setBrush(Qt::NoBrush);
    textArea->setPen(Qt::NoPen);
    textArea->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    QGraphicsTextItem *textItem = new QGraphicsTextItem(textArea);
    textItem->setPos(QPointF(0,0));
    textItem->setTextWidth(textAreaRect.width());
    textItem->setPlainText(text);
    textItem->setDefaultTextColor(textColor);

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

    textItem->setFont(font);

    Qt::Alignment alignment;
    if(hAlign == QStringLiteral("center"))
        alignment |= Qt::AlignHCenter;
    else if(hAlign == QStringLiteral("left"))
        alignment |= Qt::AlignLeft;
    else if(hAlign == QStringLiteral("right"))
        alignment |= Qt::AlignRight;

    QTextCursor cursor(textItem->document());
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QTextBlockFormat format;
    format.setAlignment(alignment);
    cursor.mergeBlockFormat(format);

    QRectF textItemRect = textItem->boundingRect();

    if(vAlign == QStringLiteral("center"))
        textItemRect.moveCenter(textAreaRect.center());
    else if(vAlign == QStringLiteral("top"))
        textItemRect.moveTop(textAreaRect.top());
    else if(vAlign == QStringLiteral("bottom"))
        textItemRect.moveBottom(textAreaRect.bottom());

    textItem->setPos(textItemRect.topLeft());
}

StructureTextAnnotation::~StructureTextAnnotation()
{

}

StructureUrlAnnotation::StructureUrlAnnotation(const Annotation *annotation)
{
    const Structure *structure = annotation->structure();
    ScriteDocument *document = structure == nullptr ? ScriteDocument::instance() : structure->scriteDocument();

    const QJsonObject attributes = annotation->attributes();

    const QString imageName = attributes.value(QStringLiteral("imageName")).toString();
    const QString imagePath = imageName.isEmpty() ? QString() : document->fileSystem()->absolutePath(imageName);
    const QString url = attributes.value(QStringLiteral("url")).toString();
    const QString title = attributes.value(QStringLiteral("title")).toString();
    const QString description = attributes.value(QStringLiteral("description")).toString();

    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0,0,geometry.width(),geometry.height()));
    this->setBrush(Qt::white);
    this->setPen(QPen(Qt::black,1));

    const QFont normalFont = ::applicationFont();

    if(url.isEmpty())
    {
        QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
        textItem->setFont(normalFont);
        textItem->setPlainText( QStringLiteral("No URL was set.") );

        QRectF textRect = textItem->boundingRect();
        textRect.moveCenter(this->rect().center());
        textItem->setPos(textRect.topLeft());

        return;
    }

    QRectF contentRect = this->rect().adjusted(8, 8, -8, -8);

    QGraphicsRectItem *contentItem = new QGraphicsRectItem(this);
    contentItem->setPos(contentRect.topLeft());
    contentItem->setRect(QRectF(0,0,contentRect.width(),contentRect.height()));
    contentItem->setBrush(Qt::NoBrush);
    contentItem->setPen(Qt::NoPen);
    contentItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    contentRect = contentItem->rect();

    QRectF imageRect = contentRect;
    imageRect.setHeight( imageRect.width()*9.0/16.0 );
    if(imagePath.isEmpty())
    {
        QGraphicsRectItem *emptyImageItem = new QGraphicsRectItem(contentItem);
        emptyImageItem->setBrush(Qt::lightGray);
        emptyImageItem->setPen(Qt::NoPen);
        emptyImageItem->setPos(imageRect.topLeft());
        emptyImageItem->setRect(QRectF(0,0,imageRect.width(),imageRect.height()));
    }
    else
    {
        QPixmap pixmap(imagePath);
        pixmap = pixmap.scaled(imageRect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(contentItem);
        pixmapItem->setPixmap(pixmap);
        pixmapItem->setPos(imageRect.topLeft());
    }

    QFont titleFont = normalFont;
    titleFont.setPointSize(titleFont.pointSize()+2);
    titleFont.setBold(true);

    QGraphicsTextItem *titleText = new QGraphicsTextItem(contentItem);
    titleText->setPlainText(title);
    titleText->setFont(titleFont);
    titleText->setTextWidth(contentRect.width());

    QRectF titleRect = titleText->boundingRect();
    titleRect.moveLeft(contentRect.left());
    titleRect.moveTop(imageRect.bottom()+8);
    titleText->setPos(titleRect.topLeft());

    QGraphicsTextItem *urlText = new QGraphicsTextItem(contentItem);
    urlText->setOpenExternalLinks(true);
    urlText->setHtml(QStringLiteral("<a href=\"") + url + QStringLiteral("\">Click here to open link.</a>"));
    urlText->setFont(normalFont);
    urlText->setTextWidth(contentRect.width());

    QRectF urlRect = urlText->boundingRect();
    urlRect.moveBottomLeft(contentRect.bottomLeft());
    urlText->setPos(urlRect.topLeft());

    QRectF descRect = contentRect;
    descRect.setTop(titleRect.bottom()+8);
    descRect.setBottom(urlRect.top()-8);

    QGraphicsRectItem *descArea = new QGraphicsRectItem(contentItem);
    descArea->setPos(descRect.topLeft());
    descArea->setRect(QRectF(0,0,descRect.width(),descRect.height()));
    descArea->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    descArea->setBrush(Qt::NoBrush);
    descArea->setPen(Qt::NoPen);

    QGraphicsTextItem *descItem = new QGraphicsTextItem(descArea);
    descItem->setPos(0, 0);
    descItem->setTextWidth(descRect.width());
    descItem->setFont(normalFont);
    descItem->setPlainText(description);
}

StructureUrlAnnotation::~StructureUrlAnnotation()
{

}

StructureImageAnnotation::StructureImageAnnotation(const Annotation *annotation)
    : StructureRectAnnotation(annotation, QStringLiteral("backgroundColor"))
{
    const Structure *structure = annotation->structure();
    ScriteDocument *document = structure == nullptr ? ScriteDocument::instance() : structure->scriteDocument();

    const QJsonObject attributes = annotation->attributes();
    const QString image = attributes.value(QStringLiteral("image")).toString();
    const QString imagePath = image.isEmpty() ? QString() : document->fileSystem()->absolutePath(image);
    const QString caption = attributes.value(QStringLiteral("caption")).toString();
    const QString captionAlignment = attributes.value(QStringLiteral("captionAlignment")).toString();
    const QColor captionColor = QColor(attributes.value(QStringLiteral("captionColor")).toString());

    const QFont normalFont = ::applicationFont();

    QRectF contentRect = this->rect().adjusted(5, 5, -5, -5);

    QGraphicsRectItem *contentItem = new QGraphicsRectItem(this);
    contentItem->setPos(contentRect.topLeft());
    contentItem->setRect(QRectF(0,0,contentRect.width(),contentRect.height()));
    contentItem->setBrush(Qt::NoBrush);
    contentItem->setPen(Qt::NoPen);
    contentItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    contentRect = contentItem->rect();

    QRectF imageRect = contentRect;
    if(imagePath.isEmpty())
    {
        imageRect.setHeight( imageRect.width()*9.0/16.0 );

        QGraphicsRectItem *emptyImageItem = new QGraphicsRectItem(contentItem);
        emptyImageItem->setBrush(Qt::lightGray);
        emptyImageItem->setPen(Qt::NoPen);
        emptyImageItem->setPos(imageRect.topLeft());
        emptyImageItem->setRect(QRectF(0,0,imageRect.width(),imageRect.height()));
    }
    else
    {
        QPixmap pixmap(imagePath);

        QSize pixmapSize = pixmap.size();
        pixmapSize.scale(imageRect.size().toSize(), Qt::KeepAspectRatio);
        pixmap = pixmap.scaled(pixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        imageRect.setWidth(pixmap.width());
        imageRect.setHeight(pixmap.height());

        QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(contentItem);
        pixmapItem->setPixmap(pixmap);
        pixmapItem->setPos(imageRect.topLeft());
    }

    QRectF textRect = contentRect;
    textRect.setTop(imageRect.bottom()+5);

    QGraphicsTextItem *textItem = new QGraphicsTextItem(contentItem);
    textItem->setFont(normalFont);
    textItem->setDefaultTextColor(captionColor);
    textItem->setPlainText(caption);
    textItem->setPos(textRect.topLeft());

    Qt::Alignment alignment;
    if(captionAlignment == QStringLiteral("center"))
        alignment |= Qt::AlignHCenter;
    else if(captionAlignment == QStringLiteral("left"))
        alignment |= Qt::AlignLeft;
    else if(captionAlignment == QStringLiteral("right"))
        alignment |= Qt::AlignRight;

    QTextCursor cursor(textItem->document());
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QTextBlockFormat format;
    format.setAlignment(alignment);
    cursor.mergeBlockFormat(format);
}

StructureImageAnnotation::~StructureImageAnnotation()
{

}

StructureLineAnnotation::StructureLineAnnotation(const Annotation *annotation)
{
    const QJsonObject attributes = annotation->attributes();

    const QColor lineColor = QColor(attributes.value(QStringLiteral("lineColor")).toString());
    const int lineWidth = attributes.value(QStringLiteral("lineWidth")).toInt();
    const qreal opacity = attributes.value(QStringLiteral("opacity")).toDouble();
    const QString orientation = attributes.value(QStringLiteral("orientation")).toString();

    const QRectF geometry = annotation->geometry();
    const QPointF center = geometry.center();
    QLineF line;

    if(orientation == QStringLiteral("Vertical"))
        line = QLineF(center.x(), geometry.top(), center.x(), geometry.bottom());
    else
        line = QLineF(geometry.left(), center.y(), geometry.right(), center.y());

    this->setLine(line);

    QPen pen(lineColor, lineWidth);
    this->setPen(pen);
    this->setOpacity(opacity);
}

StructureLineAnnotation::~StructureLineAnnotation()
{

}

StructureOvalAnnotation::StructureOvalAnnotation(const Annotation *annotation)
{
    const QJsonObject attributes = annotation->attributes();

    const QColor backgroundColor = QColor(attributes.value(QStringLiteral("color")).toString());
    const QColor borderColor = QColor(attributes.value(QStringLiteral("borderColor")).toString());
    const qreal borderWidth = attributes.value(QStringLiteral("borderWidth")).toDouble();
    const bool fillBackground = attributes.value(QStringLiteral("fillBackground")).toBool();
    const qreal opacity = qBound(0.0, attributes.value(QStringLiteral("opacity")).toDouble(), 100.0)/100.0;

    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0,0,geometry.width(),geometry.height()));

    if(fillBackground)
        this->setBrush(backgroundColor);
    else
        this->setBrush(Qt::NoBrush);
    this->setPen(QPen(borderColor,borderWidth));
    this->setOpacity(opacity);
}

StructureOvalAnnotation::~StructureOvalAnnotation()
{

}

StructureUnknownAnnotation::StructureUnknownAnnotation(const Annotation *annotation)
{
    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0,0,geometry.width(),geometry.height()));

    QBrush brush(Qt::lightGray);
    brush.setStyle(Qt::CrossPattern);

    this->setBrush(brush);
    this->setPen(QPen(Qt::black,1));
}

StructureUnknownAnnotation::~StructureUnknownAnnotation()
{

}

///////////////////////////////////////////////////////////////////////////////

/// Dump screenplay title and other meta data into a box on the top.


