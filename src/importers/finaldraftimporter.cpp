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

#include "finaldraftimporter.h"

FinalDraftImporter::FinalDraftImporter(QObject *parent) : AbstractImporter(parent) { }

FinalDraftImporter::~FinalDraftImporter() { }

bool FinalDraftImporter::canImport(const QString &fileName) const
{
    return QFileInfo(fileName).suffix().toLower() == QStringLiteral("fdx");
}

bool FinalDraftImporter::doImport(QIODevice *device)
{
    QString errMsg;
    int errLine = -1;
    int errCol = -1;

    QDomDocument doc;
    if (!doc.setContent(device, &errMsg, &errLine, &errCol)) {
        const QString msg = QString("Parse Error: %1 at Line %2, Column %3")
                                    .arg(errMsg)
                                    .arg(errLine)
                                    .arg(errCol);
        this->error()->setErrorMessage(msg);
        return false;
    }

    QDomElement rootE = doc.documentElement();
    if (rootE.tagName() != "FinalDraft") {
        this->error()->setErrorMessage("Not a Final-Draft file.");
        return false;
    }

    const int fdxVersion = rootE.attribute("Version").toInt();
    if (rootE.attribute("DocumentType") != "Script" || fdxVersion < 1 || fdxVersion > 5) {
        this->error()->setErrorMessage("Unrecognised Final Draft file version.");
        return false;
    }

    QDomElement contentE = rootE.firstChildElement("Content");
    QDomNodeList paragraphs = contentE.elementsByTagName("Paragraph");
    if (paragraphs.isEmpty()) {
        this->error()->setErrorMessage("No paragraphs to import.");
        return false;
    }

    Scene *scene = nullptr;
    this->progress()->setProgressStep(1.0 / qreal(paragraphs.size() + 1));
    this->configureCanvas(paragraphs.size());

    static const QStringList types = QStringList() << "Scene Heading"
                                                   << "Action"
                                                   << "Character"
                                                   << "Dialogue"
                                                   << "Parenthetical"
                                                   << "Shot"
                                                   << "Transition";
    QDomElement paragraphE = contentE.firstChildElement("Paragraph");
    while (!paragraphE.isNull()) {
        TraverseDomElement tde(paragraphE, this->progress());

        const QString type = paragraphE.attribute("Type");
        const int typeIndex = types.indexOf(type);
        if (typeIndex < 0)
            continue;

        QString text;
        QDomElement textE = paragraphE.firstChildElement("Text");
        while (!textE.isNull()) {
            text += textE.text();
            textE = textE.nextSiblingElement("Text");
        }

        if (text.isEmpty())
            continue;

        switch (typeIndex) {
        case 0: {
            scene = this->createScene(text);
            const QString number = paragraphE.attribute("Number");
            ScreenplayElement *element = this->document()->screenplay()->elementAt(
                    this->document()->screenplay()->elementCount() - 1);
            element->setUserSceneNumber(number);
        } break;
        case 1:
            this->addSceneElement(scene, SceneElement::Action, text);
            break;
        case 2:
            this->addSceneElement(scene, SceneElement::Character, text);
            break;
        case 3:
            this->addSceneElement(scene, SceneElement::Dialogue, text);
            break;
        case 4:
            this->addSceneElement(scene, SceneElement::Parenthetical, text);
            break;
        case 5:
            this->addSceneElement(scene, SceneElement::Shot, text);
            break;
        case 6:
            this->addSceneElement(scene, SceneElement::Transition, text);
            break;
        }
    }

    return true;
}
