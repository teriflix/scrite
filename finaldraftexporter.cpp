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

#include "finaldraftexporter.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>

FinalDraftExporter::FinalDraftExporter(QObject *parent)
                   :AbstractExporter(parent)
{

}

FinalDraftExporter::~FinalDraftExporter()
{

}

bool FinalDraftExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const Structure *structure = this->document()->structure();
    const int nrElements = screenplay->elementCount();
    if(screenplay->elementCount() == 0)
    {
        this->error()->setErrorMessage("There are no scenes in the screenplay to export.");
        return false;
    }

    this->progress()->setProgressStep( 1.0/qreal(nrElements+1) );

    QDomDocument doc;

    QDomElement rootE = doc.createElement("FinalDraft");
    rootE.setAttribute("DocumentType", "Script");
    rootE.setAttribute("Template", "No");
    rootE.setAttribute("Version", "2");
    doc.appendChild(rootE);

    QDomElement contentE = doc.createElement("Content");
    rootE.appendChild(contentE);

    auto addTextToParagraph = [&doc](QDomElement &element, const QString &text) {
        QDomElement textE = doc.createElement("Text");
        element.appendChild(textE);
        textE.appendChild(doc.createTextNode(text));
    };

    QStringList locations;
    for(int i=0; i<nrElements; i++)
    {
        const ScreenplayElement *element = screenplay->elementAt(i);
        if(element->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const Scene *scene = element->scene();
        const SceneHeading *heading = scene->heading();

        if(heading->isEnabled())
        {
            QDomElement paragraphE = doc.createElement("Paragraph");
            contentE.appendChild(paragraphE);

            paragraphE.setAttribute("Type", "Scene Heading");
            addTextToParagraph(paragraphE, heading->toString());
            locations.append(heading->location());
        }

        const int nrSceneElements = scene->elementCount();
        for(int j=0; j<nrSceneElements; j++)
        {
            const SceneElement *sceneElement = scene->elementAt(j);
            QDomElement paragraphE = doc.createElement("Paragraph");
            contentE.appendChild(paragraphE);

            paragraphE.setAttribute("Type", sceneElement->typeAsString());
            addTextToParagraph(paragraphE, sceneElement->text());
        }

        this->progress()->tick();
    }

    QDomElement watermarkingE = doc.createElement("Watermarking");
    rootE.appendChild(watermarkingE);
    watermarkingE.setAttribute("Text", qApp->applicationName());

    QDomElement smartTypeE = doc.createElement("SmartType");
    rootE.appendChild(smartTypeE);

    const QStringList characters = structure->allCharacterNames();
    QDomElement charactersE = doc.createElement("Characters");
    smartTypeE.appendChild(charactersE);
    Q_FOREACH(QString name, characters)
    {
        QDomElement characterE = doc.createElement("Character");
        charactersE.appendChild(characterE);
        characterE.appendChild(doc.createTextNode(name));
    }

    locations.removeDuplicates();
    std::sort(locations.begin(), locations.end());

    QDomElement timesOfDayE = doc.createElement("TimesOfDay");
    smartTypeE.appendChild(timesOfDayE);
    timesOfDayE.setAttribute("Separator", " - ");
    const QStringList moments = SceneHeading::momentStringMap().values();
    Q_FOREACH(QString moment, moments)
    {
        QDomElement timeOfDayE = doc.createElement("TimeOfDay");
        timesOfDayE.appendChild(timeOfDayE);
        timeOfDayE.appendChild(doc.createTextNode(moment));
    }

    static const QStringList locationTypes = QStringList() << "INT" << "EXT" << "I/E";
    QDomElement sceneIntrosE = doc.createElement("SceneIntros");
    smartTypeE.appendChild(sceneIntrosE);
    sceneIntrosE.setAttribute("Separator", ". ");
    Q_FOREACH(QString locationType, locationTypes)
    {
        QDomElement sceneIntroE = doc.createElement("SceneIntro");
        sceneIntrosE.appendChild(sceneIntroE);
        sceneIntroE.appendChild(doc.createTextNode(locationType));
    }

    const QString xml = doc.toString(4);

    QTextStream ts(device);
    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    ts << xml;
    ts.flush();

    return true;
}
