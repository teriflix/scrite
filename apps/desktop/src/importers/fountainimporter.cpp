/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "fountainimporter.h"
#include "fountain.h"
#include "user.h"
#include "scritedocument.h"

#include <QBuffer>
#include <QMimeData>
#include <QClipboard>
#include <QScopeGuard>
#include <QGuiApplication>

FountainImporter::FountainImporter(QObject *parent) : AbstractImporter(parent) { }

FountainImporter::~FountainImporter() { }

bool FountainImporter::canImport(const QString &fileName) const
{
    const QStringList suffixes = { QStringLiteral("fountain"), QStringLiteral("txt") };
    return suffixes.contains(QFileInfo(fileName).suffix().toLower());
}

bool FountainImporter::importFromClipboard()
{
    ScriteDocument *document = this->document();
    document->reset();

    // Remove any blank scenes created in reset()
    Screenplay *screenplay = document->screenplay();
    while (screenplay->elementCount())
        screenplay->removeElement(screenplay->elementAt(0));

    Structure *structure = document->structure();
    while (structure->elementCount())
        structure->removeElement(structure->elementAt(0));

    auto guard = qScopeGuard([=]() {
        const QString importerName = QString::fromLatin1(this->metaObject()->className());
        User::instance()->logActivity2(QStringLiteral("import"), importerName + "-Clipboard");
    });

    const QString text = qApp->clipboard()->mimeData()->text();

    Fountain::Parser parser(text);
    return this->doImport(parser);
}

bool FountainImporter::doImport(QIODevice *device)
{
    Fountain::Parser parser(device);
    return this->doImport(parser);
}

bool FountainImporter::doImport(const Fountain::Parser &parser)
{
    ScriteDocument *doc = this->document();
    Screenplay *screenplay = doc->screenplay();

    Fountain::loadTitlePage(parser.titlePage(), screenplay);

    const auto body = parser.body();

    Scene *currentScene = nullptr;
    SceneElement *lastCharacterElement = nullptr;

    for (const auto &element : body) {
        if (element.type == Fountain::Element::Section) {
            if (element.sectionDepth == 1) {
                screenplay->addBreakElement(Screenplay::Act);

                ScreenplayElement *act = screenplay->elementAt(screenplay->elementCount() - 1);
                act->setBreakSubtitle(element.text);
            }
            lastCharacterElement = nullptr;
            continue;
        }

        if (element.type == Fountain::Element::SceneHeading) {
            currentScene = this->createScene(element.text);

            if (!element.sceneNumber.isEmpty()) {
                ScreenplayElement *spElement =
                        screenplay->elementAt(screenplay->elementCount() - 1);
                spElement->setUserSceneNumber(element.sceneNumber);
            }

            lastCharacterElement = nullptr;
            continue;
        }

        if (element.type == Fountain::Element::Synopsis) {
            ScreenplayElement *lastElement = screenplay->elementAt(screenplay->elementCount() - 1);
            if (lastElement) {
                QString synopsis = lastElement->elementType() == ScreenplayElement::SceneElementType
                        ? lastElement->scene()->synopsis()
                        : lastElement->breakSummary();
                if (!synopsis.isEmpty())
                    synopsis += "\n\n";
                synopsis += element.text;

                if (lastElement->elementType() == ScreenplayElement::SceneElementType)
                    lastElement->scene()->setSynopsis(synopsis);
                else
                    lastElement->setBreakSummary(synopsis);
            }
            lastCharacterElement = nullptr;
            continue;
        }

        if (element.text.isEmpty())
            continue;

        if (!currentScene) {
            currentScene = this->createScene(QString());
            currentScene->heading()->setEnabled(false);
        }

        SceneElement *para = new SceneElement(currentScene);
        para->setText(element.text);
        para->setTextFormats(element.formats);
        if (element.isCentered)
            para->setAlignment(Qt::AlignHCenter);

        switch (element.type) {
        default:
        case Fountain::Element::Action:
            para->setType(SceneElement::Action);
            lastCharacterElement = nullptr;
            break;
        case Fountain::Element::Character:
            para->setType(SceneElement::Character);
            if (element.isDualDialogueRightColumn && lastCharacterElement) {
                para->setProperty("#markForDualDialogueRight", true);
            }
            lastCharacterElement = para;
            break;
        case Fountain::Element::Parenthetical:
            para->setType(SceneElement::Parenthetical);
            break;
        case Fountain::Element::Dialogue:
            para->setType(SceneElement::Dialogue);
            break;
        case Fountain::Element::Shot:
            para->setType(SceneElement::Shot);
            lastCharacterElement = nullptr;
            break;
        case Fountain::Element::Transition:
            para->setType(SceneElement::Transition);
            lastCharacterElement = nullptr;
            break;
        }

        currentScene->addElement(para);
    }

    processDualDialogueMarkers(screenplay);

    return true;
}

void FountainImporter::processDualDialogueMarkers(Screenplay *screenplay)
{
    const int nrElements = screenplay->elementCount();
    for (int i = 0; i < nrElements; i++) {
        ScreenplayElement *spElement = screenplay->elementAt(i);
        if (spElement->elementType() != ScreenplayElement::SceneElementType)
            continue;

        Scene *scene = spElement->scene();
        const int nrSceneElements = scene->elementCount();

        for (int j = 0; j < nrSceneElements; j++) {
            SceneElement *element = scene->elementAt(j);

            if (!element->property("#markForDualDialogueRight").toBool())
                continue;

            if (element->type() != SceneElement::Character)
                continue;

            if (j == 0)
                continue;

            SceneElement *previousCharacter = nullptr;
            for (int k = j - 1; k >= 0; k--) {
                SceneElement *candidate = scene->elementAt(k);
                if (candidate->type() == SceneElement::Character) {
                    previousCharacter = candidate;
                    break;
                }
            }

            if (previousCharacter) {
                scene->toggleDualDialogue(previousCharacter);
            }

            element->setProperty("#markForDualDialogueRight", QVariant());
        }
    }
}
