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

#ifndef SCREENPLAYSUBSETREPORT_H
#define SCREENPLAYSUBSETREPORT_H

#include "abstractscreenplaysubsetreport.h"

class ScreenplaySubsetReport : public AbstractScreenplaySubsetReport
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Screenplay Subset")

public:
    Q_INVOKABLE ScreenplaySubsetReport(QObject *parent=nullptr);
    ~ScreenplaySubsetReport();

    Q_CLASSINFO("sceneNumbers_FieldLabel", "Locations")
    Q_CLASSINFO("sceneNumbers_FieldEditor", "MultipleSceneSelector")
    Q_PROPERTY(QList<int> sceneNumbers READ sceneNumbers WRITE setSceneNumbers NOTIFY sceneNumbersChanged)
    void setSceneNumbers(const QList<int> &val);
    QList<int> sceneNumbers() const { return m_sceneNumbers; }
    Q_SIGNAL void sceneNumbersChanged();

protected:
    // AbstractScreenplaySubsetReport interface
    bool includeScreenplayElement(const ScreenplayElement *) const;
    QString screenplaySubtitle() const;
    void configureScreenplayTextDocument(ScreenplayTextDocument &stDoc);

private:
    QList<int> m_sceneNumbers;
};

#endif // SCREENPLAYSUBSETREPORT_H
