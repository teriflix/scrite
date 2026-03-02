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

#ifndef LOCATIONSCREENPLAYREPORT_H
#define LOCATIONSCREENPLAYREPORT_H

#include "abstractscreenplaysubsetreport.h"

class LocationScreenplayReport : public AbstractScreenplaySubsetReport
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Title", "Location Screenplay")
    Q_CLASSINFO("Description", "Generate screenplay with only those scenes at one or more locations.")
    // clang-format on

public:
    Q_INVOKABLE explicit LocationScreenplayReport(QObject *parent = nullptr);
    ~LocationScreenplayReport();

    // clang-format off
    Q_CLASSINFO("locations_FieldGroup", "Locations")
    Q_CLASSINFO("locations_FieldLabel", "Locations to include in the report")
    Q_CLASSINFO("locations_FieldEditor", "MultipleLocationSelector")
    Q_PROPERTY(QStringList locations
               READ locations
               WRITE setLocations
               NOTIFY locationsChanged)
    // clang-format on
    void setLocations(const QStringList &val);
    QStringList locations() const { return m_locations; }
    Q_SIGNAL void locationsChanged();

    // clang-format off
    Q_CLASSINFO("generateSummary_FieldGroup", "PDF Options")
    Q_CLASSINFO("generateSummary_FieldLabel", "Generate summary")
    Q_CLASSINFO("generateSummary_FieldEditor", "CheckBox")
    Q_PROPERTY(bool generateSummary
               READ generateSummary
               WRITE setGenerateSummary
               NOTIFY generateSummaryChanged)
    // clang-format on
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
    int m_summaryLocation = -1;
    QStringList m_locations;
    bool m_generateSummary = true;
    mutable QMap<QString, QList<const ScreenplayElement *>> m_locationSceneNumberList;
};

#endif // LOCATIONSCREENPLAYREPORT_H
