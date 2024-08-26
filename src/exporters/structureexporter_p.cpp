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

#include "structureexporter_p.h"

#include <QPainter>
#include <QDateTime>
#include <QPaintEngine>
#include <QAbstractTextDocumentLayout>

#include "application.h"
#include "scritedocument.h"
#include "structureexporter.h"

inline QFont applicationFont()
{
    QFont font = Application::instance()->font();
    font.setPointSize(Application::instance()->idealFontPointSize());
    return font;
}

StructureExporterScene::StructureExporterScene(const StructureExporter *exporter, QObject *parent)
    : PdfExportableGraphicsScene(parent)
{
    ScriteDocument *document = exporter->document();
    const Structure *structure = document->structure();

    this->setBackgroundBrush(Qt::white);

    // Add all elements as index cards
    QHash<StructureElement *, StructureIndexCard *> elementIndexCardMap;
    for (int i = 0; i < structure->elementCount(); i++) {
        StructureElement *element = structure->elementAt(i);
        StructureIndexCard *indexCard = new StructureIndexCard(exporter, element);
        indexCard->setZValue(10);
        this->addItem(indexCard);

        elementIndexCardMap[element] = indexCard;
    }

    // Add all connectors
    QAbstractListModel *connectorsModel = document->structureElementConnectors();
    StructureElementConnectors *connectors =
            qobject_cast<StructureElementConnectors *>(connectorsModel);
    for (int i = 0; i < connectors->count(); i++) {
        StructureElement *from = connectors->fromElement(i);
        StructureElement *to = connectors->toElement(i);
        const QString label = connectors->label(i);

        if (from->stackId().isEmpty() || to->stackId().isEmpty()
            || from->stackId() != to->stackId()) {
            StructureIndexCard *fromIndexCard = elementIndexCardMap.value(from, nullptr);
            StructureIndexCard *toIndexCard = elementIndexCardMap.value(to, nullptr);
            if (fromIndexCard == nullptr || toIndexCard == nullptr)
                continue;

            StructureIndexCardConnector *connector =
                    new StructureIndexCardConnector(fromIndexCard, toIndexCard, label);
            connector->setZValue(9);
            this->addItem(connector);
        }
    }

    const QJsonObject boxes = structure->evaluateEpisodeAndGroupBoxes(
            document->screenplay(), structure->preferredGroupCategory());

    // Add all chapters
    const QJsonArray episodeBoxes = boxes.value(QStringLiteral("episodeBoxes")).toArray();
    for (int i = 0; i < episodeBoxes.size(); i++) {
        const QJsonObject episodeBox = episodeBoxes.at(i).toObject();

        StructureEpisodeBox *episode = new StructureEpisodeBox(episodeBox, structure);
        episode->setZValue(4);
        this->addItem(episode);
    }

    // Add all groups
    const QJsonArray groupBoxes = boxes.value(QStringLiteral("groupBoxes")).toArray();
    for (int i = 0; i < groupBoxes.size(); i++) {
        const QJsonObject groupBox = groupBoxes.at(i).toObject();

        StructureIndexCardGroup *group = new StructureIndexCardGroup(groupBox, structure);
        group->setZValue(5);
        this->addItem(group);
    }

    // Add stacks
    StructureElementStacks *stacks = structure->elementStacks();
    for (int i = 0; i < stacks->objectCount(); i++) {
        QObject *stackObject = stacks->objectAt(i);
        StructureElementStack *stack = qobject_cast<StructureElementStack *>(stackObject);
        if (stack == nullptr)
            continue;

        StructureIndexCardStack *stackItem = new StructureIndexCardStack(stack);
        stackItem->setZValue(7);
        this->addItem(stackItem);
    }

    const QRectF indexCardsBox = this->itemsBoundingRect();

    // Add annotations
    for (int i = 0; i < structure->annotationCount(); i++) {
        const Annotation *annotation = structure->annotationAt(i);

        QGraphicsItem *annotationItem = nullptr;

        const QString type = annotation->type();
        if (type == QStringLiteral("rectangle"))
            annotationItem = new StructureRectAnnotation(annotation);
        else if (type == QStringLiteral("text"))
            annotationItem = new StructureTextAnnotation(annotation);
        else if (type == QStringLiteral("url"))
            annotationItem = new StructureUrlAnnotation(annotation);
        else if (type == QStringLiteral("image"))
            annotationItem = new StructureImageAnnotation(annotation);
        else if (type == QStringLiteral("line"))
            annotationItem = new StructureLineAnnotation(annotation);
        else if (type == QStringLiteral("oval"))
            annotationItem = new StructureOvalAnnotation(annotation);
        else
            annotationItem = new StructureUnknownAnnotation(annotation);

        annotationItem->setZValue(6);
        this->addItem(annotationItem);
    }

    QRectF contentsRect = this->itemsBoundingRect();
    QRectF titleCardRect;

    if (exporter->isInsertTitleCard()) {
        StructureTitleCard *titleCard = new StructureTitleCard(structure, exporter->comment());
        titleCardRect = titleCard->boundingRect();
        titleCardRect.setLeft(indexCardsBox.left() + 20);
        titleCardRect.moveBottom(indexCardsBox.top() - 20);
        if (!this->items(titleCardRect).isEmpty())
            titleCardRect.moveBottom(contentsRect.top() - 20);
        titleCard->setPos(titleCardRect.topLeft());
        this->addItem(titleCard);

        titleCardRect = titleCard->mapRectToParent(titleCard->boundingRect());
    }

    if (!structure->indexCardFields().isEmpty()) {
        StructureIndexCardFieldsLegend *icfLegend = new StructureIndexCardFieldsLegend(structure);

        QRectF icfLegendRect = icfLegend->boundingRect();
        if (titleCardRect.isEmpty())
            icfLegendRect.moveBottomLeft(contentsRect.topLeft() - QPointF(0, 20));
        else
            icfLegendRect.moveBottomLeft(titleCardRect.bottomRight() + QPointF(20, 0));

        icfLegend->setPos(icfLegendRect.topLeft());
        this->addItem(icfLegend);
    }

    this->addStandardItems(WatermarkOverlayLayer
                           + (exporter->isEnableHeaderFooter() ? HeaderFooterLayer : 0));
}

StructureExporterScene::~StructureExporterScene() { }

///////////////////////////////////////////////////////////////////////////////

StructureIndexCard::StructureIndexCard(const StructureExporter *exporter,
                                       const StructureElement *element)
{
    this->setPos(element->x(), element->y());
    this->setRect(0, 0, element->width(), element->height());

    const StructureElementStacks *stacks = element->structure()->elementStacks();
    const StructureElementStack *stack = stacks->findStackById(element->stackId());
    const bool stackedOnTop = stack == nullptr || stack->topmostElement() == element;
    if (!stackedOnTop) {
        this->setVisible(false);
        return;
    }

    const QColor sceneColor = element->scene()->color();
    const QColor penColor =
            Application::instance()->isLightColor(sceneColor) ? QColor(Qt::black) : sceneColor;

    // Transparency is best achived using opacity, rather than using alpha channel in color.
    this->setBrush(sceneColor);
    this->setPen(QPen(penColor, 2.0));

    QGraphicsRectItem *bgColorItem = new QGraphicsRectItem(this);
    bgColorItem->setRect(this->rect().adjusted(2, 2, -2, -2));
    bgColorItem->setBrush(Qt::white);
    bgColorItem->setOpacity(0.9);
    bgColorItem->setPen(Qt::NoPen);

    const QFont fixedWidthFont(QStringLiteral("Courier Prime"),
                               Application::instance()->idealFontPointSize());
    const QFont normalFont = ::applicationFont();
    auto boldFont = [](const QFont &font) {
        QFont f = font;
        f.setBold(true);
        return f;
    };
    auto smallFont = [](const QFont &font) {
        QFont f = font;
        f.setPointSize(f.pointSize() - 2);
        return f;
    };

    const QRectF contentRect(this->rect().adjusted(5, 5, -5, -5));

    // Draw scene heading at the top
    QGraphicsTextItem *headingTextItem = new QGraphicsTextItem(this);
    headingTextItem->setTextWidth(contentRect.width());
    if (!element->title().isEmpty())
        headingTextItem->setPlainText(element->title());
    else
        headingTextItem->setPlainText(QStringLiteral("NO SCENE HEADING"));
    headingTextItem->setFont(element->hasNativeTitle() ? boldFont(normalFont)
                                                       : boldFont(fixedWidthFont));

    headingTextItem->setX(contentRect.x());
    headingTextItem->setY(contentRect.y());

    // Draw tags at the bottom
    const QString tagsText =
            element->structure()->presentableGroupNames(element->scene()->groups());
    QGraphicsTextItem *tagsTextItem = new QGraphicsTextItem(this);
    tagsTextItem->setTextWidth(contentRect.width());
    tagsTextItem->setHtml(tagsText);
    tagsTextItem->setFont(smallFont(normalFont));

    tagsTextItem->setX(contentRect.x());
    tagsTextItem->setY(contentRect.bottom() - tagsTextItem->boundingRect().height());

    const QRectF headingTextRect =
            headingTextItem->mapToParent(headingTextItem->boundingRect()).boundingRect();
    const QRectF tagsTextRect =
            tagsTextItem->mapToParent(tagsTextItem->boundingRect()).boundingRect();
    QRectF cardContentRect =
            QRectF(headingTextRect.bottomLeft(), tagsTextRect.topRight()).adjusted(0, 10, 0, -10);

    // If index card fields exist, then lets place them just above tags
    const bool hasIndexCardFields = exporter->document()->structure()->indexCardFields().size() > 0;
    StructureIndexCardFields *indexCardFieldsItem = hasIndexCardFields
            ? new StructureIndexCardFields(element, contentRect.width())
            : nullptr;
    if (indexCardFieldsItem) {
        indexCardFieldsItem->setParentItem(this);
        indexCardFieldsItem->setPos(tagsTextRect.topLeft()
                                    - QPointF(0, indexCardFieldsItem->rect().height() + 10));

        QRectF indexCardFieldsItemRect =
                indexCardFieldsItem->mapToParent(indexCardFieldsItem->boundingRect())
                        .boundingRect();
        cardContentRect.setBottom(indexCardFieldsItemRect.top() - 10);
    }

    // Content in between. The content may be synopsis text or featured image.
    QGraphicsRectItem *cardContentRectItem = new QGraphicsRectItem(this);
    cardContentRectItem->setPos(cardContentRect.topLeft());
    cardContentRectItem->setRect(QRectF(0, 0, cardContentRect.width(), cardContentRect.height()));
    cardContentRectItem->setBrush(QColor::fromRgbF(1, 1, 1, 0.4));
    cardContentRectItem->setPen(Qt::NoPen);
    cardContentRectItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    // Check if the scene has featured image.
    const bool preferFeaturedImage = exporter->isPreferFeaturedImage();
    const Attachments *sceneAttachments =
            preferFeaturedImage ? element->scene()->attachments() : nullptr;
    const Attachment *featuredAttachment =
            sceneAttachments ? sceneAttachments->featuredAttachment() : nullptr;
    const Attachment *featuredImage =
            featuredAttachment && featuredAttachment->type() == Attachment::Photo
            ? featuredAttachment
            : nullptr;

    if (preferFeaturedImage && featuredImage) {
        GraphicsImageRectItem *featuredImageItem = new GraphicsImageRectItem(cardContentRectItem);
        featuredImageItem->setRect(cardContentRectItem->rect());
        featuredImageItem->setImage(QImage(featuredImage->fileSource().toLocalFile()));

        const QJsonObject userData = featuredImage->userData();
        const QString fillMode = userData.value(QLatin1String("indexCardFillMode")).toString();
        if (fillMode == QLatin1String("fit"))
            featuredImageItem->setFillMode(GraphicsImageRectItem::PreserveAspectFit);
        else
            featuredImageItem->setFillMode(GraphicsImageRectItem::PreserveAspectCrop);
    } else {
        QGraphicsTextItem *synopsisTextItem = new QGraphicsTextItem(cardContentRectItem);
        synopsisTextItem->setPos(0, 0);
        synopsisTextItem->setTextWidth(contentRect.width());
        synopsisTextItem->setPlainText(element->scene()->synopsis());
        synopsisTextItem->setFont(normalFont);

        if (synopsisTextItem->boundingRect().height() > cardContentRectItem->rect().height())
            cardContentRectItem->setPen(QPen(QColor::fromRgbF(0, 0, 0, 0.5), 1));
    }
}

StructureIndexCard::~StructureIndexCard() { }

///////////////////////////////////////////////////////////////////////////////

StructureIndexCardFields::StructureIndexCardFields(const StructureElement *element,
                                                   const qreal availableWidth)
{
    const QStringList fieldValues = element->scene()->indexCardFieldValues();
    const QJsonArray fields = element->structure()->indexCardFields();
    if (fields.isEmpty())
        return;

    const QFont normalFont = ::applicationFont();
    const QFont normalItalicFont = [normalFont]() {
        QFont ret = normalFont;
        ret.setItalic(true);
        return ret;
    }();

    // Generate text items for keys and values
    QList<QGraphicsTextItem *> keyItems, valueItems;
    for (int i = 0; i < fields.size(); i++) {
        const QJsonObject field = fields.at(i).toObject();
        const QString key = field.value(QStringLiteral("name")).toString();
        const QString desc = field.value(QStringLiteral("description")).toString();
        const QString value = i < fieldValues.size() ? fieldValues.at(i) : QString();

        QGraphicsTextItem *keyItem = new QGraphicsTextItem;
        keyItem->setPlainText(key);
        keyItem->setFont(normalFont);
        keyItems.append(keyItem);

        QGraphicsTextItem *valueItem = new QGraphicsTextItem;
        if (value.isEmpty()) {
            valueItem->setPlainText(desc);
            valueItem->setFont(normalItalicFont);
            valueItem->setOpacity(0.5);
        } else {
            valueItem->setPlainText(value);
            valueItem->setFont(normalFont);
        }
        valueItems.append(valueItem);
    }

    // Determine column widths
    const qreal keyColumnWidth = [keyItems, availableWidth]() {
        qreal ret = 0;
        for (const QGraphicsTextItem *keyItem : keyItems)
            ret = qMax(ret, keyItem->boundingRect().width());
        return qMin(ret * 1.1, availableWidth * 0.5);
    }();

    const qreal valueColumnWidth = availableWidth - keyColumnWidth;

    const qreal rowHeight = [keyItems, valueItems]() {
        qreal ret = 0;
        for (const QGraphicsTextItem *keyItem : keyItems)
            ret = qMax(ret, keyItem->boundingRect().height());
        for (const QGraphicsTextItem *valueItem : valueItems)
            ret = qMax(ret, valueItem->boundingRect().height());
        return ret;
    }();

    // Now layout key and value items in a 2 column grid
    QRectF keyItemRect(0, 0, keyColumnWidth, rowHeight),
            valueItemRect(keyColumnWidth, 0, valueColumnWidth, rowHeight);
    for (int i = 0; i < fields.size(); i++) {
        QGraphicsRectItem *keyCell = new QGraphicsRectItem(this);
        keyCell->setBrush(Qt::NoBrush);
        keyCell->setPen(Qt::NoPen);
        keyCell->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
        keyCell->setPos(keyItemRect.topLeft());
        keyCell->setRect(0, 0, keyItemRect.width(), keyItemRect.height());

        QGraphicsTextItem *keyItem = keyItems.at(i);
        keyItem->setParentItem(keyCell);

        QGraphicsRectItem *valueCell = new QGraphicsRectItem(this);
        valueCell->setBrush(Qt::NoBrush);
        valueCell->setPen(Qt::NoPen);
        valueCell->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
        valueCell->setPos(valueItemRect.topLeft());
        valueCell->setRect(0, 0, valueItemRect.width(), valueItemRect.height());

        QGraphicsTextItem *valueItem = valueItems.at(i);
        valueItem->setParentItem(valueCell);

        keyItemRect.moveTop(keyItemRect.bottom() + 1);
        valueItemRect.moveTop(valueItemRect.bottom() + 1);
    }

    const QRectF ibr = this->childrenBoundingRect();
    this->setRect(ibr);
    this->setBrush(Qt::NoBrush);
    this->setPen(Qt::NoPen);
}

StructureIndexCardFields::~StructureIndexCardFields() { }

//////////////////////////////////////////////////////////////////////////////

StructureIndexCardFieldsLegend::StructureIndexCardFieldsLegend(const Structure *structure)
{
    this->setPen(QPen(Qt::black));
    this->setBrush(Qt::NoBrush);

    QList<QGraphicsSimpleTextItem *> keyItems, descItems;
    const QJsonArray fields = structure->indexCardFields();
    const QFont normalFont = ::applicationFont();
    const QFont normalBoldFont = [normalFont]() {
        QFont ret = normalFont;
        ret.setBold(true);
        return ret;
    }();
    const QFont bigNormalBoldFont = [normalFont]() {
        QFont ret = normalFont;
        ret.setPointSize(normalFont.pointSize() + 2);
        ret.setBold(true);
        return ret;
    }();
    const qreal spacing = 5;

    QGraphicsSimpleTextItem *headingItem = new QGraphicsSimpleTextItem(this);
    headingItem->setFont(bigNormalBoldFont);
    headingItem->setText(QStringLiteral("Index Card Fields"));
    headingItem->setPos(spacing, spacing);

    QRectF headingItemRect = headingItem->mapRectToParent(headingItem->boundingRect());

    // Create key and desc items
    for (int i = 0; i < fields.size(); i++) {
        const QJsonObject field = fields.at(i).toObject();
        const QString key = field.value(QStringLiteral("name")).toString();
        const QString desc = field.value(QStringLiteral("description")).toString();

        QGraphicsSimpleTextItem *keyItem = new QGraphicsSimpleTextItem(this);
        keyItem->setText(key);
        keyItem->setFont(normalBoldFont);
        keyItems.append(keyItem);

        QGraphicsSimpleTextItem *descItem = new QGraphicsSimpleTextItem(this);
        descItem->setText(desc);
        descItem->setFont(normalFont);
        descItems.append(descItem);
    }

    // Layout all key items
    QRectF lastKeyItemRect = headingItemRect.adjusted(0, spacing, 0, spacing);
    qreal keyColumnWidth = 0;
    for (int i = 0; i < keyItems.size(); i++) {
        QGraphicsSimpleTextItem *keyItem = keyItems.at(i);
        keyItem->setPos(lastKeyItemRect.bottomLeft() + QPointF(0, spacing));
        lastKeyItemRect = keyItem->mapRectToParent(keyItem->boundingRect());
        keyColumnWidth = qMax(keyColumnWidth, lastKeyItemRect.width());
    }
    keyColumnWidth += 4 * spacing;

    QRectF lastDescItemRect = headingItemRect.adjusted(0, spacing, 0, spacing);
    lastDescItemRect.moveLeft(keyColumnWidth);
    for (int i = 0; i < descItems.size(); i++) {
        QGraphicsSimpleTextItem *descItem = descItems.at(i);
        descItem->setPos(lastDescItemRect.bottomLeft() + QPointF(0, spacing));
        lastDescItemRect = descItem->mapRectToParent(descItem->boundingRect());
    }

    const QRectF cbr = this->childrenBoundingRect();
    this->setRect(cbr.adjusted(-spacing, -spacing, spacing, spacing));
}

StructureIndexCardFieldsLegend::~StructureIndexCardFieldsLegend() { }

///////////////////////////////////////////////////////////////////////////////

QPainterPath evaluateConnectorPath(const QRectF &r1, const QRectF &r2, QPointF *labelPos = nullptr)
{
    const QPainterPath path = StructureElementConnector::curvedArrowPath(r1, r2, 7, false);
    if (!path.isEmpty() && labelPos) {
        const qreal pathLength = path.length();
        const qreal labelD = 30 / pathLength;
        const qreal labelT = 0.55 - labelD;
        if (labelT < 0 || labelT > 1)
            *labelPos = path.pointAtPercent(1.0 - labelD);
        else
            *labelPos = path.pointAtPercent(labelT);
    }
    return path;
}

StructureIndexCardConnector::StructureIndexCardConnector(const StructureIndexCard *from,
                                                         const StructureIndexCard *to,
                                                         const QString &label)
{
    const QRectF r1 = from->mapToScene(from->boundingRect()).boundingRect();
    const QRectF r2 = to->mapToScene(to->boundingRect()).boundingRect();
    QPointF labelPos;

    const QPainterPath path = evaluateConnectorPath(r1, r2, &labelPos);
    this->setPath(path);
    this->setPen(QPen(Qt::black));
    this->setBrush(Qt::NoBrush);

    const QFont labelFont = ::applicationFont();

    QGraphicsSimpleTextItem *labelText = new QGraphicsSimpleTextItem(this);
    labelText->setFont(labelFont);
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
    labelBackground->setOpacity(0.9);
    labelBackground->setRect(labelBgRect);
}

StructureIndexCardConnector::~StructureIndexCardConnector() { }

///////////////////////////////////////////////////////////////////////////////

StructureEpisodeBox::StructureEpisodeBox(const QJsonObject &data, const Structure *structure)
{
    Q_UNUSED(structure)

    const QJsonObject geometryJson = data.value(QStringLiteral("geometry")).toObject();
    QRectF geometry(0, 0, geometryJson.value(QStringLiteral("width")).toDouble(),
                    geometryJson.value(QStringLiteral("height")).toDouble());

    geometry.adjust(-40, -140, 40, 40);

    if (structure != nullptr && structure->elementStacks()->objectCount() > 0)
        geometry.adjust(0, -15, 0, 0);

    this->setRect(geometry);
    this->setBrush(Qt::NoBrush);
    this->setPen(QPen(Qt::black, 2));
    this->setPos(QPointF(geometryJson.value(QStringLiteral("x")).toDouble(),
                         geometryJson.value(QStringLiteral("y")).toDouble()));

    QGraphicsRectItem *bgItem = new QGraphicsRectItem(this);
    bgItem->setRect(geometry.adjusted(1, 1, -1, -1));
    bgItem->setPen(Qt::NoPen);
    bgItem->setBrush(Qt::gray);
    bgItem->setOpacity(0.1);

    const QFont normalFont = ::applicationFont();

    const QString name = data.value(QStringLiteral("name")).toString();
    const int sceneCount = data.value(QStringLiteral("sceneCount")).toInt();

    const QString title = QStringLiteral("<b>") + name + QStringLiteral("</b><font size=\"-2\">: ")
            + QString::number(sceneCount)
            + (sceneCount == 1 ? QStringLiteral(" Scene") : QStringLiteral(" Scenes"))
            + QStringLiteral("</font>");
    QFont titleFont = normalFont;
    titleFont.setPointSize(titleFont.pointSize() + 8);

    QGraphicsTextItem *titleItem = new QGraphicsTextItem(this);
    titleItem->setDefaultTextColor(Qt::black);
    titleItem->setFont(titleFont);
    titleItem->setHtml(title);
    titleItem->setPos(geometry.topLeft() + QPointF(8, 8));

    QGraphicsRectItem *titleBar = new QGraphicsRectItem(this);
    titleBar->setZValue(-1);
    titleBar->setRect(QRectF(geometry.x(), geometry.y(), geometry.width(),
                             titleItem->boundingRect().height() + 18)
                              .adjusted(1, 1, -1, 0));
    titleBar->setBrush(QColor("#546E7A"));
    titleBar->setPen(Qt::NoPen);
    titleBar->setOpacity(0.3);
}

StructureEpisodeBox::~StructureEpisodeBox() { }

///////////////////////////////////////////////////////////////////////////////

StructureIndexCardGroup::StructureIndexCardGroup(const QJsonObject &data,
                                                 const Structure *structure)
{
    const QJsonObject geometryJson = data.value(QStringLiteral("geometry")).toObject();
    QRectF geometry(0, 0, geometryJson.value(QStringLiteral("width")).toDouble(),
                    geometryJson.value(QStringLiteral("height")).toDouble());

    geometry.adjust(-20, -20, 20, 20);

    if (structure != nullptr && structure->elementStacks()->objectCount() > 0)
        geometry.adjust(0, -15, 0, 0);

    this->setRect(geometry);
    this->setBrush(Qt::NoBrush);
    this->setPen(QPen(Qt::gray, 2));
    this->setPos(QPointF(geometryJson.value(QStringLiteral("x")).toDouble(),
                         geometryJson.value(QStringLiteral("y")).toDouble()));

    QGraphicsRectItem *bgItem = new QGraphicsRectItem(this);
    bgItem->setRect(geometry.adjusted(1, 1, -1, -1));
    bgItem->setPen(Qt::NoPen);
    bgItem->setBrush(Qt::gray);
    bgItem->setOpacity(0.1);

    const QFont normalFont = ::applicationFont();

    const QString name = data.value(QStringLiteral("name")).toString();
    const int sceneCount = data.value(QStringLiteral("sceneCount")).toInt();

    const QString title = QStringLiteral("<b>") + name + QStringLiteral("</b><font size=\"-2\">: ")
            + QString::number(sceneCount)
            + (sceneCount == 1 ? QStringLiteral(" Scene") : QStringLiteral(" Scenes"))
            + QStringLiteral("</font>");
    const QFont titleFont = normalFont;

    QGraphicsTextItem *titleText = new QGraphicsTextItem(this);
    titleText->setDefaultTextColor(Qt::black);
    titleText->setFont(titleFont);
    titleText->setHtml(title);
    titleText->setZValue(1);

    QRectF titleRect = titleText->boundingRect();
    titleRect.adjust(-10, -10, 10, 10);
    titleRect.moveBottomLeft(geometry.topLeft());

    QGraphicsRectItem *titleBackground = new QGraphicsRectItem(this);
    titleBackground->setRect(titleRect);
    titleBackground->setBrush(Qt::NoBrush);
    titleBackground->setPen(this->pen());

    bgItem = new QGraphicsRectItem(this);
    bgItem->setRect(titleRect.adjusted(1, 1, -1, -1));
    bgItem->setPen(Qt::NoPen);
    bgItem->setBrush(Qt::gray);
    bgItem->setOpacity(0.1);

    titleRect.adjust(10, 10, -10, -10);
    titleText->setPos(titleRect.topLeft());
}

StructureIndexCardGroup::~StructureIndexCardGroup() { }

///////////////////////////////////////////////////////////////////////////////

StructureIndexCardStack::StructureIndexCardStack(const StructureElementStack *stack)
{
    QRectF geometry1 = stack->geometry();
    this->setPos(geometry1.topLeft());
    geometry1.moveTopLeft(QPointF(0, 0));

    const int nrCards = qMin(5, stack->objectCount());
    for (int i = nrCards - 1; i >= 0; i--) {
        int idx = i;
        if (idx == stack->topmostElementIndex())
            ++idx;

        const StructureElement *element = qobject_cast<StructureElement *>(stack->objectAt(i));
        const QColor lineColor = element == nullptr ? stack->topmostElement()->scene()->color()
                                                    : element->scene()->color();

        const int shift = i * 3;
        const QRectF geometry2 = geometry1.adjusted(shift, shift, shift, shift);
        QPainterPath shadowPath;
        shadowPath.moveTo(geometry2.bottomLeft());
        shadowPath.lineTo(geometry2.bottomRight());
        shadowPath.lineTo(geometry2.topRight());

        QGraphicsRectItem *shadow = new QGraphicsRectItem(this);
        shadow->setRect(geometry2);
        shadow->setBrush(Qt::white);
        shadow->setPen(QPen(lineColor, 1));
    }
}

StructureIndexCardStack::~StructureIndexCardStack() { }

QRectF StructureIndexCardStack::boundingRect() const
{
    return QRectF(0, 0, 1, 1);
}

void StructureIndexCardStack::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                    QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

///////////////////////////////////////////////////////////////////////////////

StructureRectAnnotation::StructureRectAnnotation(const Annotation *annotation,
                                                 const QString &bgColorAttr)
{
    const QJsonObject attributes = annotation->attributes();

    const QColor backgroundColor = QColor(attributes.value(bgColorAttr).toString());
    const QColor borderColor = QColor(attributes.value(QStringLiteral("borderColor")).toString());
    const qreal borderWidth = attributes.value(QStringLiteral("borderWidth")).toDouble();
    const bool fillBackground = attributes.value(QStringLiteral("fillBackground")).toBool();
    const qreal opacity =
            qBound(0.0, attributes.value(QStringLiteral("opacity")).toDouble(), 100.0) / 100.0;

    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0, 0, geometry.width(), geometry.height()));

    if (fillBackground)
        this->setBrush(backgroundColor);
    else
        this->setBrush(Qt::NoBrush);
    this->setPen(QPen(borderColor, borderWidth));
    this->setOpacity(opacity);
}

StructureRectAnnotation::~StructureRectAnnotation() { }

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

    if (text.isEmpty())
        return;

    QRectF textAreaRect = this->rect().adjusted(10, 10, -10, -10);

    QGraphicsRectItem *textArea = new QGraphicsRectItem(this);
    textArea->setPos(textAreaRect.topLeft());
    textAreaRect.moveTopLeft(QPointF(0, 0));
    textArea->setRect(textAreaRect);
    textArea->setBrush(Qt::NoBrush);
    textArea->setPen(Qt::NoPen);
    textArea->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    QGraphicsTextItem *textItem = new QGraphicsTextItem(textArea);
    textItem->setPos(QPointF(0, 0));
    textItem->setTextWidth(textAreaRect.width());
    textItem->setPlainText(text);
    textItem->setDefaultTextColor(textColor);

    QFont font(fontFamily);
    font.setPixelSize(fontSize);
    for (int i = 0; i < fontStyles.size(); i++) {
        const QString fontStyle = fontStyles.at(i).toString();
        if (!font.bold())
            font.setBold(fontStyle == QStringLiteral("bold"));
        if (!font.italic())
            font.setItalic(fontStyle == QStringLiteral("italic"));
        if (!font.underline())
            font.setUnderline(fontStyle == QStringLiteral("underline"));
    }

    textItem->setFont(font);

    Qt::Alignment alignment;
    if (hAlign == QStringLiteral("center"))
        alignment |= Qt::AlignHCenter;
    else if (hAlign == QStringLiteral("left"))
        alignment |= Qt::AlignLeft;
    else if (hAlign == QStringLiteral("right"))
        alignment |= Qt::AlignRight;

    QTextCursor cursor(textItem->document());
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QTextBlockFormat format;
    format.setAlignment(alignment);
    cursor.mergeBlockFormat(format);

    QRectF textItemRect = textItem->boundingRect();

    if (vAlign == QStringLiteral("center"))
        textItemRect.moveCenter(textAreaRect.center());
    else if (vAlign == QStringLiteral("top"))
        textItemRect.moveTop(textAreaRect.top());
    else if (vAlign == QStringLiteral("bottom"))
        textItemRect.moveBottom(textAreaRect.bottom());

    textItem->setPos(textItemRect.topLeft());
}

StructureTextAnnotation::~StructureTextAnnotation() { }

StructureUrlAnnotation::StructureUrlAnnotation(const Annotation *annotation)
{
    const Structure *structure = annotation->structure();
    ScriteDocument *document =
            structure == nullptr ? ScriteDocument::instance() : structure->scriteDocument();

    const QJsonObject attributes = annotation->attributes();

    const QString imageName = attributes.value(QStringLiteral("imageName")).toString();
    const QString imagePath =
            imageName.isEmpty() ? QString() : document->fileSystem()->absolutePath(imageName);
    const QString url = attributes.value(QStringLiteral("url")).toString();
    const QString title = attributes.value(QStringLiteral("title")).toString();
    const QString description = attributes.value(QStringLiteral("description")).toString();

    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0, 0, geometry.width(), geometry.height()));
    this->setBrush(Qt::white);
    this->setPen(QPen(Qt::black, 1));

    const QFont normalFont = ::applicationFont();

    if (url.isEmpty()) {
        QGraphicsTextItem *textItem = new QGraphicsTextItem(this);
        textItem->setFont(normalFont);
        textItem->setPlainText(QStringLiteral("No URL was set."));

        QRectF textRect = textItem->boundingRect();
        textRect.moveCenter(this->rect().center());
        textItem->setPos(textRect.topLeft());

        return;
    }

    QRectF contentRect = this->rect().adjusted(8, 8, -8, -8);

    QGraphicsRectItem *contentItem = new QGraphicsRectItem(this);
    contentItem->setPos(contentRect.topLeft());
    contentItem->setRect(QRectF(0, 0, contentRect.width(), contentRect.height()));
    contentItem->setBrush(Qt::NoBrush);
    contentItem->setPen(Qt::NoPen);
    contentItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    contentRect = contentItem->rect();

    QRectF imageRect = contentRect;
    imageRect.setHeight(imageRect.width() * 9.0 / 16.0);
    if (imagePath.isEmpty()) {
        QGraphicsRectItem *emptyImageItem = new QGraphicsRectItem(contentItem);
        emptyImageItem->setBrush(Qt::lightGray);
        emptyImageItem->setPen(Qt::NoPen);
        emptyImageItem->setPos(imageRect.topLeft());
        emptyImageItem->setRect(QRectF(0, 0, imageRect.width(), imageRect.height()));
    } else {
        QPixmap pixmap(imagePath);
        pixmap = pixmap.scaled(imageRect.size().toSize(), Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);

        QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(contentItem);
        pixmapItem->setPixmap(pixmap);
        pixmapItem->setPos(imageRect.topLeft());
    }

    QFont titleFont = normalFont;
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);

    QGraphicsTextItem *titleText = new QGraphicsTextItem(contentItem);
    titleText->setPlainText(title);
    titleText->setFont(titleFont);
    titleText->setTextWidth(contentRect.width());

    QRectF titleRect = titleText->boundingRect();
    titleRect.moveLeft(contentRect.left());
    titleRect.moveTop(imageRect.bottom() + 8);
    titleText->setPos(titleRect.topLeft());

    QGraphicsTextItem *urlText = new QGraphicsTextItem(contentItem);
    urlText->setOpenExternalLinks(true);
    urlText->setHtml(QStringLiteral("<a href=\"") + url
                     + QStringLiteral("\">Click here to open link.</a>"));
    urlText->setFont(normalFont);
    urlText->setTextWidth(contentRect.width());

    QRectF urlRect = urlText->boundingRect();
    urlRect.moveBottomLeft(contentRect.bottomLeft());
    urlText->setPos(urlRect.topLeft());

    QRectF descRect = contentRect;
    descRect.setTop(titleRect.bottom() + 8);
    descRect.setBottom(urlRect.top() - 8);

    QGraphicsRectItem *descArea = new QGraphicsRectItem(contentItem);
    descArea->setPos(descRect.topLeft());
    descArea->setRect(QRectF(0, 0, descRect.width(), descRect.height()));
    descArea->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    descArea->setBrush(Qt::NoBrush);
    descArea->setPen(Qt::NoPen);

    QGraphicsTextItem *descItem = new QGraphicsTextItem(descArea);
    descItem->setPos(0, 0);
    descItem->setTextWidth(descRect.width());
    descItem->setFont(normalFont);
    descItem->setPlainText(description);
}

StructureUrlAnnotation::~StructureUrlAnnotation() { }

StructureImageAnnotation::StructureImageAnnotation(const Annotation *annotation)
    : StructureRectAnnotation(annotation, QStringLiteral("backgroundColor"))
{
    const Structure *structure = annotation->structure();
    ScriteDocument *document =
            structure == nullptr ? ScriteDocument::instance() : structure->scriteDocument();

    const QJsonObject attributes = annotation->attributes();
    const QString image = attributes.value(QStringLiteral("image")).toString();
    const QString imagePath =
            image.isEmpty() ? QString() : document->fileSystem()->absolutePath(image);
    const QString caption = attributes.value(QStringLiteral("caption")).toString();
    const QString captionAlignment =
            attributes.value(QStringLiteral("captionAlignment")).toString();
    const QColor captionColor = QColor(attributes.value(QStringLiteral("captionColor")).toString());

    const QFont normalFont = ::applicationFont();

    QRectF contentRect = this->rect().adjusted(5, 5, -5, -5);

    QGraphicsRectItem *contentItem = new QGraphicsRectItem(this);
    contentItem->setPos(contentRect.topLeft());
    contentItem->setRect(QRectF(0, 0, contentRect.width(), contentRect.height()));
    contentItem->setBrush(Qt::NoBrush);
    contentItem->setPen(Qt::NoPen);
    contentItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    contentRect = contentItem->rect();

    QRectF imageRect = contentRect;
    if (imagePath.isEmpty()) {
        imageRect.setHeight(imageRect.width() * 9.0 / 16.0);

        QGraphicsRectItem *emptyImageItem = new QGraphicsRectItem(contentItem);
        emptyImageItem->setBrush(Qt::lightGray);
        emptyImageItem->setPen(Qt::NoPen);
        emptyImageItem->setPos(imageRect.topLeft());
        emptyImageItem->setRect(QRectF(0, 0, imageRect.width(), imageRect.height()));
    } else {
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
    textRect.setTop(imageRect.bottom() + 5);

    QGraphicsTextItem *textItem = new QGraphicsTextItem(contentItem);
    textItem->setFont(normalFont);
    textItem->setDefaultTextColor(captionColor);
    textItem->setPlainText(caption);
    textItem->setPos(textRect.topLeft());

    Qt::Alignment alignment;
    if (captionAlignment == QStringLiteral("center"))
        alignment |= Qt::AlignHCenter;
    else if (captionAlignment == QStringLiteral("left"))
        alignment |= Qt::AlignLeft;
    else if (captionAlignment == QStringLiteral("right"))
        alignment |= Qt::AlignRight;

    QTextCursor cursor(textItem->document());
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QTextBlockFormat format;
    format.setAlignment(alignment);
    cursor.mergeBlockFormat(format);
}

StructureImageAnnotation::~StructureImageAnnotation() { }

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

    if (orientation == QStringLiteral("Vertical"))
        line = QLineF(center.x(), geometry.top(), center.x(), geometry.bottom());
    else
        line = QLineF(geometry.left(), center.y(), geometry.right(), center.y());

    this->setLine(line);

    QPen pen(lineColor, lineWidth);
    this->setPen(pen);
    this->setOpacity(opacity);
}

StructureLineAnnotation::~StructureLineAnnotation() { }

StructureOvalAnnotation::StructureOvalAnnotation(const Annotation *annotation)
{
    const QJsonObject attributes = annotation->attributes();

    const QColor backgroundColor = QColor(attributes.value(QStringLiteral("color")).toString());
    const QColor borderColor = QColor(attributes.value(QStringLiteral("borderColor")).toString());
    const qreal borderWidth = attributes.value(QStringLiteral("borderWidth")).toDouble();
    const bool fillBackground = attributes.value(QStringLiteral("fillBackground")).toBool();
    const qreal opacity =
            qBound(0.0, attributes.value(QStringLiteral("opacity")).toDouble(), 100.0) / 100.0;

    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0, 0, geometry.width(), geometry.height()));

    if (fillBackground)
        this->setBrush(backgroundColor);
    else
        this->setBrush(Qt::NoBrush);
    this->setPen(QPen(borderColor, borderWidth));
    this->setOpacity(opacity);
}

StructureOvalAnnotation::~StructureOvalAnnotation() { }

StructureUnknownAnnotation::StructureUnknownAnnotation(const Annotation *annotation)
{
    const QRectF geometry = annotation->geometry();
    this->setPos(geometry.topLeft());
    this->setRect(QRectF(0, 0, geometry.width(), geometry.height()));

    QBrush brush(Qt::lightGray);
    brush.setStyle(Qt::CrossPattern);

    this->setBrush(brush);
    this->setPen(QPen(Qt::black, 1));
}

StructureUnknownAnnotation::~StructureUnknownAnnotation() { }

///////////////////////////////////////////////////////////////////////////////

StructureTitleCard::StructureTitleCard(const Structure *structure, const QString &comment)
{
    ScriteDocument *document = structure->scriteDocument();
    if (document == nullptr)
        document = ScriteDocument::instance();

    const Screenplay *screenplay = document->screenplay();
    const QSize maxCoverPhotoSize(800, 600);

    /**
      A title card is going to have the following

        Cover Photo
        Title
        Subtitle
        Based On
        Written By
        Studio Details & Address

        Logline

        Structure Grouping Category
      */

    const QString coverPhotoPath = screenplay->coverPagePhoto();
    if (!coverPhotoPath.isEmpty()) {
        QPixmap coverPhotoPixmap(coverPhotoPath);
        QSizeF coverPhotoSize = coverPhotoPixmap.size();
        coverPhotoSize.scale(maxCoverPhotoSize, Qt::KeepAspectRatio);
        coverPhotoPixmap = coverPhotoPixmap.scaled(coverPhotoSize.toSize(), Qt::IgnoreAspectRatio,
                                                   Qt::SmoothTransformation);

        QGraphicsPixmapItem *coverPhotoItem = new QGraphicsPixmapItem(this);
        coverPhotoItem->setPixmap(coverPhotoPixmap);
    }

    auto centerTextInDocument = [](QTextDocument *doc) {
        QTextCursor cursor(doc);
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        QTextBlockFormat format;
        format.setAlignment(Qt::AlignHCenter);
        cursor.mergeBlockFormat(format);
    };

    const QString title = screenplay->title().isEmpty() ? QStringLiteral("Untitled Screenplay")
                                                        : screenplay->title();
    QFont titleFont = ::applicationFont();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);

    QGraphicsTextItem *titleItem = new QGraphicsTextItem(this);
    titleItem->setPlainText(title);
    titleItem->setFont(titleFont);
    titleItem->setTextWidth(maxCoverPhotoSize.width());
    centerTextInDocument(titleItem->document());

    if (!screenplay->subtitle().isEmpty()) {
        QGraphicsTextItem *subtitleItem = new QGraphicsTextItem(this);
        subtitleItem->setPlainText(screenplay->subtitle());

        QFont subtitleFont = ::applicationFont();
        subtitleFont.setPointSize(subtitleFont.pointSize() + 5);
        subtitleItem->setFont(subtitleFont);
        subtitleItem->setTextWidth(maxCoverPhotoSize.width());
        centerTextInDocument(subtitleItem->document());
    }

    if (!screenplay->basedOn().isEmpty()) {
        QGraphicsTextItem *basedOnItem = new QGraphicsTextItem(this);
        basedOnItem->setPlainText(screenplay->basedOn());

        QFont basedOnFont = ::applicationFont();
        basedOnFont.setPointSize(basedOnFont.pointSize() + 5);
        basedOnItem->setFont(basedOnFont);
        basedOnItem->setTextWidth(maxCoverPhotoSize.width());
        centerTextInDocument(basedOnItem->document());
    }

    if (!screenplay->author().isEmpty()) {
        QGraphicsTextItem *authorItem = new QGraphicsTextItem(this);
        authorItem->setPlainText(QStringLiteral("Written by ") + screenplay->author());

        QFont authorFont = ::applicationFont();
        authorFont.setPointSize(authorFont.pointSize() + 5);
        authorItem->setFont(authorFont);
        authorItem->setTextWidth(maxCoverPhotoSize.width());
        centerTextInDocument(authorItem->document());
    }

    if (!screenplay->version().isEmpty()) {
        QGraphicsTextItem *versionItem = new QGraphicsTextItem(this);
        versionItem->setPlainText(screenplay->version());

        QFont versionFont = ::applicationFont();
        versionFont.setPointSize(versionFont.pointSize() + 2);
        versionItem->setFont(versionFont);
        versionItem->setTextWidth(maxCoverPhotoSize.width());
        centerTextInDocument(versionItem->document());
    }

    QTextDocument *contactInfoDoc = new QTextDocument;
    contactInfoDoc->setDefaultFont(::applicationFont());

    QTextCursor cursor(contactInfoDoc);
    if (!screenplay->contact().isEmpty()) {
        cursor.insertText(screenplay->contact());
        cursor.insertBlock();
    }

    if (!screenplay->address().isEmpty()) {
        cursor.insertText(screenplay->address());
        cursor.insertBlock();
    }

    if (!screenplay->phoneNumber().isEmpty()) {
        cursor.insertText(screenplay->phoneNumber());
        cursor.insertBlock();
    }

    if (!screenplay->email().isEmpty()) {
        cursor.insertText(screenplay->email());
        cursor.insertBlock();
    }

    if (!screenplay->website().isEmpty()) {
        cursor.insertText(screenplay->website());
        cursor.insertBlock();
    }

    if (contactInfoDoc->isEmpty())
        delete contactInfoDoc;
    else {
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

        QTextBlockFormat format;
        format.setAlignment(Qt::AlignHCenter);
        cursor.mergeBlockFormat(format);

        QGraphicsTextItem *contactInfoItem = new QGraphicsTextItem(this);
        contactInfoItem->setDocument(contactInfoDoc);
        contactInfoDoc->setParent(contactInfoItem);
    }

    if (!screenplay->logline().isEmpty()) {
        QGraphicsTextItem *loglineItem = new QGraphicsTextItem(this);
        loglineItem->setFont(::applicationFont());

        const QString html = QStringLiteral("<strong>Logline:</strong> ") + screenplay->logline();
        loglineItem->setHtml(html);
        loglineItem->setTextWidth(maxCoverPhotoSize.width());
    }

    if (!comment.isEmpty()) {
        QGraphicsTextItem *commentItem = new QGraphicsTextItem(this);
        commentItem->setFont(::applicationFont());

        const QString html = QStringLiteral("<strong>Comment:</strong> ") + comment;
        commentItem->setHtml(html);
        commentItem->setTextWidth(maxCoverPhotoSize.width());
    }

    QGraphicsSimpleTextItem *groupingInfoItem = new QGraphicsSimpleTextItem(this);
    groupingInfoItem->setFont(::applicationFont());
    if (structure->preferredGroupCategory().isEmpty())
        groupingInfoItem->setText(QStringLiteral("Grouping: Act Wise"));
    else
        groupingInfoItem->setText(QStringLiteral("Grouping: ")
                                  + structure->preferredGroupCategory());

    // Now lets layout all the items in a column
    QRectF boundingRect;
    const QList<QGraphicsItem *> items = this->childItems();
    for (QGraphicsItem *item : items) {
        QRectF itemBoundingRect = item->boundingRect();

        if (!boundingRect.isEmpty())
            item->setPos(boundingRect.bottomLeft() + QPointF(0, 20));

        itemBoundingRect.moveTopLeft(item->pos());
        boundingRect |= itemBoundingRect;
    }

    this->setRect(boundingRect.adjusted(-20, -20, 20, 20));
    this->setBrush(Qt::white);
    this->setPen(QPen(Qt::black, 2));

    // Now lets layout items horizontally to the center
    for (QGraphicsItem *item : items) {
        QRectF itemRect = item->boundingRect();
        itemRect.moveTopLeft(item->pos());
        itemRect.moveCenter(QPointF(boundingRect.center().x(), itemRect.center().y()));
        item->setPos(itemRect.topLeft());
    }
}

StructureTitleCard::~StructureTitleCard() { }
