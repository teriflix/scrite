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

#include "screenplay.h"
#include "application.h"
#include "scritedocument.h"
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
        const CharacterRelationshipGraph *graph, QObject *parent)
    : PdfExportableGraphicsScene(parent)
{
    const QString title = [graph]() {
        const Screenplay *screenplay = ScriteDocument::instance()->screenplay();
        const QString sptitle = screenplay->title();
        if (graph->character()) {
            const QString chname = Application::camelCased(graph->character()->name());
            return sptitle.isEmpty() ? chname : chname + QStringLiteral(" of ") + sptitle;
        }
        if (graph->scene()) {
            const Scene *scene = graph->scene();
            const QList<int> idxlist = scene->screenplayElementIndexList();
            if (idxlist.isEmpty())
                return QStringLiteral("A scene in ") + sptitle;

            const ScreenplayElement *element = screenplay->elementAt(idxlist.first());
            return QStringLiteral("Scene #") + element->resolvedSceneNumber()
                    + QStringLiteral(" of ") + sptitle;
        }
        return sptitle;
    }();
    const QString subtitle = QStringLiteral("Relationship Graph");
    this->setTitle(title + QStringLiteral(" ") + subtitle);

    const qreal idealContainerWidth = GraphicsHeaderItem::idealContainerWidth(title);

    QGraphicsRectItem *nodesAndEdges = new QGraphicsRectItem;
    nodesAndEdges->setBrush(Qt::NoBrush);
    nodesAndEdges->setPen(Qt::NoPen);
    this->addItem(nodesAndEdges);

    const QObjectListModel<CharacterRelationshipGraphNode *> *nodes =
            dynamic_cast<QObjectListModel<CharacterRelationshipGraphNode *> *>(graph->nodes());
    const QObjectListModel<CharacterRelationshipGraphEdge *> *edges =
            dynamic_cast<QObjectListModel<CharacterRelationshipGraphEdge *> *>(graph->edges());

    const int nrNodes = nodes->rowCount(QModelIndex());
    const int nrEdges = edges->rowCount(QModelIndex());

    for (int i = 0; i < nrNodes; i++) {
        const CharacterRelationshipGraphNode *node = nodes->at(i);
        CharacterRelationshipsGraphNodeItem *nodeItem =
                new CharacterRelationshipsGraphNodeItem(node);
        nodeItem->setZValue(1);
        nodeItem->setParentItem(nodesAndEdges);
    }

    for (int i = 0; i < nrEdges; i++) {
        const CharacterRelationshipGraphEdge *edge = edges->at(i);
        CharacterRelationshipsGraphEdgeItem *edgeItem =
                new CharacterRelationshipsGraphEdgeItem(edge);
        edgeItem->setZValue(2);
        edgeItem->setParentItem(nodesAndEdges);
    }

    const QRectF brect = [nodesAndEdges, nodes]() {
        const QRectF rect = nodesAndEdges->childrenBoundingRect();
        if (nodes->isEmpty())
            return rect;
        const CharacterRelationshipGraphNode *node = nodes->at(0);
        const qreal margin = qMax(node->rect().width(), node->rect().height()) * 0.075;
        return rect.adjusted(-margin, -margin, margin, margin);
    }();
    nodesAndEdges->setRect(brect);

    GraphicsHeaderItem *headerItem =
            new GraphicsHeaderItem(title, subtitle, qMax(brect.width(), idealContainerWidth));
    QRectF headerItemRect = headerItem->boundingRect();
    headerItemRect.moveCenter(brect.center());
    headerItemRect.moveBottom(brect.top());
    headerItem->setPos(headerItemRect.topLeft());
    this->addItem(headerItem);
}

CharacterRelationshipsGraphScene::~CharacterRelationshipsGraphScene() { }

///////////////////////////////////////////////////////////////////////////////

CharacterRelationshipsGraphNodeItem::CharacterRelationshipsGraphNodeItem(
        const CharacterRelationshipGraphNode *node)
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

    QImage image(hasPhotos ? character->keyPhoto()
                           : QStringLiteral(":/icons/content/character_icon.png"));
    image = image.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding,
                         Qt::SmoothTransformation);

    auto imageSourceRect = [=]() -> QRectF {
        QSizeF imageSize = image.size();
        imageSize.scale(rect.size(), Qt::IgnoreAspectRatio);

        QRectF imageRect(QPointF(0, 0), imageSize);
        imageRect.moveCenter(image.rect().center());
        return imageRect;
    };

    painter->drawImage(rect, image, imageSourceRect());

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
        const CharacterRelationshipGraphEdge *edge)
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
