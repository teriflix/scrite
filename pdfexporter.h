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

    Q_CLASSINFO("usePageBreaks_FieldLabel", "Use (MORE) and (CONT'D) breaks where appropriate. [May increase page count]")
    Q_CLASSINFO("usePageBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool usePageBreaks READ usePageBreaks WRITE setUsePageBreaks NOTIFY usePageBreaksChanged)
    void setUsePageBreaks(bool val);
    bool usePageBreaks() const { return m_usePageBreaks; }
    Q_SIGNAL void usePageBreaksChanged();

    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers in the generated PDF.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers READ isIncludeSceneNumbers WRITE setIncludeSceneNumbers NOTIFY includeSceneNumbersChanged)
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    bool canBundleFonts() const { return false; }

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString polishFileName(const QString &fileName) const; // AbstractDeviceIO interface

private:
    bool m_usePageBreaks = true;
    bool m_includeSceneNumbers = true;
};

#endif // PDFEXPORTER_H
