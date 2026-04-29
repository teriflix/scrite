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

#include "scenecharactermatrixreport.h"
#include "application.h"

#include "scene.h"
#include "screenplay.h"
#include "structure.h"
#include "scritedocument.h"

#include <QPrinter>
#include <QPainter>
#include <QSettings>
#include <QPdfWriter>
#include <QTextTable>
#include <QFile>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <OpenXLSX.hpp>

SceneCharacterMatrixReport::SceneCharacterMatrixReport(QObject *parent)
    : AbstractReportGenerator(parent)
{
    this->setFormat(OpenDocumentFormat);

    connect(this, &AbstractReportGenerator::documentChanged, [=]() {
        if (this->document() != nullptr)
            this->setCharacterNames(this->document()->structure()->characterNames());
    });
}

SceneCharacterMatrixReport::~SceneCharacterMatrixReport() { }

void SceneCharacterMatrixReport::setType(int val)
{
    if (m_type == val)
        return;

    if (val != SceneVsCharacter && val != CharacterVsScene)
        return;

    m_type = val;
    emit typeChanged();
}

void SceneCharacterMatrixReport::setMarker(const QString &val)
{
    if (m_marker == val)
        return;

    m_marker = val;
    emit markerChanged();
}

void SceneCharacterMatrixReport::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void SceneCharacterMatrixReport::setEpisodeNumbers(const QList<int> &val)
{
    if (m_episodeNumbers == val)
        return;

    m_episodeNumbers = val;
    emit episodeNumbersChanged();
}

void SceneCharacterMatrixReport::setTags(const QStringList &val)
{
    if (m_tags == val)
        return;

    m_tags = val;
    emit tagsChanged();
}

QString SceneCharacterMatrixReport::fileNameExtension() const
{
    return this->format() == OpenDocumentFormat ? QStringLiteral("xlsx") : QStringLiteral("pdf");
}

struct CreateColumnHeadingImageFunctor
{
    CreateColumnHeadingImageFunctor(const QFont &font) : font(font), fontMetrics(font) { }

    QTransform transform;
    QFont font;
    QFontMetrics fontMetrics;
    QBrush background = QBrush(Qt::white);
    typedef QImage result_type;

    QImage operator()(const QString &text)
    {
        const QRect textRect = fontMetrics.boundingRect(text);
        const qreal dpr = 2.0;

        QImage image(textRect.size() * dpr, QImage::Format_ARGB32);
        image.setDevicePixelRatio(dpr);
        image.fill(background.color());

        QPainter paint(&image);
        paint.setFont(font);
        paint.setPen(Qt::black);
        paint.drawText(QRect(0, 0, textRect.width(), textRect.height()), Qt::AlignCenter, text);
        paint.end();

        if (!transform.isIdentity())
            image = image.transformed(transform);

        return image;
    }
};

bool SceneCharacterMatrixReport::doGenerate(QTextDocument *document)
{
    const Screenplay *screenplay = this->document()->screenplay();
    QList<ScreenplayElement *> allScreenplayElements = this->getScreenplayElements();
    QList<ScreenplayElement *> screenplayElements; // ones that are not emotted
    std::copy_if(allScreenplayElements.begin(), allScreenplayElements.end(),
                 std::back_inserter(screenplayElements),
                 [](ScreenplayElement *e) { return !e->isOmitted(); });

    this->finalizeCharacterNames();

    // Lets compile a list of scene names.
    auto compileSceneTitles = [screenplayElements]() {
        QStringList ret;
        for (const ScreenplayElement *element : std::as_const(screenplayElements)) {
            const Scene *scene = element->scene();
            if (scene) {
                QString title = QStringLiteral("[") + element->resolvedSceneNumber()
                        + QStringLiteral("]: ")
                        + (scene->heading()->isEnabled() ? scene->heading()->text()
                                                         : QStringLiteral("NO SCENE HEADING"));
                if (title.length() > 25)
                    title = title.left(23) + "...";
                ret << title;
            }
        }
        return ret;
    };
    const QStringList sceneTitles = compileSceneTitles();

    // Its a good time to get clear about row and column headings
    const QStringList rowHeadings = m_type == SceneVsCharacter ? sceneTitles : m_characterNames;
    const QStringList columnHeadings = m_type == SceneVsCharacter ? m_characterNames : sceneTitles;

    // Lets create the document now.
    const QFont defaultFont = this->document()->printFormat()->defaultFont();

    QTextCursor cursor(document);
    document->setProperty("#rootFrameMarginNotRequired", true);

    QTextBlockFormat defaultBlockFormat;

    QTextCharFormat defaultCharFormat;
    defaultCharFormat.setFontFamilies({ defaultFont.family() });
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
            title = "Untitled Screenplay";
        cursor.insertText(title);
        blockFormat.setBottomMargin(20);

        const QString reportType = (m_type == SceneVsCharacter)
                ? QStringLiteral("Scene Vs Character Report")
                : QStringLiteral("Character Vs Scene Report");

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText(reportType);

        if (!m_episodeNumbers.isEmpty() || !m_tags.isEmpty())
            cursor.insertBlock();

        if (!m_episodeNumbers.isEmpty()) {
            QStringList epNos;
            epNos.reserve(m_episodeNumbers.size());
            for (int epno : std::as_const(m_episodeNumbers))
                epNos << QString::number(epno);

            cursor.insertText(QStringLiteral("Episode(s): ") + epNos.join(QStringLiteral(", ")));
            if (!m_tags.isEmpty())
                cursor.insertText(QStringLiteral(", "));
        }

        if (!m_tags.isEmpty())
            cursor.insertText(QStringLiteral("Tag(s): ") + m_tags.join(QStringLiteral(", ")));

        blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        blockFormat.setBottomMargin(20);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertHtml("This report was generated using <strong>Scrite</strong><br/>(<a "
                          "href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText("--");
    }

    QTextTableFormat tableFormat;
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(5);
    tableFormat.setBorder(1);
    tableFormat.setBorderCollapse(true);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setHeaderRowCount(1);
    tableFormat.setAlignment(Qt::AlignHCenter);

    CreateColumnHeadingImageFunctor headingImageFunctor(defaultFont);

    QTextTable *table =
            cursor.insertTable(rowHeadings.size() + 1, columnHeadings.size() + 1, tableFormat);
#if 0
    for (int i = 0; i < rowHeadings.size(); i++) {
        const QString text = rowHeadings.at(i);
        const QImage image = headingImageFunctor(text);

        const QString resourceName = QStringLiteral("row-heading-") + QString::number(i);
        document->addResource(QTextDocument::ImageResource, QUrl(resourceName), image);

        QTextTableCell cell = table->cellAt(i + 1, 0);
        QTextCursor cursor = cell.firstCursorPosition();
        cursor.insertImage(resourceName);
    }
#else
    for (int i = 0; i < rowHeadings.size(); i++) {
        const QString text = rowHeadings.at(i);
        QTextTableCell cell = table->cellAt(i + 1, 0);
        QTextCursor cursor = cell.firstCursorPosition();
        cursor.insertText(text);
    }
#endif

    headingImageFunctor.transform.rotate(90);

    for (int i = 0; i < columnHeadings.size(); i++) {
        const QString text = columnHeadings.at(i);
        const QImage image = headingImageFunctor(text);

        const QString resourceName = QStringLiteral("column-heading-") + QString::number(i);
        document->addResource(QTextDocument::ImageResource, QUrl(resourceName), image);

        QTextTableCell cell = table->cellAt(0, i + 1);

        QTextCharFormat cellFormat;
        cellFormat.setVerticalAlignment(QTextCharFormat::AlignBottom);
        cell.setFormat(cellFormat);

        QTextCursor cursor = cell.firstCursorPosition();
        cursor.insertImage(resourceName);
    }

    // Check if invisible characters are being captured
    const bool invisibleCharactersCaptured =
            Application::instance()
                    ->settings()
                    ->value("Screenplay Editor/captureInvisibleCharacters")
                    .toBool();

    // Mark cells
    int sceneNumber = 0;
    for (const ScreenplayElement *element : std::as_const(screenplayElements)) {
        const Scene *scene = element->scene();
        if (scene) {
            const QStringList characters = scene->characterNames();
            for (const QString &character : characters) {
                if (invisibleCharactersCaptured && !scene->isCharacterVisible(character))
                    continue;
                const int row =
                        m_type == SceneVsCharacter ? sceneNumber : rowHeadings.indexOf(character);
                const int column = m_type == SceneVsCharacter ? columnHeadings.indexOf(character)
                                                              : sceneNumber;
                if (row < 0 || column < 0)
                    continue;

                QTextTableCell cell = table->cellAt(row + 1, column + 1);
                QTextBlockFormat cellFormat;
                cellFormat.setBackground(Qt::black);
                cell.firstCursorPosition().setBlockFormat(cellFormat);
            }

            ++sceneNumber;
        }
    }

    return true;
}

void SceneCharacterMatrixReport::configureWriter(QPdfWriter *pdfWriter,
                                                 const QTextDocument *document) const
{
    this->configureWriterImpl(pdfWriter, document);
}

void SceneCharacterMatrixReport::configureWriter(QPrinter *printer,
                                                 const QTextDocument *document) const
{
    this->configureWriterImpl(printer, document);
}

bool SceneCharacterMatrixReport::canDirectExportToOdf() const
{
    return true;
}

bool SceneCharacterMatrixReport::directExportToOdf(QIODevice *device)
{
    try {
        QList<ScreenplayElement *> screenplayElements = this->getScreenplayElements();
        this->finalizeCharacterNames();

        const int nrRows =
                m_type == SceneVsCharacter ? screenplayElements.size() : m_characterNames.size();
        const int nrCols =
                m_type == SceneVsCharacter ? m_characterNames.size() : screenplayElements.size();

        const QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                + QStringLiteral("/") + QStringLiteral("scrite_matrix_")
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

        // Horizontal + vertical center (for SNo. and checkmark cells)
        OpenXLSX::XLStyleIndex centerStyleIndex = styles.cellFormats().create();
        OpenXLSX::XLCellFormat centerCellFormat = styles.cellFormats()[centerStyleIndex];
        auto centerAlign = centerCellFormat.alignment(OpenXLSX::XLCreateIfMissing);
        centerAlign.setHorizontal(OpenXLSX::XLAlignCenter);
        centerAlign.setVertical(OpenXLSX::XLAlignCenter);
        centerCellFormat.setApplyAlignment(true);

        // Vertical center + indent for padding (for all other text cells)
        OpenXLSX::XLStyleIndex vCenterStyleIndex = styles.cellFormats().create();
        OpenXLSX::XLCellFormat vCenterCellFormat = styles.cellFormats()[vCenterStyleIndex];
        auto vCenterAlign = vCenterCellFormat.alignment(OpenXLSX::XLCreateIfMissing);
        vCenterAlign.setIndent(1);
        vCenterAlign.setVertical(OpenXLSX::XLAlignCenter);
        vCenterCellFormat.setApplyAlignment(true);

        // Track max content character width per column (0-based)
        const int fixedCols = m_type == SceneVsCharacter ? 4 : 1;
        const int totalCols = fixedCols + nrCols;
        std::vector<int> colMaxWidths(totalCols, 0);

        auto trackWidth = [&](uint32_t colIdx, const QString &content) {
            int idx = static_cast<int>(colIdx) - 1;
            if (idx >= 0 && idx < totalCols)
                colMaxWidths[idx] = std::max(colMaxWidths[idx], static_cast<int>(content.length()));
        };

        // 30px row height: at 96 DPI, 1pt = 96/72 px, so 30px = 30 * 72/96 = 22.5pt
        const float rowHeightPt = 22.5f;

        uint32_t rowIndex = 1;
        uint32_t colIndex = 1;

        ws.row(rowIndex).setHeight(rowHeightPt);

        if (m_type == SceneVsCharacter) {
            const QStringList fixedHeaders = { "SNo.", "Type", "Location", "Time" };
            for (const QString &h : fixedHeaders) {
                ws.cell(rowIndex, colIndex).value() = h.toStdString();
                ws.cell(rowIndex, colIndex).setCellFormat(vCenterStyleIndex);
                trackWidth(colIndex, h);
                colIndex++;
            }
        }

        for (int j = 0; j < nrCols; j++) {
            const QString colName = m_type == SceneVsCharacter
                    ? m_characterNames.at(j)
                    : screenplayElements.at(j)->resolvedSceneNumber();
            ws.cell(rowIndex, colIndex).value() = colName.toStdString();
            ws.cell(rowIndex, colIndex).setCellFormat(vCenterStyleIndex);
            trackWidth(colIndex, colName);
            colIndex++;
        }
        rowIndex++;

        const bool invisibleCharactersCaptured =
                Application::instance()
                        ->settings()
                        ->value("Screenplay Editor/captureInvisibleCharacters")
                        .toBool();

        const QString checkMark = m_marker.isEmpty() ? QStringLiteral("-") : m_marker;

        for (int i = 0; i < nrRows; i++) {
            colIndex = 1;

            ws.row(rowIndex).setHeight(rowHeightPt);

            if (m_type == SceneVsCharacter) {
                Scene *scene = screenplayElements.at(i)->scene();
                const QString sceneNum = screenplayElements.at(i)->resolvedSceneNumber();
                auto cell = ws.cell(rowIndex, colIndex);
                cell.value() = sceneNum.toStdString();
                cell.setCellFormat(centerStyleIndex);
                trackWidth(colIndex, sceneNum);
                colIndex++;

                if (scene->heading()->isEnabled()) {
                    const QString locType = scene->heading()->locationType();
                    const QString location = scene->heading()->location();
                    const QString moment = scene->heading()->moment();
                    ws.cell(rowIndex, colIndex).value() = locType.toStdString();
                    ws.cell(rowIndex, colIndex).setCellFormat(vCenterStyleIndex);
                    trackWidth(colIndex, locType);
                    colIndex++;
                    ws.cell(rowIndex, colIndex).value() = location.toStdString();
                    ws.cell(rowIndex, colIndex).setCellFormat(vCenterStyleIndex);
                    trackWidth(colIndex, location);
                    colIndex++;
                    ws.cell(rowIndex, colIndex).value() = moment.toStdString();
                    ws.cell(rowIndex, colIndex).setCellFormat(vCenterStyleIndex);
                    trackWidth(colIndex, moment);
                    colIndex++;
                } else {
                    for (int k = 0; k < 3; k++) {
                        ws.cell(rowIndex, colIndex).value() = "-";
                        ws.cell(rowIndex, colIndex).setCellFormat(vCenterStyleIndex);
                        trackWidth(colIndex, QStringLiteral("-"));
                        colIndex++;
                    }
                }
            } else {
                const QString charName = m_characterNames.at(i);
                ws.cell(rowIndex, colIndex).value() = charName.toStdString();
                ws.cell(rowIndex, colIndex).setCellFormat(vCenterStyleIndex);
                trackWidth(colIndex, charName);
                colIndex++;
            }

            for (int j = 0; j < nrCols; j++) {
                Scene *scene = nullptr;
                QString characterName;

                if (m_type == SceneVsCharacter) {
                    scene = screenplayElements.at(i)->scene();
                    characterName = m_characterNames.at(j);
                } else {
                    scene = screenplayElements.at(j)->scene();
                    characterName = m_characterNames.at(i);
                }

                const QStringList sceneCharacters = scene->characterNames();
                if (sceneCharacters.contains(characterName)
                    && (invisibleCharactersCaptured ? scene->isCharacterVisible(characterName)
                                                    : true)) {
                    auto cell = ws.cell(rowIndex, colIndex);
                    cell.value() = checkMark.toStdString();
                    cell.setCellFormat(centerStyleIndex);
                    trackWidth(colIndex, checkMark);
                }
                colIndex++;
            }

            rowIndex++;
        }

        // Set column widths to max content length + 4 characters of padding
        for (int c = 0; c < totalCols; c++) {
            ws.column(static_cast<uint16_t>(c + 1))
                    .setWidth(static_cast<float>(colMaxWidths[c] + 4));
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

Q_DECL_IMPORT int qt_defaultDpi();

void SceneCharacterMatrixReport::configureWriterImpl(QPagedPaintDevice *ppd,
                                                     const QTextDocument *document) const
{
    const QSizeF idealSizeInPixels = document->size();
    if (idealSizeInPixels.width() > idealSizeInPixels.height())
        ppd->setPageOrientation(QPageLayout::Landscape);
    else
        ppd->setPageOrientation(QPageLayout::Portrait);

    const QSizeF pdfPageSizeInPixels = ppd->pageLayout().pageSize().sizePixels(qt_defaultDpi());
    const qreal scale = idealSizeInPixels.width() / pdfPageSizeInPixels.width();

    if (scale < 1 || qFuzzyCompare(scale, 1.0))
        return;

    const qreal margin = 1.0 / 2.54;
    QSizeF requiredPdfPageSize = ppd->pageLayout().pageSize().size(QPageSize::Inch);
    requiredPdfPageSize *= scale;
    requiredPdfPageSize += QSizeF(margin, margin); // margin
    ppd->setPageSize(
            QPageSize(requiredPdfPageSize, QPageSize::Inch, "Custom", QPageSize::FuzzyMatch));
}

QList<ScreenplayElement *> SceneCharacterMatrixReport::getScreenplayElements()
{
    const Screenplay *screenplay = this->document()->screenplay();

    const bool hasEpisodes = screenplay->episodeCount() > 0;
    int episodeNr = 0; // Episode number is 1+episodeIndex
    QList<ScreenplayElement *> screenplayElements;
    for (int i = 0; i < screenplay->elementCount(); i++) {
        ScreenplayElement *element = screenplay->elementAt(i);
        if (hasEpisodes && !m_episodeNumbers.isEmpty()) {
            if (element->elementType() == ScreenplayElement::BreakElementType
                && element->breakType() == Screenplay::Episode)
                ++episodeNr;
            else if (i == 0)
                ++episodeNr;

            if (!m_episodeNumbers.contains(episodeNr))
                continue;
        }

        if (!m_tags.isEmpty() && element->elementType() == ScreenplayElement::SceneElementType
            && element->scene() != nullptr) {
            Scene *scene = element->scene();

            const QStringList sceneTags = scene->groups();
            if (sceneTags.isEmpty())
                continue;

            QStringList tags;
            std::copy_if(sceneTags.begin(), sceneTags.end(), std::back_inserter(tags),
                         [=](const QString &sceneTag) {
                             return tags.isEmpty() ? m_tags.contains(sceneTag) : false;
                         });

            if (tags.isEmpty())
                continue;
        }

        if (element->scene() == nullptr)
            continue;

        screenplayElements.append(element);
    }

    return screenplayElements;
}

void SceneCharacterMatrixReport::finalizeCharacterNames()
{
    // Validate the given set of character names. Ensure that they
    // exist in the screenplay.
    const Structure *structure = this->document()->structure();
    const QStringList availableCharacters = structure->characterNames();
    if (m_characterNames.isEmpty())
        m_characterNames = availableCharacters;
    else {
        for (int i = m_characterNames.size() - 1; i >= 0; i--) {
            m_characterNames[i] = m_characterNames[i].toUpper();
            const QString name = m_characterNames.at(i);
            if (!availableCharacters.contains(name))
                m_characterNames.removeAt(i);
        }

        if (m_characterNames.isEmpty())
            m_characterNames = availableCharacters;
    }
}
