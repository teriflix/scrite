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

#include "epubexporter.h"
#include "languageengine.h"

#include "screenplayformat.h"
#include "screenplay.h"
#include "scritedocument.h"

#include "quazip.h"
#include "quazipfile.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QRawFont>
#include <QUuid>

EpubExporter::EpubExporter(QObject *parent) : AbstractExporter(parent) { }

EpubExporter::~EpubExporter() { }

void EpubExporter::setExportWithSceneColors(bool val)
{
    if (m_exportWithSceneColors == val)
        return;

    m_exportWithSceneColors = val;
    emit exportWithSceneColorsChanged();
}

void EpubExporter::setIncludeSceneNumbers(bool val)
{
    if (m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void EpubExporter::setIncludeTitlePage(bool val)
{
    if (m_includeTitlePage == val)
        return;

    m_includeTitlePage = val;
    emit includeTitlePageChanged();
}

static QString alignmentToCssValue(Qt::Alignment alignment)
{
    switch (alignment) {
    default:
    case Qt::AlignLeft:
        return QStringLiteral("left");
    case Qt::AlignRight:
        return QStringLiteral("right");
    case Qt::AlignHCenter:
        return QStringLiteral("center");
    case Qt::AlignJustify:
        return QStringLiteral("justify");
    }
}

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

static bool addFileToZip(QuaZip &zip, const QString &path, const QByteArray &data)
{
    QuaZipFile file(&zip);
    if (!file.open(QIODevice::WriteOnly, QuaZipNewInfo(path)))
        return false;

    file.write(data);
    file.close();
    return true;
}

bool EpubExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const ScreenplayFormat *formatting = this->document()->printFormat();

    const QMetaEnum scriptEnum = QMetaEnum::fromType<QtChar::Script>();
    const QMetaEnum elementTypeEnum = QMetaEnum::fromType<SceneElement::Type>();

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

    // Style sheet and fonts
    QMap<QString, QByteArray> fonts; // file name -> file contents

    QString css;
    QTextStream cs(&css);

    auto scriptIt = scriptFonts.constBegin();
    auto scriptEnd = scriptFonts.constEnd();
    while (scriptIt != scriptEnd) {
        const QStringList bundledFontFiles =
                LanguageEngine::instance()->bundledFontFilesForScript(scriptIt.key());

        for (const QString &fontFile : bundledFontFiles) {
            const QString fileName = QFileInfo(fontFile).fileName();
            if (fonts.contains(fileName))
                continue;

            QFile srcFile(fontFile);
            if (!srcFile.open(QIODevice::ReadOnly))
                continue;

            const QByteArray fontData = srcFile.readAll();
            srcFile.close();

            fonts[fileName] = fontData;

            cs << "@font-face {\n";
            cs << "  font-family: \"" << scriptIt.value() << "\";\n";
            cs << "  src: url(fonts/" << fileName << ");\n";

            const QRawFont rawFont(fontData, 12);
            if (rawFont.isValid()) {
                cs << "  font-weight: " << rawFont.weight() << ";\n";
                cs << "  font-style: "
                   << (rawFont.style() == QFont::StyleNormal ? "normal" : "italic") << ";\n";
            }

            cs << "}\n";
        }

        cs << "span." << QString::fromLatin1(scriptEnum.valueToKey(scriptIt.key())).toLower()
           << " {\n";
        cs << "  font-family: " << fontFamilyWithFallback(scriptIt.value()) << ";\n";
        cs << "}\n";

        ++scriptIt;
    }

    cs << "body {\n";
    cs << "  margin: 0;\n";
    cs << "  padding: 0 0.5em;\n";
    cs << "}\n";

    cs << "div.scrite-title-page {\n";
    cs << "  text-align: center;\n";
    cs << "  padding: 2em 0;\n";
    cs << "}\n";

    cs << "div.scrite-title-page img {\n";
    cs << "  max-width: 100%;\n";
    cs << "  height: auto;\n";
    cs << "}\n";

    cs << "div.scrite-title-page p {\n";
    cs << "  margin: 1.5em 0;\n";
    cs << "}\n";

    cs << "div.scrite-scene {\n";
    cs << "  margin-top: 1em;\n";
    cs << "}\n";

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        const SceneElement::Type elementType = SceneElement::Type(i);
        const SceneElementFormat *format = formatting->elementFormat(elementType);

        cs << "p.scrite-" << elementTypeEnum.valueToKey(elementType) << "-paragraph {\n";
        cs << "  font-family: " << fontFamilyWithFallback(format->font().family()) << ";\n";
        if (format->font().bold())
            cs << "  font-weight: bold;\n";
        if (format->font().italic())
            cs << "  font-style: italic;\n";
        cs << "  text-align: " << alignmentToCssValue(format->textAlignment()) << ";\n";
        cs << "  padding-left: " << format->leftMargin() * 100 << "%;\n";
        cs << "  padding-right: " << format->rightMargin() * 100 << "%;\n";
        if (qFuzzyIsNull(format->lineSpacingBefore())
            || format->elementType() == SceneElement::Heading)
            cs << "  padding-top: 0;\n";
        else
            cs << "  padding-top: " << format->lineSpacingBefore() << "em;\n";
        cs << "  padding-bottom: 0;\n";
        cs << "  margin: 0;\n";
        cs << "}\n";
    }

    cs.flush();

    // Screenplay content
    QString content;
    QTextStream ts(&content);

    QStringList sceneIds;
    QStringList sceneTitles;

    auto writeParagraph = [&ts, elementTypeEnum, scriptEnum,
                           scriptFonts](SceneElement::Type type, const QString &text,
                                        Qt::Alignment overrideAlignment = Qt::Alignment(),
                                        const QVector<QTextLayout::FormatRange> &textFormats =
                                                QVector<QTextLayout::FormatRange>()) {
        ts << "      <p class=\"scrite-" << QString::fromLatin1(elementTypeEnum.valueToKey(type))
           << "-paragraph\"";

        if (overrideAlignment != 0)
            ts << " style=\"text-align: " << alignmentToCssValue(overrideAlignment) << ";\"";

        ts << ">";

        const QList<ScriptBoundary> breakup = LanguageEngine::determineBoundaries(text);
        const QVector<QTextLayout::FormatRange> mergedTextFormats =
                LanguageEngine::mergeTextFormats(breakup, textFormats);
        for (const QTextLayout::FormatRange &format : mergedTextFormats) {
            const QChar::Script script =
                    (QChar::Script)format.format.property(QTextFormat::UserProperty).toInt();

            QStringList styles;
            if (format.format.hasProperty(QTextFormat::FontWeight)
                && format.format.fontWeight() == QFont::Bold)
                styles << QStringLiteral("font-weight: bold;");

            if (format.format.hasProperty(QTextFormat::FontItalic) && format.format.fontItalic())
                styles << QStringLiteral("font-style: italic;");

            QStringList textDecorations;
            if (format.format.hasProperty(QTextFormat::TextUnderlineStyle)
                && format.format.fontUnderline())
                textDecorations << QStringLiteral("underline");
            if (format.format.hasProperty(QTextFormat::FontStrikeOut)
                && format.format.fontStrikeOut())
                textDecorations << QStringLiteral("line-through");
            if (!textDecorations.isEmpty())
                styles << QStringLiteral("text-decoration: ") + textDecorations.join(" ") + ";";

            ts << "<span";
            if (scriptFonts.contains(script))
                ts << " class=\"" << QString::fromLatin1(scriptEnum.valueToKey(script)).toLower()
                   << "\"";
            if (!styles.isEmpty())
                ts << " style=\"" << styles.join(" ") << "\"";
            ts << ">";

            const bool isAnchor =
                    format.format.isAnchor() && !format.format.anchorHref().isEmpty();
            if (isAnchor)
                ts << "<a href=\"" << format.format.anchorHref().toHtmlEscaped() << "\">";

            ts << text.mid(format.start, format.length).toHtmlEscaped();

            if (isAnchor)
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
        const SceneHeading *heading = scene->heading();
        const QString headingText = screenplayElement->isOmitted()
                ? QStringLiteral("OMITTED")
                : (heading->isEnabled() ? heading->text() : QStringLiteral("NO SCENE HEADING"));
        const QString numberedHeadingText = m_includeSceneNumbers
                ? "[" + screenplayElement->resolvedSceneNumber() + "] " + headingText
                : headingText;

        const QString sceneId = QStringLiteral("scene-") + QString::number(i);
        sceneIds << sceneId;
        sceneTitles << numberedHeadingText;

        ts << "    <div class=\"scrite-scene\" id=\"" << sceneId << "\"";
        if (m_exportWithSceneColors) {
            const QColor sceneColor = scene->color();
            ts << " style=\"border-left: 0.4em solid rgba(" << sceneColor.red() << ","
               << sceneColor.green() << "," << sceneColor.blue() << ",0.6); padding-left: 0.5em;\"";
        }
        ts << ">\n";

        if (heading->isEnabled())
            writeParagraph(SceneElement::Heading, numberedHeadingText);
        else if (screenplayElement->isOmitted())
            writeParagraph(SceneElement::Heading, headingText);

        if (!screenplayElement->isOmitted()) {
            const int nrElements = scene->elementCount();
            for (int j = 0; j < nrElements; j++) {
                const SceneElement *element = scene->elementAt(j);
                writeParagraph(element->type(), element->formattedText(), element->alignment(),
                               element->textFormats());
            }
        }

        ts << "    </div>\n";
    }

    ts.flush();

    const QString xhtmlHeader =
            QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<!DOCTYPE html>\n"
                           "<html xmlns=\"http://www.w3.org/1999/xhtml\" "
                           "xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"en\">\n");

    const QString title = screenplay->title().trimmed().isEmpty()
            ? QStringLiteral("Untitled Screenplay")
            : screenplay->title().trimmed().toHtmlEscaped();

    const QString screenplayXhtml = xhtmlHeader + QStringLiteral("  <head>\n")
            + QStringLiteral("    <meta charset=\"UTF-8\"/>\n") + QStringLiteral("    <title>")
            + title + QStringLiteral("</title>\n")
            + QStringLiteral("    <link rel=\"stylesheet\" type=\"text/css\" "
                             "href=\"style.css\"/>\n")
            + QStringLiteral("  </head>\n  <body>\n") + content
            + QStringLiteral("  </body>\n</html>\n");

    // Title page, with the cover photo if there is one
    const QString coverPhotoPath = m_includeTitlePage ? screenplay->coverPagePhoto() : QString();
    QByteArray coverPhotoData;
    if (!coverPhotoPath.isEmpty()) {
        QFile coverPhotoFile(coverPhotoPath);
        if (coverPhotoFile.open(QIODevice::ReadOnly))
            coverPhotoData = coverPhotoFile.readAll();
    }
    const QString coverPhotoFileName =
            coverPhotoData.isEmpty() ? QString() : "cover." + QFileInfo(coverPhotoPath).suffix();

    QString titlePageXhtml;
    if (m_includeTitlePage) {
        QString titlePage;
        QTextStream tps(&titlePage);

        tps << "    <div class=\"scrite-title-page\">\n";

        if (!coverPhotoFileName.isEmpty()) {
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
            tps << "      <p><img src=\"images/" << coverPhotoFileName
                << "\" alt=\"Cover\" style=\"width: " << coverPhotoWidth << "%;\"/></p>\n";
        }

        tps << "      <h1>" << title << "</h1>\n";
        if (!screenplay->subtitle().trimmed().isEmpty())
            tps << "      <p>" << screenplay->subtitle().trimmed().toHtmlEscaped() << "</p>\n";
        if (!screenplay->author().trimmed().isEmpty())
            tps << "      <p>Written By<br/>" << screenplay->author().trimmed().toHtmlEscaped()
                << "</p>\n";
        if (!screenplay->basedOn().trimmed().isEmpty())
            tps << "      <p>" << screenplay->basedOn().trimmed().toHtmlEscaped() << "</p>\n";
        if (!screenplay->version().trimmed().isEmpty())
            tps << "      <p>" << screenplay->version().trimmed().toHtmlEscaped() << "</p>\n";

        QStringList contactFields({ screenplay->contact().trimmed(),
                                    screenplay->address().trimmed(),
                                    screenplay->phoneNumber().trimmed(),
                                    screenplay->email().trimmed(),
                                    screenplay->website().trimmed() });
        contactFields.removeAll(QString());
        if (!contactFields.isEmpty()) {
            for (QString &contactField : contactFields)
                contactField = contactField.toHtmlEscaped();
            tps << "      <p>Contact:<br/>" << contactFields.join("<br/>") << "</p>\n";
        }

        tps << "    </div>\n";
        tps.flush();

        titlePageXhtml = xhtmlHeader + QStringLiteral("  <head>\n")
                + QStringLiteral("    <meta charset=\"UTF-8\"/>\n") + QStringLiteral("    <title>")
                + title + QStringLiteral("</title>\n")
                + QStringLiteral("    <link rel=\"stylesheet\" type=\"text/css\" "
                                 "href=\"style.css\"/>\n")
                + QStringLiteral("  </head>\n  <body>\n") + titlePage
                + QStringLiteral("  </body>\n</html>\n");
    }

    // Navigation document, with one entry per scene
    QString navigation;
    QTextStream ns(&navigation);
    ns << xhtmlHeader;
    ns << "  <head>\n";
    ns << "    <meta charset=\"UTF-8\"/>\n";
    ns << "    <title>" << title << "</title>\n";
    ns << "  </head>\n";
    ns << "  <body>\n";
    ns << "    <nav epub:type=\"toc\" id=\"toc\">\n";
    ns << "      <ol>\n";
    ns << "        <li><a href=\""
       << (m_includeTitlePage ? "title-page.xhtml" : "screenplay.xhtml") << "\">" << title
       << "</a></li>\n";
    for (int i = 0; i < sceneIds.size(); i++)
        ns << "        <li><a href=\"screenplay.xhtml#" << sceneIds.at(i) << "\">"
           << sceneTitles.at(i).toHtmlEscaped() << "</a></li>\n";
    ns << "      </ol>\n";
    ns << "    </nav>\n";
    ns << "  </body>\n";
    ns << "</html>\n";
    ns.flush();

    // Package document
    QString package;
    QTextStream ps(&package);
    ps << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ps << "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"3.0\" "
          "unique-identifier=\"book-id\">\n";
    ps << "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n";
    ps << "    <dc:identifier id=\"book-id\">urn:uuid:"
       << QUuid::createUuid().toString(QUuid::WithoutBraces) << "</dc:identifier>\n";
    ps << "    <dc:title>" << title << "</dc:title>\n";
    if (!screenplay->author().isEmpty())
        ps << "    <dc:creator>" << screenplay->author().toHtmlEscaped() << "</dc:creator>\n";
    ps << "    <dc:language>en</dc:language>\n";
    if (!coverPhotoFileName.isEmpty())
        ps << "    <meta name=\"cover\" content=\"cover-image\"/>\n";
    ps << "    <meta property=\"dcterms:modified\">"
       << QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ"))
       << "</meta>\n";
    ps << "  </metadata>\n";
    ps << "  <manifest>\n";
    ps << "    <item id=\"nav\" href=\"nav.xhtml\" media-type=\"application/xhtml+xml\" "
          "properties=\"nav\"/>\n";
    ps << "    <item id=\"screenplay\" href=\"screenplay.xhtml\" "
          "media-type=\"application/xhtml+xml\"/>\n";
    ps << "    <item id=\"style\" href=\"style.css\" media-type=\"text/css\"/>\n";
    if (m_includeTitlePage)
        ps << "    <item id=\"title-page\" href=\"title-page.xhtml\" "
              "media-type=\"application/xhtml+xml\"/>\n";
    if (!coverPhotoFileName.isEmpty())
        ps << "    <item id=\"cover-image\" href=\"images/" << coverPhotoFileName
           << "\" media-type=\"" << imageMediaType(coverPhotoFileName)
           << "\" properties=\"cover-image\"/>\n";
    int fontCounter = 0;
    for (auto it = fonts.constBegin(); it != fonts.constEnd(); ++it)
        ps << "    <item id=\"font-" << fontCounter++ << "\" href=\"fonts/" << it.key()
           << "\" media-type=\"" << fontMediaType(it.key()) << "\"/>\n";
    ps << "  </manifest>\n";
    ps << "  <spine>\n";
    if (m_includeTitlePage)
        ps << "    <itemref idref=\"title-page\"/>\n";
    ps << "    <itemref idref=\"screenplay\"/>\n";
    ps << "  </spine>\n";
    ps << "</package>\n";
    ps.flush();

    const QString containerXml =
            QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<container version=\"1.0\" "
                           "xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
                           "  <rootfiles>\n"
                           "    <rootfile full-path=\"OEBPS/content.opf\" "
                           "media-type=\"application/oebps-package+xml\"/>\n"
                           "  </rootfiles>\n"
                           "</container>\n");

    // Pack everything into the EPUB archive
    QuaZip zip(device);
    zip.setFileNameCodec("UTF-8");
    if (!zip.open(QuaZip::mdCreate)) {
        this->error()->setErrorMessage(QStringLiteral("Could not create the EPUB archive."));
        return false;
    }

    // The mimetype file must be the first entry in the archive and must not be compressed.
    QuaZipFile mimetypeFile(&zip);
    if (!mimetypeFile.open(QIODevice::WriteOnly, QuaZipNewInfo(QStringLiteral("mimetype")),
                           nullptr, 0, 0, 0)) {
        zip.close();
        this->error()->setErrorMessage(QStringLiteral("Could not write to the EPUB archive."));
        return false;
    }
    mimetypeFile.write("application/epub+zip");
    mimetypeFile.close();

    bool success = true;
    success &= addFileToZip(zip, QStringLiteral("META-INF/container.xml"), containerXml.toUtf8());
    success &= addFileToZip(zip, QStringLiteral("OEBPS/content.opf"), package.toUtf8());
    success &= addFileToZip(zip, QStringLiteral("OEBPS/nav.xhtml"), navigation.toUtf8());
    success &= addFileToZip(zip, QStringLiteral("OEBPS/screenplay.xhtml"),
                            screenplayXhtml.toUtf8());
    success &= addFileToZip(zip, QStringLiteral("OEBPS/style.css"), css.toUtf8());
    if (m_includeTitlePage)
        success &= addFileToZip(zip, QStringLiteral("OEBPS/title-page.xhtml"),
                                titlePageXhtml.toUtf8());
    if (!coverPhotoFileName.isEmpty())
        success &= addFileToZip(zip, QStringLiteral("OEBPS/images/") + coverPhotoFileName,
                                coverPhotoData);
    for (auto it = fonts.constBegin(); it != fonts.constEnd(); ++it)
        success &= addFileToZip(zip, QStringLiteral("OEBPS/fonts/") + it.key(), it.value());

    zip.close();

    if (!success)
        this->error()->setErrorMessage(QStringLiteral("Could not write to the EPUB archive."));

    return success;
}
