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

    SceneParagraph() { }
    SceneParagraph(const QString &_sceneId, const QString &_id, bool _enabled, int _type,
                   const QString _text, Qt::Alignment _alignment,
                   const QVector<QTextLayout::FormatRange> &_formats);
    SceneParagraph(const SceneParagraph &other) { *this = other; }
    bool operator!=(const SceneParagraph &other) const { return !(*this == other); }
    bool operator==(const SceneParagraph &other) const;
    SceneParagraph &operator=(const SceneParagraph &other);

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

    SceneContent() { }
    SceneContent(int _type, int _breakType, int _serialNumber, bool _omitted, const QString &_id,
                 const QList<SceneParagraph> &_paragraph);
    SceneContent(const SceneContent &other) { *this = other; }
    bool operator!=(const SceneContent &other) const { return !(*this == other); }
    bool operator==(const SceneContent &other) const;
    SceneContent &operator=(const SceneContent &other);

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

    PaginatorDocumentInsights();
    PaginatorDocumentInsights(const PaginatorDocumentInsights &other);
    bool operator==(const PaginatorDocumentInsights &other);
    bool operator!=(const PaginatorDocumentInsights &other);
    PaginatorDocumentInsights &operator=(const PaginatorDocumentInsights &other);

    struct BlockRange
    {
        bool isValid() const;
        int serialNumber = -1;
        QString sceneId;
        QTextBlock from;
        QTextBlock until;

        BlockRange();
        BlockRange(const BlockRange &other);
        bool operator==(const BlockRange &other) const;
        bool operator!=(const BlockRange &other) const { return !(*this == other); }
        BlockRange &operator=(const BlockRange &other);
    };

    bool isEmpty() const;
    BlockRange findBlockRangeBySerialNumber(int serialNumber) const;
    BlockRange findBlockRangeBySceneId(const QString &sceneId) const;
    QTextBlock findBlock(const SceneHeading *heading) const;
    QTextBlock findBlock(const SceneElement *paragraph) const;
    QTextBlock findBlock(const QString &sceneId, const QString &paragraphId) const;

private:
    friend class ScreenplayPaginatorWorker;
    QMap<int, BlockRange> contentRangeMap;
};
Q_DECLARE_METATYPE(PaginatorDocumentInsights)

class ScreenplayPaginatorWorker : public QObject
{
    Q_OBJECT

public:
    virtual ~ScreenplayPaginatorWorker();

    // clang-format off
    Q_PROPERTY(bool synchronousSync
               READ isSynchronousSync
               WRITE setSynchronousSync
               NOTIFY synchronousSyncChanged)
    // clang-format on
    void setSynchronousSync(bool val);
    bool isSynchronousSync() const { return m_synchronousSync; }
    Q_SIGNAL void synchronousSyncChanged();

public slots:
    void useFormat(const QJsonObject &format);
    void reset(const QList<SceneContent> &screenplayContent);
    void insertElement(int index, const SceneContent &sceneContent);
    void removeElement(int index);
    void omitElement(int index);
    void includeElement(int index);
    void updateScene(const SceneContent &sceneContent);
    void updateParagraph(const SceneParagraph &paragraph);
    void queryCursor(int cursorPosition, int currentSerialNumber);

signals:
    void cursorQueryResponse(int cursorPosition, qreal pixelOffset, int pageNumber, qreal page,
                             const QTime &time, const ScreenplayPaginatorRecord &cursorRecord);
    void paginationComplete(const QList<ScreenplayPaginatorRecord> &items, qreal pixelLength,
                            int pageCount, const QTime &totalTime);
    void paginationStart();

private:
    explicit ScreenplayPaginatorWorker(QTextDocument *document = nullptr,
                                       ScreenplayFormat *format = nullptr,
                                       QObject *parent = nullptr);

    void syncDocument();
    void scheduleSyncDocument(const char *purpose = nullptr);

    qreal cursorPixelOffset(int cursorPosition, int currentSerialNumber) const;
    qreal cursorPixelOffset(const QTextCursor &cursor) const;
    ScreenplayPaginatorRecord cursorRecord(int currentSerialNumber) const;
    QList<ScenePageBreak>
    evaluateScenePageBreaks(const PaginatorDocumentInsights::BlockRange &blockRange,
                            int &lastPageNumber) const;

private:
    friend class ScreenplayPaginator;
    QJsonObject m_formatJson;
    QList<SceneContent> m_screenplayContent;
    QList<ScreenplayPaginatorRecord> m_records;
    QTextDocument *m_document = nullptr;
    QTimer *m_syncDocumentTimer = nullptr;
    ScreenplayFormat *m_defaultFormat = nullptr;
    ScreenplayFormat *m_format = nullptr;
    bool m_formatDirty = false;
    bool m_synchronousSync = false;
    int m_syncInterval = 500;
    qint64 m_lastSyncDocumentTimestamp = 0;
};

class ScreenplayPaginatorWorkerNode : public QObject
{
    Q_OBJECT

public:
    ScreenplayPaginatorWorkerNode(QObject *parent = nullptr);
    ~ScreenplayPaginatorWorkerNode();

    void setWorker(ScreenplayPaginatorWorker *worker);
    ScreenplayPaginatorWorker *worker() const { return m_worker; }

    // clang-format off
    Q_PROPERTY(bool busy
               READ isBusy
               NOTIFY busyChanged)
    // clang-format on
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

signals:
    void useFormat(const QJsonObject &format);
    void reset(const QList<SceneContent> &screenplayContent);
    void insertElement(int index, const SceneContent &sceneContent);
    void removeElement(int index);
    void omitElement(int index);
    void includeElement(int index);
    void updateScene(const SceneContent &sceneContent);
    void updateParagraph(const SceneParagraph &paragraph);
    void queryCursor(int cursorPosition, int currentSerialNumber);

    void cursorQueryResponse(int cursorPosition, qreal pixelOffset, int pageNumber, qreal page,
                             const QTime &time, const ScreenplayPaginatorRecord &cursorRecord);
    void paginationComplete(const QList<ScreenplayPaginatorRecord> &items, qreal pixelLength,
                            int pageCount, const QTime &totalTime);

private:
    void markBusy() { this->setBusy(true); }
    void markNotBusy() { this->setBusy(false); }
    void setBusy(bool val);

private:
    bool m_busy = false;
    QPointer<ScreenplayPaginatorWorker> m_worker;
};

#endif // SCREENPLAYPAGINATORWORKER_H
