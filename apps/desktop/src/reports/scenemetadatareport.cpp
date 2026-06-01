/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "scenemetadatareport.h"

#include "scene.h"
#include "screenplay.h"
#include "structure.h"
#include "scritedocument.h"
#include "screenplaypaginator.h"

#include <QFile>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QRandomGenerator>
#include <QScopedPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QTextTable>

#include <OpenXLSX.hpp>

namespace {
const QString COMMA_SPACE = QStringLiteral(", ");
const QString DASH = QStringLiteral("-");
const QString SLASH_EIGHT = QStringLiteral("/8");
const QString COLON_SPACE = QStringLiteral(": ");
const QString ACT_PREFIX = QStringLiteral("ACT ");
const QString EPISODE_PREFIX = QStringLiteral("EPISODE ");

// Column headers
const QString COL_INT_EXT = QStringLiteral("INT/EXT");
const QString COL_LOCATION = QStringLiteral("Location Name");
const QString COL_TIME_OF_DAY = QStringLiteral("Time of Day");
const QString COL_ACT = QStringLiteral("Act #");
const QString COL_EPISODE = QStringLiteral("Episode #");
const QString COL_SCENE_NUM = QStringLiteral("Scene #");
const QString COL_SYNOPSIS = QStringLiteral("Synopsis");
const QString COL_GROUPS = QStringLiteral("Formal Tags");
const QString COL_KEYWORDS = QStringLiteral("Keywords");
const QString COL_START_PAGE = QStringLiteral("Start Page");
const QString COL_PAGE_COUNT = QStringLiteral("Page Count (1/8)");
const QString COL_SCENE_TIME = QStringLiteral("Scene Time");
const QString COL_CHARACTERS = QStringLiteral("Characters");

// Column keys
const QString COL_KEY_INT_EXT = QStringLiteral("intExt");
const QString COL_KEY_LOCATION = QStringLiteral("location");
const QString COL_KEY_TIME_OF_DAY = QStringLiteral("timeOfDay");
const QString COL_KEY_SCENE_NUM = QStringLiteral("sceneNum");
const QString COL_KEY_ACT_NUM = QStringLiteral("actNum");
const QString COL_KEY_EPISODE_NUM = QStringLiteral("episodeNum");
const QString COL_KEY_SYNOPSIS = QStringLiteral("synopsis");
const QString COL_KEY_GROUPS = QStringLiteral("groups");
const QString COL_KEY_KEYWORDS = QStringLiteral("keywords");
const QString COL_KEY_START_PAGE = QStringLiteral("startPage");
const QString COL_KEY_PAGE_COUNT = QStringLiteral("pageCount");
const QString COL_KEY_SCENE_TIME = QStringLiteral("sceneTime");
const QString COL_KEY_CHARACTERS = QStringLiteral("characters");
} // namespace

Q_DECL_IMPORT int qt_defaultDpi();

SceneMetadataReport::SceneMetadataReport(QObject *parent) : AbstractReportGenerator(parent)
{
    this->setFormat(OpenDocumentFormat);
}

SceneMetadataReport::~SceneMetadataReport() { }

QString SceneMetadataReport::personalizedFileName(const QString &fileName) const
{
    QString appendix;

    if (!m_episodeNumbers.isEmpty()) {
        QStringList epStrings;
        epStrings.reserve(m_episodeNumbers.size());
        for (int ep : m_episodeNumbers)
            epStrings << QString::number(ep);
        appendix = QStringLiteral("Ep ") + listToPersonalizedNameString(epStrings);
    } else if (!m_tags.isEmpty()) {
        appendix = tagsToPersonalizedNameString(m_tags);
    } else if (!m_keywords.trimmed().isEmpty()) {
        const QString kw = m_keywords.trimmed();
        appendix = kw.length() > 40 ? kw.left(37) + QStringLiteral("...") : kw;
    } else if (!m_sceneNumbers.isEmpty()) {
        if (m_sceneNumbers.size() <= 3) {
            QStringList scStrings;
            scStrings.reserve(m_sceneNumbers.size());
            for (int sc : m_sceneNumbers)
                scStrings << QString::number(sc);
            appendix = QStringLiteral("Scenes ") + scStrings.join(QStringLiteral(", "));
        } else {
            appendix = QString::number(m_sceneNumbers.size()) + QStringLiteral(" Scenes");
        }
    }

    if (appendix.isEmpty())
        return fileName;

    return buildPersonalizedFileName(fileName, appendix);
}

void SceneMetadataReport::setEpisodeNumbers(const QList<int> &val)
{
    if (m_episodeNumbers == val)
        return;

    m_episodeNumbers = val;
    emit episodeNumbersChanged();
}

void SceneMetadataReport::setSceneNumbers(const QList<int> &val)
{
    if (m_sceneNumbers == val)
        return;

    m_sceneNumbers = val;
    emit sceneNumbersChanged();
}

void SceneMetadataReport::setTags(const QStringList &val)
{
    if (m_tags == val)
        return;

    m_tags = val;
    emit tagsChanged();
}

void SceneMetadataReport::setKeywords(const QString &val)
{
    if (m_keywords == val)
        return;

    m_keywords = val;
    emit keywordsChanged();
}

void SceneMetadataReport::setShowSynopsisColumn(bool val)
{
    if (m_showSynopsisColumn == val)
        return;

    m_showSynopsisColumn = val;
    emit showSynopsisColumnChanged();
}

void SceneMetadataReport::setShowGroupsColumn(bool val)
{
    if (m_showGroupsColumn == val)
        return;

    m_showGroupsColumn = val;
    emit showGroupsColumnChanged();
}

void SceneMetadataReport::setShowKeywordsColumn(bool val)
{
    if (m_showKeywordsColumn == val)
        return;

    m_showKeywordsColumn = val;
    emit showKeywordsColumnChanged();
}

void SceneMetadataReport::setShowStartPageColumn(bool val)
{
    if (m_showStartPageColumn == val)
        return;

    m_showStartPageColumn = val;
    emit showStartPageColumnChanged();
}

void SceneMetadataReport::setShowPageCountColumn(bool val)
{
    if (m_showPageCountColumn == val)
        return;

    m_showPageCountColumn = val;
    emit showPageCountColumnChanged();
}

void SceneMetadataReport::setShowSceneTimeColumn(bool val)
{
    if (m_showSceneTimeColumn == val)
        return;

    m_showSceneTimeColumn = val;
    emit showSceneTimeColumnChanged();
}

void SceneMetadataReport::setShowCharactersColumn(bool val)
{
    if (m_showCharactersColumn == val)
        return;

    m_showCharactersColumn = val;
    emit showCharactersColumnChanged();
}

QString SceneMetadataReport::fileNameExtension() const
{
    return this->format() == OpenDocumentFormat ? QStringLiteral("xlsx") : QStringLiteral("pdf");
}

bool SceneMetadataReport::passesFilter(const ScreenplayElement *element) const
{
    if (element->scene() == nullptr || element->isOmitted())
        return false;

    // Filter by scene numbers
    if (!m_sceneNumbers.isEmpty() && !m_sceneNumbers.contains(element->elementIndex()))
        return false;

    // Filter by episodes
    if (!m_episodeNumbers.isEmpty()
        && element->elementType() == ScreenplayElement::SceneElementType) {
        const Screenplay *screenplay = this->document()->screenplay();
        const bool hasEpisodes = screenplay->episodeCount() > 0;

        if (hasEpisodes) {
            int currentEpisode = 0;
            bool foundInEpisode = false;

            for (int i = 0; i < screenplay->elementCount(); i++) {
                if (screenplay->elementAt(i) == element) {
                    if (i == 0)
                        ++currentEpisode;
                    foundInEpisode = m_episodeNumbers.contains(currentEpisode);
                    break;
                }

                if (screenplay->elementAt(i)->elementType() == ScreenplayElement::BreakElementType
                    && screenplay->elementAt(i)->breakType() == Screenplay::Episode)
                    ++currentEpisode;
            }

            if (!foundInEpisode)
                return false;
        }
    }

    // Filter by tags
    if (!m_tags.isEmpty()) {
        const Scene *scene = element->scene();
        const QStringList sceneTags = scene->groups();

        bool hasMatchingTag = false;
        for (const QString &sceneTag : sceneTags) {
            if (m_tags.contains(sceneTag)) {
                hasMatchingTag = true;
                break;
            }
        }

        if (!hasMatchingTag)
            return false;
    }

    // Filter by keywords
    if (!m_keywords.isEmpty()) {
        const Scene *scene = element->scene();
        const QStringList keywordList = m_keywords.split(QStringLiteral(","), Qt::SkipEmptyParts);

        bool foundKeyword = false;
        for (const QString &keyword : keywordList) {
            const QString trimmed = keyword.trimmed().toLower();
            if (scene->heading()->text().toLower().contains(trimmed)
                || scene->synopsis().toLower().contains(trimmed)) {
                foundKeyword = true;
                break;
            }
        }

        if (!foundKeyword && !keywordList.isEmpty())
            return false;
    }

    return true;
}

bool SceneMetadataReport::doGenerate(QTextDocument *document)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const ScreenplayFormat *format = this->document()->printFormat();
    QList<ScreenplayElement *> screenplayElements = this->getScreenplayElements();

    // Create paginated document for page/time calculations
    QScopedPointer<QTextDocument> paginatedDoc(
            ScreenplayPaginator::paginatedDocument(screenplay, format));
    if (!paginatedDoc)
        return false;

    const QFont defaultFont = format->defaultFont();

    QTextBlockFormat defaultBlockFormat;

    QTextCharFormat defaultCharFormat;
    defaultCharFormat.setFontFamilies({ defaultFont.family() });
    defaultCharFormat.setFontPointSize(12);

    QTextCursor cursor(document);
    cursor.movePosition(QTextCursor::Start);

    QTextBlockFormat blockFormat = defaultBlockFormat;
    blockFormat.setAlignment(Qt::AlignHCenter);
    blockFormat.setTopMargin(5);
    blockFormat.setBottomMargin(3);
    QTextCharFormat charFormat = defaultCharFormat;
    charFormat.setFontPointSize(16);
    charFormat.setFontWeight(QFont::Bold);
    cursor.insertBlock(blockFormat, charFormat);
    cursor.insertText(QStringLiteral("Scene Metadata Report"));

    blockFormat = defaultBlockFormat;
    blockFormat.setAlignment(Qt::AlignHCenter);
    blockFormat.setBottomMargin(2);
    charFormat = defaultCharFormat;
    charFormat.setFontPointSize(10);
    cursor.insertBlock(blockFormat, charFormat);

    if (!m_episodeNumbers.isEmpty() || !m_sceneNumbers.isEmpty() || !m_tags.isEmpty()
        || !m_keywords.isEmpty()) {
        QStringList filters;

        if (!m_episodeNumbers.isEmpty()) {
            QStringList epNos;
            for (int epNo : std::as_const(m_episodeNumbers))
                epNos << QString::number(epNo);
            filters << QStringLiteral("Episode(s): ") + epNos.join(COMMA_SPACE);
        }

        if (!m_sceneNumbers.isEmpty()) {
            filters << QStringLiteral("Selected scenes");
        }

        if (!m_tags.isEmpty()) {
            filters << QStringLiteral("Tag(s): ") + m_tags.join(COMMA_SPACE);
        }

        if (!m_keywords.isEmpty()) {
            filters << QStringLiteral("Keywords: ") + m_keywords;
        }

        cursor.insertText(filters.join(QStringLiteral(" | ")));
    } else {
        cursor.insertText(QStringLiteral("All scenes"));
    }

    blockFormat = defaultBlockFormat;
    blockFormat.setAlignment(Qt::AlignHCenter);
    blockFormat.setBottomMargin(2);
    charFormat = defaultCharFormat;
    cursor.insertBlock(blockFormat, charFormat);
    cursor.insertHtml("This report was generated using <strong>Scrite</strong><br/>(<a "
                      "href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
    cursor.insertBlock(blockFormat, charFormat);
    cursor.insertText("--");

    // Build list of visible columns dynamically
    struct ColumnInfo
    {
        QString header;
        QString key;
        int defaultWidth;
    };

    const QHash<QString, int> stdColumnWidths = { { COL_LOCATION, 100 },    { COL_SYNOPSIS, 300 },
                                                  { COL_CHARACTERS, 150 },  { COL_INT_EXT, 50 },
                                                  { COL_TIME_OF_DAY, 100 }, { COL_KEYWORDS, 75 },
                                                  { COL_GROUPS, 75 } };

    QList<ColumnInfo> columns = {
        { COL_INT_EXT, COL_KEY_INT_EXT, 50 },          { COL_LOCATION, COL_KEY_LOCATION, 100 },
        { COL_TIME_OF_DAY, COL_KEY_TIME_OF_DAY, 100 }, { COL_SCENE_NUM, COL_KEY_SCENE_NUM, -1 },
        { COL_SYNOPSIS, COL_KEY_SYNOPSIS, 300 },       { COL_GROUPS, COL_KEY_GROUPS, 75 },
        { COL_KEYWORDS, COL_KEY_KEYWORDS, 75 },        { COL_START_PAGE, COL_KEY_START_PAGE, -1 },
        { COL_PAGE_COUNT, COL_KEY_PAGE_COUNT, -1 },    { COL_SCENE_TIME, COL_KEY_SCENE_TIME, -1 },
        { COL_CHARACTERS, COL_KEY_CHARACTERS, 150 },
    };

    // Filter to visible columns only
    QList<ColumnInfo> visibleColumns;
    for (const auto &col : std::as_const(columns)) {
        // First 4 columns are always visible
        if (col.key == COL_KEY_INT_EXT || col.key == COL_KEY_LOCATION
            || col.key == COL_KEY_TIME_OF_DAY || col.key == COL_KEY_SCENE_NUM) {
            visibleColumns.append(col);
        } else if (col.key == COL_KEY_SYNOPSIS && m_showSynopsisColumn) {
            visibleColumns.append(col);
        } else if (col.key == COL_KEY_GROUPS && m_showGroupsColumn) {
            visibleColumns.append(col);
        } else if (col.key == COL_KEY_KEYWORDS && m_showKeywordsColumn) {
            visibleColumns.append(col);
        } else if (col.key == COL_KEY_START_PAGE && m_showStartPageColumn) {
            visibleColumns.append(col);
        } else if (col.key == COL_KEY_PAGE_COUNT && m_showPageCountColumn) {
            visibleColumns.append(col);
        } else if (col.key == COL_KEY_SCENE_TIME && m_showSceneTimeColumn) {
            visibleColumns.append(col);
        } else if (col.key == COL_KEY_CHARACTERS && m_showCharactersColumn) {
            visibleColumns.append(col);
        }
    }

    if (screenplay->actCount() > 0) {
        visibleColumns.insert(4, { COL_ACT, COL_KEY_ACT_NUM, 100 });
    }

    if (screenplay->episodeCount() > 0) {
        visibleColumns.insert(4, { COL_EPISODE, COL_KEY_EPISODE_NUM, 100 });
    }

    QTextTableFormat tableFormat;
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(5);
    tableFormat.setBorder(1);
    tableFormat.setBorderCollapse(true);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setHeaderRowCount(1);
    tableFormat.setAlignment(Qt::AlignHCenter);
    tableFormat.setTopMargin(0);
    tableFormat.setBottomMargin(0);

    // Set column width constraints based on visible columns
    QVector<QTextLength> columnWidths;
    for (const auto &col : std::as_const(visibleColumns)) {
        if (col.defaultWidth > 0)
            columnWidths.append(QTextLength(QTextLength::FixedLength, col.defaultWidth));
        else
            columnWidths.append(QTextLength(QTextLength::VariableLength, 1));
    }
    tableFormat.setColumnWidthConstraints(columnWidths);

    QTextTable *table =
            cursor.insertTable(screenplayElements.size() + 1, visibleColumns.size(), tableFormat);

    // Create cell format with borders
    QTextTableCellFormat cellFormatWithBorders;
    cellFormatWithBorders.setBorder(1);
    cellFormatWithBorders.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);

    // Write headers
    for (int col = 0; col < visibleColumns.size(); col++) {
        QTextTableCell cell = table->cellAt(0, col);
        cell.setFormat(cellFormatWithBorders);

        QTextCursor cellCursor = cell.firstCursorPosition();

        QTextCharFormat headerFormat = defaultCharFormat;
        headerFormat.setFontWeight(QFont::Bold);
        cellCursor.setCharFormat(headerFormat);

        cellCursor.insertText(visibleColumns.at(col).header);
    }

    // Write data rows
    int currentEpisodeNr = 0, currentActNr = 0;
    ScreenplayElement *currentEpisode = nullptr, *currentAct = nullptr;
    for (int row = 0; row < screenplayElements.size(); row++) {
        ScreenplayElement *element = screenplayElements.at(row);

        if (element->elementType() == ScreenplayElement::BreakElementType) {
            if (element->breakType() == Screenplay::Act) {
                currentAct = element;
                ++currentActNr;
            } else if (element->breakType() == Screenplay::Episode) {
                currentEpisode = element;
                ++currentEpisodeNr;
                currentActNr = 1;
            }
            continue;
        }

        Scene *scene = element->scene();
        if (!scene)
            continue;

        if (currentEpisodeNr <= 0)
            currentEpisodeNr = 1;
        if (currentActNr <= 0)
            currentActNr = 1;

        // Get page and time info from paginated document
        const qreal pixelLength = ScreenplayPaginator::pixelLength(element, paginatedDoc.data());
        const QTime timeLength =
                ScreenplayPaginator::pixelToTimeLength(pixelLength, format, paginatedDoc.data());

        // Calculate cumulative pixel for start page (used by multiple columns)
        qreal cumulativePixel = 0;
        for (int i = 0; i < row; i++) {
            cumulativePixel +=
                    ScreenplayPaginator::pixelLength(screenplayElements.at(i), paginatedDoc.data());
        }
        const int startPage = static_cast<int>(ScreenplayPaginator::pixelToPageLength(
                                      cumulativePixel, paginatedDoc.data()))
                + 1;

        // Populate visible columns
        for (int col = 0; col < visibleColumns.size(); col++) {
            QTextTableCell cell = table->cellAt(row + 1, col);
            cell.setFormat(cellFormatWithBorders);
            QTextCursor cellCursor = cell.firstCursorPosition();

            const QString &colKey = visibleColumns.at(col).key;

            if (colKey == COL_KEY_INT_EXT) {
                cellCursor.insertText(
                        scene->heading()->isEnabled() ? scene->heading()->locationType() : DASH);
            } else if (colKey == COL_KEY_LOCATION) {
                cellCursor.insertText(scene->heading()->isEnabled() ? scene->heading()->location()
                                                                    : DASH);
            } else if (colKey == COL_KEY_TIME_OF_DAY) {
                cellCursor.insertText(scene->heading()->isEnabled() ? scene->heading()->moment()
                                                                    : DASH);
            } else if (colKey == COL_KEY_EPISODE_NUM) {
                QString epLabel;
                if (currentEpisode) {
                    epLabel = currentEpisode->breakTitle();
                    if (!currentEpisode->breakSubtitle().isEmpty())
                        epLabel += COLON_SPACE + currentEpisode->breakSubtitle();
                } else
                    epLabel = EPISODE_PREFIX + QString::number(currentEpisodeNr);
                cellCursor.insertText(epLabel);
            } else if (colKey == COL_KEY_ACT_NUM) {
                QString actLabel;
                if (currentAct) {
                    actLabel = currentAct->breakTitle();
                    if (!currentAct->breakSubtitle().isEmpty())
                        actLabel += COLON_SPACE + currentAct->breakSubtitle();
                } else
                    actLabel = ACT_PREFIX + QString::number(currentActNr);
                cellCursor.insertText(actLabel);
            } else if (colKey == COL_KEY_SCENE_NUM) {
                cellCursor.insertText(element->resolvedSceneNumber());
            } else if (colKey == COL_KEY_SYNOPSIS) {
                cellCursor.insertText(scene->synopsis());
            } else if (colKey == COL_KEY_GROUPS) {
                cellCursor.insertText(scene->groups().join(COMMA_SPACE));
            } else if (colKey == COL_KEY_KEYWORDS) {
                cellCursor.insertText(scene->tags().join(COMMA_SPACE));
            } else if (colKey == COL_KEY_START_PAGE) {
                cellCursor.insertText(QString::number(startPage));
            } else if (colKey == COL_KEY_PAGE_COUNT) {
                cellCursor.insertText(ScreenplayPaginator::pixelToPageLength1_8(
                        pixelLength, paginatedDoc.data()));
            } else if (colKey == COL_KEY_SCENE_TIME) {
                cellCursor.insertText(Utils::TMath::timeLengthString(timeLength));
            } else if (colKey == COL_KEY_CHARACTERS) {
                cellCursor.insertText(scene->characterNames().join(COMMA_SPACE));
            }
        }
    }

    return true;
}

void SceneMetadataReport::configureWriter(QPdfWriter *pdfWriter,
                                           const QTextDocument *document) const
{
    this->configureWriterImpl(pdfWriter, document);
}

void SceneMetadataReport::configureWriter(QPrinter *printer, const QTextDocument *document) const
{
    this->configureWriterImpl(printer, document);
}

bool SceneMetadataReport::canDirectExportToOdf() const
{
    return true;
}

bool SceneMetadataReport::directExportToOdf(QIODevice *device)
{
    try {
        const Screenplay *screenplay = this->document()->screenplay();
        const ScreenplayFormat *format = this->document()->printFormat();
        QList<ScreenplayElement *> screenplayElements = this->getScreenplayElements();

        // Create paginated document for page/time calculations
        QScopedPointer<QTextDocument> paginatedDoc(
                ScreenplayPaginator::paginatedDocument(screenplay, format));
        if (!paginatedDoc)
            return false;

        const QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                + QStringLiteral("/") + QStringLiteral("scrite_breakdown_")
                + QString::number(QRandomGenerator::global()->generate()) + QStringLiteral(".xlsx");

        OpenXLSX::XLDocument doc;
        doc.create(tempPath.toStdString(), true);
        OpenXLSX::XLWorksheet ws = doc.workbook().sheet(1);

        auto &styles = doc.styles();

        OpenXLSX::XLStyleIndex boldFontIndex = styles.fonts().create();
        OpenXLSX::XLFont boldFont = styles.fonts()[boldFontIndex];
        boldFont.setBold(true);

        OpenXLSX::XLStyleIndex boldStyleIndex = styles.cellFormats().create();
        OpenXLSX::XLCellFormat boldCellFormat = styles.cellFormats()[boldStyleIndex];
        boldCellFormat.setFontIndex(boldFontIndex);
        boldCellFormat.setApplyFont(true);

        OpenXLSX::XLStyleIndex centerStyleIndex = styles.cellFormats().create();
        OpenXLSX::XLCellFormat centerCellFormat = styles.cellFormats()[centerStyleIndex];
        centerCellFormat.alignment(OpenXLSX::XLCreateIfMissing)
                .setHorizontal(OpenXLSX::XLAlignCenter);
        centerCellFormat.setApplyAlignment(true);

        OpenXLSX::XLStyleIndex wrapTextStyleIndex = styles.cellFormats().create();
        OpenXLSX::XLCellFormat wrapTextCellFormat = styles.cellFormats()[wrapTextStyleIndex];
        auto wrapAlign = wrapTextCellFormat.alignment(OpenXLSX::XLCreateIfMissing);
        wrapAlign.setWrapText(true);
        wrapAlign.setVertical(OpenXLSX::XLAlignCenter);
        wrapTextCellFormat.setApplyAlignment(true);

        OpenXLSX::XLStyleIndex dataStyleIndex = styles.cellFormats().create();
        OpenXLSX::XLCellFormat dataCellFormat = styles.cellFormats()[dataStyleIndex];
        auto dataAlign = dataCellFormat.alignment(OpenXLSX::XLCreateIfMissing);
        dataAlign.setIndent(1);
        dataAlign.setVertical(OpenXLSX::XLAlignCenter);
        dataCellFormat.setApplyAlignment(true);

        uint32_t rowIndex = 1;
        uint32_t colIndex = 1;

        // Build list of visible columns dynamically
        struct ColumnInfo
        {
            QString header;
            QString key;
        };

        QList<ColumnInfo> columns = {
            { COL_INT_EXT, COL_KEY_INT_EXT },         { COL_LOCATION, COL_KEY_LOCATION },
            { COL_TIME_OF_DAY, COL_KEY_TIME_OF_DAY }, { COL_SCENE_NUM, COL_KEY_SCENE_NUM },
            { COL_SYNOPSIS, COL_KEY_SYNOPSIS },       { COL_GROUPS, COL_KEY_GROUPS },
            { COL_KEYWORDS, COL_KEY_KEYWORDS },       { COL_START_PAGE, COL_KEY_START_PAGE },
            { COL_PAGE_COUNT, COL_KEY_PAGE_COUNT },   { COL_SCENE_TIME, COL_KEY_SCENE_TIME },
            { COL_CHARACTERS, COL_KEY_CHARACTERS },
        };

        // Filter to visible columns only
        QList<ColumnInfo> visibleColumns;
        for (const auto &col : std::as_const(columns)) {
            // First 4 columns are always visible
            if (col.key == COL_KEY_INT_EXT || col.key == COL_KEY_LOCATION
                || col.key == COL_KEY_TIME_OF_DAY || col.key == COL_KEY_SCENE_NUM) {
                visibleColumns.append(col);
            } else if (col.key == COL_KEY_SYNOPSIS && m_showSynopsisColumn) {
                visibleColumns.append(col);
            } else if (col.key == COL_KEY_GROUPS && m_showGroupsColumn) {
                visibleColumns.append(col);
            } else if (col.key == COL_KEY_KEYWORDS && m_showKeywordsColumn) {
                visibleColumns.append(col);
            } else if (col.key == COL_KEY_START_PAGE && m_showStartPageColumn) {
                visibleColumns.append(col);
            } else if (col.key == COL_KEY_PAGE_COUNT && m_showPageCountColumn) {
                visibleColumns.append(col);
            } else if (col.key == COL_KEY_SCENE_TIME && m_showSceneTimeColumn) {
                visibleColumns.append(col);
            } else if (col.key == COL_KEY_CHARACTERS && m_showCharactersColumn) {
                visibleColumns.append(col);
            }
        }

        if (screenplay->actCount() > 0) {
            visibleColumns.insert(4, { COL_ACT, COL_KEY_ACT_NUM });
        }

        if (screenplay->episodeCount() > 0) {
            visibleColumns.insert(4, { COL_EPISODE, COL_KEY_EPISODE_NUM });
        }

        // Track maximum text length in each column for auto-sizing
        QVector<int> columnMaxWidths(visibleColumns.size(), 0);
        for (int i = 0; i < visibleColumns.size(); ++i) {
            columnMaxWidths[i] = visibleColumns.at(i).header.length();
        }

        // Write headers
        for (int headerCol = 0; headerCol < visibleColumns.size(); headerCol++) {
            auto cell = ws.cell(rowIndex, headerCol + 1);
            cell.value() = visibleColumns.at(headerCol).header.toStdString();

            OpenXLSX::XLStyleIndex headerStyleIndex = styles.cellFormats().create();
            OpenXLSX::XLCellFormat headerCellFormat = styles.cellFormats()[headerStyleIndex];
            headerCellFormat.setFontIndex(boldFontIndex);
            headerCellFormat.setApplyFont(true);
            auto headerAlign = headerCellFormat.alignment(OpenXLSX::XLCreateIfMissing);
            headerAlign.setIndent(1);
            headerAlign.setVertical(OpenXLSX::XLAlignCenter);
            headerCellFormat.setApplyAlignment(true);

            cell.setCellFormat(headerStyleIndex);
        }
        rowIndex++;

        // Set row height for better readability with padding
        ws.row(1).setHeight(25); // Header row

        QFontMetricsF fontMetrics(qApp->font());

        // Write data rows
        int sceneRow = 0;
        int currentEpisodeNr = 0, currentActNr = 0;
        ScreenplayElement *currentEpisode = nullptr, *currentAct = nullptr;
        for (ScreenplayElement *element : std::as_const(screenplayElements)) {
            if (element->elementType() == ScreenplayElement::BreakElementType) {
                if (element->breakType() == Screenplay::Act) {
                    currentAct = element;
                    ++currentActNr;
                } else if (element->breakType() == Screenplay::Episode) {
                    currentEpisode = element;
                    ++currentEpisodeNr;
                    currentActNr = 1;
                }
                continue;
            }

            Scene *scene = element->scene();
            if (!scene)
                continue;

            if (currentEpisodeNr <= 0)
                currentEpisodeNr = 1;
            if (currentActNr <= 0)
                currentActNr = 1;

            // Get page and time info from paginated document
            const qreal pixelLength =
                    ScreenplayPaginator::pixelLength(element, paginatedDoc.data());
            const qreal pageLength =
                    ScreenplayPaginator::pixelToPageLength(pixelLength, paginatedDoc.data());
            const QTime timeLength = ScreenplayPaginator::pixelToTimeLength(pixelLength, format,
                                                                            paginatedDoc.data());

            // Calculate cumulative pixel for start page (used by column)
            qreal cumulativePixel = 0;
            for (int i = 0; i < sceneRow; i++) {
                cumulativePixel += ScreenplayPaginator::pixelLength(screenplayElements.at(i),
                                                                    paginatedDoc.data());
            }
            const int startPage = static_cast<int>(ScreenplayPaginator::pixelToPageLength(
                                          cumulativePixel, paginatedDoc.data()))
                    + 1;

            // Populate visible columns
            const qreal synopsisColumnWidth = 300;
            float rowHeight = 30;
            for (int col = 0; col < visibleColumns.size(); ++col) {
                const QString &colKey = visibleColumns.at(col).key;
                QString cellValue;

                if (colKey == COL_KEY_INT_EXT) {
                    cellValue =
                            scene->heading()->isEnabled() ? scene->heading()->locationType() : DASH;
                } else if (colKey == COL_KEY_LOCATION) {
                    cellValue = scene->heading()->isEnabled() ? scene->heading()->location() : DASH;
                } else if (colKey == COL_KEY_TIME_OF_DAY) {
                    cellValue = scene->heading()->isEnabled() ? scene->heading()->moment() : DASH;
                } else if (colKey == COL_KEY_EPISODE_NUM) {
                    if (currentEpisode) {
                        cellValue = currentEpisode->breakTitle();
                        if (!currentEpisode->breakSubtitle().isEmpty())
                            cellValue += COLON_SPACE + currentEpisode->breakSubtitle();
                    } else
                        cellValue = EPISODE_PREFIX + QString::number(currentEpisodeNr);
                } else if (colKey == COL_KEY_ACT_NUM) {
                    if (currentAct) {
                        cellValue = currentAct->breakTitle();
                        if (!currentAct->breakSubtitle().isEmpty())
                            cellValue += COLON_SPACE + currentAct->breakSubtitle();
                    } else
                        cellValue = ACT_PREFIX + QString::number(currentActNr);
                } else if (colKey == COL_KEY_SCENE_NUM) {
                    cellValue = element->resolvedSceneNumber();
                } else if (colKey == COL_KEY_SYNOPSIS) {
                    cellValue = scene->synopsis();
                    if (!cellValue.isEmpty()
                        && fontMetrics.horizontalAdvance(cellValue) >= synopsisColumnWidth)
                        rowHeight += (float)fontMetrics
                                             .boundingRect(QRectF(0, 0, synopsisColumnWidth, 20000),
                                                           Qt::TextWordWrap, cellValue)
                                             .height();
                } else if (colKey == COL_KEY_GROUPS) {
                    cellValue = scene->groups().join(COMMA_SPACE);
                } else if (colKey == COL_KEY_KEYWORDS) {
                    cellValue = scene->tags().join(COMMA_SPACE);
                } else if (colKey == COL_KEY_START_PAGE) {
                    cellValue = QString::number(startPage);
                } else if (colKey == COL_KEY_PAGE_COUNT) {
                    cellValue = ScreenplayPaginator::pixelToPageLength1_8(pixelLength,
                                                                          paginatedDoc.data());
                } else if (colKey == COL_KEY_SCENE_TIME) {
                    cellValue = Utils::TMath::timeLengthString(timeLength);
                } else if (colKey == COL_KEY_CHARACTERS) {
                    cellValue = scene->characterNames().join(COMMA_SPACE);
                }

                auto cell = ws.cell(rowIndex, col + 1);
                cell.value() = cellValue.toStdString();

                // Use wrap text style for synopsis, data style for others
                if (colKey == COL_KEY_SYNOPSIS)
                    cell.setCellFormat(wrapTextStyleIndex);
                else
                    cell.setCellFormat(dataStyleIndex);

                if (colKey != COL_KEY_SYNOPSIS)
                    columnMaxWidths[col] = std::max(columnMaxWidths[col], (int)cellValue.length());

                ws.row(rowIndex).setHeight(rowHeight);
            }

            rowIndex++;
            sceneRow++;
        }

        // Configure column widths based on content
        for (int col = 0; col < visibleColumns.size(); ++col) {
            if (visibleColumns.at(col).key == COL_KEY_SYNOPSIS) {
                // Synopsis - fixed 300px (approx 42 character widths)
                ws.column(col + 1).setWidth(42);
            } else {
                // All other columns: content width + 4 extra characters for breathing space
                double columnWidth = columnMaxWidths[col] + 4.0;
                ws.column(col + 1).setWidth(columnWidth);
            }
        }

        doc.save();

        QFile tempFile(tempPath);
        if (!tempFile.open(QIODevice::ReadOnly)) {
            return false;
        }

        const QByteArray data = tempFile.readAll();
        tempFile.close();
        tempFile.remove();

        if (device->write(data) < 0)
            return false;

        return true;
    } catch (const std::exception &e) {
        qWarning() << "Error generating XLSX:" << QString::fromStdString(e.what());
        return false;
    }
}

void SceneMetadataReport::configureWriterImpl(QPagedPaintDevice *ppd,
                                               const QTextDocument *document) const
{
    // Use A3 landscape page size for scene metadata report
    ppd->setPageSize(QPageSize::A3);
    ppd->setPageOrientation(QPageLayout::Landscape);
}

QList<ScreenplayElement *> SceneMetadataReport::getScreenplayElements()
{
    const Screenplay *screenplay = this->document()->screenplay();
    QList<ScreenplayElement *> screenplayElements;

    for (int i = 0; i < screenplay->elementCount(); i++) {
        ScreenplayElement *element = screenplay->elementAt(i);

        if (element->elementType() == ScreenplayElement::BreakElementType
            || this->passesFilter(element))
            screenplayElements.append(element);
    }

    return screenplayElements;
}
