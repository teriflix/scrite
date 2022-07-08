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

#ifndef CHARACTERREPORT_H
#define CHARACTERREPORT_H

#include "abstractreportgenerator.h"

class CharacterReport : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Character Report")
    Q_CLASSINFO("Description", "Generate a report with dialogues and notes for one or more selected characters.")

public:
    Q_INVOKABLE explicit CharacterReport(QObject *parent = nullptr);
    ~CharacterReport();

    bool requiresConfiguration() const { return true; }

    Q_CLASSINFO("characterNames_FieldGroup", "Characters")
    Q_CLASSINFO("characterNames_FieldLabel", "Characters to include in the report")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_CLASSINFO("includeSceneHeadings_FieldGroup", "Characters")
    Q_CLASSINFO("includeSceneHeadings_FieldLabel", "Include scene headings")
    Q_CLASSINFO("includeSceneHeadings_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneHeadings READ isIncludeSceneHeadings WRITE setIncludeSceneHeadings NOTIFY includeSceneHeadingsChanged)
    void setIncludeSceneHeadings(bool val);
    bool isIncludeSceneHeadings() const { return m_includeSceneHeadings; }
    Q_SIGNAL void includeSceneHeadingsChanged();

    Q_CLASSINFO("includeDialogues_FieldGroup", "Characters")
    Q_CLASSINFO("includeDialogues_FieldLabel", "Include dialogues")
    Q_CLASSINFO("includeDialogues_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeDialogues READ isIncludeDialogues WRITE setIncludeDialogues NOTIFY includeDialoguesChanged)
    void setIncludeDialogues(bool val);
    bool isIncludeDialogues() const { return m_includeDialogues; }
    Q_SIGNAL void includeDialoguesChanged();

    Q_CLASSINFO("includeNotes_FieldGroup", "Characters")
    Q_CLASSINFO("includeNotes_FieldLabel", "Include character notes")
    Q_CLASSINFO("includeNotes_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeNotes READ includeNotes WRITE setIncludeNotes NOTIFY includeNotesChanged)
    void setIncludeNotes(bool val);
    bool includeNotes() const { return m_includeNotes; }
    Q_SIGNAL void includeNotesChanged();

protected:
    // AbstractReportGenerator interface
    bool doGenerate(QTextDocument *textDocument);

private:
    bool m_includeNotes = false;
    bool m_includeDialogues = true;
    bool m_includeSceneHeadings = true;
    QStringList m_characterNames;
};

#endif // CHARACTERREPORT_H
