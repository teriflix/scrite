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

#ifndef STATISTICSREPORT_H
#define STATISTICSREPORT_H

#include <QTime>
#include <QList>
#include <QtMath>

#include "abstractreportgenerator.h"

class StatisticsReportPage;
class ScreenplayTextDocument;

class StatisticsReport : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Statistics Report")
    Q_CLASSINFO("Description", "Generate a report key statistics of the screenplay.")

public:
    Q_INVOKABLE StatisticsReport(QObject *parent=nullptr);
    ~StatisticsReport();

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
    QList<Distribution> textDistribution(bool compact=true) const;

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
    void prepareTextDocument();
    void cleanupTextDocument();

    qreal pageHeight() const { return m_pageHeight; }

    QTime timeLength() const { return this->pixelLengthToTime(this->pixelLength()); }
    QTime timeLength(const Scene *scene) const { return this->pixelLengthToTime(this->pixelLength(scene)); }
    QTime timeLength(const SceneHeading *heading) const { return this->pixelLengthToTime(this->pixelLength(heading)); }
    QTime timeLength(const SceneElement *para) const { return this->pixelLengthToTime(this->pixelLength(para)); }
    QTime timeLength(const ScreenplayElement *element) const { return this->pixelLengthToTime(this->pixelLength(element)); }

    qreal pageLength() const { return this->pageLength(this->pixelLength()); }
    qreal pageLength(qreal pixelLength) const { return qFuzzyIsNull(m_pageHeight) ? 0 : pixelLength/m_pageHeight; }
    qreal pageLength(const Scene *scene) const { return this->pageLength(this->pixelLength(scene)); }
    qreal pageLength(const SceneHeading *heading) const { return this->pageLength(this->pixelLength(heading)); }
    qreal pageLength(const SceneElement *para) const { return this->pageLength(this->pixelLength(para)); }
    qreal pageLength(const ScreenplayElement *element) const { return this->pageLength(this->pixelLength(element)); }

    qreal pixelLength() const;
    qreal pixelLength(const Scene *scene) const;
    qreal pixelLength(const SceneHeading *heading) const;
    qreal pixelLength(const SceneElement *para) const;
    qreal pixelLength(const ScreenplayElement *element) const;

    QRectF boundingRect(const Scene *scene) const;
    QRectF boundingRect(const SceneHeading *heading) const { return this->boundingRectOfHeadingOrParagraph(heading); }
    QRectF boundingRect(const SceneElement *para) const { return this->boundingRectOfHeadingOrParagraph(para); }
    QRectF boundingRectOfHeadingOrParagraph(const QObject *object) const;
    QRectF boundingRect(const ScreenplayElement *element) const;

    QTime pixelLengthToTime(qreal val) const { return this->pageLengthToTime(this->pageLength(val)); }
    QTime pageLengthToTime(qreal val) const;

    void polish(Distribution &report) const;

private:
    QTextDocument m_textDocument;
    QMap<const QObject*,QTextBlock> m_textBlockMap;
    qreal m_pageHeight = 0;
    qreal m_lineHeight = 0;
    qreal m_scaleFactor = 1.0;
    qreal m_paragraphsLength = 0; // usually smaller than pageHeight
    qreal m_millisecondsPerPixel = 0;
};

inline const QVector<QColor> statsReportColors()
{
    static const QVector<QColor> colors = { QColor("#5e368a"), QColor("#e8bf5a") };
    return colors;
}

inline const QColor pickStatsReportColor(int index, bool cycleAround=true)
{
    const QVector<QColor> colors = ::statsReportColors();
    const QColor baseColor = colors.at( index%(colors.length()) );
    index = qAbs(index);

    if(cycleAround || index < colors.size())
        return baseColor;

    const int batch = qFloor( qreal(index)/colors.size() );
    const bool lighter = batch%2;
    const int factor = 100+batch*50;
    return lighter ? baseColor.lighter(factor) : baseColor.darker(factor);
}

#endif // STATISTICSREPORT_H
