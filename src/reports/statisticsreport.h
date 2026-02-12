/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef STATISTICSREPORT_H
#define STATISTICSREPORT_H

#include <QTime>
#include <QList>
#include <QtMath>

#include "screenplaypaginator.h"
#include "abstractreportgenerator.h"

class StatisticsReportPage;
class ScreenplayTextDocument;
class StatisticsReportTimeline;
class StatisticsReportKeyNumbers;

class StatisticsReport : public AbstractReportGenerator
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Title", "Statistics Report")
    Q_CLASSINFO("Description", "Generate a report with key statistics of the screenplay.")
    Q_CLASSINFO("Icon", ":/icons/reports/statistics_report.png")
    // clang-format on

public:
    Q_INVOKABLE explicit StatisticsReport(QObject *parent = nullptr);
    ~StatisticsReport();

    enum ColorGroup { Character, Location, Beat, Act, Episode };
    static const QVector<QColor> colors(ColorGroup group = Character);
    static const QColor pickColor(int index, bool cycleAround = true, ColorGroup group = Character);
    static const QColor pickRandomColor(ColorGroup group);

    bool requiresConfiguration() const { return true; }
    bool isSinglePageReport() const { return true; }

    // clang-format off
    Q_CLASSINFO("includeCharacterPresenceGraphs_FieldGroup", "Basic")
    Q_CLASSINFO("includeCharacterPresenceGraphs_FieldLabel", "Include Character Presence Graphs.")
    Q_CLASSINFO("includeCharacterPresenceGraphs_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeCharacterPresenceGraphs
               READ isIncludeCharacterPresenceGraphs
               WRITE setIncludeCharacterPresenceGraphs
               NOTIFY includeCharacterPresenceGraphsChanged)
    // clang-format on
    void setIncludeCharacterPresenceGraphs(bool val);
    bool isIncludeCharacterPresenceGraphs() const { return m_includeCharacterPresenceGraphs; }
    Q_SIGNAL void includeCharacterPresenceGraphsChanged();

    // clang-format off
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldGroup", "Characters")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldLabel", "Maximum Number Of Characters")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldNote", "Value of ZERO, means you need all character's graphs plotted. If one or more " "characters are explicitly picked, then preference is given to them.")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldEditor", "IntegerSpinBox")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldMinValue", "0")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldMaxValue", "500")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldDefaultValue", "6")
    Q_PROPERTY(int maxCharacterPresenceGraphs
               READ maxCharacterPresenceGraphs
               WRITE setMaxCharacterPresenceGraphs
               NOTIFY maxCharacterPresenceGraphsChanged)
    // clang-format on
    void setMaxCharacterPresenceGraphs(int val);
    int maxCharacterPresenceGraphs() const { return m_maxCharacterPresenceGraphs; }
    Q_SIGNAL void maxCharacterPresenceGraphsChanged();

    // clang-format off
    Q_CLASSINFO("characterNames_FieldGroup", "Characters")
    Q_CLASSINFO("characterNames_FieldLabel", "Characters to include in the presence graphs.")
    Q_CLASSINFO("characterNames_FieldNote", "Leave it empty to let Scrite automatically pick from most active characters.")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_CLASSINFO("characterNames_IsPersistent", "false")
    Q_PROPERTY(QStringList characterNames
               READ characterNames
               WRITE setCharacterNames
               NOTIFY characterNamesChanged)
    // clang-format on
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    // clang-format off
    Q_CLASSINFO("includeLocationPresenceGraphs_FieldGroup", "Basic")
    Q_CLASSINFO("includeLocationPresenceGraphs_FieldLabel", "Include Location Presence Graphs.")
    Q_CLASSINFO("includeLocationPresenceGraphs_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeLocationPresenceGraphs
               READ isIncludeLocationPresenceGraphs
               WRITE setIncludeLocationPresenceGraphs
               NOTIFY includeLocationPresenceGraphsChanged)
    // clang-format on
    void setIncludeLocationPresenceGraphs(bool val);
    bool isIncludeLocationPresenceGraphs() const { return m_includeLocationPresenceGraphs; }
    Q_SIGNAL void includeLocationPresenceGraphsChanged();

    // clang-format off
    Q_CLASSINFO("considerPreferredGroupCategoryOnly_FieldGroup", "Basic")
    Q_CLASSINFO("considerPreferredGroupCategoryOnly_FieldLabel", "Consider Preferred Group Category Only.")
    Q_CLASSINFO("considerPreferredGroupCategoryOnly_FieldEditor", "CheckBox")
    Q_PROPERTY(bool considerPreferredGroupCategoryOnly
               READ isConsiderPreferredGroupCategoryOnly
               WRITE setConsiderPreferredGroupCategoryOnly
               NOTIFY considerPreferredGroupCategoryOnlyChanged)
    // clang-format on
    void setConsiderPreferredGroupCategoryOnly(bool val);
    bool isConsiderPreferredGroupCategoryOnly() const;
    Q_SIGNAL void considerPreferredGroupCategoryOnlyChanged();

    // clang-format off
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldGroup", "Locations")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldLabel", "Maximum Number Of Locations")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldNote", "Value of ZERO, means you need all location's graphs plotted. If one or more " "locations are explicitly picked, then preference is given to them.")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldEditor", "IntegerSpinBox")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldMinValue", "0")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldMaxValue", "500")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldDefaultValue", "6")
    Q_PROPERTY(int maxLocationPresenceGraphs
               READ maxLocationPresenceGraphs
               WRITE setMaxLocationPresenceGraphs
               NOTIFY maxLocationPresenceGraphsChanged)
    // clang-format on
    void setMaxLocationPresenceGraphs(int val);
    int maxLocationPresenceGraphs() const { return m_maxLocationPresenceGraphs; }
    Q_SIGNAL void maxLocationPresenceGraphsChanged();

    // clang-format off
    Q_CLASSINFO("locations_FieldGroup", "Locations")
    Q_CLASSINFO("locations_FieldLabel", "Locations to include in the presence graphs.")
    Q_CLASSINFO("locations_FieldNote", "Leave it empty to let Scrite automatically pick from most busy locations.")
    Q_CLASSINFO("locations_FieldEditor", "MultipleLocationSelector")
    Q_CLASSINFO("locations_IsPersistent", "false")
    Q_PROPERTY(QStringList locations
               READ locations
               WRITE setLocations
               NOTIFY locationsChanged)
    // clang-format on
    void setLocations(const QStringList &val);
    QStringList locations() const { return m_locations; }
    Q_SIGNAL void locationsChanged();

    struct Distribution
    {
        QString key;
        int count = 0;
        qreal ratio = 0;
        QString percent;
        QTime timeLength;
        qreal pixelLength = 0;
        qreal pageLength = 0;
        QColor color = Qt::transparent;
    };

    // Distribution of Action, Dialogue and Heading paragraphs
    QList<Distribution> textDistribution(bool compact = true) const;

    // Distribution of Character Dialogues
    QList<Distribution> dialogueDistribution() const;

    // Distribution of Scene lengths
    QList<Distribution> sceneDistribution() const;

    // Distrubution of Act lengths
    QList<Distribution> actDistribution() const;

    // Distribution of Episode lenghts
    QList<Distribution> episodeDistribution() const;

    qreal totalPixelLength() const { return this->pixelLength(); }
    int pageCount() const { return qCeil(this->pageLength()); }
    QTime estimatedTime() const { return this->timeLength(); }

protected:
    // AbstractReportGenerator interface
    bool doGenerate(QTextDocument *textDocument);

    bool canDirectPrintToPdf() const;
    bool usePdfWriter() const;
    bool directPrintToPdf(QPdfWriter *);

private:
    friend class StatisticsReportTimeline;
    friend class StatisticsReportKeyNumbers;

    void prepareTextDocument();
    void cleanupTextDocument();

    qreal pageHeight() const { return m_textDocument.pageSize().height(); }

    QTime timeLength() const
    {
        return ScreenplayPaginator::pixelToTimeLength(
                this->pixelLength(), this->document()->printFormat(), &m_textDocument);
    }
    qreal pageLength() const
    {
        return ScreenplayPaginator::pixelToPageLength(this->pixelLength(), &m_textDocument);
    }
    qreal pixelLength() const { return ScreenplayPaginator::pixelLength(&m_textDocument); }
    qreal pageLength(qreal pixelLength) const
    {
        return ScreenplayPaginator::pixelToPageLength(pixelLength, &m_textDocument);
    }
    QTime pixelLengthToTime(qreal val) const
    {
        return ScreenplayPaginator::pixelToTimeLength(val, this->document()->printFormat(),
                                                      &m_textDocument);
    }

    template<class T>
    QTime timeLength(const T *obj) const
    {
        return ScreenplayPaginator::pixelToTimeLength(
                this->pixelLength(obj), this->document()->printFormat(), &m_textDocument);
    }

    template<class T>
    qreal pageLength(const T *obj) const
    {
        return ScreenplayPaginator::pixelToPageLength(this->pixelLength(obj), &m_textDocument);
    }

    template<class T>
    qreal pixelLength(const T *obj) const
    {
        return ScreenplayPaginator::pixelLength(obj, &m_textDocument);
    }

    void polish(Distribution &distribution) const;
    void normalizeRatios(QList<Distribution> &distributions) const;

private:
    QTextDocument m_textDocument;
    int m_maxLocationPresenceGraphs = 6;
    int m_maxCharacterPresenceGraphs = 6;
    QStringList m_locations;
    QStringList m_characterNames;
    bool m_includeLocationPresenceGraphs = true;
    bool m_includeCharacterPresenceGraphs = true;
    bool m_considerPreferredGroupCategoryOnly = true;
};

#endif // STATISTICSREPORT_H
