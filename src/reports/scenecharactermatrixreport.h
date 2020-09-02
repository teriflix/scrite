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

#ifndef SCENECHARACTERMATRIXREPORT_H
#define SCENECHARACTERMATRIXREPORT_H

#include "abstractreportgenerator.h"

class SceneCharacterMatrixReport : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Scene Character Matrix")
    Q_CLASSINFO("Description", "Generate a table of scene names and characters.")

public:
    Q_INVOKABLE SceneCharacterMatrixReport(QObject *parent=nullptr);
    ~SceneCharacterMatrixReport();

    bool requiresConfiguration() const { return true; }

    enum Type
    {
        SceneVsCharacter,
        CharacterVsScene
    };
    Q_ENUM(Type)
    Q_CLASSINFO("Type_CharacterVsScene", "Character vs Scene")
    Q_CLASSINFO("Type_SceneVsCharacter", "Scene Vs Character")

    Q_CLASSINFO("type_FieldGroup", "Characters")
    Q_CLASSINFO("type_FieldLabel", "Select type of matrix to generate")
    Q_CLASSINFO("type_FieldEditor", "EnumSelector")
    Q_CLASSINFO("type_FieldEnum", "Type")
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)
    void setType(int val);
    int type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_CLASSINFO("characterNames_FieldGroup", "Characters")
    Q_CLASSINFO("characterNames_FieldLabel", "Characters to include in the report")
    Q_CLASSINFO("characterNames_FieldNote", "If no characters are selected, then the report is generted for all characters in the screenplay.")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

protected:
    // AbstractReportGenerator interface
    bool supportsFormat(Format) const;
    bool doGenerate(QTextDocument *document);
    void configureWriter(QPdfWriter *pdfWriter, const QTextDocument *document) const;

private:
    QStringList m_characterNames;
    int m_type = SceneVsCharacter;
};

#endif // SCENECHARACTERMATRIXREPORT_H
