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

#include "statisticsreport_p.h"
#include "statisticsreport.h"

#include "application.h"
#include "screenplaytextdocument.h"

#include <QPen>
#include <QBrush>
#include <QChart>
#include <QBarSet>
#include <QPieSlice>
#include <QPieSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QStackedBarSeries>
#include <QGraphicsBlurEffect>

/**
 * Note from Prashanth:
 * The code in this file (and its header) could be cleaner. I do hope
 * to get around to cleaning it up sometime. I wrote this quickly, just
 * to get this stats report feature out for the 0.8 release. It works, but
 * not as modular and maintanable as I would like it to be.
 */

template<class Key, class Value>
class SequentialMap
{
public:
    SequentialMap() { }
    ~SequentialMap() { }

    void insert(const Key &key, const Value &value)
    {
        int index = this->indexOf(key);
        if (index < 0)
            vector.append(qMakePair(key, value));
        else
            vector[index].second = value;
    }

    Value &value(const Key &key)
    {
        int index = this->indexOf(key);
        if (index < 0) {
            vector.append(qMakePair(key, Value()));
            return vector.last().second;
        }
        return vector[index].second;
    }

    Value &operator[](const Key &key) { return this->value(key); }

    QList<Key> keys() const
    {
        QList<Key> ret;
        for (int i = 0; i < vector.size(); i++)
            ret.append(vector.at(i).first);
        return ret;
    }

    int size() const { return vector.size(); }

    int indexOf(const Key &key) const
    {
        for (int i = 0; i < vector.size(); i++)
            if (vector.at(i).first == key)
                return i;
        return -1;
    }

    void clear() { vector.clear(); }

    void sort(std::function<bool(const QPair<Key, Value> &, const QPair<Key, Value> &)> sortFunc)
    {
        std::sort(vector.begin(), vector.end(), sortFunc);
    }

    QVector<QPair<Key, Value>> vector;
};

inline static QString timeToString(const QTime &time, bool inclSecs = false)
{
    QStringList timeComps;
    if (time == QTime(0, 0, 0, 0))
        timeComps << QStringLiteral("0m");
    else {
        if (time.hour() > 0)
            timeComps << QString::number(time.hour()) + QStringLiteral("h");
        if (time.minute() > 0)
            timeComps << QString::number(time.minute()) + QStringLiteral("m");
        if ((inclSecs || timeComps.isEmpty()) && time.second() > 0)
            timeComps << QString::number(time.second()) + QStringLiteral("s");
    }
    return timeComps.isEmpty() ? QStringLiteral("0m") : timeComps.join(QStringLiteral(" "));
}

StatisticsReportPage::StatisticsReportPage(StatisticsReport *report)
    : PdfExportableGraphicsScene(report)
{
    auto ir = [](const QGraphicsItem *item) {
        return item->mapToScene(item->boundingRect()).boundingRect();
    };

    StatisticsReportKeyNumbers *keyNumbers = new StatisticsReportKeyNumbers(report);
    keyNumbers->setPos(0, 0);
    this->addItem(keyNumbers);

    StatisticsReportDialogueActionRatio *dialogueActionPieChart =
            new StatisticsReportDialogueActionRatio(report);
    dialogueActionPieChart->setPos(ir(keyNumbers).bottomLeft() + QPointF(0, 20));
    this->addItem(dialogueActionPieChart);

    StatisticsReportSceneHeadingStats *headingStats = new StatisticsReportSceneHeadingStats(report);
    headingStats->setPos(ir(dialogueActionPieChart).topRight() + QPointF(20, 0));
    this->addItem(headingStats);

    QRectF brect = this->itemsBoundingRect();

    StatisticsReportTimeline *actLengths = new StatisticsReportTimeline(brect.width(), report);
    actLengths->setPos(brect.bottomLeft());
    this->addItem(actLengths);

    const QString title = report->document()->screenplay()->title();
    const QString subtitle = QStringLiteral("Key Statistics");

    GraphicsHeaderItem *headerItem = new GraphicsHeaderItem(title, subtitle, brect.width());
    QRectF headerItemRect = headerItem->boundingRect();
    headerItemRect.moveBottomLeft(brect.topLeft());
    headerItem->setPos(headerItemRect.topLeft());
    this->addItem(headerItem);

    brect = this->itemsBoundingRect().adjusted(0, 0, 0, 20);

    QGraphicsLineItem *footerSeparator = new QGraphicsLineItem;
    footerSeparator->setLine(QLineF(brect.bottomLeft(), brect.bottomRight()));
    footerSeparator->setPen(QPen(Qt::gray));
    this->addItem(footerSeparator);
}

StatisticsReportPage::~StatisticsReportPage() { }

////////////////////////////////////////////////////////////////////////////////////

StatisticsReportKeyNumbers::StatisticsReportKeyNumbers(const StatisticsReport *report,
                                                       QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    const Screenplay *screenplay = report->document()->screenplay();
    const Structure *structure = report->document()->structure();

    auto sceneElements = screenplay->getFilteredElements([](ScreenplayElement *e) {
        return e->scene() != nullptr && e->scene()->heading()->isEnabled();
    });

    const int nrDialogues = screenplay->dialogueCount();
    const qreal avgScenePxLength = report->totalPixelLength() / sceneElements.size();
    const QTime avgSceneTime = report->pixelLengthToTime(avgScenePxLength);

    const QStringList statLabels = {
        QStringLiteral("Est. Runtime: ") + ::timeToString(report->estimatedTime()),
        QString::number(sceneElements.size()) + QStringLiteral(" Scenes"),
        QString::number(structure->characterNames().size()) + QStringLiteral(" Characters"),
        QString::number(report->pageCount()) + QStringLiteral(" Pages"),
        QString::number(nrDialogues) + QStringLiteral(" Dialogues"),
        QString::number(structure->allLocations().size()) + QStringLiteral(" Locations"),
        QStringLiteral("Avg. Scene Time: ") + ::timeToString(avgSceneTime, true)
    };

    const QColor bgColor = StatisticsReport::pickColor(0);
    const QColor textColor = Application::textColorFor(bgColor);

    QPointF pos(0, 0);

    for (const QString &statLabel : statLabels) {
        QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem;
        label->setFont(Application::font());
        label->setBrush(textColor);
        label->setText(statLabel);

        QRectF labelRect = label->boundingRect();

        QGraphicsRectItem *labelBg = new QGraphicsRectItem(this);
        labelBg->setBrush(bgColor);
        labelBg->setPen(Qt::NoPen);
        labelBg->setRect(0, 0, labelRect.width() + 20, labelRect.height() + 8);
        label->setParentItem(labelBg);

        labelRect.moveCenter(labelBg->rect().center());
        label->setPos(labelRect.topLeft());

        labelBg->setPos(pos);
        pos.setX(pos.x() + labelBg->rect().width() + 10);
    }

    QRectF myRect = this->childrenBoundingRect();
    this->setRect(myRect);
    this->setBrush(Qt::NoBrush);
    this->setPen(Qt::NoPen);
}

StatisticsReportKeyNumbers::~StatisticsReportKeyNumbers() { }

////////////////////////////////////////////////////////////////////////////////////

StatisticsReportTimeline::StatisticsReportTimeline(qreal suggestedWidth,
                                                   const StatisticsReport *report,
                                                   QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    this->setRect(0, 0, suggestedWidth, 1300);
    this->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    this->setPen(Qt::NoPen);
    this->setBrush(Qt::NoBrush);

    QGraphicsRectItem *container = new QGraphicsRectItem(this);
    container->setRect(0, 0, this->rect().width() - 40, 10); // some initial rectangle size
    container->setBrush(Qt::NoBrush);
    container->setPen(Qt::NoPen);

    QList<QPair<QList<StatisticsReport::Distribution>, qreal>> dists;
    dists.append(qMakePair(report->episodeDistribution(), 40.0));
    dists.append(qMakePair(report->actDistribution(), 40.0));
    dists.append(qMakePair(report->sceneDistribution(), 40.0));

    const qreal totalPixelLength = [=]() {
        qreal ret = 0;
        for (const auto &distsItem : qAsConst(dists))
            ret = qMax(evalPixelLength(distsItem.first), ret);
        return ret;
    }();

    QGraphicsRectItem *prevItem = this->createTimelineItem(report, container);
    for (int i = 0; i < dists.size(); i++) {
        const auto &distsItem = dists[i];
        auto dist = distsItem.first;
        if (dist.isEmpty())
            continue;

        QGraphicsRectItem *distsRect = this->createDistributionItems(
                container, dist, distsItem.second, totalPixelLength, i == dists.size() - 1);
        if (distsRect) {
            distsRect->setPos(prevItem->pos() + prevItem->boundingRect().bottomLeft()
                              + QPointF(0, 10));
            prevItem = distsRect;
        }
    }

    QGraphicsRectItem *sceneItemsContainer = prevItem;
    QGraphicsRectItem *tracksItem =
            this->createScreenplayTracks(report, container, 40, totalPixelLength);
    if (tracksItem != nullptr) {
        tracksItem->setPos(sceneItemsContainer->pos());
        sceneItemsContainer->setPos(tracksItem->pos() + tracksItem->boundingRect().bottomLeft()
                                    + QPointF(0, 10));
    }

    this->createScenePullouts(report, container, sceneItemsContainer);
    if (report->isIncludeCharacterPresenceGraphs()) {
        this->createSeparator(container, QStringLiteral("Character Presence"),
                              StatisticsReport::pickColor(0));
        this->createCharacterPresenceGraph(report, container, sceneItemsContainer);
    }

    if (report->isIncludeLocationPresenceGraphs()) {
        this->createSeparator(container, QStringLiteral("Location Presence"),
                              StatisticsReport::pickColor(0, false, StatisticsReport::Location));
        this->createLocationPresenceGraph(report, container, sceneItemsContainer);
    }

    QRectF containerItemsRect = container->childrenBoundingRect();
    containerItemsRect.moveTopLeft(QPointF(0, 0));

    QRectF thisItemRect = containerItemsRect.adjusted(0, 0, 40, 40);
    thisItemRect.setWidth(qMax(this->rect().width(), thisItemRect.width()));
    this->setRect(thisItemRect);

    container->setRect(containerItemsRect);
    containerItemsRect.moveCenter(thisItemRect.center());
    containerItemsRect.moveTop(20);
    container->setPos(containerItemsRect.topLeft());
}

StatisticsReportTimeline::~StatisticsReportTimeline() { }

QGraphicsRectItem *StatisticsReportTimeline::createTimelineItem(const StatisticsReport *report,
                                                                QGraphicsRectItem *container) const
{
    const qreal containerWidth = container->boundingRect().width();
    const qreal totalMinutes = (QTime(0, 0, 0, 0).msecsTo(report->estimatedTime()) / 1000.0) / 60.0;
    const qreal minuteScaleFactor = containerWidth / totalMinutes;

    // time-scale
    const qreal timelineHeight = 20;
    QGraphicsRectItem *timeline = new QGraphicsRectItem(container);
    timeline->setBrush(Qt::NoBrush);
    timeline->setPen(Qt::NoPen);
    timeline->setRect(0, 0, containerWidth, timelineHeight);

    QGraphicsLineItem *hline = new QGraphicsLineItem(timeline);
    hline->setLine(0, timelineHeight / 2, containerWidth, timelineHeight / 2);
    hline->setPen(QPen(Qt::gray));

    auto createTimelineTick = [timelineHeight](const QTime &time, bool inclSecs = false) {
        QGraphicsLineItem *tick = new QGraphicsLineItem;
        tick->setLine(0, 0, 0, timelineHeight);

        QGraphicsSimpleTextItem *text = new QGraphicsSimpleTextItem(tick);
        text->setText(::timeToString(time, inclSecs));

        QRectF textRect = text->boundingRect();
        textRect.moveCenter(QPointF(0, 0));
        textRect.moveBottom(0);
        text->setPos(textRect.topLeft());

        return tick;
    };

    QGraphicsItem *firstTick = createTimelineTick(QTime(0, 0, 0, 0));
    firstTick->setParentItem(timeline);
    firstTick->setPos(0, 0);

    QGraphicsItem *lastTick = createTimelineTick(report->estimatedTime());
    lastTick->setParentItem(timeline);
    lastTick->setPos(containerWidth, 0);

    const qreal minuteStep = qCeil((totalMinutes / 8) / 5.0) * 5.0;
    qreal min = minuteStep;
    while (min < totalMinutes - minuteStep / 2) {
        QTime time(0, 0, 0, 0);
        time = time.addSecs(int(min * 60));

        QGraphicsItem *middleTick = createTimelineTick(time);
        middleTick->setParentItem(timeline);
        middleTick->setPos(min * minuteScaleFactor, 0);

        min += minuteStep;
    }

    return timeline;
}

QGraphicsRectItem *StatisticsReportTimeline::createDistributionItems(
        QGraphicsRectItem *container, const QList<StatisticsReport::Distribution> &dist,
        qreal heightHint, qreal totalPixelLength, bool lightenFillColors) const
{
    const qreal containerWidth = container->boundingRect().width();
    const qreal pixelScaleFactor = containerWidth / totalPixelLength;

    if (dist.isEmpty())
        return nullptr;

    QGraphicsRectItem *itemsLayout = new QGraphicsRectItem(container);
    itemsLayout->setRect(0, 0, containerWidth, heightHint);
    itemsLayout->setBrush(Qt::NoBrush);
    itemsLayout->setPen(Qt::NoPen);

    QRectF rect(0, 0, 0, heightHint);
    for (int i = 0; i < dist.length(); i++) {
        const StatisticsReport::Distribution &distItem = dist[i];

        rect.moveTopLeft(rect.topRight());
        rect.setWidth(distItem.pixelLength * pixelScaleFactor);

        const QColor color =
                distItem.color == Qt::transparent ? StatisticsReport::pickColor(i) : distItem.color;
        QPen pen;
        pen.setColor(Application::isLightColor(color) ? Qt::gray : color);
        pen.setCosmetic(true);
        pen.setWidthF(0.5);
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setCapStyle(Qt::FlatCap);

        QBrush brush(color);

        QGraphicsRectItem *item = new QGraphicsRectItem(itemsLayout);
        item->setPos(rect.topLeft());
        item->setRect(QRectF(0, 0, rect.width(), rect.height()));
        item->setPen(Qt::NoPen);
        if (lightenFillColors)
            item->setBrush(distItem.color == Qt::transparent ? brush : Qt::NoBrush);
        else
            item->setBrush(color);

        if (lightenFillColors && distItem.color != Qt::transparent) {
            QGraphicsRectItem *bgItem = new QGraphicsRectItem(item);
            bgItem->setRect(item->boundingRect());
            bgItem->setBrush(brush);
            bgItem->setPen(Qt::NoPen);
            bgItem->setOpacity(0.35);

            const QRectF itemRect = item->boundingRect();
            QGraphicsLineItem *line = new QGraphicsLineItem(item);
            line->setLine(QLineF(itemRect.topRight(), itemRect.bottomRight()));
            line->setPen(pen);
        }

        const QString timeString = ::timeToString(distItem.timeLength);
        const QString labelHtml =
                QStringLiteral("<center><b>%1</b><br/><font size=\"-2\">%2 - %3</font></center>")
                        .arg(distItem.key, timeString, distItem.percent);

        QGraphicsTextItem *label = new QGraphicsTextItem(item);
        label->setHtml(labelHtml);
        if (distItem.color == Qt::transparent || !lightenFillColors)
            label->setDefaultTextColor(Application::textColorFor(item->brush().color()));

        QRectF labelRect = label->boundingRect();
        if (labelRect.width() > item->rect().width()) {
            const QString labelHtml2 =
                    QStringLiteral("<center><b>%1</b><br/><font size=\"-2\">%2</font></center>")
                            .arg(distItem.key, timeString);
            label->setHtml(labelHtml2);
            labelRect = label->boundingRect();
        }
        if (labelRect.width() > item->rect().width()) {
            const QString labelHtml3 =
                    QStringLiteral("<center><b>%1</b><br/><font size=\"-2\">%2</font></center>")
                            .arg(distItem.key, distItem.percent);
            label->setHtml(labelHtml3);
            labelRect = label->boundingRect();
        }
        if (labelRect.width() > item->rect().width()) {
            label->setPlainText(distItem.key);
            labelRect = label->boundingRect();
        }
        if (labelRect.width() > item->rect().width())
            label->setVisible(false);
        else {
            labelRect.moveCenter(item->boundingRect().center());
            label->setPos(labelRect.topLeft());
        }
    }

    return itemsLayout;
}

QGraphicsRectItem *StatisticsReportTimeline::createScreenplayTracks(const StatisticsReport *report,
                                                                    QGraphicsRectItem *container,
                                                                    qreal heightHint,
                                                                    qreal totalPixelLength)
{
    // We are not using ScreenplayTracks here, because
    // ScreenplayTracks is meant to be used only as a model with the timeline.
    // The data that it gives is not sufficient for use with this report.
    struct BeatTrack
    {
        QString tag;
        struct
        {
            qreal from = 0;
            qreal to = 0;
        } pixel;
        QTime timeLength;
        int sceneIndex = -1;
    };
    QList<BeatTrack> overlappingTracks;

    const Screenplay *screenplay = report->document()->screenplay();
    const QList<ScreenplayElement *> sceneElements = screenplay->getFilteredElements(
            [](ScreenplayElement *e) { return e->scene() != nullptr; });

    qreal currentPixel = 0;
    int currentSceneIndex = -1;

    auto addBeatTrack = [&](const QString &tag, qreal pixelLength) -> BeatTrack & {
        for (int i = overlappingTracks.size() - 1; i >= 0; i--) {
            BeatTrack &track = overlappingTracks[i];
            if (track.tag == tag && qAbs(track.sceneIndex - currentSceneIndex) == 1) {
                track.sceneIndex = currentSceneIndex;
                track.pixel.to += pixelLength;
                track.timeLength = report->pixelLengthToTime(track.pixel.to - track.pixel.from);
                return track;
            }
        }

        BeatTrack track;
        track.tag = tag;
        track.sceneIndex = currentSceneIndex;
        track.pixel.from = currentPixel;
        track.pixel.to = currentPixel + pixelLength;
        track.timeLength = report->pixelLengthToTime(track.pixel.to - track.pixel.from);
        overlappingTracks.append(track);
        return overlappingTracks.last();
    };

    const QString activeGroupPrefix =
            report->document()->structure()->preferredGroupCategory() + QLatin1String("/");
    for (auto sceneElement : sceneElements) {
        ++currentSceneIndex;
        const Scene *scene = sceneElement->scene();
        const qreal pixelLength = report->pixelLength(sceneElement);

        const QStringList tags = scene->groups();
        for (const QString &tag : tags) {
            if (!report->isConsiderPreferredGroupCategoryOnly()
                || tag.startsWith(activeGroupPrefix, Qt::CaseInsensitive))
                addBeatTrack(tag, pixelLength);
        }

        currentPixel += pixelLength;
    }

    if (overlappingTracks.isEmpty())
        return nullptr;

    QList<QList<BeatTrack>> distinctTracks;
    distinctTracks << QList<BeatTrack>();

    for (const auto &track : qAsConst(overlappingTracks)) {
        bool trackAdded = false;

        for (auto &tracks : distinctTracks) {
            bool trackOverlaps = false;
            for (auto &item : tracks) {
                if ((track.pixel.from >= item.pixel.from && track.pixel.from < item.pixel.to)
                    || (track.pixel.to >= item.pixel.from && track.pixel.to < item.pixel.to)) {
                    trackOverlaps = true;
                    break;
                }
            }

            if (!trackOverlaps) {
                tracks.append(track);
                trackAdded = true;
                break;
            }
        }

        if (!trackAdded) {
            QList<BeatTrack> newTrack;
            newTrack << track;
            distinctTracks.append(newTrack);
        }
    }

    const QPointF tracksContainerPos = container->childrenBoundingRect().bottomLeft();
    const qreal containerWidth = container->boundingRect().width();

    QGraphicsRectItem *tracksContainer = new QGraphicsRectItem(container);
    tracksContainer->setBrush(Qt::NoBrush);
    tracksContainer->setPen(Qt::NoPen);
    tracksContainer->setPos(tracksContainerPos);
    tracksContainer->setRect(QRectF(0, 0, containerWidth, heightHint * distinctTracks.size()));

    QFont smallFont = Application::font();
    smallFont.setPointSize(8);

    QList<QStringList> numberedBeats;

    int trackNr = 0;
    qreal trackItemY = 0;
    for (const auto &tracks : qAsConst(distinctTracks)) {
        bool firstTrack = true;
        for (const auto &track : tracks) {
            QGraphicsRectItem *trackItem = new QGraphicsRectItem(tracksContainer);
            trackItem->setBrush(
                    StatisticsReport::pickColor(trackNr++, false, StatisticsReport::Beat));
            trackItem->setPen(Qt::NoPen);
            trackItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

            QRectF rect(0, trackItemY, 0, heightHint);
            rect.setLeft(containerWidth * (track.pixel.from / totalPixelLength));
            rect.setRight(containerWidth * (track.pixel.to / totalPixelLength));
            trackItem->setRect(QRectF(0, 0, rect.width(), rect.height()));
            trackItem->setPos(rect.topLeft());

            QPainterPath trackBorderPath;
            trackBorderPath.moveTo(trackItem->boundingRect().topLeft());
            trackBorderPath.lineTo(trackItem->boundingRect().topRight());
            trackBorderPath.lineTo(trackItem->boundingRect().bottomRight());
            trackBorderPath.lineTo(trackItem->boundingRect().bottomLeft());
            if (firstTrack)
                trackBorderPath.closeSubpath();

            QGraphicsPathItem *trackBorderItem = new QGraphicsPathItem(trackItem);
            trackBorderItem->setPath(trackBorderPath);
            trackBorderItem->setBrush(Qt::NoBrush);
            trackBorderItem->setPen(QPen(trackItem->brush().color().darker(), 1, Qt::SolidLine,
                                         Qt::RoundCap, Qt::MiterJoin));

            const QString text = Application::camelCased(
                    track.tag.section(QStringLiteral("/"), 1).section(QStringLiteral("("), 0, 0));
            const QString timeString = ::timeToString(track.timeLength);
            const QString percent = QString::number(qRound(100 * (track.pixel.to - track.pixel.from)
                                                           / totalPixelLength))
                    + QStringLiteral("%");
            const QString labelHtml =
                    QStringLiteral(
                            "<center><b>%1</b><br/><font size=\"-2\">%2 - %3</font></center>")
                            .arg(text, timeString, percent);

            QGraphicsTextItem *label = new QGraphicsTextItem(trackItem);
            label->setHtml(labelHtml);
            label->setDefaultTextColor(Application::textColorFor(trackItem->brush().color()));

            QRectF labelRect = label->boundingRect();
            if (labelRect.width() > trackItem->rect().width()) {
                const QString labelHtml2 =
                        QStringLiteral("<center><b>%1</b><br/><font size=\"-2\">%2</font></center>")
                                .arg(text, timeString);
                label->setHtml(labelHtml2);
                labelRect = label->boundingRect();
            }
            if (labelRect.width() > trackItem->rect().width()) {
                const QString labelHtml3 =
                        QStringLiteral("<center><b>%1</b><br/><font size=\"-2\">%2</font></center>")
                                .arg(text, percent);
                label->setHtml(labelHtml3);
                labelRect = label->boundingRect();
            }
            if (labelRect.width() > trackItem->rect().width()) {
                label->setPlainText(text);
                labelRect = label->boundingRect();
            }

            if (labelRect.width() > trackItem->rect().width()) {
                label->setFont(smallFont);
                label->setPlainText(QString::number(trackNr));
                labelRect = label->boundingRect();

                numberedBeats << QStringList({ QString::number(trackNr) + QStringLiteral(": "),
                                               text, timeString, percent });
            }

            if (labelRect.width() > trackItem->rect().width())
                label->setVisible(false);
            else {
                labelRect.moveCenter(trackItem->boundingRect().center());
                label->setPos(labelRect.topLeft());
            }

            firstTrack = false;
        }

        trackItemY += heightHint;
    }

    if (numberedBeats.isEmpty())
        return tracksContainer;

    const QFontMetricsF fontMetrics(Application::font());

    QVector<qreal> colWidths({ 0, 0, 0, 0 });
    for (const auto &row : qAsConst(numberedBeats)) {
        for (int i = 0; i < row.size(); i++)
            colWidths[i] =
                    qMax(colWidths[i],
                         fontMetrics.horizontalAdvance(row[i]) + fontMetrics.averageCharWidth());
    }

    qreal tableWidth = std::accumulate(colWidths.begin(), colWidths.end(), 0.0);
    const int nrTables = qMax(1, qFloor(containerWidth / tableWidth) - 1);
    const qreal tableXStep = containerWidth / nrTables;

    int tableIndex = 0;
    qreal tableRowY = (tracksContainer->boundingRect().bottomLeft() + QPointF(0, 10)).y();

    for (const auto &row : qAsConst(numberedBeats)) {
        qreal tableRowX = tableIndex * tableXStep + (tableXStep - tableWidth) / 2;
        for (int i = 0; i < row.size(); i++) {
            const QString text = row.at(i);
            const QRectF tableCellRect(tableRowX, tableRowY, colWidths[i],
                                       fontMetrics.lineSpacing());

            QGraphicsSimpleTextItem *cellText = new QGraphicsSimpleTextItem(tracksContainer);
            cellText->setFont(Application::font());
            cellText->setText(text);

            QRectF cellTextRect = cellText->boundingRect();
            cellTextRect.moveCenter(tableCellRect.center());
            if (i)
                cellTextRect.moveLeft(tableCellRect.left());
            else
                cellTextRect.moveRight(tableCellRect.right());
            cellText->setPos(cellTextRect.topLeft());

            tableRowX += tableCellRect.width();
        }

        tableIndex = (tableIndex + 1) % nrTables;
        if (tableIndex == 0)
            tableRowY += fontMetrics.lineSpacing();
    }

    tableRowY += fontMetrics.lineSpacing();

    for (int i = 1; i < nrTables; i++) {
        const qreal lineY1 = (tracksContainer->boundingRect().bottomLeft() + QPointF(0, 10)).y();
        const qreal lineY2 = tableRowY;
        const qreal lineX = i * tableXStep;

        QGraphicsLineItem *lineItem = new QGraphicsLineItem(tracksContainer);
        lineItem->setLine(lineX, lineY1, lineX, lineY2);

        QPen pen;
        pen.setColor(Qt::black);
        pen.setStyle(Qt::DashDotDotLine);
        lineItem->setPen(pen);
    }

    QRectF tracksContainerRect = tracksContainer->rect();
    tracksContainerRect.setHeight(tableRowY);
    tracksContainer->setRect(tracksContainerRect);

    return tracksContainer;
}

qreal StatisticsReportTimeline::evalPixelLength(
        const QList<StatisticsReport::Distribution> &dist) const
{
    qreal ret = 0;
    for (const auto &distItem : dist)
        ret += distItem.pixelLength;

    return ret;
}

QGraphicsRectItem *
StatisticsReportTimeline::createScenePullouts(const StatisticsReport *report,
                                              QGraphicsRectItem *container,
                                              QGraphicsRectItem *sceneItemsContainer) const
{
    const Screenplay *screenplay = report->document()->screenplay();
    const QList<ScreenplayElement *> sceneElements = screenplay->getFilteredElements(
            [](ScreenplayElement *e) { return e->scene() != nullptr; });

    struct SceneInfo
    {
        int index = -1;
        QColor color;
        QString sceneNumber;
        qreal pixelLength = 0;
        qreal pageLength = 0;
        int nrCharacters = 0;
        QTime timeLength;
        QString label;

        QString makeLabel(const QString &prefix) const
        {
            return prefix + QStringLiteral(": ") + sceneNumber + QStringLiteral(", ")
                    + QString::number(nrCharacters) + QStringLiteral(" Characters, ")
                    + QString::number(this->pageLength, 'g', 1) + QStringLiteral(" Pages, ")
                    + ::timeToString(timeLength);
        }
    };

    int sceneIndex = -1;
    QVector<SceneInfo> sceneInfos;
    for (int i = 0; i < sceneElements.size(); i++) {
        const Scene *scene = sceneElements.at(i)->scene();
        if (scene->heading()->isEnabled() || sceneInfos.isEmpty())
            sceneInfos.append(SceneInfo());

        SceneInfo &info = sceneInfos.last();
        if (info.index < 0) {
            info.index = ++sceneIndex;
            info.sceneNumber = sceneElements.at(i)->resolvedSceneNumber();
            info.color = scene->color();
        }
        info.pixelLength += report->pixelLength(scene);
        info.nrCharacters += scene->characterNames().size();
        info.timeLength = report->pixelLengthToTime(info.pixelLength);
        info.pageLength = report->pageLength(info.pixelLength);
    }

    // Longest & Smallest Scene
    std::sort(sceneInfos.begin(), sceneInfos.end(),
              [](const SceneInfo &a, const SceneInfo &b) { return a.pixelLength < b.pixelLength; });

    SceneInfo shortestScene = sceneInfos.first();
    shortestScene.label = shortestScene.makeLabel(QStringLiteral("Shortest Scene"));

    SceneInfo longestScene = sceneInfos.last();
    longestScene.label = longestScene.makeLabel(QStringLiteral("Longest Scene"));

    // Most & Least Crowded Scene
    for (int i = sceneInfos.size() - 1; i >= 0; i--) {
        if (sceneInfos.at(i).nrCharacters == 0)
            sceneInfos.removeAt(i);
    }
    std::sort(sceneInfos.begin(), sceneInfos.end(), [](const SceneInfo &a, const SceneInfo &b) {
        if (a.nrCharacters == b.nrCharacters)
            return a.pixelLength / a.nrCharacters > b.pixelLength / b.nrCharacters;
        return a.nrCharacters < b.nrCharacters;
    });

    SceneInfo leastCrowdedScene = sceneInfos.isEmpty() ? SceneInfo() : sceneInfos.first();
    leastCrowdedScene.label = leastCrowdedScene.makeLabel(QStringLiteral("Least Crowded Scene"));

    SceneInfo mostCrowdedScene = sceneInfos.isEmpty() ? SceneInfo() : sceneInfos.last();
    mostCrowdedScene.label = mostCrowdedScene.makeLabel(QStringLiteral("Most Crowded Scene"));

    // Now prepare to render these labels from left-to-right
    sceneInfos = QVector<SceneInfo>(
            { shortestScene, longestScene, mostCrowdedScene, leastCrowdedScene });
    for (int i = sceneInfos.size() - 1; i >= 0; i--) {
        const SceneInfo &sceneInfo = sceneInfos[i];
        if (sceneInfo.index < 0)
            sceneInfos.removeAt(i);
    }
    std::sort(sceneInfos.begin(), sceneInfos.end(),
              [](const SceneInfo &a, const SceneInfo &b) { return a.index < b.index; });

    int letter = 0;
    QMap<int, QStringList> labelMap;
    for (const auto &sceneInfo : qAsConst(sceneInfos))
        labelMap[sceneInfo.index] << QString::number(++letter);

    QFont font = Application::font();

    // Container for labels.
    QGraphicsRectItem *labelsItem = new QGraphicsRectItem(container);
    labelsItem->setPen(Qt::NoPen);
    labelsItem->setBrush(Qt::NoBrush);
    labelsItem->setPos(sceneItemsContainer->boundingRect().bottomLeft()
                       + sceneItemsContainer->pos());
    labelsItem->setRect(QRectF(0, 0, sceneItemsContainer->boundingRect().width(), 10));

    // Create the labels themselves.
    for (int i = 0; i < sceneInfos.size(); i++) {
        const SceneInfo &sceneInfo = sceneInfos[i];
        if (sceneInfo.index < 0)
            continue;

        const QStringList labelText = labelMap.take(sceneInfo.index);
        if (labelText.isEmpty())
            continue;

        const QGraphicsItem *sceneItem = sceneItemsContainer->childItems().at(sceneInfo.index);
        const QRectF sceneItemRect =
                labelsItem->mapFromItem(sceneItem, sceneItem->boundingRect()).boundingRect();
        const QPointF p1(sceneItemRect.center().x(), sceneItemRect.bottom());
        const QPointF p2 = p1 + QPointF(0, 30);

        QGraphicsLineItem *lineItem = new QGraphicsLineItem(labelsItem);
        lineItem->setLine(QLineF(p1, p2));
        lineItem->setPen(QPen(Application::isLightColor(sceneInfo.color) ? sceneInfo.color.darker()
                                                                         : sceneInfo.color));

        QGraphicsSimpleTextItem *labelItem = new QGraphicsSimpleTextItem(lineItem);
        labelItem->setText(labelText.join(QStringLiteral(", ")));
        labelItem->setFont(font);

        QRectF labelItemRect = labelItem->boundingRect();
        labelItemRect.moveCenter(p2);
        labelItemRect.moveTop(p2.y() + 5);
        labelItem->setPos(labelItemRect.topLeft());

        QGraphicsPathItem *pathItem = new QGraphicsPathItem(labelItem);

        const qreal size =
                8 + qMax(labelItem->boundingRect().width(), labelItem->boundingRect().height());
        QRectF pathRect(0, 0, size, size);
        pathRect.moveCenter(labelItem->boundingRect().center());

        QPainterPath path;
        path.addEllipse(pathRect);
        pathItem->setPath(path);
        pathItem->setPen(QPen(Qt::black));
        pathItem->setBrush(sceneInfo.color);
        pathItem->setFlag(QGraphicsItem::ItemStacksBehindParent);
        pathItem->setOpacity(0.5);
    }

    const qreal cellWidth = labelsItem->rect().width() / sceneInfos.size();
    const qreal cellY = labelsItem->childrenBoundingRect().bottom() + 20;

    for (int i = 0; i < sceneInfos.size(); i++) {
        const SceneInfo &sceneInfo = sceneInfos[i];
        if (sceneInfo.index < 0)
            continue;

        const QRectF cellRect(i * cellWidth, cellY, cellWidth, 30);

        QGraphicsTextItem *textItem = new QGraphicsTextItem(labelsItem);
        textItem->setTextWidth(cellWidth - 10);
        textItem->setFont(font);
        textItem->setHtml(QStringLiteral("<center>") + QString::number(i + 1) + QStringLiteral(": ")
                          + sceneInfo.label + QStringLiteral("</center>"));

        QRectF textItemRect = textItem->boundingRect();
        textItemRect.moveCenter(cellRect.center());
        textItemRect.moveTop(cellRect.top());
        textItem->setPos(textItemRect.topLeft());

        if (i < sceneInfos.size() - 1) {
            QGraphicsLineItem *lineItem = new QGraphicsLineItem(labelsItem);
            lineItem->setLine(QLineF(cellRect.topRight(), cellRect.bottomRight()));

            QPen pen;
            pen.setColor(Qt::black);
            pen.setStyle(Qt::DashDotDotLine);
            lineItem->setPen(pen);
        }
    }

    QRectF labelsItemRect = labelsItem->rect();
    labelsItemRect.setBottom(labelsItem->childrenBoundingRect().bottom());
    labelsItem->setRect(labelsItemRect);

    return labelsItem;
}

QList<QPair<QString, QList<int>>>
StatisticsReportTimeline::evalCharacterPresence(const StatisticsReport *report) const
{
    const Structure *structure = report->document()->structure();
    const QStringList allCharacterNames = structure->characterNames();
    const QStringList specificCharacterNames = report->characterNames();
    const QStringList characterNames =
            specificCharacterNames.isEmpty() ? allCharacterNames : specificCharacterNames;

    QList<QPair<QString, QList<int>>> ret = this->evalPresence(
            report, characterNames, [](const Scene *scene, const QString &characterName) -> int {
                return scene->hasCharacter(characterName)
                        ? scene->characterPresence(characterName) + 2
                        : 0;
            });
    if (!specificCharacterNames.isEmpty())
        ret = ret.mid(0, specificCharacterNames.size());
    else if (report->maxCharacterPresenceGraphs() > 1)
        ret = ret.mid(0, report->maxCharacterPresenceGraphs());

    return ret;
}

QList<QPair<QString, QList<int>>>
StatisticsReportTimeline::evalLocationPresence(const StatisticsReport *report) const
{
    const Structure *structure = report->document()->structure();
    const QStringList allLocations = structure->allLocations();
    const QStringList specificLocations = report->locations();
    const QStringList locations = specificLocations.isEmpty() ? allLocations : specificLocations;

    QString lastLocation;
    QList<QPair<QString, QList<int>>> ret = this->evalPresence(
            report, locations, [&lastLocation](const Scene *scene, const QString &location) -> int {
                const QString sceneLocation =
                        scene->heading()->isEnabled() ? scene->heading()->location() : lastLocation;
                const int ret = sceneLocation == location ? 10 : 0;
                lastLocation = sceneLocation;
                return ret;
            });

    if (!specificLocations.isEmpty())
        ret = ret.mid(0, specificLocations.size());
    else if (report->maxLocationPresenceGraphs() > 0)
        ret = ret.mid(0, report->maxLocationPresenceGraphs());

    return ret;
}

QList<QPair<QString, QList<int>>> StatisticsReportTimeline::evalPresence(
        const StatisticsReport *report, const QStringList &allNames,
        std::function<int(const Scene *, const QString &)> determinePresenceFunc) const
{
    QList<QPair<QString, QList<int>>> ret;

    const Screenplay *screenplay = report->document()->screenplay();
    const QList<ScreenplayElement *> sceneElements = screenplay->getFilteredElements(
            [](ScreenplayElement *e) { return e->scene() != nullptr; });

    for (const QString &name : allNames)
        ret << qMakePair(name, QList<int>());

    for (const auto sceneElement : sceneElements) {
        auto scene = sceneElement->scene();
        for (auto &pair : ret)
            pair.second.append(determinePresenceFunc(scene, pair.first));
    }

    std::sort(ret.begin(), ret.end(),
              [](const QPair<QString, QList<int>> &a, const QPair<QString, QList<int>> &b) {
                  return std::accumulate(a.second.begin(), a.second.end(), 0)
                          > std::accumulate(b.second.begin(), b.second.end(), 0);
              });

    return ret;
}

QGraphicsRectItem *StatisticsReportTimeline::createCharacterPresenceGraph(
        const StatisticsReport *report, QGraphicsItem *container,
        const QGraphicsRectItem *sceneItemsContainer) const
{
    const Structure *structure = report->document()->structure();
    const QList<QPair<QString, QList<int>>> characterPresence = this->evalCharacterPresence(report);
    auto evalCharacterColor = [structure](const QString &name) -> QColor {
        const Character *character = structure->findCharacter(name);
        return character ? character->color() : Qt::white;
    };

    const Screenplay *screenplay = report->document()->screenplay();
    const QList<ScreenplayElement *> sceneElements = screenplay->getFilteredElements(
            [](ScreenplayElement *e) { return e->scene() != nullptr; });

    const QList<StatisticsReport::Distribution> dialogueDist = report->dialogueDistribution();
    auto evalCharacterLabel = [=](const QString &characterName,
                                  const QList<int> presence) -> QString {
        // dialogue count & time can be found from dialogue distribution
        auto it = std::find_if(dialogueDist.begin(), dialogueDist.end(),
                               [characterName](const StatisticsReport::Distribution &dist) {
                                   return dist.key == characterName;
                               });

        // screen time can be found by summing up scene times across all scenes
        // in which the character exists.
        int nrScenes = 0;
        qreal screenPresenceLength = 0;
        for (int scIdx = 0; scIdx < presence.size(); scIdx++) {
            if (presence[scIdx] > 0) {
                ++nrScenes;
                const Scene *scene = sceneElements.at(scIdx)->scene();
                const qreal scenePixelLength = report->pixelLength(scene);
                screenPresenceLength += scenePixelLength;
            }
        }

        const QTime screenTime = report->pixelLengthToTime(screenPresenceLength);

        // Prepare to format with what we have now.
        QString ret = characterName + QStringLiteral(" - Sc: ") + QString::number(nrScenes)
                + QStringLiteral(", ") + timeToString(screenTime);
        if (it == dialogueDist.end())
            return ret;

        ret += QStringLiteral(", D: ") + QString::number(it->count) + QStringLiteral(", ")
                + timeToString(it->timeLength);
        return ret;
    };

    return this->createPresenceGraph(characterPresence, evalCharacterColor, evalCharacterLabel,
                                     report, container, sceneItemsContainer);
}

QGraphicsRectItem *StatisticsReportTimeline::createLocationPresenceGraph(
        const StatisticsReport *report, QGraphicsItem *container,
        const QGraphicsRectItem *sceneItemsContainer) const
{
    const QList<QPair<QString, QList<int>>> locationPresence = this->evalLocationPresence(report);

    const Screenplay *screenplay = report->document()->screenplay();
    const QList<ScreenplayElement *> sceneElements = screenplay->getFilteredElements(
            [](ScreenplayElement *e) { return e->scene() != nullptr; });

    auto evalLocationLabel = [=](const QString &locationName,
                                 const QList<int> presence) -> QString {
        int nrScenes = 0;
        qreal scenePixelLength = 0;
        for (int scIdx = 0; scIdx < presence.size(); scIdx++) {
            if (presence[scIdx] > 0) {
                ++nrScenes;
                scenePixelLength += report->pixelLength(sceneElements.at(scIdx));
            }
        }

        const QTime timeLength = report->pixelLengthToTime(scenePixelLength);
        return locationName + QStringLiteral(" - Sc: ") + QString::number(nrScenes)
                + QStringLiteral(", ") + timeToString(timeLength);
    };

    int locationIndex = 0;
    auto evalColorFunc = [&locationIndex](const QString &) {
        return StatisticsReport::pickColor(locationIndex++ % 2, true, StatisticsReport::Location);
    };

    return this->createPresenceGraph(locationPresence, evalColorFunc, evalLocationLabel, report,
                                     container, sceneItemsContainer, false);
}

QGraphicsRectItem *StatisticsReportTimeline::createPresenceGraph(
        const QList<QPair<QString, QList<int>>> &presence,
        std::function<QColor(const QString &)> evalColorFunc,
        std::function<QString(const QString &, const QList<int> &)> evalLabelFunc,
        const StatisticsReport *report, QGraphicsItem *container,
        const QGraphicsRectItem *sceneItemsContainer, bool useCurvedPath) const
{
    const Structure *structure = report->document()->structure();
    if (presence.isEmpty())
        return nullptr;

    const qreal containerWidth = container->boundingRect().width();
    const qreal heightPerGraph = 40;
    const int nrGraphs = presence.size();
    const QPointF sceneItemsBottomLeft = container->mapFromItem(
            sceneItemsContainer, sceneItemsContainer->boundingRect().bottomLeft());
    const QPointF containerBottomLeft = container->childrenBoundingRect().bottomLeft();
    const QPointF graphContainerPos(sceneItemsBottomLeft.x(), containerBottomLeft.y() + 20);

    QGraphicsRectItem *graphContainer = new QGraphicsRectItem(container);
    graphContainer->setRect(0, 0, containerWidth, heightPerGraph * nrGraphs);
    graphContainer->setBrush(Qt::NoBrush);
    graphContainer->setPen(Qt::NoPen);
    graphContainer->setPos(graphContainerPos);

    // First evaluate the max presence across all items
    // That's going to give us the max-Y axis across all line graphs
    int maxPresence = 0;
    for (const auto &presenceItem : presence) {
        for (int presence : presenceItem.second)
            maxPresence = qMax(presence, maxPresence);
    }
    ++maxPresence;

    const qreal heightPerPresence = heightPerGraph / maxPresence;
    const qreal cubicCurveThreshold = 2.0 * heightPerPresence;

    // Now create path items for each character presence info
    int uncoloredCount = 0;
    QList<QGraphicsPathItem *> graphs;
    const QList<QGraphicsItem *> sceneItems = sceneItemsContainer->childItems();
    for (const auto &presenceItem : presence) {
        bool hasPresence = false;

        QPainterPath path;
        path.moveTo(0, heightPerGraph);
        for (int i = 0; i < presenceItem.second.size(); i++) {
            const QGraphicsItem *sceneItem =
                    i >= 0 && i < sceneItems.size() ? sceneItems.at(i) : nullptr;
            if (sceneItem == nullptr)
                break;

            hasPresence = true;
            const int presence = presenceItem.second.at(i);
            const QRectF sceneItemRect =
                    graphContainer->mapFromItem(sceneItem, sceneItem->boundingRect())
                            .boundingRect();
            const qreal y = heightPerGraph - presence * heightPerPresence;
            const QPointF prevPos = path.currentPosition();

            if (useCurvedPath) {
                const qreal x =
                        graphContainer->mapFromItem(sceneItem, sceneItem->boundingRect().center())
                                .x();

                const QPointF newPos = QPointF(x, y);
                if (qAbs(newPos.y() - prevPos.y()) > cubicCurveThreshold) {
                    // Approxmiating a spline curve
                    const QRectF posRect = QRectF(prevPos, newPos).normalized();
                    const QPointF topCenter(posRect.center().x(), posRect.top() + 1);
                    const QPointF bottomCenter(posRect.center().x(), posRect.bottom() - 1);
                    const bool goingUp = newPos.y() < prevPos.y();
                    const QPointF ctrlPoint1 = goingUp ? bottomCenter : topCenter;
                    const QPointF ctrlPoint2 = goingUp ? topCenter : bottomCenter;
                    path.cubicTo(ctrlPoint1, ctrlPoint2, newPos);
                } else
                    path.lineTo(newPos);
            } else {
                const QPointF tl(sceneItemRect.left(), y);
                const QPointF tr(sceneItemRect.right(), y);
                if (!qFuzzyCompare(prevPos.y(), y))
                    path.lineTo(tl);
                path.lineTo(tr);
            }
        }

        path.lineTo(containerWidth, heightPerGraph);

        if (!hasPresence)
            continue;

        QPainterPath fillPath = path;
        fillPath.closeSubpath();

        auto polishColor = [&uncoloredCount](const QColor &color) {
            if (color == Qt::white)
                return StatisticsReport::pickColor(uncoloredCount++);
            return color.darker();
        };
        const Character *character = structure->findCharacter(presenceItem.first);
        const QColor evaledColor = evalColorFunc ? polishColor(evalColorFunc(presenceItem.first))
                                                 : StatisticsReport::pickColor(uncoloredCount++);

        QPen chartPen(evaledColor);
        chartPen.setCapStyle(Qt::RoundCap);
        chartPen.setJoinStyle(Qt::RoundJoin);
        chartPen.setWidth(1);

        QBrush chartBrush(evaledColor);

        QGraphicsPathItem *chartFill = new QGraphicsPathItem(graphContainer);
        chartFill->setPath(fillPath);
        chartFill->setBrush(chartBrush);
        chartFill->setPen(Qt::NoPen);
        chartFill->setOpacity(0.3);
        chartFill->setPos(QPointF(0, heightPerGraph * graphs.size()));

        QGraphicsPathItem *chartOutline = new QGraphicsPathItem(graphContainer);
        chartOutline->setPath(path);
        chartOutline->setBrush(Qt::NoBrush);
        chartOutline->setPen(chartPen);
        chartOutline->setPos(chartFill->pos());

        QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(chartOutline);
        if (evalLabelFunc)
            textItem->setText(evalLabelFunc(presenceItem.first, presenceItem.second));
        else
            textItem->setText(presenceItem.first);
        textItem->setPos(4, (heightPerGraph - textItem->boundingRect().height()) / 3);

        QGraphicsRectItem *textItemBg = new QGraphicsRectItem(chartOutline);
        textItemBg->setZValue(-1);
        textItemBg->setBrush(character ? character->color() : Qt::white);
        textItemBg->setPen(evaledColor);
        textItemBg->setRect(textItem->boundingRect().adjusted(-4, -4, 4, 4));
        textItemBg->setOpacity(0.5);
        textItemBg->setPos(textItem->pos());

        graphs.append(chartOutline);
    }

    return graphContainer;
}

QGraphicsRectItem *StatisticsReportTimeline::createSeparator(QGraphicsItem *container,
                                                             const QString &label,
                                                             const QColor &color) const
{
    QGraphicsRectItem *separatorItem = new QGraphicsRectItem(container);
    separatorItem->setPen(Qt::NoPen);
    separatorItem->setBrush(Qt::NoBrush);

    QGraphicsRectItem *labelBg = new QGraphicsRectItem(separatorItem);
    labelBg->setPen(Qt::NoPen);
    labelBg->setBrush(color);

    QGraphicsSimpleTextItem *labelText = new QGraphicsSimpleTextItem(separatorItem);
    labelText->setText(label);
    labelText->setBrush(Application::textColorFor(color));

    QRectF containerBRect = container->childrenBoundingRect();
    QRectF labelTextRect = labelText->boundingRect();
    QRectF labelBgRect;
    QRectF separatorRect(0, 0, containerBRect.width(), labelTextRect.height() + 10);
    separatorItem->setRect(separatorRect);
    separatorItem->setPos(containerBRect.bottomLeft() + QPointF(0, 20));

    labelTextRect.moveCenter(separatorRect.center());
    labelText->setPos(labelTextRect.topLeft());

    labelBgRect = labelTextRect.adjusted(-20, -5, 20, 5);
    labelBg->setRect(QRectF(0, 0, labelBgRect.width(), labelBgRect.height()));
    labelBg->setPos(labelBgRect.topLeft());

    QGraphicsLineItem *leftLine = new QGraphicsLineItem(separatorItem);
    leftLine->setLine(0, labelBgRect.center().y(), labelBgRect.left(), labelBgRect.center().y());
    leftLine->setPen(QPen(color, 5));
    leftLine->setZValue(-1);

    QGraphicsLineItem *rightLine = new QGraphicsLineItem(separatorItem);
    rightLine->setLine(labelBgRect.right(), labelBgRect.center().y(), containerBRect.width(),
                       labelBgRect.center().y());
    rightLine->setPen(QPen(color, 5));
    rightLine->setZValue(-1);

    return separatorItem;
}

////////////////////////////////////////////////////////////////////////////////////

StatisticsReportDialogueActionRatio::StatisticsReportDialogueActionRatio(
        const StatisticsReport *report, QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    const qreal chartSize = 200;

    this->setPen(Qt::NoPen);
    this->setBrush(Qt::NoBrush);

    auto textDistrubution = report->textDistribution(true);

    QtCharts::QChart *chart = new QtCharts::QChart(this);
    chart->legend()->setVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->resize(chartSize, chartSize);
    chart->setBackgroundVisible(true);

    QFont smallFont = Application::font();
    smallFont.setPointSize(8);

    // I can never get QChart's own legend to show up on the right
    // I give up. I am going to manually put together a legend.
    StatisticsReportGraphVLegend *legend = new StatisticsReportGraphVLegend(this);

    QtCharts::QPieSeries *pieSeries = new QtCharts::QPieSeries(chart);
    for (const auto &dist : qAsConst(textDistrubution)) {
        const QColor color = StatisticsReport::pickColor(pieSeries->slices().size());
        QtCharts::QPieSlice *slice = pieSeries->append(dist.key, dist.ratio);
        slice->setBrush(color);
        slice->setLabel(dist.percent);
        slice->setLabelFont(smallFont);
        slice->setLabelColor(Application::textColorFor(color));
        slice->setLabelVisible(true);
        slice->setLabelPosition(QtCharts::QPieSlice::LabelInsideNormal);
        legend->add(color, dist.key);
    }

    chart->addSeries(pieSeries);
    chart->setPlotArea(QRectF(0, 0, chartSize, chartSize));

    legend->place(chart->boundingRect(), Qt::AlignRight);

    this->setRect(this->childrenBoundingRect());
}

StatisticsReportDialogueActionRatio::~StatisticsReportDialogueActionRatio() { }

////////////////////////////////////////////////////////////////////////////////////

StatisticsReportSceneHeadingStats::StatisticsReportSceneHeadingStats(const StatisticsReport *report,
                                                                     QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    this->setPen(Qt::NoPen);
    this->setBrush(Qt::NoBrush);

    auto ir = [](const QGraphicsItem *item) {
        return item->mapToScene(item->boundingRect()).boundingRect();
    };

    const qreal chartSize = 200;
    const Screenplay *screenplay = report->document()->screenplay();
    const auto sceneElements = screenplay->getFilteredElements(
            [](ScreenplayElement *e) { return e->scene() && e->scene()->heading()->isEnabled(); });

    const QString othersKey = QStringLiteral("OTHERS");

    SequentialMap<QString, int> typeMap;
    SequentialMap<QString, int> locationMap;
    SequentialMap<QString, QMap<QString, int>> momentMap;
    SequentialMap<QString, QColor> typeColorMap;
    QString otherLocations, otherMoments;

    for (const auto sceneElement : sceneElements) {
        const Scene *scene = sceneElement->scene();
        const SceneHeading *sceneHeading = scene->heading();

        typeMap[sceneHeading->locationType()]++;
        locationMap[sceneHeading->location()]++;
        momentMap[sceneHeading->moment()][sceneHeading->locationType()]++;
    }

    const int maxMoments = 5;
    if (momentMap.size() > maxMoments) {
        typedef QPair<QString, QMap<QString, int>> MomentItem;
        momentMap.sort([](const MomentItem &a, const MomentItem &b) {
            const QList<int> avalues = a.second.values();
            const QList<int> bvalues = b.second.values();
            return std::accumulate(avalues.begin(), avalues.end(), 0)
                    > std::accumulate(bvalues.begin(), bvalues.end(), 0);
        });

        const QVector<MomentItem> exclMoments = momentMap.vector.mid(maxMoments - 1);
        momentMap.vector = momentMap.vector.mid(0, maxMoments - 1);

        for (const auto &item : exclMoments) {
            auto it = item.second.constBegin();
            auto end = item.second.constEnd();
            while (it != end) {
                momentMap[othersKey][it.key()] += it.value();
                ++it;
            }
        }

        otherMoments = exclMoments.first().first;
        if (exclMoments.size() > 1)
            otherMoments += QStringLiteral(" and ") + QString::number(exclMoments.size() - 1)
                    + QStringLiteral(" more.");
    }

    const int maxLocations = 6;
    if (locationMap.size() > maxLocations) {
        typedef QPair<QString, int> LocationItem;
        locationMap.sort(
                [](const LocationItem &a, const LocationItem &b) { return a.second > b.second; });

        const QVector<LocationItem> exclLocs = locationMap.vector.mid(maxLocations - 1);
        locationMap.vector = locationMap.vector.mid(0, maxLocations - 1);

        for (const auto &item : exclLocs)
            locationMap[othersKey] += item.second;

        otherLocations = exclLocs.first().first;
        if (exclLocs.size() > 1)
            otherLocations += QStringLiteral(" and ") + QString::number(exclLocs.size() - 1)
                    + QStringLiteral(" more.");
    }

    QFont smallFont = Application::font();
    smallFont.setPointSize(8);

    QFont tinyFont = smallFont;
    tinyFont.setPointSize(6);

    // Common legend for the first two graphs
    StatisticsReportGraphVLegend *typeLegend = new StatisticsReportGraphVLegend(this);

    // Pie chart for INT,EXT distribution
    QtCharts::QChart *typeChart = new QtCharts::QChart(this);
    typeChart->legend()->setVisible(false);
    typeChart->setMargins(QMargins(0, 0, 0, 0));
    typeChart->resize(chartSize, chartSize);
    typeChart->setBackgroundVisible(false);
    QtCharts::QPieSeries *typeSeries = new QtCharts::QPieSeries(typeChart);
    auto it1 = typeMap.vector.constBegin();
    auto end1 = typeMap.vector.constEnd();
    while (it1 != end1) {
        const int percent = qRound(100.0 * qreal(it1->second) / qreal(sceneElements.size()));
        const QColor color = StatisticsReport::pickColor(typeSeries->slices().size(), false);
        const QString label = QString::number(it1->second) + QStringLiteral(" Scenes: ")
                + QString::number(percent) + QStringLiteral("%");
        QtCharts::QPieSlice *slice = typeSeries->append(label, it1->second);
        slice->setBrush(color);
        slice->setLabel(label);
        slice->setLabelColor(Application::textColorFor(color));
        slice->setLabelPosition(QtCharts::QPieSlice::LabelInsideNormal);
        slice->setLabelVisible(true);
        slice->setLabelFont(tinyFont);

        typeLegend->add(color, it1->first);
        typeColorMap[it1->first] = color;

        ++it1;
    }
    typeChart->addSeries(typeSeries);
    typeChart->setPlotArea(QRectF(0, 0, chartSize, chartSize));

    // Stacked bar chart for DAY, NIGHT etc..
    auto it2 = momentMap.vector.constBegin();
    auto end2 = momentMap.vector.constEnd();
    QtCharts::QChart *momentChart = new QtCharts::QChart(this);
    QtCharts::QStackedBarSeries *momentSeries = new QtCharts::QStackedBarSeries(momentChart);
    QtCharts::QBarCategoryAxis *momentNameAxis = new QtCharts::QBarCategoryAxis(momentSeries);
    QtCharts::QValueAxis *momentValueAxis = new QtCharts::QValueAxis(momentSeries);
    momentNameAxis->setVisible(true);
    momentNameAxis->setLabelsVisible(true);
    momentNameAxis->append(momentMap.keys());
    momentNameAxis->setLabelsFont(smallFont);
    momentNameAxis->setGridLineVisible(false);
    momentValueAxis->setVisible(true);
    momentValueAxis->setLabelFormat(QStringLiteral("%d"));
    momentValueAxis->setLabelsFont(tinyFont);
    momentChart->legend()->setVisible(false);
    momentChart->setMargins(QMargins(0, 0, 0, 0));
    momentChart->setBackgroundVisible(false);
    momentChart->addAxis(momentNameAxis, Qt::AlignBottom);
    momentChart->addAxis(momentValueAxis, Qt::AlignLeft);
    momentSeries->attachAxis(momentNameAxis);
    momentSeries->attachAxis(momentValueAxis);
    momentSeries->setLabelsVisible(true);
    momentSeries->setLabelsPosition(QtCharts::QStackedBarSeries::LabelsCenter);

    qreal categoryWidth = 60;
    QFontMetricsF fm(momentNameAxis->labelsFont());
    int momentValueAxisMax = 0;
    while (it2 != end2) {
        categoryWidth = qMax(categoryWidth, fm.boundingRect(it2->first).width());

        int maxValue = 0;
        const QStringList types = typeColorMap.keys();
        for (const QString &type : types) {
            QtCharts::QBarSet *barSet =
                    momentSeries->findChild<QtCharts::QBarSet *>(type, Qt::FindDirectChildrenOnly);
            if (barSet == nullptr) {
                barSet = new QtCharts::QBarSet(type, momentSeries);
                barSet->setObjectName(type);
                barSet->setColor(typeColorMap.value(type));
                barSet->setLabelFont(smallFont);
                barSet->setLabelColor(Application::textColorFor(barSet->color()));
                momentSeries->append(barSet);
            }

            const int value = it2->second.value(type);
            barSet->append(value);
            maxValue += value;
        }

        momentValueAxisMax = qMax(maxValue, momentValueAxisMax);

        ++it2;
    }

    categoryWidth *= 1.2;
    momentNameAxis->setLabelsFont(tinyFont);
    momentValueAxis->setRange(0, momentValueAxisMax);
    momentChart->addSeries(momentSeries);
    momentChart->resize(categoryWidth * momentMap.size(), chartSize);

    // Another pie chart for locations (legend only for top-5 locations)
    auto it3 = locationMap.vector.constBegin();
    auto end3 = locationMap.vector.constEnd();
    QtCharts::QChart *locationChart = new QtCharts::QChart(this);
    QtCharts::QStackedBarSeries *locationSeries = new QtCharts::QStackedBarSeries(locationChart);
    QtCharts::QBarCategoryAxis *locationNameAxis = new QtCharts::QBarCategoryAxis(locationSeries);
    QtCharts::QValueAxis *locationValueAxis = new QtCharts::QValueAxis(locationSeries);
    locationNameAxis->setVisible(true);
    locationNameAxis->setLabelsVisible(true);
    locationNameAxis->setLabelsFont(smallFont);
    locationNameAxis->setGridLineVisible(false);
    locationValueAxis->setVisible(true);
    locationValueAxis->setLabelsFont(tinyFont);
    locationValueAxis->setLabelFormat(QStringLiteral("%d"));
    locationChart->legend()->setVisible(false);
    locationChart->setMargins(QMargins(0, 0, 0, 0));
    locationChart->setBackgroundVisible(false);
    locationSeries->setLabelsVisible(true);
    locationSeries->setLabelsPosition(QtCharts::QStackedBarSeries::LabelsCenter);

    QtCharts::QBarSet *locationBarSet = new QtCharts::QBarSet(QString(), locationSeries);
    locationBarSet->setColor(QColor("#864879"));
    locationSeries->append(locationBarSet);
    locationBarSet->setLabelFont(tinyFont);
    locationBarSet->setLabelColor(Application::textColorFor(locationBarSet->color()));

    // Location lookup will be another legend, but without color.
    StatisticsReportGraphVLegend *locationLegend = new StatisticsReportGraphVLegend(this);
    locationLegend->setFont(smallFont);
    while (it3 != end3) {
        const bool othersLoc = it3->first == othersKey;
        if (!othersLoc) {
            const QString locName = othersLoc
                    ? it3->first
                    : QStringLiteral("L%1").arg(locationNameAxis->count() + 1);
            locationBarSet->append(it3->second);
            locationNameAxis->append(locName);
            locationLegend->add(Qt::transparent,
                                locName + QStringLiteral(": ") + it3->first + QStringLiteral(" (")
                                        + QString::number(it3->second) + QStringLiteral(")"));
        } else
            locationLegend->add(Qt::transparent,
                                othersKey + QStringLiteral(": ") + otherLocations
                                        + QStringLiteral(" (") + QString::number(it3->second)
                                        + QStringLiteral(")"));
        ++it3;
    }

    locationNameAxis->setLabelsFont(tinyFont);
    locationChart->addSeries(locationSeries);
    locationChart->addAxis(locationNameAxis, Qt::AlignBottom);
    locationChart->addAxis(locationValueAxis, Qt::AlignLeft);
    locationSeries->attachAxis(locationNameAxis);
    locationSeries->attachAxis(locationValueAxis);
    locationChart->resize(categoryWidth * (locationMap.size() - 1), chartSize);

    // Place them in a horizontal row
    typeChart->setPos(0, 0);
    typeLegend->place(ir(typeChart));
    momentChart->setPos(ir(typeLegend).right(), 0);
    locationChart->setPos(ir(momentChart).right(), 0);
    locationLegend->place(ir(locationChart));

    this->setRect(this->childrenBoundingRect());
}

StatisticsReportSceneHeadingStats::~StatisticsReportSceneHeadingStats() { }

////////////////////////////////////////////////////////////////////////////////////

StatisticsReportGraphVLegend::StatisticsReportGraphVLegend(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    this->setPen(Qt::NoPen);
    this->setBrush(Qt::NoBrush);
}

StatisticsReportGraphVLegend::~StatisticsReportGraphVLegend() { }

void StatisticsReportGraphVLegend::place(const QRectF &rect, Qt::Alignment alignment)
{
    QRectF myRect = this->boundingRect();
    myRect.moveCenter(rect.center());

    switch (alignment) {
    case Qt::AlignTop:
        myRect.moveBottom(rect.top());
        break;
    case Qt::AlignLeft:
        myRect.moveRight(rect.left());
        break;
    case Qt::AlignBottom:
        myRect.moveTop(rect.bottom());
        break;
    case Qt::AlignRight:
    default:
        myRect.moveLeft(rect.right());
        break;
    }

    this->setPos(myRect.topLeft());
}

void StatisticsReportGraphVLegend::add(const QColor &color, const QString &label)
{
    const QPointF legendItemPos = this->childItems().isEmpty()
            ? QPointF(0, 0)
            : this->boundingRect().bottomLeft() + QPointF(0, 5);

    if (color == Qt::transparent) {
        QGraphicsSimpleTextItem *legendText = new QGraphicsSimpleTextItem(this);
        legendText->setText(label);
        legendText->setFont(m_font);
        legendText->setPos(legendItemPos);
    } else {
        QGraphicsRectItem *legendItem = new QGraphicsRectItem(this);
        legendItem->setPos(legendItemPos);
        legendItem->setPen(Qt::NoPen);
        legendItem->setBrush(Qt::NoBrush);

        QGraphicsRectItem *legendRect = new QGraphicsRectItem(legendItem);
        legendRect->setBrush(color);
        legendRect->setPen(Qt::NoPen);
        legendRect->setRect(0, 0, 10, 10);

        QGraphicsSimpleTextItem *legendText = new QGraphicsSimpleTextItem(legendItem);
        legendText->setText(label);
        legendText->setFont(m_font);
        legendText->setPos(legendRect->rect().width() + 5, 0);
        legendRect->setPos(
                0, (legendText->boundingRect().height() - legendRect->boundingRect().height()) / 2);

        legendItem->setRect(legendItem->childrenBoundingRect());
    }

    this->setRect(this->childrenBoundingRect());
}
