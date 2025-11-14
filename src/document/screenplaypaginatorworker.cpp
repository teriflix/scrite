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

#include "screenplaypaginatorworker.h"
#include "scene.h"
#include "utils.h"
#include "screenplay.h"
#include "formatting.h"
#include "languageengine.h"
#include "timeprofiler.h"

#include <QtMath>
#include <QThread>
#include <QMetaObject>
#include <QScopeGuard>
#include <QMetaProperty>
#include <QTextDocument>
#include <QStandardPaths>
#include <QTextDocumentWriter>
#include <QAbstractTextDocumentLayout>
#include <QPdfWriter>

static void registerPaginatorTypes()
{
    static bool done = false;

    if (done)
        return;

    // These may not be required??
    qRegisterMetaType<QTime>("QTime");
    qRegisterMetaType<QJsonObject>("QJsonObject");

    // These are absolutely required, because they go across thread boundaries.
    qRegisterMetaType<SceneContent>("SceneContent");
    qRegisterMetaType<SceneParagraph>("SceneParagraph");
    qRegisterMetaType<ScenePageBreak>("ScenePageBreak");
    qRegisterMetaType<ScreenplayPaginatorRecord>("ScreenplayPaginatorRecord");

    qRegisterMetaType<QList<SceneContent>>("QList<SceneContent>");
    qRegisterMetaType<QList<SceneParagraph>>("QList<SceneParagraph>");
    qRegisterMetaType<QList<ScenePageBreak>>("QList<ScenePageBreak>");
    qRegisterMetaType<QList<ScreenplayPaginatorRecord>>("QList<ScreenplayPaginatorRecord>");

    done = true;
}

///////////////////////////////////////////////////////////////////////////////////////////

bool SceneParagraph::isValid() const
{
    return !this->sceneId.isEmpty() && this->type >= 0 && (this->type == SceneElement::Heading || !this->id.isEmpty());
}

SceneParagraph SceneParagraph::fromSceneHeading(const SceneHeading *heading)
{
    if (heading == nullptr || heading->scene() == nullptr)
        return SceneParagraph();

    return { heading->scene()->id(),
             QString(),
             heading->isEnabled(),
             SceneElement::Heading,
             heading->displayText(),
             Qt::AlignLeft,
             QVector<QTextLayout::FormatRange>() };
}

SceneParagraph SceneParagraph::fromSceneElement(const SceneElement *element)
{
    if (element == nullptr || element->scene() == nullptr)
        return SceneParagraph();

    return { element->scene()->id(), element->id(),         true, element->type(), element->formattedText(),
             element->alignment(),   element->textFormats() };
}

bool SceneContent::isValid() const
{
    return this->type >= 0 && (this->type == ScreenplayElement::SceneElementType ? !this->id.isEmpty() : true);
}

SceneContent SceneContent::fromScreenplayElement(const ScreenplayElement *element)
{
    if (element == nullptr || element->screenplay() == nullptr)
        return SceneContent();

    SceneContent ret;
    ret.type = element->elementType();
    ret.serialNumber = element->serialNumber();
    if (element->elementType() == ScreenplayElement::BreakElementType) {
        ret.breakType = element->breakType();
        return ret;
    }

    const Scene *scene = element->scene();
    if (scene == nullptr)
        return SceneContent();

    ret.omitted = element->isOmitted();
    ret.id = scene->id();

    if (scene->heading()->isEnabled()) {
        SceneParagraph headingParagraph = SceneParagraph::fromSceneHeading(scene->heading());
        if (headingParagraph.isValid())
            ret.paragraphs.append(headingParagraph);
    }

    for (int i = 0; i < scene->elementCount(); i++) {
        const SceneElement *paragraph = scene->elementAt(i);
        SceneParagraph sceneParagraph = SceneParagraph::fromSceneElement(paragraph);
        if (sceneParagraph.isValid())
            ret.paragraphs.append(sceneParagraph);
    }

    return ret;
}

QList<SceneContent> SceneContent::fromScreenplay(const Screenplay *screenplay)
{
    QList<SceneContent> ret;

    if (screenplay == nullptr)
        return ret;

    for (int i = 0; i < screenplay->elementCount(); i++) {
        SceneContent sceneContent = fromScreenplayElement(screenplay->elementAt(i));
        if (sceneContent.isValid())
            ret.append(sceneContent);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////

PaginatorDocumentInsights::PaginatorDocumentInsights() { }

PaginatorDocumentInsights::PaginatorDocumentInsights(const PaginatorDocumentInsights &other)
{
    this->contentRangeMap = other.contentRangeMap;
}

bool PaginatorDocumentInsights::operator==(const PaginatorDocumentInsights &other)
{
    return this->contentRangeMap == other.contentRangeMap;
}

bool PaginatorDocumentInsights::operator!=(const PaginatorDocumentInsights &other)
{
    return this->contentRangeMap != other.contentRangeMap;
}

PaginatorDocumentInsights &PaginatorDocumentInsights::operator=(const PaginatorDocumentInsights &other)
{
    this->contentRangeMap = other.contentRangeMap;
    return *this;
}

bool PaginatorDocumentInsights::isEmpty() const
{
    return contentRangeMap.isEmpty();
}

QTextBlock PaginatorDocumentInsights::findBlock(const SceneHeading *heading) const
{
    if (heading == nullptr || heading->scene() == nullptr)
        return QTextBlock();

    return this->findBlock(heading->scene()->id(), QString());
}

QTextBlock PaginatorDocumentInsights::findBlock(const SceneElement *paragraph) const
{
    if (paragraph == nullptr || paragraph->scene() == nullptr)
        return QTextBlock();

    return this->findBlock(paragraph->scene()->id(), paragraph->id());
}

QTextBlock PaginatorDocumentInsights::findBlock(const QString &sceneId, const QString &paragraphId) const
{
    if (sceneId.isEmpty())
        return QTextBlock();

    BlockRange range = this->findBlockRangeBySceneId(sceneId);
    if (!range.isValid())
        return QTextBlock();

    QTextBlock block = range.from;
    while (block.isValid() && block.position() <= range.until.position()) {
        const ScreenplayPaginatorBlockData *data = ScreenplayPaginatorBlockData::get(block);

        if (data->sceneId != sceneId)
            return QTextBlock();

        if (data->paragraphId == paragraphId)
            return block;

        block = block.next();
    }

    return QTextBlock();
}

PaginatorDocumentInsights::BlockRange PaginatorDocumentInsights::findBlockRangeBySceneId(const QString &sceneId) const
{
    const QList<BlockRange> blockRanges = this->contentRangeMap.values();
    auto it = std::find_if(blockRanges.begin(), blockRanges.end(),
                           [sceneId](const BlockRange &item) { return (item.sceneId == sceneId); });
    if (it != blockRanges.end())
        return *it;
    return BlockRange();
}

PaginatorDocumentInsights::BlockRange PaginatorDocumentInsights::findBlockRangeBySerialNumber(int serialNumber) const
{
    return this->contentRangeMap.value(serialNumber);
}

bool PaginatorDocumentInsights::BlockRange::isValid() const
{
    return serialNumber >= 0 && from.isValid() && until.isValid();
}

PaginatorDocumentInsights::BlockRange::BlockRange() { }

PaginatorDocumentInsights::BlockRange::BlockRange(const BlockRange &other)
{
    *this = other;
}

bool PaginatorDocumentInsights::BlockRange::operator==(const BlockRange &other) const
{
    return this->serialNumber == other.serialNumber && this->sceneId == other.sceneId && this->from == other.from
            && this->until == other.until;
}

PaginatorDocumentInsights::BlockRange &PaginatorDocumentInsights::BlockRange::operator=(const BlockRange &other)
{
    this->serialNumber = other.serialNumber;
    this->sceneId = other.sceneId;
    this->from = other.from;
    this->until = other.until;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////

const char *PaginatorDocumentInsights::property = "#PaginatorDocumentInsights";
const int ScreenplayPaginatorWorker::syncInterval = 500;

ScreenplayPaginatorWorker::ScreenplayPaginatorWorker(QTextDocument *document, ScreenplayFormat *format, QObject *parent)
    : QObject(parent), m_document(document), m_defaultFormat(format)
{
    ::registerPaginatorTypes();
}

ScreenplayPaginatorWorker::~ScreenplayPaginatorWorker() { }

void ScreenplayPaginatorWorker::setSynchronousSync(bool val)
{
    if (m_synchronousSync == val)
        return;

    m_synchronousSync = val;
    emit synchronousSyncChanged();
}

void ScreenplayPaginatorWorker::useFormat(const QJsonObject &format)
{
    if (m_defaultFormat == nullptr) {
        m_formatJson = format;
        this->scheduleSyncDocument(Q_FUNC_INFO);
    }
}

void ScreenplayPaginatorWorker::reset(const QList<SceneContent> &screenplayContent)
{
    m_screenplayContent = screenplayContent;
    this->scheduleSyncDocument(Q_FUNC_INFO);
}

void ScreenplayPaginatorWorker::insertElement(int index, const SceneContent &sceneContent)
{
    if (!sceneContent.isValid())
        return;

    index = qBound(0, index, m_screenplayContent.size());
    m_screenplayContent.insert(index, sceneContent);
    this->scheduleSyncDocument(Q_FUNC_INFO);
}

void ScreenplayPaginatorWorker::removeElement(int index)
{
    if (index < 0 || index >= m_screenplayContent.size())
        return;

    m_screenplayContent.removeAt(index);
    this->scheduleSyncDocument(Q_FUNC_INFO);
}

void ScreenplayPaginatorWorker::omitElement(int index)
{
    if (index < 0 || index >= m_screenplayContent.size())
        return;

    m_screenplayContent[index].omitted = true;
    this->scheduleSyncDocument(Q_FUNC_INFO);
}

void ScreenplayPaginatorWorker::includeElement(int index)
{
    if (index < 0 || index >= m_screenplayContent.size())
        return;

    m_screenplayContent[index].omitted = false;
    this->scheduleSyncDocument(Q_FUNC_INFO);
}

void ScreenplayPaginatorWorker::updateScene(const SceneContent &sceneContent)
{
    if (!sceneContent.isValid() || sceneContent.type != ScreenplayElement::SceneElementType)
        return;

    auto it = std::find_if(m_screenplayContent.begin(), m_screenplayContent.end(),
                           [sceneContent](const SceneContent &item) { return (item.id == sceneContent.id); });
    if (it == m_screenplayContent.end())
        return;

    *it = sceneContent;
    this->scheduleSyncDocument(Q_FUNC_INFO);
}

void ScreenplayPaginatorWorker::updateParagraph(const SceneParagraph &paragraph)
{
    if (!paragraph.isValid())
        return;

    auto sceneIt = std::find_if(m_screenplayContent.begin(), m_screenplayContent.end(),
                                [paragraph](const SceneContent &item) { return (item.id == paragraph.sceneId); });
    if (sceneIt == m_screenplayContent.end())
        return;

    auto paraIt = std::find_if(sceneIt->paragraphs.begin(), sceneIt->paragraphs.end(),
                               [paragraph](const SceneParagraph &item) { return (item.id == paragraph.id); });
    if (paraIt == sceneIt->paragraphs.end())
        return;

    *paraIt = paragraph;
    this->scheduleSyncDocument(Q_FUNC_INFO);
}

void ScreenplayPaginatorWorker::query(int cursorPosition, int currentSerialNumber)
{
    qreal pixelOffset = this->cursorPixelOffset(cursorPosition, currentSerialNumber);
    QTime time = ScreenplayPaginator::pixelToTimeLength(pixelOffset, m_format, m_document);
    int pageNr = qMax(qCeil(ScreenplayPaginator::pixelToPageLength(pixelOffset, m_document)), 1);

    emit cursorQueryResponse(cursorPosition, pixelOffset, pageNr, time);
}

void ScreenplayPaginatorWorker::syncDocument()
{
#if 0
    PROFILE_THIS_FUNCTION2;
#endif

    if (m_syncDocumentTimer != nullptr)
        m_syncDocumentTimer->stop();

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (!m_synchronousSync && now - m_lastSyncDocumentTimestamp < ScreenplayPaginatorWorker::syncInterval)
        return;

    m_lastSyncDocumentTimestamp = now;

    if (m_screenplayContent.isEmpty()) {
        paginationComplete(QList<ScreenplayPaginatorRecord>(), 0, 0, QTime());
        return;
    }

    /**
     * When this function is called, we got to recreate the complete QTextDocument
     * from ground up, using the SceneContent list available in this worker. We always
     * have to assume that this function maybe called for the first time.
     *
     * This function is called whenever any little change happens in the screenplay.
     * This means we are reconstructing the QTextDocument fully from ground up for
     * every little change in the screenplay.
     *
     * Why this kolaveri di?
     *
     * In the past part updates to the QTextDocument has resulted in extremely inconsistent
     * updates. Meaning, updating a block in part seems to offer a different result than
     * when the whole thing is reconstructed in full. Sometimes line spaces between blocks
     * would disappear. Sometimes formatting would get messed up. This means we would
     * end up getting very different impressions of the document metrics, and that's
     * bad news.
     *
     * The only solace is that we combine changes into a batch update every once in 500ms.
     * And the updates are done in a separate thread, so the UI is free to do its own thing.
     */

    // We cannot use ScriteDocument::instance()->printFormat() in here, because
    // its possible that this ScreenplayPaginatorWorker instance is created, only
    // to be thrown into a separate thread. So, we have to create a brand new
    // ScreenplayFormat instance, disconnected from ScriteDocument and the rest of it.

    // Unless we have accepted the ScreenplayFormat in the constructor itself, in which
    // case we are allowed to use that format directly. In such cases, we will always ignore
    // any JSON supplied via useFormat() method.
    if (m_format == nullptr) {
        if (m_defaultFormat != nullptr && m_defaultFormat->thread() == QThread::currentThread())
            m_format = m_defaultFormat;
        else {
            m_format = new ScreenplayFormat(this);
            m_format->pageLayout()->evaluateRectsNow();
        }
    }

    // Everytime this function is called, its possible that the format got updated.
    if (m_format != m_defaultFormat && !m_formatJson.isEmpty()) {
        QObjectSerializer::fromJson(m_formatJson, m_format);
        m_format->pageLayout()->evaluateRectsNow();
        m_formatJson = QJsonObject();
    }

    // Maybe we don't even have a document just yet
    if (m_document == nullptr)
        m_document = new QTextDocument(this);

    // First clear the document of all content
    const qreal pageWidth = qCeil(m_format->pageLayout()->contentWidth());
    m_document->clear();
    m_document->setTextWidth(pageWidth);
    m_format->pageLayout()->configure(m_document);

    // Create formatted paragraphs for all the SceneContent objects we have in the worker.
    QTextCursor cursor(m_document);

    auto prepareCursor = [=](QTextCursor &cursor, SceneElement::Type paraType, Qt::Alignment overrideAlignment) {
        const SceneElementFormat *eformat = m_format->elementFormat(paraType);
        QTextBlockFormat blockFormat = eformat->createBlockFormat(overrideAlignment, &pageWidth);
        QTextCharFormat charFormat = eformat->createCharFormat(&pageWidth);
        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
    };

    auto maybeAbort = [=]() -> bool {
        if (QThread::currentThread()->isFinished() || QThread::currentThread()->isInterruptionRequested()) {
            m_document->clear();
            return true;
        }
        return false;
    };

    PaginatorDocumentInsights insights;

    int currentActSerialNumber = -1;
    int currentEpisodeSerialNumber = -1;
    QMap<int, QList<int>> breakSerialNumbers;

    for (const SceneContent &content : qAsConst(m_screenplayContent)) {
        if (maybeAbort())
            break;

        if (!content.isValid() || content.omitted)
            continue;

        if (content.type == ScreenplayElement::BreakElementType) {
            if (content.breakType == Screenplay::Act) {
                currentActSerialNumber = content.serialNumber;
            } else if (content.breakType == Screenplay::Episode) {
                currentEpisodeSerialNumber = content.serialNumber;
            }

            continue;
        }

        PaginatorDocumentInsights::BlockRange range;
        range.serialNumber = content.serialNumber;
        range.from = QTextBlock();
        range.until = QTextBlock();
        range.sceneId = content.id;

        if (currentActSerialNumber >= 0)
            breakSerialNumbers[currentActSerialNumber].append(content.serialNumber);
        if (currentEpisodeSerialNumber >= 0)
            breakSerialNumbers[currentEpisodeSerialNumber].append(content.serialNumber);

        for (const SceneParagraph &paragraph : qAsConst(content.paragraphs)) {
            if (maybeAbort())
                break;

            if (cursor.position() > 0)
                cursor.insertBlock();

            prepareCursor(cursor, SceneElement::Type(paragraph.type), paragraph.alignment);
            LanguageEngine::polishFontsAndInsertTextAtCursor(cursor, paragraph.text, paragraph.formats);

            ScreenplayPaginatorBlockData *blockData = new ScreenplayPaginatorBlockData;
            blockData->serialNumber = content.serialNumber;
            blockData->paragraphType = SceneElement::Type(paragraph.type);
            blockData->sceneId = paragraph.sceneId;
            blockData->paragraphId = paragraph.id;

            QTextBlock block = cursor.block();
            block.setUserData(blockData);

            if (!range.from.isValid())
                range.from = block;
            range.until = block;
        }

        insights.contentRangeMap.insert(range.serialNumber, range);
    }

    // For each block range, construct a record
    QMap<int, int> serialNumberMap;
    int lastPageNr = -1;

    QList<ScreenplayPaginatorRecord> records;
    records.reserve(m_screenplayContent.size());

    for (const SceneContent &sceneContent : qAsConst(m_screenplayContent)) {
        if (maybeAbort())
            break;

        ScreenplayPaginatorRecord record;
        record.serialNumber = sceneContent.serialNumber;

        PaginatorDocumentInsights::BlockRange range = insights.findBlockRangeBySerialNumber(sceneContent.serialNumber);
        if (!range.isValid()) {
            records.append(record);
            continue;
        }

        record.pixelLength = ScreenplayPaginator::pixelLength(range.from, range.until, m_document);
        record.pageLength = ScreenplayPaginator::pixelToPageLength(record.pixelLength, m_document);
        record.timeLength = ScreenplayPaginator::pixelToTimeLength(record.pixelLength, m_format, m_document);
        record.pageBreaks = this->evaluateScenePageBreaks(range, lastPageNr);

        serialNumberMap[sceneContent.serialNumber] = records.size();

        records.append(record);
    }

    // Reconcile lengths for break elements.
    for (ScreenplayPaginatorRecord &record : records) {
        if (!breakSerialNumbers.contains(record.serialNumber))
            continue;

        const QList<int> elementSerialNumbers = breakSerialNumbers.value(record.serialNumber);
        for (int elementSerialNumber : elementSerialNumbers) {
            int recordIndex = serialNumberMap.value(elementSerialNumber, -1);
            if (recordIndex >= 0 && recordIndex < records.size()) {
                record.pixelLength += records[recordIndex].pixelLength;
                record.pageLength += records[recordIndex].pageLength;
            }
        }
        record.timeLength = ScreenplayPaginator::pageToTimeLength(record.pageLength, m_format, m_document);
    }

    m_document->setProperty(PaginatorDocumentInsights::property,
                            QVariant::fromValue<PaginatorDocumentInsights>(insights));

    // Calculate totals
    const qreal pixelLength = ScreenplayPaginator::pixelLength(m_document);
    const int pageCount = qMax(qCeil(ScreenplayPaginator::pixelToPageLength(pixelLength, m_document)), 1);
    const QTime totalTime = ScreenplayPaginator::pixelToTimeLength(pixelLength, m_format, m_document);

    // All done, emit result.
    emit paginationComplete(records, pixelLength, pageCount, totalTime);

#if 1
    QTextDocumentWriter writer(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/scrite.odt");
    writer.write(m_document);

    QPdfWriter pdfWriter(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/scrite.pdf");
    m_document->print(&pdfWriter);
#endif
}

void ScreenplayPaginatorWorker::scheduleSyncDocument(const char *purpose)
{
    if (m_synchronousSync) {
        this->syncDocument();
        return;
    }

    if (m_syncDocumentTimer == nullptr) {
        m_syncDocumentTimer = new QTimer(this);
        m_syncDocumentTimer->setSingleShot(true);
        m_syncDocumentTimer->setInterval(ScreenplayPaginatorWorker::syncInterval);

        connect(m_syncDocumentTimer, &QTimer::timeout, this, &ScreenplayPaginatorWorker::syncDocument);
        connect(QThread::currentThread(), &QThread::finished, m_syncDocumentTimer, &QTimer::stop);
    }

#if 0
    Utils::Gui::log(
            QStringLiteral("ScreenplayPaginatorWorker::scheduleSyncDocument(%1)").arg(purpose ? purpose : "NONE"));
#else
    Q_UNUSED(purpose)
#endif

    m_syncDocumentTimer->start();
}

qreal ScreenplayPaginatorWorker::cursorPixelOffset(int cursorPosition, int currentSerialNumber) const
{
#if 0
    PROFILE_THIS_FUNCTION2;
#endif

    if (m_document == nullptr || cursorPosition < 0 || currentSerialNumber < 0)
        return 0;

    const PaginatorDocumentInsights insights =
            m_document->property(PaginatorDocumentInsights::property).value<PaginatorDocumentInsights>();
    if (insights.isEmpty())
        return 0;

    PaginatorDocumentInsights::BlockRange range = insights.findBlockRangeBySerialNumber(currentSerialNumber);
    if (!range.isValid())
        return 0;

    QTextBlock block = range.from;
    ScreenplayPaginatorBlockData *blockData = ScreenplayPaginatorBlockData::get(block);
    if (blockData == nullptr)
        return 0;

    if (blockData->paragraphId.isEmpty())
        block = block.next();

    cursorPosition += block.position();

    QTextCursor cursor(m_document);
    cursor.setPosition(cursorPosition);

    return this->cursorPixelOffset(cursor);
}

qreal ScreenplayPaginatorWorker::cursorPixelOffset(const QTextCursor &cursor) const
{
    QTextBlock block = cursor.block();
    if (!block.isValid())
        return 0;

    QTextLayout *layout = block.layout();
    if (layout == nullptr)
        return 0;

    int relativePos = cursor.position() - block.position();
    QTextLine line = layout->lineForTextPosition(relativePos);
    if (!line.isValid())
        return layout->position().y();

    return layout->position().y() + line.y() + line.height() / 2;
}

QList<ScenePageBreak>
ScreenplayPaginatorWorker::evaluateScenePageBreaks(const PaginatorDocumentInsights::BlockRange &range,
                                                   int &lastPageNumber) const
{
    QList<ScenePageBreak> ret;
    if (QThread::currentThread()->isFinished() || QThread::currentThread()->isInterruptionRequested())
        return ret;

    if (m_document == nullptr || m_document->isEmpty() || !range.isValid())
        return ret;

    if (range.from.document() != m_document || range.until.document() != m_document)
        return ret;

    QTextBlock block = range.from;
    ScreenplayPaginatorBlockData *blockData = ScreenplayPaginatorBlockData::get(block);
    if (blockData == nullptr)
        return ret;

    if (blockData->paragraphId.isEmpty()) {
        // Check if scene heading of this scene is the first line of the current page
        QTextCursor cursor(block);
        qreal pixelOffset = this->cursorPixelOffset(cursor);
        int cursorPageNr = qMax(qCeil(ScreenplayPaginator::pixelToPageLength(pixelOffset, m_document)), 1);
        if (cursorPageNr > lastPageNumber) {
            ret.append(ScenePageBreak(-1, cursorPageNr));
            lastPageNumber = cursorPageNr;
        }

        block = block.next();
    }

    const int firstPosition = block.position();
    const int lastPosition = [=]() -> int {
        QTextCursor cursor(range.until);
        cursor.movePosition(QTextCursor::EndOfBlock);
        return cursor.position();
    }();
    if (firstPosition >= lastPosition)
        return ret; // Something is really odd

    QTextCursor cursor(block);

    while (cursor.position() < lastPosition || cursor.atEnd()) {
        if (QThread::currentThread()->isFinished() || QThread::currentThread()->isInterruptionRequested())
            return QList<ScenePageBreak>();

        qreal pixelOffset = this->cursorPixelOffset(cursor);
        int cursorPageNr = qMax(qCeil(ScreenplayPaginator::pixelToPageLength(pixelOffset, m_document)), 1);
        if (cursorPageNr > lastPageNumber) {
            ret.append(ScenePageBreak(cursor.position() - firstPosition, cursorPageNr));
            lastPageNumber = cursorPageNr;
        }

        if (!cursor.movePosition(QTextCursor::Down))
            break;
    }

    return ret;
}
