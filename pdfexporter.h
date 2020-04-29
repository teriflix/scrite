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

#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include "abstracttextdocumentexporter.h"

class PdfExporter : public AbstractTextDocumentExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Adobe PDF")
    Q_CLASSINFO("NameFilters", "Adobe PDF (*.pdf)")

public:
    Q_INVOKABLE PdfExporter(QObject *parent=nullptr);
    ~PdfExporter();

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString polishFileName(const QString &fileName) const; // AbstractDeviceIO interface
};

#endif // PDFEXPORTER_H
