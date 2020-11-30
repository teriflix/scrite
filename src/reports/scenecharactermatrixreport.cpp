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

#include "scenecharactermatrixreport.h"
#include "transliteration.h"

#include <QPainter>
#include <QPdfWriter>
#include <QTextTable>

SceneCharacterMatrixReport::SceneCharacterMatrixReport(QObject *parent)
    : AbstractReportGenerator(parent)
{
    connect(this, &AbstractReportGenerator::documentChanged, [=]() {
        if(this->document() != nullptr)
            this->setCharacterNames( this->document()->structure()->characterNames() );
    });
}

SceneCharacterMatrixReport::~SceneCharacterMatrixReport()
{

}

void SceneCharacterMatrixReport::setType(int val)
{
    if(m_type == val)
        return;

    if(val != SceneVsCharacter && val != CharacterVsScene)
        return;

    m_type = val;
    emit typeChanged();
}

void SceneCharacterMatrixReport::setCharacterNames(const QStringList &val)
{
    if(m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

bool SceneCharacterMatrixReport::supportsFormat(AbstractReportGenerator::Format format) const
{
    return format == AdobePDF;
}

struct CreateColumnHeadingImageFunctor
{
    CreateColumnHeadingImageFunctor(const QFont &font)
        : font(font),
          fontMetrics(font) {}

    QTransform transform;
    QFont font;
    QFontMetrics fontMetrics;
    QBrush background = QBrush(Qt::white);
    typedef QImage result_type;

    QImage operator () (const QString &text) {
        const QRect textRect = fontMetrics.boundingRect(text);
        const qreal dpr = 2.0;

        QImage image(textRect.size()*dpr, QImage::Format_ARGB32);
        image.setDevicePixelRatio(dpr);
        image.fill(background.color());

        QPainter paint(&image);
        paint.setFont(font);
        paint.setPen(Qt::black);
        paint.drawText(QRect(0, 0, textRect.width(), textRect.height()), Qt::AlignCenter, text);
        paint.end();

        if(!transform.isIdentity())
            image = image.transformed(transform);

        return image;
    }
};

bool SceneCharacterMatrixReport::doGenerate(QTextDocument *document)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const Structure *structure = this->document()->structure();
    const QStringList availableCharacters = structure->characterNames();

    // Validate the given set of character names. Ensure that they
    // exist in the screenplay.
    if(m_characterNames.isEmpty())
        m_characterNames = availableCharacters;
    else
    {
        for(int i=m_characterNames.size()-1; i>=0; i--)
        {
            m_characterNames[i] = m_characterNames[i].toUpper();
            const QString name = m_characterNames.at(i);
            if( !availableCharacters.contains(name) )
                m_characterNames.removeAt(i);
        }

        if(m_characterNames.isEmpty())
            m_characterNames = availableCharacters;
    }

    // Lets compile a list of scene names.
    auto compileSceneTitles = [screenplay]() {
        QStringList ret;
        for(int i=0; i<screenplay->elementCount(); i++) {
            const ScreenplayElement *element = screenplay->elementAt(i);
            const Scene *scene = element->scene();
            if(scene) {
                QString title = QStringLiteral("[") + element->resolvedSceneNumber() + QStringLiteral("]: ")
                        + (scene->heading()->isEnabled() ? scene->heading()->text() : QStringLiteral("NO SCENE HEADING"));
                if(title.length() > 25)
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
        if(title.isEmpty())
            title = "Untitled Screenplay";
        cursor.insertText(title);
        if(!screenplay->subtitle().isEmpty())
        {
            cursor.insertBlock();
            cursor.insertText(screenplay->subtitle());
        }

        blockFormat.setBottomMargin(20);

        const QString reportType = (m_type == SceneVsCharacter) ? QStringLiteral("Scene Vs Character Report") : QStringLiteral("Character Vs Scene Report");

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText(reportType);

        blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        blockFormat.setBottomMargin(20);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertHtml("This report was generated using <strong>scrite</strong><br/>(<a href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText("--");
    }

    QTextTableFormat tableFormat;
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(5);
    tableFormat.setBorder(3);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setHeaderRowCount(1);
    tableFormat.setAlignment(Qt::AlignHCenter);

    CreateColumnHeadingImageFunctor headingImageFunctor(defaultFont);

    QTextTable *table = cursor.insertTable(rowHeadings.size()+1, columnHeadings.size()+1, tableFormat);

    for(int i=0; i<rowHeadings.size(); i++)
    {
        const QString text = rowHeadings.at(i);
        const QImage image = headingImageFunctor(text);

        const QString resourceName = QStringLiteral("row-heading-") + QString::number(i);
        document->addResource(QTextDocument::ImageResource, QUrl(resourceName), image);

        QTextTableCell cell = table->cellAt(i+1, 0);
        QTextCursor cursor = cell.firstCursorPosition();
        cursor.insertImage(resourceName);
    }

    headingImageFunctor.transform.rotate(90);

    for(int i=0; i<columnHeadings.size(); i++)
    {
        const QString text = columnHeadings.at(i);
        const QImage image = headingImageFunctor(text);

        const QString resourceName = QStringLiteral("column-heading-") + QString::number(i);
        document->addResource(QTextDocument::ImageResource, QUrl(resourceName), image);

        QTextTableCell cell = table->cellAt(0, i+1);

        QTextCharFormat cellFormat;
        cellFormat.setVerticalAlignment(QTextCharFormat::AlignBottom);
        cell.setFormat(cellFormat);

        QTextCursor cursor = cell.firstCursorPosition();
        cursor.insertImage(resourceName);
    }

    // Mark cells
    int sceneNumber = 0;
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        const ScreenplayElement *element = screenplay->elementAt(i);
        const Scene *scene = element->scene();
        if(scene)
        {
            const QStringList characters = scene->characterNames();
            Q_FOREACH(QString character, characters)
            {
                const int row = m_type == SceneVsCharacter ? sceneNumber : rowHeadings.indexOf(character);
                const int column = m_type == SceneVsCharacter ? columnHeadings.indexOf(character) : sceneNumber;
                if(row < 0 || column < 0)
                    continue;

                QTextTableCell cell = table->cellAt(row+1, column+1);
                QTextBlockFormat cellFormat;
                cellFormat.setBackground(Qt::black);
                cell.firstCursorPosition().setBlockFormat(cellFormat);
            }

            ++sceneNumber;
        }
    }

    return true;
}

void SceneCharacterMatrixReport::configureWriter(QPdfWriter *pdfWriter, const QTextDocument *document) const
{
    const QSizeF idealSizeInPixels = document->size();
    if(idealSizeInPixels.width() > idealSizeInPixels.height())
        pdfWriter->setPageOrientation(QPageLayout::Landscape);
    else
        pdfWriter->setPageOrientation(QPageLayout::Portrait);

    const QSizeF pdfPageSizeInPixels = pdfWriter->pageLayout().pageSize().sizePixels(72);
    const qreal scale = idealSizeInPixels.width() / pdfPageSizeInPixels.width();

    if(scale < 1 || qFuzzyCompare(scale, 1.0) )
        return;

    const qreal margin = 1.0/2.54;
    QSizeF requiredPdfPageSize = pdfWriter->pageLayout().pageSize().size(QPageSize::Inch);
    requiredPdfPageSize *= scale;
    requiredPdfPageSize += QSizeF(margin, margin); // margin
    pdfWriter->setPageSize( QPageSize(requiredPdfPageSize,QPageSize::Inch,"Custom",QPageSize::FuzzyMatch) );
}
