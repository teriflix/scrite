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

#include "abstractscreenplaysubsetreport.h"
#include "qtextdocumentpagedprinter.h"
#include "screenplaytextdocument.h"
#include "application.h"
#include "scene.h"

AbstractScreenplaySubsetReport::AbstractScreenplaySubsetReport(QObject *parent)
    : AbstractReportGenerator(parent)
{

}

AbstractScreenplaySubsetReport::~AbstractScreenplaySubsetReport()
{

}

void AbstractScreenplaySubsetReport::setGenerateTitlePage(bool val)
{
    if(m_generateTitlePage == val)
        return;

    m_generateTitlePage = val;
    emit generateTitlePageChanged();
}

void AbstractScreenplaySubsetReport::setListSceneCharacters(bool val)
{
    if(m_listSceneCharacters == val)
        return;

    m_listSceneCharacters = val;
    emit listSceneCharactersChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneSynopsis(bool val)
{
    if(m_includeSceneSynopsis == val)
        return;

    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneContents(bool val)
{
    if(m_includeSceneContents == val)
        return;

    m_includeSceneContents = val;
    emit includeSceneContentsChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneNumbers(bool val)
{
    if(m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void AbstractScreenplaySubsetReport::setIncludeSceneIcons(bool val)
{
    if(m_includeSceneIcons == val)
        return;

    m_includeSceneIcons = val;
    emit includeSceneIconsChanged();
}

void AbstractScreenplaySubsetReport::setPrintEachSceneOnANewPage(bool val)
{
    if(m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();
}

bool AbstractScreenplaySubsetReport::doGenerate(QTextDocument *textDocument)
{
    ScriteDocument *document = this->document();
    Screenplay *screenplay = document->screenplay();

    if(m_screenplaySubset)
        delete m_screenplaySubset;

    m_screenplaySubset = new Screenplay(this);
    m_screenplaySubset->setEmail(screenplay->email());
    m_screenplaySubset->setTitle(screenplay->title());
    m_screenplaySubset->setAuthor(screenplay->author());
    m_screenplaySubset->setAddress(screenplay->address());
    m_screenplaySubset->setBasedOn(screenplay->basedOn());
    m_screenplaySubset->setContact(screenplay->contact());
    m_screenplaySubset->setVersion(screenplay->version());
    m_screenplaySubset->setSubtitle(this->screenplaySubtitle());
    m_screenplaySubset->setPhoneNumber(screenplay->phoneNumber());
    m_screenplaySubset->setProperty("#useDocumentScreenplayForCoverPagePhoto", true);

    for(int i=0; i<screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = screenplay->elementAt(i);
        if(this->includeScreenplayElement(element))
        {
            ScreenplayElement *element2 = new ScreenplayElement(m_screenplaySubset);
            element2->setElementType(element->elementType());
            if(element->elementType() == ScreenplayElement::BreakElementType)
                element2->setBreakType(element->breakType());
            element2->setScene(element->scene());
            element2->setProperty("#sceneNumber", element->sceneNumber());
            element2->setUserSceneNumber(element->userSceneNumber());
            m_screenplaySubset->addElement(element2);
        }
    }

    ScreenplayTextDocument stDoc;
    stDoc.setTitlePage(this->format() == AdobePDF ? m_generateTitlePage : false);
    stDoc.setSceneNumbers(this->format() == AdobePDF ? m_includeSceneNumbers : false);
    stDoc.setSceneIcons(this->format() == AdobePDF ? m_includeSceneIcons : false);
    stDoc.setListSceneCharacters(m_listSceneCharacters);
    stDoc.setPrintEachSceneOnANewPage(this->format() == AdobePDF ? m_printEachSceneOnANewPage : false);
    stDoc.setSyncEnabled(false);
    if(this->format() == AdobePDF)
        stDoc.setPurpose(ScreenplayTextDocument::ForPrinting);
    else
        stDoc.setPurpose(ScreenplayTextDocument::ForDisplay);
    stDoc.setScreenplay(m_screenplaySubset);
    stDoc.setFormatting(document->printFormat());
    stDoc.setTextDocument(textDocument);
    stDoc.setIncludeSceneSynopsis(m_includeSceneSynopsis);
    stDoc.setInjection(this);
    this->configureScreenplayTextDocument(stDoc);
    stDoc.syncNow();

    return true;
}

void AbstractScreenplaySubsetReport::configureTextDocumentPrinter(QTextDocumentPagedPrinter *printer, const QTextDocument *)
{
    printer->header()->setVisibleFromPageOne(!m_generateTitlePage);
    printer->footer()->setVisibleFromPageOne(!m_generateTitlePage);
    printer->watermark()->setVisibleFromPageOne(true);
}

void AbstractScreenplaySubsetReport::inject(QTextCursor &, AbstractScreenplayTextDocumentInjectionInterface::InjectLocation)
{
    // Incase we need to plug something in.
}

bool AbstractScreenplaySubsetReport::filterSceneElement() const
{
    return !m_includeSceneContents;
}
