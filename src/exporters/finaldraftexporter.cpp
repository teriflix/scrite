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
#include "application.h"

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

    QDomElement rootE = doc.createElement(QStringLiteral("FinalDraft"));
    rootE.setAttribute(QStringLiteral("DocumentType"), QStringLiteral("Script"));
    rootE.setAttribute(QStringLiteral("Template"), QStringLiteral("No"));
    rootE.setAttribute(QStringLiteral("Version"), QStringLiteral("2"));
    doc.appendChild(rootE);

    QDomElement contentE = doc.createElement(QStringLiteral("Content"));
    rootE.appendChild(contentE);

    auto addTextToParagraph = [&doc, this](QDomElement &paraE, const QString &text,
                                           Qt::Alignment overrideAlignment = Qt::Alignment(),
                                           const QVector<QTextLayout::FormatRange> &textFormats =
                                                   QVector<QTextLayout::FormatRange>()) {
        if (overrideAlignment != 0) {
            const QString alignmentAttr = QStringLiteral("Alignment");
            switch (overrideAlignment) {
            default:
            case Qt::AlignLeft:
                paraE.setAttribute(alignmentAttr, QStringLiteral("Left"));
                break;
            case Qt::AlignRight:
                paraE.setAttribute(alignmentAttr, QStringLiteral("Right"));
                break;
            case Qt::AlignHCenter:
                paraE.setAttribute(alignmentAttr, QStringLiteral("Center"));
                break;
            case Qt::AlignJustify:
                paraE.setAttribute(alignmentAttr, QStringLiteral("Justify"));
                break;
            }
        }

        QVector<QTextLayout::FormatRange> mergedTextFormats = textFormats;
        if (m_markLanguagesExplicitly) {
            const QList<TransliterationEngine::Boundary> breakup =
                    TransliterationEngine::instance()->evaluateBoundaries(text, true);
            mergedTextFormats = TransliterationEngine::mergeTextFormats(breakup, textFormats);
        }

        auto createTextElement = [&]() {
            QDomElement textE = doc.createElement(QStringLiteral("Text"));
            textE.setAttribute(QStringLiteral("Font"), QStringLiteral("Courier Final Draft"));
            textE.setAttribute(QStringLiteral("Language"), QStringLiteral("English"));
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
                        styles << QStringLiteral("Bold");
                }

                if (format.format.hasProperty(QTextFormat::FontItalic)) {
                    if (format.format.fontItalic())
                        styles << QStringLiteral("Italic");
                }

                if (format.format.hasProperty(QTextFormat::TextUnderlineStyle)) {
                    if (format.format.fontUnderline())
                        styles << QStringLiteral("Underline");
                }

                if (format.format.hasProperty(QTextFormat::FontStrikeOut)) {
                    if (format.format.fontStrikeOut())
                        styles << QStringLiteral("Strikeout");
                }

                if (!styles.isEmpty())
                    textE.setAttribute(QStringLiteral("Style"), styles.join('+'));

                if (format.format.hasProperty(QTextFormat::BackgroundBrush)) {
                    const QColor color = format.format.background().color();
                    textE.setAttribute(QStringLiteral("Background"), fdxColorCode(color));
                }

                if (format.format.hasProperty(QTextFormat::ForegroundBrush)) {
                    const QColor color = format.format.foreground().color();
                    textE.setAttribute(QStringLiteral("Color"), fdxColorCode(color));
                }

                if (m_markLanguagesExplicitly) {
                    TransliterationEngine::Language lang =
                            (TransliterationEngine::Language)format.format
                                    .property(QTextFormat::UserProperty)
                                    .toInt();
                    if (lang != TransliterationEngine::English) {
                        const QFont font = TransliterationEngine::instance()->languageFont(
                                lang, m_useScriteFonts);
                        textE.setAttribute(QStringLiteral("Font"), font.family());
                        textE.setAttribute(
                                QStringLiteral("Language"),
                                TransliterationEngine::instance()->languageAsString(lang));
                    }
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
            QDomElement paragraphE = doc.createElement(QStringLiteral("Paragraph"));
            contentE.appendChild(paragraphE);

            paragraphE.setAttribute(QStringLiteral("Type"), QStringLiteral("Scene Heading"));
            if (element->hasUserSceneNumber())
                paragraphE.setAttribute(QStringLiteral("Number"), element->userSceneNumber());

            QDomElement textE = doc.createElement(QStringLiteral("Text"));
            textE.appendChild(doc.createTextNode(QStringLiteral("OMITTED")));
            paragraphE.appendChild(textE);

            QDomElement omittedSceneE = doc.createElement(QStringLiteral("OmittedScene"));
            paragraphE.appendChild(omittedSceneE);

            paragraphContainerE = omittedSceneE;
        }

        const Scene *scene = element->scene();
        const StructureElement *selement = scene->structureElement();
        const SceneHeading *heading = scene->heading();

        if (heading->isEnabled() || scene->hasSynopsis()
            || (selement && selement->hasNativeTitle())) {
            QDomElement paragraphE = doc.createElement(QStringLiteral("Paragraph"));
            paragraphContainerE.appendChild(paragraphE);

            paragraphE.setAttribute(QStringLiteral("Type"), QStringLiteral("Scene Heading"));
            if (element->hasUserSceneNumber())
                paragraphE.setAttribute(QStringLiteral("Number"), element->userSceneNumber());

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
                    QDomElement scenePropsE = doc.createElement(QStringLiteral("SceneProperties"));
                    paragraphE.appendChild(scenePropsE);
                    if (selement && selement->hasNativeTitle())
                        scenePropsE.setAttribute(QStringLiteral("Title"), selement->nativeTitle());

                    const QColor sceneColor = scene->color();
                    const QColor tintColor(QStringLiteral("#E7FFFFFF"));
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

                        QDomElement summaryParagraphE =
                                doc.createElement(QStringLiteral("Paragraph"));
                        summaryE.appendChild(summaryParagraphE);

                        const QString synopsis = scene->synopsis();
                        QVector<QTextLayout::FormatRange> formats;

#if 0
                    // We don't need to apply scene color to synopsis text also.

                    QTextLayout::FormatRange format;
                    format.start = 0;
                    format.length = synopsis.length();
                    format.format.setBackground(exportSceneColor);
                    format.format.setForeground(Application::textColorFor(exportSceneColor));
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
            QDomElement paragraphE = doc.createElement(QStringLiteral("Paragraph"));
            paragraphContainerE.appendChild(paragraphE);

            paragraphE.setAttribute(QStringLiteral("Type"), sceneElement->typeAsString());
            addTextToParagraph(paragraphE, sceneElement->formattedText(), sceneElement->alignment(),
                               sceneElement->textFormats());
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

    const QString xml = doc.toString(2);

    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    ts << xml;
    ts.flush();

    return true;
}
