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

#ifndef ODTEXPORTER_H
#define ODTEXPORTER_H

#include "abstracttextdocumentexporter.h"

class OdtExporter : public AbstractTextDocumentExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Open Document Format")
    Q_CLASSINFO("NameFilters", "Open Document Format (*.odt)")
    Q_CLASSINFO("Description", "Exports the current screenplay to Open Document Text file format. Such files can be opened in Google Docs, Microsoft Word and Libre/Open Office.")
    Q_CLASSINFO("Icon", ":/icons/exporter/odt.png")

public:
    Q_INVOKABLE explicit OdtExporter(QObject *parent = nullptr);
    ~OdtExporter();

    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers in the generated document.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers READ isIncludeSceneNumbers WRITE setIncludeSceneNumbers NOTIFY includeSceneNumbersChanged)
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    bool generateTitlePage() const { return false; }

    bool isExportForPrintingPurpose() const { return false; }

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString fileNameExtension() const { return QStringLiteral("odt"); }

private:
    bool m_includeSceneNumbers = false;
};

#endif // ODTEXPORTER_H
