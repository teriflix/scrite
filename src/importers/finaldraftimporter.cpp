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
#include "application.h"

#include <QXmlSimpleReader>

FinalDraftImporter::FinalDraftImporter(QObject *parent) : AbstractImporter(parent) { }

FinalDraftImporter::~FinalDraftImporter() { }

bool FinalDraftImporter::canImport(const QString &fileName) const
{
    return QFileInfo(fileName).suffix().toLower() == QLatin1String("fdx");
}

bool FinalDraftImporter::doImport(QIODevice *device)
{
    QString errMsg;
    int errLine = -1;
    int errCol = -1;

    /**
     * We cannot use QDomDocument::setContent(QIODevice*, QString*, int*, int*)
     * because DOM Elements with spaces will be read as empty strings, instead of
     * actual number of spaces. This is obviously a problem for us.
     *
     * The only way to address that is to actually use a QXmlInputSource over the
     * QIODevice, and then parse that using QXmlSimpleReader instance.
     *
     * In Qt 5.15, QXmlInputSource and QXmlSimpleReader classes are depricated.
     * But until we can find a replacement that also parses spaces properly,
     * we will have to simply use these deprecated classes.
     */

    QXmlInputSource xmlInputSource(device);
    QXmlSimpleReader xmlParser;

    QDomDocument doc;
    if (!doc.setContent(&xmlInputSource, &xmlParser, &errMsg, &errLine, &errCol)) {
        const QString msg = QLatin1String("Parse Error: %1 at Line %2, Column %3")
                                    .arg(errMsg)
                                    .arg(errLine)
                                    .arg(errCol);
        this->error()->setErrorMessage(msg);
        return false;
    }

    QDomElement rootE = doc.documentElement();
    if (rootE.tagName() != QLatin1String("FinalDraft")) {
        this->error()->setErrorMessage("Not a Final-Draft file.");
        return false;
    }

    const int fdxVersion = rootE.attribute("Version").toInt();
    if (rootE.attribute("DocumentType") != QLatin1String("Script") || fdxVersion < 1
        || fdxVersion > 5) {
        this->error()->setErrorMessage("Unrecognised Final Draft file version.");
        return false;
    }

    QDomElement contentE = rootE.firstChildElement(QLatin1String("Content"));
    QDomNodeList paragraphs = contentE.elementsByTagName(QLatin1String("Paragraph"));
    if (paragraphs.isEmpty()) {
        this->error()->setErrorMessage(QLatin1String("No paragraphs to import."));
        return false;
    }

    Scene *scene = nullptr;
    this->progress()->setProgressStep(1.0 / qreal(paragraphs.size() + 1));
    this->configureCanvas(paragraphs.size());

    static const QStringList types({ QLatin1String("Scene Heading"), QLatin1String("Action"),
                                     QLatin1String("Character"), QLatin1String("Dialogue"),
                                     QLatin1String("Parenthetical"), QLatin1String("Shot"),
                                     QLatin1String("Transition") });
    QDomElement paragraphE = contentE.firstChildElement(QLatin1String("Paragraph"));
    while (!paragraphE.isNull()) {
        TraverseDomElement tde(paragraphE, this->progress());

        const QString type = paragraphE.attribute(QLatin1String("Type"));
        const int typeIndex = types.indexOf(type);
        if (typeIndex < 0)
            continue;

        const QString textN = QLatin1String("Text");
        QString text;
        QDomElement textE = paragraphE.firstChildElement(textN);
        while (!textE.isNull()) {
            const QString textEText = textE.text();
            text += textEText;
            textE = textE.nextSiblingElement(textN);
        }

        if (text.isEmpty())
            continue;

        switch (typeIndex) {
        case 0: {
            scene = this->createScene(text);
            const QString number = paragraphE.attribute(QLatin1String("Number"));
            if (!number.isEmpty()) {
                ScreenplayElement *element = this->document()->screenplay()->elementAt(
                        this->document()->screenplay()->elementCount() - 1);
                element->setUserSceneNumber(number);
            }
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
