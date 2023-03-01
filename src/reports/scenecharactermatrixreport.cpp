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

#include "scenecharactermatrixreport.h"
#include "transliteration.h"

#include <QPrinter>
#include <QPainter>
#include <QPdfWriter>
#include <QTextTable>

SceneCharacterMatrixReport::SceneCharacterMatrixReport(QObject *parent)
    : AbstractReportGenerator(parent)
{
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
    return this->format() == OpenDocumentFormat ? QStringLiteral("csv") : QStringLiteral("pdf");
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
    QList<ScreenplayElement *> screenplayElements = this->getScreenplayElements();
    this->finalizeCharacterNames();

    // Lets compile a list of scene names.
    auto compileSceneTitles = [screenplayElements]() {
        QStringList ret;
        for (const ScreenplayElement *element : qAsConst(screenplayElements)) {
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
            for (int epno : qAsConst(m_episodeNumbers))
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

    // Mark cells
    int sceneNumber = 0;
    for (const ScreenplayElement *element : qAsConst(screenplayElements)) {
        const Scene *scene = element->scene();
        if (scene) {
            const QStringList characters = scene->characterNames();
            for (const QString &character : characters) {
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
    QList<ScreenplayElement *> screenplayElements = this->getScreenplayElements();
    this->finalizeCharacterNames();

    QTextStream ts(device);
    ts.setAutoDetectUnicode(true);
    ts.setCodec("utf-8");

    const int nrRows =
            m_type == SceneVsCharacter ? screenplayElements.size() : m_characterNames.size();
    const int nrCols =
            m_type == SceneVsCharacter ? m_characterNames.size() : screenplayElements.size();
    auto escapeComma = [](const QString &text) {
        if (!text.contains(QChar(',')))
            return text;
        const QString quote = QStringLiteral("\"");
        return quote + text + quote;
    };

    if (m_type == SceneVsCharacter)
        ts << "SNo.,Type,Location,Time";

    // Column headings
    for (int j = 0; j < nrCols; j++) {
        const QString colName = m_type == SceneVsCharacter
                ? m_characterNames.at(j)
                : screenplayElements.at(j)->resolvedSceneNumber();
        ts << "," << escapeComma(colName);
    }
    ts << "\n";

    // Row contents
    const QString checkMark = m_marker.isEmpty() ? QStringLiteral("✓") : escapeComma(m_marker);
    for (int i = 0; i < nrRows; i++) {
        if (m_type == SceneVsCharacter) {
            Scene *scene = screenplayElements.at(i)->scene();
            ts << screenplayElements.at(i)->resolvedSceneNumber() << ",";
            if (scene->heading()->isEnabled())
                ts << escapeComma(scene->heading()->locationType()) << ","
                   << escapeComma(scene->heading()->location()) << ","
                   << escapeComma(scene->heading()->moment());
            else
                ts << "-,-,-";
        } else
            ts << escapeComma(m_characterNames.at(i));

        for (int j = 0; j < nrCols; j++) {
            ts << ",";

            Scene *scene = nullptr;
            QString characterName;

            if (m_type == SceneVsCharacter) {
                scene = screenplayElements.at(i)->scene();
                characterName = m_characterNames.at(j);
            } else {
                scene = screenplayElements.at(j)->scene();
                characterName = m_characterNames.at(i);
            }

            if (scene->characterNames().contains(characterName))
                ts << checkMark;
        }

        ts << "\n";
    }

    ts.flush();

    return true;
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
