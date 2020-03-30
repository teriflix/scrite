/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "finaldraftimporter.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>

FinalDraftImporter::FinalDraftImporter(QObject *parent)
    : AbstractImporter(parent)
{

}

FinalDraftImporter::~FinalDraftImporter()
{

}

class TraverseDomElement
{
public:
    TraverseDomElement(QDomElement &element, ProgressReport *progress)
        : m_element(&element), m_progress(progress) { }
    ~TraverseDomElement() {
        *m_element = m_element->nextSiblingElement(m_element->tagName());
        m_progress->tick();
    }

private:
    QDomElement *m_element;
    ProgressReport *m_progress;
};

bool FinalDraftImporter::doImport(QIODevice *device)
{
    QString errMsg;
    int errLine = -1;
    int errCol = -1;

    QDomDocument doc;
    if( !doc.setContent(device, &errMsg, &errLine, &errCol) )
    {
        const QString msg = QString("Parse Error: %1 at Line %2, Column %3").arg(errMsg).arg(errLine).arg(errCol);
        this->error()->setErrorMessage(msg);
        return false;
    }

    QDomElement rootE = doc.documentElement();
    if(rootE.tagName() != "FinalDraft")
    {
        this->error()->setErrorMessage("Not a Final-Draft file.");
        return false;
    }

    const int fdxVersion = rootE.attribute("Version").toInt();
    if(rootE.attribute("DocumentType") != "Script" || fdxVersion < 2 || fdxVersion >4)
    {
        this->error()->setErrorMessage("Unrecognised Final Draft file version.");
        return false;
    }

    QDomElement contentE = rootE.firstChildElement("Content");
    QDomNodeList paragraphs = contentE.elementsByTagName("Paragraph");
    if(paragraphs.isEmpty())
    {
        this->error()->setErrorMessage("No paragraphs to import.");
        return false;
    }

    this->progress()->setProgressStep(1.0 / qreal(paragraphs.size()+1));

    static const QList<QColor> sceneColors = QList<QColor>() <<
            QColor("purple") << QColor("blue") << QColor("orange") <<
            QColor("red") << QColor("brown") << QColor("gray");
    Structure *structure = this->document()->structure();
    Screenplay *screenplay = this->document()->screenplay();
    Scene *scene = nullptr;

    static const qreal elementX = 275;
    static const qreal elementY = 100;
    static const qreal elementSpacing = 120;
    static const qreal canvasSpaceBuffer = 500;

    const qreal requiredSpace = paragraphs.size()*elementSpacing + canvasSpaceBuffer;
    if(structure->canvasHeight() < requiredSpace)
    {
        structure->setCanvasWidth(requiredSpace);
        structure->setCanvasHeight(requiredSpace);
    }

    auto createScene = [&structure,&screenplay](const QString &heading) {
        const int sceneIndex = structure->elementCount();

        StructureElement *structureElement = new StructureElement(structure);
        Scene *scene = new Scene(structureElement);
        scene->setColor(sceneColors.at(sceneIndex%sceneColors.length()));
        structureElement->setScene(scene);
        structureElement->setX(elementX);
        structureElement->setY(elementY + elementSpacing*sceneIndex);
        structure->addElement(structureElement);

        ScreenplayElement *screenplayElement = new ScreenplayElement(screenplay);
        screenplayElement->setScene(scene);
        screenplay->addElement(screenplayElement);

        const int field1SepLoc = heading.indexOf(' ');
        const int field2SepLoc = heading.lastIndexOf('-');
        const QString locationType = heading.left(field1SepLoc).trimmed();
        const QString moment = heading.mid(field2SepLoc+1).trimmed();
        const QString location = heading.mid(field1SepLoc+1,(field2SepLoc-field1SepLoc-1)).trimmed();

        scene->heading()->setEnabled(true);
        scene->heading()->setLocationType(SceneHeading::locationTypeStringMap().key(locationType));
        scene->heading()->setLocation(location);

        const QString titleBit = location.length() > 50 ? location.left(47) + "..." : location;
        scene->setTitle( QString("[%1] %2").arg(sceneIndex+1).arg(titleBit.toLower()) );

        scene->heading()->setMoment(SceneHeading::momentStringMap().key(moment));

        return scene;
    };

    auto addSceneElement = [&scene](SceneElement::Type type, const QString &text) {
        SceneElement *element = new SceneElement(scene);
        element->setType(type);
        element->setText(text);
        scene->addElement(element);
        return element;
    };

    static const QStringList types = QStringList()
            << "Scene Heading" << "Action" << "Character"
            << "Dialogue" << "Parenthetical" << "Shot"
            << "Transition";
    QDomElement paragraphE = contentE.firstChildElement("Paragraph");
    while(!paragraphE.isNull())
    {
        TraverseDomElement tde(paragraphE, this->progress());

        const QString type = paragraphE.attribute("Type");
        const int typeIndex = types.indexOf(type);
        if(typeIndex < 0)
            continue;

        const QString text = paragraphE.firstChildElement("Text").text();
        if(text.isEmpty())
            continue;

        switch(typeIndex)
        {
        case 0:
            scene = createScene(text);
            break;
        case 1:
            addSceneElement(SceneElement::Action, text);
            break;
        case 2:
            addSceneElement(SceneElement::Character, text);
            break;
        case 3:
            addSceneElement(SceneElement::Dialogue, text);
            break;
        case 4:
            addSceneElement(SceneElement::Parenthetical, text);
            break;
        case 5:
            addSceneElement(SceneElement::Shot, text);
            break;
        case 6:
            addSceneElement(SceneElement::Transition, text);
            break;
        }
    }

    return true;
}
