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

#include "screenplaytextdocumentoffsets.h"
#include "timeprofiler.h"

#include <QDir>
#include <QtMath>
#include <QFileInfo>
#include <QJsonDocument>
#include <QAbstractTextDocumentLayout>
#include <QVersionNumber>

inline QString timeToString(const QTime &t)
{
    if(t == QTime(0,0,0))
        return QStringLiteral("0:00 min");

    if(t.hour() > 0)
        return t.toString(QStringLiteral("H:mm:ss")) + QStringLiteral(" hrs");

    return t.toString(QStringLiteral("m:ss")) + QStringLiteral(" min");
}

ScreenplayTextDocumentOffsets::ScreenplayTextDocumentOffsets(QObject *parent)
    : QAbstractListModel(parent),
      m_screenplay(this, "screenplay"),
      m_document(this, "document"),
      m_format(this, "format")
{
    m_reloadTimer = new QTimer(this);
    m_reloadTimer->setInterval(0);
    m_reloadTimer->setSingleShot(true);
    connect(m_reloadTimer, &QTimer::timeout, this, &ScreenplayTextDocumentOffsets::reloadDocument);
    connect(this, &QAbstractListModel::modelReset, this, &ScreenplayTextDocumentOffsets::offsetCountChanged);

    m_document = new QTextDocument(this);
}

ScreenplayTextDocumentOffsets::~ScreenplayTextDocumentOffsets()
{

}

void ScreenplayTextDocumentOffsets::setScreenplay(Screenplay *val)
{
    if(m_screenplay == val)
        return;

    if(!m_screenplay.isNull())
        m_screenplay->disconnect(this);

    m_screenplay = val;
    emit screenplayChanged();

    if(!m_screenplay.isNull())
        connect(m_screenplay, SIGNAL(screenplayChanged()), m_reloadTimer, SLOT(start()));

    m_reloadTimer->start();
}

void ScreenplayTextDocumentOffsets::setDocument(QTextDocument *val)
{
    if(m_document == val)
        return;

    if(!m_document.isNull() && m_document->parent() == this)
        m_document->deleteLater();

    if(val == nullptr)
        val = new QTextDocument(this);

    m_document = val;
    emit documentChanged();

    m_reloadTimer->start();
}

void ScreenplayTextDocumentOffsets::setFormat(ScreenplayFormat *val)
{
    if(m_format == val)
        return;

    if(!m_format.isNull())
        m_format->disconnect(this);

    m_format = val;
    emit formatChanged();

    if(!m_format.isNull())
        connect(m_format, SIGNAL(formatChanged()), m_reloadTimer, SLOT(start()));

    m_reloadTimer->start();
}

void ScreenplayTextDocumentOffsets::setFileName(const QString &val)
{
    if(m_fileName == val)
        return;

    m_fileName = val;
    emit fileNameChanged();

    if( QFile::exists(val) )
        this->loadOffsets();
    else
        this->saveOffsets();
}

QString ScreenplayTextDocumentOffsets::fileNameFrom(const QString &mediaFileNameOrUrl) const
{
    QString mediaFileName;
    if(mediaFileNameOrUrl.startsWith(QStringLiteral("file://")))
        mediaFileName = QUrl(mediaFileNameOrUrl).toLocalFile();
    else
        mediaFileName = mediaFileNameOrUrl;

    QFileInfo fi(mediaFileName);
    return fi.absoluteDir().absoluteFilePath( fi.baseName() + QStringLiteral(" Scrited View Offsets.json") );
}

QJsonObject ScreenplayTextDocumentOffsets::offsetInfoAt(int row) const
{
    if(row < 0 || row >= m_offsets.size())
        return _OffsetInfo().toJson();

    return m_offsets[row].toJson();
}

QJsonObject ScreenplayTextDocumentOffsets::offsetInfoAtPoint(const QPointF &pos) const
{
    if(m_offsets.isEmpty() || pos.x() < 0 || pos.x() >= m_document->textWidth())
        return _OffsetInfo().toJson();

    if(m_offsets.size() > 1)
    {
        for(int i=0; i<m_offsets.size(); i++)
        {
            if(m_offsets[i].pixelOffset >= pos.y())
            {
                if( qFuzzyCompare(m_offsets[i].pixelOffset, pos.y()) )
                    return m_offsets[i].toJson();

                return m_offsets[qMax(i-1,0)].toJson();
            }
        }
    }

    return m_offsets.last().toJson();
}

QJsonObject ScreenplayTextDocumentOffsets::offsetInfoAtTime(int timeInMs, int rowHint) const
{
    if(m_offsets.isEmpty() || timeInMs < 0)
        return _OffsetInfo().toJson();

    if(m_offsets.size() > 1)
    {
        const int startRow = qBound(0, rowHint, m_offsets.size()-1);
        for(int i=startRow; i<m_offsets.size(); i++)
        {
            if(m_offsets[i].sceneTime.msecsSinceStartOfDay() >= timeInMs)
            {
                if(m_offsets[i].sceneTime.msecsSinceStartOfDay() == timeInMs)
                    return m_offsets[i].toJson();

                return m_offsets[qMax(i-1,0)].toJson();
            }
        }
    }

    return m_offsets.last().toJson();
}

const qreal lastScenePixelLength = 20.0;
const int lastSceneTimeLength = 500;

int ScreenplayTextDocumentOffsets::evaluateTimeAtPoint(const QPointF &pos, int rowHint) const
{
    if(m_offsets.isEmpty() || pos.y() < 0)
        return 0;

    if(qFuzzyIsNull(pos.y()))
        return 0;

    if(pos.y() >= m_offsets.last().pixelOffset+lastScenePixelLength)
        return m_offsets.last().sceneTime.msecsSinceStartOfDay() + lastSceneTimeLength;

    if(rowHint < 0)
        rowHint = this->offsetInfoAtPoint(pos).value("row").toInt();

    auto computeTime = [](const qreal p1, const qreal p, const qreal p2, const QTime &t1, const QTime &t2) {
        return t1.msecsSinceStartOfDay() + qAbs(((p-p1)/(p2-p1)) * qreal(t2.msecsSinceStartOfDay() - t1.msecsSinceStartOfDay()));
    };

    if(rowHint >= 0 && rowHint < m_offsets.size())
    {
        const qreal cpo = m_offsets[rowHint].pixelOffset;
        const qreal npo = rowHint < m_offsets.size()-1 ? m_offsets[rowHint+1].pixelOffset : m_offsets.last().pixelOffset+lastScenePixelLength;
        const QTime t1 = m_offsets[rowHint].sceneTime;
        const QTime t2 = rowHint < m_offsets.size()-1 ? m_offsets[rowHint+1].sceneTime : m_offsets.last().sceneTime.addMSecs(lastSceneTimeLength);
        if(cpo <= pos.y() && pos.y() <= npo)
            return computeTime(cpo, pos.y(), npo, t1, t2);
    }

    return 0;
}

QPointF ScreenplayTextDocumentOffsets::evaluatePointAtTime(int timeInMs, int rowHint) const
{
    if(m_offsets.isEmpty() || timeInMs <= 0)
        return QPointF(10,0);

    if(timeInMs >= m_offsets.last().sceneTime.msecsSinceStartOfDay()+lastSceneTimeLength)
        return QPointF(10, m_offsets.last().pixelOffset+lastScenePixelLength);

    if(rowHint < 0)
        rowHint = this->offsetInfoAtTime(timeInMs).value("row").toInt();

    auto computePoint = [](int t1, int t, int t2, qreal p1, qreal p2) {
        return QPointF(10, p1 + ((qreal(t-t1)/qreal(t2-t1)) * (p2-p1)) );
    };

    if(rowHint >= 0 && rowHint < m_offsets.size())
    {
        const int ct = m_offsets[rowHint].sceneTime.msecsSinceStartOfDay();
        const int nt = rowHint < m_offsets.size()-1 ? m_offsets[rowHint+1].sceneTime.msecsSinceStartOfDay() : m_offsets.last().sceneTime.msecsSinceStartOfDay()+lastSceneTimeLength;
        const qreal p1 = m_offsets[rowHint].pixelOffset;
        const qreal p2 = rowHint < m_offsets.size()-1 ? m_offsets[rowHint+1].pixelOffset : m_offsets.last().pixelOffset+lastScenePixelLength;
        if(ct <= timeInMs && timeInMs <= nt)
            return computePoint(ct, timeInMs, nt, p1, p2);
    }

    return QPointF(10,0);
}

void ScreenplayTextDocumentOffsets::setTime(int row, int timeInMs, bool adjustFollowingRows)
{
    if(row < 0 || row >= m_offsets.size())
        return;

    int startRow = row;

    int timeDiffInMs = 0;
    for(int i=row; i<m_offsets.size(); i++)
    {
        _OffsetInfo &offset = m_offsets[i];
        if(i == row)
            timeDiffInMs = timeInMs - offset.sceneTime.msecsSinceStartOfDay();

        offset.sceneTime = offset.sceneTime.addMSecs(timeDiffInMs);

        if(!adjustFollowingRows)
            break;
    }

    const QModelIndex start = this->index(startRow);
    const QModelIndex end = adjustFollowingRows ? this->index(m_offsets.size()-1) : start;
    emit dataChanged(start, end);

    this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::resetTime(int row, bool andFollowingRows)
{
    if(row < 0 || row >= m_offsets.size() || m_format.isNull())
        return;

    const qreal contentHeight = m_format->pageLayout()->contentRect().height();
    const qreal msPerPixel = (m_format->secondsPerPage() * 1000)/contentHeight;

    for(int i=row; i<m_offsets.size(); i++)
    {
        _OffsetInfo &offset = m_offsets[i];
        const int timeMs = qAbs(msPerPixel * offset.pixelOffset);
        offset.sceneTime = QTime(0,0,0,1).addMSecs(timeMs-1);
        if(!andFollowingRows)
            break;
    }

    const QModelIndex start = this->index(row);
    const QModelIndex end = andFollowingRows ? this->index(m_offsets.size()-1) : start;
    emit dataChanged(start, end);

    this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::resetAllTimes()
{
    this->resetTime(0, true);
}

QHash<int, QByteArray> ScreenplayTextDocumentOffsets::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if(roles.isEmpty())
    {
        roles[ScreenplayElementIndexRole] = QByteArrayLiteral("screenplayElementIndex");
        roles[SceneIndexRole] = QByteArrayLiteral("sceneIndex");
        roles[SceneNumberRole] = QByteArrayLiteral("sceneNumber");
        roles[SceneHeadingRole] = QByteArrayLiteral("sceneHeading");
        roles[PageNumberRole] = QByteArrayLiteral("pageNumber");
        roles[TimeOffsetRole] = QByteArrayLiteral("timeOffset");
        roles[PixelOffsetRole] = QByteArrayLiteral("pixelOffset");
        roles[OffsetInfoRole] = QByteArrayLiteral("offsetInfo");
    }

    return roles;
}

QVariant ScreenplayTextDocumentOffsets::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= m_offsets.size())
        return QVariant();

    const _OffsetInfo &offset = m_offsets[index.row()];

    switch(role)
    {
    case ScreenplayElementIndexRole:
        return offset.elementIndex;
    case SceneIndexRole:
        return offset.sceneIndex;
    case SceneNumberRole:
        return offset.sceneNumber;
    case SceneHeadingRole:
        return offset.sceneHeading;
    case PageNumberRole:
        return offset.pageNumber;
    case TimeOffsetRole:
        return offset.sceneTime;
    case PixelOffsetRole:
        return offset.pixelOffset;
    case OffsetInfoRole:
        return offset.toJson();
    }

    return QVariant();
}

int ScreenplayTextDocumentOffsets::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_offsets.size();
}

Qt::ItemFlags ScreenplayTextDocumentOffsets::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable;
}

bool ScreenplayTextDocumentOffsets::setData(const QModelIndex &index, const QVariant &data, int role)
{
    if(role != TimeOffsetRole)
        return false;

    if(index.row() < 0 || index.row() >= m_offsets.size())
        return false;

    _OffsetInfo &offset = m_offsets[index.row()];

    if(data.userType() == QMetaType::QTime)
        offset.sceneTime = data.toTime();
    else if(data.userType() == QMetaType::Int)
        offset.sceneTime = QTime(0,0,0,1).addMSecs(data.toInt()-1);
    else
        return false;

    emit dataChanged(index, index);
    return true;
}

void ScreenplayTextDocumentOffsets::reloadDocument()
{
    if(m_format.isNull() || m_screenplay.isNull() || m_document.isNull() || m_screenplay->elementCount() == 0)
    {
        this->beginResetModel();
        m_offsets.clear();
        if(!m_document.isNull())
            m_document->clear();
        this->endResetModel();
        return;
    }

    const qreal textWidth = m_format->pageLayout()->contentWidth();
    const qreal contentHeight = m_format->pageLayout()->contentRect().height();
    const qreal msPerPixel = (m_format->secondsPerPage() * 1000)/contentHeight;

    m_document->clear();
    m_document->setTextWidth(textWidth);

    QTextCursor cursor(m_document);
    auto prepareCursor = [=](QTextCursor &cursor, SceneElement::Type paraType) {
        const SceneElementFormat *format = m_format->elementFormat(paraType);
        QTextBlockFormat blockFormat = format->createBlockFormat(&textWidth);
        QTextCharFormat charFormat = format->createCharFormat(&textWidth);
        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
    };

    const QString noSceneNumber = QStringLiteral("-");
    const QString theEndSceneHeading = QStringLiteral("THE END");

    QAbstractTextDocumentLayout *layout = m_document->documentLayout();

    QList<_OffsetInfo> offsets;
    const int nrElements = m_screenplay->elementCount();
    int si = 0;
    for(int i=0; i<nrElements; i++)
    {
        QTextBlock sceneBlock;

        const ScreenplayElement *element = m_screenplay->elementAt(i);
        if(element->scene() == nullptr)
            continue;

        const Scene *scene = element->scene();
        if(scene->heading()->isEnabled())
        {
            if(cursor.position() > 0)
                cursor.insertBlock();

            prepareCursor(cursor, SceneElement::Heading);
            cursor.insertText(QStringLiteral("[") + element->resolvedSceneNumber() + QStringLiteral("] "));
            cursor.insertText(scene->heading()->text());
            sceneBlock = cursor.block();
        }

        for(int p=0; p<scene->elementCount(); p++)
        {
            if(cursor.position() > 0)
                cursor.insertBlock();

            const SceneElement *para = scene->elementAt(p);
            prepareCursor(cursor, para->type());
            cursor.insertText(para->text());

            if(!sceneBlock.isValid())
                sceneBlock = cursor.block();
        }

        _OffsetInfo offset;
        offset.row = offsets.size();
        offset.elementIndex = i;
        offset.sceneIndex = si++;
        offset.pixelOffset = offsets.isEmpty() ? 0 : layout->blockBoundingRect(sceneBlock).y();
        offset.pageNumber = 1+qFloor(offset.pixelOffset / contentHeight);
        offset.sceneHeading = scene->heading()->text();
        offset.sceneNumber = scene->heading()->isEnabled() ? element->resolvedSceneNumber() : noSceneNumber;
        const int timeMs = qAbs(msPerPixel * offset.pixelOffset);
        offset.sceneTime = QTime(0,0,0,1).addMSecs(timeMs-1);
        offsets.append(offset);
    }

    // This is required so that we can estimate time required for
    // the last scene in the screenplay.
    {
        if(cursor.position() > 0)
            cursor.insertBlock();
        cursor.insertText(theEndSceneHeading);

        _OffsetInfo offset;
        offset.row = offsets.size();
        offset.elementIndex = -1;
        offset.sceneIndex = si++;
        offset.pixelOffset = layout->blockBoundingRect(cursor.block()).y();
        offset.pageNumber = 1+qFloor(offset.pixelOffset / contentHeight);
        offset.sceneHeading = theEndSceneHeading;
        offset.sceneNumber = noSceneNumber;
        const int timeMs = qAbs(msPerPixel * offset.pixelOffset);
        offset.sceneTime = QTime(0,0,0,1).addMSecs(timeMs-1);
        offsets.append(offset);
    }

    this->beginResetModel();
    m_offsets = offsets;
    this->endResetModel();

    if( QFile::exists(m_fileName) )
        this->loadOffsets();
    else
        this->saveOffsets();
}

void ScreenplayTextDocumentOffsets::setErrorMessage(const QString &val)
{
    if(m_errorMessage == val)
        return;

    m_errorMessage = val;
    emit errorMessageChanged();
}

void ScreenplayTextDocumentOffsets::loadOffsets()
{
    this->clearErrorMessage();

    if(m_fileName.isNull() || m_offsets.isEmpty() || m_screenplay.isNull())
        return;

    QFile file(m_fileName);
    if(!file.open(QFile::ReadOnly))
        return;

    const QString errMsg = QStringLiteral("Data stored in offsets-file is out of sync with the current screenplay. Recomputed time offsets will be used instead.");

    const QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
    if(array.isEmpty())
        return;

    if(array.size() != m_offsets.size())
    {
        this->setErrorMessage(errMsg);
        return;
    }

    const QVersionNumber minVersion(0,5,5);

    QList<_OffsetInfo> offsets = m_offsets;
    for(int i=0; i<array.size(); i++)
    {
        const QJsonObject item = array.at(i).toObject();
        const QString sceneId = item.value("sceneId").toString();
        const int elementIndex = item.value("elementIndex").toInt();
        const QString itemVersionString = item.value("version").toString();
        const QVersionNumber itemVersion = i == 0 ? QVersionNumber::fromString(itemVersionString) : minVersion;
        if(itemVersionString.isEmpty() || itemVersion < minVersion)
        {
            this->setErrorMessage(errMsg);
            return;
        }

        _OffsetInfo &offset = offsets[i];
        if(offset.elementIndex != elementIndex)
        {
            this->setErrorMessage(errMsg);
            return;
        }

        const ScreenplayElement *element = m_screenplay->elementAt(offset.elementIndex);
        if(element && element->scene() && element->scene()->id() != sceneId)
        {
            this->setErrorMessage(errMsg);
            return;
        }

        offset.sceneTime = QTime(0,0,0,1).addMSecs(item.value("timestamp").toInt()-1);
    }

    m_offsets = offsets;

    const QModelIndex start = this->index(0);
    const QModelIndex end = this->index(m_offsets.size()-1);
    emit dataChanged(start, end);
}

void ScreenplayTextDocumentOffsets::saveOffsets()
{
    if(m_fileName.isEmpty() || m_screenplay.isNull() || m_offsets.isEmpty())
        return;

    const QVersionNumber version = QVersionNumber::fromString(qApp->applicationVersion());
    const QString versionString = version.toString();

    QJsonArray array;
    for(const _OffsetInfo &offset : m_offsets)
    {
        const ScreenplayElement *element = m_screenplay->elementAt(offset.sceneIndex);
        QJsonObject item = offset.toJson();
        if(element && element->scene())
            item.insert("sceneId", element->scene()->id());
        item.insert("version", versionString);
        item.remove("sceneTime");
        item.insert("timestamp", offset.sceneTime.msecsSinceStartOfDay());
        array.append(item);
    }

    QFile file(m_fileName);
    if( !file.open(QFile::WriteOnly) )
        return;

    file.write( QJsonDocument(array).toJson() );
}

QJsonObject ScreenplayTextDocumentOffsets::_OffsetInfo::toJson() const
{
    QJsonObject ret;

    ret.insert("row", this->row);
    ret.insert( QStringLiteral("elementIndex"), this->elementIndex );
    ret.insert( QStringLiteral("sceneIndex"), this->sceneIndex );
    ret.insert( QStringLiteral("pixelOffset"), this->pixelOffset );
    ret.insert( QStringLiteral("pageNumber"), this->pageNumber );
    ret.insert( QStringLiteral("sceneHeading"), this->sceneHeading );
    ret.insert( QStringLiteral("sceneNumber"), this->sceneNumber );

    QJsonObject timeJs;
    timeJs.insert("hour", this->sceneTime.hour());
    timeJs.insert("minute", this->sceneTime.minute());
    timeJs.insert("second", this->sceneTime.second());
    timeJs.insert("timestamp", this->sceneTime.msecsSinceStartOfDay());
    timeJs.insert("text", timeToString(this->sceneTime));

    ret.insert( QStringLiteral("sceneTime"), timeJs );

    return ret;
}
