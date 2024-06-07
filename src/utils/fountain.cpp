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

#include "fountain.h"

#include <QIODevice>
#include <QJsonArray>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QtDebug>

namespace Fountain {
static bool resolveEmphasis(const QString &input, QString &plainText,
                            QVector<QTextLayout::FormatRange> &formats);
static bool encodeEmphasis(const QString &plainText,
                           const QVector<QTextLayout::FormatRange> &formats, QString &mdText);
static QStringList sceneHeadingPrefixes();

} // namespace Fountain

QJsonObject Fountain::Element::toJson() const
{
    QJsonObject ret;

    auto typeAsString = [](Fountain::Element::Type type) -> QString {
        switch (type) {
        case Fountain::Element::None:
            return QStringLiteral("None");
        case Fountain::Element::Unknown:
            return QStringLiteral("Unknown");
        case Fountain::Element::SceneHeading:
            return QStringLiteral("SceneHeading");
        case Fountain::Element::Action:
            return QStringLiteral("Action");
        case Fountain::Element::Character:
            return QStringLiteral("Character");
        case Fountain::Element::Dialogue:
            return QStringLiteral("Dialogue");
        case Fountain::Element::Parenthetical:
            return QStringLiteral("Parenthetical");
        case Fountain::Element::Lyrics:
            return QStringLiteral("Lyrics");
        case Fountain::Element::Shot:
            return QStringLiteral("Shot");
        case Fountain::Element::Transition:
            return QStringLiteral("Transition");
        case Fountain::Element::PageBreak:
            return QStringLiteral("PageBreak");
        case Fountain::Element::LineBreak:
            return QStringLiteral("LineBreak");

        case Fountain::Element::Section:
            return QStringLiteral("Section");
        case Fountain::Element::Synopsis:
            return QStringLiteral("Synopsis");
        default:
            return QStringLiteral("InvalidType");
        }
    };

    ret["type"] = typeAsString(this->type);

    if (!this->text.isEmpty())
        ret["text"] = this->text;

    if (this->isCentered)
        ret["isCentered"] = this->isCentered;

    if (!this->sceneNumber.isEmpty())
        ret["sceneNumber"] = this->sceneNumber;

    if (this->sectionDepth > 0)
        ret["sectionDepth"] = this->sectionDepth;

    if (!this->notes.isEmpty())
        ret["notes"] = QJsonArray::fromStringList(this->notes);

    if (!this->formats.isEmpty()) {
        QJsonArray formatsArray;
        for (const QTextLayout::FormatRange &format : this->formats) {
            QJsonObject fmt;
            fmt["start"] = format.start;
            fmt["length"] = format.length;
            if (format.format.hasProperty(QTextFormat::FontWeight))
                fmt["bold"] = format.format.fontWeight() != QFont::Medium;
            if (format.format.hasProperty(QTextFormat::FontItalic))
                fmt["italic"] = format.format.fontItalic();
            if (format.format.hasProperty(QTextFormat::FontUnderline))
                fmt["underline"] = format.format.fontUnderline();
            formatsArray.append(fmt);
        }
        ret["formats"] = formatsArray;
    }

    return ret;
}

Fountain::Parser::Parser(const QString &content, int options) : m_options(options)
{
    this->parseContents(content);
}

Fountain::Parser::Parser(const QByteArray &content, int options) : m_options(options)
{
    this->parseContents(QString::fromUtf8(content));
}

Fountain::Parser::Parser(QIODevice *device, int options) : m_options(options)
{
    if (device) {
        if (!device->isOpen())
            device->open(QIODevice::ReadOnly);

        if (device->isOpen())
            this->parseContents(QString::fromUtf8(device->readAll()));

        device->close();
    }
}

Fountain::Parser::~Parser() { }

QJsonObject Fountain::Parser::toJson() const
{
    QJsonObject ret;

    ret["#kind"] = "Fountain/Parser/Json";
    ret["#standard"] = "https://fountain.io/syntax/";

    QJsonObject titlePage;
    for (const QPair<QString, QString> &tuple : m_titlePage)
        titlePage[tuple.first] = tuple.second;
    if (!titlePage.isEmpty())
        ret["titlePage"] = titlePage;

    QJsonArray body;
    for (const Fountain::Element &element : m_body)
        body.append(element.toJson());

    if (!body.isEmpty())
        ret["body"] = body;

    return ret;
}

void Fountain::Parser::parseContents(const QString &givenContent)
{
    m_body.clear();
    m_titlePage.clear();

    if (m_options == 0) {
        const QStringList lines = givenContent.split("\n", Qt::SkipEmptyParts);
        std::transform(lines.begin(), lines.end(), std::back_inserter(m_body),
                       [](const QString &line) {
                           Fountain::Element fElement;
                           fElement.type = Fountain::Element::Action;
                           fElement.text = line.trimmed();
                           return fElement;
                       });
        return;
    }

    // Remove leading whitespaces in each line, standardize all new-lines
    const QString content = this->cleanup(givenContent);

    // See if the file has title-page fields.
    const int firstBlankLine = content.indexOf("\n\n");
    if (firstBlankLine >= 0) {
        const QString titlePageContent = content.left(firstBlankLine);
        this->parseTitlePage(titlePageContent);

        const QString bodyContent =
                m_titlePage.isEmpty() ? content : content.mid(firstBlankLine + 1);
        this->parseBody(bodyContent);
    } else
        this->parseBody(content);
}

void Fountain::Parser::parseTitlePage(const QString &content)
{
    const QChar colon = ':';
    const QChar newline = '\n';
    const QStringList lines = content.split(newline, Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        const QString trimmedLine = line.trimmed();

        if (trimmedLine.contains(colon)) {
            QString key = trimmedLine.section(colon, 0, 0).toLower();
            if (key == "author")
                key = "authors";

            if (trimmedLine.endsWith(colon)) {
                // Contains only key, no value
                m_titlePage.append(qMakePair(key, QString()));
                continue;
            }

            // Contains both key and value
            QString value = trimmedLine.section(colon, 1).trimmed();
            m_titlePage.append(qMakePair(key, value));
        } else {
            // This means that the line belongs to a multiline setup.
            if (m_titlePage.size()) {
                QString &value = m_titlePage.last().second;
                if (value.isEmpty())
                    value = trimmedLine;
                else
                    value += newline + trimmedLine;
            }
        }
    }
}

void Fountain::Parser::parseBody(const QString &content)
{
    // Split content across line boundary
    const QChar newline = '\n';
    const QStringList lines = content.split(newline);

    auto isPageBreak = [](const QString &text) -> bool {
        /*
         * http://fountain.io/syntax/#page-breaks
         */
        static const QRegularExpression regExp("={3,}");
        const QRegularExpressionMatch match = regExp.match(text);
        return (match.hasMatch() && match.captured() == text);
    };

    // Construct an element for each line, assuming that each line is a new
    // element.
    std::transform(lines.begin(), lines.end(), std::back_inserter(m_body),
                   [=](const QString &line) {
                       static const QRegularExpression regex("[\r\n]+$");
                       static const QRegularExpression leadingWhitespaceRegex("^\\s+");
                       static const QRegularExpression trailingWhitespaceRegex("\\s+$");

                       QString endingNewLinesRemoved = line;
                       endingNewLinesRemoved.remove(regex);

                       QString whiteSpacesRemoved = endingNewLinesRemoved;
                       if (m_options & IgnoreLeadingWhitespaceOption)
                           whiteSpacesRemoved = whiteSpacesRemoved.remove(leadingWhitespaceRegex);
                       if (m_options & IgnoreTrailingWhiteSpaceOption)
                           whiteSpacesRemoved = whiteSpacesRemoved.remove(trailingWhitespaceRegex);

                       Fountain::Element element;
                       element.type = whiteSpacesRemoved.isEmpty() ? Fountain::Element::LineBreak
                               : isPageBreak(whiteSpacesRemoved)   ? Fountain::Element::PageBreak
                                                                   : Fountain::Element::Unknown;
                       if (element.type == Fountain::Element::Unknown) {
                           element.text = endingNewLinesRemoved;

                           element.trimmedText = line.trimmed();
                           element.simplifiedText = line.simplified();
                           element.containsNonLatinChars = [](const QString &text) {
                               for (const QChar &ch : text) {
                                   if (ch.isLetter() && ch.script() != QChar::Script_Latin)
                                       return true;
                               }
                               return false;
                           }(line);
                       } else
                           element.text = QString();

                       return element;
                   });

    // Remove starting and trailing newlines.
    while (m_body.size() && m_body.last().type == Fountain::Element::LineBreak)
        m_body.takeLast();
    while (m_body.size() && m_body.first().type == Fountain::Element::LineBreak)
        m_body.takeFirst();

    this->processSectionsAndSynopsis();
    this->processLyrics();

    this->processFormalAction();
    this->processSceneHeadings();
    this->processShotsAndTransitions();
    this->processCharacters();
    this->processDialogueAndParentheticals();
    this->processAction();

    this->joinAdjacentElements();

    this->processNotes();
    this->processEmphasis();

    this->removeEmptyLines();

    std::for_each(m_body.begin(), m_body.end(), [](Fountain::Element &element) {
        element.trimmedText = QString();
        element.simplifiedText = QString();
    });
}

void Fountain::Parser::processFormalAction()
{
    /*
     * https://fountain.io/syntax/#action
     */

    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];
        if (element.type != Fountain::Element::Unknown)
            continue;

        const QString trimmedText = element.trimmedText;
        if (trimmedText.startsWith('!')) {
            element.type = Fountain::Element::Action;
            element.text = trimmedText.mid(1);
            continue;
        }
    }
}

void Fountain::Parser::processSceneHeadings()
{
    /*
     * http://fountain.io/syntax/#scene-headings
     */
    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];
        if (element.type != Fountain::Element::Unknown)
            continue;

        auto extractSceneNumber = [](QString &sceneHeading) -> QString {
            /*
             * Power user: Scene Headings can optionally be appended with Scene
             * Numbers. Scene numbers are any alphanumerics (plus dashes and periods),
             * wrapped in #. All of the following are valid scene numbers:
             */
            static const QRegularExpression regExp("(.*)(\\#([0-9A-Za-z\\.\\)-]+)\\#)");
            const QRegularExpressionMatch match = regExp.match(sceneHeading);
            if (match.hasMatch()) {
                sceneHeading = match.captured(1).trimmed();
                return match.captured(3);
            }

            return QString();
        };

        // If the line is forced into being a scene heading
        if (element.trimmedText.startsWith('.') && element.trimmedText.length() >= 2
            && element.trimmedText.at(1) != QChar('.')) {
            element.text = element.trimmedText.mid(1).toUpper().simplified();
            element.sceneNumber = extractSceneNumber(element.text);
            element.type = Fountain::Element::SceneHeading;
            continue;
        }

        const bool nextLineIsEmpty = (i == m_body.size() - 1)
                || ((i + 1) < m_body.size()
                    && m_body.at(i + 1).type == Fountain::Element::LineBreak);
        const bool prevLineIsEmpty =
                i == 0 || m_body.at(i - 1).type == Fountain::Element::LineBreak;

        if (nextLineIsEmpty && prevLineIsEmpty) {
            // Otherwise it should begin with one of the following
            // INT, EXT, EST, INT./EXT, INT/EXT, I/E
            const QStringList prefixes = Fountain::sceneHeadingPrefixes();
            const QString simplifiedText = element.simplifiedText;
            for (const QString &prefix : prefixes) {
                if (simplifiedText.startsWith(prefix + ".", Qt::CaseSensitive)) {
                    element.text = simplifiedText;
                    element.sceneNumber = extractSceneNumber(element.text);
                    element.type = Fountain::Element::SceneHeading;
                    continue;
                }
            }
        }
    }
}

void Fountain::Parser::processShotsAndTransitions()
{
    /*
     * http://fountain.io/syntax/#transition
     */

    /**
     * Although Fountain syntax says that transitions must end with TO:, in the
     * real world a lot of transitions don't end that way. So, we can't really
     * rely on that alone.
     */

    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];
        if (element.type != Fountain::Element::Unknown)
            continue;

        const bool nextLineIsEmpty = (i == m_body.size() - 1)
                || ((i + 1) < m_body.size()
                    && m_body.at(i + 1).type == Fountain::Element::LineBreak);
        const bool prevLineIsEmpty =
                i == 0 || m_body.at(i - 1).type == Fountain::Element::LineBreak;

        if (element.trimmedText.startsWith('>') && !element.trimmedText.endsWith('<')) {
            element.text = element.trimmedText.mid(1).toUpper().simplified();
            element.type = Fountain::Element::Transition;
            continue;
        }

        if (prevLineIsEmpty && nextLineIsEmpty /* && element.text.toUpper() == element.text*/) {
            const QString simplifiedText = element.simplifiedText.toUpper();
            if (simplifiedText.endsWith("TO:")) {
                element.text = simplifiedText;
                element.type = Fountain::Element::Transition;
                continue;
            }

            static const QStringList knownTransitions = { QStringLiteral("CUT TO"),
                                                          QStringLiteral("DISSOLVE TO"),
                                                          QStringLiteral("FADE IN"),
                                                          QStringLiteral("FADE OUT"),
                                                          QStringLiteral("FADE TO"),
                                                          QStringLiteral("FLASHBACK"),
                                                          QStringLiteral("FLASH CUT TO"),
                                                          QStringLiteral("FREEZE FRAME"),
                                                          QStringLiteral("IRIS IN"),
                                                          QStringLiteral("IRIS OUT"),
                                                          QStringLiteral("JUMP CUT TO"),
                                                          QStringLiteral("MATCH CUT TO"),
                                                          QStringLiteral("MATCH DISSOLVE TO"),
                                                          QStringLiteral("SMASH CUT TO"),
                                                          QStringLiteral("STOCK SHOT"),
                                                          QStringLiteral("TIME CUT"),
                                                          QStringLiteral("WIPE TO") };
            for (const QString &knownTransition : knownTransitions) {
                if (simplifiedText == knownTransition || simplifiedText == knownTransition + ":"
                    || simplifiedText == knownTransition + ".") {
                    element.text = knownTransition + ":";
                    element.type = Fountain::Element::Transition;
                    continue;
                }
            }

            static const QStringList knownShots = {
                QStringLiteral("AIR"),          QStringLiteral("CLOSE ON"),
                QStringLiteral("CLOSER ON"),    QStringLiteral("CLOSEUP"),
                QStringLiteral("ESTABLISHING"), QStringLiteral("EXTREME CLOSEUP"),
                QStringLiteral("INSERT"),       QStringLiteral("POV"),
                QStringLiteral("SURFACE"),      QStringLiteral("THREE SHOT"),
                QStringLiteral("TWO SHOT"),     QStringLiteral("UNDERWATER"),
                QStringLiteral("WIDE"),         QStringLiteral("WIDE ON"),
                QStringLiteral("WIDER ANGLE")
            };

            for (const QString &knownShot : knownShots) {
                if (simplifiedText == knownShot || simplifiedText == knownShot + ":"
                    || simplifiedText == knownShot + ".") {
                    element.text = knownShot + ":";
                    element.type = Fountain::Element::Shot;
                    continue;
                }
            }
        }
    }
}

void Fountain::Parser::processCharacters()
{
    /*
     * http://fountain.io/syntax/#charater
     */

    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];
        if (element.type != Fountain::Element::Unknown)
            continue;

        const bool nextLineIsEmpty = (i == m_body.size() - 1)
                || ((i + 1) < m_body.size()
                    && m_body.at(i + 1).type == Fountain::Element::LineBreak);
        const bool prevLineIsEmpty =
                i == 0 || m_body.at(i - 1).type == Fountain::Element::LineBreak;

        if (prevLineIsEmpty && !nextLineIsEmpty && i + 1 < m_body.size()) {
            const QString simplifiedText = element.simplifiedText;
            if (simplifiedText.endsWith('.') || simplifiedText.endsWith(':')
                || simplifiedText.startsWith('>') || simplifiedText.endsWith('<'))
                continue;

            if (simplifiedText.startsWith('@')) {
                element.type = Fountain::Element::Character;
                element.text = simplifiedText.mid(1).trimmed();
                continue;
            }

            bool isCharacter = false;
            const int boIndex = simplifiedText.indexOf('(');
            const int bcIndex = simplifiedText.lastIndexOf(')');
            if (boIndex > 0) {
                if (bcIndex > 0 && bcIndex > boIndex) {
                    const QString maybeCharacterName = simplifiedText.left(boIndex).trimmed();
                    isCharacter = (maybeCharacterName.toUpper() == maybeCharacterName);
                }
            } else {
                isCharacter = !element.containsNonLatinChars
                        && simplifiedText.toUpper() == simplifiedText;
            }

            if (isCharacter) {
                element.type = Fountain::Element::Character;
                element.text = simplifiedText;
                continue;
            }
        }
    }
}

void Fountain::Parser::processDialogueAndParentheticals()
{
    /*
     * http://fountain.io/syntax/#dialogue
     * http://fountain.io/syntax/#parenthetical
     */

    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];

        // Go on until we find a character element.
        if (element.type != Fountain::Element::Character)
            continue;

        // Once we get a character element, determine if the following lines are
        // parentheticals or dialogue.
        ++i;
        int nrParentheticals = 0;
        for (; i < m_body.size(); i++) {
            Fountain::Element &dpElement = m_body[i];
            if (dpElement.type != Fountain::Element::Unknown) {
                --i;
                break;
            }

            const QString simplifiedText = dpElement.simplifiedText;
            dpElement.text = simplifiedText;

            if (simplifiedText.startsWith('(')) {
                ++nrParentheticals;

                dpElement.type = Fountain::Element::Parenthetical;
                if (simplifiedText.endsWith(')'))
                    --nrParentheticals;
            } else {
                if (nrParentheticals > 0) {
                    dpElement.type = Fountain::Element::Parenthetical;
                    if (simplifiedText.endsWith(')'))
                        --nrParentheticals;
                } else
                    dpElement.type = Fountain::Element::Dialogue;
            }
        }
    }
}

void Fountain::Parser::processLyrics()
{
    /*
     * http://fountain.io/syntax/#lyrics
     */
    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];
        if (element.type != Fountain::Element::Unknown)
            continue;

        const QString trimmedText = element.trimmedText;

        if (trimmedText.startsWith('~')) {
            element.type = Fountain::Element::Lyrics;
            element.text = trimmedText.mid(1).trimmed();
        }
    }
}

void Fountain::Parser::processSectionsAndSynopsis()
{
    /*
     * https://fountain.io/syntax/#sections-synopses
     */

    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];
        if (element.type != Fountain::Element::Unknown)
            continue;

        QString trimmedText = element.trimmedText;

        if (trimmedText.startsWith('=')) {
            element.type = Fountain::Element::Synopsis;
            element.text = trimmedText.mid(1).trimmed();
            continue;
        }

        static const QRegularExpression sectionRegExp("^(#+)(.*)$");
        const QRegularExpressionMatch sectionMatch = sectionRegExp.match(trimmedText);
        if (sectionMatch.hasMatch()) {
            const QString hashes = sectionMatch.captured(1);
            element.type = Fountain::Element::Section;
            element.sectionDepth = hashes.length();
            element.text = sectionMatch.captured(2).trimmed();
            continue;
        }
    }
}

void Fountain::Parser::processAction()
{
    /*
     * https://fountain.io/syntax/#action
     */

    for (int i = 0; i < m_body.size(); i++) {
        Fountain::Element &element = m_body[i];
        if (element.type == Fountain::Element::Unknown)
            element.type = Fountain::Element::Action;

        if (element.type == Fountain::Element::Action) {
            QString trimmedText = element.trimmedText;
            if (trimmedText.startsWith('>') && trimmedText.endsWith('<')) {
                trimmedText = trimmedText.mid(1, trimmedText.length() - 2);
                element.text = trimmedText.trimmed();
                element.isCentered = true;
            }
        }
    }
}

void Fountain::Parser::joinAdjacentElements()
{
    /*
     * This part is specific to this particular parser. If we have two dialogue or
     * action paragraphs adjacent to each other, we should merge them into a
     * single paragraph.
     */
    if (m_options & JoinAdjacentElementOption) {
        const QList<Fountain::Element::Type> joinableTypes = { Fountain::Element::Action,
                                                               Fountain::Element::Dialogue };

        for (int i = m_body.size() - 1; i >= 1; i--) {
            Fountain::Element &current = m_body[i];
            Fountain::Element &previous = m_body[i - 1];
            if (joinableTypes.contains(current.type) && current.type == previous.type) {
                previous.text = previous.text + " " + current.text;
                previous.text = previous.text.trimmed();

                current.text.clear();
                current.type = Fountain::Element::None;
            }
        }
    }
}

void Fountain::Parser::processNotes()
{
    /*
     * https://fountain.io/syntax/#notes
     */

    // Here, we only support limited parsing of notes.
    // If JoinAdjacentElementOption is not enabled, then we only process lines
    // in which an entire note exists.
    // Notes with line breaks are not supported.

    static const QList<Fountain::Element::Type> allowedTypes = { Fountain::Element::Action,
                                                                 Fountain::Element::Dialogue };
    static const QRegularExpression regex("\\[\\[(.*?)\\]\\]");

    for (Fountain::Element &element : m_body) {
        if (allowedTypes.contains(element.type)) {
            const QRegularExpressionMatch match = regex.match(element.text);
            if (match.hasMatch()) {
                element.notes = match.capturedTexts();
                if (!element.notes.isEmpty())
                    element.notes.removeFirst();
                element.text = element.text.remove(regex).simplified();
            }
        } else {
            element.text = element.text.remove(regex).simplified();
        }
    }
}

void Fountain::Parser::processEmphasis()
{
    /*
     * Fountain follows Markdown’s rules for emphasis, except that it reserves the
     * use of underscores for underlining, which is not interchangeable with
     * italics in a screenplay.
     *      * *italics*
     * **bold**
     * ***bold italics***
     * _underline_
     *      * In this way the writer can mix and match and combine bold, italics
     * and underlining, as screenwriters often do.
     */

    if (m_options & ResolveEmphasisOption) {
        for (Fountain::Element &element : m_body)
            Fountain::resolveEmphasis(element.text, element.text, element.formats);
    }
}

void Fountain::Parser::removeEmptyLines()
{
    QList<Fountain::Element> filteredElements;
    std::copy_if(m_body.begin(), m_body.end(), std::back_inserter(filteredElements),
                 [](const Fountain::Element &element) {
                     return (element.type != Fountain::Element::LineBreak
                             && element.type != Fountain::Element::Unknown
                             && element.type != Fountain::Element::None);
                 });

    m_body = filteredElements;
}

QString Fountain::Parser::cleanup(const QString &content) const
{
    QString ret = content.trimmed();

    // Remove all comments from the entire code.
    static const QRegularExpression commentRegex("/\\*.*?\\*/",
                                                 QRegularExpression::DotMatchesEverythingOption);
    ret = ret.remove(commentRegex);

    QStringList lines = ret.split("\n");
    for (QString &line : lines) {
        static const QRegularExpression splitTxHeadingRegex(
                "^[A-Z ]*: *\\b(INT|EXT|EST|INT\\.?\\/ ?EXT|I\\/E)\\b");
        if (splitTxHeadingRegex.match(line).hasMatch()) {
            int index = line.indexOf(':');
            line.insert(index + 1, "\n\n");
        }
    }
    ret = lines.join("\n");

    return ret;
}

static bool Fountain::resolveEmphasis(const QString &input, QString &plainText,
                                      QVector<QTextLayout::FormatRange> &formats)
{
    static const QRegularExpression regex("\\*{1,3}|_{1}");
    if (!regex.match(input).hasMatch())
        return false;

    // Define regular expression patterns for formatting
    static const QRegularExpression italicPattern("\\*(.*?)\\*");
    static const QRegularExpression boldPattern("\\*\\*(.*?)\\*\\*");
    static const QRegularExpression boldItalicPattern("\\*\\*\\*(.*?)\\*\\*\\*");
    static const QRegularExpression underlinePattern("\\_(.*?)\\_");

    // Apply formatting using regular expressions
    QString formattedText = input;
    formattedText.replace(boldItalicPattern, "<b><i>\\1</i></b>");
    formattedText.replace(boldPattern, "<b>\\1</b>");
    formattedText.replace(italicPattern, "<i>\\1</i>");
    formattedText.replace(underlinePattern, "<u>\\1</u>");

    if (formattedText == input)
        return false;

    QTextDocument doc;
    doc.setHtml(formattedText);

    const QTextBlock block = doc.firstBlock();
    plainText = block.text();
    formats = block.textFormats();

    return true;
}

static bool Fountain::encodeEmphasis(const QString &plainText,
                                     const QVector<QTextLayout::FormatRange> &formats,
                                     QString &mdText)
{
    if (formats.isEmpty()) {
        mdText = plainText;
        return true;
    }

    QTextDocument document;

    QTextCursor cursor(&document);
    cursor.insertText(plainText);
    for (const QTextLayout::FormatRange &format : formats) {
        cursor.setPosition(format.start);
        cursor.setPosition(format.start + format.length, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(format.format);
        cursor.clearSelection();
    }
    cursor.setPosition(0);

    const QTextBlock block = cursor.block();

    QTextBlock::iterator it = block.begin();
    while (it != block.end()) {
        const QTextFragment fragment = it.fragment();
        if (fragment.isValid()) {
            QString text = fragment.text();
            if (text.isEmpty()) {
                ++it;
                continue;
            }

            QString leftOver;
            while (text.length() > 0 && text.at(text.length() - 1).isSpace()) {
                leftOver = text.at(text.length() - 1) + leftOver;
                text = text.remove(text.length() - 1, 1);
            }

            if (text.isEmpty()) {
                mdText += leftOver;
                ++it;
                continue;
            }

            const QTextCharFormat charFormat = fragment.charFormat();
            const bool bold = charFormat.fontWeight() == QFont::Bold;
            const bool italic = charFormat.fontItalic();
            const bool underline = charFormat.fontUnderline();

            if (italic)
                mdText += "*";
            if (bold)
                mdText += "**";
            if (underline)
                mdText += "_";

            mdText += text;

            if (underline)
                mdText += "_";
            if (bold)
                mdText += "**";
            if (italic)
                mdText += "*";

            mdText += leftOver;
        }

        ++it;
    }

    return true;
}

static QStringList Fountain::sceneHeadingPrefixes()
{
    return { "INT", "EXT", "EST", "INT./EXT", "INT/EXT", "I/E" };
}

Fountain::Writer::Writer(QList<QPair<QString, QString>> &titlePage, const QList<Element> &body,
                         int options)
    : m_titlePage(titlePage), m_body(body), m_options(options)
{
}

Fountain::Writer::Writer(const QList<Element> &body, int options) : m_body(body), m_options(options)
{
}

Fountain::Writer::Writer(const Screenplay *screenplay, int options) : m_options(options)
{
    Fountain::populateTitlePage(screenplay, m_titlePage);
    Fountain::populateBody(screenplay, m_body);
}

Fountain::Writer::Writer(const ScreenplayElement *element, int options) : m_options(options)
{
    Fountain::populateBody(element, m_body);
}

Fountain::Writer::Writer(const Scene *scene, const ScreenplayElement *element, int options)
    : m_options(options)
{
    Fountain::populateBody(scene, m_body, element);
}

Fountain::Writer::~Writer() { }

bool Fountain::Writer::write(const QString &fileName) const
{
    QFile file(fileName);
    return this->write(&file);
}

bool Fountain::Writer::write(QIODevice *device) const
{
    if (device == nullptr)
        return false;

    if (!device->isOpen()) {
        if (!device->open(QFile::WriteOnly))
            return false;
    }

    if (!device->isWritable())
        return false;

    const QByteArray bytes = this->toByteArray();

    bool success = device->write(bytes) == qint64(bytes.size());

    device->close();

    return success;
}

bool Fountain::Writer::writeInto(QString &text) const
{
    text = this->toString();
    return true;
}

bool Fountain::Writer::writeInto(QByteArray &text) const
{
    text = this->toByteArray();
    return true;
}

static const char *newline = "\n";
static const char *colon = ":";

/*
I could go on with the following too. But they are not referenced as much as newline and colon.

static const char *ampersat = "@";
static const char *bracketOpen = "(";
static const char *bracketClose = ")";
static const char *noteOpen = "[[";
static const char *noteClose = "]]";
static const char *pound = "#";
static const char *exclamation = "!";
static const char *greaterThan = ">";
static const char *lessThan = "<";
static const char *dot = ".";
static const char *singleSpace = " ";
*/

QString Fountain::Writer::toString() const
{
    QString ret;

    QTextStream ts(&ret, QIODevice::WriteOnly);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    for (const QPair<QString, QString> &item : m_titlePage) {
        if (item.second.contains(newline)) {
            ts << item.first << colon << newline;

            const QStringList lines = item.second.split(newline, Qt::SkipEmptyParts);
            for (const QString &line : lines)
                ts << "    " << line << newline;
        } else
            ts << item.first << ": " << item.second << newline;
    }

    if (!m_titlePage.isEmpty())
        ts << newline;

    for (const Fountain::Element &element : m_body) {
        switch (element.type) {
        case Fountain::Element::SceneHeading:
            writeSceneHeading(ts, element);
            break;
        case Fountain::Element::Action:
            writeAction(ts, element);
            break;
        case Fountain::Element::Character:
            writeCharacter(ts, element);
            break;
        case Fountain::Element::Dialogue:
            writeDialogue(ts, element);
            break;
        case Fountain::Element::Parenthetical:
            writeParenthetical(ts, element);
            break;
        case Fountain::Element::Shot:
        case Fountain::Element::Transition:
            writeShotOrTransition(ts, element);
            break;
        case Fountain::Element::PageBreak:
            if (m_options & StrictSyntaxOption)
                writePageBreak(ts, element);
            break;
        case Fountain::Element::Lyrics:
            if (m_options & StrictSyntaxOption)
                writeLyrics(ts, element);
            break;
        case Fountain::Element::LineBreak:
            writeLineBreak(ts, element);
            break;
        case Fountain::Element::Section:
            if (m_options & StrictSyntaxOption)
                writeSection(ts, element);
            break;
        case Fountain::Element::Synopsis:
            if (m_options & StrictSyntaxOption)
                writeSynopsis(ts, element);
            break;
        default:
            ts << "/* " << element.text << " */" << newline;
            break;
        }
    }

    ts.flush();

    static const QRegularExpression multipleNewlines("\\n{3,}");
    ret = ret.replace(multipleNewlines, "\n\n");

    return ret;
}

QByteArray Fountain::Writer::toByteArray() const
{
    const QString text = this->toString();
    return text.toUtf8();
}

void Fountain::Writer::writeSceneHeading(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#action
    // A Scene Heading always has at least one blank line preceding it.
    ts << newline;

    // You can force a Scene Heading by starting the line with a single period.
    if (m_options & StrictSyntaxOption)
        ts << ".";

    ts << this->emphasisedText(element).toUpper();

    // Scene Headings can optionally be appended with Scene Numbers. Scene numbers are any
    // alphanumerics (plus dashes and periods), wrapped in #.
    if (!element.sceneNumber.isEmpty())
        ts << " #" << element.sceneNumber << "#";

    ts << newline;

    // Second newline, thought not required, is added anyway to make the resulting file look good as
    // a plain text document also
    ts << newline;
}

void Fountain::Writer::writeAction(QTextStream &ts, const Element &element) const
{
    ts << newline;

    // http://fountain.io/syntax/#action

    // You can force an Action element can by preceding it with an exclamation point !.
    if (m_options & StrictSyntaxOption)
        ts << "!";

    // Centered text constitutes an Action element, and is bracketed with greater/less-than:
    if (element.isCentered)
        ts << ">";

    ts << this->emphasisedText(element);

    if (element.isCentered)
        ts << "<";

    // A Note is created by enclosing some text with double brackets. Notes can be inserted between
    // lines, or in the middle of a line.
    if (!element.notes.isEmpty()) {
        if (element.isCentered)
            ts << newline;

        for (const QString &note : element.notes)
            ts << "[[" << note << "]]";
    }

    ts << newline;
}

void Fountain::Writer::writeCharacter(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#character

    // A Character element is any line entirely in uppercase, with one empty line before it and
    // without an empty line after it.

    ts << newline;

    // You can force a Character element by preceding it with the “at” symbol @.
    if (m_options & StrictSyntaxOption)
        ts << "@";

    ts << this->emphasisedText(element).toUpper() << newline;
}

void Fountain::Writer::writeParenthetical(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#parenthetical

    // Parentheticals follow a Character or Dialogue element, and are wrapped in parentheses ().
    if (!element.text.startsWith('('))
        ts << "(";

    ts << this->emphasisedText(element);

    if (!element.text.endsWith(')'))
        ts << ")";

    ts << newline;
}

void Fountain::Writer::writeDialogue(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#dialogue

    // Dialogue is any text following a Character or Parenthetical element.

    ts << this->emphasisedText(element) << newline;
}

void Fountain::Writer::writeShotOrTransition(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#transition

    // The requirements for Transition elements are:
    //  - Uppercase
    //  - Preceded by and followed by an empty line
    //  - Ending in TO:

    // In Scrite: We don't need transitions to end with TO:
    // But we do, want them to end with :

    // You can force any line to be a transition by beginning it with a greater-than symbol >.

    ts << newline;

    if (element.type == Fountain::Element::Transition) {
        if (m_options & StrictSyntaxOption)
            ts << ">";
    }

    ts << this->emphasisedText(element).toUpper();

    if (!element.text.endsWith(':'))
        ts << ":";

    ts << newline;
}

void Fountain::Writer::writeLyrics(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#lyrics
    // You create a Lyric by starting with a line with a tilde ~.

    ts << "~" << this->emphasisedText(element) << newline;
}

void Fountain::Writer::writePageBreak(QTextStream &ts, const Element &element) const
{
    Q_UNUSED(element);

    // http://fountain.io/syntax/#page-breaks

    // Page Breaks are indicated by a line containing three or more consecutive equals signs, and
    // nothing more.

    ts << newline << "===" << newline;
}

void Fountain::Writer::writeLineBreak(QTextStream &ts, const Element &element) const
{
    Q_UNUSED(element);

    ts << newline;
}

void Fountain::Writer::writeSection(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#sections-synopses
    // Create a Section by preceding a line with one or more pound-sign # characters:

    ts << QString(element.sectionDepth, '#') << " " << this->emphasisedText(element) << newline;
}

void Fountain::Writer::writeSynopsis(QTextStream &ts, const Element &element) const
{
    // http://fountain.io/syntax/#sections-synopses
    // Synopses are single lines prefixed by an equals sign =. They can be located anywhere within
    // the screenplay.

    ts << "= " << this->emphasisedText(element) << newline;
}

QString Fountain::Writer::emphasisedText(const Element &element) const
{
    QString ret;
    if (m_options & EmphasisOption && Fountain::encodeEmphasis(element.text, element.formats, ret))
        return ret;

    return element.text;
}

#include "scene.h"
#include "structure.h"
#include "screenplay.h"

void Fountain::populateTitlePage(const Screenplay *screenplay, TitlePage &titlePage)
{
    using GetterType = QString (Screenplay::*)() const;
    const QList<QPair<QString, GetterType>> titlePageGetters = {
        { "Title", &Screenplay::title },
        { "Subtitle", &Screenplay::subtitle },
        { "Based On", &Screenplay::basedOn },
        { "Authors", &Screenplay::author },
        { "Contact", &Screenplay::contact },
        { "Address", &Screenplay::address },
        { "Phone Number", &Screenplay::phoneNumber },
        { "Website", &Screenplay::website },
        { "Email", &Screenplay::email },
        { "Logline", &Screenplay::logline }
    };

    for (const auto &titlePageGetter : titlePageGetters) {
        const QString value = ((*screenplay).*titlePageGetter.second)();
        if (!value.isEmpty())
            titlePage.append(qMakePair(titlePageGetter.first, value));
    }
}

void Fountain::populateBody(const Scene *scene, Body &body, const ScreenplayElement *element)
{
    if (scene == nullptr)
        return;

    const SceneHeading *heading = scene->heading();

    Fountain::Element fSceneHeading;
    fSceneHeading.type = Fountain::Element::SceneHeading;
    fSceneHeading.text = heading->isEnabled() ? heading->text() : QString();
    fSceneHeading.sceneNumber = element ? element->userSceneNumber() : QString();
    body.append(fSceneHeading);

    if (element && element->isOmitted()) {
        Fountain::Element fOmittedPara;
        fOmittedPara.type = Fountain::Element::Action;
        fOmittedPara.notes << "Omitted";
        body.append(fOmittedPara);
        return;
    }

    const int nrParas = scene->elementCount();
    for (int j = 0; j < nrParas; j++) {
        const SceneElement *para = scene->elementAt(j);

        Fountain::Element fPara;
        fPara.text = para->formattedText();
        fPara.formats = para->textFormats();

        switch (para->type()) {
        case SceneElement::Shot:
            fPara.type = Fountain::Element::Shot;
            break;
        case SceneElement::Transition:
            fPara.type = Fountain::Element::Transition;
            break;
        case SceneElement::Character:
            fPara.type = Fountain::Element::Character;
            break;
        case SceneElement::Action:
            fPara.type = Fountain::Element::Action;
            break;
        case SceneElement::Dialogue:
            fPara.type = Fountain::Element::Dialogue;
            break;
        case SceneElement::Parenthetical:
            fPara.type = Fountain::Element::Parenthetical;
            break;
        default:
            break;
        }

        if (fPara.type != fPara.None)
            body.append(fPara);
    }
}

void Fountain::populateBody(const Screenplay *screenplay, Body &body)
{
    if (screenplay == nullptr)
        return;

    const int nrElements = screenplay->elementCount();
    for (int i = 0; i < nrElements; i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);

        if (element->elementType() == ScreenplayElement::BreakElementType) {
            Fountain::Element fElement;
            fElement.type = Fountain::Element::Section;
            fElement.sectionDepth = element->breakType() == Screenplay::Act ? 1 : 2;
            fElement.text = (element->breakTitle() + " " + element->breakSubtitle()).simplified();
            body.append(fElement);

            if (!element->breakSummary().isEmpty()) {
                fElement = Fountain::Element();
                fElement.type = Fountain::Element::Synopsis;
                fElement.text = element->breakSummary();
                body.append(fElement);
            }

            continue;
        }

        Fountain::populateBody(element->scene(), body, element);
    }
}

void Fountain::populateBody(const ScreenplayElement *element, Body &body)
{
    if (element != nullptr)
        Fountain::populateBody(element->scene(), body, element);
}

void Fountain::loadTitlePage(const TitlePage &titlePage, Screenplay *screenplay)
{
    const QMap<QString, QString> keyValuePairs = [titlePage]() -> QMap<QString, QString> {
        QMap<QString, QString> ret;
        for (const auto &tpitem : titlePage)
            ret[tpitem.first] = tpitem.second;

        return ret;
    }();

    screenplay->setTitle(keyValuePairs.value("title"));
    screenplay->setSubtitle(keyValuePairs.value("subtitle"));
    screenplay->setLogline(keyValuePairs.value("logline"));
    screenplay->setBasedOn(keyValuePairs.value("basedon"));
    screenplay->setAuthor(keyValuePairs.value("authors"));
    screenplay->setContact(keyValuePairs.value("contact"));
    screenplay->setAddress(keyValuePairs.value("address"));
    screenplay->setPhoneNumber(keyValuePairs.value("phone"));
    screenplay->setEmail(keyValuePairs.value("email"));
    screenplay->setWebsite(keyValuePairs.value("website"));
    screenplay->setVersion(keyValuePairs.value("version"));
}

void Fountain::loadIntoScene(const Body &body, Scene *scene, ScreenplayElement *element)
{
    if (scene == nullptr)
        return;

    for (const Fountain::Element &fPara : body)
        Fountain::loadIntoScene(fPara, scene, element);
}

bool Fountain::loadIntoScene(const Element &fPara, Scene *scene, ScreenplayElement *element)
{
    if (scene == nullptr)
        return false;

    if (fPara.type == Fountain::Element::SceneHeading && scene->elementCount() == 0) {
        scene->heading()->parseFrom(fPara.text);
        scene->heading()->setEnabled(true);
        if (element && !fPara.sceneNumber.isEmpty())
            element->setUserSceneNumber(fPara.sceneNumber);
        return true;
    }

    if (fPara.type == Fountain::Element::Synopsis) {
        QString synopsis = scene->synopsis();
        if (!synopsis.isEmpty())
            synopsis += "\n\n";
        synopsis += fPara.text;
        scene->setSynopsis(synopsis);
        return true;
    }

    SceneElement *para = new SceneElement(scene);
    para->setText(fPara.text);
    para->setTextFormats(fPara.formats);
    if (fPara.isCentered)
        para->setAlignment(Qt::AlignHCenter);

    switch (fPara.type) {
    default:
    case Fountain::Element::Action:
        para->setType(SceneElement::Action);
        break;
    case Fountain::Element::Character:
        para->setType(SceneElement::Character);
        break;
    case Fountain::Element::Parenthetical:
        para->setType(SceneElement::Parenthetical);
        break;
    case Fountain::Element::Dialogue:
        para->setType(SceneElement::Dialogue);
        break;
    case Fountain::Element::Shot:
        para->setType(SceneElement::Shot);
        break;
    case Fountain::Element::Transition:
        para->setType(SceneElement::Transition);
        break;
    }

    scene->addElement(para);
    return true;
}
