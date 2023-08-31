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

static void fixOmittedScenes(QDomElement &contentE);

FinalDraftImporter::FinalDraftImporter(QObject *parent) : AbstractImporter(parent) { }

FinalDraftImporter::~FinalDraftImporter() { }

bool FinalDraftImporter::canImport(const QString &fileName) const
{
    return QFileInfo(fileName).suffix().toLower() == QStringLiteral("fdx");
}

static QColor fromFdxColorCode(const QString &code)
{
    if (code.isEmpty())
        return Qt::black;
    const QString red = code.mid(1, 2);
    const QString green = code.mid(5, 2);
    const QString blue = code.mid(9, 2);
    return QColor(code.mid(0, 1) + red + green + blue);
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
        const QString msg = QStringLiteral("Parse Error: %1 at Line %2, Column %3")
                                    .arg(errMsg)
                                    .arg(errLine)
                                    .arg(errCol);
        this->error()->setErrorMessage(msg);
        return false;
    }

    QDomElement rootE = doc.documentElement();
    if (rootE.tagName() != QStringLiteral("FinalDraft")) {
        this->error()->setErrorMessage("Not a Final-Draft file.");
        return false;
    }

    const int fdxVersion = rootE.attribute("Version").toInt();
    if (rootE.attribute("DocumentType") != QStringLiteral("Script") || fdxVersion < 1
        || fdxVersion > 5) {
        this->error()->setErrorMessage("Unrecognised Final Draft file version.");
        return false;
    }

    QDomElement contentE = rootE.firstChildElement(QStringLiteral("Content"));
    QDomNodeList paragraphs = contentE.elementsByTagName(QStringLiteral("Paragraph"));
    if (paragraphs.isEmpty()) {
        this->error()->setErrorMessage(QStringLiteral("No paragraphs to import."));
        return false;
    }

    ::fixOmittedScenes(contentE);

    Scene *scene = nullptr;
    this->progress()->setProgressStep(1.0 / qreal(paragraphs.size() + 1));
    this->configureCanvas(paragraphs.size());

    auto parseParagraphTexts =
            [](const QDomElement &paragraphE) -> QPair<QString, QVector<QTextLayout::FormatRange>> {
        QVector<QTextLayout::FormatRange> formats;
        QString text;
        if (paragraphE.isNull())
            return qMakePair(text, formats);

        const QString textN = QStringLiteral("Text");
        QDomElement textE = paragraphE.firstChildElement(textN);
        while (!textE.isNull()) {
            QTextLayout::FormatRange format;
            format.start = text.length();

            const QString textEText = textE.text();
            text += textEText;

            format.length = text.length() - format.start;

            const QStringList styles = textE.attribute(QStringLiteral("Style")).split(QChar('+'));
            if (styles.contains(QStringLiteral("Bold")))
                format.format.setFontWeight(QFont::Bold);
            if (styles.contains(QStringLiteral("Italic")))
                format.format.setFontItalic(true);
            if (styles.contains(QStringLiteral("Underline")))
                format.format.setFontUnderline(true);

            const QString colorAttr = QStringLiteral("Color");
            const QString backgroundAttr = QStringLiteral("Background");
            if (textE.hasAttribute(colorAttr))
                format.format.setForeground(QBrush(fromFdxColorCode(textE.attribute(colorAttr))));
            if (textE.hasAttribute(backgroundAttr))
                format.format.setBackground(
                        QBrush(fromFdxColorCode(textE.attribute(backgroundAttr))));

            if (!format.format.isEmpty())
                formats.append(format);

            textE = textE.nextSiblingElement(textN);
        }

        return qMakePair(text, formats);
    };

    const QStringList types({ QStringLiteral("Scene Heading"), QStringLiteral("Action"),
                              QStringLiteral("Character"), QStringLiteral("Dialogue"),
                              QStringLiteral("Parenthetical"), QStringLiteral("Shot"),
                              QStringLiteral("Transition") });
    const QString paragraphName = QStringLiteral("Paragraph");
    QDomElement paragraphE = contentE.firstChildElement(paragraphName);
    while (!paragraphE.isNull()) {
        TraverseDomElement tde(paragraphE, this->progress());

        const QString flags = paragraphE.attribute(QStringLiteral("Flags"));
        if (flags == "Ignore")
            continue;

        const QString type = paragraphE.attribute(QStringLiteral("Type"));
        const int typeIndex = types.indexOf(type);
        if (typeIndex < 0)
            continue;

        const QString alignmentHint = paragraphE.attribute(QStringLiteral("Alignment"));
        const Qt::Alignment alignment = [alignmentHint]() {
            return QHash<QString, Qt::Alignment>({ { QStringLiteral("Left"), Qt::AlignLeft },
                                                   { QStringLiteral("Right"), Qt::AlignRight },
                                                   { QStringLiteral("Center"), Qt::AlignCenter } })
                    .value(alignmentHint, Qt::Alignment());
        }();

        const QPair<QString, QVector<QTextLayout::FormatRange>> paragraphText =
                parseParagraphTexts(paragraphE);

        const QString text = paragraphText.first;
        const QVector<QTextLayout::FormatRange> formats = paragraphText.second;

        SceneElement *sceneElement = nullptr;
        switch (typeIndex) {
        case 0: {
            scene = this->createScene(text);

            ScreenplayElement *element = this->document()->screenplay()->elementAt(
                    this->document()->screenplay()->elementCount() - 1);
            element->setOmitted(flags == "Omitted");

            const QString number = paragraphE.attribute(QStringLiteral("Number"));
            if (!number.isEmpty())
                element->setUserSceneNumber(number);

            const QDomElement sceneProperiesE =
                    paragraphE.firstChildElement(QStringLiteral("SceneProperties"));
            if (!sceneProperiesE.isNull()) {
                const QString title = sceneProperiesE.attribute(QStringLiteral("Title"));
                const QColor color =
                        fromFdxColorCode(sceneProperiesE.attribute(QStringLiteral("Color")));
                scene->setColor(color);
                scene->structureElement()->setTitle(title);

                const QDomElement summaryE =
                        sceneProperiesE.firstChildElement(QStringLiteral("Summary"));
                const QDomElement summaryParagraphE = summaryE.isNull()
                        ? QDomElement()
                        : summaryE.firstChildElement(paragraphName);
                const QPair<QString, QVector<QTextLayout::FormatRange>> summaryParagraphText =
                        parseParagraphTexts(summaryParagraphE);

                // Ignore formatting, just retain the text.
                scene->setSynopsis(summaryParagraphText.first);
            }
        } break;
        case 1:
            sceneElement = this->addSceneElement(scene, SceneElement::Action, text);
            break;
        case 2:
            sceneElement = this->addSceneElement(scene, SceneElement::Character, text);
            break;
        case 3:
            sceneElement = this->addSceneElement(scene, SceneElement::Dialogue, text);
            break;
        case 4:
            sceneElement = this->addSceneElement(scene, SceneElement::Parenthetical, text);
            break;
        case 5:
            sceneElement = this->addSceneElement(scene, SceneElement::Shot, text);
            break;
        case 6:
            sceneElement = this->addSceneElement(scene, SceneElement::Transition, text);
            break;
        }

        if (sceneElement != nullptr) {
            sceneElement->setAlignment(alignment);
            sceneElement->setTextFormats(formats);
        }
    }

    return true;
}

void fixOmittedScenes(QDomElement &contentE)
{
    /**
     * The final-draft importer is built to process a flat <Paragraph> hierarchy like this.
     *
     * <FinalDraft ...>
     * <Content>
     *   <Paragraph ...>
     *
     *   </Paragraph>
     *
     *   <Paragraph ....>
     *
     *   </Paragraph>
     *
     *   ....
     * </Content>
     * </FinalDraft>
     *
     * However, if there are omitted scenes, then they show up like this in the FDX file.
     *
     * <FinalDraft ...>
     *  <Content>
     *      <Paragraph Type="Scene Heading" ...>
     *          <Text>Omitted</Text>
     *          <OmittedScene>
     *              <Paragraph Type="Scene Heading" ...>...</Paragraph>
     *              <Paragraph Type="Action" ...>...</Paragraph>
     *              <Paragraph Type="Character" ...>...</Paragraph>
     *              <Paragraph Type="Dialog" ...>...</Paragraph>
     *              <Paragraph Type="Shot" ...>...</Paragraph>
     *              .....
     *          </OmittedScene>
     *      </Paragraph>
     *      <Paragraph Type="Scene Heading">...</Paragraph>
     *      ....
     *  </Content>
     * </FinalDraft>
     *
     * We need to transform this into ...
     *
     * <FinalDraft ...>
     *  <Content>
     *     <Paragraph Flags="Omitted" Type="Scene Heading" ...>...</Paragraph>
     *     <Paragraph Flags="Omitted" Type="Action" ...>...</Paragraph>
     *     <Paragraph Flags="Omitted" Type="Character" ...>...</Paragraph>
     *     <Paragraph Flags="Omitted" Type="Dialog" ...>...</Paragraph>
     *     <Paragraph Flags="Omitted" Type="Shot" ...>...</Paragraph>
     *     .....
     *     <Paragraph Flags="Ignore" Type="Scene Heading" ...>
     *          <Text>Omitted</Text>
     *          <OmittedScene>
     *     </Paragraph>
     *     <Paragraph Type="Scene Heading">...</Paragraph>
     *     ....
     *  </Content>
     * </FinalDraft>
     *
     * So, that the existing parser can continue to import this FDX file without
     * having to make too many adjustments.
     *
     * So, that's what this function does!
     */

    const QString paragraphName = QStringLiteral("Paragraph");
    const QString omittedSceneName = QStringLiteral("OmittedScene");
    const QString flagsAttr = QStringLiteral("Flags");

    QDomElement paragraphE = contentE.firstChildElement(paragraphName);
    while (!paragraphE.isNull()) {

        QDomElement omittedSceneE = paragraphE.firstChildElement(omittedSceneName);
        if (!omittedSceneE.isNull()) {
            paragraphE.setAttribute(flagsAttr, QStringLiteral("Ignore"));
            const QDomNodeList childParagraphs = omittedSceneE.elementsByTagName(paragraphName);
            for (int i = childParagraphs.size() - 1; i >= 0; i--) {
                QDomElement childParagraphE = childParagraphs.at(i).toElement();
                childParagraphE.setAttribute(flagsAttr, QStringLiteral("Omitted"));
                contentE.insertAfter(childParagraphE, paragraphE);
            }
        }

        paragraphE = paragraphE.nextSiblingElement(paragraphName);
    }
}
