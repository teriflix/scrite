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
#include <QRegularExpression>

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

void TextExporter::setIncludeSceneNumbers(bool val)
{
    if (m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void TextExporter::setIncludeEpisodeAndActBreaks(bool val)
{
    if (m_includeEpisodeAndActBreaks == val)
        return;

    m_includeEpisodeAndActBreaks = val;
    emit includeEpisodeAndActBreaksChanged();
}

void TextExporter::setIncludeSceneSynopsis(bool val)
{
    if (m_includeSceneSynopsis == val)
        return;

    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();
}

bool TextExporter::doExport(QIODevice *device)
{
    QTextStream ts(device);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    ts << this->toString();

    return true;
}

QString TextExporter::toString() const
{
    const ScreenplayFormat *screenplayFormat = this->document()->formatting();
    const Screenplay *screenplay = this->document()->screenplay();
    const int nrScenes = screenplay->elementCount();
    const int maxChars = m_maxLettersPerLine;
    const char *newline = "\n";

    QString ret;

    QTextStream ts(&ret, QIODevice::WriteOnly);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    auto writeParagraph = [&ts, maxChars, newline](const SceneElementFormat *format,
                                                   const QString &text) {
        for (int i = 0; i < format->lineSpacingBefore(); i++)
            ts << newline;

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
            ts << QString(leftMarginChars, ' ') << prefix << line << newline;
        }
    };

    int lastElementType = -1;

    for (int i = 0; i < nrScenes; i++) {
        const ScreenplayElement *screenplayElement = screenplay->elementAt(i);
        if (lastElementType >= 0 && lastElementType != screenplayElement->elementType())
            ts << newline;

        lastElementType = screenplayElement->elementType();

        if (screenplayElement->elementType() == ScreenplayElement::SceneElementType) {
            const Scene *scene = screenplayElement->scene();

            if (m_includeSceneSynopsis) {
                if (scene->structureElement()->hasNativeTitle() || !scene->synopsis().isEmpty()) {
                    ts << newline;

                    ts << QString(m_maxLettersPerLine, '=') << newline;
                    if (scene->structureElement()->hasNativeTitle())
                        ts << scene->structureElement()->nativeTitle() << newline;

                    if (!scene->synopsis().isEmpty())
                        writeParagraph(screenplayFormat->elementFormat(SceneElement::Action),
                                       scene->synopsis());
                    ts << QString(m_maxLettersPerLine, '=') << newline;
                }
            }

            const SceneHeading *heading = scene->heading();
            if (heading->isEnabled()) {
                ts << newline;

                if (m_includeSceneNumbers)
                    ts << "[" << screenplayElement->resolvedSceneNumber() << "] ";

                ts << heading->text() << newline;
            }

            const int nrElements = scene->elementCount();
            for (int j = 0; j < nrElements; j++) {
                const SceneElement *element = scene->elementAt(j);
                const SceneElementFormat *format = screenplayFormat->elementFormat(element->type());
                writeParagraph(format, element->formattedText());
            }
        } else {
            if (m_includeEpisodeAndActBreaks) {
                ts << screenplayElement->breakTitle();
                if (!screenplayElement->breakSubtitle().isEmpty())
                    ts << ": " << screenplayElement->breakSubtitle();
                ts << newline << newline;
            }
        }
    }

    ts.flush();

    static const QRegularExpression multipleNewlines("\\n{3,}");
    ret = ret.replace(multipleNewlines, "\n\n");
    ret = ret.trimmed();

    return ret;
}
