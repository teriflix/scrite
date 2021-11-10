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

#include "statisticsreport.h"
#include "statisticsreport_p.h"
#include "screenplaytextdocument.h"

#include "scene.h"
#include "hourglass.h"
#include "screenplay.h"
#include "application.h"
#include "scritedocument.h"

#include <QTextTable>
#include <QScopeGuard>
#include <QTextCursor>
#include <QTextDocument>

StatisticsReport::StatisticsReport(QObject *parent)
    : AbstractReportGenerator(parent)
{

}

StatisticsReport::~StatisticsReport()
{

}

QList<StatisticsReport::Distribution> StatisticsReport::textDistribution(bool compact) const
{
    QList<StatisticsReport::Distribution> ret;

    QMap<SceneElement::Type, StatisticsReport::Distribution> map;
    if(m_textBlockMap.isEmpty() || m_textDocument.isEmpty())
        return ret;

    auto add = [&map](SceneElement::Type type, qreal pixelLength) {
        auto distribution = map.value(type);
        distribution.count += 1;
        distribution.pixelLength += pixelLength;
        map.insert(type, distribution);
    };

    {
        // First, lets sum up pixel lengths of all paragrap types.
        auto it = m_textBlockMap.constBegin();
        auto end = m_textBlockMap.constEnd();
        while(it != end)
        {
            const QObject *object = it.key();
            const SceneElement *paragraph = qobject_cast<const SceneElement*>(object);
            if(paragraph)
            {
                if(paragraph)
                    add(paragraph->type(), this->pixelLength(paragraph));
            }
            else
            {
                const SceneHeading *heading = qobject_cast<const SceneHeading*>(object);
                if(heading)
                    add(SceneElement::Heading, this->pixelLength(heading));
            }

            ++it;
        }

        if(compact)
        {
            map[SceneElement::Action].pixelLength += map[SceneElement::Shot].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Heading].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Character].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Transition].pixelLength;
            map[SceneElement::Action].pixelLength += map[SceneElement::Parenthetical].pixelLength;
            map.remove(SceneElement::Shot);
            map.remove(SceneElement::Heading);
            map.remove(SceneElement::Character);
            map.remove(SceneElement::Transition);
            map.remove(SceneElement::Parenthetical);
        }
    }

    const QMetaObject *mo = &SceneElement::staticMetaObject;
    const QMetaEnum typeEnum = mo->enumerator( mo->indexOfEnumerator("Type") );

    // Now lets convert pixel lengths to page & time lengths
    auto it = map.begin();
    auto end = map.end();
    while(it != end)
    {
        Distribution &dist = it.value();

        this->polish(dist);
        dist.key = QString::fromLatin1(typeEnum.valueToKey(it.key()));

        ret.append(dist);
        ++it;
    }

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::dialogueDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();

    QMap<QString,Distribution> map;
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        const ScreenplayElement *element = screenplay->elementAt(i);
        const Scene *scene = element->scene();
        if(scene == nullptr)
            continue;

        auto dialogues = scene->dialogueElements();
        auto it = dialogues.constBegin();
        auto end = dialogues.constEnd();
        while(it != end)
        {
            auto paragraphs = it.value();
            for(auto paragraph : qAsConst(paragraphs))
            {
                Distribution &dist = map[it.key()];
                dist.pixelLength += this->pixelLength(paragraph);
                dist.count++;
            }
            ++it;
        }
    }

    auto it = map.begin();
    auto end = map.end();
    while(it != end)
    {
        Distribution &dist = it.value();

        this->polish(dist);
        dist.key = it.key();

        ret.append(dist);
        ++it;
    }

    std::sort(ret.begin(), ret.end(), [](const Distribution &a, const Distribution &b) {
        return a.ratio > b.ratio;
    });

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::sceneDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        const ScreenplayElement *element = screenplay->elementAt(i);
        const Scene *scene = element->scene();
        if(!scene)
            continue;

        if(scene && scene->heading()->isEnabled())
        {
            StatisticsReport::Distribution item;
            item.key = element->resolvedSceneNumber();
            item.color = scene->color();
            ret.append(item);
        }
        else
        {
            if(ret.isEmpty())
                ret.append(StatisticsReport::Distribution());
        }

        ret.last().pixelLength += this->pixelLength(scene);
    }

    for(auto &item : ret)
        this->polish(item);

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::actDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();
    const Structure *structure = this->document()->structure();
    const auto actElements = screenplay->getFilteredElements([](ScreenplayElement *e) {
        return e->elementType() == ScreenplayElement::BreakElementType &&
               e->breakType() == Screenplay::Act;
    });

    if(actElements.isEmpty())
        return ret;

    const QString prefrredGroupCategory = structure->preferredGroupCategory();
    const QStringList actNames = structure->categoryActNames().value( prefrredGroupCategory ).toStringList();
    auto actName = [actNames](int actIndex) {
        return actIndex >= 0 && actIndex < actNames.size() ? actNames.at(actIndex) : QStringLiteral("ACT %1").arg(actIndex+1);
    };

    typedef QPair< QString, QList<Scene*> > ActSceneListPair;
    QList<ActSceneListPair> actScenesList;
    QString currentAct;
    int actIndex = -1;

    const auto elements = screenplay->getElements();
    for(auto element : elements)
    {
        if(element->scene())
        {
            if(currentAct.isEmpty())
            {
                actIndex = 0;
                currentAct = actName(actIndex);
                ActSceneListPair pair;
                pair.first = currentAct;
                actScenesList.append(pair);
            }

            actScenesList.last().second.append(element->scene());
        }
        else if(element->breakType() == Screenplay::Act)
        {
            ++actIndex;
            currentAct = actName(actIndex);
            ActSceneListPair pair;
            pair.first = currentAct;
            actScenesList.append(pair);
        }
        else
        {
            currentAct.clear();
            actIndex = -1;
        }
    }

    int episodeIndex = 0;
    QColor baseColor = screenplay->episodeCount() > 0 ? ::pickStatsReportColor(episodeIndex) : Qt::transparent;
    actIndex = 0;
    for(auto actScenes : qAsConst(actScenesList))
    {
        StatisticsReport::Distribution item;
        item.key = actScenes.first;

        if(baseColor == Qt::transparent)
            item.color = ::pickStatsReportColor(actIndex++);
        else
        {
            if(actIndex > 0 && item.key == actName(0))
                baseColor = ::pickStatsReportColor(++episodeIndex);

            const bool baseColorIsLight = Application::isLightColor(baseColor);
            item.color = (actIndex++%2) ? baseColor.lighter(baseColorIsLight ? 150 : 240) : baseColor.lighter(baseColorIsLight ? 120 : 200);
        }

        for(Scene *scene : qAsConst(actScenes.second))
            item.pixelLength += this->pixelLength(scene);

        this->polish(item);
        ret.append(item);
    }

    return ret;
}

QList<StatisticsReport::Distribution> StatisticsReport::episodeDistribution() const
{
    QList<StatisticsReport::Distribution> ret;

    const Screenplay *screenplay = this->document()->screenplay();
    const auto episodeElements = screenplay->getFilteredElements([](ScreenplayElement *e) {
        return e->elementType() == ScreenplayElement::BreakElementType &&
               e->breakType() == Screenplay::Episode;
    });
    if(episodeElements.isEmpty())
        return ret;

    typedef QPair< QString, QList<Scene*> > EpisodeSceneListPair;
    QList<EpisodeSceneListPair> episodeScenesList;
    QString currentEpisode;
    int episodeIndex = -1;

    const auto elements = screenplay->getElements();
    for(auto element : elements)
    {
        if(element->scene())
        {
            if(currentEpisode.isEmpty())
            {
                episodeIndex = 0;
                currentEpisode = QStringLiteral("Episode 1");
                EpisodeSceneListPair pair;
                pair.first = currentEpisode;
                episodeScenesList.append(pair);
            }

            episodeScenesList.last().second.append(element->scene());
        }
        else if(element->breakType() == Screenplay::Episode)
        {
            ++episodeIndex;
            currentEpisode = QStringLiteral("Episode %1").arg(episodeIndex+1);
            EpisodeSceneListPair pair;
            pair.first = currentEpisode;
            episodeScenesList.append(pair);
        }
    }

    int epIndex=0;
    for(auto episodeScenes : qAsConst(episodeScenesList))
    {
        StatisticsReport::Distribution item;
        item.key = episodeScenes.first;
        item.color = ::pickStatsReportColor(epIndex++);

        for(Scene *scene : qAsConst(episodeScenes.second))
            item.pixelLength += this->pixelLength(scene);

        this->polish(item);
        ret.append(item);
    }

    return ret;
}

bool StatisticsReport::doGenerate(QTextDocument *textDocument)
{   
    qScopeGuard([=]() { this->cleanupTextDocument(); });
    this->prepareTextDocument();

    /**
     * This function is called to generate report into ODT files.
     */
    Screenplay *screenplay = ScriteDocument::instance()->screenplay();
    Structure *structure = ScriteDocument::instance()->structure();

    QTextDocument &document = *textDocument;
    QTextCursor cursor(&document);

    const QFont defaultFont = this->document()->printFormat()->defaultFont();

    QTextBlockFormat defaultBlockFormat;

    QTextCharFormat defaultCharFormat;
    defaultCharFormat.setFontFamily(defaultFont.family());
    defaultCharFormat.setFontPointSize(12);

    // Report Title
    {
        QTextBlockFormat blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        cursor.setBlockFormat(blockFormat);

        QTextCharFormat charFormat = defaultCharFormat;
        charFormat.setFontPointSize(24);
        charFormat.setFontCapitalization(QFont::AllUppercase);
        charFormat.setFontWeight(QFont::Bold);
        charFormat.setFontUnderline(true);
        cursor.setCharFormat(charFormat);

        QString title = screenplay->title();
        if(title.isEmpty())
            title = QStringLiteral("Untitled Screenplay");
        cursor.insertText(title);
        cursor.insertBlock();
        cursor.insertText(QStringLiteral("Key Statistics"));

        blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        blockFormat.setBottomMargin(20);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertHtml("This report was generated using <strong>Scrite</strong><br/>(<a href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
    }

    QTextTableFormat tableFormat;
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(5);
    tableFormat.setBorder(3);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setHeaderRowCount(1);

    QTextTable *table = cursor.insertTable(5, 3, tableFormat);

    cursor = table->cellAt(0, 0).firstCursorPosition();
    cursor.insertText( QStringLiteral("Page Count: ") );
    cursor = table->cellAt(0, 1).firstCursorPosition();
    cursor.insertText( QString::number(this->pageCount()) );
    cursor = table->cellAt(0, 2).firstCursorPosition();
    cursor.insertHtml( QStringLiteral("<font size=\"-2\">Page count may change in generated PDFs of the screenplay.</font>") );

    // Number of scenes
    cursor = table->cellAt(1, 0).firstCursorPosition();
    cursor.insertText( QStringLiteral("Number Of Scenes: ") );
    cursor = table->cellAt(1, 1).firstCursorPosition();
    cursor.insertText( QString::number(screenplay->sceneCount()) );

    // Text Distribution
    const auto textDist = this->textDistribution(true);
    for(int i=0; i<textDist.size(); i++)
    {
        const int row = i+2;
        const Distribution dist = textDist.at(i);
        cursor = table->cellAt(row, 0).firstCursorPosition();
        cursor.insertText( dist.key + QStringLiteral(": ") );
        cursor = table->cellAt(row, 1).firstCursorPosition();
        cursor.insertText( dist.percent );
    }

    // Dialogue screen time for each character
    cursor = table->lastCursorPosition();
    cursor.movePosition(QTextCursor::Down);

    cursor.insertBlock();

    const auto dialogueLengths = this->dialogueDistribution();
    const QStringList characters = structure->allCharacterNames();

    table = cursor.insertTable(dialogueLengths.size()+1, 3, tableFormat);
    cursor = table->cellAt(0, 0).firstCursorPosition();
    cursor.insertHtml( QStringLiteral("<b>Character</b>") );
    cursor = table->cellAt(0, 1).firstCursorPosition();
    cursor.insertHtml( QStringLiteral("<b>Dialogue Count</b>") );
    cursor = table->cellAt(0, 2).firstCursorPosition();
    cursor.insertHtml( QStringLiteral("<b>Dialogue Percent</b>") );

    QTextBlockFormat cellFormat;
    cellFormat.setAlignment(Qt::AlignRight);

    for(int i=0; i<dialogueLengths.size(); i++)
    {
        const int row = i+1;
        const Distribution dist = dialogueLengths.at(i);
        const QString character = dist.key;

        cursor = table->cellAt(row, 0).firstCursorPosition();
        cursor.insertText(character);

        cursor = table->cellAt(row, 1).firstCursorPosition();
        cursor.mergeBlockFormat(cellFormat);
        cursor.insertText( QString::number(dist.count) );

        cursor = table->cellAt(row, 2).firstCursorPosition();
        cursor.mergeBlockFormat(cellFormat);
        cursor.insertText( dist.percent );
    }

    return true;
}

bool StatisticsReport::canDirectPrintToPdf() const
{
    return true;
}

bool StatisticsReport::usePdfWriter() const
{
    return true;
}

bool StatisticsReport::directPrintToPdf(QPdfWriter *pdfWriter)
{
    qScopeGuard([=]() { this->cleanupTextDocument(); });
    this->prepareTextDocument();

    const Screenplay *screenplay = this->document()->screenplay();

    StatisticsReportPage scene(this);
    scene.setWatermark(this->watermark());
    scene.setComment(this->comment());
    scene.addStandardItems(StatisticsReportPage::WatermarkOverlayLayer|StatisticsReportPage::HeaderFooterLayer);
    scene.setTitle(screenplay->title() + QStringLiteral(" - Statistics"));
    const bool ret = scene.exportToPdf(pdfWriter);
    return ret;
}

void StatisticsReport::prepareTextDocument()
{
    HourGlass hourGlass;
    this->cleanupTextDocument();

    const Screenplay *screenplay = this->document()->screenplay();
    const ScreenplayFormat *format = this->document()->printFormat();
    const qreal pageWidth = qCeil(format->pageLayout()->contentWidth());
    m_pageHeight = qCeil(format->pageLayout()->contentRect().height());
    m_millisecondsPerPixel = (format->secondsPerPage() * 1000)/m_pageHeight;

    m_textDocument.setTextWidth(pageWidth);
    m_textDocument.setDefaultFont(format->defaultFont());

    QTextCursor cursor(&m_textDocument);
    auto prepareCursor = [=](QTextCursor &cursor, SceneElement::Type paraType) {
        const SceneElementFormat *eformat = format->elementFormat(paraType);
        QTextBlockFormat blockFormat = eformat->createBlockFormat(&pageWidth);
        QTextCharFormat charFormat = eformat->createCharFormat(&pageWidth);
        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
    };

    auto polishFontsAndInsertTextAtCursor = [](QTextCursor &cursor, const QString &text) {
        const QList<TransliterationEngine::Boundary> items = TransliterationEngine::instance()->evaluateBoundaries(text);
        Q_FOREACH(TransliterationEngine::Boundary item, items)
        {
            if(item.string.isEmpty())
                continue;
            const QFont font = TransliterationEngine::instance()->languageFont(item.language);
            QTextCharFormat format;
            format.setFontFamily(font.family());
            cursor.mergeCharFormat(format);
            cursor.insertText(item.string);
        }
    };

    const int nrElements = screenplay->elementCount();
    for(int i=0; i<nrElements; i++)
    {
        const ScreenplayElement *element = screenplay->elementAt(i);
        if(element->scene() == nullptr)
            continue;

        const Scene *scene = element->scene();
        if(scene->heading()->isEnabled())
        {
            if(cursor.position() > 0)
                cursor.insertBlock();
            prepareCursor(cursor, SceneElement::Heading);
            polishFontsAndInsertTextAtCursor(cursor, scene->heading()->text());
            m_textBlockMap.insert(scene->heading(), cursor.block());
        }

        for(int p=0; p<scene->elementCount(); p++)
        {
            if(cursor.position() > 0)
                cursor.insertBlock();

            const SceneElement *para = scene->elementAt(p);
            prepareCursor(cursor, para->type());
            polishFontsAndInsertTextAtCursor(cursor, para->text());
            m_textBlockMap.insert(para, cursor.block());
        }
    }

    QAbstractTextDocumentLayout *layout = m_textDocument.documentLayout();
    auto it = m_textBlockMap.constBegin();
    auto end = m_textBlockMap.constEnd();
    m_lineHeight = m_pageHeight;
    while(it != end)
    {
        const QTextBlock paraBlock = m_textBlockMap.value(it.key());
        const QRectF paraBlockRect = layout->blockBoundingRect(paraBlock);
        const qreal paraHeight = paraBlockRect.height();

        m_paragraphsLength += paraHeight;
        if( paraHeight > 0 )
            m_lineHeight = qMin(paraHeight, m_lineHeight);

        ++it;
    }
}

void StatisticsReport::cleanupTextDocument()
{
    m_textBlockMap.clear();
    m_textDocument.clear();
    m_pageHeight = 0;
    m_paragraphsLength = 0;
    m_millisecondsPerPixel = 0;
}

qreal StatisticsReport::pixelLength() const
{
    return m_textDocument.isEmpty() ? 0.0 : m_textDocument.size().height();
}

qreal StatisticsReport::pixelLength(const Scene *scene) const
{
    return scene ? qMax(this->boundingRect(scene).height(), 0.0) : 0;
}

qreal StatisticsReport::pixelLength(const SceneHeading *heading) const
{
    return qMax(this->boundingRect(heading).height(), 0.0);
}

qreal StatisticsReport::pixelLength(const SceneElement *para) const
{
    return qMax(this->boundingRect(para).height(), 0.0);
}

qreal StatisticsReport::pixelLength(const ScreenplayElement *element) const
{
    return qMax(this->boundingRect(element).height(), 0.0);
}

QRectF StatisticsReport::boundingRect(const Scene *scene) const
{
    QRectF nullRect;
    if(scene == nullptr || m_textDocument.isEmpty() || qFuzzyIsNull(m_pageHeight))
        return nullRect;

    QTextBlock fromBlock, toBlock;

    if(scene->heading()->isEnabled())
    {
        if(!m_textBlockMap.contains(scene->heading()))
            return nullRect;
        fromBlock = m_textBlockMap.value(scene->heading());
        toBlock = fromBlock;
    }
    else
    {
        const SceneElement *firstPara = scene->elementAt(0);
        if(firstPara == nullptr || !m_textBlockMap.contains(firstPara))
            return nullRect;

        fromBlock = m_textBlockMap.value(firstPara);
        toBlock = fromBlock;
    }

    const SceneElement *lastPara = scene->elementAt(scene->elementCount()-1);
    if(!lastPara || !m_textBlockMap.contains(lastPara))
        return nullRect;

    toBlock = m_textBlockMap.value(lastPara);

    QAbstractTextDocumentLayout *layout = m_textDocument.documentLayout();
    const QRectF fromBlockRect = layout->blockBoundingRect(fromBlock);
    const QRectF toBlockRect = layout->blockBoundingRect(toBlock);
    return QRectF(fromBlockRect.topLeft(), toBlockRect.bottomRight() + QPointF(0,m_lineHeight));
}

QRectF StatisticsReport::boundingRectOfHeadingOrParagraph(const QObject *object) const
{
    QRectF nullRect;
    if(object == nullptr || m_textDocument.isEmpty() || qFuzzyIsNull(m_pageHeight))
        return nullRect;

    if(!m_textBlockMap.contains(object))
        return nullRect;

    QAbstractTextDocumentLayout *layout = m_textDocument.documentLayout();
    const QTextBlock paraBlock = m_textBlockMap.value(object);
    const QRectF paraBlockRect = layout->blockBoundingRect(paraBlock);
    return paraBlockRect;
}

QRectF StatisticsReport::boundingRect(const ScreenplayElement *element) const
{
    QRectF nullRect;
    if(element->scene())
        return this->boundingRect(element->scene());

    const Screenplay *screenplay = this->document()->screenplay();
    ScreenplayElement *ncelement = const_cast<ScreenplayElement*>(element);
    const QList<int> idxList = screenplay->sceneElementsInBreak(ncelement);
    if(idxList.isEmpty())
        return nullRect;

    const QRectF firstRect = this->boundingRect( screenplay->elementAt(idxList.first()) );
    const QRectF lastRect = this->boundingRect( screenplay->elementAt(idxList.last()) );
    if(firstRect.isNull() || lastRect.isNull())
        return nullRect;

    return QRectF(firstRect.topLeft(), lastRect.bottomRight() + QPointF(0,m_lineHeight));
}

QTime StatisticsReport::pageLengthToTime(qreal val) const
{
    const ScreenplayFormat *format = this->document()->printFormat();
    const int secsPerPage = format->secondsPerPage();
    const int totalSecs = qreal(secsPerPage) * val;
    return Application::secondsToTime(totalSecs);
}

void StatisticsReport::polish(Distribution &report) const
{
    if(!qFuzzyIsNull(m_paragraphsLength))
    {
        report.ratio = report.pixelLength/m_paragraphsLength;
        const int cent = qRound(report.ratio*100);
        report.percent = QString::number(cent) + QStringLiteral("%");
        report.pageLength = this->pageLength(report.pixelLength);
        report.timeLength = this->pixelLengthToTime(report.pixelLength);
    }
}
