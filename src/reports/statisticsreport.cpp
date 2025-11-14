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
    if (group == Character)
        return QVector<QColor>({ QColor(94, 54, 138), QColor(232, 191, 90) });
    if (group == Beat)
        return QVector<QColor>(
                { QColor(206, 229, 208), QColor(243, 240, 215), QColor(254, 210, 170),
                  QColor(255, 191, 134), QColor(219, 208, 192), QColor(250, 238, 224),
                  QColor(249, 228, 200), QColor(249, 207, 147), QColor(121, 180, 183),
                  QColor(254, 251, 243), QColor(248, 240, 223), QColor(157, 157, 157),
                  QColor(255, 230, 153), QColor(255, 249, 182), QColor(255, 146, 146),
                  QColor(255, 204, 210), QColor(233, 148, 151), QColor(243, 197, 131),
                  QColor(232, 228, 110), QColor(179, 226, 131), QColor(134, 122, 233),
                  QColor(255, 245, 171), QColor(255, 206, 173), QColor(196, 73, 194) });

    return QVector<QColor>({ QColor(134, 72, 121), QColor(63, 51, 81) });
}

const QColor StatisticsReport::pickColor(int index, bool cycleAround, ColorGroup group)
{
    const QVector<QColor> colors = StatisticsReport::colors(group);
    const QColor baseColor = colors.at(index % (colors.length()));
    index = qAbs(index);

    if (cycleAround || index < colors.size())
        return baseColor;

    const int batch = qFloor(qreal(index) / colors.size());
    const bool lighter = batch % 2;
    const int factor = 100 + batch * 50;
    return lighter ? baseColor.lighter(factor) : baseColor.darker(factor);
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
            item.color = StatisticsReport::pickColor(actIndex++);
        else {
            if (actIndex > 0 && item.key == actName(0))
                baseColor = StatisticsReport::pickColor(++episodeIndex);

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
        item.color = StatisticsReport::pickColor(epIndex++);

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
    cursor.insertHtml(QStringLiteral(
            "<font size=\"-2\">Page count may change in generated PDFs of the screenplay.</font>"));

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
