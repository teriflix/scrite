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

    Q_CLASSINFO("includeSceneIcons_FieldLabel", "Include scene icons in the generated PDF.")
    Q_CLASSINFO("includeSceneIcons_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneIcons READ isIncludeSceneIcons WRITE setIncludeSceneIcons NOTIFY includeSceneIconsChanged)
    void setIncludeSceneIcons(bool val);
    bool isIncludeSceneIcons() const { return m_includeSceneIcons; }
    Q_SIGNAL void includeSceneIconsChanged();

    Q_CLASSINFO("printEachSceneOnANewPage_FieldLabel", "Print each scene on a new page.")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool printEachSceneOnANewPage READ isPrintEachSceneOnANewPage WRITE setPrintEachSceneOnANewPage NOTIFY printEachSceneOnANewPageChanged)
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    Q_CLASSINFO("watermark_FieldLabel", "Watermark text, if enabled.")
    Q_CLASSINFO("watermark_FieldEditor", "TextBox")
    Q_PROPERTY(QString watermark READ watermark WRITE setWatermark NOTIFY watermarkChanged)
    void setWatermark(const QString &val);
    QString watermark() const { return m_watermark; }
    Q_SIGNAL void watermarkChanged();

    Q_CLASSINFO("comment_FieldLabel", "Comment text for use with header & footer.")
    Q_CLASSINFO("comment_FieldEditor", "TextBox")
    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)
    void setComment(const QString &val);
    QString comment() const { return m_comment; }
    Q_SIGNAL void commentChanged();

    bool canBundleFonts() const { return false; }

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString polishFileName(const QString &fileName) const; // AbstractDeviceIO interface

private:
    QString m_comment;
    QString m_watermark;
    bool m_usePageBreaks = true;
    bool m_includeSceneIcons = true;
    bool m_includeSceneNumbers = true;
    bool m_printEachSceneOnANewPage = false;
};

#endif // PDFEXPORTER_H
