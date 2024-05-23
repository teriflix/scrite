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

#include "fountainexporter.h"
#include "fountain.h"

#include <QFileInfo>

FountainExporter::FountainExporter(QObject *parent) : AbstractExporter(parent) { }

FountainExporter::~FountainExporter() { }

bool FountainExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const int nrElements = screenplay->elementCount();

    using GetterType = QString (Screenplay::*)() const;
    const QList<QPair<QString, GetterType>> titlePageGetters = {
        { "Title", &Screenplay::title },
        { "Subtitle", &Screenplay::subtitle },
        { "Based On", &Screenplay::basedOn },
        { "Authors", &Screenplay::author },
        { "Contact", &Screenplay::contact },
        { "Address", &Screenplay::address },
        { "Phone Number", &Screenplay::phoneNumber },
        { "Website", &Screenplay::website },
        { "Email", &Screenplay::email },
        { "Logline", &Screenplay::logline }
    };

    Fountain::TitlePage fTitlePage;
    for (const auto &titlePageGetter : titlePageGetters) {
        const QString value = ((*screenplay).*titlePageGetter.second)();
        if (!value.isEmpty())
            fTitlePage.append(qMakePair(titlePageGetter.first, value));
    }

    Fountain::Body fBody;

    for (int i = 0; i < nrElements; i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);

        if (element->elementType() == ScreenplayElement::BreakElementType) {
            Fountain::Element fElement;
            fElement.type = Fountain::Element::Section;
            fElement.sectionDepth = 1;
            fElement.text = element->sceneID();
            fBody.append(fElement);
            continue;
        }

        const Scene *scene = element->scene();
        const SceneHeading *heading = scene->heading();

        Fountain::Element fSceneHeading;
        fSceneHeading.type = Fountain::Element::SceneHeading;
        fSceneHeading.text = heading->isEnabled() ? heading->text() : QString();
        fSceneHeading.sceneNumber = element->userSceneNumber();
        fBody.append(fSceneHeading);

        if (element->isOmitted()) {
            Fountain::Element fOmittedPara;
            fOmittedPara.type = Fountain::Element::Action;
            fOmittedPara.notes << "Omitted";
            fBody.append(fOmittedPara);
            continue;
        }

        const int nrParas = scene->elementCount();
        for (int j = 0; j < nrParas; j++) {
            const SceneElement *para = scene->elementAt(j);

            Fountain::Element fPara;
            fPara.text = para->formattedText();
            fPara.formats = para->textFormats();

            switch (para->type()) {
            case SceneElement::Shot:
                fPara.type = Fountain::Element::Shot;
                break;
            case SceneElement::Transition:
                fPara.type = Fountain::Element::Transition;
                break;
            case SceneElement::Character:
                fPara.type = Fountain::Element::Character;
                break;
            case SceneElement::Action:
                fPara.type = Fountain::Element::Action;
                break;
            case SceneElement::Dialogue:
                fPara.type = Fountain::Element::Dialogue;
                break;
            case SceneElement::Parenthetical:
                fPara.type = Fountain::Element::Parenthetical;
                break;
            default:
                break;
            }

            if (fPara.type != fPara.None)
                fBody.append(fPara);
        }
    }

    Fountain::Writer writer(fTitlePage, fBody);

    return writer.write(device);
}
