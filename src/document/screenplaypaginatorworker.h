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

#ifndef SCREENPLAYPAGINATORWORKER_H
#define SCREENPLAYPAGINATORWORKER_H

#include <QJsonObject>
#include <QObject>
#include <QTextLayout>

#include "screenplaypaginator.h"

class Screenplay;
class SceneElement;
class SceneHeading;
class QTextDocument;
class ScreenplayFormat;
class ScreenplayElement;
class ScreenplayPaginator;

struct SceneParagraph
{
    QString sceneId;
    QString id;
    bool enabled = true;
    int type = -1; // Must be one of SceneElement::Type
    QString text;
    Qt::Alignment alignment;
    QVector<QTextLayout::FormatRange> formats;

    bool isValid() const;

    static SceneParagraph fromSceneHeading(const SceneHeading *heading);
    static SceneParagraph fromSceneElement(const SceneElement *element);
};
Q_DECLARE_METATYPE(SceneParagraph)
Q_DECLARE_METATYPE(QList<SceneParagraph>)

struct SceneContent
{
    int type = -1; // Must be one of ScreenplayElement::ElementType
    int breakType = -1; // Must be one of Screenplay::BreakType
    int serialNumber = -1;

    // In case type is ScreenplayElement::SceneElementType
    bool omitted = false;
    QString id;
    QList<SceneParagraph> paragraphs;

    bool isValid() const;

    static SceneContent fromScreenplayElement(const ScreenplayElement *element);
    static QList<SceneContent> fromScreenplay(const Screenplay *screenplay);
};
Q_DECLARE_METATYPE(SceneContent)
Q_DECLARE_METATYPE(QList<SceneContent>)

class ScreenplayPaginatorWorker;
class PaginatorDocumentInsights
{
public:
    static const char *property;

    PaginatorDocumentInsights() { }
    PaginatorDocumentInsights(const PaginatorDocumentInsights &other) { this->contentRangeMap = other.contentRangeMap; }
    bool operator==(const PaginatorDocumentInsights &other) { return this->contentRangeMap == other.contentRangeMap; }
    bool operator!=(const PaginatorDocumentInsights &other) { return this->contentRangeMap != other.contentRangeMap; }
    PaginatorDocumentInsights &operator=(const PaginatorDocumentInsights &other)
    {
        this->contentRangeMap = other.contentRangeMap;
        return *this;
    }

    struct BlockRange
    {
        bool isValid() const { return serialNumber >= 0 && from.isValid() && until.isValid(); }
        int serialNumber = -1;
        QString sceneId;
        QTextBlock from;
        QTextBlock until;

        BlockRange() { }
        BlockRange(const BlockRange &other) { *this = other; }
        bool operator==(const BlockRange &other) const
        {
            return this->serialNumber == other.serialNumber && this->sceneId == other.sceneId
                    && this->from == other.from && this->until == other.until;
        }
        bool operator!=(const BlockRange &other) const { return !(*this == other); }
        BlockRange &operator=(const BlockRange &other)
        {
            this->serialNumber = other.serialNumber;
            this->sceneId = other.sceneId;
            this->from = other.from;
            this->until = other.until;
            return *this;
        }
    };

    bool isEmpty() const { return contentRangeMap.isEmpty(); }

    BlockRange findBlockRangeBySerialNumber(int serialNumber) const
    {
        return this->contentRangeMap.value(serialNumber);
    }

    BlockRange findBlockRangeBySceneId(const QString &sceneId) const
    {
        const QList<BlockRange> blockRanges = this->contentRangeMap.values();
        auto it = std::find_if(blockRanges.begin(), blockRanges.end(),
                               [sceneId](const BlockRange &item) { return (item.sceneId == sceneId); });
        if (it != blockRanges.end())
            return *it;
        return BlockRange();
    }

private:
    friend class ScreenplayPaginatorWorker;
    QMap<int, BlockRange> contentRangeMap;
};
Q_DECLARE_METATYPE(PaginatorDocumentInsights)

class ScreenplayPaginatorWorker : public QObject
{
    Q_OBJECT

public:
    static const int syncInterval;
    virtual ~ScreenplayPaginatorWorker();

public slots:
    void useFormat(const QJsonObject &format);
    void reset(const QList<SceneContent> &screenplayContent);
    void insertElement(int index, const SceneContent &sceneContent);
    void removeElement(int index);
    void omitElement(int index);
    void includeElement(int index);
    void updateScene(const SceneContent &sceneContent);
    void updateParagraph(const SceneParagraph &paragraph);
    void query(int cursorPosition, int currentSerialNumber);

signals:
    void cursorQueryResponse(int cursorPosition, qreal pixelOffset, int pageNumber, const QTime &time);
    void paginationComplete(const QList<ScreenplayPaginatorRecord> &items, qreal pixelLength, int pageCount,
                            const QTime &totalTime);

private:
    explicit ScreenplayPaginatorWorker(QTextDocument *document = nullptr, QObject *parent = nullptr);

    void syncDocument();
    void scheduleSyncDocument(const char *purpose = nullptr);

    qreal cursorPixelOffset(int cursorPosition, int currentSerialNumber) const;
    qreal cursorPixelOffset(const QTextCursor &cursor) const;
    QList<ScenePageBreak> evaluateScenePageBreaks(const PaginatorDocumentInsights::BlockRange &blockRange,
                                                  int &lastPageNumber) const;

private:
    friend class ScreenplayPaginator;
    QList<SceneContent> m_screenplayContent;
    QTextDocument *m_document = nullptr;
    QTimer *m_syncDocumentTimer = nullptr;
    qint64 m_lastSyncDocumentTimestamp = 0;
    ScreenplayFormat *m_format = nullptr;
    QJsonObject m_formatJson;
    qreal m_lineHeight = 0;
};

#endif // SCREENPLAYPAGINATORWORKER_H
