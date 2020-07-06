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

#ifndef CHARACTERSCREENPLAYREPORT_H
#define CHARACTERSCREENPLAYREPORT_H

#include "abstractscreenplaysubsetreport.h"

class CharacterScreenplayReport : public AbstractScreenplaySubsetReport
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Character Screenplay")

public:
    Q_INVOKABLE CharacterScreenplayReport(QObject *parent=nullptr);
    ~CharacterScreenplayReport();

    Q_CLASSINFO("includeNotes_FieldLabel", "Include character notes")
    Q_CLASSINFO("includeNotes_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeNotes READ includeNotes WRITE setIncludeNotes NOTIFY includeNotesChanged)
    void setIncludeNotes(bool val);
    bool includeNotes() const { return m_includeNotes; }
    Q_SIGNAL void includeNotesChanged();

    Q_CLASSINFO("characterNames_FieldLabel", "Character names")
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

    // AbstractScreenplayTextDocumentInjectionInterface interface
    void inject(QTextCursor &, InjectLocation);

private:
    QString m_comment;
    QString m_watermark;
    bool m_includeNotes;
    QStringList m_characterNames;
    bool m_includeSceneIcons = true;
    bool m_includeSceneNumbers = true;
    bool m_printEachSceneOnANewPage = false;    
};

#endif // CHARACTERSCREENPLAYREPORT_H
