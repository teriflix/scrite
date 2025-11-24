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

#include "statisticsreport.h"
#include "screenplaypaginatorworker.h"
#include "statisticsreport_p.h"
#include "languageengine.h"

#include "utils.h"
#include "scene.h"
#include "hourglass.h"
#include "screenplay.h"
#include "scritedocument.h"

#include <QPdfWriter>
#include <QRandomGenerator>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextTable>

StatisticsReport::StatisticsReport(QObject *parent) : AbstractReportGenerator(parent) { }

StatisticsReport::~StatisticsReport() { }

const QVector<QColor> StatisticsReport::colors(ColorGroup group)
{
    switch (group) {
    case Character:
    case Episode:
        return { QColor(94, 54, 138), // #5E368A - Cyber Grape
                 QColor(232, 191, 90) }; // #E8BF5A - Saffron
    case Beat:
        return ScreenplayTracks::defaultColors();
    case Act:
        return {
            QColor(0, 90, 158), // #005A9E - Strong Blue
            QColor(218, 56, 49), // #DA3831 - Bright Red
            QColor(0, 120, 100), // #007864 - Teal Green
            QColor(106, 76, 156), // #6A4C9C - Royal Purple
            QColor(242, 148, 0), // #F29400 - Bright Orange
            QColor(128, 130, 133), // #808285 - Medium Gray
            QColor(139, 69, 19), // #8B4513 - Saddle Brown
            QColor(23, 143, 173), // #178FAD - Cerulean Blue
            QColor(255, 105, 180), // #FF69B4 - Hot Pink
            QColor(60, 179, 113), // #3CB371 - Medium Sea Green
            QColor(70, 130, 180), // #4682B4 - Steel Blue
            QColor(210, 105, 30), // #D2691E - Chocolate
            QColor(255, 215, 0), // #FFD700 - Gold
            QColor(176, 48, 96), // #B03060 - Maroon
            QColor(0, 191, 255), // #00BFFF - Deep Sky Blue
            QColor(154, 205, 50), // #9ACD32 - Yellow Green
            QColor(255, 69, 0), // #FF4500 - Orange Red
            QColor(32, 178, 170), // #20B2AA - Light Sea Green
            QColor(147, 112, 219), // #9370DB - Medium Purple
            QColor(218, 165, 32), // #DAA520 - Goldenrod
        };
    default:
        return {
            QColor(134, 72, 121), // #864879 - Fuchsia Purple
            QColor(255, 193, 7), // #FFC107 - Amber
            QColor(76, 175, 80), // #4CAF50 - Green
            QColor(3, 169, 244), // #03A9F4 - Light Blue
            QColor(255, 87, 34), // #FF5722 - Deep Orange
        };
    }
}

const QColor StatisticsReport::pickColor(int index, bool cycleAround, ColorGroup group)
{
    const QVector<QColor> colors = StatisticsReport::colors(group);
    const QColor baseColor = colors.at(qAbs(index) % (colors.length()));

    if (cycleAround || index < colors.size())
        return baseColor;

    const int batch = qFloor(qreal(index) / colors.size());
    const bool lighter = batch % 2;
    const int factor = 100 + batch * 50;
    return lighter ? baseColor.lighter(factor) : baseColor.darker(factor);
}

const QColor StatisticsReport::pickRandomColor(ColorGroup group)
{
    const QVector<QColor> list = colors(group);
    int index = QRandomGenerator::system()->bounded(list.size());
    return list.at(qBound(0, index, list.size() - 1));
}

void StatisticsReport::setIncludeCharacterPresenceGraphs(bool val)
{
    if (m_includeCharacterPresenceGraphs == val)
        return;

    m_includeCharacterPresenceGraphs = val;
    emit includeCharacterPresenceGraphsChanged();
}

void StatisticsReport::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void StatisticsReport::setIncludeLocationPresenceGraphs(bool val)
{
    if (m_includeLocationPresenceGraphs == val)
        return;

    m_includeLocationPresenceGraphs = val;
    emit includeLocationPresenceGraphsChanged();
}

void StatisticsReport::setConsiderPreferredGroupCategoryOnly(bool val)
{
    if (m_considerPreferredGroupCategoryOnly == val)
        return;

    m_considerPreferredGroupCategoryOnly = val;
    emit considerPreferredGroupCategoryOnlyChanged();
}

bool StatisticsReport::isConsiderPreferredGroupCategoryOnly() const
{
    return m_considerPreferredGroupCategoryOnly;
}

void StatisticsReport::setMaxLocationPresenceGraphs(int val)
{
    if (m_maxLocationPresenceGraphs == val)
        return;

    m_maxLocationPresenceGraphs = val;
    emit maxLocationPresenceGraphsChanged();
}

void StatisticsReport::setLocations(const QStringList &val)
{
    if (m_locations == val)
        return;

    m_locations = val;
    emit locationsChanged();
}

void StatisticsReport::setMaxCharacterPresenceGraphs(int val)
{
    if (m_maxCharacterPresenceGraphs == val)
        return;

    m_maxCharacterPresenceGraphs = val;
    emit maxCharacterPresenceGraphsChanged();
}

QList<StatisticsReport::Distribution> StatisticsReport::textDistribution(bool compact) const
{
    QList<StatisticsReport::Distribution> ret;
    QMap<SceneElement::Type, StatisticsReport::Distribution> map;

    const PaginatorDocumentInsights insights =
            m_textDocument.property(PaginatorDocumentInsights::property)
                    .value<PaginatorDocumentInsights>();
    if (m_textDocument.isEmpty() || insights.isEmpty())
        return ret;

    auto add = [&map](SceneElement::Type type, qreal pixelLength) {
        auto distribution = map.value(type);
        distribution.count += 1;
        distribution.pixelLength += pixelLength;
        map.insert(type, distribution);
    };

    {
        // First, lets sum up pixel lengths of all paragraph types.
        QTextBlock block = m_textDocument.firstBlock();
        while (block.isValid()) {
            const ScreenplayPaginatorBlockData *data = ScreenplayPaginatorBlockData::get(block);
            if (data) {
                add(data->paragraphType,
                    ScreenplayPaginator::pixelLength(block, block, &m_textDocument));
            }
            block = block.next();
        }

        if (compact) {
            map[SceneElement::Action].pixelLength += map[SceneElement::Shot].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Heading].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Character].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Transition].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Parenthetical].pixelLength;
            map.remove(SceneElement::Shot);
            map.remove(SceneElement::Heading);
            map.remove(SceneElement::Character);
            map.remove(SceneElement::Transition);
            map.remove(SceneElement::Parenthetical);
        }
    }

    const QMetaObject *mo = &SceneElement::staticMetaObject;
    const QMetaEnum typeEnum = mo->enumerator(mo->indexOfEnumerator("Type"));

    // Now lets convert pixel lengths to page & time lengths
    auto it = map.begin();
    auto end = map.end();
    while (it != end) {
        Distribution &dist = it.value();

        this->polish(dist);
        dist.key = QString::fromLatin1(typeEnum.valueToKey(it.key()));

        ret.append(dist);
        ++it;
    }

    this->normalizeRatios(ret);

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::dialogueDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();

    QMap<QString, Distribution> map;
    for (int i = 0; i < screenplay->elementCount(); i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);
        const Scene *scene = element->scene();
        if (scene == nullptr || element->isOmitted())
            continue;

        auto dialogues = scene->dialogueElements();
        auto it = dialogues.constBegin();
        auto end = dialogues.constEnd();
        while (it != end) {
            auto paragraphs = it.value();
            for (auto paragraph : qAsConst(paragraphs)) {
                Distribution &dist = map[it.key()];
                dist.pixelLength += this->pixelLength(paragraph);
                dist.count++;
            }
            ++it;
        }
    }

    auto it = map.begin();
    auto end = map.end();
    while (it != end) {
        Distribution &dist = it.value();

        this->polish(dist);
        dist.key = it.key();

        ret.append(dist);
        ++it;
    }

    std::sort(ret.begin(), ret.end(),
              [](const Distribution &a, const Distribution &b) { return a.ratio > b.ratio; });

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::sceneDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();
    for (int i = 0; i < screenplay->elementCount(); i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);
        const Scene *scene = element->scene();
        if (!scene || element->isOmitted())
            continue;

        if (scene && scene->heading()->isEnabled()) {
            StatisticsReport::Distribution item;
            item.key = element->resolvedSceneNumber();
            item.color = scene->color();
            ret.append(item);
        } else {
            if (ret.isEmpty())
                ret.append(StatisticsReport::Distribution());
        }

        ret.last().pixelLength += this->pixelLength(scene);
    }

    for (auto &item : ret)
        this->polish(item);

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::actDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();
    const Structure *structure = this->document()->structure();
    const auto actElements = screenplay->getFilteredElements([](ScreenplayElement *e) {
        return e->elementType() == ScreenplayElement::BreakElementType
                && e->breakType() == Screenplay::Act;
    });

    if (actElements.isEmpty())
        return ret;

    const QString prefrredGroupCategory = structure->preferredGroupCategory();
    const QStringList actNames =
            structure->categoryActNames().value(prefrredGroupCategory).toStringList();
    auto actName = [actNames](int actIndex) {
        return actIndex >= 0 && actIndex < actNames.size()
                ? actNames.at(actIndex)
                : QStringLiteral("ACT %1").arg(actIndex + 1);
    };

    typedef QPair<QString, QList<Scene *>> ActSceneListPair;
    QList<ActSceneListPair> actScenesList;
    QString currentAct;
    int actIndex = -1;

    const auto elements = screenplay->getElements();
    for (auto element : elements) {
        if (element->scene()) {
            if (currentAct.isEmpty()) {
                actIndex = 0;
                currentAct = actName(actIndex);
                ActSceneListPair pair;
                pair.first = currentAct;
                actScenesList.append(pair);
            }

            actScenesList.last().second.append(element->scene());
        } else if (element->breakType() == Screenplay::Act) {
            ++actIndex;
            currentAct = actName(actIndex);
            ActSceneListPair pair;
            pair.first = currentAct;
            actScenesList.append(pair);
        } else {
            currentAct.clear();
            actIndex = -1;
        }
    }

    int episodeIndex = 0;
    QColor baseColor = screenplay->episodeCount() > 0 ? StatisticsReport::pickColor(episodeIndex)
                                                      : Qt::transparent;
    actIndex = 0;
    for (auto actScenes : qAsConst(actScenesList)) {
        StatisticsReport::Distribution item;
        item.key = actScenes.first;

        if (baseColor == Qt::transparent)
            item.color = StatisticsReport::pickColor(actIndex++, true,
                                                     screenplay->episodeCount() > 0
                                                             ? StatisticsReport::Act
                                                             : StatisticsReport::Episode);
        else {
            if (actIndex > 0 && item.key == actName(0))
                baseColor = StatisticsReport::pickColor(++episodeIndex, true,
                                                        screenplay->episodeCount() > 0
                                                                ? StatisticsReport::Act
                                                                : StatisticsReport::Episode);

            const bool baseColorIsLight = Utils::Color::isLight(baseColor);
            item.color = (actIndex++ % 2) ? baseColor.lighter(baseColorIsLight ? 150 : 240)
                                          : baseColor.lighter(baseColorIsLight ? 120 : 200);
        }

        for (Scene *scene : qAsConst(actScenes.second))
            item.pixelLength += this->pixelLength(scene);

        this->polish(item);
        ret.append(item);
    }

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::episodeDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();
    const auto episodeElements = screenplay->getFilteredElements([](ScreenplayElement *e) {
        return e->elementType() == ScreenplayElement::BreakElementType
                && e->breakType() == Screenplay::Episode;
    });
    if (episodeElements.isEmpty())
        return ret;

    typedef QPair<QString, QList<Scene *>> EpisodeSceneListPair;
    QList<EpisodeSceneListPair> episodeScenesList;
    QString currentEpisode;
    int episodeIndex = -1;

    const auto elements = screenplay->getElements();
    for (auto element : elements) {
        if (element->scene()) {
            if (currentEpisode.isEmpty()) {
                episodeIndex = 0;
                currentEpisode = QStringLiteral("Episode 1");
                EpisodeSceneListPair pair;
                pair.first = currentEpisode;
                episodeScenesList.append(pair);
            }

            episodeScenesList.last().second.append(element->scene());
        } else if (element->breakType() == Screenplay::Episode) {
            ++episodeIndex;
            currentEpisode = QStringLiteral("Episode %1").arg(episodeIndex + 1);
            EpisodeSceneListPair pair;
            pair.first = currentEpisode;
            episodeScenesList.append(pair);
        }
    }

    int epIndex = 0;
    for (auto episodeScenes : qAsConst(episodeScenesList)) {
        StatisticsReport::Distribution item;
        item.key = episodeScenes.first;
        item.color = StatisticsReport::pickColor(epIndex++, true, StatisticsReport::Episode);

        for (Scene *scene : qAsConst(episodeScenes.second))
            item.pixelLength += this->pixelLength(scene);

        this->polish(item);
        ret.append(item);
    }

    return ret;
}

bool StatisticsReport::doGenerate(QTextDocument *textDocument)
{
    auto guard = qScopeGuard([=]() { this->cleanupTextDocument(); });
    this->prepareTextDocument();

    /**
     * This function is called to generate report into ODT files.
     */
    Screenplay *screenplay = ScriteDocument::instance()->screenplay();
    Structure *structure = ScriteDocument::instance()->structure();

    QTextDocument &document = *textDocument;
    QTextCursor cursor(&document);

    const QFont defaultFont = this->document()->printFormat()->defaultFont();

    QTextBlockFormat defaultBlockFormat;

    QTextCharFormat defaultCharFormat;
    defaultCharFormat.setFontFamily(defaultFont.family());
    defaultCharFormat.setFontPointSize(12);

    // Report Title
    {
        QTextBlockFormat blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        cursor.setBlockFormat(blockFormat);

        QTextCharFormat charFormat = defaultCharFormat;
        charFormat.setFontPointSize(24);
        charFormat.setFontCapitalization(QFont::AllUppercase);
        charFormat.setFontWeight(QFont::Bold);
        charFormat.setFontUnderline(true);
        cursor.setCharFormat(charFormat);

        QString title = screenplay->title();
        if (title.isEmpty())
            title = QStringLiteral("Untitled Screenplay");
        cursor.insertText(title);
        cursor.insertBlock();
        cursor.insertText(QStringLiteral("Key Statistics"));

        blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        blockFormat.setBottomMargin(20);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertHtml("This report was generated using <strong>Scrite</strong><br/>(<a "
                          "href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
    }

    QTextTableFormat tableFormat;
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(5);
    tableFormat.setBorder(3);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setHeaderRowCount(1);

    QTextTable *table = cursor.insertTable(5, 3, tableFormat);

    cursor = table->cellAt(0, 0).firstCursorPosition();
    cursor.insertText(QStringLiteral("Page Count: "));
    cursor = table->cellAt(0, 1).firstCursorPosition();
    cursor.insertText(QString::number(this->pageCount()));
    cursor = table->cellAt(0, 2).firstCursorPosition();
    cursor.insertHtml(QStringLiteral("<font size=\"-2\">Page count may change in generated "
                                     "PDFs of the screenplay.</font>"));

    // Number of scenes
    cursor = table->cellAt(1, 0).firstCursorPosition();
    cursor.insertText(QStringLiteral("Number Of Scenes: "));
    cursor = table->cellAt(1, 1).firstCursorPosition();
    cursor.insertText(QString::number(screenplay->sceneCount()));

    // Text Distribution
    const auto textDist = this->textDistribution(true);
    for (int i = 0; i < textDist.size(); i++) {
        const int row = i + 2;
        const Distribution dist = textDist.at(i);
        cursor = table->cellAt(row, 0).firstCursorPosition();
        cursor.insertText(dist.key + QStringLiteral(": "));
        cursor = table->cellAt(row, 1).firstCursorPosition();
        cursor.insertText(dist.percent);
    }

    // Dialogue screen time for each character
    cursor = table->lastCursorPosition();
    cursor.movePosition(QTextCursor::Down);

    cursor.insertBlock();

    const auto dialogueLengths = this->dialogueDistribution();
    const QStringList characters = structure->allCharacterNames();

    table = cursor.insertTable(dialogueLengths.size() + 1, 3, tableFormat);
    cursor = table->cellAt(0, 0).firstCursorPosition();
    cursor.insertHtml(QStringLiteral("<b>Character</b>"));
    cursor = table->cellAt(0, 1).firstCursorPosition();
    cursor.insertHtml(QStringLiteral("<b>Dialogue Count</b>"));
    cursor = table->cellAt(0, 2).firstCursorPosition();
    cursor.insertHtml(QStringLiteral("<b>Dialogue Percent</b>"));

    QTextBlockFormat cellFormat;
    cellFormat.setAlignment(Qt::AlignRight);

    for (int i = 0; i < dialogueLengths.size(); i++) {
        const int row = i + 1;
        const Distribution dist = dialogueLengths.at(i);
        const QString character = dist.key;

        cursor = table->cellAt(row, 0).firstCursorPosition();
        cursor.insertText(character);

        cursor = table->cellAt(row, 1).firstCursorPosition();
        cursor.mergeBlockFormat(cellFormat);
        cursor.insertText(QString::number(dist.count));

        cursor = table->cellAt(row, 2).firstCursorPosition();
        cursor.mergeBlockFormat(cellFormat);
        cursor.insertText(dist.percent);
    }

    return true;
}

bool StatisticsReport::canDirectPrintToPdf() const
{
    return true;
}

bool StatisticsReport::usePdfWriter() const
{
    return true;
}

bool StatisticsReport::directPrintToPdf(QPdfWriter *pdfWriter)
{
    auto guard = qScopeGuard([=]() { this->cleanupTextDocument(); });
    this->prepareTextDocument();

    const Screenplay *screenplay = this->document()->screenplay();

    StatisticsReportPage scene(this);
    scene.setWatermark(this->watermark());
    scene.setComment(this->comment());
    scene.addStandardItems(StatisticsReportPage::WatermarkOverlayLayer
                           | StatisticsReportPage::FooterLayer
                           | StatisticsReportPage::DontIncludeScriteLink);
    scene.setTitle(screenplay->title() + QStringLiteral(" - Statistics"));
    const bool ret = scene.exportToPdf(pdfWriter);
    return ret;
}

void StatisticsReport::prepareTextDocument()
{
    HourGlass hourGlass;
    this->cleanupTextDocument();

    ScreenplayPaginator::paginateIntoDocument(this->document()->screenplay(),
                                              this->document()->printFormat(), &m_textDocument);

#if 0
    QTextDocumentWriter writer(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
                               + "/scrite-stats.odt");
    writer.write(&m_textDocument);

    QPdfWriter pdfWriter(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/scrite-stats.pdf");
    m_textDocument.print(&pdfWriter);
#endif
}

void StatisticsReport::cleanupTextDocument()
{
    m_textDocument.clear();
}

void StatisticsReport::polish(Distribution &distribution) const
{
    const qreal totalPixelLength = this->pixelLength();

    distribution.ratio = distribution.pixelLength / totalPixelLength;
    const int cent = qRound(distribution.ratio * 100);
    distribution.percent = QString::number(cent) + QStringLiteral("%");
    distribution.ratio = distribution.pixelLength / totalPixelLength;
    distribution.pageLength = this->pageLength(distribution.pixelLength);
    distribution.timeLength = this->pixelLengthToTime(distribution.pixelLength);
}

void StatisticsReport::normalizeRatios(QList<Distribution> &distributions) const
{
    // NOTE: This assumes that each of the Distributions in the list are already
    // polished.
    const qreal totalRatio =
            std::accumulate(distributions.begin(), distributions.end(), qreal(0),
                            [](qreal sum, const Distribution &d) { return sum += d.ratio; });
    if (qFuzzyIsNull(totalRatio) || totalRatio < 0)
        return;

    for (Distribution &d : distributions) {
        d.ratio = d.ratio / totalRatio;
        const int cent = qRound(d.ratio * 100);
        d.percent = QString::number(cent) + QStringLiteral("%");
    }
};
