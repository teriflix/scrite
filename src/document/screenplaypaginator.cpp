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

#include "screenplaypaginator.h"
#include "screenplaypaginatorworker.h"
#include "scritedocument.h"
#include "utils.h"

#include <QThread>
#include <QAbstractTextDocumentLayout>

ScenePageBreak::ScenePageBreak() { }

ScenePageBreak::ScenePageBreak(const ScenePageBreak &other)
{
    *this = other;
}

bool ScenePageBreak::operator==(const ScenePageBreak &other) const
{
    return this->cursorPosition == other.cursorPosition && this->pageNumber == other.pageNumber;
}

bool ScenePageBreak::operator!=(const ScenePageBreak &other) const
{
    return !(*this == other);
}

ScenePageBreak &ScenePageBreak::operator=(const ScenePageBreak &other)
{
    this->cursorPosition = other.cursorPosition;
    this->pageNumber = other.pageNumber;
    return *this;
}

ScreenplayPaginatorRecord::ScreenplayPaginatorRecord() { }

ScreenplayPaginatorRecord::ScreenplayPaginatorRecord(const ScreenplayPaginatorRecord &other)
{
    *this = other;
}

bool ScreenplayPaginatorRecord::operator==(const ScreenplayPaginatorRecord &other) const
{
    return this->serialNumber == other.serialNumber
            && qFuzzyCompare(this->pixelLength, other.pixelLength)
            && this->firstCursorPosition == other.firstCursorPosition
            && this->firstParagraphCursorPosition == other.firstParagraphCursorPosition
            && this->lastCursorPosition == other.lastCursorPosition
            && this->pageLength == other.pageLength && this->timeLength == other.timeLength
            && this->pixelOffset == other.pixelOffset && this->pageOffset == other.pageOffset
            && this->timeOffset == other.timeOffset && this->pageBreaks == other.pageBreaks
            && this->screenplayElement == other.screenplayElement;
}

bool ScreenplayPaginatorRecord::operator!=(const ScreenplayPaginatorRecord &other) const
{
    return !(*this == other);
}

ScreenplayPaginatorRecord &
ScreenplayPaginatorRecord::operator=(const ScreenplayPaginatorRecord &other)
{
    this->serialNumber = other.serialNumber;
    this->firstCursorPosition = other.firstCursorPosition;
    this->firstParagraphCursorPosition = other.firstParagraphCursorPosition;
    this->lastCursorPosition = other.lastCursorPosition;
    this->pixelLength = other.pixelLength;
    this->pageLength = other.pageLength;
    this->timeLength = other.timeLength;
    this->pixelOffset = other.pixelOffset;
    this->pageOffset = other.pageOffset;
    this->timeOffset = other.timeOffset;
    this->pageBreaks = other.pageBreaks;
    this->screenplayElement = other.screenplayElement;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayPaginator::ScreenplayPaginator(QObject *parent) : QObject(parent)
{
    m_workerThread = new QThread(this);

    m_worker = new ScreenplayPaginatorWorker;
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();

    m_workerNode = new ScreenplayPaginatorWorkerNode(this);
    connect(m_workerNode, &ScreenplayPaginatorWorkerNode::paginationComplete, this,
            &ScreenplayPaginator::onPaginationComplete);
    connect(m_workerNode, &ScreenplayPaginatorWorkerNode::cursorQueryResponse, this,
            &ScreenplayPaginator::onCursorQueryResponse);
    m_workerNode->setWorker(m_worker);

    connect(this, &ScreenplayPaginator::enabledChanged, this, &ScreenplayPaginator::reset);
    connect(this, &ScreenplayPaginator::formatChanged, this, &ScreenplayPaginator::onFormatChanged);
    connect(this, &ScreenplayPaginator::screenplayChanged, this,
            &ScreenplayPaginator::onScreenplayReset);
    connect(this, &ScreenplayPaginator::cursorPositionChanged, this,
            &ScreenplayPaginator::onCursorPositionChanged);

    QTimer::singleShot(0, this, &ScreenplayPaginator::useDefaultFormatAndScreenplay);
}

ScreenplayPaginator::~ScreenplayPaginator()
{
    m_workerThread->requestInterruption();
    m_workerThread->quit();
    m_workerThread->wait();
}

void ScreenplayPaginator::useDefaultFormatAndScreenplay()
{
    if (m_componentComplete) {
        if (m_format == nullptr)
            this->setFormat(ScriteDocument::instance()->printFormat());

        if (m_screenplay == nullptr)
            this->setScreenplay(ScriteDocument::instance()->screenplay());
    }
}

void ScreenplayPaginator::setScreenplay(Screenplay *val)
{
    if (m_screenplay == val)
        return;

    if (m_screenplay != nullptr)
        disconnect(m_screenplay, nullptr, this, nullptr);

    this->clear();

    m_screenplay = val;

    if (m_screenplay != nullptr) {
        connect(m_screenplay, &Screenplay::aboutToDelete, this,
                &ScreenplayPaginator::onScreenplayDestroyed);
        connect(m_screenplay, &Screenplay::modelReset, this,
                &ScreenplayPaginator::onScreenplayReset);
        connect(m_screenplay, &Screenplay::elementInserted, this,
                &ScreenplayPaginator::onScreenplayElementInserted);
        connect(m_screenplay, &Screenplay::elementRemoved, this,
                &ScreenplayPaginator::onScreenplayElementRemoved);
        connect(m_screenplay, &Screenplay::elementOmitted, this,
                &ScreenplayPaginator::onScreenplayElementOmitted);
        connect(m_screenplay, &Screenplay::elementIncluded, this,
                &ScreenplayPaginator::onScreenplayElementIncluded);
        connect(m_screenplay, &Screenplay::elementSceneContentChanged, this,
                &ScreenplayPaginator::onScreenplayElementSceneReset);
        connect(m_screenplay, &Screenplay::elementSceneHeadingChanged, this,
                &ScreenplayPaginator::onScreenplayElementSceneHeadingChanged);
        connect(m_screenplay, &Screenplay::elementSceneElementChanged, this,
                &ScreenplayPaginator::onScreenplayElementSceneElementChanged);
        connect(m_screenplay, &Screenplay::currentElementIndexChanged, this,
                &ScreenplayPaginator::onCursorPositionChanged);
    }

    emit screenplayChanged();
}

void ScreenplayPaginator::setFormat(ScreenplayFormat *val)
{
    if (m_format == val)
        return;

    if (m_format != nullptr)
        disconnect(m_format, nullptr, this, nullptr);

    m_format = val;

    if (m_format != nullptr)
        connect(m_format, &ScreenplayFormat::formatChanged, this,
                &ScreenplayPaginator::onFormatChanged);

    this->clear();

    emit formatChanged();
}

void ScreenplayPaginator::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    this->clear();

    emit enabledChanged();
}

void ScreenplayPaginator::reset()
{
    this->clear();
    this->onFormatChanged();
    this->onScreenplayReset();
    this->onCursorPositionChanged();
}

bool ScreenplayPaginator::paginateIntoDocument(const Screenplay *screenplay,
                                               const ScreenplayFormat *format,
                                               QTextDocument *document)
{
    ScreenplayPaginatorWorker worker(document, const_cast<ScreenplayFormat *>(format));
    worker.setSynchronousSync(true);
    if (format)
        worker.useFormat(QObjectSerializer::toJson(format));
    if (screenplay)
        worker.reset(SceneContent::fromScreenplay(screenplay));
    return document;
}

QTextDocument *ScreenplayPaginator::paginatedDocument(const Screenplay *screenplay,
                                                      const ScreenplayFormat *format,
                                                      QObject *documentParent)
{
    if (screenplay == nullptr || format == nullptr)
        return nullptr;

    if (screenplay->thread() != QThread::currentThread()
        || format->thread() != QThread::currentThread())
        return nullptr;

    if (documentParent && documentParent->thread() != QThread::currentThread())
        return nullptr;

    QTextDocument *document = new QTextDocument(documentParent);

    if (paginateIntoDocument(screenplay, format, document))
        return document;

    delete document;
    return nullptr;
}

qreal ScreenplayPaginator::pixelLength(const ScreenplayElement *element,
                                       const QTextDocument *document)
{
    if (document == nullptr || element == nullptr || element->isOmitted())
        return 0;

    if (document->thread() != QThread::currentThread()
        || element->thread() != QThread::currentThread())
        return 0;

    const PaginatorDocumentInsights insights =
            document->property(PaginatorDocumentInsights::property)
                    .value<PaginatorDocumentInsights>();
    if (insights.isEmpty())
        return 0;

    if (element->elementType() == ScreenplayElement::BreakElementType) {
        const Screenplay *screenplay = element->screenplay();
        ScreenplayElement *ncElement = const_cast<ScreenplayElement *>(element);
        const QList<int> elementIndexes = screenplay->sceneElementsInBreak(ncElement);
        qreal breakLength = 0;
        for (int index : elementIndexes) {
            const ScreenplayElement *sceneInBreak = screenplay->elementAt(index);
            if (sceneInBreak->elementType() == ScreenplayElement::SceneElementType)
                breakLength += pixelLength(sceneInBreak, document);
        }

        return breakLength;
    }

    const PaginatorDocumentInsights::BlockRange range =
            insights.findBlockRangeBySerialNumber(element->serialNumber());
    if (!range.isValid())
        return 0;

    return pixelLength(range.from, range.until, document);
}

qreal ScreenplayPaginator::pixelLength(const Scene *scene, const QTextDocument *document)
{
    if (document == nullptr || scene == nullptr)
        return 0;

    if (document->thread() != QThread::currentThread()
        || scene->thread() != QThread::currentThread())
        return 0;

    const PaginatorDocumentInsights insights =
            document->property(PaginatorDocumentInsights::property)
                    .value<PaginatorDocumentInsights>();
    if (insights.isEmpty())
        return 0;

    const PaginatorDocumentInsights::BlockRange range =
            insights.findBlockRangeBySceneId(scene->id());
    if (!range.isValid())
        return 0;

    return pixelLength(range.from, range.until, document);
}

qreal ScreenplayPaginator::pixelLength(const SceneHeading *sceneHeading,
                                       const QTextDocument *document)
{
    if (document == nullptr || sceneHeading == nullptr || sceneHeading->scene() == nullptr)
        return 0;

    if (document->thread() != QThread::currentThread()
        || sceneHeading->thread() != QThread::currentThread())
        return 0;

    const PaginatorDocumentInsights insights =
            document->property(PaginatorDocumentInsights::property)
                    .value<PaginatorDocumentInsights>();
    if (insights.isEmpty())
        return 0;

    const PaginatorDocumentInsights::BlockRange range =
            insights.findBlockRangeBySceneId(sceneHeading->scene()->id());
    if (!range.isValid())
        return 0;

    // Lookup block range corresponding to the scene's paragraphs
    QTextBlock block = range.from;
    while (block.isValid()) {
        ScreenplayPaginatorBlockData *data = ScreenplayPaginatorBlockData::get(block);
        if (data && data->sceneId == sceneHeading->scene()->id() && data->paragraphId.isEmpty())
            break;

        if (block == range.until) {
            block = QTextBlock();
            break;
        }

        block = block.next();
    }

    return pixelLength(block, block, document);
}

qreal ScreenplayPaginator::pixelLength(const SceneElement *paragraph, const QTextDocument *document)
{
    if (document == nullptr || paragraph == nullptr || paragraph->scene() == nullptr)
        return 0;

    if (document->thread() != QThread::currentThread()
        || paragraph->thread() != QThread::currentThread())
        return 0;

    const PaginatorDocumentInsights insights =
            document->property(PaginatorDocumentInsights::property)
                    .value<PaginatorDocumentInsights>();
    if (insights.isEmpty())
        return 0;

    const PaginatorDocumentInsights::BlockRange range =
            insights.findBlockRangeBySceneId(paragraph->scene()->id());
    if (!range.isValid())
        return 0;

    // Lookup block range corresponding to the scene's paragraphs
    QTextBlock block = range.from;
    while (block.isValid()) {
        ScreenplayPaginatorBlockData *data = ScreenplayPaginatorBlockData::get(block);
        if (data && data->sceneId == paragraph->scene()->id()
            && data->paragraphId == paragraph->id())
            break;

        if (block == range.until) {
            block = QTextBlock();
            break;
        }

        block = block.next();
    }

    return pixelLength(block, block, document);
}

qreal ScreenplayPaginator::pixelLength(const QTextDocument *document)
{
    if (document == nullptr)
        return 0;

    if (document->thread() != QThread::currentThread())
        return 0;

    const QAbstractTextDocumentLayout *layout = document->documentLayout();
    return layout->documentSize().height();
}

qreal ScreenplayPaginator::pixelLength(const QTextBlock &from, const QTextBlock &until,
                                       const QTextDocument *document)
{
    if (!from.isValid() || !until.isValid() || document == nullptr)
        return 0;

    if (document->thread() != QThread::currentThread())
        return 0;

    const QAbstractTextDocumentLayout *layout = document->documentLayout();
    const QRectF fromRect = layout->blockBoundingRect(from);
    const QRectF untilRect = layout->blockBoundingRect(until);
    const QRectF unifiedRect = fromRect.united(untilRect);

    return unifiedRect.height();
}

qreal ScreenplayPaginator::pixelToPageLength(qreal pixelLength, const QTextDocument *document)
{
    if (qFuzzyIsNull(pixelLength) || pixelLength < 0 || document == nullptr)
        return 0;

    if (document->thread() != QThread::currentThread())
        return 0;

    const qreal pageLength = document->pageSize().height();
    if (qFuzzyIsNull(pageLength) || pageLength < 0)
        return 0;

    return pixelLength / pageLength;
}

QTime ScreenplayPaginator::pixelToTimeLength(qreal pixelLength, ScreenplayFormat *format,
                                             const QTextDocument *document)
{
    if (qFuzzyIsNull(pixelLength) || pixelLength < 0 || format == nullptr || document == nullptr)
        return QTime();

    if (document->thread() != QThread::currentThread()
        || format->thread() != QThread::currentThread())
        return QTime();

    return Utils::TMath::secondsToTime(
            qCeil(qreal(format->secondsPerPage()) * pixelToPageLength(pixelLength, document)));
}

QTime ScreenplayPaginator::pageToTimeLength(qreal pageLength, ScreenplayFormat *format,
                                            const QTextDocument *document)
{
    if (qFuzzyIsNull(pageLength) || pageLength < 0 || format == nullptr || document == nullptr)
        return QTime();

    if (document->thread() != QThread::currentThread()
        || format->thread() != QThread::currentThread())
        return QTime();

    return Utils::TMath::secondsToTime(qCeil(qreal(format->secondsPerPage()) * pageLength));
}

void ScreenplayPaginator::setCursorPosition(int val)
{
    if (m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    emit cursorPositionChanged();
}

int ScreenplayPaginator::indexOf(ScreenplayElement *element) const
{
    if (element == nullptr || element->screenplay() != m_screenplay || m_screenplay == nullptr)
        return -1;

    auto it = std::find_if(m_records.begin(), m_records.end(),
                           [element](const ScreenplayPaginatorRecord &record) {
                               return record.serialNumber == element->serialNumber();
                           });
    if (it != m_records.end())
        return std::distance(m_records.begin(), it);

    return -1;
}

ScreenplayPaginatorRecord ScreenplayPaginator::recordAt(int row) const
{
    return row >= 0 && row < m_records.size() ? m_records.at(row) : ScreenplayPaginatorRecord();
}

qreal ScreenplayPaginator::pixelLength(ScreenplayElement *from, ScreenplayElement *until) const
{
    qreal pixelLength = 0;
    if (this->aggregate(from, until, &pixelLength, nullptr, nullptr))
        return pixelLength;

    return 0;
}

qreal ScreenplayPaginator::pageLength(ScreenplayElement *from, ScreenplayElement *until) const
{
    qreal pageLength = 0;
    if (this->aggregate(from, until, nullptr, &pageLength, nullptr))
        return pageLength;

    return 0;
}

QTime ScreenplayPaginator::timeLength(ScreenplayElement *from, ScreenplayElement *until) const
{
    QTime timeLength = QTime(0, 0, 0);
    if (this->aggregate(from, until, nullptr, nullptr, &timeLength))
        return timeLength;

    return QTime(0, 0, 0);
}

void ScreenplayPaginator::classBegin()
{
    m_componentComplete = false;
}

void ScreenplayPaginator::componentComplete()
{
    m_componentComplete = true;

    this->useDefaultFormatAndScreenplay();
}

void ScreenplayPaginator::clear()
{
    this->clearRecords();
    this->clearCursor();
}

void ScreenplayPaginator::clearRecords()
{
    if (!m_records.isEmpty() || m_pageCount >= 0 || m_totalTime.isValid()
        || m_totalPixelLength > 0) {
        m_records.clear();
        m_pageCount = 0;
        m_totalTime = QTime(0, 0, 0);
        m_totalPixelLength = 0;
        emit paginationUpdated();
    }
}

void ScreenplayPaginator::clearCursor()
{
    if (m_cursorPage >= 0 || m_cursorTime.isValid() || m_cursorPixelOffset > 0) {
        m_cursorPage = 0;
        m_cursorTime = QTime(0, 0, 0);
        m_cursorPixelOffset = 0;
        emit cursorUpdated();
    }
}

void ScreenplayPaginator::incrementSyncCounter()
{
    ++m_syncCounter;
    if (m_syncCounter == 1)
        emit syncingChanged();
}

void ScreenplayPaginator::resetSyncCounter()
{
    if (m_syncCounter > 0) {
        m_syncCounter = 0;
        emit syncingChanged();
    }
}

void ScreenplayPaginator::onFormatChanged()
{
    if (!m_enabled)
        return;

    ScreenplayFormat *format =
            m_format == nullptr ? ScriteDocument::instance()->printFormat() : m_format;

    const QJsonObject formatJson = QObjectSerializer::toJson(format);
    this->incrementSyncCounter();
    m_workerNode->useFormat(formatJson);
}

void ScreenplayPaginator::onScreenplayReset()
{
    if (!m_enabled)
        return;

    QList<SceneContent> screenplayContent = SceneContent::fromScreenplay(m_screenplay);
    this->incrementSyncCounter();
    m_workerNode->reset(screenplayContent);
}

void ScreenplayPaginator::onScreenplayDestroyed()
{
    disconnect(m_screenplay, nullptr, this, nullptr);

    m_screenplay = nullptr;
    emit screenplayChanged();
}

void ScreenplayPaginator::onScreenplayElementInserted(ScreenplayElement *element, int index)
{
    if (!m_enabled)
        return;

    SceneContent sceneContent = SceneContent::fromScreenplayElement(element);
    this->incrementSyncCounter();
    m_workerNode->insertElement(index, sceneContent);
}

void ScreenplayPaginator::onScreenplayElementRemoved(ScreenplayElement *element, int index)
{
    if (!m_enabled)
        return;

    Q_UNUSED(element)
    this->incrementSyncCounter();
    m_workerNode->removeElement(index);
}

void ScreenplayPaginator::onScreenplayElementOmitted(ScreenplayElement *element, int index)
{
    if (!m_enabled)
        return;

    Q_UNUSED(element)
    this->incrementSyncCounter();
    m_workerNode->omitElement(index);
}

void ScreenplayPaginator::onScreenplayElementIncluded(ScreenplayElement *element, int index)
{
    if (!m_enabled)
        return;

    Q_UNUSED(element)
    this->incrementSyncCounter();
    m_workerNode->includeElement(index);
}

void ScreenplayPaginator::onScreenplayElementSceneReset(ScreenplayElement *element, Scene *scene)
{
    if (!m_enabled)
        return;

    Q_UNUSED(scene)
    SceneContent sceneContent = SceneContent::fromScreenplayElement(element);
    this->incrementSyncCounter();
    m_workerNode->updateScene(sceneContent);
}

void ScreenplayPaginator::onScreenplayElementSceneHeadingChanged(ScreenplayElement *element,
                                                                 SceneHeading *sceneHeading)
{
    if (!m_enabled)
        return;

    Q_UNUSED(element)
    SceneParagraph paragraph = SceneParagraph::fromSceneHeading(sceneHeading);
    this->incrementSyncCounter();
    m_workerNode->updateParagraph(paragraph);
}

void ScreenplayPaginator::onScreenplayElementSceneElementChanged(ScreenplayElement *element,
                                                                 SceneElement *sceneElement)
{
    if (!m_enabled)
        return;

    Q_UNUSED(element)
    SceneParagraph paragraph = SceneParagraph::fromSceneElement(sceneElement);
    this->incrementSyncCounter();
    m_workerNode->updateParagraph(paragraph);
}

void ScreenplayPaginator::onCursorPositionChanged()
{
    if (!m_enabled)
        return;

    if (m_cursorPosition < 0) {
        this->clearCursor();
        return;
    }

    const int currentElementIndex = m_screenplay->currentElementIndex();
    const ScreenplayElement *currentElement =
            currentElementIndex >= 0 ? m_screenplay->elementAt(currentElementIndex) : nullptr;
    if (currentElement == nullptr) {
        this->clearCursor();
        return;
    }

    const int currentSerialNumber = currentElement->serialNumber();
    m_workerNode->queryCursor(m_cursorPosition, currentSerialNumber);
}

void ScreenplayPaginator::onCursorQueryResponse(int cursorPosition, qreal cursorPixel,
                                                int cursorPageNumber, qreal cursorPage,
                                                const QTime &cursorTime,
                                                const ScreenplayPaginatorRecord &cursorRecord)
{
    if (m_cursorPosition == cursorPosition) {
        m_cursorPage = cursorPageNumber;
        m_cursorTime = cursorTime;
        m_cursorPixelOffset = cursorPixel;
        m_cursorRecord = cursorRecord;
        emit cursorUpdated();
        emit cursorQueryResponse(cursorPosition, cursorPixel, cursorPageNumber, cursorPage,
                                 cursorTime, cursorRecord);
    }
}

void ScreenplayPaginator::onPaginationComplete(const QList<ScreenplayPaginatorRecord> &records,
                                               qreal pixelLength, int pageCount,
                                               const QTime &totalTime)
{
    m_records = records;
    m_pageCount = pageCount;
    m_totalTime = totalTime;
    m_totalPixelLength = pixelLength;

    if (m_screenplay != nullptr) {
        QMap<int, int> serialNumberMap;
        for (int r = 0; r < m_records.length(); r++)
            serialNumberMap[m_records[r].serialNumber] = r;

        for (int i = 0; i < m_screenplay->elementCount(); i++) {
            ScreenplayElement *screenplayElement = m_screenplay->elementAt(i);
            const int serialNumber = screenplayElement->serialNumber();
            const int recordIndex = serialNumberMap.value(serialNumber, -1);
            if (recordIndex >= 0)
                m_records[recordIndex].screenplayElement = screenplayElement;
        }
    }

    emit paginationUpdated();

    this->resetSyncCounter();
}

bool ScreenplayPaginator::aggregate(ScreenplayElement *from, ScreenplayElement *until,
                                    qreal *pixelLength, qreal *pageLength, QTime *timeLength) const
{
    if (from == nullptr
        || (pixelLength == nullptr && pageLength == nullptr && timeLength == nullptr))
        return false;

    if (pixelLength)
        *pixelLength = 0;
    if (pageLength)
        *pageLength = 0;
    if (timeLength)
        *timeLength = QTime(0, 0, 0);

    int start = this->indexOf(from);
    if (start < 0)
        return false;

    int end = until == nullptr ? start : this->indexOf(until);
    if (end < 0)
        end = start;

    for (int i = start; i <= end; i++) {
        const ScreenplayPaginatorRecord &record = m_records[i];
        if (record.screenplayElement.isNull()
            || record.screenplayElement->elementType() != ScreenplayElement::SceneElementType)
            continue;

        if (pixelLength)
            *pixelLength += record.pixelLength;

        if (pageLength)
            *pageLength += record.pageLength;

        if (timeLength)
            *timeLength = timeLength->addSecs(QTime(0, 0, 0).secsTo(record.timeLength));
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayPaginatorWatcher::ScreenplayPaginatorWatcher(QObject *parent) : QObject(parent)
{
    connect(this, &ScreenplayPaginatorWatcher::paginatorChanged, this,
            &ScreenplayPaginatorWatcher::lookupRecord);
    connect(this, &ScreenplayPaginatorWatcher::elementChanged, this,
            &ScreenplayPaginatorWatcher::lookupRecord);
}

ScreenplayPaginatorWatcher::~ScreenplayPaginatorWatcher() { }

void ScreenplayPaginatorWatcher::setPaginator(ScreenplayPaginator *val)
{
    if (m_paginator == val)
        return;

    if (m_paginator != nullptr)
        disconnect(m_paginator, nullptr, this, nullptr);

    m_paginator = val;

    if (m_paginator != nullptr) {
        connect(m_paginator, &ScreenplayPaginator::paginationUpdated, this,
                &ScreenplayPaginatorWatcher::onPaginationUpdated);
        connect(m_paginator, &ScreenplayPaginator::cursorQueryResponse, this,
                &ScreenplayPaginatorWatcher::onCursorQueryResponse);
    }

    emit paginatorChanged();
}

void ScreenplayPaginatorWatcher::setElement(ScreenplayElement *val)
{
    if (m_element == val)
        return;

    m_element = val;
    emit elementChanged();
}

void ScreenplayPaginatorWatcher::lookupRecord()
{
    if (m_paginator == nullptr || m_element == nullptr) {
        this->setRecord(ScreenplayPaginatorRecord());
        return;
    }

    const int index = m_paginator->indexOf(m_element);
    if (index < 0) {
        this->setRecord(ScreenplayPaginatorRecord());
        return;
    }

    this->setRecord(m_paginator->recordAt(index));
}

void ScreenplayPaginatorWatcher::onPaginationUpdated()
{
    this->lookupRecord();
}

void ScreenplayPaginatorWatcher::setRecord(const ScreenplayPaginatorRecord &val)
{
    if (m_record == val)
        return;

    if (val.isValid() && val.screenplayElement != m_element)
        return;

    m_record = val;
    emit recordChanged();
}

void ScreenplayPaginatorWatcher::onCursorQueryResponse(
        int cursorPosition, qreal cursorPixel, int cursorPageNumber, qreal cursorPage,
        const QTime &cursorTime, const ScreenplayPaginatorRecord &cursorRecord)
{
    Q_UNUSED(cursorPageNumber);

    bool hasCursor = cursorRecord.isValid() && m_record.isValid()
            && m_record.serialNumber == cursorRecord.serialNumber;
    if (!hasCursor && !m_hasCursor) {
        return;
    }

    m_hasCursor = hasCursor;
    if (!m_hasCursor) {
        m_relativeCursorPosition = -1;
        m_relativeCursorPixel = 0;
        m_relativeCursorPage = 0;
        m_relativeCursorTime = QTime();
        emit cursorInfoChanged();
        return;
    }

    m_relativeCursorPosition =
            cursorPosition + (m_record.firstParagraphCursorPosition - m_record.firstCursorPosition);
    m_relativeCursorPixel = cursorPixel - m_record.pixelOffset;
    m_relativeCursorPage = cursorPage - m_record.pageOffset;
    m_relativeCursorTime = QTime(0, 0, 0).addSecs(m_record.timeOffset.secsTo(cursorTime));
    emit cursorInfoChanged();
}
