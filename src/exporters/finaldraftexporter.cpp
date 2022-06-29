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

#include "finaldraftexporter.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>
#include <QFileInfo>

FinalDraftExporter::FinalDraftExporter(QObject *parent) : AbstractExporter(parent) { }

FinalDraftExporter::~FinalDraftExporter() { }

void FinalDraftExporter::setMarkLanguagesExplicitly(bool val)
{
    if (m_markLanguagesExplicitly == val)
        return;

    m_markLanguagesExplicitly = val;
    emit markLanguagesExplicitlyChanged();
}

void FinalDraftExporter::setUseScriteFonts(bool val)
{
    if (m_useScriteFonts == val)
        return;

    m_useScriteFonts = val;
    emit useScriteFontsChanged();
}

bool FinalDraftExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const Structure *structure = this->document()->structure();
    QStringList moments = structure->standardMoments();
    QStringList locationTypes = structure->standardLocationTypes();

    const int nrElements = screenplay->elementCount();
    if (screenplay->elementCount() == 0) {
        this->error()->setErrorMessage(
                QStringLiteral("There are no scenes in the screenplay to export."));
        return false;
    }

    this->progress()->setProgressStep(1.0 / qreal(nrElements + 1));

    QDomDocument doc;

    QDomElement rootE = doc.createElement(QStringLiteral("FinalDraft"));
    rootE.setAttribute(QStringLiteral("DocumentType"), QStringLiteral("Script"));
    rootE.setAttribute(QStringLiteral("Template"), QStringLiteral("No"));
    rootE.setAttribute(QStringLiteral("Version"), QStringLiteral("2"));
    doc.appendChild(rootE);

    QDomElement contentE = doc.createElement(QStringLiteral("Content"));
    rootE.appendChild(contentE);

    auto addTextToParagraph = [&doc, this](QDomElement &element, const QString &text) {
        if (m_markLanguagesExplicitly) {
            const QList<TransliterationEngine::Boundary> breakup =
                    TransliterationEngine::instance()->evaluateBoundaries(text, true);
            for (const TransliterationEngine::Boundary &item : breakup) {
                QDomElement textE = doc.createElement(QStringLiteral("Text"));
                element.appendChild(textE);
                if (item.language == TransliterationEngine::English) {
                    textE.setAttribute(QStringLiteral("Font"),
                                       QStringLiteral("Courier Final Draft"));
                    textE.setAttribute(QStringLiteral("Language"), QStringLiteral("English"));
                } else {
                    const QFont font = TransliterationEngine::instance()->languageFont(
                            item.language, m_useScriteFonts);
                    textE.setAttribute(QStringLiteral("Font"), font.family());
                    textE.setAttribute(
                            QStringLiteral("Language"),
                            TransliterationEngine::instance()->languageAsString(item.language));
                }
                textE.appendChild(doc.createTextNode(item.string));
            }
        } else {
            QDomElement textE = doc.createElement(QStringLiteral("Text"));
            element.appendChild(textE);
            textE.setAttribute(QStringLiteral("Font"), QStringLiteral("Courier Final Draft"));
            textE.appendChild(doc.createTextNode(text));
        }
    };

    QStringList locations;
    for (int i = 0; i < nrElements; i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);
        if (element->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const Scene *scene = element->scene();
        const SceneHeading *heading = scene->heading();

        if (heading->isEnabled()) {
            QDomElement paragraphE = doc.createElement(QStringLiteral("Paragraph"));
            contentE.appendChild(paragraphE);

            paragraphE.setAttribute(QStringLiteral("Type"), QStringLiteral("Scene Heading"));
            if (element->hasUserSceneNumber())
                paragraphE.setAttribute(QStringLiteral("Number"), element->userSceneNumber());

            addTextToParagraph(paragraphE, heading->text());
            locations.append(heading->location());

            if (!locationTypes.contains(heading->locationType()))
                locationTypes.append(heading->locationType());

            if (!moments.contains(heading->moment()))
                moments.append(heading->moment());
        }

        const int nrSceneElements = scene->elementCount();
        for (int j = 0; j < nrSceneElements; j++) {
            const SceneElement *sceneElement = scene->elementAt(j);
            QDomElement paragraphE = doc.createElement(QStringLiteral("Paragraph"));
            contentE.appendChild(paragraphE);

            paragraphE.setAttribute(QStringLiteral("Type"), sceneElement->typeAsString());
            addTextToParagraph(paragraphE, sceneElement->formattedText());
        }

        this->progress()->tick();
    }

    QDomElement watermarkingE = doc.createElement(QStringLiteral("Watermarking"));
    rootE.appendChild(watermarkingE);
    watermarkingE.setAttribute(QStringLiteral("Text"), qApp->applicationName());

    QDomElement smartTypeE = doc.createElement("SmartType");
    rootE.appendChild(smartTypeE);

    const QStringList characters = structure->allCharacterNames();
    QDomElement charactersE = doc.createElement(QStringLiteral("Characters"));
    smartTypeE.appendChild(charactersE);
    for (const QString &name : qAsConst(characters)) {
        QDomElement characterE = doc.createElement(QStringLiteral("Character"));
        charactersE.appendChild(characterE);
        characterE.appendChild(doc.createTextNode(name));
    }

    locations.removeDuplicates();
    std::sort(locations.begin(), locations.end());

    QDomElement timesOfDayE = doc.createElement(QStringLiteral("TimesOfDay"));
    smartTypeE.appendChild(timesOfDayE);
    timesOfDayE.setAttribute(QStringLiteral("Separator"), QStringLiteral(" - "));
    std::sort(moments.begin(), moments.end());
    for (const QString &moment : qAsConst(moments)) {
        QDomElement timeOfDayE = doc.createElement(QStringLiteral("TimeOfDay"));
        timesOfDayE.appendChild(timeOfDayE);
        timeOfDayE.appendChild(doc.createTextNode(moment));
    }

    std::sort(locationTypes.begin(), locationTypes.end());
    QDomElement sceneIntrosE = doc.createElement(QStringLiteral("SceneIntros"));
    smartTypeE.appendChild(sceneIntrosE);
    sceneIntrosE.setAttribute(QStringLiteral("Separator"), QStringLiteral(". "));
    for (const QString &locationType : qAsConst(locationTypes)) {
        QDomElement sceneIntroE = doc.createElement(QStringLiteral("SceneIntro"));
        sceneIntrosE.appendChild(sceneIntroE);
        sceneIntroE.appendChild(doc.createTextNode(locationType));
    }

    const QString xml = doc.toString(4);

    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    ts << xml;
    ts.flush();

    return true;
}

QString FinalDraftExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if (fi.suffix().toLower() != "fdx")
        return fileName + ".fdx";
    return fileName;
}
