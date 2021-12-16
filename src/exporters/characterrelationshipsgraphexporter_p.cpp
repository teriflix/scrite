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

#include "application.h"
#include "characterrelationshipsgraphexporter_p.h"

#include <QPainter>
#include <QAbstractTextDocumentLayout>

struct SavePainterState
{
    SavePainterState(QPainter *painter) : m_painter(painter) { m_painter->save(); }
    ~SavePainterState() { m_painter->restore(); }

private:
    QPainter *m_painter;
};
#define SAVE_PAINTER_STATE SavePainterState painterStateSaver(painter);

CharacterRelationshipsGraphScene::CharacterRelationshipsGraphScene(
        const CharacterRelationshipsGraph *graph, QObject *parent)
    : PdfExportableGraphicsScene(parent)
{
    const ObjectListPropertyModel<CharacterRelationshipsGraphNode *> *nodes =
            dynamic_cast<ObjectListPropertyModel<CharacterRelationshipsGraphNode *> *>(
                    graph->nodes());
    const ObjectListPropertyModel<CharacterRelationshipsGraphEdge *> *edges =
            dynamic_cast<ObjectListPropertyModel<CharacterRelationshipsGraphEdge *> *>(
                    graph->edges());

    const int nrNodes = nodes->rowCount(QModelIndex());
    const int nrEdges = edges->rowCount(QModelIndex());

    for (int i = 0; i < nrNodes; i++) {
        const CharacterRelationshipsGraphNode *node = nodes->at(i);
        CharacterRelationshipsGraphNodeItem *nodeItem =
                new CharacterRelationshipsGraphNodeItem(node);
        nodeItem->setZValue(2);
        this->addItem(nodeItem);
    }

    for (int i = 0; i < nrEdges; i++) {
        const CharacterRelationshipsGraphEdge *edge = edges->at(i);
        CharacterRelationshipsGraphEdgeItem *edgeItem =
                new CharacterRelationshipsGraphEdgeItem(edge);
        edgeItem->setZValue(1);
        this->addItem(edgeItem);
    }

    const QRectF brect = this->itemsBoundingRect();

    const QString title = graph->character()->name();
    const QString subtitle = QStringLiteral("Relationships");
    this->setTitle(title + QStringLiteral(" ") + subtitle);

    GraphicsHeaderItem *headerItem = new GraphicsHeaderItem(title, subtitle, brect.width());
    QRectF headerItemRect = headerItem->boundingRect();
    headerItemRect.moveBottomLeft(brect.topLeft());
    headerItem->setPos(headerItemRect.topLeft());
    this->addItem(headerItem);
}

CharacterRelationshipsGraphScene::~CharacterRelationshipsGraphScene() { }

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipsGraphNodeItem::CharacterRelationshipsGraphNodeItem(
        const CharacterRelationshipsGraphNode *node)
    : QGraphicsRectItem(nullptr), m_node(node)
{
    this->setRect(node->rect());
}

CharacterRelationshipsGraphNodeItem::~CharacterRelationshipsGraphNodeItem() { }

void CharacterRelationshipsGraphNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                                                QWidget *)
{
    SAVE_PAINTER_STATE

    const QRectF rect = m_node->rect();

    const Character *character = m_node->character();
    const QColor color = character->color();
    const QStringList photos = character->photos();
    const bool hasPhotos = !photos.isEmpty();

    if (hasPhotos) {
        const qreal margin = qMin(rect.width(), rect.height()) * 0.075;
        const qreal radius = margin * 0.5;
        const QRectF backlight = rect.adjusted(-margin, -margin, margin, margin);

        // While rendering scenes into PDF, transparency is best achieved by
        // setting painter opacity, rather than by setting alpha value in the
        // brush or pen color.
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        painter->setOpacity(0.15);
        painter->drawRoundedRect(backlight, radius, radius, Qt::AbsoluteSize);

        QPen pen;
        pen.setWidth(1);
        pen.setCosmetic(true);
        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->setOpacity(0.5);
        painter->drawRoundedRect(backlight, radius, radius, Qt::AbsoluteSize);

        painter->setOpacity(1.0);
    } else {
        painter->setBrush(Qt::white);
        painter->setPen(Qt::NoPen);
        painter->setOpacity(0.5);
        painter->drawRect(rect);

        painter->setBrush(color);
        painter->drawRect(rect);

        QPen pen;
        pen.setWidth(1);
        pen.setCosmetic(true);
        pen.setColor(Qt::black);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->setOpacity(0.5);
        painter->drawRect(rect);

        painter->setOpacity(1.0);
    }

    QImage image(hasPhotos ? photos.first() : QStringLiteral(":/icons/content/character_icon.png"));
    image = image.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding,
                         Qt::SmoothTransformation);
    painter->drawImage(rect, image);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::lightGray);
    painter->drawRect(rect);

    const qreal textWidth = rect.width() - 30.0;

    QTextDocument document;
    document.setDefaultFont(qApp->font());
    document.setTextWidth(textWidth);
    document.setHtml(
            QStringLiteral("<center><b>%1</b><br/><font size=\"-1\"><i>%2</i></font></center>")
                    .arg(character->name(), character->designation()));

    QRectF documentRect(QPointF(0, 0), document.size());
    documentRect.moveCenter(rect.center());
    documentRect.moveBottom(rect.bottom() - 15.0);

    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->setOpacity(0.5);
    painter->drawRect(documentRect);
    painter->setBrush(color);
    painter->drawRect(documentRect);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    painter->setOpacity(0.5);
    painter->drawRect(documentRect);

    painter->setOpacity(1.0);

    QAbstractTextDocumentLayout *documentLayout = document.documentLayout();
    QAbstractTextDocumentLayout::PaintContext context;
    context.palette.setColor(QPalette::Text, Application::textColorFor(color));
    painter->translate(documentRect.topLeft());
    documentLayout->draw(painter, context);
    painter->translate(documentRect.topLeft());
}

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipsGraphEdgeItem::CharacterRelationshipsGraphEdgeItem(
        const CharacterRelationshipsGraphEdge *edge)
    : QGraphicsPathItem(nullptr)
{
    const QPainterPath path = Application::stringToPainterPath(edge->pathString());
    this->setPath(path);

    QPen pen;
    pen.setWidth(1.0);
    pen.setColor(Qt::black);
    pen.setCosmetic(false);
    this->setPen(pen);

    QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(this);
    label->setText(edge->relationship()->name());
    QRectF labelRect = label->boundingRect();
    labelRect.moveCenter(edge->labelPosition());
    label->setPos(labelRect.topLeft());

    QGraphicsRectItem *labelBackdrop = new QGraphicsRectItem(this);
    labelBackdrop->setZValue(-1);
    labelBackdrop->setRect(labelRect.adjusted(-5, -2, 5, 2));
    labelBackdrop->setBrush(QColor::fromRgbF(0.8, 0.8, 0.8));
    pen.setWidth(1.0);
    labelBackdrop->setPen(pen);
}

CharacterRelationshipsGraphEdgeItem::~CharacterRelationshipsGraphEdgeItem() { }
