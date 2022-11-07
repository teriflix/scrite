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

#ifndef SCREENPLAYSUBSETREPORT_H
#define SCREENPLAYSUBSETREPORT_H

#include "abstractscreenplaysubsetreport.h"

class ScreenplaySubsetReport : public AbstractScreenplaySubsetReport
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Screenplay Subset")
    Q_CLASSINFO("Description", "Generate screenplay with only a subset of scenes selected by you.")

public:
    Q_INVOKABLE explicit ScreenplaySubsetReport(QObject *parent = nullptr);
    ~ScreenplaySubsetReport();

    Q_CLASSINFO("sceneNumbers_FieldGroup", "Scenes")
    Q_CLASSINFO("sceneNumbers_FieldLabel", "Scenes to include in the report")
    Q_CLASSINFO("sceneNumbers_FieldEditor", "MultipleSceneSelector")
    Q_CLASSINFO("sceneNumbers_FieldNote",
                "If no scenes are selected, then the report is generted for all scenes in the "
                "screenplay.")
    Q_PROPERTY(QList<int> sceneNumbers READ sceneNumbers WRITE setSceneNumbers NOTIFY
                       sceneNumbersChanged)
    void setSceneNumbers(const QList<int> &val);
    QList<int> sceneNumbers() const { return m_sceneNumbers; }
    Q_SIGNAL void sceneNumbersChanged();

    Q_CLASSINFO("generateSummary_FieldGroup", "Options")
    Q_CLASSINFO("generateSummary_FieldLabel", "Generate summary")
    Q_CLASSINFO("generateSummary_FieldEditor", "CheckBox")
    Q_PROPERTY(bool generateSummary READ generateSummary WRITE setGenerateSummary NOTIFY
                       generateSummaryChanged)
    void setGenerateSummary(bool val);
    bool generateSummary() const { return m_generateSummary; }
    Q_SIGNAL void generateSummaryChanged();

protected:
    // AbstractScreenplaySubsetReport interface
    bool includeScreenplayElement(const ScreenplayElement *) const;
    QString screenplaySubtitle() const;
    void configureScreenplayTextDocument(ScreenplayTextDocument &stDoc);

    // AbstractScreenplayTextDocumentInjectionInterface interface
    void inject(QTextCursor &, InjectLocation);

private:
    QList<int> m_sceneNumbers;
    bool m_generateSummary = true;
};

#endif // SCREENPLAYSUBSETREPORT_H
