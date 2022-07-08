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

#ifndef CHARACTERSCREENPLAYREPORT_H
#define CHARACTERSCREENPLAYREPORT_H

#include "abstractscreenplaysubsetreport.h"

class CharacterScreenplayReport : public AbstractScreenplaySubsetReport
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Character Screenplay")
    Q_CLASSINFO("Description", "Generate screenplay with only those scenes where one or more characters are present.")

public:
    Q_INVOKABLE explicit CharacterScreenplayReport(QObject *parent = nullptr);
    ~CharacterScreenplayReport();

    Q_CLASSINFO("highlightDialogues_FieldGroup", "Characters")
    Q_CLASSINFO("highlightDialogues_FieldLabel", "Highlight dialogues in yellow background")
    Q_CLASSINFO("highlightDialogues_FieldEditor", "CheckBox")
    Q_PROPERTY(bool highlightDialogues READ isHighlightDialogues WRITE setHighlightDialogues NOTIFY highlightDialoguesChanged)
    void setHighlightDialogues(bool val);
    bool isHighlightDialogues() const { return m_highlightDialogues; }
    Q_SIGNAL void highlightDialoguesChanged();

    Q_CLASSINFO("characterNames_FieldGroup", "Characters")
    Q_CLASSINFO("characterNames_FieldLabel", "Characters to include in the report")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

protected:
    // AbstractScreenplaySubsetReport interface
    bool includeScreenplayElement(const ScreenplayElement *) const;
    QString screenplaySubtitle() const;
    void configureScreenplayTextDocument(ScreenplayTextDocument &stDoc);

private:
    QString m_comment;
    QString m_watermark;
    bool m_includeNotes = false;
    QStringList m_characterNames;
    bool m_includeSceneIcons = true;
    bool m_highlightDialogues = true;
    bool m_includeSceneNumbers = true;
    bool m_printEachSceneOnANewPage = false;
};

#endif // CHARACTERSCREENPLAYREPORT_H
