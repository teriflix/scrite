/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "pdfexporter.h"

#include <QPainter>
#include <QPdfWriter>
#include <QTextCursor>
#include <QTextDocument>
#include <QFontMetrics>
#include <QAbstractTextDocumentLayout>
#include <QFileInfo>

PdfExporter::PdfExporter(QObject *parent)
            : AbstractExporter(parent)
{

}

PdfExporter::~PdfExporter()
{

}

bool PdfExporter::doExport(QIODevice *device)
{
    const ScreenplayFormat *screenplayFormat = this->document()->formatting();
    const Screenplay *screenplay = this->document()->screenplay();
    const int nrScreenplayElements = screenplay->elementCount();

    QPdfWriter pdfWriter(device);
    pdfWriter.setTitle(screenplay->title());
    pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

    const qreal pageWidth = pdfWriter.width();

    const QFont defaultFont = screenplayFormat->defaultFont();

    QTextDocument textDocument;
    textDocument.setDefaultFont(defaultFont);
    QTextCursor cursor(&textDocument);

    // Title Page
    if( !screenplay->title().isEmpty() )
    {
        // Title
        {
            QTextBlockFormat blockFormat;
            blockFormat.setAlignment(Qt::AlignHCenter);
            blockFormat.setBottomMargin(100);

            QTextCharFormat charFormat;
            charFormat.setFontFamily(defaultFont.family());
            charFormat.setFontPointSize(36);

            cursor.setBlockFormat(blockFormat);
            cursor.setCharFormat(charFormat);

            cursor.insertBlock();
            cursor.insertText(screenplay->title());
        }

        // Author
        {
            QTextBlockFormat blockFormat;
            blockFormat.setAlignment(Qt::AlignHCenter);

            QTextCharFormat charFormat;
            charFormat.setFontFamily(defaultFont.family());
            charFormat.setFontPointSize(defaultFont.pointSize());

            cursor.insertBlock();
            cursor.setBlockFormat(blockFormat);
            cursor.setCharFormat(charFormat);
            cursor.insertText(screenplay->author());
        }

        // Contact
        {
            QTextBlockFormat blockFormat;
            blockFormat.setAlignment(Qt::AlignHCenter);

            QTextCharFormat charFormat;
            charFormat.setFontFamily(defaultFont.family());
            charFormat.setFontPointSize(defaultFont.pointSize());

            cursor.insertBlock();
            cursor.setBlockFormat(blockFormat);
            cursor.setCharFormat(charFormat);
            cursor.insertText(screenplay->contact());
        }

        // Version
        {
            QTextBlockFormat blockFormat;
            blockFormat.setAlignment(Qt::AlignHCenter);

            QTextCharFormat charFormat;
            charFormat.setFontFamily(defaultFont.family());
            charFormat.setFontPointSize(defaultFont.pointSize());

            cursor.insertBlock();
            cursor.setBlockFormat(blockFormat);
            cursor.setCharFormat(charFormat);
            cursor.insertText("Version: " + screenplay->version());
        }

        // Version
        {
            QTextBlockFormat blockFormat;
            blockFormat.setAlignment(Qt::AlignHCenter);
            blockFormat.setTopMargin(100);

            QTextCharFormat charFormat;
            charFormat.setFontFamily(defaultFont.family());
            charFormat.setFontPointSize(9);

            cursor.insertBlock();
            cursor.setBlockFormat(blockFormat);
            cursor.setCharFormat(charFormat);
            cursor.insertText("This PDF was generated using scrite (https://scrite.teriflix.com)");
        }

        cursor.insertBlock();
    }

    int nrBlocks = 0, nrSceneHeadings=0;
    auto createNewTextBlock = [&nrBlocks,pageWidth,screenplayFormat](QTextCursor &cursor, SceneElement::Type type) {
        if(nrBlocks > 0)
            cursor.insertBlock();

        const SceneElementFormat *format = screenplayFormat->elementFormat(type);
        QTextBlockFormat blkFormat = format->createBlockFormat(&pageWidth);
        if(nrBlocks == 0)
            blkFormat.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);

        QTextCharFormat chrFormat = format->createCharFormat(&pageWidth);

        cursor.setBlockFormat(blkFormat);
        cursor.setCharFormat(chrFormat);
    };

    for(int i=0; i<nrScreenplayElements; i++)
    {
        const ScreenplayElement *screenplayElement = screenplay->elementAt(i);
        const Scene *scene = screenplayElement->scene();
        const int nrSceneElements = scene->elementCount();

        // Scene heading
        SceneHeading *heading = scene->heading();
        if(heading->isEnabled())
        {
            ++nrSceneHeadings;
            const QString sceneNr = QString("[%1] ").arg(nrSceneHeadings);
            createNewTextBlock(cursor,SceneElement::Heading);
            cursor.insertText(sceneNr + heading->toString());

            ++nrBlocks;
        }

        for(int j=0; j<nrSceneElements; j++)
        {
            SceneElement *element = scene->elementAt(j);
            createNewTextBlock(cursor, element->type());
            cursor.insertText(element->text());

            ++nrBlocks;
        }
    }

    textDocument.print(&pdfWriter);
    return true;
}

QString PdfExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix() != "pdf")
        return fileName + ".pdf";
    return fileName;
}
