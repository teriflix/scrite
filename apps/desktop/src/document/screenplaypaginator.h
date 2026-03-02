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

#ifndef SCREENPLAYPAGINATOR_H
#define SCREENPLAYPAGINATOR_H

#include <QQmlEngine>
#include <QTextDocument>

#include "screenplay.h"
#include "formatting.h"

class ScreenplayPaginatorBlockData : public QTextBlockUserData
{
public:
    int serialNumber = -1;
    QString sceneId;
    QString paragraphId;
    SceneElement::Type paragraphType;

    static ScreenplayPaginatorBlockData *get(const QTextBlock &block)
    {
        QTextBlockUserData *userData = block.userData();
        return reinterpret_cast<ScreenplayPaginatorBlockData *>(userData);
    }
};

struct ScenePageBreak
{
    Q_GADGET

public:
    // clang-format off
    Q_PROPERTY(int cursorPosition
               MEMBER cursorPosition)
    // clang-format on
    int cursorPosition = -1;

    // clang-format off
    Q_PROPERTY(int pageNumber
               MEMBER pageNumber)
    // clang-format on
    int pageNumber = -1;

    ScenePageBreak();
    ScenePageBreak(int cp, int pn) : cursorPosition(cp), pageNumber(pn) { }
    ScenePageBreak(const ScenePageBreak &other);
    bool operator==(const ScenePageBreak &other) const;
    bool operator!=(const ScenePageBreak &other) const;
    ScenePageBreak &operator=(const ScenePageBreak &other);
};
Q_DECLARE_METATYPE(ScenePageBreak)
Q_DECLARE_METATYPE(QList<ScenePageBreak>)

struct ScreenplayPaginatorRecord
{
    Q_GADGET

public:
    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const { return this->serialNumber > 0; }

    // clang-format off
    Q_PROPERTY(int serialNumber
               MEMBER serialNumber)
    // clang-format on
    int serialNumber = -1;

    // clang-format off
    Q_PROPERTY(int firstCursorPosition
               MEMBER firstCursorPosition)
    // clang-format on
    int firstCursorPosition = -1;

    // clang-format off
    Q_PROPERTY(int firstParagraphCursorPosition
               MEMBER firstParagraphCursorPosition)
    // clang-format on
    int firstParagraphCursorPosition = -1;

    // clang-format off
    Q_PROPERTY(int lastCursorPosition
               MEMBER firstCursorPosition)
    // clang-format on
    int lastCursorPosition = -1;

    // clang-format off
    Q_PROPERTY(qreal pixelLength
               MEMBER pixelLength)
    // clang-format on
    qreal pixelLength = 0;

    // clang-format off
    Q_PROPERTY(qreal pageLength
               MEMBER pageLength)
    // clang-format on
    qreal pageLength = 0;

    // clang-format off
    Q_PROPERTY(QTime timeLength
               MEMBER timeLength)
    // clang-format on
    QTime timeLength;

    // clang-format off
    Q_PROPERTY(qreal pixelOffset
               MEMBER pixelOffset)
    // clang-format on
    qreal pixelOffset = 0;

    // clang-format off
    Q_PROPERTY(qreal pageOffset
               MEMBER pageOffset)
    // clang-format on
    qreal pageOffset = 0;

    // clang-format off
    Q_PROPERTY(QTime timeOffset
               MEMBER timeOffset)
    // clang-format on
    QTime timeOffset;

    // clang-format off
    Q_PROPERTY(QList<ScenePageBreak> pageBreaks
               MEMBER pageBreaks)
    // clang-format on
    QList<ScenePageBreak> pageBreaks;

    // clang-format off
    Q_PROPERTY(ScreenplayElement *screenplayElement
               MEMBER screenplayElement)
    // clang-format on
    QPointer<ScreenplayElement> screenplayElement;

    ScreenplayPaginatorRecord();
    ScreenplayPaginatorRecord(const ScreenplayPaginatorRecord &other);
    bool operator==(const ScreenplayPaginatorRecord &other) const;
    bool operator!=(const ScreenplayPaginatorRecord &other) const;
    ScreenplayPaginatorRecord &operator=(const ScreenplayPaginatorRecord &other);
};
Q_DECLARE_METATYPE(ScreenplayPaginatorRecord)
Q_DECLARE_METATYPE(QList<ScreenplayPaginatorRecord>)

class QThread;
class ScreenplayPaginatorWorker;
class ScreenplayPaginatorWorkerNode;
class ScreenplayPaginator : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit ScreenplayPaginator(QObject *parent = nullptr);
    virtual ~ScreenplayPaginator();

    Q_INVOKABLE void useDefaultFormatAndScreenplay();

    // clang-format off
    Q_PROPERTY(Screenplay *screenplay
               READ screenplay
               WRITE setScreenplay
               NOTIFY screenplayChanged)
    // clang-format on
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    // clang-format off
    Q_PROPERTY(ScreenplayFormat *format
               READ format
               WRITE setFormat
               NOTIFY formatChanged)
    // clang-format on
    void setFormat(ScreenplayFormat *val);
    ScreenplayFormat *format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    // clang-format off
    Q_PROPERTY(bool enabled
               READ enabled
               WRITE setEnabled
               NOTIFY enabledChanged)
    // clang-format on
    void setEnabled(bool val);
    bool enabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    // clang-format off
    Q_PROPERTY(bool syncing
               READ isSyncing
               NOTIFY syncingChanged)
    // clang-format on
    bool isSyncing() const { return m_syncCounter > 0; }
    Q_SIGNAL void syncingChanged();

    // clang-format off
    Q_PROPERTY(bool busy
               READ isBusy
               NOTIFY busyChanged)
    // clang-format on
    bool isBusy() const;
    Q_SIGNAL void busyChanged();

    Q_INVOKABLE void reset();

    // These static functions assume that the document supplied as parameter
    // is a document constructed by this paginator only, using the paginatedDocument()
    // method. These methods bother to do any computation at all only if the
    // QTextDocument pointer passed to it belong to the same thread as the current one.
    static bool paginateIntoDocument(const Screenplay *screenplay, const ScreenplayFormat *format,
                                     QTextDocument *document);
    static QTextDocument *paginatedDocument(const Screenplay *screenplay,
                                            const ScreenplayFormat *format,
                                            QObject *documentParent = nullptr);
    static qreal pixelLength(const ScreenplayElement *element, const QTextDocument *document);
    static qreal pixelLength(const Scene *scene, const QTextDocument *document);
    static qreal pixelLength(const SceneHeading *sceneHeading, const QTextDocument *document);
    static qreal pixelLength(const SceneElement *paragraph, const QTextDocument *document);
    static qreal pixelLength(const QTextDocument *document);
    static qreal pixelLength(const QTextBlock &from, const QTextBlock &until,
                             const QTextDocument *document);
    static qreal pixelToPageLength(qreal pixelLength, const QTextDocument *document);
    static QTime pixelToTimeLength(qreal pixelLength, ScreenplayFormat *format,
                                   const QTextDocument *document);
    static QTime pageToTimeLength(qreal pixelLength, ScreenplayFormat *format,
                                  const QTextDocument *document);

    // clang-format off
    Q_PROPERTY(int recordCount
               READ recordCount
               NOTIFY paginationUpdated)
    // clang-format on
    int recordCount() const { return m_records.size(); }

    // clang-format off
    Q_PROPERTY(int pageCount
               READ pageCount
               NOTIFY paginationUpdated)
    // clang-format on
    int pageCount() const { return m_pageCount; }

    // clang-format off
    Q_PROPERTY(QTime totalTime
               READ totalTime
               NOTIFY paginationUpdated)
    // clang-format on
    QTime totalTime() const { return m_totalTime; }

    // clang-format off
    Q_PROPERTY(qreal totalPixelLength
               READ totalPixelLength
               NOTIFY paginationUpdated)
    // clang-format on
    qreal totalPixelLength() const { return m_totalPixelLength; }

    // clang-format off
    Q_PROPERTY(int cursorPosition
               READ cursorPosition
               WRITE setCursorPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    // clang-format off
    Q_PROPERTY(int cursorPage
               READ cursorPage
               NOTIFY cursorUpdated)
    // clang-format on
    int cursorPage() const { return m_cursorPage; }

    // clang-format off
    Q_PROPERTY(QTime cursorTime
               READ cursorTime
               NOTIFY cursorUpdated)
    // clang-format on
    QTime cursorTime() const { return m_cursorTime; }

    // clang-format off
    Q_PROPERTY(qreal cursorPixelOffset
               READ cursorPixelOffset
               NOTIFY cursorUpdated)
    // clang-format on
    qreal cursorPixelOffset() const { return m_cursorPixelOffset; }

    // clang-format off
    Q_PROPERTY(ScreenplayPaginatorRecord cursorRecord
               READ cursorRecord
               NOTIFY cursorUpdated)
    // clang-format on
    ScreenplayPaginatorRecord cursorRecord() const { return m_cursorRecord; }

    Q_INVOKABLE int indexOf(ScreenplayElement *element) const;
    Q_INVOKABLE ScreenplayPaginatorRecord recordAt(int row) const;

    Q_INVOKABLE qreal pixelLength(ScreenplayElement *from,
                                  ScreenplayElement *until = nullptr) const;
    Q_INVOKABLE qreal pageLength(ScreenplayElement *from, ScreenplayElement *until = nullptr) const;
    Q_INVOKABLE QTime timeLength(ScreenplayElement *from, ScreenplayElement *until = nullptr) const;

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

signals:
    void cursorUpdated();
    void paginationUpdated();
    void cursorQueryResponse(int cursorPosition, qreal pixelOffset, int pageNumber, qreal page,
                             const QTime &time, const ScreenplayPaginatorRecord &cursorRecord);

private:
    void clear();
    void clearRecords();
    void clearCursor();

    void incrementSyncCounter();
    void resetSyncCounter();

    void onFormatChanged();

    void onScreenplayReset();
    void onScreenplayDestroyed();
    void onScreenplayElementInserted(ScreenplayElement *element, int index);
    void onScreenplayElementRemoved(ScreenplayElement *element, int index);
    void onScreenplayElementOmitted(ScreenplayElement *element, int index);
    void onScreenplayElementIncluded(ScreenplayElement *element, int index);
    void onScreenplayElementSceneReset(ScreenplayElement *element, Scene *scene);
    void onScreenplayElementSceneHeadingChanged(ScreenplayElement *element,
                                                SceneHeading *sceneHeading);
    void onScreenplayElementSceneElementChanged(ScreenplayElement *element,
                                                SceneElement *sceneElement);

    void onCursorPositionChanged();

    void onCursorQueryResponse(int cursorPosition, qreal cursorPixel, int cursorPageNumber,
                               qreal cursorPage, const QTime &cursorTime,
                               const ScreenplayPaginatorRecord &cursorRecord);
    void onPaginationComplete(const QList<ScreenplayPaginatorRecord> &items, qreal pixelLength,
                              int pageCount, const QTime &totalTime);

    bool aggregate(ScreenplayElement *from, ScreenplayElement *until, qreal *pixelLength,
                   qreal *pageLength, QTime *timeLength) const;

private:
    bool m_componentComplete = true;
    bool m_enabled = true;
    int m_syncCounter = 0;

    int m_cursorPosition = -1;
    int m_cursorPage = 0;
    qreal m_cursorPixelOffset = 0;
    QTime m_cursorTime;
    ScreenplayPaginatorRecord m_cursorRecord;

    int m_pageCount = 0;
    qreal m_totalPixelLength = 0;
    QTime m_totalTime;

    QList<ScreenplayPaginatorRecord> m_records;

    Screenplay *m_screenplay = nullptr;
    ScreenplayFormat *m_format = nullptr;

    QThread *m_workerThread = nullptr;
    ScreenplayPaginatorWorker *m_worker = nullptr;
    ScreenplayPaginatorWorkerNode *m_workerNode = nullptr;
};

class ScreenplayPaginatorWatcher : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenplayPaginatorWatcher(QObject *parent = nullptr);
    virtual ~ScreenplayPaginatorWatcher();

    // clang-format off
    Q_PROPERTY(ScreenplayPaginator *paginator
               READ paginator
               WRITE setPaginator
               NOTIFY paginatorChanged)
    // clang-format on
    void setPaginator(ScreenplayPaginator *val);
    ScreenplayPaginator *paginator() const { return m_paginator; }
    Q_SIGNAL void paginatorChanged();

    // clang-format off
    Q_PROPERTY(ScreenplayElement *element
               READ element
               WRITE setElement
               NOTIFY elementChanged)
    // clang-format on
    void setElement(ScreenplayElement *val);
    ScreenplayElement *element() const { return m_element; }
    Q_SIGNAL void elementChanged();

    // clang-format off
    Q_PROPERTY(bool hasValidRecord
               READ hasValidRecord
               NOTIFY recordChanged)
    // clang-format on
    bool hasValidRecord() const { return m_record.isValid(); }

    // clang-format off
    Q_PROPERTY(ScreenplayPaginatorRecord record
               READ record
               NOTIFY recordChanged)
    // clang-format on
    ScreenplayPaginatorRecord record() const { return m_record; }
    Q_SIGNAL void recordChanged();

    // clang-format off
    Q_PROPERTY(qreal pixelLength
               READ pixelLength
               NOTIFY recordChanged)
    // clang-format on
    qreal pixelLength() const { return m_record.pixelLength; }

    // clang-format off
    Q_PROPERTY(qreal pageLength
               READ pageLength
               NOTIFY recordChanged)
    // clang-format on
    qreal pageLength() const { return m_record.pageLength; }

    // clang-format off
    Q_PROPERTY(QTime timeLength
               READ timeLength
               NOTIFY recordChanged)
    // clang-format on
    QTime timeLength() const { return m_record.timeLength; }

    // clang-format off
    Q_PROPERTY(QList<ScenePageBreak> pageBreaks
               READ pageBreaks
               NOTIFY recordChanged)
    // clang-format on
    QList<ScenePageBreak> pageBreaks() const { return m_record.pageBreaks; }

    // clang-format off
    Q_PROPERTY(qreal pixelOffset
               READ pixelOffset
               NOTIFY recordChanged)
    // clang-format on
    qreal pixelOffset() const { return m_record.pixelOffset; }

    // clang-format off
    Q_PROPERTY(qreal pageOffset
               READ pageOffset
               NOTIFY recordChanged)
    // clang-format on
    qreal pageOffset() const { return m_record.pageOffset; }

    // clang-format off
    Q_PROPERTY(QTime timeOffset
               READ timeOffset
               NOTIFY recordChanged)
    // clang-format on
    QTime timeOffset() const { return m_record.timeOffset; }

    // clang-format off
    Q_PROPERTY(bool hasCursor
               READ hasCursor
               NOTIFY cursorInfoChanged)
    // clang-format on
    bool hasCursor() const { return m_hasCursor; }

    // clang-format off
    Q_PROPERTY(int relativeCursorPosition
               READ relativeCursorPosition
               NOTIFY cursorInfoChanged)
    // clang-format on
    int relativeCursorPosition() const { return m_relativeCursorPosition; }

    // clang-format off
    Q_PROPERTY(qreal relativeCursorPixel
               READ relativeCursorPixel
               NOTIFY cursorInfoChanged)
    // clang-format on
    qreal relativeCursorPixel() const { return m_relativeCursorPixel; }

    // clang-format off
    Q_PROPERTY(qreal relativeCursorPage
               READ relativeCursorPage
               NOTIFY cursorInfoChanged)
    // clang-format on
    qreal relativeCursorPage() const { return m_relativeCursorPage; }

    // clang-format off
    Q_PROPERTY(QTime relativeCursorTime
               READ relativeCursorTime
               NOTIFY cursorInfoChanged)
    // clang-format on
    QTime relativeCursorTime() const { return m_relativeCursorTime; }

signals:
    Q_SIGNAL void cursorInfoChanged();

private:
    void lookupRecord();
    void onPaginationUpdated();
    void setRecord(const ScreenplayPaginatorRecord &val);
    void onCursorQueryResponse(int cursorPosition, qreal cursorPixel, int cursorPageNumber,
                               qreal cursorPage, const QTime &cursorTime,
                               const ScreenplayPaginatorRecord &cursorRecord);

private:
    ScreenplayElement *m_element = nullptr;
    ScreenplayPaginator *m_paginator = nullptr;
    ScreenplayPaginatorRecord m_record;

    bool m_hasCursor = false;
    int m_relativeCursorPosition = -1;
    qreal m_relativeCursorPixel = 0;
    qreal m_relativeCursorPage = 0;
    QTime m_relativeCursorTime;
};

#endif // SCREENPLAYPAGINATOR_H
