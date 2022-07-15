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

#ifndef STATISTICSREPORT_H
#define STATISTICSREPORT_H

#include <QTime>
#include <QList>
#include <QtMath>

#include "abstractreportgenerator.h"

class StatisticsReportPage;
class ScreenplayTextDocument;
class StatisticsReportTimeline;
class StatisticsReportKeyNumbers;

class StatisticsReport : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Statistics Report")
    Q_CLASSINFO("Description", "Generate a report with key statistics of the screenplay.")

public:
    Q_INVOKABLE explicit StatisticsReport(QObject *parent = nullptr);
    ~StatisticsReport();

    enum ColorGroup { Character, Location, Beat };
    static const QVector<QColor> colors(ColorGroup group = Character);
    static const QColor pickColor(int index, bool cycleAround = true, ColorGroup group = Character);

    bool requiresConfiguration() const { return true; }
    bool isSinglePageReport() const { return true; }

    Q_CLASSINFO("includeCharacterPresenceGraphs_FieldGroup", "Basic")
    Q_CLASSINFO("includeCharacterPresenceGraphs_FieldLabel", "Include Character Presence Graphs.")
    Q_CLASSINFO("includeCharacterPresenceGraphs_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeCharacterPresenceGraphs READ isIncludeCharacterPresenceGraphs WRITE setIncludeCharacterPresenceGraphs NOTIFY includeCharacterPresenceGraphsChanged)
    void setIncludeCharacterPresenceGraphs(bool val);
    bool isIncludeCharacterPresenceGraphs() const { return m_includeCharacterPresenceGraphs; }
    Q_SIGNAL void includeCharacterPresenceGraphsChanged();

    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldGroup", "Characters")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldLabel", "Maximum Number Of Characters")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldNote", "Value of ZERO, means you need all character's graphs plotted. If one or more characters are explicitly picked, then preference is given to them.")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldEditor", "IntegerSpinBox")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldMinValue", "0")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldMaxValue", "500")
    Q_CLASSINFO("maxCharacterPresenceGraphs_FieldDefaultValue", "6")
    Q_PROPERTY(int maxCharacterPresenceGraphs READ maxCharacterPresenceGraphs WRITE setMaxCharacterPresenceGraphs NOTIFY maxCharacterPresenceGraphsChanged)
    void setMaxCharacterPresenceGraphs(int val);
    int maxCharacterPresenceGraphs() const { return m_maxCharacterPresenceGraphs; }
    Q_SIGNAL void maxCharacterPresenceGraphsChanged();

    Q_CLASSINFO("characterNames_FieldGroup", "Characters")
    Q_CLASSINFO("characterNames_FieldLabel", "Characters to include in the presence graphs.")
    Q_CLASSINFO("characterNames_FieldNote", "Leave it empty to let Scrite automatically pick from most active characters.")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_CLASSINFO("includeLocationPresenceGraphs_FieldGroup", "Basic")
    Q_CLASSINFO("includeLocationPresenceGraphs_FieldLabel", "Include Location Presence Graphs.")
    Q_CLASSINFO("includeLocationPresenceGraphs_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeLocationPresenceGraphs READ isIncludeLocationPresenceGraphs WRITE setIncludeLocationPresenceGraphs NOTIFY includeLocationPresenceGraphsChanged)
    void setIncludeLocationPresenceGraphs(bool val);
    bool isIncludeLocationPresenceGraphs() const { return m_includeLocationPresenceGraphs; }
    Q_SIGNAL void includeLocationPresenceGraphsChanged();

    Q_CLASSINFO("considerPreferredGroupCategoryOnly_FieldGroup", "Basic")
    Q_CLASSINFO("considerPreferredGroupCategoryOnly_FieldLabel", "Consider Preferred Group Category Only.")
    Q_CLASSINFO("considerPreferredGroupCategoryOnly_FieldEditor", "CheckBox")
    Q_PROPERTY(bool considerPreferredGroupCategoryOnly READ isConsiderPreferredGroupCategoryOnly WRITE setConsiderPreferredGroupCategoryOnly NOTIFY considerPreferredGroupCategoryOnlyChanged)
    void setConsiderPreferredGroupCategoryOnly(bool val);
    bool isConsiderPreferredGroupCategoryOnly() const;
    Q_SIGNAL void considerPreferredGroupCategoryOnlyChanged();

    Q_CLASSINFO("maxLocationPresenceGraphs_FieldGroup", "Locations")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldLabel", "Maximum Number Of Locations")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldNote", "Value of ZERO, means you need all location's graphs plotted. If one or more locations are explicitly picked, then preference is given to them.")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldEditor", "IntegerSpinBox")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldMinValue", "0")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldMaxValue", "500")
    Q_CLASSINFO("maxLocationPresenceGraphs_FieldDefaultValue", "6")
    Q_PROPERTY(int maxLocationPresenceGraphs READ maxLocationPresenceGraphs WRITE setMaxLocationPresenceGraphs NOTIFY maxLocationPresenceGraphsChanged)
    void setMaxLocationPresenceGraphs(int val);
    int maxLocationPresenceGraphs() const { return m_maxLocationPresenceGraphs; }
    Q_SIGNAL void maxLocationPresenceGraphsChanged();

    Q_CLASSINFO("locations_FieldGroup", "Locations")
    Q_CLASSINFO("locations_FieldLabel", "Locations to include in the presence graphs.")
    Q_CLASSINFO("locations_FieldNote", "Leave it empty to let Scrite automatically pick from most busy locations.")
    Q_CLASSINFO("locations_FieldEditor", "MultipleLocationSelector")
    Q_PROPERTY(QStringList locations READ locations WRITE setLocations NOTIFY locationsChanged)
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

    qreal pageHeight() const { return m_pageHeight; }

    QTime timeLength() const { return this->pixelLengthToTime(this->pixelLength()); }
    QTime timeLength(const Scene *scene) const
    {
        return this->pixelLengthToTime(this->pixelLength(scene));
    }
    QTime timeLength(const SceneHeading *heading) const
    {
        return this->pixelLengthToTime(this->pixelLength(heading));
    }
    QTime timeLength(const SceneElement *para) const
    {
        return this->pixelLengthToTime(this->pixelLength(para));
    }
    QTime timeLength(const ScreenplayElement *element) const
    {
        return this->pixelLengthToTime(this->pixelLength(element));
    }

    qreal pageLength() const { return this->pageLength(this->pixelLength()); }
    qreal pageLength(qreal pixelLength) const
    {
        return qFuzzyIsNull(m_pageHeight) ? 0 : pixelLength / m_pageHeight;
    }
    qreal pageLength(const Scene *scene) const
    {
        return this->pageLength(this->pixelLength(scene));
    }
    qreal pageLength(const SceneHeading *heading) const
    {
        return this->pageLength(this->pixelLength(heading));
    }
    qreal pageLength(const SceneElement *para) const
    {
        return this->pageLength(this->pixelLength(para));
    }
    qreal pageLength(const ScreenplayElement *element) const
    {
        return this->pageLength(this->pixelLength(element));
    }

    qreal pixelLength() const;
    qreal pixelLength(const Scene *scene) const;
    qreal pixelLength(const SceneHeading *heading) const;
    qreal pixelLength(const SceneElement *para) const;
    qreal pixelLength(const ScreenplayElement *element) const;

    QRectF boundingRect(const Scene *scene) const;
    QRectF boundingRect(const SceneHeading *heading) const
    {
        return this->boundingRectOfHeadingOrParagraph(heading);
    }
    QRectF boundingRect(const SceneElement *para) const
    {
        return this->boundingRectOfHeadingOrParagraph(para);
    }
    QRectF boundingRectOfHeadingOrParagraph(const QObject *object) const;
    QRectF boundingRect(const ScreenplayElement *element) const;

    QTime pixelLengthToTime(qreal val) const
    {
        return this->pageLengthToTime(this->pageLength(val));
    }
    QTime pageLengthToTime(qreal val) const;

    void polish(Distribution &report) const;

private:
    QTextDocument m_textDocument;
    QMap<const QObject *, QTextBlock> m_textBlockMap;
    qreal m_pageHeight = 0;
    qreal m_lineHeight = 0;
    qreal m_scaleFactor = 1.0;
    int m_maxLocationPresenceGraphs = 6;
    int m_maxCharacterPresenceGraphs = 6;
    qreal m_paragraphsLength = 0; // usually smaller than pageHeight
    qreal m_millisecondsPerPixel = 0;
    QStringList m_locations;
    QStringList m_characterNames;
    bool m_includeLocationPresenceGraphs = true;
    bool m_includeCharacterPresenceGraphs = true;
    bool m_considerPreferredGroupCategoryOnly = true;
};

#endif // STATISTICSREPORT_H
