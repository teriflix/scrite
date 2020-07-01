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

#include "pdfexporter.h"
#include "imageprinter.h"
#include "qtextdocumentpagedprinter.h"

#include <QFileInfo>
#include <QPdfWriter>
#include <QTextCursor>
#include <QTextDocument>
#include <QFontMetrics>
#include <QAbstractTextDocumentLayout>

PdfExporter::PdfExporter(QObject *parent)
            : AbstractTextDocumentExporter(parent)
{

}

PdfExporter::~PdfExporter()
{

}

void PdfExporter::setWatermark(const QString &val)
{
    if(m_watermark == val)
        return;

    m_watermark = val;
    emit watermarkChanged();
}

void PdfExporter::setComment(const QString &val)
{
    if(m_comment == val)
        return;

    m_comment = val;
    emit commentChanged();
}

void PdfExporter::setIncludeSceneNumbers(bool val)
{
    if(m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void PdfExporter::setIncludeSceneIcons(bool val)
{
    if(m_includeSceneIcons == val)
        return;

    m_includeSceneIcons = val;
    emit includeSceneIconsChanged();
}

void PdfExporter::setUsePageBreaks(bool val)
{
    if(m_usePageBreaks == val)
        return;

    m_usePageBreaks = val;
    emit usePageBreaksChanged();
}

bool PdfExporter::doExport(QIODevice *device)
{
    Screenplay *screenplay = this->document()->screenplay();
    ScreenplayFormat *format = this->document()->printFormat();

    QPdfWriter pdfWriter(device);
    pdfWriter.setTitle(screenplay->title());
    pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
    format->pageLayout()->configure(&pdfWriter);
    pdfWriter.setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

    const qreal pageWidth = pdfWriter.width();
    QTextDocument textDocument;
    this->AbstractTextDocumentExporter::generate(&textDocument, pageWidth);    
    textDocument.setProperty("#comment", m_comment);
    textDocument.setProperty("#watermark", m_watermark);

    QTextDocumentPagedPrinter printer;
    printer.header()->setVisibleFromPageOne(false);
    printer.footer()->setVisibleFromPageOne(false);
    printer.watermark()->setVisibleFromPageOne(false);
    return printer.print(&textDocument, &pdfWriter);
}

QString PdfExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix().toLower() != "pdf")
        return fileName + ".pdf";
    return fileName;
}
