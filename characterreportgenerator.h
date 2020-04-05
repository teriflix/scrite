/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CHARACTERREPORTGENERATOR_H
#define CHARACTERREPORTGENERATOR_H

#include "abstractreportgenerator.h"

class CharacterReportGenerator : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Character Report")

public:
    Q_INVOKABLE CharacterReportGenerator(QObject *parent=nullptr);
    ~CharacterReportGenerator();

    bool requiresConfiguration() const { return true; }

    Q_CLASSINFO("characterNames_FieldLabel", "Select one or more characters")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_CLASSINFO("includeSceneHeadings_FieldLabel", "Include scene headings")
    Q_CLASSINFO("includeSceneHeadings_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneHeadings READ isIncludeSceneHeadings WRITE setIncludeSceneHeadings NOTIFY includeSceneHeadingsChanged)
    void setIncludeSceneHeadings(bool val);
    bool isIncludeSceneHeadings() const { return m_includeSceneHeadings; }
    Q_SIGNAL void includeSceneHeadingsChanged();

    Q_CLASSINFO("includeDialogues_FieldLabel", "Include dialogues")
    Q_CLASSINFO("includeDialogues_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeDialogues READ isIncludeDialogues WRITE setIncludeDialogues NOTIFY includeDialoguesChanged)
    void setIncludeDialogues(bool val);
    bool isIncludeDialogues() const { return m_includeDialogues; }
    Q_SIGNAL void includeDialoguesChanged();

protected:
    // AbstractReportGenerator interface
    bool doGenerate(QTextDocument *textDocument);

private:
    bool m_includeDialogues;
    bool m_includeSceneHeadings;
    QStringList m_characterNames;

};

#endif // CHARACTERREPORTGENERATOR_H
