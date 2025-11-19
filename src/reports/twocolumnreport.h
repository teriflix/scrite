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
    // clang-format off
    Q_CLASSINFO("Title", "Two Column Report")
    Q_CLASSINFO("Keywords", "audio video format,av format,left right format")
    Q_CLASSINFO("Description", "Generate screenplay output in two columns.")
    Q_CLASSINFO("Icon", ":/icons/reports/twocolumn_report.png")
    // clang-format on

public:
    Q_INVOKABLE TwoColumnReport(QObject *parent = nullptr);
    ~TwoColumnReport();

    enum Layout { VideoAudioLayout, EverythingLeft, EverythingRight };
    Q_ENUM(Layout)

    // clang-format off
    Q_CLASSINFO("layout_FieldGroup", "Layout")
    Q_CLASSINFO("layout_FieldLabel", "Page layout")
    Q_CLASSINFO("layout_FieldEditor", "TwoColumnLayoutSelector")
    Q_PROPERTY(Layout layout
               READ layout
               WRITE setLayout
               NOTIFY layoutChanged)
    // clang-format on
    void setLayout(Layout val);
    Layout layout() const { return m_layout; }
    Q_SIGNAL void layoutChanged();

    // clang-format off
    Q_CLASSINFO("leftColumnWidth_FieldGroup", "Layout")
    Q_CLASSINFO("leftColumnWidth_FieldLabel", "Column Width Distribution")
    Q_CLASSINFO("leftColumnWidth_FieldEditor", "TwoColumnWidthDistributionEditor")
    Q_PROPERTY(qreal leftColumnWidth
               READ leftColumnWidth
               WRITE setLeftColumnWidth
               NOTIFY leftColumnWidthChanged)
    // clang-format on
    void setLeftColumnWidth(qreal val);
    qreal leftColumnWidth() const { return m_leftColumnWidth; }
    Q_SIGNAL void leftColumnWidthChanged();

    // clang-format off
    Q_CLASSINFO("generateTitlePage_FieldGroup", "Options")
    Q_CLASSINFO("generateTitlePage_FieldLabel", "Generate title page.")
    Q_CLASSINFO("generateTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool generateTitlePage
               READ isGenerateTitlePage
               WRITE setGenerateTitlePage
               NOTIFY generateTitlePageChanged)
    // clang-format on
    void setGenerateTitlePage(bool val);
    bool isGenerateTitlePage() const { return m_generateTitlePage; }
    Q_SIGNAL void generateTitlePageChanged();

    // clang-format off
    Q_CLASSINFO("includeLogline_FieldGroup", "Options")
    Q_CLASSINFO("includeLogline_FieldLabel", "Include logline in title page.")
    Q_CLASSINFO("includeLogline_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeLogline
               READ isIncludeLogline
               WRITE setIncludeLogline
               NOTIFY includeLoglineChanged)
    // clang-format on
    void setIncludeLogline(bool val);
    bool isIncludeLogline() const { return m_includeLogline; }
    Q_SIGNAL void includeLoglineChanged();

    // clang-format off
    Q_CLASSINFO("includeSceneNumbers_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers.")
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
    Q_CLASSINFO("includeSceneIcons_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneIcons_FieldLabel", "Include scene icons.")
    Q_CLASSINFO("includeSceneIcons_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneIcons
               READ isIncludeSceneIcons
               WRITE setIncludeSceneIcons
               NOTIFY includeSceneIconsChanged)
    // clang-format on
    void setIncludeSceneIcons(bool val);
    bool isIncludeSceneIcons() const { return m_includeSceneIcons; }
    Q_SIGNAL void includeSceneIconsChanged();

    // clang-format off
    Q_CLASSINFO("printEachSceneOnANewPage_FieldGroup", "Options")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldLabel", "Print each scene on a new page.")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool printEachSceneOnANewPage
               READ isPrintEachSceneOnANewPage
               WRITE setPrintEachSceneOnANewPage
               NOTIFY printEachSceneOnANewPageChanged)
    // clang-format on
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    // clang-format off
    Q_CLASSINFO("ignorePageBreaks_FieldGroup", "Options")
    Q_CLASSINFO("ignorePageBreaks_FieldLabel", "Ignore page breaks.")
    Q_CLASSINFO("ignorePageBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool ignorePageBreaks
               READ isIgnorePageBreaks
               WRITE setIgnorePageBreaks
               NOTIFY ignorePageBreaksChanged)
    // clang-format on
    void setIgnorePageBreaks(bool val);
    bool isIgnorePageBreaks() const { return m_ignorePageBreaks; }
    Q_SIGNAL void ignorePageBreaksChanged();

    // clang-format off
    Q_CLASSINFO("preserveMarkupFormatting_FieldGroup", "Options")
    Q_CLASSINFO("preserveMarkupFormatting_FieldLabel", "Preserve bold, italics, and other formatting done using Markup tools.")
    Q_CLASSINFO("preserveMarkupFormatting_FieldEditor", "CheckBox")
    Q_CLASSINFO("preserveMarkupFormatting_FieldNote", "Checked by default if single font is used.")
    Q_PROPERTY(bool preserveMarkupFormatting
               READ isPreserveMarkupFormatting
               WRITE setPreserveMarkupFormatting
               NOTIFY preserveMarkupFormattingChanged)
    // clang-format on
    void setPreserveMarkupFormatting(bool val);
    bool isPreserveMarkupFormatting() const { return m_preserveMarkupFormatting; }
    Q_SIGNAL void preserveMarkupFormattingChanged();

    // clang-format off
    Q_CLASSINFO("useSingleFont_FieldGroup", "Options")
    Q_CLASSINFO("useSingleFont_FieldLabel", "Use a single font, instead of separate ones for each language.")
    Q_CLASSINFO("useSingleFont_FieldEditor", "CheckBox")
    Q_PROPERTY(bool useSingleFont
               READ isUseSingleFont
               WRITE setUseSingleFont
               NOTIFY useSingleFontChanged)
    // clang-format on
    void setUseSingleFont(bool val);
    bool isUseSingleFont() const { return m_useSingleFont; }
    Q_SIGNAL void useSingleFontChanged();

    // clang-format off
    Q_CLASSINFO("characterNames_FieldGroup", "Characters")
    Q_CLASSINFO("characterNames_FieldLabel", "Characters to include in the report")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_CLASSINFO("characterNames_IsPersistent", "false")
    Q_PROPERTY(QStringList characterNames
               READ characterNames
               WRITE setCharacterNames
               NOTIFY characterNamesChanged)
    // clang-format on
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    // clang-format off
    Q_CLASSINFO("highlightCharacterDialogues_FieldGroup", "Characters")
    Q_CLASSINFO("highlightCharacterDialogues_FieldLabel", "Highlight dialogues of characters selected above.")
    Q_CLASSINFO("highlightCharacterDialogues_FieldEditor", "CheckBox")
    Q_PROPERTY(bool highlightCharacterDialogues
               READ isHighlightCharacterDialogues
               WRITE setHighlightCharacterDialogues
               NOTIFY highlightCharacterDialoguesChanged)
    // clang-format on
    void setHighlightCharacterDialogues(bool val);
    bool isHighlightCharacterDialogues() const { return m_highlightCharacterDialogues; }
    Q_SIGNAL void highlightCharacterDialoguesChanged();

    // clang-format off
    Q_CLASSINFO("sceneNumbers_FieldGroup", "Scenes")
    Q_CLASSINFO("sceneNumbers_FieldLabel", "Scenes to include in the report")
    Q_CLASSINFO("sceneNumbers_FieldEditor", "MultipleSceneSelector")
    Q_CLASSINFO("sceneNumbers_FieldNote", "If no scenes are selected, then the report is generted for all scenes in the " "screenplay.")
    Q_CLASSINFO("sceneNumbers_IsPersistent", "false")
    Q_PROPERTY(QList<int> sceneNumbers
               READ sceneIndexes
               WRITE setSceneIndexes
               NOTIFY sceneIndexesChanged)
    // clang-format on
    void setSceneIndexes(const QList<int> &val);
    QList<int> sceneIndexes() const { return m_sceneIndexes; }
    Q_SIGNAL void sceneIndexesChanged();

    // clang-format off
    Q_CLASSINFO("episodeNumbers_FieldGroup", "Episodes")
    Q_CLASSINFO("episodeNumbers_FieldLabel", "Episodes to include in the report")
    Q_CLASSINFO("episodeNumbers_FieldEditor", "MultipleEpisodeSelector")
    Q_CLASSINFO("episodeNumbers_FieldNote", "If no episodes are selected, then the report is generted for all episodes in the " "screenplay.")
    Q_CLASSINFO("episodeNumbers_IsPersistent", "false")
    Q_PROPERTY(QList<int> episodeNumbers
               READ episodeNumbers
               WRITE setEpisodeNumbers
               NOTIFY episodeNumbersChanged)
    // clang-format on
    void setEpisodeNumbers(const QList<int> &val);
    QList<int> episodeNumbers() const { return m_episodeNumbers; }
    Q_SIGNAL void episodeNumbersChanged();

    // clang-format off
    Q_CLASSINFO("tags_FieldGroup", "Tags")
    Q_CLASSINFO("tags_FieldLabel", "Groups/Tags to include in the report")
    Q_CLASSINFO("tags_FieldEditor", "MultipleTagGroupSelector")
    Q_CLASSINFO( "tags_FieldNote", "If no tags are selected, then the report is generated for all tags in the screenplay.")
    Q_CLASSINFO("tags_IsPersistent", "false")
    Q_PROPERTY(QStringList tags
               READ tags
               WRITE setTags
               NOTIFY tagsChanged)
    // clang-format on
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

protected:
    bool doGenerate(QTextDocument *document);
    bool requiresOdtContentPolish() const;
    bool polishOdtContent(QDomDocument &);

    // These functions must return true if element should be included in the
    // final report.
    bool includeElementByTag(const ScreenplayElement *element) const;
    bool includeElementByCharacter(const ScreenplayElement *element) const;
    bool includeElementBySceneIndex(const ScreenplayElement *element) const;
    bool includeElementByEpisodeNumber(const ScreenplayElement *element) const;

private:
    Layout m_layout = VideoAudioLayout;
    QStringList m_tags;
    bool m_useSingleFont = false;
    bool m_includeLogline = true;
    qreal m_leftColumnWidth = 0.5;
    bool m_generateTitlePage = true;
    bool m_includeSceneIcons = true;
    bool m_includeSceneNumbers = true;
    bool m_highlightCharacterDialogues = false;
    QList<int> m_sceneIndexes;
    QList<int> m_episodeNumbers;
    QStringList m_characterNames;
    bool m_preserveMarkupFormatting = true;
    bool m_printEachSceneOnANewPage = true;
    bool m_ignorePageBreaks = false;
};

#endif // TWOCOLUMNREPORT_H
