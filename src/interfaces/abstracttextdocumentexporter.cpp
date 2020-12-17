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

#include "abstracttextdocumentexporter.h"
#include "screenplaytextdocument.h"

AbstractTextDocumentExporter::AbstractTextDocumentExporter(QObject *parent)
    :AbstractExporter(parent)
{

}

AbstractTextDocumentExporter::~AbstractTextDocumentExporter()
{

}

void AbstractTextDocumentExporter::setListSceneCharacters(bool val)
{
    if(m_listSceneCharacters == val)
        return;

    m_listSceneCharacters = val;
    emit listSceneCharactersChanged();
}

void AbstractTextDocumentExporter::setIncludeSceneSynopsis(bool val)
{
    if(m_includeSceneSynopsis == val)
        return;

    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();
}

void AbstractTextDocumentExporter::setIncludeSceneContents(bool val)
{
    if(m_includeSceneContents == val)
        return;

    m_includeSceneContents = val;
    emit includeSceneContentsChanged();
}

void AbstractTextDocumentExporter::generate(QTextDocument *textDoc, const qreal pageWidth)
{
    Q_UNUSED(pageWidth)

    ScreenplayTextDocument stDoc;
    stDoc.setTitlePage(this->generateTitlePage());
    stDoc.setSceneNumbers(this->isIncludeSceneNumbers());
    stDoc.setSceneIcons(this->isIncludeSceneIcons());
    stDoc.setListSceneCharacters(m_listSceneCharacters);
    stDoc.setIncludeSceneSynopsis(m_includeSceneSynopsis);
    stDoc.setPrintEachSceneOnANewPage(this->isPrintEachSceneOnANewPage());
    stDoc.setSyncEnabled(false);
    if(this->usePageBreaks() && m_includeSceneContents)
        stDoc.setPurpose(ScreenplayTextDocument::ForPrinting);
    else
        stDoc.setPurpose(ScreenplayTextDocument::ForDisplay);
    stDoc.setScreenplay(this->document()->screenplay());
    stDoc.setFormatting(this->document()->printFormat());
    stDoc.setTitlePageIsCentered(this->document()->screenplay()->isTitlePageIsCentered());
    stDoc.setTextDocument(textDoc);
    stDoc.setInjection(this);
    stDoc.syncNow();
}

bool AbstractTextDocumentExporter::filterSceneElement() const
{
    return !m_includeSceneContents;
}

