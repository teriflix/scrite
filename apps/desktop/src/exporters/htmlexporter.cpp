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

#include "htmlexporter.h"
#include "languageengine.h"

#include "screenplayformat.h"
#include "screenplay.h"
#include "scritedocument.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QRawFont>
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

void HtmlExporter::setBundleFonts(bool val)
{
    if (m_bundleFonts == val)
        return;

    m_bundleFonts = val;
    emit bundleFontsChanged();
}

void HtmlExporter::setIncludeTitlePage(bool val)
{
    if (m_includeTitlePage == val)
        return;

    m_includeTitlePage = val;
    emit includeTitlePageChanged();
}

void HtmlExporter::setEmbedResources(bool val)
{
    if (m_embedResources == val)
        return;

    m_embedResources = val;
    emit embedResourcesChanged();
}

void HtmlExporter::setMobileFriendly(bool val)
{
    if (m_mobileFriendly == val)
        return;

    m_mobileFriendly = val;
    emit mobileFriendlyChanged();
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

static QString fontFamilyWithFallback(const QString &family)
{
    QString ret = "\"" + family + "\"";
    if (QFontDatabase::isFixedPitch(family))
        ret += QStringLiteral(", \"Courier New\", Courier, monospace");
    return ret;
}

static QString fontMediaType(const QString &fileName)
{
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix == QStringLiteral("otf"))
        return QStringLiteral("font/otf");
    if (suffix == QStringLiteral("woff"))
        return QStringLiteral("font/woff");
    if (suffix == QStringLiteral("woff2"))
        return QStringLiteral("font/woff2");
    return QStringLiteral("font/ttf");
}

static QString imageMediaType(const QString &fileName)
{
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix == QStringLiteral("png"))
        return QStringLiteral("image/png");
    if (suffix == QStringLiteral("gif"))
        return QStringLiteral("image/gif");
    if (suffix == QStringLiteral("webp"))
        return QStringLiteral("image/webp");
    return QStringLiteral("image/jpeg");
}

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

    QTextStream ts(device);
    ts.setEncoding(QStringConverter::Utf8);
    ts.setAutoDetectUnicode(true);

    ts << "<!DOCTYPE html>\n";
    ts << "<html>\n";
    ts << "  <head>\n";
    ts << "    <title>" << screenplay->title() << "</title>\n";
    ts << "    <meta charset=\"UTF-8\">\n";
    if (m_mobileFriendly)
        ts << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    ts << "  </head>\n";
    ts << "  <body>\n";
    ts << "    <style>\n";

    const QMap<QChar::Script, QString> scriptFonts = [screenplay]() {
        QMap<QChar::Script, QString> ret;
        for (int i = 0; i < screenplay->elementCount(); i++) {
            const ScreenplayElement *element = screenplay->elementAt(i);
            const Scene *scene = element->scene();
            if (!scene)
                continue;

            const SceneHeading *heading = scene->heading();
            if (heading->isEnabled()) {
                const QList<ScriptBoundary> breakup =
                        LanguageEngine::determineBoundaries(heading->displayText());
                for (const ScriptBoundary &boundary : breakup)
                    ret[boundary.script] = boundary.fontFamily();
            }

            for (int j = 0; j < scene->elementCount(); j++) {
                const SceneElement *para = scene->elementAt(j);
                const QList<ScriptBoundary> breakup =
                        LanguageEngine::determineBoundaries(para->text());
                for (const ScriptBoundary &boundary : breakup)
                    ret[boundary.script] = boundary.fontFamily();
            }
        }

        return ret;
    }();
    const QMetaEnum scriptEnum = QMetaEnum::fromType<QtChar::Script>();
    const QMetaEnum elementTypeEnum = QMetaEnum::fromType<SceneElement::Type>();

    const QString resourcesDir = QFileInfo(this->fileName()).absolutePath() + "/resources";

    QString coverPhotoHref;
    if (m_includeTitlePage && !screenplay->coverPagePhoto().isEmpty()) {
        if (m_embedResources) {
            QFile coverPhotoFile(screenplay->coverPagePhoto());
            if (coverPhotoFile.open(QIODevice::ReadOnly))
                coverPhotoHref = "data:" + imageMediaType(screenplay->coverPagePhoto())
                        + ";base64," + QString::fromLatin1(coverPhotoFile.readAll().toBase64());
        } else {
            const QString coverPhotoFileName =
                    "cover." + QFileInfo(screenplay->coverPagePhoto()).suffix();
            QDir().mkpath(resourcesDir);
            QFile::remove(resourcesDir + "/" + coverPhotoFileName);
            if (QFile::copy(screenplay->coverPagePhoto(), resourcesDir + "/" + coverPhotoFileName))
                coverPhotoHref = "resources/" + coverPhotoFileName;
        }
    }

    if (m_bundleFonts) {
        if (!m_embedResources && !scriptFonts.isEmpty()) {
            QDir().mkpath(resourcesDir);
        }

        auto it = scriptFonts.constBegin();
        auto end = scriptFonts.constEnd();

        while (it != end) {
            const QChar::Script script = it.key();
            const QStringList bundledFontFiles =
                    LanguageEngine::instance()->bundledFontFilesForScript(script);

            for (const QString &fontFile : bundledFontFiles) {
                QFile srcFile(fontFile);
                if (srcFile.open(QIODevice::ReadOnly)) {
                    const QString fileName = QFileInfo(fontFile).fileName();
                    const QByteArray fontData = srcFile.readAll();
                    srcFile.close();

                    QString fontHref;
                    QRawFont rawFont;
                    if (m_embedResources) {
                        fontHref = "data:" + fontMediaType(fileName) + ";base64,"
                                + QString::fromLatin1(fontData.toBase64());
                        rawFont = QRawFont(fontData, 12);
                    } else {
                        const QString fontDest = resourcesDir + "/" + fileName;
                        QFile destFile(fontDest);
                        if (destFile.open(QIODevice::WriteOnly)) {
                            destFile.write(fontData);
                            destFile.close();
                        }
                        fontHref = "resources/" + fileName;
                        rawFont = QRawFont(fontDest, 12);
                    }

                    ts << "    @font-face {\n";
                    ts << "      font-family: \"" << it.value() << "\";\n";
                    ts << "      src: url(" << fontHref << ");\n";

                    if (rawFont.isValid()) {
                        const int weight = rawFont.weight();
                        const QFont::Style style = rawFont.style();

                        ts << "      font-weight: ";
                        switch (weight) {
                        case int(QFont::Light):
                            ts << "lighter";
                            break;
                        case int(QFont::Normal):
                            ts << "normal";
                            break;
                        case int(QFont::Bold):
                            ts << "bold";
                            break;
                        case int(QFont::ExtraBold):
                            ts << "bolder";
                            break;
                        default:
                            ts << weight;
                            break;
                        }
                        ts << ";\n";

                        ts << "      font-style: ";
                        switch (style) {
                        case QFont::StyleNormal:
                            ts << "normal";
                            break;
                        case QFont::StyleItalic:
                            ts << "italic";
                            break;
                        case QFont::StyleOblique:
                            ts << "oblique";
                            break;
                        }
                        ts << ";\n";
                    }

                    ts << "    }\n";
                }
            }

            ++it;
        }
    }

    auto it = scriptFonts.constBegin();
    auto end = scriptFonts.constEnd();

    while (it != end) {
        ts << "    span." << QString::fromLatin1(scriptEnum.valueToKey(it.key())).toLower()
           << " {\n";
        ts << "      font-family: " << fontFamilyWithFallback(it.value()) << ";\n";
        ts << "    }\n";
        ++it;
    }

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        if (i > SceneElement::Min)
            ts << "\n";

        SceneElement::Type elementType = SceneElement::Type(i);
        SceneElementFormat *format = formatting->elementFormat(elementType);
        ts << "    p.scrite-" << elementTypeEnum.valueToKey(elementType) << "-paragraph {\n";
        ts << "      font-family: " << fontFamilyWithFallback(format->font().family()) << ";\n";
        if (m_mobileFriendly && QFontDatabase::isFixedPitch(format->font().family()))
            ts << "      -webkit-text-stroke: 0.3px;\n";
        if (m_mobileFriendly)
            ts << "      font-size: calc(" << format->font().pointSize()
               << "pt * var(--scrite-font-scale, 1));\n";
        else
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

        if (m_mobileFriendly) {
            ts << "      padding-left: " << format->leftMargin() * 100 << "%;\n";
            ts << "      padding-right: " << format->rightMargin() * 100 << "%;\n";
        } else {
            ts << "      padding-left: " << pLeftMargin << "px;\n";
            ts << "      padding-right: " << pRightMargin << "px;\n";
        }
        if (qFuzzyIsNull(format->lineSpacingBefore())
            || format->elementType() == SceneElement::Heading)
            ts << "      padding-top: 0px;\n";
        else
            ts << "      padding-top: " << format->lineSpacingBefore() << "em;\n";
        ts << "      padding-bottom: 0px;\n";
        ts << "      margin: 0px;\n";
        if (m_mobileFriendly)
            ts << "      line-height: 1.35em;\n";
        else
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
    if (m_mobileFriendly) {
        ts << "        max-width: " << int(paperWidth) << "px;\n";
        ts << "        margin: 0 auto;\n";
        ts << "        padding: 1em 0.5em;\n";
    } else {
        ts << "        width: " << int(paperWidth) << "px;\n";
        ts << "        border: 1px solid gray;\n";
        ts << "        margin-left: auto;\n";
        ts << "        margin-right: auto;\n";
        ts << "        margin-top: " << topMargin << "px;\n";
        ts << "        margin-bottom: " << bottomMargin << "px;\n";
    }
    ts << "    }\n";

    ts << "\n    /* Dual dialogue formatting */\n";

    const SceneElementFormat *actionFormat = formatting->elementFormat(SceneElement::Action);
    const int tableLeftMargin = int(actionFormat->leftMargin() * contentWidth + leftMargin);
    const int tableRightMargin = int(actionFormat->rightMargin() * contentWidth + rightMargin);

    ts << "    table.scrite-dual-dialogue {\n";
    ts << "      border: 0;\n";
    ts << "      border-collapse: collapse;\n";
    if (m_mobileFriendly) {
        ts << "      margin-left: " << actionFormat->leftMargin() * 100 << "%;\n";
        ts << "      margin-right: " << actionFormat->rightMargin() * 100 << "%;\n";
    } else {
        ts << "      margin-left: " << tableLeftMargin << "px;\n";
        ts << "      margin-right: " << tableRightMargin << "px;\n";
    }
    ts << "      margin-top: 1em;\n";
    ts << "      margin-bottom: 0px;\n";
    ts << "      padding: 0px;\n";
    ts << "    }\n";
    ts << "    table.scrite-dual-dialogue td {\n";
    ts << "      border: 0;\n";
    ts << "      padding: 0;\n";
    ts << "      width: 50%;\n";
    ts << "      vertical-align: top;\n";
    ts << "    }\n";

    ts << "    table.scrite-dual-dialogue td p.scrite-Character-paragraph {\n";
    ts << "      text-align: center !important;\n";
    ts << "      padding-top: 0px !important;\n";
    ts << "      padding-left: 0 !important;\n";
    ts << "      padding-right: 0 !important;\n";
    ts << "    }\n";

    ts << "    table.scrite-dual-dialogue td p.scrite-Dialogue-paragraph {\n";
    ts << "      padding-left: 1em !important;\n";
    ts << "      padding-right: 1em !important;\n";
    ts << "    }\n";

    ts << "    table.scrite-dual-dialogue td p.scrite-Parenthetical-paragraph {\n";
    ts << "      padding-left: 2em !important;\n";
    ts << "      padding-right: 2em !important;\n";
    ts << "      text-align: center !important;\n";
    ts << "    }\n";

    ts << "\n    /* Title page formatting */\n";
    ts << "    div.scrite-title-page {\n";
    ts << "      font-family: " << fontFamilyWithFallback(actionFormat->font().family()) << ";\n";
    if (m_mobileFriendly && QFontDatabase::isFixedPitch(actionFormat->font().family()))
        ts << "      -webkit-text-stroke: 0.3px;\n";
    if (m_mobileFriendly)
        ts << "      font-size: calc(" << actionFormat->font().pointSize()
           << "pt * var(--scrite-font-scale, 1));\n";
    else
        ts << "      font-size: " << actionFormat->font().pointSize() << "pt;\n";
    if (m_mobileFriendly)
        ts << "      line-height: 1.35em;\n";
    ts << "      text-align: center;\n";
    ts << "      padding: 2em 0;\n";
    ts << "      page-break-after: always;\n";
    ts << "    }\n";
    ts << "    div.scrite-title-page img {\n";
    ts << "      max-width: 100%;\n";
    ts << "      height: auto;\n";
    ts << "    }\n";
    ts << "    div.scrite-title-page h1 {\n";
    ts << "      font-size: 1.5em;\n";
    ts << "    }\n";
    ts << "    div.scrite-title-page p {\n";
    ts << "      margin: 1.5em 0;\n";
    ts << "    }\n";

    if (m_mobileFriendly) {
        ts << "\n    /* Light and dark mode */\n";
        ts << "    :root {\n";
        ts << "      color-scheme: light dark;\n";
        ts << "    }\n";
        ts << "    @media (prefers-color-scheme: dark) {\n";
        ts << "      body {\n";
        ts << "        background-color: #121212;\n";
        ts << "        color: #e8e8e8;\n";
        ts << "      }\n";
        ts << "      p[class^=\"scrite-\"][class$=\"-paragraph\"] {\n";
        ts << "        color: #e8e8e8;\n";
        ts << "        background-color: transparent;\n";
        ts << "      }\n";
        ts << "    }\n";

        ts << "\n    /* Font size controls */\n";
        ts << "    div.scrite-font-controls {\n";
        ts << "      position: fixed;\n";
        ts << "      top: 0.5em;\n";
        ts << "      right: 0.5em;\n";
        ts << "      z-index: 10;\n";
        ts << "    }\n";
        ts << "    div.scrite-font-controls button {\n";
        ts << "      font-size: 16px;\n";
        ts << "      min-width: 2.5em;\n";
        ts << "      min-height: 2.5em;\n";
        ts << "      margin-left: 0.25em;\n";
        ts << "    }\n";
        ts << "    @media print {\n";
        ts << "      div.scrite-font-controls {\n";
        ts << "        display: none;\n";
        ts << "      }\n";
        ts << "    }\n";
    }

    ts << "    </style>\n\n";

    if (m_mobileFriendly) {
        ts << "    <div class=\"scrite-font-controls\">\n";
        ts << "      <button type=\"button\" aria-label=\"Decrease font size\" "
              "onclick=\"scriteChangeFontScale(-0.1)\">A-</button>\n";
        ts << "      <button type=\"button\" aria-label=\"Increase font size\" "
              "onclick=\"scriteChangeFontScale(0.1)\">A+</button>\n";
        ts << "    </div>\n";
        ts << "    <script>\n";
        ts << "      (function () {\n";
        ts << "        var scale = 1;\n";
        ts << "        try {\n";
        ts << "          scale = parseFloat(window.localStorage.getItem(\"scrite-font-scale\")) "
              "|| 1;\n";
        ts << "        } catch (e) { }\n";
        ts << "        scale = Math.min(2, Math.max(0.7, scale));\n";
        ts << "        document.documentElement.style.setProperty(\"--scrite-font-scale\", "
              "scale);\n";
        ts << "        window.scriteChangeFontScale = function (delta) {\n";
        ts << "          scale = Math.min(2, Math.max(0.7, Math.round((scale + delta) * 10) / "
              "10));\n";
        ts << "          document.documentElement.style.setProperty(\"--scrite-font-scale\", "
              "scale);\n";
        ts << "          try {\n";
        ts << "            window.localStorage.setItem(\"scrite-font-scale\", scale);\n";
        ts << "          } catch (e) { }\n";
        ts << "        };\n";
        ts << "      })();\n";
        ts << "    </script>\n\n";
    }

    ts << "    <div class=\"scrite-screenplay\">\n";

    if (m_includeTitlePage) {
        const QString title = screenplay->title().trimmed().isEmpty()
                ? QStringLiteral("Untitled Screenplay")
                : screenplay->title().trimmed();

        ts << "      <div class=\"scrite-title-page\">\n";

        if (!coverPhotoHref.isEmpty()) {
            int coverPhotoWidth = 100;
            switch (screenplay->coverPagePhotoSize()) {
            case Screenplay::LargeCoverPhoto:
                break;
            case Screenplay::MediumCoverPhoto:
                coverPhotoWidth = 50;
                break;
            case Screenplay::SmallCoverPhoto:
                coverPhotoWidth = 25;
                break;
            }
            ts << "        <p><img src=\"" << coverPhotoHref.toHtmlEscaped() << "\" style=\"width: "
               << coverPhotoWidth << "%;\"></p>\n";
        }

        ts << "        <h1>" << title.toHtmlEscaped() << "</h1>\n";
        if (!screenplay->subtitle().trimmed().isEmpty())
            ts << "        <p>" << screenplay->subtitle().trimmed().toHtmlEscaped() << "</p>\n";
        if (!screenplay->author().trimmed().isEmpty())
            ts << "        <p>Written By<br>" << screenplay->author().trimmed().toHtmlEscaped()
               << "</p>\n";
        if (!screenplay->basedOn().trimmed().isEmpty())
            ts << "        <p>" << screenplay->basedOn().trimmed().toHtmlEscaped() << "</p>\n";
        if (!screenplay->version().trimmed().isEmpty())
            ts << "        <p>" << screenplay->version().trimmed().toHtmlEscaped() << "</p>\n";

        QStringList contactFields({ screenplay->contact().trimmed(),
                                    screenplay->address().trimmed(),
                                    screenplay->phoneNumber().trimmed(),
                                    screenplay->email().trimmed(),
                                    screenplay->website().trimmed() });
        contactFields.removeAll(QString());
        if (!contactFields.isEmpty()) {
            for (QString &contactField : contactFields)
                contactField = contactField.toHtmlEscaped();
            ts << "        <p>Contact:<br>" << contactFields.join("<br>") << "</p>\n";
        }

        ts << "      </div>\n";
    }

    auto writeParagraph = [&ts, elementTypeEnum, scriptEnum,
                           scriptFonts](SceneElement::Type type, const QString &text,
                                        Qt::Alignment overrideAlignment = Qt::Alignment(),
                                        const QVector<QTextLayout::FormatRange> &textFormats =
                                                QVector<QTextLayout::FormatRange>()) {
        const QString styleName =
                "scrite-" + QString::fromLatin1(elementTypeEnum.valueToKey(type)) + "-paragraph";
        ts << "        <p class=\"" << styleName << "\" custom-style=\"" << styleName << "\"";

        if (overrideAlignment != 0) {
            ts << " style=\"text-align: ";
            alignmentToCssValue(ts, overrideAlignment);
            ts << ";\"";
        }

        ts << ">";

        const QList<ScriptBoundary> breakup = LanguageEngine::determineBoundaries(text);
        const QVector<QTextLayout::FormatRange> mergedTextFormats =
                LanguageEngine::mergeTextFormats(breakup, textFormats);
        for (const QTextLayout::FormatRange &format : mergedTextFormats) {
            QChar::Script script =
                    (QChar::Script)format.format.property(QTextFormat::UserProperty).toInt();
            ts << "<span ";
            if (scriptFonts.contains(script))
                ts << "class=\"" << QString::fromLatin1(scriptEnum.valueToKey(script)).toLower()
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

            ts << ">";

            if (format.format.isAnchor() && !format.format.anchorHref().isEmpty())
                ts << "<a href=\"" << format.format.anchorHref() << "\">";

            ts << text.mid(format.start, format.length);

            if (format.format.isAnchor() && !format.format.anchorHref().isEmpty())
                ts << "</a>";

            ts << "</span>";
        }
        ts << "</p>\n";
    };

    const int nrScenes = screenplay->elementCount();
    for (int i = 0; i < nrScenes; i++) {
        const ScreenplayElement *screenplayElement = screenplay->elementAt(i);
        if (screenplayElement->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const Scene *scene = screenplayElement->scene();

        if (m_exportWithSceneColors) {
            const QColor sceneColor = scene->color();
            const QString sceneColorText = "rgba(" + QString::number(sceneColor.red()) + ","
                    + QString::number(sceneColor.green()) + "," + QString::number(sceneColor.blue())
                    + (m_mobileFriendly ? ",0.6)" : ",0.1)");
            if (m_mobileFriendly)
                ts << "      <div class=\"scrite-scene\" custom-style=\"scrite-scene\" "
                      "style=\"border-left: 0.4em solid "
                   << sceneColorText << "; padding-left: 0.5em;\">\n";
            else
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
            SceneDualDialogue *dualDialogue = scene->findContainingDualDialogue(element);

            if (dualDialogue) {
                const QList<SceneElement *> &leftElements = dualDialogue->leftElements();
                if (!leftElements.isEmpty() && leftElements.first() == element) {
                    ts << "        <table class=\"scrite-dual-dialogue\">\n";
                    ts << "          <tr>\n";
                    ts << "            <td>\n";

                    for (SceneElement *leftElement : leftElements) {
                        writeParagraph(leftElement->type(), leftElement->formattedText(),
                                       leftElement->alignment(), leftElement->textFormats());
                    }

                    ts << "            </td>\n";
                    ts << "            <td>\n";

                    const QList<SceneElement *> &rightElements = dualDialogue->rightElements();
                    for (SceneElement *rightElement : rightElements) {
                        writeParagraph(rightElement->type(), rightElement->formattedText(),
                                       rightElement->alignment(), rightElement->textFormats());
                    }

                    ts << "            </td>\n";
                    ts << "          </tr>\n";
                    ts << "        </table>\n";

                    j += leftElements.size() + rightElements.size();
                }
            } else {
                writeParagraph(element->type(), element->formattedText(), element->alignment(),
                               element->textFormats());
            }
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
