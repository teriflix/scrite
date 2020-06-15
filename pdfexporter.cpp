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
    this->setUsePageBreaks(true);
}

PdfExporter::~PdfExporter()
{

}

void PdfExporter::setIncludeSceneNumbers(bool val)
{
    if(m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
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
