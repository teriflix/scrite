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

#ifndef EPUBEXPORTER_H
#define EPUBEXPORTER_H

#include "abstractexporter.h"

class EpubExporter : public AbstractExporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Screenplay/EPUB")
    Q_CLASSINFO("NameFilters", "EPUB (*.epub)")
    Q_CLASSINFO("Description", "Exports the current screenplay to EPUB file format, which can be read on phones, tablets and e-readers.")
    Q_CLASSINFO("Icon", ":/icons/exporter/epub.png")
    // clang-format on

public:
    Q_INVOKABLE explicit EpubExporter(QObject *parent = nullptr);
    ~EpubExporter() override;

    // clang-format off
    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers in the generated EPUB.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers
               READ isIncludeSceneNumbers
               WRITE setIncludeSceneNumbers
               NOTIFY includeSceneNumbersChanged)
    // clang-format on
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    // clang-format off
    Q_CLASSINFO("exportWithSceneColors_FieldLabel", "Export with scene colors.")
    Q_CLASSINFO("exportWithSceneColors_FieldEditor", "CheckBox")
    Q_PROPERTY(bool exportWithSceneColors
               READ isExportWithSceneColors
               WRITE setExportWithSceneColors
               NOTIFY exportWithSceneColorsChanged)
    // clang-format on
    void setExportWithSceneColors(bool val);
    bool isExportWithSceneColors() const { return m_exportWithSceneColors; }
    Q_SIGNAL void exportWithSceneColorsChanged();

    // clang-format off
    Q_CLASSINFO("includeTitlePage_FieldLabel", "Include title page.")
    Q_CLASSINFO("includeTitlePage_FieldNote", "The cover photo, if any, is also used as the cover image of the EPUB.")
    Q_CLASSINFO("includeTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeTitlePage
               READ isIncludeTitlePage
               WRITE setIncludeTitlePage
               NOTIFY includeTitlePageChanged)
    // clang-format on
    void setIncludeTitlePage(bool val);
    bool isIncludeTitlePage() const { return m_includeTitlePage; }
    Q_SIGNAL void includeTitlePageChanged();

    bool requiresConfiguration() const override { return true; }

protected:
    bool doExport(QIODevice *device) override; // AbstractExporter interface
    QString fileNameExtension() const override { return QStringLiteral("epub"); }

private:
    bool m_includeSceneNumbers = false;
    bool m_exportWithSceneColors = false;
    bool m_includeTitlePage = true;
};

#endif // EPUBEXPORTER_H
