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

#include "user.h"
#include "scrite.h"
#include "undoredo.h"
#include "application.h"
#include "abstractimporter.h"

#include <QFile>
#include <QScopeGuard>
#include <QRandomGenerator>
#include <QRegularExpression>

AbstractImporter::AbstractImporter(QObject *parent) : AbstractDeviceIO(parent)
{
    connect(User::instance(), &User::infoChanged, this, &AbstractImporter::featureEnabledChanged);
}

AbstractImporter::~AbstractImporter() { }

QString AbstractImporter::format() const
{
    const int cii = this->metaObject()->indexOfClassInfo("Format");
    return QString::fromLatin1(this->metaObject()->classInfo(cii).value());
}

QString AbstractImporter::nameFilters() const
{
    const int cii = this->metaObject()->indexOfClassInfo("NameFilters");
    return QString::fromLatin1(this->metaObject()->classInfo(cii).value());
}

bool AbstractImporter::isFeatureEnabled() const
{
    if (User::instance()->isLoggedIn()) {
        const bool allImportersEnabled = User::instance()->isFeatureEnabled(Scrite::ImportFeature);
        const bool thisSpecificImporterEnabled = allImportersEnabled
                ? User::instance()->isFeatureNameEnabled(QStringLiteral("import/") + this->format())
                : false;
        return allImportersEnabled && thisSpecificImporterEnabled;
    }

    return QStringList({ QStringLiteral("Final Draft"), QStringLiteral("Fountain") })
            .contains(this->format());
}

bool AbstractImporter::read()
{
    QString fileName = this->fileName();
    ScriteDocument *document = this->document();

    this->error()->clear();

    if (!this->isFeatureEnabled()) {
        this->error()->setErrorMessage(QStringLiteral("Importing from ") + this->format()
                                       + QStringLiteral(" is not enabled."));
        return false;
    }

    if (fileName.isEmpty()) {
        this->error()->setErrorMessage(QStringLiteral("Nothing to import."));
        return false;
    }

    if (document == nullptr) {
        this->error()->setErrorMessage(QStringLiteral("No document available to import into."));
        return false;
    }

    document->reset();

    // Remove any blank scenes created in reset()
    Structure *structure = document->structure();
    while (structure->elementCount())
        structure->removeElement(structure->elementAt(0));

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        this->error()->setErrorMessage(
                QString("Could not open file '%1' for reading.").arg(fileName));
        return false;
    }

    auto guard = qScopeGuard([=]() {
        const QString importerName = QString::fromLatin1(this->metaObject()->className());
        User::instance()->logActivity2(QStringLiteral("import"), importerName);
    });

    const QMetaObject *mo = this->metaObject();
    const QMetaClassInfo classInfo = mo->classInfo(mo->indexOfClassInfo("Format"));
    this->progress()->setProgressText(QString("Importing from \"%1\"").arg(classInfo.value()));

    ScriteDocument *doc = this->document();
    Screenplay *screenplay = doc->screenplay();

    this->progress()->start();
    UndoStack::ignoreUndoCommands = true;
    const bool ret = this->doImport(&file);
    if (ret) {
        Structure *structure = doc->structure();
        for (int i = 0; i < structure->elementCount(); i++) {
            StructureElement *element = structure->elementAt(i);
            if (element != nullptr && element->scene() != nullptr)
                element->scene()->inferTitleFromContent();
        }
    }
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
    const qreal requiredSpace = nrBlocks * elementYSpacing + canvasSpaceBuffer;
    if (structure->canvasHeight() < requiredSpace) {
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
    scene->setColor(sceneColors.at(rand->bounded(sceneColors.length() - 1)));
#endif
    structureElement->setScene(scene);
    structureElement->setX(elementX + (sceneIndex % 2 ? elementXSpacing : 0));
    structureElement->setY(elementY + elementYSpacing * sceneIndex);
    structure->addElement(structureElement);

    ScreenplayElement *screenplayElement = new ScreenplayElement(screenplay);
    screenplayElement->setScene(scene);
    screenplay->addElement(screenplayElement);

    scene->heading()->setEnabled(true);
    scene->heading()->parseFrom(heading);

    const QString location = scene->heading()->location();
    const QString titleBit = location.length() > 50 ? location.left(47) + "..." : location;
    scene->setTitle(QString("Scene number #%1 at %2").arg(sceneIndex + 1).arg(titleBit.toLower()));

    return scene;
}

SceneElement *AbstractImporter::addSceneElement(Scene *scene, SceneElement::Type type,
                                                const QString &text)
{
    if (scene == nullptr || type == SceneElement::Heading || text.isEmpty())
        return nullptr;

    SceneElement *element = new SceneElement(scene);
    element->setType(type);
    element->setText(text);
    scene->addElement(element);
    return element;
}
