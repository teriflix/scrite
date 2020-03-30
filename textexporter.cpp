/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "textexporter.h"

#include <QtMath>
#include <QFileInfo>

TextExporter::TextExporter(QObject *parent)
             :AbstractExporter(parent),
              m_maxLettersPerLine(80)
{

}

TextExporter::~TextExporter()
{

}

void TextExporter::setMaxLettersPerLine(int val)
{
    if(m_maxLettersPerLine == val)
        return;

    m_maxLettersPerLine = val;
    emit maxLettersPerLineChanged();
}

bool TextExporter::doExport(QIODevice *device)
{
    const ScreenplayFormat *screenplayFormat = this->document()->formatting();
    const Screenplay *screenplay = this->document()->screenplay();
    const int nrScenes = screenplay->elementCount();

    QTextStream ts(device);

    auto writeParagraph = [&ts,this](const SceneElementFormat *format, const QString &text) {
        if(format->topMargin() > 0)
            ts << "\n";

        const int maxCharsInBlock = int(qreal(m_maxLettersPerLine)*format->blockWidth());
        const QString rightAlignPrefix(m_maxLettersPerLine-maxCharsInBlock, ' ');
        const QString centerAlignPrefix(int(qreal(m_maxLettersPerLine-maxCharsInBlock)/2.0), ' ');

        QStringList words = text.trimmed().split(" ", QString::SkipEmptyParts);
        QStringList lines;
        QString line;
        while(!words.isEmpty()) {
            const QString word = words.takeFirst();
            if(line.length() + word.length() + 1 > maxCharsInBlock) {
                lines.append(line);
                line.clear();
            }
            if(!line.isEmpty())
                line += " ";
            line += word;
        }
        if(!line.isEmpty())
            lines.append(line);

        for(int i=0; i<lines.size(); i++) {
            QString line = lines.at(i);
            if(line.length() < maxCharsInBlock) {
                words = line.split(" ");

                if(format->textAlignment() == Qt::AlignJustify && i < lines.size()-1) {
                    const qreal extraSpacePerWord = qreal(maxCharsInBlock-line.length())/qreal(words.size()-1);
                    qreal space = 0;
                    line.clear();
                    for(int j=0; j<words.size(); j++) {
                        int nrSpaceChars = qRound(space);
                        if(j == words.size()-1)
                            nrSpaceChars += maxCharsInBlock-(line.length()+words.at(j).length()+nrSpaceChars);
                        if(nrSpaceChars > 0)
                            space -= qreal(nrSpaceChars);
                        if(!line.isEmpty())
                            line += QString(nrSpaceChars+1, ' ');
                        line += words.at(j);
                        space += extraSpacePerWord;
                    }
                } else if(format->textAlignment() == Qt::AlignRight) {
                    line = QString(maxCharsInBlock-line.length(), ' ') + line;
                } else if(format->textAlignment() & Qt::AlignHCenter) {
                    const int nrSpaceChars = qFloor(qreal(maxCharsInBlock-line.length())/2.0);
                    line = QString(nrSpaceChars, ' ') + line;
                }
            }

            if(format->blockAlignment() & Qt::AlignHCenter)
                line = centerAlignPrefix + line;
            else if(format->blockAlignment() == Qt::AlignRight)
                line = rightAlignPrefix + line;

            ts << line << "\n";
        }

        if(format->bottomMargin() > 0)
            ts << "\n";
    };

    int nrHeadings = 0;
    for(int i=0 ;i<nrScenes; i++)
    {
        const Scene *scene = screenplay->elementAt(i)->scene();
        const SceneHeading *heading = scene->heading();
        if(heading->isEnabled())
        {
            ++nrHeadings;
            ts << "\n[" << nrHeadings << "] " << heading->toString() << "\n";
        }

        const int nrElements = scene->elementCount();
        for(int j=0; j<nrElements; j++)
        {
            const SceneElement *element = scene->elementAt(j);
            const SceneElementFormat *format = screenplayFormat->elementFormat(element->type());
            writeParagraph(format, element->text());
        }
    }

    ts.flush();

    return true;
}

QString TextExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix() != "txt")
        return fileName + ".txt";
    return fileName;
}
