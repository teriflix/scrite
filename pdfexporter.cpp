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

bool PdfExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();

    QPdfWriter pdfWriter(device);
    pdfWriter.setTitle(screenplay->title());
    pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

    const qreal pageWidth = pdfWriter.width();
    QTextDocument textDocument;
    this->AbstractTextDocumentExporter::generate(&textDocument, pageWidth);

    textDocument.print(&pdfWriter);
    return true;
}

QString PdfExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix().toLower() != "pdf")
        return fileName + ".pdf";
    return fileName;
}
