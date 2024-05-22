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
                       QString line2 = line;
                       line2.remove("\n");

                       // Remove leading and trailing white spaces, if that option is turned on
                       static const QRegularExpression leadingWhitespaceRegex("^\\s+");
                       static const QRegularExpression trailingWhitespaceRegex("\\s+$");
                       if (m_options & IgnoreLeadingWhitespaceOption)
                           line2 = line2.remove(leadingWhitespaceRegex);
                       if (m_options & IgnoreTrailingWhiteSpaceOption)
                           line2 = line2.remove(trailingWhitespaceRegex);

                       Fountain::Element element;
                       element.type = line2.isEmpty() ? Fountain::Element::LineBreak
                               : isPageBreak(line2)   ? Fountain::Element::PageBreak
                                                      : Fountain::Element::Unknown;
                       if (element.type == Fountain::Element::Unknown)
                           element.text = line;
                       return element;
                   });

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

        const QString trimmedText = element.text.trimmed();
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
            sceneHeading = sceneHeading.simplified();

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
        if (element.text.startsWith('.') && element.text.length() >= 2
            && element.text.at(1) != QChar('.')) {
            element.text = element.text.mid(1).toUpper().simplified();
            element.sceneNumber = extractSceneNumber(element.text);
            element.type = Fountain::Element::SceneHeading;
            continue;
        }

        const bool nextLineIsEmpty =
                ((i + 1) < m_body.size() && m_body.at(i + 1).type == Fountain::Element::LineBreak);
        const bool prevLineIsEmpty =
                i == 0 || m_body.at(i - 1).type == Fountain::Element::LineBreak;

        if (nextLineIsEmpty && prevLineIsEmpty) {
            // Otherwise it should begin with one of the following
            // INT, EXT, EST, INT./EXT, INT/EXT, I/E
            const QStringList prefixes = Fountain::sceneHeadingPrefixes();
            const QString simplifiedText = element.text.simplified();
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

        const bool nextLineIsEmpty =
                ((i + 1) < m_body.size() && m_body.at(i + 1).type == Fountain::Element::LineBreak);
        const bool prevLineIsEmpty =
                i == 0 || m_body.at(i - 1).type == Fountain::Element::LineBreak;

        if (element.text.startsWith('>') && !element.text.endsWith('<')) {
            element.text = element.text.mid(1).toUpper().simplified();
            element.text = Fountain::Element::Transition;
            continue;
        }

        if (prevLineIsEmpty && nextLineIsEmpty && element.text.toUpper() == element.text) {
            const QString simplifiedText = element.text.simplified();
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
                if (simplifiedText == knownTransition || simplifiedText == knownTransition + ":") {
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
                if (simplifiedText == knownShot || simplifiedText == knownShot + ":") {
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

        const bool nextLineIsEmpty =
                ((i + 1) < m_body.size() && m_body.at(i + 1).type == Fountain::Element::LineBreak);
        const bool prevLineIsEmpty =
                i == 0 || m_body.at(i - 1).type == Fountain::Element::LineBreak;

        if (prevLineIsEmpty && !nextLineIsEmpty) {
            const QString simplifiedText = element.text.simplified();
            if (simplifiedText.endsWith('.') || simplifiedText.endsWith(':')
                || simplifiedText.startsWith('>') || simplifiedText.endsWith('<'))
                continue;

            if (simplifiedText.startsWith('@')) {
                element.type = Fountain::Element::Character;
                element.text = simplifiedText;
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
                isCharacter = simplifiedText.toUpper() == simplifiedText;
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

            const QString trimmedText = dpElement.text.simplified();
            dpElement.text = trimmedText;

            if (trimmedText.startsWith('(')) {
                ++nrParentheticals;

                dpElement.type = Fountain::Element::Parenthetical;
                if (trimmedText.endsWith(')'))
                    --nrParentheticals;
            } else {
                if (nrParentheticals > 0) {
                    dpElement.type = Fountain::Element::Parenthetical;
                    if (trimmedText.endsWith(')'))
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

        const QString trimmedText = element.text.trimmed();

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

        QString trimmedText = element.text.trimmed();

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
            QString trimmedText = element.text.trimmed();
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
        for (int i = m_body.size() - 1; i >= 1; i--) {
            Fountain::Element &current = m_body[i];
            Fountain::Element &previous = m_body[i - 1];
            if (current.type == previous.type) {
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

static QStringList Fountain::sceneHeadingPrefixes()
{
    return { "INT", "EXT", "EST", "INT./EXT", "INT/EXT", "I/E" };
}
