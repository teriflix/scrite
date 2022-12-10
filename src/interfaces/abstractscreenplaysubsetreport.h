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

#ifndef ABSTRACTSCREENPLAYSUBSETREPORT_H
#define ABSTRACTSCREENPLAYSUBSETREPORT_H

#include "screenplaytextdocument.h"
#include "abstractreportgenerator.h"

class AbstractScreenplaySubsetReport : public AbstractReportGenerator,
                                       public AbstractScreenplayTextDocumentInjectionInterface
{
    Q_OBJECT
    Q_INTERFACES(AbstractScreenplayTextDocumentInjectionInterface)

public:
    ~AbstractScreenplaySubsetReport();

    bool requiresConfiguration() const { return true; }

    Q_CLASSINFO("listSceneCharacters_FieldGroup", "Options")
    Q_CLASSINFO("listSceneCharacters_FieldLabel", "List characters for each scene.")
    Q_CLASSINFO("listSceneCharacters_FieldEditor", "CheckBox")
    Q_PROPERTY(bool listSceneCharacters READ isListSceneCharacters WRITE setListSceneCharacters
                       NOTIFY listSceneCharactersChanged)
    void setListSceneCharacters(bool val);
    bool isListSceneCharacters() const { return m_listSceneCharacters; }
    Q_SIGNAL void listSceneCharactersChanged();

    Q_CLASSINFO("includeSceneSynopsis_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneSynopsis_FieldLabel", "Include synopsis of each scene.")
    Q_CLASSINFO("includeSceneSynopsis_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneSynopsis READ isIncludeSceneSynopsis WRITE setIncludeSceneSynopsis
                       NOTIFY includeSceneSynopsisChanged)
    void setIncludeSceneSynopsis(bool val);
    bool isIncludeSceneSynopsis() const { return m_includeSceneSynopsis; }
    Q_SIGNAL void includeSceneSynopsisChanged();

    Q_CLASSINFO("includeSceneFeaturedImage_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneFeaturedImage_FieldLabel",
                "Include featured image for scene, if available.")
    Q_CLASSINFO("includeSceneFeaturedImage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneFeaturedImage READ isIncludeSceneFeaturedImage WRITE
                       setIncludeSceneFeaturedImage NOTIFY includeSceneFeaturedImageChanged)
    void setIncludeSceneFeaturedImage(bool val);
    bool isIncludeSceneFeaturedImage() const { return m_includeSceneFeaturedImage; }
    Q_SIGNAL void includeSceneFeaturedImageChanged();

    Q_CLASSINFO("includeSceneComments_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneComments_FieldLabel", "Include scene comments, if available.")
    Q_CLASSINFO("includeSceneComments_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneComments READ isIncludeSceneComments WRITE setIncludeSceneComments
                       NOTIFY includeSceneCommentsChanged)
    void setIncludeSceneComments(bool val);
    bool isIncludeSceneComments() const { return m_includeSceneComments; }
    Q_SIGNAL void includeSceneCommentsChanged();

    Q_CLASSINFO("includeSceneContents_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneContents_FieldLabel", "Include scene content.")
    Q_CLASSINFO("includeSceneContents_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneContents READ isIncludeSceneContents WRITE setIncludeSceneContents
                       NOTIFY includeSceneContentsChanged)
    void setIncludeSceneContents(bool val);
    bool isIncludeSceneContents() const { return m_includeSceneContents; }
    Q_SIGNAL void includeSceneContentsChanged();

    Q_CLASSINFO("includeSceneNumbers_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers READ isIncludeSceneNumbers WRITE setIncludeSceneNumbers
                       NOTIFY includeSceneNumbersChanged)
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    Q_CLASSINFO("generateTitlePage_FieldGroup", "Options")
    Q_CLASSINFO("generateTitlePage_FieldLabel", "Generate title page. (PDF Only)")
    Q_CLASSINFO("generateTitlePage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool generateTitlePage READ isGenerateTitlePage WRITE setGenerateTitlePage NOTIFY
                       generateTitlePageChanged)
    void setGenerateTitlePage(bool val);
    bool isGenerateTitlePage() const { return m_generateTitlePage; }
    Q_SIGNAL void generateTitlePageChanged();

    Q_CLASSINFO("includeLogline_FieldGroup", "Options")
    Q_CLASSINFO("includeLogline_FieldLabel", "Include logline in title page. (PDF Only)")
    Q_CLASSINFO("includeLogline_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeLogline READ isIncludeLogline WRITE setIncludeLogline NOTIFY includeLoglineChanged)
    void setIncludeLogline(bool val);
    bool isIncludeLogline() const { return m_includeLogline; }
    Q_SIGNAL void includeLoglineChanged();

    Q_CLASSINFO("includeSceneIcons_FieldGroup", "Options")
    Q_CLASSINFO("includeSceneIcons_FieldLabel",
                "Include scene icons in the generated PDF. (PDF Only)")
    Q_CLASSINFO("includeSceneIcons_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneIcons READ isIncludeSceneIcons WRITE setIncludeSceneIcons NOTIFY
                       includeSceneIconsChanged)
    void setIncludeSceneIcons(bool val);
    bool isIncludeSceneIcons() const { return m_includeSceneIcons; }
    Q_SIGNAL void includeSceneIconsChanged();

    Q_CLASSINFO("printEachSceneOnANewPage_FieldGroup", "Options")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldLabel", "Print each scene on a new page. (PDF Only)")
    Q_CLASSINFO("printEachSceneOnANewPage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool printEachSceneOnANewPage READ isPrintEachSceneOnANewPage WRITE
                       setPrintEachSceneOnANewPage NOTIFY printEachSceneOnANewPageChanged)
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    Q_CLASSINFO("includeActBreaks_FieldGroup", "Options")
    Q_CLASSINFO("includeActBreaks_FieldLabel", "Include act breaks.")
    Q_CLASSINFO("includeActBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeActBreaks READ isIncludeActBreaks WRITE setIncludeActBreaks NOTIFY
                       includeActBreaksChanged)
    void setIncludeActBreaks(bool val);
    bool isIncludeActBreaks() const { return m_includeActBreaks; }
    Q_SIGNAL void includeActBreaksChanged();

    // This property is not presented to the user, because it will be consistent with
    // options configured in Settings.
    Q_PROPERTY(bool capitalizeSentences READ isCapitalizeSentences WRITE setCapitalizeSentences NOTIFY capitalizeSentencesChanged DESIGNABLE false)
    void setCapitalizeSentences(bool val);
    bool isCapitalizeSentences() const { return m_capitalizeSentences; }
    Q_SIGNAL void capitalizeSentencesChanged();

    // This property is not presented to the user, because it will be consistent with
    // options configured in Settings.
    Q_PROPERTY(bool polishParagraphs READ isPolishParagraphs WRITE setPolishParagraphs NOTIFY polishParagraphsChanged DESIGNABLE false)
    void setPolishParagraphs(bool val);
    bool isPolishParagraphs() const { return m_polishParagraphs; }
    Q_SIGNAL void polishParagraphsChanged();

    Q_CLASSINFO("episodeNumbers_FieldGroup", "Episodes")
    Q_CLASSINFO("episodeNumbers_FieldLabel", "Episodes to include in the report")
    Q_CLASSINFO("episodeNumbers_FieldEditor", "MultipleEpisodeSelector")
    Q_CLASSINFO("episodeNumbers_FieldNote",
                "If no episodes are selected, then the report is generted for all episodes in the "
                "screenplay.")
    Q_PROPERTY(QList<int> episodeNumbers READ episodeNumbers WRITE setEpisodeNumbers NOTIFY
                       episodeNumbersChanged)
    void setEpisodeNumbers(const QList<int> &val);
    QList<int> episodeNumbers() const { return m_episodeNumbers; }
    Q_SIGNAL void episodeNumbersChanged();

    Q_CLASSINFO("tags_FieldGroup", "Tags")
    Q_CLASSINFO("tags_FieldLabel", "Groups/Tags to include in the report")
    Q_CLASSINFO("tags_FieldEditor", "MultipleTagGroupSelector")
    Q_CLASSINFO(
            "tags_FieldNote",
            "If no tags are selected, then the report is generated for all tags in the screenplay.")
    Q_PROPERTY(QStringList tags READ tags WRITE setTags NOTIFY tagsChanged)
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

    virtual QString screenplaySubtitle() const { return QStringLiteral("Screenplay Subset"); }
    virtual bool includeScreenplayElement(const ScreenplayElement *) const { return true; }

protected:
    AbstractScreenplaySubsetReport(QObject *parent = nullptr);

    Screenplay *screenplaySubset() const { return m_screenplaySubset; }

    // AbstractReportGenerator interface
    bool doGenerate(QTextDocument *);

    // AbstractReportGenerator interface
    void configureTextDocumentPrinter(QTextDocumentPagedPrinter *, const QTextDocument *);

    // AbstractScreenplayTextDocumentInjectionInterface interface
    void inject(QTextCursor &, InjectLocation);
    bool filterSceneElement() const;

    virtual void configureScreenplayTextDocument(ScreenplayTextDocument &) { }

private:
    QStringList m_tags;
    bool m_generateTitlePage = true;
    bool m_includeLogline = false;
    bool m_printSceneContent = true;
    bool m_includeSceneIcons = true;
    bool m_includeSceneNumbers = true;
    bool m_listSceneCharacters = false;
    bool m_includeSceneContents = true;
    bool m_includeSceneSynopsis = false;
    bool m_includeSceneFeaturedImage = false;
    bool m_includeSceneComments = false;
    bool m_includeActBreaks = false;
    bool m_polishParagraphs = false;
    bool m_capitalizeSentences = false;
    QList<int> m_episodeNumbers;
    Screenplay *m_screenplaySubset = nullptr;
    bool m_printEachSceneOnANewPage = false;
};

#endif // ABSTRACTSCREENPLAYSUBSETREPORT_H
