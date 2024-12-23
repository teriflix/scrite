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

#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include "abstracttextdocumentexporter.h"

class PdfExporter : public AbstractTextDocumentExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Adobe PDF")
    Q_CLASSINFO("NameFilters", "Adobe PDF (*.pdf)")
    Q_CLASSINFO("Description", "Exports the current screenplay to PDF format.")
    Q_CLASSINFO("Icon", ":/icons/exporter/pdf.png")

public:
    Q_INVOKABLE explicit PdfExporter(QObject *parent = nullptr);
    ~PdfExporter();

    Q_CLASSINFO("generateTitlePage_FieldLabel", "Generate title page.")
    Q_CLASSINFO("generateTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool generateTitlePage READ isGenerateTitlePage WRITE setGenerateTitlePage NOTIFY generateTitlePageChanged)
    void setGenerateTitlePage(bool val);
    bool isGenerateTitlePage() const { return m_generateTitlePage; }
    Q_SIGNAL void generateTitlePageChanged();

    Q_CLASSINFO("includeLogline_FieldLabel", "Include logline in title page.")
    Q_CLASSINFO("includeLogline_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeLogline READ isIncludeLogline WRITE setIncludeLogline NOTIFY includeLoglineChanged)
    void setIncludeLogline(bool val);
    bool isIncludeLogline() const { return m_includeLogline; }
    Q_SIGNAL void includeLoglineChanged();

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
    Q_CLASSINFO("printEachSceneOnANewPage_FieldNote", "Automatically turned off if acts are included or printed on a new page.")
    Q_PROPERTY(bool printEachSceneOnANewPage READ isPrintEachSceneOnANewPage WRITE setPrintEachSceneOnANewPage NOTIFY printEachSceneOnANewPageChanged)
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    Q_CLASSINFO("printEachActOnANewPage_FieldLabel", "Print each act on a new page.")
    Q_CLASSINFO("printEachActOnANewPage_FieldEditor", "CheckBox")
    Q_CLASSINFO("printEachActOnANewPage_FieldNote", "Automatically includes act breaks in the generated PDF.")
    Q_PROPERTY(bool printEachActOnANewPage READ isPrintEachActOnANewPage WRITE setPrintEachActOnANewPage NOTIFY printEachActOnANewPageChanged)
    void setPrintEachActOnANewPage(bool val);
    bool isPrintEachActOnANewPage() const { return m_printEachActOnANewPage; }
    Q_SIGNAL void printEachActOnANewPageChanged();

    Q_CLASSINFO("includeActBreaks_FieldLabel", "Include act breaks.")
    Q_CLASSINFO("includeActBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeActBreaks READ isIncludeActBreaks WRITE setIncludeActBreaks NOTIFY includeActBreaksChanged)
    void setIncludeActBreaks(bool val);
    bool isIncludeActBreaks() const { return m_includeActBreaks; }
    Q_SIGNAL void includeActBreaksChanged();

    Q_CLASSINFO("watermark_FieldLabel", "Watermark text, if enabled.")
    Q_CLASSINFO("watermark_FieldEditor", "TextBox")
    Q_CLASSINFO("watermark_IsPersistent", "false")
    Q_CLASSINFO("watermark_Feature", "watermark")
    Q_PROPERTY(QString watermark READ watermark WRITE setWatermark NOTIFY watermarkChanged)
    void setWatermark(const QString &val);
    QString watermark() const { return m_watermark; }
    Q_SIGNAL void watermarkChanged();

    Q_CLASSINFO("comment_FieldLabel", "Comment text for use with header & footer.")
    Q_CLASSINFO("comment_FieldEditor", "TextBox")
    Q_CLASSINFO("comment_IsPersistent", "false")
    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)
    void setComment(const QString &val);
    QString comment() const { return m_comment; }
    Q_SIGNAL void commentChanged();

    bool generateTitlePage() const { return m_generateTitlePage; }
    bool canBundleFonts() const { return false; }
    bool isExportForPrintingPurpose() const { return true; }

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString fileNameExtension() const { return QStringLiteral("pdf"); }

private:
    QString m_comment;
    QString m_watermark;
    bool m_usePageBreaks = true;
    bool m_includeLogline = false;
    bool m_generateTitlePage = true;
    bool m_includeSceneIcons = true;
    bool m_includeSceneNumbers = true;
    bool m_printEachSceneOnANewPage = false;
    bool m_printEachActOnANewPage = false;
    bool m_includeActBreaks = false;
};

#endif // PDFEXPORTER_H
