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

#include "screenplaytextdocumentoffsets.h"
#include "timeprofiler.h"
#include "application.h"

#include <QDir>
#include <QtMath>
#include <QFileInfo>
#include <QJsonDocument>
#include <QVersionNumber>
#include <QtConcurrentRun>
#include <QAbstractTextDocumentLayout>
#include <QJsonDocument>

class OffsetItem
{
public:
    OffsetItem()
    {
        this->setRow(-1);
        this->setType(-1);
        this->setPageNumber(-1);
        this->setPixelOffset(0);
        this->setTimestamp(0);
        this->setDefaultTimestamp(0);
        this->setLocked(false);
        this->setTimeManuallySet(false);
    }
    OffsetItem(const QJsonValue &value) : m_object(value.toObject()) { }
    OffsetItem(const QJsonObject &object) : m_object(object) { }

    QJsonObject json() const { return m_object; }
    QJsonObject &json() { return m_object; }

    bool isValid() const { return !m_object.isEmpty(); }

    int row() const { return m_object.value(rowAttrib).toInt(); }
    int type() const { return m_object.value(typeAttrib).toInt(); }
    QString id() const { return m_object.value(idAttrib).toString(); }
    int paragraphIndex() const { return m_object.value(paraIndexAttrib).toInt(); }
    QString snippet() const { return m_object.value(snippetAttrib).toString(); }
    QString number() const { return m_object.value(numberAttrib).toString(); }
    qreal pixelOffset() const { return m_object.value(pixelOffsetAttrib).toDouble(); }
    int timestamp() const { return m_object.value(timestampAttrib).toInt(); }
    QTime time() const { return QTime(0, 0, 0, 1).addMSecs(this->timestamp() - 1); }
    int defaultTimestamp() const { return m_object.value(defaultTimestampAttrib).toInt(); }
    QTime defaultTime() const { return QTime(0, 0, 0, 1).addMSecs(this->defaultTimestamp() - 1); }
    int pageNumber() const { return m_object.value(pageNumberAttrib).toInt(); }
    QVersionNumber version() const
    {
        return QVersionNumber::fromString(m_object.value(versionAttrib).toString());
    }
    bool isLocked() const { return m_object.value(lockedAttrib).toBool(); }
    bool isTimeManuallySet() const { return m_object.value(timeManuallySet).toBool(); }

    void setRow(int val) { m_object.insert(rowAttrib, val); }
    void setType(int val) { m_object.insert(typeAttrib, val); }
    void setId(const QString &val) { m_object.insert(idAttrib, val); }
    void setParagraphIndex(int val) { m_object.insert(paraIndexAttrib, val); }
    void setSnippet(const QString &val) { m_object.insert(snippetAttrib, val); }
    void setNumber(const QString &val) { m_object.insert(numberAttrib, val); }
    void setPixelOffset(qreal val) { m_object.insert(pixelOffsetAttrib, val); }
    void setTimestamp(int val) { m_object.insert(timestampAttrib, val); }
    void setTime(const QTime &val) { m_object.insert(timestampAttrib, val.msecsSinceStartOfDay()); }
    void setDefaultTimestamp(int val) { m_object.insert(defaultTimestampAttrib, val); }
    void setDefaultTime(const QTime &val)
    {
        m_object.insert(defaultTimestampAttrib, val.msecsSinceStartOfDay());
    }
    void setPageNumber(int val) { m_object.insert(pageNumberAttrib, val); }
    void setVersion(const QVersionNumber &val) { m_object.insert(versionAttrib, val.toString()); }
    void setLocked(bool val) { m_object.insert(lockedAttrib, val); }
    void setTimeManuallySet(bool val) { m_object.insert(timeManuallySet, val); }

    bool canMerge(const OffsetItem &other) const
    {
        return this->id() == other.id() && this->type() == other.type();
    }

    bool mergeFrom(const OffsetItem &other)
    {
        if (!this->canMerge(other))
            return false;

        this->setTimestamp(other.timestamp());
        this->setLocked(other.isLocked());
        this->setTimeManuallySet(other.isTimeManuallySet());

        return true;
    }

    bool operator==(const OffsetItem &other) const { return m_object == other.m_object; }

private:
    QJsonObject m_object;

    static const QString rowAttrib;
    static const QString typeAttrib;
    static const QString idAttrib;
    static const QString paraIndexAttrib;
    static const QString numberAttrib;
    static const QString snippetAttrib;
    static const QString pixelOffsetAttrib;
    static const QString timestampAttrib;
    static const QString defaultTimestampAttrib;
    static const QString pageNumberAttrib;
    static const QString versionAttrib;
    static const QString lockedAttrib;
    static const QString timeManuallySet;
};

const QString OffsetItem::rowAttrib = QStringLiteral("row");
const QString OffsetItem::typeAttrib = QStringLiteral("type");
const QString OffsetItem::idAttrib = QStringLiteral("id");
const QString OffsetItem::paraIndexAttrib = QStringLiteral("paragraphIndex");
const QString OffsetItem::snippetAttrib = QStringLiteral("snippet");
const QString OffsetItem::numberAttrib = QStringLiteral("number");
const QString OffsetItem::pixelOffsetAttrib = QStringLiteral("pixelOffset");
const QString OffsetItem::timestampAttrib = QStringLiteral("timestamp");
const QString OffsetItem::defaultTimestampAttrib = QStringLiteral("defaultTimestamp");
const QString OffsetItem::pageNumberAttrib = QStringLiteral("pageNumber");
const QString OffsetItem::versionAttrib = QStringLiteral("version");
const QString OffsetItem::lockedAttrib = QStringLiteral("locked");
const QString OffsetItem::timeManuallySet = QStringLiteral("timeManuallySet");

inline QString timeToString(const QTime &t)
{
    if (t == QTime(0, 0, 0))
        return QStringLiteral("0:00");

    if (t.hour() > 0)
        return t.toString(QStringLiteral("H:mm:ss"));

    return t.toString(QStringLiteral("m:ss"));
}

ScreenplayTextDocumentOffsets::ScreenplayTextDocumentOffsets(QObject *parent)
    : GenericArrayModel(parent),
      m_screenplay(this, "screenplay"),
      m_document(this, "document"),
      m_format(this, "format")
{
    m_reloadTimer = new QTimer(this);
    m_reloadTimer->setInterval(0);
    m_reloadTimer->setSingleShot(true);
    connect(m_reloadTimer, &QTimer::timeout, this, &ScreenplayTextDocumentOffsets::reloadDocument);

    m_document = new QTextDocument(this);
}

ScreenplayTextDocumentOffsets::~ScreenplayTextDocumentOffsets() { }

void ScreenplayTextDocumentOffsets::setScreenplay(Screenplay *val)
{
    if (m_screenplay == val)
        return;

    if (!m_screenplay.isNull())
        m_screenplay->disconnect(this);

    m_screenplay = val;
    emit screenplayChanged();

    if (!m_screenplay.isNull())
        connect(m_screenplay, SIGNAL(screenplayChanged()), m_reloadTimer, SLOT(start()));

    m_reloadTimer->start();
}

void ScreenplayTextDocumentOffsets::setDocument(QTextDocument *val)
{
    if (m_document == val)
        return;

    if (!m_document.isNull() && m_document->parent() == this)
        m_document->deleteLater();

    if (val == nullptr)
        val = new QTextDocument(this);

    m_document = val;
    emit documentChanged();

    m_reloadTimer->start();
}

void ScreenplayTextDocumentOffsets::setFormat(ScreenplayFormat *val)
{
    if (m_format == val)
        return;

    if (!m_format.isNull())
        m_format->disconnect(this);

    m_format = val;
    emit formatChanged();

    if (!m_format.isNull())
        connect(m_format, SIGNAL(formatChanged()), m_reloadTimer, SLOT(start()));

    m_reloadTimer->start();
}

void ScreenplayTextDocumentOffsets::setFileName(const QString &val)
{
    if (m_fileName == val)
        return;

    m_fileName = val;
    emit fileNameChanged();

    if (QFile::exists(val))
        this->loadOffsets();
    else
        this->saveOffsets();
}

QString ScreenplayTextDocumentOffsets::fileNameFrom(const QString &mediaFileNameOrUrl) const
{
    QString mediaFileName;
    if (mediaFileNameOrUrl.startsWith(QStringLiteral("file://")))
        mediaFileName = QUrl(mediaFileNameOrUrl).toLocalFile();
    else
        mediaFileName = mediaFileNameOrUrl;

    QFileInfo fi(mediaFileName);
    return fi.absoluteDir().absoluteFilePath(fi.completeBaseName()
                                             + QStringLiteral(" Scrited View Offsets.json"));
}

QString ScreenplayTextDocumentOffsets::timestampToString(int timeInMs) const
{
    if (timeInMs <= 0)
        return QStringLiteral("0:00 min");

    return timeToString(QTime(0, 0, 0, 1).addMSecs(timeInMs - 1));
}

QJsonObject ScreenplayTextDocumentOffsets::offsetInfoAtPoint(const QPointF &pos) const
{
    const QJsonArray &offsets = this->internalArray();

    OffsetItem ret;
    if (offsets.isEmpty() || pos.x() < 0 || pos.x() >= m_document->textWidth())
        return ret.json();

    if (offsets.size() > 1) {
        for (int i = 0; i < offsets.size(); i++) {
            const OffsetItem item(offsets[i].toObject());
            if (item.pixelOffset() >= pos.y()) {
                if (qFuzzyCompare(item.pixelOffset(), pos.y()))
                    return offsets[i].toObject();

                return offsets[qMax(i - 1, 0)].toObject();
            }
        }
    }

    return offsets.last().toObject();
}

QJsonObject ScreenplayTextDocumentOffsets::offsetInfoAtTime(int timeInMs, int rowHint) const
{
    const QJsonArray &offsets = this->internalArray();

    OffsetItem ret;
    if (offsets.isEmpty() || timeInMs < 0)
        return ret.json();

    if (offsets.size() > 1) {
        const int startRow = qBound(0, rowHint, offsets.size() - 1);
        for (int i = startRow; i < offsets.size(); i++) {
            const OffsetItem item(offsets[i].toObject());

            if (item.timestamp() >= timeInMs) {
                if (item.timestamp() == timeInMs)
                    return offsets[i].toObject();

                return offsets[qMax(i - 1, 0)].toObject();
            }
        }
    }

    return offsets.last().toObject();
}

const qreal lastScenePixelLength = 20.0;
const int lastSceneTimeLength = 500;

int ScreenplayTextDocumentOffsets::evaluateTimeAtPoint(const QPointF &pos, int rowHint) const
{
    const QJsonArray &offsets = this->internalArray();

    if (offsets.isEmpty() || pos.y() < 0)
        return 0;

    if (qFuzzyIsNull(pos.y()))
        return 0;

    const OffsetItem lastOffset(offsets.last());
    if (pos.y() >= lastOffset.pixelOffset() + lastScenePixelLength)
        return lastOffset.timestamp() + lastSceneTimeLength;

    if (rowHint < 0) {
        const OffsetItem item(this->offsetInfoAtPoint(pos));
        rowHint = item.row();
    }

    auto computeTime = [](const qreal p1, const qreal p, const qreal p2, const QTime &t1,
                          const QTime &t2) {
        return t1.msecsSinceStartOfDay()
                + qAbs(((p - p1) / (p2 - p1))
                       * qreal(t2.msecsSinceStartOfDay() - t1.msecsSinceStartOfDay()));
    };

    if (rowHint >= 0 && rowHint < offsets.size()) {
        const OffsetItem i1(offsets[rowHint]);
        const OffsetItem i2(offsets[qMin(rowHint + 1, offsets.size() - 1)]);

        const qreal cpo = i1.pixelOffset();
        const qreal npo =
                i2.pixelOffset() + (rowHint < offsets.size() - 1 ? 0 : lastScenePixelLength);
        const QTime t1 = i1.time();
        const QTime t2 =
                rowHint < offsets.size() - 1 ? i2.time() : i2.time().addMSecs(lastSceneTimeLength);
        if (cpo <= pos.y() && pos.y() <= npo)
            return computeTime(cpo, pos.y(), npo, t1, t2);
    }

    return 0;
}

QPointF ScreenplayTextDocumentOffsets::evaluatePointAtTime(int timeInMs, int rowHint) const
{
    const QJsonArray &offsets = this->internalArray();
    if (offsets.isEmpty() || timeInMs <= 0)
        return QPointF(10, 0);

    const OffsetItem lastOffset(offsets.last());
    if (timeInMs >= lastOffset.timestamp() + lastSceneTimeLength)
        return QPointF(10, lastOffset.pixelOffset() + lastScenePixelLength);

    if (rowHint < 0) {
        const OffsetItem item(this->offsetInfoAtTime(timeInMs));
        rowHint = item.row();
    }

    auto computePoint = [](int t1, int t, int t2, qreal p1, qreal p2) {
        return QPointF(10, p1 + ((qreal(t - t1) / qreal(t2 - t1)) * (p2 - p1)));
    };

    if (rowHint >= 0 && rowHint < offsets.size()) {
        const OffsetItem i1(offsets[rowHint]);
        const OffsetItem i2(offsets[qMin(rowHint + 1, offsets.size() - 1)]);

        const int ct = i1.timestamp();
        const int nt = rowHint < offsets.size() - 1 ? i2.timestamp()
                                                    : i2.timestamp() + lastSceneTimeLength;
        const qreal p1 = i1.pixelOffset();
        const qreal p2 =
                i2.pixelOffset() + (rowHint < offsets.size() - 1 ? 0 : lastScenePixelLength);
        if (ct <= timeInMs && timeInMs <= nt)
            return computePoint(ct, timeInMs, nt, p1, p2);
    }

    return QPointF(10, 0);
}

void ScreenplayTextDocumentOffsets::setTime(int row, int timeInMs, bool adjustFollowingRows)
{
    QJsonArray &offsets = this->internalArray();
    if (row < 0 || row >= offsets.size())
        return;

    ModelDataChangedTracker tracker(this);

    OffsetItem rowOffsetItem;

    int timeDiffInMs = 0;
    for (int i = row; i < offsets.size(); i++) {
        OffsetItem item(offsets[i]);
        if (item.isLocked())
            break;

        if (i == row) {
            timeDiffInMs = timeInMs - item.defaultTimestamp();
            rowOffsetItem = item;
        }

        item.setTimestamp(item.defaultTimestamp() + timeDiffInMs);
        if (i == row)
            item.setTimeManuallySet(true);

        offsets[i] = item.json();
        tracker.changeRow(i);

        if (!adjustFollowingRows)
            break;
    }

    if (row > 0 && rowOffsetItem.type() != SceneElement::Heading) {
        for (int i = row - 1; i >= 0; i--) {
            OffsetItem item(offsets[i]);
            if (item.isTimeManuallySet() || item.type() == SceneElement::Heading || item.isLocked())
                break;

            item.setTimestamp(timeInMs - qAbs(row - i));
            offsets[i] = item.json();
            tracker.changeRow(i);
        }
    }

    this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::resetTime(int row, bool andFollowingRows)
{
    QJsonArray &offsets = this->internalArray();
    if (row < 0 || row >= offsets.size() || m_format.isNull())
        return;

    ModelDataChangedTracker tracker(this);

    for (int i = row; i < offsets.size(); i++) {
        OffsetItem item(offsets[i]);
        if (item.isLocked())
            break;

        item.setTimestamp(item.defaultTimestamp());
        item.setTimeManuallySet(false);

        offsets[i] = item.json();
        tracker.changeRow(i);

        if (!andFollowingRows)
            break;
    }

    this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::toggleSceneTimeLock(int row)
{
    QJsonArray &offsets = this->internalArray();
    if (row < 0 || row >= offsets.size())
        return;

    OffsetItem item(offsets[row]);
    item.setLocked(!item.isLocked());
    offsets[row] = item.json();

    const QModelIndex index = this->index(row);
    emit dataChanged(index, index);

    this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::adjustUnlockedTimes(int duration)
{
    QJsonArray &offsets = this->internalArray();
    if (offsets.isEmpty() || m_document.isNull())
        return;

    QJsonArray lockedOffsets;
    std::copy_if(offsets.begin(), offsets.end(), std::back_inserter(lockedOffsets),
                 [](const QJsonValue &value) { return OffsetItem(value.toObject()).isLocked(); });

    if (lockedOffsets.isEmpty()) {
        this->resetAllTimes();
        return;
    }

    auto adjustRange = [&](int fromRow, int toRow, qreal po1, qreal po2, int ts1, int ts2) {
        if (fromRow >= toRow || po1 >= po2 || ts1 >= ts2)
            return;
        ModelDataChangedTracker tracker(this);
        const qreal msPerPixel = (ts2 - ts1) / (po2 - po1);
        for (int i = fromRow; i <= toRow; i++) {
            OffsetItem item(offsets.at(i));
            if (item.isLocked())
                continue; // Just to be safe.
            const int ts = ts1 + (item.pixelOffset() - po1) * msPerPixel;
            item.setTimestamp(ts);
            offsets[i] = item.json();
            tracker.changeRow(i);
        }
    };

    const OffsetItem firstLockedOffset(lockedOffsets.first());
    if (firstLockedOffset.row() > 0)
        adjustRange(0, firstLockedOffset.row() - 1, 0, firstLockedOffset.pixelOffset(), 0,
                    firstLockedOffset.timestamp());

    for (int i = 0; i < lockedOffsets.size() - 1; i++) {
        const OffsetItem l1(lockedOffsets.at(i));
        const OffsetItem l2(lockedOffsets.at(i + 1));
        if (l2.row() == l1.row() + 1)
            continue;

        adjustRange(l1.row() + 1, l2.row() - 1, l1.pixelOffset(), l2.pixelOffset(), l1.timestamp(),
                    l2.timestamp());
    }

    if (duration > 0) {
        const OffsetItem lastLockedOffset(lockedOffsets.last());
        if (lastLockedOffset.row() < offsets.size() - 1) {
            QAbstractTextDocumentLayout *documentLayout = m_document->documentLayout();
            const qreal contentHeight = documentLayout->documentSize().height();
            adjustRange(lastLockedOffset.row() + 1, offsets.size() - 1,
                        lastLockedOffset.pixelOffset(), contentHeight, lastLockedOffset.timestamp(),
                        duration);
        }
    }

    this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::unlockAllSceneTimes()
{
    QJsonArray &offsets = this->internalArray();
    if (offsets.isEmpty())
        return;

    ModelDataChangedTracker tracker(this);
    for (int i = 0; i < offsets.size(); i++) {
        OffsetItem item(offsets[i]);
        if (item.isLocked()) {
            item.setLocked(false);
            offsets[i] = item.json();
            tracker.changeRow(i);
        }
    }

    this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::resetAllTimes()
{
    QJsonArray &offsets = this->internalArray();
    if (offsets.isEmpty())
        return;

    ModelDataChangedTracker tracker(this);
    for (int i = 0; i < offsets.size(); i++) {
        OffsetItem item(offsets[i]);
        if (!item.isLocked()) {
            item.setTimestamp(item.defaultTimestamp());
            item.setTimeManuallySet(false);
            offsets[i] = item.json();
            tracker.changeRow(i);
        }
    }

    this->saveOffsets();
}

int ScreenplayTextDocumentOffsets::currentSceneHeadingIndex(int row) const
{
    const QJsonArray &offsets = this->internalArray();
    if (row <= 0 || row >= offsets.size())
        return 0;

    const QString sceneID = OffsetItem(offsets[row]).id();

    for (int i = row - 1; i >= 0; i--) {
        OffsetItem item(offsets[i]);
        if (item.type() == SceneElement::Heading && item.id() == sceneID)
            return i;
    }

    return 0;
}

int ScreenplayTextDocumentOffsets::nextSceneHeadingIndex(int row) const
{
    const QJsonArray &offsets = this->internalArray();
    if (row < 0 || row >= offsets.size() - 1)
        return offsets.size() - 1;

    const QString sceneID = OffsetItem(offsets[row]).id();

    for (int i = row + 1; i < offsets.size(); i++) {
        OffsetItem item(offsets[i]);
        if (item.type() == SceneElement::Heading && item.id() != sceneID)
            return i;
    }

    return offsets.size() - 1;
}

int ScreenplayTextDocumentOffsets::previousSceneHeadingIndex(int row) const
{
    const QJsonArray &offsets = this->internalArray();
    if (row <= 0 || row >= offsets.size())
        return 0;

    const QString sceneID = OffsetItem(offsets[row]).id();

    for (int i = row - 1; i >= 0; i--) {
        OffsetItem item(offsets[i]);
        if (item.type() == SceneElement::Heading && item.id() != sceneID)
            return i;
    }

    return 0;
}

void ScreenplayTextDocumentOffsets::setBusy(bool val)
{
    if (m_busy == val)
        return;

    m_busy = val;
    emit busyChanged();
}

inline void polishFontsAndInsertTextAtCursor(QTextCursor &cursor, const QString &text)
{
    TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor, text);
};

void ScreenplayTextDocumentOffsets::reloadDocument()
{
    if (m_format.isNull() || m_screenplay.isNull() || m_document.isNull()
        || m_screenplay->elementCount() == 0) {
        this->setBusy(false);
        this->setArray(QJsonArray());
        return;
    }

    if (m_busy)
        return;

    this->setBusy(true);

    /**
     * We are doing this because we want to allow some time for the UI to update itself
     * and show the busy message, before we get into a long operation.
     */
    QTimer::singleShot(100, this, [=]() {
        this->reloadDocumentNow();
        this->setBusy(false);
    });
}

void ScreenplayTextDocumentOffsets::reloadDocumentNow()
{
    const qreal textWidth = m_format->pageLayout()->contentWidth();
    const qreal contentHeight = m_format->pageLayout()->contentRect().height();
    const qreal msPerPixel = (m_format->secondsPerPage() * 1000) / contentHeight;

    m_document->clear();
    m_document->setTextWidth(textWidth);
    m_document->setDefaultFont(m_format->defaultFont());

    QTextCursor cursor(m_document);
    auto prepareCursor = [=](QTextCursor &cursor, SceneElement::Type paraType,
                             Qt::Alignment overrideAlignment) {
        const SceneElementFormat *format = m_format->elementFormat(paraType);
        QTextBlockFormat blockFormat = format->createBlockFormat(overrideAlignment, &textWidth);
        QTextCharFormat charFormat = format->createCharFormat(&textWidth);
        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
    };

    const QString noSceneNumber = QStringLiteral("-");
    const QString theEndSceneHeading = QStringLiteral("THE END");
    const QVersionNumber version = QVersionNumber::fromString(qApp->applicationVersion());

    QAbstractTextDocumentLayout *layout = m_document->documentLayout();

    QJsonArray offsets;
    auto registerOffsetForTextBlock = [&](const QTextBlock &block, const QString &text,
                                          SceneElement::Type type, const QString &sceneID,
                                          const QVariant &number) {
        OffsetItem item;
        item.setRow(offsets.size());
        item.setType(type);
        item.setId(sceneID);
        if (type == SceneElement::Heading) {
            item.setNumber(number.toString());
            item.setParagraphIndex(-1);
        } else
            item.setParagraphIndex(number.toInt());

        QString snippet = text;
        switch (type) {
        case SceneElement::Heading:
            break;
        case SceneElement::Character:
            snippet = QStringLiteral("Dialogue: ") + text;
            break;
        default:
            if (snippet.length() > 50)
                snippet = snippet.left(50) + QStringLiteral("...");
            break;
        }

        item.setSnippet(snippet);

        const qreal pixelOffset = offsets.isEmpty() ? 0 : layout->blockBoundingRect(block).y();
        item.setPixelOffset(pixelOffset);

        const int timeMs = qRound(msPerPixel * pixelOffset);
        item.setTimestamp(timeMs);
        item.setDefaultTimestamp(timeMs);

        const int pageNr = 1 + qFloor(pixelOffset / contentHeight);
        item.setPageNumber(pageNr);
        item.setVersion(version);
        item.setLocked(false);

        offsets.append(item.json());
    };

    const int nrElements = m_screenplay->elementCount();
    for (int i = 0; i < nrElements; i++) {
        const ScreenplayElement *element = m_screenplay->elementAt(i);
        if (element->scene() == nullptr)
            continue;

        const Scene *scene = element->scene();
        if (scene->heading()->isEnabled()) {
            if (cursor.position() > 0)
                cursor.insertBlock();
            prepareCursor(cursor, SceneElement::Heading, Qt::Alignment());
            polishFontsAndInsertTextAtCursor(cursor, scene->heading()->text());
        }

        registerOffsetForTextBlock(cursor.block(), scene->heading()->text(), SceneElement::Heading,
                                   scene->id(), element->resolvedSceneNumber());

        for (int p = 0; p < scene->elementCount(); p++) {
            if (cursor.position() > 0)
                cursor.insertBlock();

            const SceneElement *para = scene->elementAt(p);
            prepareCursor(cursor, para->type(), para->alignment());
            polishFontsAndInsertTextAtCursor(cursor, para->text());

            switch (para->type()) {
            case SceneElement::Action:
            case SceneElement::Character:
                registerOffsetForTextBlock(cursor.block(), para->formattedText(), para->type(),
                                           scene->id(), p);
                break;
            default:
                break;
            }
        }
    }

    if (cursor.position() > 0)
        cursor.insertBlock();
    polishFontsAndInsertTextAtCursor(cursor, theEndSceneHeading);
    registerOffsetForTextBlock(cursor.block(), theEndSceneHeading, SceneElement::Heading, QString(),
                               noSceneNumber);

    this->setArray(offsets);

    if (QFile::exists(m_fileName))
        this->loadOffsets();
    else
        this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::setErrorMessage(const QString &val)
{
    if (m_errorMessage == val)
        return;

    m_errorMessage = val;
    emit errorMessageChanged();
}

void ScreenplayTextDocumentOffsets::loadOffsets()
{
    this->clearErrorMessage();

    if (m_fileName.isNull() || this->count() == 0 || m_screenplay.isNull())
        return;

    QFile file(m_fileName);
    if (!file.open(QFile::ReadOnly))
        return;

    const QString errMsg =
            QStringLiteral("Data stored in offsets-file is out of sync with the current "
                           "screenplay. Recomputed time offsets will be used instead.");

    const QJsonArray modelArray = this->array();
    const QJsonArray fileArray = QJsonDocument::fromJson(file.readAll()).array();
    if (fileArray.isEmpty())
        return;

    const QVersionNumber minVersion(0, 5, 9);

    struct Segment
    {
        OffsetItem sceneOffset;
        QList<OffsetItem> paragraphOffsets;

        bool canMerge(const Segment &other) const
        {
            return this->sceneOffset.canMerge(other.sceneOffset);
        }

        void merge(const Segment &other)
        {
            this->sceneOffset.mergeFrom(other.sceneOffset);

            if (this->paragraphOffsets.size() == other.paragraphOffsets.size()) {
                bool success = true;
                for (int i = 0; i < this->paragraphOffsets.size(); i++) {
                    success &= this->paragraphOffsets[i].mergeFrom(other.paragraphOffsets.at(i));
                    if (!success)
                        break;
                }
                if (success)
                    return;
            }

            this->adjustParagraphs();
        }

    private:
        void adjustParagraphs()
        {
            const int dt = this->sceneOffset.timestamp() - this->sceneOffset.defaultTimestamp();
            for (OffsetItem &offset : this->paragraphOffsets)
                offset.setTimestamp(offset.defaultTimestamp() + dt);
        }
    };
    auto evaluateSegments = [=](const QJsonArray &array) {
        QList<Segment> ret;
        for (int i = 0; i < array.size(); i++) {
            OffsetItem item(array.at(i).toObject());
            if (!item.isValid())
                continue;

            if (item.type() == SceneElement::Heading) {
                Segment segment;
                segment.sceneOffset = item;
                ret.append(segment);
            } else {
                if (!ret.isEmpty())
                    ret.last().paragraphOffsets.append(item);
            }
        }
        return ret;
    };

    QList<Segment> fileSegments = evaluateSegments(fileArray);
    QList<Segment> modelSegments = evaluateSegments(modelArray);
    if (fileSegments.size() != modelSegments.size()) {
        this->setErrorMessage(errMsg);
        return;
    }

    for (int i = 0; i < fileSegments.size(); i++) {
        Segment fileSegment = fileSegments.at(i);
        Segment &modelSegment = modelSegments[i];
        if (!modelSegment.canMerge(fileSegment)) {
            this->setErrorMessage(errMsg);
            return;
        }

        modelSegment.merge(fileSegment);
    }

    QJsonArray offsets;
    for (const Segment &segment : qAsConst(modelSegments)) {
        offsets.append(segment.sceneOffset.json());
        for (const OffsetItem &offset : qAsConst(segment.paragraphOffsets))
            offsets.append(offset.json());
    }

    this->setArray(offsets);
}

void ScreenplayTextDocumentOffsets::saveOffsets()
{
    if (m_fileName.isEmpty() || m_screenplay.isNull() || this->count() == 0)
        return;

    // While we want to save offsets to file in a separate thread, we
    // dont want multiple threads writing to the file. So, we use a custom
    // thread-pool with exactly one thread in it.
    static QThreadPool saveOffsetsThreadPool;
    if (saveOffsetsThreadPool.maxThreadCount() != 1)
        saveOffsetsThreadPool.setMaxThreadCount(1);

    QtConcurrent::run(
            &saveOffsetsThreadPool,
            [](const QString &fileName, const QJsonArray &array) {
                QFile file(fileName);
                if (!file.open(QFile::WriteOnly))
                    return;
                file.write(QJsonDocument(array).toJson());
            },
            m_fileName, this->internalArray());
}
