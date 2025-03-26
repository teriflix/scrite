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

#include "htmlexporter.h"

#include <QDir>
#include <QFileInfo>
#include <QTextBoundaryFinder>

HtmlExporter::HtmlExporter(QObject *parent) : AbstractExporter(parent) { }

HtmlExporter::~HtmlExporter() { }

void HtmlExporter::setExportWithSceneColors(bool val)
{
    if (m_exportWithSceneColors == val)
        return;

    m_exportWithSceneColors = val;
    emit exportWithSceneColorsChanged();
}

void HtmlExporter::setIncludeSceneNumbers(bool val)
{
    if (m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

static void alignmentToCssValue(QTextStream &ts, Qt::Alignment alignment)
{
    switch (alignment) {
    default:
    case Qt::AlignLeft:
        ts << "left;";
        break;
    case Qt::AlignRight:
        ts << "right;";
        break;
    case Qt::AlignHCenter:
        ts << "center;";
        break;
    case Qt::AlignJustify:
        ts << "justify;";
        break;
    }
};

bool HtmlExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const ScreenplayFormat *formatting = this->document()->printFormat();

    // Different systems have different qt_defaultDpi() values. While that works for everything we
    // do in Scrite, it doesnt work for HTML export. So, we will have to do something else.
    const qreal paperWidth =
            formatting->pageLayout()->paperSize() == ScreenplayPageLayout::A4 ? 794 : 816;
    const qreal layoutScale = paperWidth / formatting->pageLayout()->paperWidth();
    const int topMargin = int(formatting->pageLayout()->topMargin() * layoutScale);
    const qreal leftMargin = formatting->pageLayout()->leftMargin() * layoutScale;
    const qreal rightMargin = formatting->pageLayout()->rightMargin() * layoutScale;
    const int bottomMargin = int(formatting->pageLayout()->bottomMargin() * layoutScale);
    const qreal contentWidth = formatting->pageLayout()->contentWidth() * layoutScale;

    QMap<SceneElement::Type, QString> typeStringMap;
    typeStringMap[SceneElement::Heading] = "heading";
    typeStringMap[SceneElement::Action] = "action";
    typeStringMap[SceneElement::Character] = "character";
    typeStringMap[SceneElement::Dialogue] = "dialogue";
    typeStringMap[SceneElement::Parenthetical] = "parenthetical";
    typeStringMap[SceneElement::Shot] = "shot";
    typeStringMap[SceneElement::Transition] = "transition";

    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    ts << "<!DOCTYPE html>\n";
    ts << "<html>\n";
    ts << "  <head>\n";
    ts << "    <title>" << screenplay->title() << "</title>\n";
    ts << "    <meta charset=\"UTF-8\">\n";
    ts << "  </head>\n";
    ts << "  <body>\n";
    ts << "    <style>\n";

    const QMetaObject *tmo = TransliterationEngine::instance()->metaObject();
    const QMetaEnum langEnum = tmo->enumerator(tmo->indexOfEnumerator("Language"));

    QMap<TransliterationEngine::Language, bool> langBundleMap = this->languageBundleMap();
    QMap<TransliterationEngine::Language, bool>::const_iterator it = langBundleMap.constBegin();
    QMap<TransliterationEngine::Language, bool>::const_iterator end = langBundleMap.constEnd();
    const QString fontsDir = QFileInfo(this->fileName()).absolutePath() + "/fonts";

    while (it != end) {
        if (it.value()) {
            QDir().mkpath(fontsDir);

            const QStringList fontSources =
                    TransliterationEngine::instance()->languageFontFilePaths(it.key());
            for (const QString &fontSource : fontSources) {
                const QString lang = QString::fromLatin1(langEnum.valueToKey(it.key()));
                const QString fontFile = QFileInfo(fontSource).fileName();
                const QString fontDest = fontsDir + "/" + lang + "/" + fontFile;
                QDir().mkpath(QFileInfo(fontDest).absolutePath());
                QFile::copy(fontSource, fontDest);

                QRawFont rawFont(fontSource, 12);

                ts << "    @font-face {\n";
                ts << "      font-family: lang_" << it.key() << "_" << rawFont.weight() << "_"
                   << rawFont.style() << ";\n";
                ts << "      src: url(fonts/" << lang << "/" << fontFile << ");\n";
                ts << "      font-weight: " << rawFont.weight() << ";\n";
                ts << "      font-style: ";
                switch (rawFont.style()) {
                case QFont::StyleNormal:
                    ts << "normal;\n";
                    break;
                case QFont::StyleItalic:
                    ts << "italic;\n";
                    break;
                case QFont::StyleOblique:
                    ts << "oblique;\n";
                    break;
                }
                ts << "    }\n";
                ts << "    span.lang_" << it.key() << "_" << rawFont.weight() << "_"
                   << rawFont.style() << " {\n";
                ts << "      font-family: lang_" << it.key() << "_" << rawFont.weight() << "_"
                   << rawFont.style() << ";\n";
                ts << "    }\n";
            }
        }

        ++it;
    }

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        if (i > SceneElement::Min)
            ts << "\n";

        SceneElement::Type elementType = SceneElement::Type(i);
        SceneElementFormat *format = formatting->elementFormat(elementType);
        ts << "    p.scrite-" << typeStringMap.value(elementType) << " {\n";
#if 0
        ts << "      font-family: \"" << format->font().family() << "\";\n";
#else
        ts << "      font-family: \"Courier New\", Courier, monospace;\n";
#endif
        ts << "      font-size: " << format->font().pointSize() << "pt;\n";
        if (format->font().bold())
            ts << "      font-weight: bold;\n";
        if (format->font().italic())
            ts << "      font-style: italic;\n";
        ts << "      color: " << format->textColor().name() << ";\n";
        if (format->backgroundColor() != Qt::transparent)
            ts << "      background-color: " << format->backgroundColor().name() << ";\n";
        ts << "      text-align: ";
        alignmentToCssValue(ts, format->textAlignment());
        ts << ";\n";

        const int pLeftMargin = int(format->leftMargin() * contentWidth + leftMargin);
        const int pRightMargin = int(format->rightMargin() * contentWidth + rightMargin);

        ts << "      padding-left: " << pLeftMargin << "px;\n";
        ts << "      padding-right: " << pRightMargin << "px;\n";
        if (qFuzzyIsNull(format->lineSpacingBefore())
            || format->elementType() == SceneElement::Heading)
            ts << "      padding-top: 0px;\n";
        else
            ts << "      padding-top: " << format->lineSpacingBefore() << "em;\n";
        ts << "      padding-bottom: 0px;\n";
        ts << "      margin: 0px;\n";
        ts << "      line-height: " << format->lineHeight() * 1.1 << "em;\n";
        ts << "    }\n";
    }

    const SceneElementFormat *headingFormat = formatting->elementFormat(SceneElement::Heading);

    ts << "\n";
    ts << "    div.scrite-scene {\n";
    if (qFuzzyIsNull(headingFormat->lineSpacingBefore()))
        ts << "      padding-top: 0px;\n";
    else
        ts << "      padding-top: " << headingFormat->lineSpacingBefore() << "em;\n";
    ts << "      padding-bottom: 0px;\n";
    ts << "      padding-left: 0px;\n";
    ts << "      padding-right: 0px;\n";
    ts << "    }\n";

    ts << "\n";
    ts << "    div.scrite-screenplay {\n";
    ts << "        width: " << int(paperWidth) << "px;\n";
    ts << "        border: 1px solid gray;\n";
    ts << "        margin-left: auto;\n";
    ts << "        margin-right: auto;\n";
    ts << "        margin-top: " << topMargin << "px;\n";
    ts << "        margin-bottom: " << bottomMargin << "px;\n";
    ts << "    }\n";

    ts << "    </style>\n\n";

    ts << "    <div class=\"scrite-screenplay\">\n";

    auto writeParagraph = [&ts, typeStringMap,
                           langBundleMap](SceneElement::Type type, const QString &text,
                                          Qt::Alignment overrideAlignment = Qt::Alignment(),
                                          const QVector<QTextLayout::FormatRange> &textFormats =
                                                  QVector<QTextLayout::FormatRange>()) {
        const QString styleName = "scrite-" + typeStringMap.value(type);
        ts << "        <p class=\"" << styleName << "\" custom-style=\"" << styleName << "\"";

        if (overrideAlignment != 0) {
            ts << " style=\"text-align: ";
            alignmentToCssValue(ts, overrideAlignment);
            ts << ";\"";
        }

        ts << ">";

        const QList<TransliterationEngine::Boundary> breakup =
                TransliterationEngine::instance()->evaluateBoundaries(text);
        const QVector<QTextLayout::FormatRange> mergedTextFormats =
                TransliterationEngine::mergeTextFormats(breakup, textFormats);
        for (const QTextLayout::FormatRange &format : mergedTextFormats) {
            TransliterationEngine::Language lang = (TransliterationEngine::Language)format.format
                                                           .property(QTextFormat::UserProperty)
                                                           .toInt();
            ts << "<span ";
            if (langBundleMap.value(lang))
                ts << "class=\"lang_" << lang << "_" << QFont::Normal << "_" << QFont::StyleNormal
                   << "\" ";

            bool customStyle = false;
            auto startCustomStyle = [&customStyle, &ts]() {
                if (customStyle)
                    return;
                customStyle = true;
                ts << "style=\"";
            };

            if (format.format.hasProperty(QTextFormat::FontWeight)) {
                if (format.format.fontWeight() == QFont::Bold) {
                    startCustomStyle();
                    ts << "font-weight: bold; ";
                }
            }

            if (format.format.hasProperty(QTextFormat::FontItalic)) {
                if (format.format.fontItalic()) {
                    startCustomStyle();
                    ts << "font-style: italic; ";
                }
            }

            QStringList textDecorationCss;

            if (format.format.hasProperty(QTextFormat::TextUnderlineStyle)
                && format.format.fontUnderline())
                textDecorationCss << "underline";

            if (format.format.hasProperty(QTextFormat::FontStrikeOut)
                && format.format.fontStrikeOut())
                textDecorationCss << "line-through";

            if (!textDecorationCss.isEmpty()) {
                startCustomStyle();
                ts << "text-decoration: " << textDecorationCss.join(" ") << "; ";
            }

            if (format.format.hasProperty(QTextFormat::BackgroundBrush)) {
                const QColor color = format.format.background().color();
                if (!qFuzzyIsNull(color.alphaF())) {
                    startCustomStyle();
                    ts << "background-color: " << color.name() << "; ";
                }
            }

            if (format.format.hasProperty(QTextFormat::ForegroundBrush)) {
                const QColor color = format.format.foreground().color();
                if (!qFuzzyIsNull(color.alphaF())) {
                    startCustomStyle();
                    ts << "color: " << color.name() << "; ";
                }
            }

            if (customStyle)
                ts << "\"";

            ts << ">" << text.mid(format.start, format.length) << "</span>";
        }
        ts << "</p>\n";
    };

    const int nrScenes = screenplay->elementCount();
    int nrHeadings = 0;
    for (int i = 0; i < nrScenes; i++) {
        const ScreenplayElement *screenplayElement = screenplay->elementAt(i);
        if (screenplayElement->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const Scene *scene = screenplayElement->scene();

        if (m_exportWithSceneColors) {
            const QColor sceneColor = scene->color();
            const QString sceneColorText = "rgba(" + QString::number(sceneColor.red()) + ","
                    + QString::number(sceneColor.green()) + "," + QString::number(sceneColor.blue())
                    + ",0.1)";
            ts << "      <div class=\"scrite-scene\" custom-style=\"scrite-scene\" "
                  "style=\"background-color: "
               << sceneColorText << ";\">\n";
        } else
            ts << "      <div class=\"scrite-scene\" custom-style=\"scrite-scene\">\n";

        const SceneHeading *heading = scene->heading();
        const QString headingText = screenplayElement->isOmitted()
                ? QStringLiteral("OMITTED")
                : (heading->isEnabled() ? heading->text() : QStringLiteral("NO SCENE HEADING"));
        if (heading->isEnabled()) {
            ++nrHeadings;
            if (m_includeSceneNumbers)
                writeParagraph(SceneElement::Heading,
                               "[" + screenplayElement->resolvedSceneNumber() + "] " + headingText);
            else
                writeParagraph(SceneElement::Heading, headingText);
        } else {
            if (screenplayElement->isOmitted())
                writeParagraph(SceneElement::Heading, headingText);
        }

        if (screenplayElement->isOmitted())
            continue;

        const int nrElements = scene->elementCount();
        for (int j = 0; j < nrElements; j++) {
            SceneElement *element = scene->elementAt(j);
            writeParagraph(element->type(), element->formattedText(), element->alignment(),
                           element->textFormats());
        }

        if (i == nrScenes - 1)
            ts << "        <p class=\"scrite-action\" custom-style=\"scrite-action\">&nbsp;</p>";

        ts << "      </div>\n";
    }

    ts << "    </div>\n\n";

    ts << "  </body>\n";
    ts << "</html>\n";

    ts.flush();

    return true;
}
