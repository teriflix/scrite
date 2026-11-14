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

#include "odtexporter.h"
#include "screenplayformat.h"
#include "screenplay.h"
#include "scritedocument.h"
#include "languageengine.h"
#include "scrite.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QPdfWriter>
#include <QTemporaryDir>
#include <QTextDocumentWriter>

OdtExporter::OdtExporter(QObject *parent) : AbstractTextDocumentExporter(parent) { }

OdtExporter::~OdtExporter() { }

void OdtExporter::setIncludeSceneNumbers(bool val)
{
    if (m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void OdtExporter::setPrintEachSceneOnANewPage(bool val)
{
    if (m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();

    if (val)
        this->setPrintEachActOnANewPage(false);
}

void OdtExporter::setPrintEachActOnANewPage(bool val)
{
    if (m_printEachActOnANewPage == val)
        return;

    m_printEachActOnANewPage = val;
    emit printEachActOnANewPageChanged();

    if (val)
        this->setPrintEachSceneOnANewPage(false);
}

void OdtExporter::setIncludeActBreaks(bool val)
{
    if (m_includeActBreaks == val)
        return;

    m_includeActBreaks = val;
    emit includeActBreaksChanged();
}

bool OdtExporter::doExport(QIODevice *device)
{
    const qreal pageWidth = 0; // pdfWriter.width();
    QTextDocument textDocument;
    OdtUtils::enablePageBreakMarkers(&textDocument);
    this->AbstractTextDocumentExporter::generate(&textDocument, pageWidth);

    const bool hasPageBreakMarkers =
            textDocument.toPlainText().contains(OdtUtils::pageBreakMarker());

    QTextDocumentWriter writer;
    writer.setFormat("ODF");
    writer.setDevice(device);
    writer.write(&textDocument);

    QFile *file = qobject_cast<QFile *>(device);
    if (file != nullptr) {
        file->close();
        OdtUtils::polish(file->fileName(), hasPageBreakMarkers);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString OdtUtils::pageBreakMarker()
{
    static const QString marker(3, QChar(0x2063));
    return marker;
}

static const char *pageBreakMarkersEnabledProperty = "#pageBreakMarkersEnabled";

void OdtUtils::enablePageBreakMarkers(QTextDocument *textDocument)
{
    textDocument->setProperty(pageBreakMarkersEnabledProperty, true);
}

QString OdtUtils::pageBreakMarker(const QTextDocument *textDocument)
{
    return textDocument->property(pageBreakMarkersEnabledProperty).toBool() ? pageBreakMarker()
                                                                             : QString();
}

static void removePageBreakMarker(QDomNode node)
{
    if (node.isText()) {
        QDomText textNode = node.toText();
        QString data = textNode.data();
        if (data.contains(OdtUtils::pageBreakMarker()))
            textNode.setData(data.remove(OdtUtils::pageBreakMarker()));
        return;
    }

    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling())
        removePageBreakMarker(child);
}

// Returns the first table or paragraph in or under element that contains a page break marker. A
// table is preferred over the paragraph in its cell, because LibreOffice does not break pages before
// paragraphs in cells. A section, which is what a text frame is written as, is not given a page
// break by LibreOffice, but the table or paragraph it begins with can be.
static QDomElement findMarkedElement(const QDomElement &element)
{
    const QString tagName = element.tagName();
    if ((tagName == QStringLiteral("table:table") || tagName == QStringLiteral("text:p"))
        && element.text().contains(OdtUtils::pageBreakMarker()))
        return element;

    for (QDomElement childE = element.firstChildElement(); !childE.isNull();
         childE = childE.nextSiblingElement()) {
        const QDomElement found = findMarkedElement(childE);
        if (!found.isNull())
            return found;
    }

    return QDomElement();
}

// Removes page break markers from content.xml, and makes the table or paragraph (which may be
// inside a top-level section) that contained a marker start on a new page. Returns true if
// content.xml was modified.
static bool applyPageBreakMarkers(QDomDocument &xmlDoc)
{
    QDomElement rootE = xmlDoc.documentElement();
    QDomElement stylesE = rootE.firstChildElement("office:automatic-styles");
    QDomElement textE = rootE.firstChildElement("office:body").firstChildElement("office:text");
    if (stylesE.isNull() || textE.isNull())
        return false;

    QMap<QString, QString> breakStyleNames; // original style name -> style name with page break
    bool success = false;

    QDomElement childE = textE.firstChildElement();
    while (!childE.isNull()) {
        if (childE.text().contains(OdtUtils::pageBreakMarker())) {
            QDomElement targetE = findMarkedElement(childE);
            if (targetE.isNull())
                targetE = childE;
            const bool isTable = targetE.tagName() == QStringLiteral("table:table");
            const QString styleAttr = isTable ? QStringLiteral("table:style-name")
                                              : QStringLiteral("text:style-name");
            const QString propsTag = isTable ? QStringLiteral("style:table-properties")
                                             : QStringLiteral("style:paragraph-properties");
            const QString styleName = targetE.attribute(styleAttr);

            if (!breakStyleNames.contains(styleName)) {
                const QString breakStyleName = styleName.isEmpty()
                        ? QStringLiteral("ScritePageBreak")
                        : styleName + QStringLiteral("_PageBreak");

                QDomElement styleE = stylesE.firstChildElement("style:style");
                while (!styleE.isNull() && styleE.attribute("style:name") != styleName)
                    styleE = styleE.nextSiblingElement("style:style");

                QDomElement breakStyleE;
                if (styleE.isNull()) {
                    breakStyleE = xmlDoc.createElement("style:style");
                    breakStyleE.setAttribute("style:family", isTable ? "table" : "paragraph");
                } else
                    breakStyleE = styleE.cloneNode(true).toElement();
                breakStyleE.setAttribute("style:name", breakStyleName);

                QDomElement propsE = breakStyleE.firstChildElement(propsTag);
                if (propsE.isNull()) {
                    propsE = xmlDoc.createElement(propsTag);
                    breakStyleE.appendChild(propsE);
                }
                propsE.setAttribute("fo:break-before", "page");

                stylesE.appendChild(breakStyleE);
                breakStyleNames[styleName] = breakStyleName;
            }

            targetE.setAttribute(styleAttr, breakStyleNames.value(styleName));
            removePageBreakMarker(childE);
            success = true;
        }

        childE = childE.nextSiblingElement();
    }

    return success;
}

// Copies the bundled fonts used in content.xml into the Fonts folder of the unzipped ODT, declares
// them in content.xml and lists them in the manifest. Returns true if any font was embedded, in
// which case the paths of the copied files are appended to files.
static bool embedBundledFonts(const QDir &packageDir, QDomDocument &xmlDoc,
                              QList<QPair<QString, int>> &files)
{
    QDomElement rootE = xmlDoc.documentElement();

    QSet<QString> usedFamilies;
    const QDomNodeList textPropertiesList = xmlDoc.elementsByTagName("style:text-properties");
    for (int i = 0; i < textPropertiesList.size(); i++) {
        const QString family = textPropertiesList.at(i).toElement().attribute("fo:font-family");
        if (!family.isEmpty())
            usedFamilies.insert(family);
    }

    QMap<QString, QStringList> fontFiles; // family -> resource paths
    for (const QString &family : std::as_const(usedFamilies)) {
        const QStringList files = LanguageEngine::instance()->bundledFontFilesForFamily(family);
        if (!files.isEmpty())
            fontFiles[family] = files;
    }

    if (fontFiles.isEmpty())
        return false;

    QFile manifestFile(packageDir.filePath("META-INF/manifest.xml"));
    QDomDocument manifestXml;
    if (!manifestFile.open(QFile::ReadOnly) || !manifestXml.setContent(&manifestFile))
        return false;
    manifestFile.close();

    if (!rootE.hasAttribute("xmlns:svg"))
        rootE.setAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    if (!rootE.hasAttribute("xmlns:xlink"))
        rootE.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");

    QDomElement declsE = rootE.firstChildElement("office:font-face-decls");
    if (declsE.isNull()) {
        declsE = xmlDoc.createElement("office:font-face-decls");
        rootE.insertBefore(declsE, rootE.firstChildElement("office:automatic-styles"));
    }

    packageDir.mkpath("Fonts");
    QDomElement manifestRootE = manifestXml.documentElement();

    for (auto fit = fontFiles.constBegin(); fit != fontFiles.constEnd(); ++fit) {
        QDomElement faceE = xmlDoc.createElement("style:font-face");
        faceE.setAttribute("style:name", fit.key());
        faceE.setAttribute("svg:font-family", QStringLiteral("'%1'").arg(fit.key()));
        QDomElement srcE = xmlDoc.createElement("svg:font-face-src");
        faceE.appendChild(srcE);

        for (const QString &path : fit.value()) {
            const QString fileName = QFileInfo(path).fileName();
            const QString relPath = QStringLiteral("Fonts/") + fileName;
            const QString dstPath = packageDir.filePath(relPath);
            if (!QFile::copy(path, dstPath))
                continue;

            // Files copied from resources are read-only, which can keep the temporary folder from
            // being removed on some platforms.
            QFile::setPermissions(dstPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);

            QDomElement uriE = xmlDoc.createElement("svg:font-face-uri");
            uriE.setAttribute("xlink:href", relPath);
            uriE.setAttribute("xlink:type", "simple");
            uriE.setAttribute("svg:font-style", fileName.contains("Italic") ? "italic" : "normal");
            uriE.setAttribute("svg:font-weight", fileName.contains("Bold") ? "bold" : "normal");
            QDomElement formatE = xmlDoc.createElement("svg:font-face-format");
            formatE.setAttribute("svg:string", "truetype");
            uriE.appendChild(formatE);
            srcE.appendChild(uriE);

            QDomElement entryE = manifestXml.createElement("manifest:file-entry");
            entryE.setAttribute("manifest:full-path", relPath);
            entryE.setAttribute("manifest:media-type", "application/x-font-ttf");
            manifestRootE.appendChild(entryE);

            files.append({ relPath, -1 });
        }

        declsE.appendChild(faceE);
    }

    if (!manifestFile.open(QFile::WriteOnly))
        return false;
    QTextStream ts(&manifestFile);
    ts.setEncoding(QStringConverter::Utf8);
    ts << manifestXml.toString();
    ts.flush();
    manifestFile.close();

    return true;
}

// QTextDocumentWriter writes only fo:font-family, fo:font-size, fo:font-weight and fo:font-style.
// LibreOffice applies those to western text alone, and picks fonts for complex (Indic etc.) and
// asian text from the corresponding style:*-complex and style:*-asian attributes. So we copy the
// western font attributes over to them. Returns true if content.xml was modified.
static bool applyFontsToAllScripts(QDomDocument &xmlDoc)
{
    static const QList<QPair<QString, QString>> attributeNames = {
        { QStringLiteral("fo:font-family"), QStringLiteral("font-family") },
        { QStringLiteral("fo:font-size"), QStringLiteral("font-size") },
        { QStringLiteral("fo:font-weight"), QStringLiteral("font-weight") },
        { QStringLiteral("fo:font-style"), QStringLiteral("font-style") }
    };

    bool success = false;

    const QDomNodeList textPropertiesList =
            xmlDoc.elementsByTagName(QStringLiteral("style:text-properties"));
    for (int i = 0; i < textPropertiesList.size(); i++) {
        QDomElement textProperties = textPropertiesList.at(i).toElement();
        for (const QPair<QString, QString> &names : attributeNames) {
            if (!textProperties.hasAttribute(names.first))
                continue;

            const QString value = textProperties.attribute(names.first);
            textProperties.setAttribute(QStringLiteral("style:") + names.second
                                                + QStringLiteral("-complex"),
                                        value);
            textProperties.setAttribute(QStringLiteral("style:") + names.second
                                                + QStringLiteral("-asian"),
                                        value);
            success = true;
        }
    }

    return success;
}

bool OdtUtils::polish(const QString &fileName, bool applyMarkers)
{
    const QFileInfo fi(fileName);

    QTemporaryDir tmpDir;
    tmpDir.setAutoRemove(true);

    if (!Scrite::doUnzip(fi, tmpDir))
        return false;

    // Load XML
    const QString contentXmlFilePath = tmpDir.filePath("content.xml");
    if (!QFile::exists(contentXmlFilePath))
        return false;

    QFile contentXmlFile(contentXmlFilePath);
    if (!contentXmlFile.open(QFile::ReadOnly))
        return false;

    QDomDocument contentXmlDom;
    QDomDocument::ParseResult parseResult = contentXmlDom.setContent(&contentXmlFile);
    if (!parseResult)
        return false;

    contentXmlFile.close();

    // Polish & Save
    QList<QPair<QString, int>> zipFiles = { qMakePair<QString, int>("mimetype", 0),
                                            qMakePair<QString, int>("content.xml", -1),
                                            qMakePair<QString, int>("META-INF/manifest.xml", -1) };

    bool polished = applyMarkers && applyPageBreakMarkers(contentXmlDom);
    polished |= applyFontsToAllScripts(contentXmlDom);
    polished |= embedBundledFonts(QDir(tmpDir.path()), contentXmlDom, zipFiles);
    if (!polished)
        return false;

    if (!contentXmlFile.open(QFile::WriteOnly))
        return false;

    QTextStream ts(&contentXmlFile);
    ts.setEncoding(QStringConverter::Utf8);
    ts.setAutoDetectUnicode(true);
    ts << contentXmlDom.toString();
    ts.flush();

    contentXmlFile.close();

    // Zip Contents
    return Scrite::doZip(fi, QDir(tmpDir.path()), zipFiles);
}
