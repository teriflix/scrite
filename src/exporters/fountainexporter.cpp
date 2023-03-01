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

#include <QFileInfo>

FountainExporter::FountainExporter(QObject *parent) : AbstractExporter(parent) { }

FountainExporter::~FountainExporter() { }

bool FountainExporter::doExport(QIODevice *device)
{
    // Have tried to generate the Fountain file as closely as possible to
    // the syntax described here: https://fountain.io/syntax
    const Screenplay *screenplay = this->document()->screenplay();
    const int nrElements = screenplay->elementCount();

    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    bool hasTitleSegment = false;
    if (!screenplay->title().isEmpty()) {
        ts << "Title: " << screenplay->title();
        if (!screenplay->subtitle().isEmpty())
            ts << " (" << screenplay->subtitle() << ")\n";
        else
            ts << "\n";
        hasTitleSegment = true;
    }

    if (!screenplay->author().isEmpty()) {
        ts << "Author: " << screenplay->author() << "\n";
        hasTitleSegment = true;
    }

    if (!screenplay->contact().isEmpty()) {
        ts << "Contact: " << screenplay->contact() << "\n";
        hasTitleSegment = true;
    }

    if (!screenplay->version().isEmpty()) {
        ts << "Version: " << screenplay->version() << "\n";
        hasTitleSegment = true;
    }

    if (hasTitleSegment)
        ts << "\n\n";

    for (int i = 0; i < nrElements; i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);
        if (element->elementType() == ScreenplayElement::BreakElementType)
            ts << "#" << element->sceneID() << "\n\n";
        else {
            const Scene *scene = element->scene();
            const SceneHeading *heading = scene->heading();
            if (heading->isEnabled())
                ts << "." << heading->text() << "\n\n";

            const int nrParas = scene->elementCount();
            for (int j = 0; j < nrParas; j++) {
                const SceneElement *para = scene->elementAt(j);

                switch (para->type()) {
                case SceneElement::All:
                    break;
                case SceneElement::Shot:
                case SceneElement::Transition:
                    ts << "> ";
                    break;
                case SceneElement::Heading:
                    ts << ".";
                    break;
                case SceneElement::Character:
                    ts << "@";
                    break;
                case SceneElement::Action:
                case SceneElement::Dialogue:
                case SceneElement::Parenthetical:
                    break;
                }

                ts << para->formattedText();

                switch (para->type()) {
                case SceneElement::All:
                    break;
                case SceneElement::Transition:
                case SceneElement::Heading:
                case SceneElement::Action:
                case SceneElement::Dialogue:
                    ts << "\n\n";
                    break;
                case SceneElement::Shot:
                    ts << "<\n\n";
                    break;
                case SceneElement::Character:
                case SceneElement::Parenthetical:
                    ts << "\n";
                    break;
                }
            }
        }
    }

    return true;
}
