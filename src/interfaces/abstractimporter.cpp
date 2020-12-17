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

#include "undoredo.h"
#include "application.h"
#include "abstractimporter.h"

#include <QFile>
#include <QRandomGenerator>
#include <QRegularExpression>

AbstractImporter::AbstractImporter(QObject *parent)
                 :AbstractDeviceIO(parent)
{

}

AbstractImporter::~AbstractImporter()
{

}

bool AbstractImporter::read()
{
    QString fileName = this->fileName();
    ScriteDocument *document = this->document();

    this->error()->clear();

    if(fileName.isEmpty())
    {
        this->error()->setErrorMessage("Nothing to import.");
        return false;
    }

    if(document == nullptr)
    {
        this->error()->setErrorMessage("No document available to import into.");
        return false;
    }

    document->reset();

    QFile file(fileName);
    if( !file.open(QFile::ReadOnly) )
    {
        this->error()->setErrorMessage( QString("Could not open file '%1' for reading.").arg(fileName) );
        return false;
    }

    const QMetaObject *mo = this->metaObject();
    const QMetaClassInfo classInfo = mo->classInfo(mo->indexOfClassInfo("Format"));
    this->progress()->setProgressText( QString("Importing from \"%1\"").arg(classInfo.value()));

    ScriteDocument *doc = this->document();
    Screenplay *screenplay = doc->screenplay();

    this->progress()->start();
    UndoStack::ignoreUndoCommands = true;
    const bool ret = this->doImport(&file);
    screenplay->setCurrentElementIndex(0);
    UndoStack::ignoreUndoCommands = false;
    UndoStack::clearAllStacks();
    this->progress()->finish();

    GarbageCollector::instance()->add(this);

    return ret;
}

static const qreal elementX = 5000;
static const qreal elementY = 5000;
static const qreal elementXSpacing = 400;
static const qreal elementYSpacing = 400;
static const qreal canvasSpaceBuffer = 500;

void AbstractImporter::configureCanvas(int nrBlocks)
{
    Structure *structure = this->document()->structure();
    const qreal requiredSpace = nrBlocks*elementYSpacing + canvasSpaceBuffer;
    if(structure->canvasHeight() < requiredSpace)
    {
        structure->setCanvasWidth(requiredSpace);
        structure->setCanvasHeight(requiredSpace);
    }
}

Scene *AbstractImporter::createScene(const QString &heading)
{
    const QVector<QColor> sceneColors = Application::standardColors(QVersionNumber());
    Structure *structure = this->document()->structure();
    Screenplay *screenplay = this->document()->screenplay();
    Scene *scene = nullptr;

    const int sceneIndex = structure->elementCount();

    StructureElement *structureElement = new StructureElement(structure);
    scene = new Scene(structureElement);
#if 0
    scene->setColor(sceneColors.at(sceneIndex%sceneColors.length()));
#else
    QRandomGenerator *rand = QRandomGenerator::global();
    scene->setColor( sceneColors.at(rand->bounded(sceneColors.length()-1)) );
#endif
    structureElement->setScene(scene);
    structureElement->setX(elementX + (sceneIndex%2 ? elementXSpacing : 0));
    structureElement->setY(elementY + elementYSpacing*sceneIndex);
    structure->addElement(structureElement);

    ScreenplayElement *screenplayElement = new ScreenplayElement(screenplay);
    screenplayElement->setScene(scene);
    screenplay->addElement(screenplayElement);

    scene->heading()->setEnabled(true);
    scene->heading()->parseFrom(heading);

    const QString location = scene->heading()->location();
    const QString titleBit = location.length() > 50 ? location.left(47) + "..." : location;
    scene->setTitle( QString("Scene number #%1 at %2").arg(sceneIndex+1).arg(titleBit.toLower()) );

    return scene;
}

SceneElement *AbstractImporter::addSceneElement(Scene *scene, SceneElement::Type type, const QString &text)
{
    if(scene == nullptr || type == SceneElement::Heading || text.isEmpty())
        return nullptr;

    SceneElement *element = new SceneElement(scene);
    element->setType(type);
    element->setText(text);
    scene->addElement(element);
    return element;
}
