/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ODTEXPORTER_H
#define ODTEXPORTER_H

#include "abstracttextdocumentexporter.h"

class OdtExporter : public AbstractTextDocumentExporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Screenplay/Open Document Format")
    Q_CLASSINFO("NameFilters", "Open Document Format (*.odt)")
    Q_CLASSINFO("Description", "Exports the current screenplay to Open Document Text file format. Such files can " "be opened in Google Docs, Microsoft Word and Libre/Open Office.")
    Q_CLASSINFO("Icon", ":/icons/exporter/odt.png")
    // clang-format on

public:
    Q_INVOKABLE explicit OdtExporter(QObject *parent = nullptr);
    ~OdtExporter() override;

    // clang-format off
    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers in the generated document.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers
               READ isIncludeSceneNumbers
               WRITE setIncludeSceneNumbers
               NOTIFY includeSceneNumbersChanged)
    // clang-format on
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const override { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    // clang-format off
    Q_CLASSINFO("printEachSceneOnANewPage_FieldLabel", "Print each scene on a new page.")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldEditor", "CheckBox")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldNote", "Automatically turned off if acts are included or printed on a new page.")
    Q_PROPERTY(bool printEachSceneOnANewPage
               READ isPrintEachSceneOnANewPage
               WRITE setPrintEachSceneOnANewPage
               NOTIFY printEachSceneOnANewPageChanged)
    // clang-format on
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const override { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    // clang-format off
    Q_CLASSINFO("printEachActOnANewPage_FieldLabel", "Print each act on a new page.")
    Q_CLASSINFO("printEachActOnANewPage_FieldEditor", "CheckBox")
    Q_CLASSINFO("printEachActOnANewPage_FieldNote", "Automatically includes act breaks in the generated document.")
    Q_PROPERTY(bool printEachActOnANewPage
               READ isPrintEachActOnANewPage
               WRITE setPrintEachActOnANewPage
               NOTIFY printEachActOnANewPageChanged)
    // clang-format on
    void setPrintEachActOnANewPage(bool val);
    bool isPrintEachActOnANewPage() const override { return m_printEachActOnANewPage; }
    Q_SIGNAL void printEachActOnANewPageChanged();

    // clang-format off
    Q_CLASSINFO("includeActBreaks_FieldLabel", "Include act breaks.")
    Q_CLASSINFO("includeActBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeActBreaks
               READ isIncludeActBreaks
               WRITE setIncludeActBreaks
               NOTIFY includeActBreaksChanged)
    // clang-format on
    void setIncludeActBreaks(bool val);
    bool isIncludeActBreaks() const override { return m_includeActBreaks; }
    Q_SIGNAL void includeActBreaksChanged();

    bool generateTitlePage() const override { return false; }

    bool isExportForPrintingPurpose() const override { return false; }

protected:
    bool doExport(QIODevice *device) override; // AbstractExporter interface
    QString fileNameExtension() const override { return QStringLiteral("odt"); }

private:
    bool m_includeSceneNumbers = false;
    bool m_printEachSceneOnANewPage = false;
    bool m_printEachActOnANewPage = false;
    bool m_includeActBreaks = false;
};

class QTextDocument;

class OdtUtils
{
public:
    // The marker that can be inserted into a QTextDocument to make the top-level paragraph or
    // table containing it start on a new page in the ODT written from that document. It is
    // converted into a page break by polish().
    static QString pageBreakMarker();

    // Tells code that writes into the text document that it is going to be written to ODT, and so
    // it must insert page break markers (instead of page breaks on frames and tables, which the ODT
    // writer ignores).
    static void enablePageBreakMarkers(QTextDocument *textDocument);

    // The marker to insert into the text document to start a new page, or an empty string if page
    // break markers were not enabled for it.
    static QString pageBreakMarker(const QTextDocument *textDocument);

    // Unzips the ODT file, applies the requested changes to its content.xml and zips it back.
    // Page break markers are converted into page breaks, fonts are applied to complex and asian
    // scripts as well (so that non-latin text gets its font in LibreOffice), bundled fonts used in
    // the document are embedded in the file. Returns true if the file was modified.
    static bool polish(const QString &fileName, bool applyPageBreakMarkers);
};

#endif // ODTEXPORTER_H
