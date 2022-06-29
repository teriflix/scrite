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

#include "htmlimporter.h"

HtmlImporter::HtmlImporter(QObject *parent) : AbstractImporter(parent) { }

HtmlImporter::~HtmlImporter() { }

bool HtmlImporter::canImport(const QString &fileName) const
{
    return QFileInfo(fileName).suffix().toLower() == QStringLiteral("html");
}

bool HtmlImporter::doImport(QIODevice *device)
{
    const QByteArray bytes = this->preprocess(device);
    return this->importFrom(bytes);
}

QByteArray HtmlImporter::preprocess(QIODevice *device) const
{
    QByteArray bytes = device->readAll();

    // Check for celtx HTML format
    static const QByteArray celtxSignature("chrome://celtx/");
    if (bytes.indexOf(celtxSignature) >= 0) {
        const int index = bytes.indexOf("<body>");
        bytes.remove(0, index);
        bytes.prepend("<html>");

        bytes.replace("<br>", "");
        bytes.replace("class=\"sceneheading\"", "class=\"heading\"");
    }

    return bytes;
}

bool HtmlImporter::importFrom(const QByteArray &bytes)
{
    QString errMsg;
    int errLine = -1;
    int errCol = -1;

    QDomDocument htmlDoc;
    if (!htmlDoc.setContent(bytes, &errMsg, &errLine, &errCol)) {
        const QString msg = QString("Parse Error: %1 at Line %2, Column %3")
                                    .arg(errMsg)
                                    .arg(errLine)
                                    .arg(errCol);
        this->error()->setErrorMessage(msg);
        return false;
    }

    const QDomElement rootE = htmlDoc.documentElement();
    const QDomElement bodyE = rootE.firstChildElement("body");
    if (bodyE.isNull()) {
        this->error()->setErrorMessage("Could not find <BODY> tag.");
        return false;
    }

    const QDomNodeList pList = bodyE.elementsByTagName("p");
    if (pList.isEmpty()) {
        this->error()->setErrorMessage("No paragraphs to import.");
        return false;
    }

    this->progress()->setProgressStep(1.0 / qreal(pList.size() + 1));
    this->configureCanvas(pList.size());

    static const QStringList types = QStringList() << "heading"
                                                   << "action"
                                                   << "character"
                                                   << "dialog"
                                                   << "parenthetical"
                                                   << "shot"
                                                   << "transition";

    Scene *scene = nullptr;
    QDomElement paragraphE = bodyE.firstChildElement("p");
    while (!paragraphE.isNull()) {
        TraverseDomElement tde(paragraphE, this->progress());

        const QString type = paragraphE.attribute("class");
        const int typeIndex = types.indexOf(type);
        if (typeIndex < 0)
            continue;

        QString text = paragraphE.text().trimmed();
        text = text.replace("\r\n", " ");
        text = text.replace("\n", " ");
        if (text.isEmpty())
            continue;

        if (typeIndex == 0)
            scene = this->createScene(text);
        else {
            if (scene == nullptr) {
                scene = this->createScene(QStringLiteral("INT. SOMEWHERE - DAY"));
                scene->heading()->setEnabled(false);
                scene->setTitle(QString());
            }

            switch (typeIndex) {
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
    }

    return true;
}
