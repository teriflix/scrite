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

#include "characterscreenplayreport.h"
#include "screenplaytextdocument.h"

CharacterScreenplayReport::CharacterScreenplayReport(QObject *parent)
    : AbstractScreenplaySubsetReport(parent)
{
}

CharacterScreenplayReport::~CharacterScreenplayReport() { }

void CharacterScreenplayReport::setIncludeNotes(bool val)
{
    if (m_includeNotes == val)
        return;

    m_includeNotes = val;
    emit includeNotesChanged();
}

void CharacterScreenplayReport::setHighlightDialogues(bool val)
{
    if (m_highlightDialogues == val)
        return;

    m_highlightDialogues = val;
    emit highlightDialoguesChanged();
}

void CharacterScreenplayReport::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

bool CharacterScreenplayReport::includeScreenplayElement(const ScreenplayElement *element) const
{
    const Scene *scene = element->scene();
    if (scene == nullptr)
        return false;

    if (m_characterNames.isEmpty())
        return true;

    const QStringList sceneCharacters = scene->characterNames();
    for (const QString &characterName : qAsConst(m_characterNames))
        if (sceneCharacters.contains(characterName))
            return true;

    return false;
}

QString CharacterScreenplayReport::screenplaySubtitle() const
{
    if (m_characterNames.isEmpty())
        return QStringLiteral("Character Screenplay Of: ALL CHARACTERS");

    QString subtitle = m_characterNames.join(", ") + QStringLiteral(": Character Screenplay");
    if (subtitle.length() > 60)
        return QStringLiteral("Character Screenplay of") + m_characterNames.first()
                + QStringLiteral(" and ") + QString::number(m_characterNames.size() - 1)
                + QStringLiteral(" other characters(s).");

    return subtitle;
}

void CharacterScreenplayReport::configureScreenplayTextDocument(ScreenplayTextDocument &stDoc)
{
    if (m_highlightDialogues)
        stDoc.setHighlightDialoguesOf(m_characterNames);
}

void CharacterScreenplayReport::inject(
        QTextCursor &cursor,
        AbstractScreenplayTextDocumentInjectionInterface::InjectLocation location)
{
    AbstractScreenplaySubsetReport::inject(cursor, location);

    if (location != AfterTitlePage || !m_includeNotes)
        return;

    const QFont defaultFont = this->document()->printFormat()->defaultFont();

    QTextBlockFormat defaultBlockFormat;

    QTextCharFormat defaultCharFormat;
    defaultCharFormat.setFontFamily(defaultFont.family());
    defaultCharFormat.setFontPointSize(12);

    QTextBlockFormat blockFormat = defaultBlockFormat;
    blockFormat.setAlignment(Qt::AlignLeft);
    blockFormat.setTopMargin(20);

    QTextCharFormat charFormat = defaultCharFormat;
    charFormat.setFontPointSize(20);
    charFormat.setFontCapitalization(QFont::AllUppercase);
    charFormat.setFontWeight(QFont::Bold);
    charFormat.setFontItalic(true);

    cursor.insertBlock(blockFormat, charFormat);
    cursor.insertText("NOTES:");

    const Structure *structure = this->document()->structure();
    for (const QString &characterName : qAsConst(m_characterNames)) {
        blockFormat = defaultBlockFormat;
        blockFormat.setIndent(1);

        charFormat = defaultCharFormat;
        charFormat.setFontWeight(QFont::Bold);

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText(characterName);

        charFormat.setFontWeight(QFont::Normal);
        cursor.setCharFormat(charFormat);

        const Character *character = structure->findCharacter(characterName);
        const Notes *characterNotes = character ? character->notes() : nullptr;
        if (characterNotes == nullptr || characterNotes->noteCount() == 0) {
            cursor.insertText(": no notes available.");
            continue;
        }

        cursor.insertText(": " + QString::number(characterNotes->noteCount())
                          + " note(s) available.");

        for (int i = 0; i < characterNotes->noteCount(); i++) {
            const Note *note = characterNotes->noteAt(i);
            QString heading = note->title().trimmed();
            if (heading.isEmpty())
                heading = "Note #" + QString::number(i + 1);

            blockFormat = defaultBlockFormat;
            blockFormat.setIndent(2);
            blockFormat.setTopMargin(10);

            charFormat = defaultCharFormat;
            charFormat.setFontWeight(QFont::Bold);

            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(heading);

            blockFormat.setTopMargin(0);
            if (i == characterNotes->noteCount() - 1 || characterName != m_characterNames.last())
                blockFormat.setBottomMargin(10);
            blockFormat.setAlignment(Qt::AlignJustify);

            charFormat.setFontWeight(QFont::Normal);
            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(note->content().toString());
        }
    }

    blockFormat = defaultBlockFormat;
    blockFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);

    charFormat = defaultCharFormat;

    cursor.insertBlock(blockFormat, charFormat);
    cursor.insertText(QStringLiteral("-- end of notes --"));
}
