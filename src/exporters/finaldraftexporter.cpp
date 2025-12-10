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
#include "utils.h"
#include "languageengine.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>
#include <QFileInfo>

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
static QString FDX_FontAttr = QStringLiteral("Font");
static QString FDX_LanguageAttr = QStringLiteral("Language");
static QString FDX_ScriptAttr = QStringLiteral("Script");
static QString FDX_SeparatorAttr = QStringLiteral("Separator");

FinalDraftExporter::FinalDraftExporter(QObject *parent) : AbstractExporter(parent) { }

FinalDraftExporter::~FinalDraftExporter() { }

void FinalDraftExporter::setMarkLanguagesExplicitly(bool val)
{
    if (m_markLanguagesExplicitly == val)
        return;

    m_markLanguagesExplicitly = val;
    emit markLanguagesExplicitlyChanged();
}

void FinalDraftExporter::setIncludeSceneSynopsis(bool val)
{
    if (m_includeSceneSynopsis == val)
        return;

    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();
}

static QString fdxColorCode(const QColor &color)
{
    const QChar fillChar('0');
    const QString templ = QStringLiteral("%1");
    const QString red = templ.arg(color.red(), 2, 16, fillChar).toUpper();
    const QString green = templ.arg(color.green(), 2, 16, fillChar).toUpper();
    const QString blue = templ.arg(color.blue(), 2, 16, fillChar).toUpper();
    return QStringLiteral("#") + red + red + green + green + blue + blue;
};

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

    QDomElement rootE = doc.createElement(FDX_RootTag);
    rootE.setAttribute(FDX_DocumentTypeAttr, FDX_ScriptDocumentType);
    rootE.setAttribute(QStringLiteral("Template"), QStringLiteral("No"));
    rootE.setAttribute(FDX_VersionAttr, QStringLiteral("2"));
    doc.appendChild(rootE);

    QDomElement contentE = doc.createElement(FDX_ContentTag);
    rootE.appendChild(contentE);

    auto addTextToParagraph = [&doc, this](QDomElement &paraE, const QString &text,
                                           Qt::Alignment overrideAlignment = Qt::Alignment(),
                                           const QVector<QTextLayout::FormatRange> &textFormats =
                                                   QVector<QTextLayout::FormatRange>()) {
        if (overrideAlignment != 0) {
            switch (overrideAlignment) {
            default:
            case Qt::AlignLeft:
                paraE.setAttribute(FDX_AlignmentAttr, FDX_LeftAlignment);
                break;
            case Qt::AlignRight:
                paraE.setAttribute(FDX_AlignmentAttr, FDX_RightAlignment);
                break;
            case Qt::AlignHCenter:
                paraE.setAttribute(FDX_AlignmentAttr, FDX_CenterAlignment);
                break;
            case Qt::AlignJustify:
                paraE.setAttribute(FDX_AlignmentAttr, FDX_CenterAlignment);
                break;
            }
        }

        QVector<QTextLayout::FormatRange> mergedTextFormats = textFormats;
        if (m_markLanguagesExplicitly) {
            const QList<ScriptBoundary> breakup = LanguageEngine::determineBoundaries(text);
            mergedTextFormats = LanguageEngine::mergeTextFormats(breakup, textFormats);
        }

        auto createTextElement = [&]() {
            QDomElement textE = doc.createElement(FDX_TextTag);
            textE.setAttribute(FDX_FontAttr, QStringLiteral("Courier Final Draft"));
            textE.setAttribute(FDX_LanguageAttr, QStringLiteral("English"));
            paraE.appendChild(textE);
            return textE;
        };

        if (mergedTextFormats.isEmpty()) {
            QDomElement textE = createTextElement();
            textE.appendChild(doc.createTextNode(text));
        } else {
            for (const QTextLayout::FormatRange &format : qAsConst(mergedTextFormats)) {
                const QString snippet = text.mid(format.start, format.length);
                if (snippet.isEmpty())
                    continue;

                QDomElement textE = createTextElement();

                QStringList styles;
                if (format.format.hasProperty(QTextFormat::FontWeight)) {
                    if (format.format.fontWeight() == QFont::Bold)
                        styles << FDX_BoldStyle;
                }

                if (format.format.hasProperty(QTextFormat::FontItalic)) {
                    if (format.format.fontItalic())
                        styles << FDX_ItalicStyle;
                }

                if (format.format.hasProperty(QTextFormat::TextUnderlineStyle)) {
                    if (format.format.fontUnderline())
                        styles << FDX_UnderlineStyle;
                }

                if (format.format.hasProperty(QTextFormat::FontStrikeOut)) {
                    if (format.format.fontStrikeOut())
                        styles << FDX_StrikeoutStyle;
                }

                if (!styles.isEmpty())
                    textE.setAttribute(FDX_StyleAttr, styles.join('+'));

                if (format.format.hasProperty(QTextFormat::BackgroundBrush)) {
                    const QColor color = format.format.background().color();
                    textE.setAttribute(FDX_BackgroundAttr, fdxColorCode(color));
                }

                if (format.format.hasProperty(QTextFormat::ForegroundBrush)) {
                    const QColor color = format.format.foreground().color();
                    textE.setAttribute(FDX_ColorProperty, fdxColorCode(color));
                }

                if (m_markLanguagesExplicitly) {
                    const QMetaEnum scriptEnum = QMetaEnum::fromType<QtChar::Script>();
                    QChar::Script script =
                            (QChar::Script)format.format.property(QTextFormat::UserProperty)
                                    .toInt();
                    const QString fontFamily = LanguageEngine::instance()->scriptFontFamily(script);
                    textE.setAttribute(FDX_FontAttr, fontFamily);
                    textE.setAttribute(FDX_ScriptAttr,
                                       QString::fromLatin1(scriptEnum.valueToKey(script)));
                }

                textE.appendChild(doc.createTextNode(snippet));
            }
        }
    };

    QStringList locations;
    for (int i = 0; i < nrElements; i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);
        if (element->elementType() != ScreenplayElement::SceneElementType)
            continue;

        QDomElement paragraphContainerE = contentE;
        if (element->isOmitted()) {
            QDomElement paragraphE = doc.createElement(FDX_ParagraphTag);
            contentE.appendChild(paragraphE);

            paragraphE.setAttribute(FDX_TypeAttr, FDX_SceneHeadingType);
            if (element->hasUserSceneNumber())
                paragraphE.setAttribute(FDX_SceneNumberAttr, element->userSceneNumber());

            QDomElement textE = doc.createElement(FDX_TextTag);
            textE.appendChild(doc.createTextNode(QStringLiteral("OMITTED")));
            paragraphE.appendChild(textE);

            QDomElement omittedSceneE = doc.createElement(FDX_OmittedSceneTag);
            paragraphE.appendChild(omittedSceneE);

            paragraphContainerE = omittedSceneE;
        }

        const Scene *scene = element->scene();
        const StructureElement *selement = scene->structureElement();
        const SceneHeading *heading = scene->heading();

        if (heading->isEnabled() || scene->hasSynopsis()
            || (selement && selement->hasNativeTitle())) {
            QDomElement paragraphE = doc.createElement(FDX_ParagraphTag);
            paragraphContainerE.appendChild(paragraphE);

            paragraphE.setAttribute(FDX_TypeAttr, FDX_SceneHeadingType);
            if (element->hasUserSceneNumber())
                paragraphE.setAttribute(FDX_SceneNumberAttr, element->userSceneNumber());

            if (heading->isEnabled()) {
                addTextToParagraph(paragraphE, heading->text());
                locations.append(heading->location());

                if (!locationTypes.contains(heading->locationType()))
                    locationTypes.append(heading->locationType());

                if (!moments.contains(heading->moment()))
                    moments.append(heading->moment());
            }

            if (m_includeSceneSynopsis) {
                if (scene->hasSynopsis() || (selement && selement->hasNativeTitle())) {
                    QDomElement scenePropsE = doc.createElement(FDX_ScenePropertiesTag);
                    paragraphE.appendChild(scenePropsE);
                    if (selement && selement->hasNativeTitle())
                        scenePropsE.setAttribute(FDX_TitleProperty, selement->nativeTitle());

                    const QColor sceneColor = scene->color();
                    const QColor tintColor = QColor(
                            QStringLiteral("#9CFFFFFF")); // TODO: We really need to standardize
                                                          // these hardcoded colors
                    const QColor exportSceneColor =
                            QColor::fromRgbF((sceneColor.redF() + tintColor.redF()) / 2,
                                             (sceneColor.greenF() + tintColor.greenF()) / 2,
                                             (sceneColor.blueF() + tintColor.blueF()) / 2,
                                             (sceneColor.alphaF() + tintColor.alphaF()) / 2);

                    scenePropsE.setAttribute(QStringLiteral("Color"),
                                             fdxColorCode(exportSceneColor));

                    if (scene->hasSynopsis()) {
                        QDomElement summaryE = doc.createElement(QStringLiteral("Summary"));
                        scenePropsE.appendChild(summaryE);

                        QDomElement summaryParagraphE = doc.createElement(FDX_ParagraphTag);
                        summaryE.appendChild(summaryParagraphE);

                        const QString synopsis = scene->synopsis();
                        QVector<QTextLayout::FormatRange> formats;

#if 0
                    // We don't need to apply scene color to synopsis text also.

                    QTextLayout::FormatRange format;
                    format.start = 0;
                    format.length = synopsis.length();
                    format.format.setBackground(exportSceneColor);
                    format.format.setForeground(Utils::Color::textColorFor(exportSceneColor));
                    formats.append(format);
#endif
                        addTextToParagraph(summaryParagraphE, synopsis, Qt::Alignment(), formats);
                    }
                }
            }
        }

        const int nrSceneElements = scene->elementCount();
        for (int j = 0; j < nrSceneElements; j++) {
            const SceneElement *sceneElement = scene->elementAt(j);
            QDomElement paragraphE = doc.createElement(FDX_ParagraphTag);
            paragraphContainerE.appendChild(paragraphE);

            paragraphE.setAttribute(QStringLiteral("Type"), sceneElement->typeAsString());
            addTextToParagraph(paragraphE, sceneElement->formattedText(), sceneElement->alignment(),
                               sceneElement->textFormats());
        }

        this->progress()->tick();
    }

    QDomElement watermarkingE = doc.createElement("Watermarking");
    rootE.appendChild(watermarkingE);
    watermarkingE.setAttribute(FDX_TextTag, qApp->applicationName());

    QDomElement smartTypeE = doc.createElement("SmartType");
    rootE.appendChild(smartTypeE);

    const QStringList characters = structure->allCharacterNames();
    QDomElement charactersE = doc.createElement("Characters");
    smartTypeE.appendChild(charactersE);
    for (const QString &name : qAsConst(characters)) {
        QDomElement characterE = doc.createElement("Character");
        charactersE.appendChild(characterE);
        characterE.appendChild(doc.createTextNode(name));
    }

    locations.removeDuplicates();
    std::sort(locations.begin(), locations.end());

    QDomElement timesOfDayE = doc.createElement("TimesOfDay");
    smartTypeE.appendChild(timesOfDayE);
    timesOfDayE.setAttribute(FDX_SeparatorAttr, " - ");
    std::sort(moments.begin(), moments.end());
    for (const QString &moment : qAsConst(moments)) {
        QDomElement timeOfDayE = doc.createElement("TimeOfDay");
        timesOfDayE.appendChild(timeOfDayE);
        timeOfDayE.appendChild(doc.createTextNode(moment));
    }

    std::sort(locationTypes.begin(), locationTypes.end());
    QDomElement sceneIntrosE = doc.createElement("SceneIntros");
    smartTypeE.appendChild(sceneIntrosE);
    sceneIntrosE.setAttribute(FDX_SeparatorAttr, ". ");
    for (const QString &locationType : qAsConst(locationTypes)) {
        QDomElement sceneIntroE = doc.createElement("SceneIntro");
        sceneIntrosE.appendChild(sceneIntroE);
        sceneIntroE.appendChild(doc.createTextNode(locationType));
    }

    const QString xml = doc.toString(2);

    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    ts << xml;
    ts.flush();

    return true;
}
