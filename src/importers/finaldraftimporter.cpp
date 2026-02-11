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

#include "finaldraftimporter.h"
#include "application.h"

#include <QXmlSimpleReader>

static QString FDX_Suffix = QStringLiteral("fdx");
static QString FDX_RootTag = QStringLiteral("FinalDraft");
static QString FDX_VersionAttr = QStringLiteral("Version");
static QString FDX_DocumentTypeAttr = QStringLiteral("DocumentType");
static QString FDX_ScriptDocumentType = QStringLiteral("Script");
static QString FDX_ContentTag = QStringLiteral("Content");
static QString FDX_ParagraphTag = QStringLiteral("Paragraph");
static QString FDX_TypeAttr = QStringLiteral("Type");
static QString FDX_FlagsAttr = QStringLiteral("Flags");
static QString FDX_OmittedFlag = QStringLiteral("Omitted");
static QString FDX_IgnoreFlag = QStringLiteral("Ignore");
static QString FDX_OmittedSceneTag = QStringLiteral("OmittedScene");
static QString FDX_SceneHeadingType = QStringLiteral("Scene Heading");
static QString FDX_ActionType = QStringLiteral("Action");
static QString FDX_CharacterType = QStringLiteral("Character");
static QString FDX_DialogueType = QStringLiteral("Dialogue");
static QString FDX_ParentheticalType = QStringLiteral("Parenthetical");
static QString FDX_TextTag = QStringLiteral("Text");
static QString FDX_ShotType = QStringLiteral("Shot");
static QString FDX_TransitionType = QStringLiteral("Transition");
static QString FDX_StyleAttr = QStringLiteral("Style");
static QString FDX_BoldStyle = QStringLiteral("Bold");
static QString FDX_ItalicStyle = QStringLiteral("Italic");
static QString FDX_UnderlineStyle = QStringLiteral("Underline");
static QString FDX_StrikeoutStyle = QStringLiteral("Strikeout");
static QString FDX_ColorAttr = QStringLiteral("Color");
static QString FDX_BackgroundAttr = QStringLiteral("Background");
static QString FDX_AlignmentAttr = QStringLiteral("Alignment");
static QString FDX_LeftAlignment = QStringLiteral("Left");
static QString FDX_RightAlignment = QStringLiteral("Right");
static QString FDX_CenterAlignment = QStringLiteral("Center");
static QString FDX_SceneNumberAttr = QStringLiteral("Number");
static QString FDX_ScenePropertiesTag = QStringLiteral("SceneProperties");
static QString FDX_TitleProperty = QStringLiteral("Title");
static QString FDX_ColorProperty = QStringLiteral("Color");
static QString FDX_SummaryProperty = QStringLiteral("Summary");

static void fixOmittedScenes(QDomElement &contentE);

FinalDraftImporter::FinalDraftImporter(QObject *parent) : AbstractImporter(parent) { }

FinalDraftImporter::~FinalDraftImporter() { }

bool FinalDraftImporter::canImport(const QString &fileName) const
{
    return QFileInfo(fileName).suffix().toLower() == FDX_Suffix;
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

    const QByteArray xml = device->readAll();

    QDomDocument doc;
    if (!doc.setContent(xml, true, &errMsg, &errLine, &errCol)) {
        const QString msg = QStringLiteral("Parse Error: %1 at Line %2, Column %3")
                                    .arg(errMsg)
                                    .arg(errLine)
                                    .arg(errCol);
        this->error()->setErrorMessage(msg);
        return false;
    }

    QDomElement rootE = doc.documentElement();
    if (rootE.tagName() != FDX_RootTag) {
        this->error()->setErrorMessage("Not a Final-Draft file.");
        return false;
    }

    const int fdxVersion = rootE.attribute(FDX_VersionAttr).toInt();
    if (rootE.attribute(FDX_DocumentTypeAttr) != FDX_ScriptDocumentType || fdxVersion < 1
        || fdxVersion > 6) {
        this->error()->setErrorMessage("Unrecognised Final Draft file version.");
        return false;
    }

    QDomElement contentE = rootE.firstChildElement(FDX_ContentTag);
    QDomNodeList paragraphs = contentE.elementsByTagName(FDX_ParagraphTag);
    if (paragraphs.isEmpty()) {
        this->error()->setErrorMessage(QStringLiteral("No paragraphs to import."));
        return false;
    }

    ::fixOmittedScenes(contentE);

    Scene *scene = nullptr;
    this->progress()->setProgressStep(1.0 / qreal(paragraphs.size() + 1));

    const int nrScenes = [paragraphs]() -> int {
        int ret = 0;
        for (int i = 0; i < paragraphs.size(); i++) {
            const QDomElement paragraphE = paragraphs.at(i).toElement();
            if (paragraphE.attribute(FDX_TypeAttr) == FDX_SceneHeadingType) {
                ++ret;
            }
        }
        return ret;
    }();
    this->configureCanvas(nrScenes);

    auto parseParagraphTexts =
            [](const QDomElement &paragraphE) -> QPair<QString, QVector<QTextLayout::FormatRange>> {
        QVector<QTextLayout::FormatRange> formats;
        QString text;
        if (paragraphE.isNull())
            return qMakePair(text, formats);

        QDomElement textE = paragraphE.firstChildElement(FDX_TextTag);
        while (!textE.isNull()) {
            QTextLayout::FormatRange format;
            format.start = text.length();

            const QString textEText = textE.text();
            text += textEText;

            format.length = text.length() - format.start;

            const QStringList styles = textE.attribute(FDX_StyleAttr).split(QChar('+'));
            if (styles.contains(FDX_BoldStyle))
                format.format.setFontWeight(QFont::Bold);
            if (styles.contains(FDX_ItalicStyle))
                format.format.setFontItalic(true);
            if (styles.contains(FDX_UnderlineStyle))
                format.format.setFontUnderline(true);
            if (styles.contains(FDX_StrikeoutStyle))
                format.format.setFontStrikeOut(true);

            if (textE.hasAttribute(FDX_ColorAttr))
                format.format.setForeground(
                        QBrush(fromFdxColorCode(textE.attribute(FDX_ColorAttr))));
            if (textE.hasAttribute(FDX_BackgroundAttr))
                format.format.setBackground(
                        QBrush(fromFdxColorCode(textE.attribute(FDX_BackgroundAttr))));

            if (!format.format.isEmpty())
                formats.append(format);

            textE = textE.nextSiblingElement(FDX_TextTag);
        }

        return qMakePair(text, formats);
    };

    const QStringList types({ FDX_SceneHeadingType, FDX_ActionType, FDX_CharacterType,
                              FDX_DialogueType, FDX_ParentheticalType, FDX_ShotType,
                              FDX_TransitionType });
    QDomElement paragraphE = contentE.firstChildElement(FDX_ParagraphTag);
    while (!paragraphE.isNull()) {
        TraverseDomElement tde(paragraphE, this->progress());

        const QString flags = paragraphE.attribute(FDX_FlagsAttr);
        if (flags == "Ignore")
            continue;

        const QString type = paragraphE.attribute(FDX_TypeAttr);
        const int typeIndex = types.indexOf(type);
        if (typeIndex < 0)
            continue;

        const QString alignmentHint = paragraphE.attribute(FDX_AlignmentAttr);
        const Qt::Alignment alignment = [alignmentHint]() {
            return QHash<QString, Qt::Alignment>({ { FDX_LeftAlignment, Qt::AlignLeft },
                                                   { FDX_RightAlignment, Qt::AlignRight },
                                                   { FDX_CenterAlignment, Qt::AlignCenter } })
                    .value(alignmentHint, Qt::Alignment());
        }();

        const QPair<QString, QVector<QTextLayout::FormatRange>> paragraphText =
                parseParagraphTexts(paragraphE);

        const QString text = paragraphText.first;
        const QVector<QTextLayout::FormatRange> formats = paragraphText.second;

        if (typeIndex != 0 && scene == nullptr) {
            scene = this->createScene(QString());
        }

        SceneElement *sceneElement = nullptr;
        switch (typeIndex) {
        case 0: {
            scene = this->createScene(text);

            ScreenplayElement *element = this->document()->screenplay()->elementAt(
                    this->document()->screenplay()->elementCount() - 1);
            element->setOmitted(flags == FDX_OmittedFlag);

            const QString number = paragraphE.attribute(FDX_SceneNumberAttr);
            if (!number.isEmpty())
                element->setUserSceneNumber(number);

            const QDomElement sceneProperiesE = paragraphE.firstChildElement(FDX_SceneNumberAttr);
            if (!sceneProperiesE.isNull()) {
                const QString title = sceneProperiesE.attribute(FDX_TitleProperty);
                const QColor color = fromFdxColorCode(sceneProperiesE.attribute(FDX_ColorProperty));
                scene->setColor(color);
                scene->structureElement()->setTitle(title);

                const QDomElement summaryE = sceneProperiesE.firstChildElement(FDX_SummaryProperty);
                const QDomElement summaryParagraphE = summaryE.isNull()
                        ? QDomElement()
                        : summaryE.firstChildElement(FDX_ParagraphTag);
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

    QDomElement paragraphE = contentE.firstChildElement(FDX_ParagraphTag);
    while (!paragraphE.isNull()) {

        QDomElement omittedSceneE = paragraphE.firstChildElement(FDX_OmittedSceneTag);
        if (!omittedSceneE.isNull()) {
            paragraphE.setAttribute(FDX_FlagsAttr, FDX_IgnoreFlag);
            const QDomNodeList childParagraphs = omittedSceneE.elementsByTagName(FDX_ParagraphTag);
            for (int i = childParagraphs.size() - 1; i >= 0; i--) {
                QDomElement childParagraphE = childParagraphs.at(i).toElement();
                childParagraphE.setAttribute(FDX_FlagsAttr, FDX_OmittedFlag);
                contentE.insertAfter(childParagraphE, paragraphE);
            }
        }

        paragraphE = paragraphE.nextSiblingElement(FDX_ParagraphTag);
    }
}
