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
    Q_PROPERTY(int cursorPosition MEMBER cursorPosition)
    int cursorPosition = -1;

    Q_PROPERTY(int pageNumber MEMBER pageNumber)
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
    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const { return this->serialNumber > 0; }

    Q_PROPERTY(int serialNumber MEMBER serialNumber)
    int serialNumber = -1;

    Q_PROPERTY(qreal pixelLength MEMBER pixelLength)
    qreal pixelLength = 0;

    Q_PROPERTY(qreal pageLength MEMBER pageLength)
    qreal pageLength = 0;

    Q_PROPERTY(QTime timeLength MEMBER timeLength)
    QTime timeLength;

    Q_PROPERTY(QList<ScenePageBreak> pageBreaks MEMBER pageBreaks)
    QList<ScenePageBreak> pageBreaks;

    Q_PROPERTY(ScreenplayElement* screenplayElement MEMBER screenplayElement)
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
class ScreenplayPaginator : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit ScreenplayPaginator(QObject *parent = nullptr);
    virtual ~ScreenplayPaginator();

    Q_INVOKABLE void useDefaultFormatAndScreenplay();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged)
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(ScreenplayFormat* format READ format WRITE setFormat NOTIFY formatChanged)
    void setFormat(ScreenplayFormat *val);
    ScreenplayFormat *format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool enabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_INVOKABLE void reset();

    // These static functions assume that the document supplied as parameter
    // is a document constructed by this paginator only, using the paginatedDocument()
    // method. These methods bother to do any computation at all only if the
    // QTextDocument pointer passed to it belong to the same thread as the current one.
    static bool paginateIntoDocument(const Screenplay *screenplay, const ScreenplayFormat *format,
                                     QTextDocument *document);
    static QTextDocument *paginatedDocument(const Screenplay *screenplay, const ScreenplayFormat *format,
                                            QObject *documentParent = nullptr);
    static qreal pixelLength(const ScreenplayElement *element, const QTextDocument *document);
    static qreal pixelLength(const Scene *scene, const QTextDocument *document);
    static qreal pixelLength(const SceneHeading *sceneHeading, const QTextDocument *document);
    static qreal pixelLength(const SceneElement *paragraph, const QTextDocument *document);
    static qreal pixelLength(const QTextDocument *document);
    static qreal pixelLength(const QTextBlock &from, const QTextBlock &until, const QTextDocument *document);
    static qreal pixelToPageLength(qreal pixelLength, const QTextDocument *document);
    static QTime pixelToTimeLength(qreal pixelLength, ScreenplayFormat *format, const QTextDocument *document);
    static QTime pageToTimeLength(qreal pixelLength, ScreenplayFormat *format, const QTextDocument *document);

    Q_PROPERTY(int pageCount READ pageCount NOTIFY paginationUpdated)
    int pageCount() const { return m_pageCount; }

    Q_PROPERTY(QTime totalTime READ totalTime NOTIFY paginationUpdated)
    QTime totalTime() const { return m_totalTime; }

    Q_PROPERTY(qreal totalPixelLength READ totalPixelLength NOTIFY paginationUpdated)
    qreal totalPixelLength() const { return m_totalPixelLength; }

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_PROPERTY(int cursorPage READ cursorPage NOTIFY cursorUpdated)
    int cursorPage() const { return m_cursorPage; }

    Q_PROPERTY(QTime cursorTime READ cursorTime NOTIFY cursorUpdated)
    QTime cursorTime() const { return m_cursorTime; }

    Q_PROPERTY(qreal cursorPixelOffset READ cursorPixelOffset NOTIFY cursorUpdated)
    qreal cursorPixelOffset() const { return m_cursorPixelOffset; }

    Q_INVOKABLE int indexOf(ScreenplayElement *element) const;
    Q_INVOKABLE ScreenplayPaginatorRecord recordAt(int row) const;

    Q_INVOKABLE qreal pixelLength(ScreenplayElement *from, ScreenplayElement *until = nullptr) const;
    Q_INVOKABLE qreal pageLength(ScreenplayElement *from, ScreenplayElement *until = nullptr) const;
    Q_INVOKABLE QTime timeLength(ScreenplayElement *from, ScreenplayElement *until = nullptr) const;

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

signals:
    void cursorUpdated();
    void paginationUpdated();

private:
    void clear();
    void clearRecords();
    void clearCursor();

    void onFormatChanged();

    void onScreenplayChanged();
    void onScreenplayDestroyed();
    void onScreenplayElementInserted(ScreenplayElement *element, int index);
    void onScreenplayElementRemoved(ScreenplayElement *element, int index);
    void onScreenplayElementOmitted(ScreenplayElement *element, int index);
    void onScreenplayElementIncluded(ScreenplayElement *element, int index);
    void onScreenplayElementSceneReset(ScreenplayElement *element, Scene *scene);
    void onScreenplayElementSceneHeadingChanged(ScreenplayElement *element, SceneHeading *sceneHeading);
    void onScreenplayElementSceneElementChanged(ScreenplayElement *element, SceneElement *sceneElement);

    void onCursorPositionChanged();

    void onCursorQueryResponse(int cursorPosition, qreal pixelOffset, int pageNumber, const QTime &time);
    void onPaginationComplete(const QList<ScreenplayPaginatorRecord> &items, qreal pixelLength, int pageCount,
                              const QTime &totalTime);

    bool aggregate(ScreenplayElement *from, ScreenplayElement *until, qreal *pixelLength, qreal *pageLength,
                   QTime *timeLength) const;

private:
    bool m_componentComplete = true;
    bool m_enabled = true;

    int m_cursorPosition = -1;
    int m_cursorPage = 0;
    qreal m_cursorPixelOffset = 0;
    QTime m_cursorTime;

    int m_pageCount = 0;
    qreal m_totalPixelLength = 0;
    QTime m_totalTime;

    QList<ScreenplayPaginatorRecord> m_records;

    Screenplay *m_screenplay = nullptr;
    ScreenplayFormat *m_format = nullptr;

    QThread *m_workerThread = nullptr;
    ScreenplayPaginatorWorker *m_worker = nullptr;
};

class ScreenplayPaginatorWatcher : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenplayPaginatorWatcher(QObject *parent = nullptr);
    virtual ~ScreenplayPaginatorWatcher();

    Q_PROPERTY(ScreenplayPaginator* paginator READ paginator WRITE setPaginator NOTIFY paginatorChanged)
    void setPaginator(ScreenplayPaginator *val);
    ScreenplayPaginator *paginator() const { return m_paginator; }
    Q_SIGNAL void paginatorChanged();

    Q_PROPERTY(ScreenplayElement* element READ element WRITE setElement NOTIFY elementChanged)
    void setElement(ScreenplayElement *val);
    ScreenplayElement *element() const { return m_element; }
    Q_SIGNAL void elementChanged();

    Q_PROPERTY(bool hasValidRecord READ hasValidRecord NOTIFY recordChanged)
    bool hasValidRecord() const { return m_record.isValid(); }

    Q_PROPERTY(ScreenplayPaginatorRecord record READ record NOTIFY recordChanged)
    ScreenplayPaginatorRecord record() const { return m_record; }
    Q_SIGNAL void recordChanged();

    Q_PROPERTY(qreal pixelLength READ pixelLength NOTIFY recordChanged)
    qreal pixelLength() const { return m_record.pixelLength; }

    Q_PROPERTY(qreal pageLength READ pageLength NOTIFY recordChanged)
    qreal pageLength() const { return m_record.pageLength; }

    Q_PROPERTY(QTime timeLength READ timeLength NOTIFY recordChanged)
    QTime timeLength() const { return m_record.timeLength; }

    Q_PROPERTY(QList<ScenePageBreak> pageBreaks READ pageBreaks NOTIFY recordChanged)
    QList<ScenePageBreak> pageBreaks() const { return m_record.pageBreaks; }

private:
    void lookupRecord();
    void onPaginationUpdated();
    void setRecord(const ScreenplayPaginatorRecord &val);

private:
    ScreenplayElement *m_element = nullptr;
    ScreenplayPaginator *m_paginator = nullptr;
    ScreenplayPaginatorRecord m_record;
};

#endif // SCREENPLAYPAGINATOR_H
