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

#include <QString>
#include <QStringList>

static QStringList breakStringIntoLines(const QString &inputString, int maxCharactersPerLine)
{
    QStringList lines;
    QString currentLine;

    const QStringList words = inputString.split(' ', Qt::SkipEmptyParts);

    for (const QString &word : words) {
        if (currentLine.length() + word.length() + 1 <= maxCharactersPerLine) {
            if (!currentLine.isEmpty())
                currentLine.append(' ');
            currentLine.append(word);
        } else {
            lines.append(currentLine);
            currentLine.clear();

            if (word.length() > maxCharactersPerLine) {
                int startIndex = 0;
                while (startIndex < word.length()) {
                    QString chunk = word.mid(startIndex, maxCharactersPerLine - 1) + "-";
                    lines.append(chunk);
                    startIndex += maxCharactersPerLine - 1;
                }
            } else
                currentLine.append(word);
        }
    }

    if (!currentLine.isEmpty())
        lines.append(currentLine);

    return lines;
}

QString adjustSpacesToLength(const QString &inputString, int maxLength)
{
    QString simplifiedText = inputString.simplified();
    if (simplifiedText.length() == maxLength)
        return simplifiedText;

    QStringList words = simplifiedText.split(' ', Qt::SkipEmptyParts);
    int totalSpaces = maxLength - simplifiedText.length();
    int spacesPerWord = totalSpaces / (words.size() - 1);
    QString adjustedString = words.join(QString(1 + spacesPerWord, ' '));
    int extraSpaces = maxLength - adjustedString.length();

    int insertIndex = 0;
    for (int i = 0; i < extraSpaces; ++i) {
        insertIndex = adjustedString.indexOf(' ', insertIndex) + 1;
        if (insertIndex == -1)
            break;
        adjustedString.insert(insertIndex, ' ');
        insertIndex += 2;
    }

    return adjustedString;
}

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
    const int maxChars = m_maxLettersPerLine;

    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    auto writeParagraph = [&ts, maxChars](const SceneElementFormat *format, const QString &text) {
        for (int i = 0; i < format->lineSpacingBefore(); i++)
            ts << "\n";

        const qreal blockWidth = 1.0 - format->leftMargin() - format->rightMargin();
        const int maxCharsInBlock = int(qreal(maxChars) * blockWidth);

        const QStringList lines = breakStringIntoLines(text.simplified(), maxCharsInBlock);

        for (int i = 0; i < lines.size(); i++) {
            QString line = lines.at(i);

            if (format->textAlignment().testFlag(Qt::AlignJustify)) {
                if (i < lines.size() - 1)
                    line = adjustSpacesToLength(line, maxCharsInBlock);
            }

            QString prefix;
            if (format->textAlignment().testFlag(Qt::AlignRight))
                prefix = QString(maxCharsInBlock - line.length(), ' ');

            if (format->textAlignment().testFlag(Qt::AlignHCenter))
                prefix = QString(qCeil(qreal(maxCharsInBlock - line.length()) / 2.0), ' ');

            const int leftMarginChars = int(format->leftMargin() * maxChars);
            ts << QString(leftMarginChars, ' ') << prefix << line << "\n";
        }
    };

    for (int i = 0; i < nrScenes; i++) {
        const ScreenplayElement *screenplayElement = screenplay->elementAt(i);
        if (screenplayElement->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const Scene *scene = screenplayElement->scene();
        const SceneHeading *heading = scene->heading();
        if (heading->isEnabled()) {
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
