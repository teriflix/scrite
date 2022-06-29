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

#include "textexporter.h"

#include <QtMath>
#include <QFileInfo>

TextExporter::TextExporter(QObject *parent) : AbstractExporter(parent) { }

TextExporter::~TextExporter() { }

void TextExporter::setMaxLettersPerLine(int val)
{
    if (m_maxLettersPerLine == val)
        return;

    m_maxLettersPerLine = val;
    emit maxLettersPerLineChanged();
}

bool TextExporter::doExport(QIODevice *device)
{
    const ScreenplayFormat *screenplayFormat = this->document()->formatting();
    const Screenplay *screenplay = this->document()->screenplay();
    const int nrScenes = screenplay->elementCount();
    const int maxChars = m_maxLettersPerLine - 1;

    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    auto writeParagraph = [&ts, maxChars](const SceneElementFormat *format, const QString &text) {
        for (int i = 0; i < format->lineSpacingBefore(); i++)
            ts << "\n";

        const qreal blockWidth = 1.0 - format->leftMargin() - format->rightMargin();
        const int maxCharsInBlock = int(qreal(maxChars) * blockWidth);
        const QString rightAlignPrefix(maxChars - maxCharsInBlock, ' ');
        const QString centerAlignPrefix(int(qreal(maxChars - maxCharsInBlock) / 2.0), ' ');

        QStringList words = text.trimmed().split(" ", Qt::SkipEmptyParts);
        QStringList lines;
        QString line;
        while (!words.isEmpty()) {
            const QString word = words.takeFirst();
            if (line.length() + word.length() + 1 > maxCharsInBlock) {
                lines.append(line);
                line.clear();
            }
            if (!line.isEmpty())
                line += " ";
            line += word;
        }
        if (!line.isEmpty())
            lines.append(line);

        for (int i = 0; i < lines.size(); i++) {
            QString line = lines.at(i);
            if (line.length() < maxCharsInBlock) {
                words = line.split(" ");

                if (format->textAlignment() == Qt::AlignJustify && i < lines.size() - 1) {
                    const qreal extraSpacePerWord =
                            qreal(maxCharsInBlock - line.length()) / qreal(words.size() - 1);
                    qreal space = 0;
                    line.clear();
                    for (int j = 0; j < words.size(); j++) {
                        int nrSpaceChars = qRound(space);
                        if (j == words.size() - 1)
                            nrSpaceChars += maxCharsInBlock
                                    - (line.length() + words.at(j).length() + nrSpaceChars);
                        if (nrSpaceChars > 0)
                            space -= qreal(nrSpaceChars);
                        if (!line.isEmpty())
                            line += QString(nrSpaceChars + 1, ' ');
                        line += words.at(j);
                        space += extraSpacePerWord;
                    }
                } else if (format->textAlignment() == Qt::AlignRight) {
                    line = QString(maxCharsInBlock - line.length(), ' ') + line;
                } else if (format->textAlignment() & Qt::AlignHCenter) {
                    const int nrSpaceChars = qFloor(qreal(maxCharsInBlock - line.length()) / 2.0);
                    line = QString(nrSpaceChars, ' ') + line;
                }
            }

            ts << line << "\n";
        }
    };

    int nrHeadings = 0;
    for (int i = 0; i < nrScenes; i++) {
        const ScreenplayElement *screenplayElement = screenplay->elementAt(i);
        if (screenplayElement->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const Scene *scene = screenplayElement->scene();
        const SceneHeading *heading = scene->heading();
        if (heading->isEnabled()) {
            ++nrHeadings;
            ts << "\n[" << screenplayElement->resolvedSceneNumber() << "] " << heading->text()
               << "\n";
        }

        const int nrElements = scene->elementCount();
        for (int j = 0; j < nrElements; j++) {
            const SceneElement *element = scene->elementAt(j);
            const SceneElementFormat *format = screenplayFormat->elementFormat(element->type());
            writeParagraph(format, element->formattedText());
        }
    }

    ts.flush();

    return true;
}

QString TextExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if (fi.suffix().toLower() != "txt")
        return fileName + ".txt";
    return fileName;
}
