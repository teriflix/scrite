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

#ifndef HTMLEXPORTER_H
#define HTMLEXPORTER_H

#include "abstractexporter.h"

class HtmlExporter : public AbstractExporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Screenplay/HTML")
    Q_CLASSINFO("NameFilters", "HTML (*.html)")
    Q_CLASSINFO("Description", "Exports the current screenplay to HTML file format.")
    Q_CLASSINFO("Icon", ":/icons/exporter/html.png")
    // clang-format on

public:
    Q_INVOKABLE explicit HtmlExporter(QObject *parent = nullptr);
    ~HtmlExporter() override;

    // clang-format off
    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers in the generated HTML.")
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
    Q_CLASSINFO("includeTitlePage_FieldNote", "The cover photo, if any, is exported into a resources folder next to the HTML file.")
    Q_CLASSINFO("includeTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeTitlePage
               READ isIncludeTitlePage
               WRITE setIncludeTitlePage
               NOTIFY includeTitlePageChanged)
    // clang-format on
    void setIncludeTitlePage(bool val);
    bool isIncludeTitlePage() const { return m_includeTitlePage; }
    Q_SIGNAL void includeTitlePageChanged();

    // clang-format off
    Q_CLASSINFO("bundleFonts_FieldLabel", "Export font files and reference them.")
    Q_CLASSINFO("bundleFonts_FieldNote", "Only built in fonts are exported when checked. And when unchecked, fonts will be looked up in the system.")
    Q_CLASSINFO("bundleFonts_FieldEditor", "CheckBox")
    Q_PROPERTY(bool bundleFonts
               READ isBundleFonts
               WRITE setBundleFonts
               NOTIFY bundleFontsChanged)
    // clang-format on
    void setBundleFonts(bool val);
    bool isBundleFonts() const { return m_bundleFonts; }
    Q_SIGNAL void bundleFontsChanged();

    // clang-format off
    Q_CLASSINFO("embedResources_FieldLabel", "Embed fonts and images in the HTML file.")
    Q_CLASSINFO("embedResources_FieldNote", "A single HTML file is generated, without a resources folder. The file gets larger.")
    Q_CLASSINFO("embedResources_FieldEditor", "CheckBox")
    Q_PROPERTY(bool embedResources
               READ isEmbedResources
               WRITE setEmbedResources
               NOTIFY embedResourcesChanged)
    // clang-format on
    void setEmbedResources(bool val);
    bool isEmbedResources() const { return m_embedResources; }
    Q_SIGNAL void embedResourcesChanged();

    // clang-format off
    Q_CLASSINFO("mobileFriendly_FieldLabel", "Make the HTML mobile friendly.")
    Q_CLASSINFO("mobileFriendly_FieldNote", "The text reflows to fit the width of the screen, instead of being laid out on a fixed size page.")
    Q_CLASSINFO("mobileFriendly_FieldEditor", "CheckBox")
    Q_PROPERTY(bool mobileFriendly
               READ isMobileFriendly
               WRITE setMobileFriendly
               NOTIFY mobileFriendlyChanged)
    // clang-format on
    void setMobileFriendly(bool val);
    bool isMobileFriendly() const { return m_mobileFriendly; }
    Q_SIGNAL void mobileFriendlyChanged();

    bool canBundleFonts() const { return true; }
    bool requiresConfiguration() const override { return true; }

protected:
    bool doExport(QIODevice *device) override; // AbstractExporter interface
    QString fileNameExtension() const override { return QStringLiteral("html"); }

private:
    bool m_includeSceneNumbers = false;
    bool m_exportWithSceneColors = false;
    bool m_bundleFonts = true;
    bool m_includeTitlePage = true;
    bool m_embedResources = true;
    bool m_mobileFriendly = true;
};

#endif // HTMLEXPORTER_H
