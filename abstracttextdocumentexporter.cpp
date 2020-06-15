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

void AbstractTextDocumentExporter::setUsePageBreaks(bool val)
{
    if(m_usePageBreaks == val)
        return;

    m_usePageBreaks = val;
    emit usePageBreaksChanged();
}

void AbstractTextDocumentExporter::generate(QTextDocument *textDoc, const qreal pageWidth)
{
    Q_UNUSED(pageWidth)

    ScreenplayTextDocument stDoc;
    stDoc.setTitlePage(true);
    stDoc.setSceneNumbers(this->isIncludeSceneNumbers());
    stDoc.setSyncEnabled(false);
    if(m_usePageBreaks)
        stDoc.setPurpose(ScreenplayTextDocument::ForPrinting);
    else
        stDoc.setPurpose(ScreenplayTextDocument::ForDisplay);
    stDoc.setScreenplay(this->document()->screenplay());
    stDoc.setFormatting(this->document()->printFormat());
    stDoc.setTextDocument(textDoc);
    stDoc.syncNow();
}

