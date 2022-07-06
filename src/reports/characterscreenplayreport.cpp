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

#include "characterscreenplayreport.h"
#include "screenplaytextdocument.h"

CharacterScreenplayReport::CharacterScreenplayReport(QObject *parent)
    : AbstractScreenplaySubsetReport(parent)
{
}

CharacterScreenplayReport::~CharacterScreenplayReport() { }

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
