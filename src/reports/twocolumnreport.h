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

#ifndef TWOCOLUMNREPORT_H
#define TWOCOLUMNREPORT_H

#include "abstractreportgenerator.h"

class TwoColumnReport : public AbstractReportGenerator
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instances must be created using ScriteDocument only.")
    Q_CLASSINFO("Title", "Two Column Report")
    Q_CLASSINFO("Description", "Generate screenplay output in two columns.")
    Q_CLASSINFO("Icon", ":/icons/reports/twocolumn_report.png")

public:
    Q_INVOKABLE TwoColumnReport(QObject *parent = nullptr);
    ~TwoColumnReport();

    enum Layout { VideoAudioLayout, EverythingLeft, EverythingRight };
    Q_ENUM(Layout)

    Q_CLASSINFO("layout_FieldGroup", "Layout")
    Q_CLASSINFO("layout_FieldLabel", "Page layout")
    Q_CLASSINFO("layout_FieldEditor", "TwoColumnLayoutSelector")
    Q_PROPERTY(Layout layout READ layout WRITE setLayout NOTIFY layoutChanged)
    void setLayout(Layout val);
    Layout layout() const { return m_layout; }
    Q_SIGNAL void layoutChanged();

    Q_CLASSINFO("generateTitlePage_FieldGroup", "Options")
    Q_CLASSINFO("generateTitlePage_FieldLabel", "Generate title page.")
    Q_CLASSINFO("generateTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool generateTitlePage READ isGenerateTitlePage WRITE setGenerateTitlePage NOTIFY generateTitlePageChanged)
    void setGenerateTitlePage(bool val);
    bool isGenerateTitlePage() const { return m_generateTitlePage; }
    Q_SIGNAL void generateTitlePageChanged();

    Q_CLASSINFO("includeLogline_FieldGroup", "Options")
    Q_CLASSINFO("includeLogline_FieldLabel", "Include logline in title page.")
    Q_CLASSINFO("includeLogline_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeLogline READ isIncludeLogline WRITE setIncludeLogline NOTIFY includeLoglineChanged)
    void setIncludeLogline(bool val);
    bool isIncludeLogline() const { return m_includeLogline; }
    Q_SIGNAL void includeLoglineChanged();

    Q_CLASSINFO("includeSceneNumbers_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers READ isIncludeSceneNumbers WRITE setIncludeSceneNumbers NOTIFY includeSceneNumbersChanged)
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    Q_CLASSINFO("includeSceneIcons_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneIcons_FieldLabel", "Include scene icons.")
    Q_CLASSINFO("includeSceneIcons_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneIcons READ isIncludeSceneIcons WRITE setIncludeSceneIcons NOTIFY includeSceneIconsChanged)
    void setIncludeSceneIcons(bool val);
    bool isIncludeSceneIcons() const { return m_includeSceneIcons; }
    Q_SIGNAL void includeSceneIconsChanged();

    Q_CLASSINFO("printEachSceneOnANewPage_FieldGroup", "Options")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldLabel", "Print each scene on a new page.")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool printEachSceneOnANewPage READ isPrintEachSceneOnANewPage WRITE setPrintEachSceneOnANewPage NOTIFY printEachSceneOnANewPageChanged)
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    Q_CLASSINFO("preserveMarkupFormatting_FieldGroup", "Options")
    Q_CLASSINFO("preserveMarkupFormatting_FieldLabel", "Preserve bold, italics, and other formatting done using Markup tools.")
    Q_CLASSINFO("preserveMarkupFormatting_FieldEditor", "CheckBox")
    Q_CLASSINFO("preserveMarkupFormatting_FieldNote", "Checked by default if single font is used.")
    Q_PROPERTY(bool preserveMarkupFormatting READ isPreserveMarkupFormatting WRITE setPreserveMarkupFormatting NOTIFY preserveMarkupFormattingChanged)
    void setPreserveMarkupFormatting(bool val);
    bool isPreserveMarkupFormatting() const { return m_preserveMarkupFormatting; }
    Q_SIGNAL void preserveMarkupFormattingChanged();

    Q_CLASSINFO("useSingleFont_FieldGroup", "Options")
    Q_CLASSINFO("useSingleFont_FieldLabel", "Use a single font, instead of separate ones for each language.")
    Q_CLASSINFO("useSingleFont_FieldEditor", "CheckBox")
    Q_PROPERTY(bool useSingleFont READ isUseSingleFont WRITE setUseSingleFont NOTIFY useSingleFontChanged)
    void setUseSingleFont(bool val);
    bool isUseSingleFont() const { return m_useSingleFont; }
    Q_SIGNAL void useSingleFontChanged();

protected:
    bool doGenerate(QTextDocument *document);

private:
    Layout m_layout = VideoAudioLayout;
    bool m_useSingleFont = false;
    bool m_includeLogline = true;
    bool m_generateTitlePage = true;
    bool m_includeSceneIcons = true;
    bool m_includeSceneNumbers = true;
    bool m_preserveMarkupFormatting = true;
    bool m_printEachSceneOnANewPage = true;
};

#endif // TWOCOLUMNREPORT_H
