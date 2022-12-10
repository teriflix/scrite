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

#ifndef ABSTRACTTEXTDOCUMENTEXPORTER_H
#define ABSTRACTTEXTDOCUMENTEXPORTER_H

#include "abstractexporter.h"
#include "screenplaytextdocument.h"

class AbstractTextDocumentExporter : public AbstractExporter,
                                     public AbstractScreenplayTextDocumentInjectionInterface
{
    Q_OBJECT
    Q_INTERFACES(AbstractScreenplayTextDocumentInjectionInterface)

public:
    ~AbstractTextDocumentExporter();

    Q_CLASSINFO("listSceneCharacters_FieldLabel", "List characters for each scene")
    Q_CLASSINFO("listSceneCharacters_FieldEditor", "CheckBox")
    Q_PROPERTY(bool listSceneCharacters READ isListSceneCharacters WRITE setListSceneCharacters NOTIFY listSceneCharactersChanged)
    void setListSceneCharacters(bool val);
    bool isListSceneCharacters() const { return m_listSceneCharacters; }
    Q_SIGNAL void listSceneCharactersChanged();

    Q_CLASSINFO("includeSceneSynopsis_FieldLabel", "Include title & synopsis of each scene, if available.")
    Q_CLASSINFO("includeSceneSynopsis_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneSynopsis READ isIncludeSceneSynopsis WRITE setIncludeSceneSynopsis NOTIFY includeSceneSynopsisChanged)
    void setIncludeSceneSynopsis(bool val);
    bool isIncludeSceneSynopsis() const { return m_includeSceneSynopsis; }
    Q_SIGNAL void includeSceneSynopsisChanged();

    Q_CLASSINFO("includeSceneFeaturedImage_FieldLabel", "Include featured image for scene, if available.")
    Q_CLASSINFO("includeSceneFeaturedImage_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneFeaturedImage READ isIncludeSceneFeaturedImage WRITE setIncludeSceneFeaturedImage NOTIFY includeSceneFeaturedImageChanged)
    void setIncludeSceneFeaturedImage(bool val);
    bool isIncludeSceneFeaturedImage() const { return m_includeSceneFeaturedImage; }
    Q_SIGNAL void includeSceneFeaturedImageChanged();

    Q_CLASSINFO("includeSceneComments_FieldLabel", "Include scene comments, if available.")
    Q_CLASSINFO("includeSceneComments_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneComments READ isIncludeSceneComments WRITE setIncludeSceneComments NOTIFY includeSceneCommentsChanged)
    void setIncludeSceneComments(bool val);
    bool isIncludeSceneComments() const { return m_includeSceneComments; }
    Q_SIGNAL void includeSceneCommentsChanged();

    Q_CLASSINFO("includeSceneContents_FieldLabel", "Include scene content.")
    Q_CLASSINFO("includeSceneContents_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneContents READ isIncludeSceneContents WRITE setIncludeSceneContents NOTIFY includeSceneContentsChanged)
    void setIncludeSceneContents(bool val);
    bool isIncludeSceneContents() const { return m_includeSceneContents; }
    Q_SIGNAL void includeSceneContentsChanged();

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

    virtual bool generateTitlePage() const { return true; }
    virtual bool isIncludeLogline() const { return false; }
    virtual bool usePageBreaks() const { return false; }
    virtual bool isIncludeSceneNumbers() const { return false; }
    virtual bool isIncludeSceneIcons() const { return false; }
    virtual bool isPrintEachSceneOnANewPage() const { return false; }
    virtual bool isPrintEachActOnANewPage() const { return false; }
    virtual bool isIncludeActBreaks() const { return false; }
    virtual bool isExportForPrintingPurpose() const { return true; }

    bool requiresConfiguration() const { return true; }

protected:
    AbstractTextDocumentExporter(QObject *parent = nullptr);
    void generate(QTextDocument *textDocument, const qreal pageWidth);

    // AbstractScreenplayTextDocumentInjectionInterface interface
    bool filterSceneElement() const;

private:
    bool m_listSceneCharacters = false;
    bool m_includeSceneSynopsis = false;
    bool m_includeSceneContents = true;
    bool m_includeSceneFeaturedImage = false;
    bool m_includeSceneComments = false;
    bool m_polishParagraphs = false;
    bool m_capitalizeSentences = false;
};

#endif // ABSTRACTTEXTDOCUMENTEXPORTER_H
