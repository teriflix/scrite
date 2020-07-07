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
    :AbstractScreenplaySubsetReport(parent)
{

}

CharacterScreenplayReport::~CharacterScreenplayReport()
{

}

void CharacterScreenplayReport::setIncludeNotes(bool val)
{
    if(m_includeNotes == val)
        return;

    m_includeNotes = val;
    emit includeNotesChanged();
}

void CharacterScreenplayReport::setCharacterNames(const QStringList &val)
{
    if(m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

bool CharacterScreenplayReport::includeScreenplayElement(const ScreenplayElement *element) const
{
    const Scene *scene = element->scene();
    if(scene == nullptr)
        return false;

    if(m_characterNames.isEmpty())
        return true;

    const QStringList sceneCharacters = scene->characterNames();
    Q_FOREACH(QString characterName, m_characterNames)
        if(sceneCharacters.contains(characterName))
            return true;

    return false;
}

QString CharacterScreenplayReport::screenplaySubtitle() const
{
    if(m_characterNames.isEmpty())
        return QStringLiteral("Location Screenplay of: ALL CHARACTERS");

    const QString subtitle = QStringLiteral("Character Screenplay Of: ") + m_characterNames.join(", ");
    if(subtitle.length() > 60)
        return  m_characterNames.first() + QStringLiteral(" and ") +
                QString::number(m_characterNames.size()-1) + QStringLiteral(" other characters(s).");

    return subtitle;
}

void CharacterScreenplayReport::configureScreenplayTextDocument(ScreenplayTextDocument &stDoc)
{
    stDoc.setHighlightDialoguesOf(m_characterNames);
}

void CharacterScreenplayReport::inject(QTextCursor &cursor, AbstractScreenplayTextDocumentInjectionInterface::InjectLocation location)
{
    if(location != AfterTitlePage || !m_includeNotes)
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
    Q_FOREACH(QString characterName, m_characterNames)
    {
        blockFormat = defaultBlockFormat;
        blockFormat.setIndent(1);

        charFormat = defaultCharFormat;
        charFormat.setFontWeight(QFont::Bold);

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText(characterName);

        charFormat.setFontWeight(QFont::Normal);
        cursor.setCharFormat(charFormat);

        const Character *character = structure->findCharacter(characterName);
        if(character == nullptr || character->noteCount() == 0)
        {
            cursor.insertText(": no notes available.");
            continue;
        }

        cursor.insertText(": " + QString::number(character->noteCount()) + " note(s) available.");

        for(int i=0; i<character->noteCount(); i++)
        {
            const Note *note = character->noteAt(i);
            QString heading = note->heading().trimmed();
            if(heading.isEmpty())
                heading = "Note #" + QString::number(i+1);

            blockFormat = defaultBlockFormat;
            blockFormat.setIndent(2);
            blockFormat.setTopMargin(10);

            charFormat = defaultCharFormat;
            charFormat.setFontWeight(QFont::Bold);

            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(heading);

            blockFormat.setTopMargin(0);
            if(i == character->noteCount()-1 || characterName != m_characterNames.last())
                blockFormat.setBottomMargin(10);
            blockFormat.setAlignment(Qt::AlignJustify);

            charFormat.setFontWeight(QFont::Normal);
            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(note->content());
        }
    }

    blockFormat = defaultBlockFormat;
    blockFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);

    charFormat = defaultCharFormat;

    cursor.insertBlock(blockFormat, charFormat);
    cursor.insertText(QStringLiteral("-- end of notes --"));
}
