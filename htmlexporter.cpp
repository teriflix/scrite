/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
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

HtmlExporter::HtmlExporter(QObject *parent)
             :AbstractExporter(parent)
{

}

HtmlExporter::~HtmlExporter()
{

}

void HtmlExporter::setExportWithSceneColors(bool val)
{
    if(m_exportWithSceneColors == val)
        return;

    m_exportWithSceneColors = val;
    emit exportWithSceneColorsChanged();
}

bool HtmlExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    const ScreenplayFormat *formatting = this->document()->printFormat();
    QMap<SceneElement::Type,QString> typeStringMap;
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
    const QMetaEnum langEnum = tmo->enumerator( tmo->indexOfEnumerator("Language") );

    QMap<TransliterationEngine::Language,bool> langBundleMap = this->languageBundleMap();
    QMap<TransliterationEngine::Language,bool>::const_iterator it = langBundleMap.constBegin();
    QMap<TransliterationEngine::Language,bool>::const_iterator end = langBundleMap.constEnd();
    const QString fontsDir = QFileInfo(this->fileName()).absolutePath() + "/fonts";

    while(it != end)
    {
        if(it.key() == TransliterationEngine::English)
        {
            ++it;
            continue;
        }

        if(it.value())
        {
            QDir().mkpath(fontsDir);

            const QStringList fontSources = TransliterationEngine::instance()->languageFontFilePaths(it.key());
            Q_FOREACH(QString fontSource, fontSources)
            {
                const QString lang = QString::fromLatin1(langEnum.valueToKey(it.key()));
                const QString fontFile = QFileInfo(fontSource).fileName();
                const QString fontDest = fontsDir + "/" + lang + "/" + fontFile;
                QDir().mkpath( QFileInfo(fontDest).absolutePath() );
                QFile::copy(fontSource, fontDest);

                QRawFont rawFont(fontSource, 12);

                ts << "    @font-face {\n";
                ts << "      font-family: lang_" <<  it.key() << "_" << rawFont.weight() << "_" << rawFont.style() << ";\n";
                ts << "      src: url(fonts/" << lang << "/" << fontFile << ");\n";
                ts << "      font-weight: " << rawFont.weight() << ";\n";
                ts << "      ";
                switch(rawFont.style())
                {
                case QFont::StyleNormal: ts << "normal;\n"; break;
                case QFont::StyleItalic: ts << "italic;\n"; break;
                case QFont::StyleOblique: ts << "oblique;\n"; break;
                }
                ts << "    }\n";
                ts << "    span.lang_" << it.key() << "_" << rawFont.weight() << "_" << rawFont.style() << " {\n";
                ts << "      font-family: lang_" << it.key() << "_" << rawFont.weight() << "_" << rawFont.style() << ";\n";
                ts << "    }\n";
            }
        }

        ++it;
    }

    for(int i=SceneElement::Min; i<=SceneElement::Max; i++)
    {
        if(i > SceneElement::Min)
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
        if(format->font().bold())
            ts << "      font-weight: bold;\n";
        if(format->font().italic())
            ts << "      font-style: italic;\n";
        ts << "      color: " << format->textColor().name() << ";\n";
        if(format->backgroundColor() != Qt::transparent)
            ts << "      background-color: " << format->backgroundColor().name() << ";\n";
        ts << "      text-align: ";
        switch(format->textAlignment())
        {
        case Qt::AlignLeft:
            ts << "left;\n";
            break;
        case Qt::AlignRight:
            ts << "right;\n";
            break;
        case Qt::AlignHCenter:
            ts << "center;\n";
            break;
        case Qt::AlignJustify:
        default:
            ts << "justify;\n";
            break;
        }

        const int blockWidth = int(format->blockWidth()*100);
        int leftMargin = 0;
        int rightMargin = 0;

        ts << "      width: " << int(format->blockWidth()*100.0) << "%;\n";
        if(format->blockWidth() < 1)
        {
            switch(format->blockAlignment())
            {
            case Qt::AlignLeft:
                rightMargin = 100 - blockWidth;
                break;
            default:
            case Qt::AlignHCenter:
                leftMargin = (100 - blockWidth) >> 1;
                rightMargin = 100 - blockWidth - leftMargin;
                break;
            case Qt::AlignRight:
                leftMargin = 100 - blockWidth;
                break;
            }
        }

        ts << "      margin-left: " << leftMargin << "%;\n";
        ts << "      margin-right: " << rightMargin << "%;\n";
        ts << "      margin-top: " << format->topMargin() << "px;\n";
        ts << "      margin-bottom: " << format->bottomMargin() << "px;\n";
        ts << "      line-height: " << format->lineHeight()*1.1 << "em;\n";
        ts << "    }\n";
    }

    ts << "\n";
    ts << "    div.scrite-scene {\n";
    ts << "      padding-top: 10px;\n";
    ts << "      padding-bottom: 10px;\n";
    ts << "      padding-left: 10px;\n";
    ts << "      padding-right: 10px;\n";
    ts << "    }\n";

    ts << "    </style>\n\n";

    ts << "    <div class=\"scrite-screenplay\">\n";

    auto writeParagraph = [&ts,typeStringMap,langBundleMap](SceneElement::Type type, const QString &text) {
        const QString styleName = "scrite-" + typeStringMap.value(type);
        ts << "        <p class=\"" << styleName << "\" custom-style=\"" << styleName << "\">";
        QList<TransliterationEngine::Breakup> breakup = TransliterationEngine::instance()->breakupText(text);
        Q_FOREACH(TransliterationEngine::Breakup item, breakup)
        {
            if(item.language == TransliterationEngine::English || !langBundleMap.value(item.language,false))
                ts << "<span>" << item.string << "</span>";
            else
                ts << "<span class=\"lang_" << item.language << "_" << QFont::Normal << "_" << QFont::StyleNormal << "\">" << item.string << "</span>";
        }
        ts << "</p>\n";
    };

    const int nrScenes = screenplay->elementCount();
    int nrHeadings = 0;
    for(int i=0; i<nrScenes; i++)
    {
        const ScreenplayElement *screenplayElement = screenplay->elementAt(i);
        if(screenplayElement->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const Scene *scene = screenplayElement->scene();

        if(m_exportWithSceneColors)
        {
            const QColor sceneColor = scene->color();
            const QString sceneColorText = "rgba(" + QString::number(sceneColor.red()) + "," +
                    QString::number(sceneColor.green()) + "," +
                    QString::number(sceneColor.blue()) + ",0.1)";
            ts << "      <div class=\"scrite-scene\" custom-style=\"scrite-scene\" style=\"background-color: " << sceneColorText << ";\">\n";
        }

        const SceneHeading *heading = scene->heading();
        if(heading->isEnabled())
        {
            ++nrHeadings;
            writeParagraph(SceneElement::Heading, "[" + QString::number(nrHeadings) + "] " + heading->text());
        }

        const int nrElements = scene->elementCount();
        for(int j=0; j<nrElements; j++)
        {
            SceneElement *element = scene->elementAt(j);
            writeParagraph(element->type(), element->formattedText());
        }

        if(m_exportWithSceneColors)
            ts << "      </div>\n";
    }

    ts << "    </div>\n\n";

    ts << "  </body>\n";
    ts << "</html>\n";

    ts.flush();

    return true;
}

QString HtmlExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix().toLower() != "html")
        return fileName + ".html";
    return fileName;
}
