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

#include "fountainimporter.h"
#include "application.h"
#include "fountain.h"

#include <QBuffer>
#include <QRegExp>

FountainImporter::FountainImporter(QObject *parent) : AbstractImporter(parent) { }

FountainImporter::~FountainImporter() { }

bool FountainImporter::canImport(const QString &fileName) const
{
    const QStringList suffixes = { QStringLiteral("fountain"), QStringLiteral("txt") };
    return suffixes.contains(QFileInfo(fileName).suffix().toLower());
}

bool FountainImporter::doImport(QIODevice *device)
{
    ScriteDocument *doc = this->document();
    Screenplay *screenplay = doc->screenplay();

    Fountain::Parser parser(device);

    const QMap<QString, QString> titlePage = [parser]() -> QMap<QString, QString> {
        QMap<QString, QString> ret;

        const auto tp = parser.titlePage();
        for (const auto &tpitem : tp)
            ret[tpitem.first] = tpitem.second;

        return ret;
    }();

    screenplay->setTitle(titlePage.value("title"));
    screenplay->setSubtitle(titlePage.value("subtitle"));
    screenplay->setLogline(titlePage.value("logline"));
    screenplay->setBasedOn(titlePage.value("basedon"));
    screenplay->setAuthor(titlePage.value("authors"));
    screenplay->setContact(titlePage.value("contact"));
    screenplay->setAddress(titlePage.value("address"));
    screenplay->setPhoneNumber(titlePage.value("phone"));
    screenplay->setEmail(titlePage.value("email"));
    screenplay->setWebsite(titlePage.value("website"));
    screenplay->setVersion(titlePage.value("version"));

    const auto body = parser.body();

    Scene *currentScene = nullptr;
    for (const auto &element : body) {
        if (element.type == Fountain::Element::Section) {
            if (element.sectionDepth == 1) {
                screenplay->addBreakElement(Screenplay::Act);

                ScreenplayElement *act = screenplay->elementAt(screenplay->elementCount() - 1);
                act->setBreakSubtitle(element.text);
            }
            continue;
        }

        if (element.type == Fountain::Element::SceneHeading) {
            currentScene = this->createScene(element.text);

            if (!element.sceneNumber.isEmpty()) {
                ScreenplayElement *spElement =
                        screenplay->elementAt(screenplay->elementCount() - 1);
                spElement->setUserSceneNumber(element.sceneNumber);
            }

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
            break;
        case Fountain::Element::Character:
            para->setType(SceneElement::Character);
            break;
        case Fountain::Element::Parenthetical:
            para->setType(SceneElement::Parenthetical);
            break;
        case Fountain::Element::Dialogue:
            para->setType(SceneElement::Dialogue);
            break;
        case Fountain::Element::Shot:
            para->setType(SceneElement::Shot);
            break;
        case Fountain::Element::Transition:
            para->setType(SceneElement::Transition);
            break;
        }

        currentScene->addElement(para);
    }

    return true;
}
