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

//#define DISPLAY_DOCUMENT_IN_TEXTEDIT

#include "screenplaytextdocument.h"

#include <QAbstractTextDocumentLayout>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QJsonDocument>
#include <QPaintEngine>
#include <QPainter>
#include <QPdfWriter>
#include <QPropertyAnimation>
#include <QQmlEngine>
#include <QScopedValueRollback>
#include <QSettings>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextBlockUserData>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextTable>
#include <QUrl>
#include <QtDebug>
#include <QtMath>

#include "application.h"
#include "garbagecollector.h"
#include "hourglass.h"
#include "pdfexportablegraphicsscene.h"
#include "printerobject.h"
#include "scritedocument.h"
#include "timeprofiler.h"

inline QTime secondsToTime(int seconds)
{
    return Application::secondsToTime(seconds);
}

inline QString timeToString(const QTime &t)
{
    if (t == QTime(0, 0, 0))
        return QStringLiteral("00:00");

    if (t.hour() > 0)
        return t.toString(QStringLiteral("H:mm:ss"));

    return t.toString(QStringLiteral("m:ss"));
}

class ScreenplayParagraphBlockData : public QTextBlockUserData
{
public:
    enum { Type = 1002 };
    const int type = Type;

    explicit ScreenplayParagraphBlockData(const SceneElement *element);
    ~ScreenplayParagraphBlockData();

    bool contains(const SceneElement *other) const;
    SceneElement::Type elementType() const;
    QString elementText() const;
    const SceneElement *element() const { return m_element; }
    bool isFirstElementInScene() const;
    bool doesDialogueFinish() const;

    const SceneElement *getCharacterElement() const;
    QString getCharacterElementText() const;

    bool isModified() const
    {
        if (m_element)
            return m_element->isModified(&m_elementModificationTime);
        return true;
    }
    void touch()
    {
        if (m_element)
            m_element->isModified(&m_elementModificationTime);
    }

    static ScreenplayParagraphBlockData *get(const QTextBlock &block);
    static ScreenplayParagraphBlockData *get(QTextBlockUserData *userData);

private:
    const SceneElement *m_element = nullptr;
    mutable int m_elementModificationTime = 0;
};

ScreenplayParagraphBlockData::ScreenplayParagraphBlockData(const SceneElement *element)
    : m_element(element)
{
}

ScreenplayParagraphBlockData::~ScreenplayParagraphBlockData() { }

bool ScreenplayParagraphBlockData::contains(const SceneElement *other) const
{
    return m_element != nullptr && m_element == other;
}

SceneElement::Type ScreenplayParagraphBlockData::elementType() const
{
    return m_element ? m_element->type() : SceneElement::Heading;
}

QString ScreenplayParagraphBlockData::elementText() const
{
    return m_element ? m_element->text() : QString();
}

bool ScreenplayParagraphBlockData::isFirstElementInScene() const
{
    if (m_element)
        return m_element->scene()->elementAt(0) == m_element;
    return false;
}

bool ScreenplayParagraphBlockData::doesDialogueFinish() const
{
    SceneElement *element = const_cast<SceneElement *>(m_element);
    auto isDialogueOrParenthetical = [](SceneElement *e) {
        return e
                && (e->type() == SceneElement::Dialogue
                    || e->type() == SceneElement::Parenthetical);
    };

    if (isDialogueOrParenthetical(element)) {
        const int index = element->scene()->indexOfElement(element);
        element = element->scene()->elementAt(index + 1);
        if (isDialogueOrParenthetical(element))
            return false;
    }

    return true;
}

const SceneElement *ScreenplayParagraphBlockData::getCharacterElement() const
{
    if (m_element
        && (m_element->type() == SceneElement::Dialogue
            || m_element->type() == SceneElement::Parenthetical)) {
        Scene *scene = m_element->scene();
        SceneElement *element = const_cast<SceneElement *>(m_element);
        int index = m_element->scene()->indexOfElement(element) - 1;
        while (index >= 0) {
            element = scene->elementAt(index);
            if (element->type() == SceneElement::Character)
                return element;
            --index;
        }
    }

    return nullptr;
}

QString ScreenplayParagraphBlockData::getCharacterElementText() const
{
    const SceneElement *element = this->getCharacterElement();
    return element ? element->formattedText() : QString();
}

ScreenplayParagraphBlockData *ScreenplayParagraphBlockData::get(const QTextBlock &block)
{
    return get(block.userData());
}

ScreenplayParagraphBlockData *ScreenplayParagraphBlockData::get(QTextBlockUserData *userData)
{
    if (userData == nullptr)
        return nullptr;

    ScreenplayParagraphBlockData *userData2 =
            reinterpret_cast<ScreenplayParagraphBlockData *>(userData);
    return (userData2->type == ScreenplayParagraphBlockData::Type) ? userData2 : nullptr;
}

///////////////////////////////////////////////////////////////////////////////

class ScreenplayTextDocumentUpdate
{
public:
    ScreenplayTextDocumentUpdate(ScreenplayTextDocument *document) : m_document(document)
    {
        if (m_document) {
            m_document->setUpdating(true);
            if (m_document->m_progressReport)
                m_document->m_progressReport->start();
        }
    }
    ~ScreenplayTextDocumentUpdate()
    {
        if (m_document) {
            m_document->setUpdating(false);
            if (m_document->m_progressReport)
                m_document->m_progressReport->finish();
        }
    }

private:
    ScreenplayTextDocument *m_document = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

ScreenplayTextDocument::ScreenplayTextDocument(QObject *parent)
    : QObject(parent),
      m_injection(this, "injection"),
      m_screenplay(this, "screenplay"),
      m_textDocument(this, "textDocument"),
      m_formatting(this, "formatting")
{
    m_textDocument = new QTextDocument(this);
    this->init();
}

ScreenplayTextDocument::ScreenplayTextDocument(QTextDocument *document, QObject *parent)
    : QObject(parent),
      m_injection(this, "injection"),
      m_screenplay(this, "screenplay"),
      m_textDocument(this, "textDocument"),
      m_formatting(this, "formatting")
{
    m_textDocument = document;
    this->init();
}

ScreenplayTextDocument::~ScreenplayTextDocument()
{
    m_sceneResetTimer.stop();
    m_loadScreenplayTimer.stop();
    m_pageBoundaryEvalTimer.stop();

    if (m_textDocument != nullptr && m_textDocument->parent() == this)
        m_textDocument->setUndoRedoEnabled(true);
}

int ScreenplayTextDocument::headingFontPointSize(int headingLevel)
{
    static QList<int> fontSizes({ 22, 20, 18, 16, 14 });
    return fontSizes[qBound(0, headingLevel, fontSizes.size() - 1)];
}

void ScreenplayTextDocument::setTextDocument(QTextDocument *val)
{
    if (m_textDocument != nullptr && m_textDocument == val)
        return;

    if (m_textDocument != nullptr && m_textDocument->parent() == this)
        delete m_textDocument;

    m_textDocument = val ? val : new QTextDocument(this);
    m_textDocument->setUndoRedoEnabled(false);
    this->loadScreenplayLater();

    emit textDocumentChanged();
}

void ScreenplayTextDocument::setScreenplay(Screenplay *val)
{
    if (m_screenplay == val)
        return;

    this->disconnectFromScreenplaySignals();

    if (m_screenplay)
        disconnect(m_screenplay, &Screenplay::aboutToDelete, this,
                   &ScreenplayTextDocument::resetScreenplay);

    m_screenplay = val;

    if (m_screenplay)
        connect(m_screenplay, &Screenplay::aboutToDelete, this,
                &ScreenplayTextDocument::resetScreenplay);

    this->loadScreenplayLater();

    emit screenplayChanged();
}

void ScreenplayTextDocument::setFormatting(ScreenplayFormat *val)
{
    if (m_formatting == val)
        return;

    this->disconnectFromScreenplayFormatSignals();

    if (m_formatting && m_formatting->parent() == this)
        GarbageCollector::instance()->add(m_formatting);

    m_formatting = val;

#if 0
    this->formatAllBlocks();
    this->connectToScreenplayFormatSignals();
#else
    // It is less time consuming to reload the whole document than it is
    // to apply formatting. This is mostly because iterating over text blocks
    // in a document is more expensive than just creating them from scratch
    this->loadScreenplayLater();
#endif

    emit formattingChanged();
}

void ScreenplayTextDocument::resetFormatting()
{
    m_formatting = nullptr;
    this->loadScreenplayLater();
    emit formattingChanged();
}

void ScreenplayTextDocument::resetTextDocument()
{
    m_textDocument = new QTextDocument(this);
    m_textDocument->setUndoRedoEnabled(false);
    this->loadScreenplayLater();
    emit textDocumentChanged();
}

void ScreenplayTextDocument::setTitlePage(bool val)
{
    if (m_titlePage == val)
        return;

    m_titlePage = val;
    emit titlePageChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setIncludeLoglineInTitlePage(bool val)
{
    if (m_includeLoglineInTitlePage == val)
        return;

    m_includeLoglineInTitlePage = val;
    emit includeLoglineInTitlePageChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSceneNumbers(bool val)
{
    if (m_sceneNumbers == val)
        return;

    m_sceneNumbers = val;
    emit sceneNumbersChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSceneIcons(bool val)
{
    if (m_sceneIcons == val)
        return;

    m_sceneIcons = val;
    emit sceneIconsChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSceneColors(bool val)
{
    if (m_sceneColors == val)
        return;

    m_sceneColors = val;
    emit sceneColorsChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSyncEnabled(bool val)
{
    if (m_syncEnabled == val)
        return;

    m_syncEnabled = val;
    m_loadScreenplayTimer.stop();

    if (m_syncEnabled) {
        this->connectToScreenplaySignals();
        this->connectToScreenplayFormatSignals();
        this->syncNow();
    } else {
        this->disconnectFromScreenplaySignals();
        this->disconnectFromScreenplayFormatSignals();
    }

    emit syncEnabledChanged();
}

void ScreenplayTextDocument::setListSceneCharacters(bool val)
{
    if (m_listSceneCharacters == val)
        return;

    m_listSceneCharacters = val;
    emit listSceneCharactersChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setHighlightDialoguesOf(QStringList val)
{
    if (m_highlightDialoguesOf == val)
        return;

    m_highlightDialoguesOf = val;
    emit highlightDialoguesOfChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setIncludeSceneSynopsis(bool val)
{
    if (m_includeSceneSynopsis == val)
        return;

    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setIncludeSceneFeaturedImage(bool val)
{
    if (m_includeSceneFeaturedImage == val)
        return;

    m_includeSceneFeaturedImage = val;
    emit includeSceneFeaturedImageChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setIncludeSceneComments(bool val)
{
    if (m_includeSceneComments == val)
        return;

    m_includeSceneComments = val;
    emit includeSceneCommentsChanged();
}

void ScreenplayTextDocument::setPurpose(ScreenplayTextDocument::Purpose val)
{
    if (m_purpose == val)
        return;

    m_purpose = val;
    emit purposeChanged();
}

void ScreenplayTextDocument::setPrintEachSceneOnANewPage(bool val)
{
    if (m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setPrintEachActOnANewPage(bool val)
{
    if (m_printEachActOnANewPage == val)
        return;

    m_printEachActOnANewPage = val;
    emit printEachActOnANewPageChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setIncludeActBreaks(bool val)
{
    if (m_includeActBreaks == val)
        return;

    m_includeActBreaks = val;
    emit includeActBreaksChanged();
}

void ScreenplayTextDocument::setTitlePageIsCentered(bool val)
{
    if (m_titlePageIsCentered == val)
        return;

    m_titlePageIsCentered = val;
    emit titlePageIsCenteredChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setIncludeMoreAndContdMarkers(bool val)
{
    if (m_includeMoreAndContdMarkers == val)
        return;

    m_includeMoreAndContdMarkers = val;
    emit includeMoreAndContdMarkersChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSecondsPerPage(int val)
{
    val = qBound(15, val, 300);
    const int secs = val % 60;
    const int mins = (val - secs) / 60;
    this->setTimePerPage(QTime(0, mins, secs));
}

int ScreenplayTextDocument::secondsPerPage() const
{
    return m_timePerPage.minute() * 60 + m_timePerPage.second();
}

void ScreenplayTextDocument::setTimePerPage(const QTime &val)
{
    if (m_timePerPage == val)
        return;

    m_timePerPage = val;
    emit timePerPageChanged();

    this->evaluatePageBoundariesLater();
}

QString ScreenplayTextDocument::timePerPageAsString() const
{
    return timeToString(m_timePerPage);
}

QString ScreenplayTextDocument::totalTimeAsString() const
{
    return timeToString(m_totalTime);
}

QString ScreenplayTextDocument::currentTimeAsString() const
{
    return timeToString(m_currentTime);
}

void ScreenplayTextDocument::print(QObject *printerObject)
{
    HourGlass hourGlass;

    if (m_textDocument == nullptr || m_screenplay == nullptr || m_formatting == nullptr)
        return;

    if (m_loadScreenplayTimer.isActive() || m_textDocument->isEmpty())
        this->syncNow();

    QPagedPaintDevice *printer = nullptr;

    QPdfWriter *pdfWriter = qobject_cast<QPdfWriter *>(printerObject);
    PrinterObject *qprinter = pdfWriter ? nullptr : qobject_cast<PrinterObject *>(printerObject);

    if (pdfWriter) {
        printer = pdfWriter;

        pdfWriter->setTitle(m_screenplay->title());
        pdfWriter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                              + qApp->applicationVersion() + QStringLiteral(" PdfWriter"));
        pdfWriter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
    } else if (qprinter) {
        printer = qprinter;

        qprinter->setDocName(m_screenplay->title());
        qprinter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                             + qApp->applicationVersion() + QStringLiteral(" PdfWriter"));
        if (qprinter->outputFormat() == QPrinter::PdfFormat)
            qprinter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
    }

    if (printer) {
        m_formatting->pageLayout()->configure(printer);
        printer->setPageMargins(QMarginsF(0.2, 0.1, 0.2, 0.1), QPageLayout::Inch);
    }

    QTextDocumentPagedPrinter docPrinter;
    docPrinter.header()->setVisibleFromPageOne(!m_titlePage);
    docPrinter.footer()->setVisibleFromPageOne(!m_titlePage);
    docPrinter.watermark()->setVisibleFromPageOne(!m_titlePage);
    docPrinter.print(m_textDocument, printer);
}

QList<QPair<int, int>> ScreenplayTextDocument::pageBreaksFor(ScreenplayElement *element) const
{
    QList<QPair<int, int>> ret;
    if (element == nullptr)
        return ret;

    QTextFrame *frame = this->findTextFrame(element);
    if (frame == nullptr)
        return ret;

    // We need to know three positions within each scene frame.
    // 1. Start of the frame
    // 2. Start of the first paragraph in the scene (after the scene heading)
    // 3. End of the scene frame
    QTextCursor cursor = frame->firstCursorPosition();
    QTextBlock block = cursor.block();
    ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
    if (blockData && blockData->elementType() == SceneElement::Heading)
        block = block.next();

    // This is the range of cursor positions inside the frame
    int sceneHeadingStart = frame->firstPosition();
    int paragraphStart = block.position();
    int paragraphEnd = frame->lastPosition();

    // This method includes 'pageBorderPosition' and 'pageNumber' in the returned
    // list If pageBorderPosition lies within the frame, then it is included in
    // the list.
    auto checkAndAdd = [sceneHeadingStart, paragraphStart, paragraphEnd,
                        &ret](int pageBorderPosition, int pageNumber) {
        if (pageBorderPosition >= sceneHeadingStart && pageBorderPosition <= paragraphEnd) {
            const int offset = qMax(pageBorderPosition - paragraphStart, -1);
            if (ret.isEmpty() || ret.last().first != offset)
                ret << qMakePair(offset, pageNumber);
        }
    };

    // Special case for page #1
    if (element == m_screenplay->elementAt(0))
        checkAndAdd(sceneHeadingStart, 1);

    // Now loop through all pages and gather all pages that lie within the scene
    // boundaries
    for (int i = 0; i < m_pageBoundaries.count(); i++) {
        const QPair<int, int> pgBoundary = m_pageBoundaries.at(i);
        if (pgBoundary.first > paragraphEnd)
            break;

        checkAndAdd(pgBoundary.first, i + 1);
    }

    return ret;
}

QTime ScreenplayTextDocument::lengthInTime(ScreenplayElement *from, ScreenplayElement *to) const
{
    const qreal nrPages = this->lengthInPages(from, to);
    const QTime ret = ::secondsToTime(secondsPerPage() * nrPages);
    return ret;
}

QString ScreenplayTextDocument::lengthInTimeAsString(ScreenplayElement *from,
                                                     ScreenplayElement *to) const
{
    const QTime time = this->lengthInTime(from, to);
    return ::timeToString(time);
}

qreal ScreenplayTextDocument::lengthInPixels(ScreenplayElement *from, ScreenplayElement *to) const
{
    if (m_screenplay == nullptr || m_formatting == nullptr || from == nullptr)
        return 0;

    if ((from && from->screenplay() != m_screenplay) || (to && to->screenplay() != m_screenplay))
        return 0;

    const int fromIndex = m_screenplay->indexOfElement(from);
    const int toIndex = to ? m_screenplay->indexOfElement(to) : fromIndex;

    qreal ret = 0;
    for (int i = fromIndex; i <= toIndex; i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        QTextFrame *frame = this->findTextFrame(element);
        if (frame == nullptr)
            continue;

        QAbstractTextDocumentLayout *layout = m_textDocument->documentLayout();
        ret += layout->frameBoundingRect(frame).height();
    }

    return ret;
}

qreal ScreenplayTextDocument::lengthInPages(ScreenplayElement *from, ScreenplayElement *to) const
{
    if (m_screenplay == nullptr || m_formatting == nullptr)
        return 0;

    if ((from && from->screenplay() != m_screenplay) || (to && to->screenplay() != m_screenplay))
        return 0;

    const qreal pxLength = this->lengthInPixels(from, to);
    if (qFuzzyIsNull(pxLength))
        return 0;

    const QTextFrameFormat rootFrameFormat = m_textDocument->rootFrame()->frameFormat();
    const qreal topMargin = rootFrameFormat.topMargin();
    const qreal bottomMargin = rootFrameFormat.bottomMargin();
    const qreal pageLength = m_textDocument->pageSize().height() - topMargin - bottomMargin;
    if (qFuzzyIsNull(pageLength))
        return 0;

    return pxLength / pageLength;
}

void ScreenplayTextDocument::setInjection(QObject *val)
{
    if (m_injection == val)
        return;

    if (m_injection != val)
        disconnect(m_injection, &QObject::destroyed, this, &ScreenplayTextDocument::resetInjection);

    m_injection = val;

    if (m_injection != val)
        connect(m_injection, &QObject::destroyed, this, &ScreenplayTextDocument::resetInjection);

    emit injectionChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::syncNow(ProgressReport *progress)
{
    m_progressReport = progress;

    m_loadScreenplayTimer.stop();
    this->loadScreenplay();

    m_progressReport = nullptr;
}

/*
This function is experiemental, which is the reason why we dont make it
accessible via a button or menu option on the GUI. This function can be invoked
only from the scripting interface, which is also an experimental feature.

If you ran the following script, then you would be able to get a fairly good
super-imposition of the save the cat structure on an existing screenplay.

var structure = {
 "name": "Save The Cat",
 "pageCount": 110,
 "elements": [
     {"name": "Opening Image", "page": 1, "act": "ACT 1" },
     {"name": "Setup", "page": "1-10", "act": "ACT 1" },
     {"name": "Theme Stated", "page": 5, "act": "ACT 1", "allowMultiple": true
},
     {"name": "Catalyst", "page": 12, "act": "ACT 1" },
     {"name": "Debate", "page": "12-25", "act": "ACT 1"},
     {"name": "Break Into Two", "page": "25-30", "act": "ACT 1" },
     {"name": "B Story", "page": 30, "act": "ACT 2A" },
     {"name": "Fun And Games", "page": "30-55", "act": "ACT 2A" },
     {"name": "Midpoint", "page": 55, "act": "ACT 2A" },
     {"name": "Bad Guys Close In", "page": "55-75", "act": "ACT 2B" },
     {"name": "All Is Lost", "page": 75, "act": "ACT 2B" },
     {"name": "Dark Night Of The Soul", "page": "75-85", "act": "ACT 2B" },
     {"name": "Break Into Three", "page": 85, "act": "ACT 2B"},
     {"name": "Finale", "page": "85-110", "act": "ACT 3" },
     {"name": "Final Image", "page": 110, "act": "ACT 3" }
 ]
}

screenplayTextDocument.superImposeStructure(structure)
*/
void ScreenplayTextDocument::superImposeStructure(const QJsonObject &model)
{
    if (m_screenplay == nullptr || m_screenplay->scriteDocument() == nullptr
        || m_textDocument == nullptr)
        return;

    if (m_purpose == ForPrinting)
        return;

    ScriteDocument *document = m_screenplay->scriteDocument();
    Structure *structure = document->structure();
    const int nrPages = model.value(QStringLiteral("pageCount")).toInt();
    if (nrPages <= 0)
        return;

    const QString tagGroup = model.value(QStringLiteral("name")).toString();
    struct TagInfo
    {
        QString name;
        QString act;
        QString id;
        int fromPageNr = 0;
        int toPageNr = 0;
        bool allowMultiple = false;
    };
    QVector<TagInfo> tags;

    /**
     * First lets parse model and set groups data on structure.
     */
    {
        QString groupsData;
        QTextStream ts(&groupsData, QIODevice::WriteOnly);

        ts << "[" << tagGroup << "]\n";

        const QJsonArray elements = model.value(QStringLiteral("elements")).toArray();
        tags.reserve(elements.size());

        for (const QJsonValue &item : elements) {
            const QJsonObject element = item.toObject();

            TagInfo tag;
            tag.name = element.value(QStringLiteral("name")).toString();
            tag.act = element.value(QStringLiteral("act")).toString();
            tag.allowMultiple =
                    element.value(QStringLiteral("allowMultiple")).toBool(tag.allowMultiple);

            const QJsonValue pageValue = element.value(QStringLiteral("page"));
            const QString page = pageValue.toVariant().toString();
            if (page.contains(QStringLiteral("-"))) {
                const QStringList fields = page.split(QStringLiteral("-"), Qt::SkipEmptyParts);
                const int a = fields.size() >= 2 ? fields.first().toInt() : 0;
                const int b = fields.size() >= 2 ? fields.last().toInt() : 0;
                if (a == 0 || b == 0)
                    return;

                tag.fromPageNr = qMin(a, b);
                tag.toPageNr = qMax(a, b);
            } else {
                tag.fromPageNr = page.toInt();
                if (tag.fromPageNr == 0)
                    return;

                tag.toPageNr = tag.fromPageNr;
            }

            tag.id = tagGroup + QStringLiteral("/") + tag.name + QStringLiteral(" (") + page
                    + QStringLiteral(")");
            tag.id = tag.id.toUpper();

            ts << "<" << tag.act << ">" << tag.name << " (" << page << ")\n";
            tags.append(tag);
        }

        ts.flush();

        if (tags.isEmpty())
            return;

        structure->setGroupsData(groupsData);
    }

    // Lets sync the document if it has not already been done
    if (m_loadScreenplayTimer.isActive())
        this->syncNow();

    // What we have in the screenplay is (potentially) a bunch of episodes.
    // Each episode made up of scenes with their corresponding text frames
    struct _SceneFrame
    {
        StructureElement *element = nullptr;
        QTextFrame *textFrame = nullptr;
        qreal startPage = 0;
        qreal endPage = 0;
        qreal pageCount = 0;
    };

    struct _Episode
    {
        QList<_SceneFrame> sceneFrames;
        qreal pageCount = 0;
    };

    QList<_Episode> episodes;

    // First lets gather all episodes from the screenplay. There will always be
    // atleast one episode.
    QList<ScreenplayElement *> actBreaksToRemove;
    const int nrElements = m_screenplay->elementCount();
    for (int i = 0; i < nrElements; i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        if (element->elementType() != ScreenplayElement::SceneElementType) {
            if (element->breakType() == Screenplay::Episode)
                episodes.append(_Episode());
            else
                actBreaksToRemove.append(element);
            continue;
        }

        Scene *scene = element->scene();
        if (scene == nullptr)
            continue;

        StructureElement *selement = scene->structureElement();
        if (selement == nullptr)
            continue;

        QTextFrame *frame = this->findTextFrame(element);
        if (frame == nullptr)
            return;

        if (episodes.isEmpty())
            episodes.append(_Episode());

        _SceneFrame sceneFrame;
        sceneFrame.element = selement;
        sceneFrame.textFrame = frame;
        episodes.last().sceneFrames.append(sceneFrame);
    }

    auto findTags = [tags](int pageNr) -> QVector<TagInfo> {
        QVector<TagInfo> ret;
        for (const TagInfo &tag : qAsConst(tags)) {
            if (pageNr >= tag.fromPageNr && pageNr <= tag.toPageNr)
                ret.append(tag);
        }
        return ret;
    };

    // Compute height in pixels of each page.
    const QTextFrameFormat rootFrameFormat = m_textDocument->rootFrame()->frameFormat();
    const qreal topMargin = rootFrameFormat.topMargin();
    const qreal bottomMargin = rootFrameFormat.bottomMargin();
    const qreal pageLength = m_textDocument->pageSize().height() - topMargin - bottomMargin;

    // Now lets compute page extents of each episode and each scene in those
    // episodes.
    QAbstractTextDocumentLayout *layout = m_textDocument->documentLayout();
    for (_Episode &episode : episodes) {
        if (episode.sceneFrames.isEmpty())
            episode.pageCount = 0;
        else {
            qreal episodeY = -1;
            for (_SceneFrame &sceneFrame : episode.sceneFrames) {
                const QRectF rect = layout->frameBoundingRect(sceneFrame.textFrame);
                if (episodeY < 0)
                    episodeY = rect.top();

                sceneFrame.startPage = (rect.top() - episodeY) / pageLength;
                sceneFrame.endPage = (rect.bottom() - episodeY) / pageLength;
                sceneFrame.pageCount = sceneFrame.endPage - sceneFrame.startPage;
            }

            episode.pageCount =
                    episode.sceneFrames.last().endPage - episode.sceneFrames.first().startPage;
        }
    }

    // Now we go ahead and apply tags to each scene.
    for (const _Episode &episode : qAsConst(episodes)) {
        if (episode.pageCount == 0)
            continue;

        const qreal pageScale = qreal(nrPages) / episode.pageCount;
        for (const _SceneFrame &sceneFrame : qAsConst(episode.sceneFrames)) {
            const int pageNr = 1 + floor(sceneFrame.startPage * pageScale);
            const QVector<TagInfo> sceneTags = findTags(pageNr);
            if (sceneTags.isEmpty())
                continue;

            QStringList tagGroups;
            if (sceneTags.size() == 0)
                tagGroups << sceneTags.first().id;
            else {
                for (const TagInfo &sceneTag : sceneTags) {
                    if (sceneTag.fromPageNr == sceneTag.toPageNr)
                        tagGroups.clear();
                    tagGroups << sceneTag.id;
                    if (sceneTag.fromPageNr == sceneTag.toPageNr)
                        break;
                }
            }

            sceneFrame.element->scene()->setGroups(tagGroups);
        }
    }

    // Insert act breaks
    QString actName = tags.first().act;
    int startingElementIndex = 0;
    while (!tags.isEmpty()) {
        if (tags.first().act == actName) {
            tags.takeFirst();
            continue;
        }

        for (int i = startingElementIndex; i < m_screenplay->elementCount(); i++) {
            ScreenplayElement *element = m_screenplay->elementAt(i);
            if (element->scene() == nullptr)
                continue;

            if (element->scene()->groups().contains(tags.first().id)) {
                ScreenplayElement *actBreak = new ScreenplayElement(m_screenplay);
                actBreak->setElementType(ScreenplayElement::BreakElementType);
                actBreak->setBreakType(Screenplay::Act);
                m_screenplay->insertElementAt(actBreak, i);
                startingElementIndex = i + 1;
                break;
            }
        }

        actName = tags.first().act;
        tags.takeFirst();
    }

    // Remove all act breaks that existed before.
    while (!actBreaksToRemove.isEmpty())
        m_screenplay->removeElement(actBreaksToRemove.takeLast());

    // Rework all numbers.
    m_screenplay->evaluateSceneNumbers();

    // Set the preferred grouping to the newly loaded tag group.
    structure->setPreferredGroupCategory(tagGroup);

    // Place index cards in beat board layout
    structure->placeElementsInBeatBoardLayout(m_screenplay);
    structure->setForceBeatBoardLayout(true);
}

void ScreenplayTextDocument::reload()
{
    this->loadScreenplayLater();
}

void ScreenplayTextDocument::classBegin()
{
    m_updating = true;
    m_componentComplete = false;
}

void ScreenplayTextDocument::componentComplete()
{
    m_updating = false;
    m_componentComplete = true;

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_loadScreenplayTimer.timerId()) {
        this->syncNow();
        this->connectToScreenplaySignals();
        this->connectToScreenplayFormatSignals();
    } else if (event->timerId() == m_pageBoundaryEvalTimer.timerId()) {
        m_pageBoundaryEvalTimer.stop();
        this->evaluatePageBoundaries();
    } else if (event->timerId() == m_sceneResetTimer.timerId()) {
        m_sceneResetTimer.stop();
        this->processSceneResetList();
    } else
        QObject::timerEvent(event);
}

void ScreenplayTextDocument::init()
{
    if (m_textDocument == nullptr)
        m_textDocument = new QTextDocument(this);

#ifdef DISPLAY_DOCUMENT_IN_TEXTEDIT
    m_sceneFrameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    m_sceneFrameFormat.setBorderBrush(QBrush(Qt::black));
    m_sceneFrameFormat.setBackground(QBrush(QColor(255, 0, 255, 64)));
#endif
}

void ScreenplayTextDocument::setUpdating(bool val)
{
    if (m_updating == val)
        return;

    m_updating = val;
    emit updatingChanged();

    if (val)
        emit updateStarted();
    else {
        this->evaluatePageBoundariesLater();
        emit updateFinished();
    }
}

void ScreenplayTextDocument::setPageCount(qreal val)
{
    if (qCeil(val) != m_pageCount) {
        m_pageCount = qCeil(val);
        emit pageCountChanged();
    }

    const int secsPerPage =
            m_timePerPage.hour() * 60 * 60 + m_timePerPage.minute() * 60 + m_timePerPage.second();
    const int totalSecs = int(qCeil(val * secsPerPage));
    const QTime totalT = ::secondsToTime(totalSecs);
    if (m_totalTime != totalT) {
        m_totalTime = totalT;
        emit totalTimeChanged();
    }
}

void ScreenplayTextDocument::setCurrentPageAndPosition(int page, qreal pos)
{
    page = m_pageCount > 0 ? qBound(1, page, qCeil(m_pageCount)) : 0;
    if (m_currentPage != page) {
        m_currentPage = page;
        emit currentPageChanged();
    }

    pos = m_pageCount > 0 ? qBound(0.0, pos, 1.0) : 0;
    if (!qFuzzyCompare(pos, m_currentPosition)) {
        m_currentPosition = pos;
        emit currentPositionChanged();
    }

    const int totalSecs =
            m_totalTime.hour() * 60 * 60 + m_totalTime.minute() * 60 + m_totalTime.second();
    const int currentSecs = int(m_currentPosition * qreal(totalSecs));

    const QTime currentT = ::secondsToTime(currentSecs);
    if (m_currentTime != currentT) {
        m_currentTime = currentT;
        emit currentTimeChanged();
    }
}

#ifdef DISPLAY_DOCUMENT_IN_TEXTEDIT
#include <QTextEdit>
#endif // DISPLAY_DOCUMENT_IN_TEXTEDIT

void ScreenplayTextDocument::loadScreenplay()
{
#ifdef DISPLAY_DOCUMENT_IN_TEXTEDIT
    static QTextEdit *textEdit = nullptr;
    if (m_purpose == ForDisplay) {
        if (textEdit == nullptr) {
            textEdit = new QTextEdit;
            textEdit->setDocument(m_textDocument);
            textEdit->setReadOnly(true);
            textEdit->setFixedSize(m_formatting->pageLayout()->paperRect().size().toSize());
        }

        textEdit->show();
    }
#endif // DISPLAY_DOCUMENT_IN_TEXTEDIT

    HourGlass hourGlass;

    if (m_updating || !m_componentComplete) // so that we avoid recursive updates
        return;

    if (!m_screenplayModificationTracker.isModified(m_screenplay)
        && !m_formattingModificationTracker.isModified(m_formatting))
        return;

    ScreenplayTextDocumentUpdate update(this);

    if (m_progressReport) {
        m_progressReport->setProgressStep(1.0 / (m_screenplay->elementCount() + 3));
    }

    // Here we discard anything we have previously loaded and load the entire
    // document fresh from the start.
    this->clearTextFrames();
    m_sceneResetList.clear();
    m_textDocument->clear();
    m_textDocument->setProperty("#characterImageResourceUrls", QVariant());
    m_sceneResetTimer.stop();
    m_pageBoundaryEvalTimer.stop();

    if (m_screenplay == nullptr)
        return;

    if (m_screenplay->elementCount() == 0)
        return;

    if (m_formatting == nullptr)
        this->setFormatting(new ScreenplayFormat(this));

    m_textDocument->setDefaultFont(m_formatting->defaultFont());
    m_formatting->pageLayout()->configure(m_textDocument);
    m_textDocument->setIndentWidth(10);

    if (m_sceneNumbers || (m_purpose == ForPrinting && m_syncEnabled) || m_sceneIcons) {
        ScreenplayTextObjectInterface *toi =
                m_textDocument->findChild<ScreenplayTextObjectInterface *>();
        if (toi == nullptr) {
            toi = new ScreenplayTextObjectInterface(m_textDocument);
            m_textDocument->documentLayout()->registerHandler(ScreenplayTextObjectInterface::Kind,
                                                              toi);
        }
    }

    // const QTextFrameFormat rootFrameFormat =
    // m_textDocument->rootFrame()->frameFormat();

    // So that QTextDocumentPrinter can pick up this for header and footer fields.
    m_textDocument->setProperty("#title", m_screenplay->title());
    m_textDocument->setProperty("#subtitle", m_screenplay->subtitle());
    m_textDocument->setProperty("#author", m_screenplay->author());
    m_textDocument->setProperty("#contact", m_screenplay->contact());
    m_textDocument->setProperty("#version", m_screenplay->version());
    m_textDocument->setProperty("#phone", m_screenplay->phoneNumber());
    m_textDocument->setProperty("#email", m_screenplay->email());
    m_textDocument->setProperty("#website", m_screenplay->website());
    m_textDocument->setProperty("#includeLoglineInTitlePage", m_includeLoglineInTitlePage);

    QTextBlockFormat frameBoundaryBlockFormat;
    frameBoundaryBlockFormat.setLineHeight(0, QTextBlockFormat::FixedHeight);

    QTextCursor cursor(m_textDocument);

    // Title Page
    if (m_titlePage) {
        ScreenplayTitlePageObjectInterface *tpoi =
                m_textDocument->findChild<ScreenplayTitlePageObjectInterface *>();
        if (tpoi == nullptr) {
            tpoi = new ScreenplayTitlePageObjectInterface(m_textDocument);
            m_textDocument->documentLayout()->registerHandler(
                    ScreenplayTitlePageObjectInterface::Kind, tpoi);
        }

        QTextBlockFormat pageBreakFormat;
        pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
        cursor.setBlockFormat(pageBreakFormat);

        QTextCharFormat titlePageFormat;
        titlePageFormat.setObjectType(ScreenplayTitlePageObjectInterface::Kind);
        titlePageFormat.setProperty(ScreenplayTitlePageObjectInterface::ScreenplayProperty,
                                    QVariant::fromValue<QObject *>(m_screenplay));
        titlePageFormat.setProperty(ScreenplayTitlePageObjectInterface::TitlePageIsCentered,
                                    m_titlePageIsCentered);
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), titlePageFormat);
    }

    AbstractScreenplayTextDocumentInjectionInterface *injection =
            qobject_cast<AbstractScreenplayTextDocumentInjectionInterface *>(m_injection);
    if (injection != nullptr)
        injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::AfterTitlePage);

    bool hasEpisdoes = m_screenplay->episodeCount() > 0;
    if (m_screenplay->scriteDocument() == nullptr) {
        const QList<ScreenplayElement *> allElements = m_screenplay->getElements();

        QList<ScreenplayElement *> episodeElements;
        std::copy_if(allElements.begin(), allElements.end(), std::back_inserter(episodeElements),
                     [](ScreenplayElement *e) {
                         return e->elementType() == ScreenplayElement::BreakElementType
                                 && e->breakType() == Screenplay::Episode;
                     });

        hasEpisdoes = !episodeElements.isEmpty();
    }

    const ScreenplayElement *lastPrintedElement = nullptr;

    auto printActBreak = [=](QTextCursor &cursor, const ScreenplayElement *element,
                             bool addPageBreak) {
        QTextBlockFormat actBlockFormat;
        if (addPageBreak)
            actBlockFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysBefore);
        else {
            const SceneElementFormat *firstParaFormat =
                    m_formatting->elementFormat(SceneElement::Heading);
            const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
            const QTextBlockFormat blockFormat =
                    firstParaFormat->createBlockFormat(Qt::Alignment(), &pageWidth);
            actBlockFormat.setTopMargin(blockFormat.topMargin());
        }

        cursor.insertBlock();
        cursor.setBlockFormat(actBlockFormat);

        QTextCharFormat actCharFormat;
        actCharFormat.setFontPointSize(m_textDocument->defaultFont().pointSize());
        actCharFormat.setFontWeight(QFont::ExtraBold);
        cursor.setCharFormat(actCharFormat);

        if (hasEpisdoes
            && (!lastPrintedElement
                || lastPrintedElement->elementType() != ScreenplayElement::BreakElementType))
            cursor.insertText(QStringLiteral("Episode ")
                              + QString::number(element->episodeIndex() + 1)
                              + QStringLiteral(", "));
        cursor.insertText(element->breakTitle());

        if (!element->breakSubtitle().isEmpty())
            cursor.insertText(QStringLiteral(": ") + element->breakSubtitle().toUpper());
    };

    const int fsi = m_screenplay->firstSceneIndex();
    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        const ScreenplayElement *element = m_screenplay->elementAt(i);

        if (m_progressReport) {
            m_progressReport->setProgressText(QString("Processing %1 of %2 elements ...")
                                                      .arg(i + 1)
                                                      .arg(m_screenplay->elementCount() + 1));
            m_progressReport->tick();
        }

        if (!m_printEachSceneOnANewPage) {
            if (hasEpisdoes && element->elementType() == ScreenplayElement::BreakElementType
                && element->breakType() == Screenplay::Episode) {
                QTextBlockFormat episodeBlockFormat;
                if (i > 0)
                    episodeBlockFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysBefore);

                cursor.insertBlock();
                cursor.setBlockFormat(episodeBlockFormat);

                QTextCharFormat episodeCharFormat;
                episodeCharFormat.setFontPointSize(m_textDocument->defaultFont().pointSize() + 2);
                episodeCharFormat.setFontWeight(QFont::ExtraBold);
                cursor.setCharFormat(episodeCharFormat);

                cursor.insertText(element->breakTitle().toUpper());

                if (!element->breakSubtitle().isEmpty())
                    cursor.insertText(QStringLiteral(": ") + element->breakSubtitle().toUpper());

                lastPrintedElement = element;
                continue;
            }

            if (m_printEachActOnANewPage
                && element->elementType() == ScreenplayElement::BreakElementType
                && element->breakType() == Screenplay::Act) {
                printActBreak(cursor, element, i > 0);
                lastPrintedElement = element;
                continue;
            }
        }

        if (m_includeActBreaks && element->elementType() == ScreenplayElement::BreakElementType
            && element->breakType() == Screenplay::Act && lastPrintedElement != element) {
            printActBreak(cursor, element, false);
            lastPrintedElement = element;
            continue;
        }

        if (element->elementType() != ScreenplayElement::SceneElementType)
            continue;

        QTextFrameFormat frameFormat = m_sceneFrameFormat;
        const Scene *scene = element->scene();
        if (scene != nullptr && (i > fsi || element != lastPrintedElement)) {
            SceneElement::Type firstParaType = SceneElement::Heading;
            if (!scene->heading()->isEnabled() && scene->elementCount()) {
                SceneElement *firstPara = scene->elementAt(0);
                firstParaType = firstPara->type();
            }

            const SceneElementFormat *firstParaFormat = m_formatting->elementFormat(firstParaType);
            const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
            const QTextBlockFormat blockFormat =
                    firstParaFormat->createBlockFormat(Qt::Alignment(), &pageWidth);
            frameFormat.setTopMargin(blockFormat.topMargin());
        }

        if ((i > 0 && m_printEachSceneOnANewPage)
            || (m_purpose == ForPrinting && element->isPageBreakBefore()))
            frameFormat.setPageBreakPolicy(QTextFrameFormat::PageBreak_AlwaysBefore);
        if (m_purpose == ForPrinting && element->isPageBreakAfter())
            frameFormat.setPageBreakPolicy(QTextFrameFormat::PageBreak_AlwaysAfter);

        // Each screenplay element (or scene) has its own frame. That makes
        // moving them in one bunch easy.
        QTextFrame *frame = cursor.insertFrame(frameFormat);
        this->registerTextFrame(element, frame);
        this->loadScreenplayElement(element, cursor);

        // We have to move the cursor out of the frame we created for the scene
        // https://doc.qt.io/qt-5/richtext-cursor.html#frames
        cursor = m_textDocument->rootFrame()->lastCursorPosition();
        cursor.setBlockFormat(frameBoundaryBlockFormat);

        lastPrintedElement = element;
    }

    if (injection != nullptr)
        injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::AfterLastScene);

    if (m_includeMoreAndContdMarkers)
        this->includeMoreAndContdMarkers();

    this->evaluatePageBoundariesLater();
}

void ScreenplayTextDocument::includeMoreAndContdMarkers()
{
    if (m_purpose != ForPrinting || !m_includeMoreAndContdMarkers /* || m_syncEnabled*/)
        return;

    /**
      When we print a screenplay, we expect it to do the following

       1. Slug line or Scene Heading cannot come on the last line of the page
       2. Character name cannot be on the last line of the page
       3. If only one line of the dialogue can be squeezed into the last line of
      the page, then we must move it to the next page along with the
      charactername.
       4. If a dialogue spans across page break, then we must insert MORE and
      CONT'D markers, with character name.
       */
    const ScreenplayPageLayout *pageLayout = m_formatting->pageLayout();
    const QMarginsF pageMargins = pageLayout->margins();
    const QFont defaultFont = m_formatting->defaultFont();

    m_textDocument->setDefaultFont(defaultFont);
    pageLayout->configure(m_textDocument);

    QAbstractTextDocumentLayout *layout = m_textDocument->documentLayout();
    QTextCursor endCursor(m_textDocument);
    endCursor.movePosition(QTextCursor::End);

    int nrPages = m_textDocument->pageCount();
    int pageIndex = m_titlePage ? 1 : 0;

    QRectF paperRect = pageLayout->paperRect();

    auto insertPageBreakAfter = [](const QTextBlock &block) {
        QTextBlockFormat blockFormat;
        blockFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
        QTextCursor cursor(block);
        cursor.setPosition(block.position());
        cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
        cursor.mergeBlockFormat(blockFormat);
        cursor.clearSelection();

        // Top Margin of the block moved to the next page should become 0
        cursor.movePosition(QTextCursor::NextBlock);
        blockFormat = cursor.blockFormat();
        blockFormat.setTopMargin(0);
        cursor.setBlockFormat(blockFormat);
    };

    const ScreenplayFormat *format = m_formatting;
    const SceneElementFormat *characterFormat = format->elementFormat(SceneElement::Character);
    const SceneElementFormat *dialogueFormat = format->elementFormat(SceneElement::Dialogue);
    const QFontMetricsF dialogFontMetrics(dialogueFormat->font());
    const int nrCharsPerDialogLine = int(
            qCeil((pageLayout->contentWidth() - pageLayout->leftMargin() - pageLayout->rightMargin()
                   - dialogueFormat->leftMargin() - dialogueFormat->rightMargin())
                  / dialogFontMetrics.averageCharWidth()));

    auto insertMarkers = [=](const QTextBlock &block) {
        ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
        if (blockData == nullptr)
            return;

        const QString characterName = blockData ? blockData->getCharacterElementText() : QString();
        if (characterName.isEmpty())
            return;

        SceneElementFormat *elementFormat = format->elementFormat(blockData->elementType());
        const QFont font = elementFormat->font();

        QTextCursor cursor(block);
        cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);

        // const QTextBlockFormat restoreBlockFormat = cursor.blockFormat();
        // const QTextCharFormat restoreCharFormat = cursor.charFormat();

        QTextBlockFormat pageBreakFormat;
        pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
        cursor.mergeBlockFormat(pageBreakFormat);
        cursor.clearSelection();

        QTextCharFormat moreMarkerFormat;
        moreMarkerFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
        moreMarkerFormat.setFont(font);
        moreMarkerFormat.setForeground(elementFormat->textColor());
        moreMarkerFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty,
                                     ScreenplayTextObjectInterface::MoreMarkerType);
        moreMarkerFormat.setProperty(ScreenplayTextObjectInterface::DataProperty,
                                     QStringLiteral("  (MORE)"));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), moreMarkerFormat);

        QTextBlockFormat characterBlockFormat = characterFormat->createBlockFormat(Qt::Alignment());
        QTextCharFormat characterCharFormat = characterFormat->createCharFormat();
        characterBlockFormat.setTopMargin(0);
        cursor.insertBlock(characterBlockFormat, characterCharFormat);
        // cursor.insertText(characterName);
        if (m_purpose == ForDisplay)
            cursor.insertText(characterName);
        else
            TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, characterName);

        QTextCharFormat contdMarkerFormat;
        contdMarkerFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
        contdMarkerFormat.setFont(characterFormat->font());
        contdMarkerFormat.setForeground(characterFormat->textColor());
        contdMarkerFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty,
                                      ScreenplayTextObjectInterface::ContdMarkerType);
        contdMarkerFormat.setProperty(ScreenplayTextObjectInterface::DataProperty,
                                      QStringLiteral(" (CONT'D)"));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), contdMarkerFormat);
    };

    while (pageIndex < nrPages) {
        paperRect =
                QRectF(0, pageIndex * paperRect.height(), paperRect.width(), paperRect.height());
        const QRectF contentsRect = paperRect.adjusted(pageMargins.left(), pageMargins.top(),
                                                       -pageMargins.right(), -pageMargins.bottom());
        const int lastPosition = pageIndex == nrPages - 1
                ? endCursor.position()
                : layout->hitTest(contentsRect.bottomRight(), Qt::FuzzyHit);

        QTextCursor cursor(m_textDocument);
        cursor.setPosition(lastPosition - 1);

        QTextBlock block = cursor.block();
        ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
        if (blockData == nullptr) {
            block = block.previous();
            blockData = ScreenplayParagraphBlockData::get(block);
        }

        if (blockData) {
            switch (blockData->elementType()) {
            case SceneElement::All:
                break;
            case SceneElement::Character: {
                const SceneElement *element = blockData->element();
                if (element->scene()->elementAt(0) == element)
                    block = block.previous();
                insertPageBreakAfter(block.previous());
            } break;
            case SceneElement::Heading:
                insertPageBreakAfter(block.previous());
                break;
            case SceneElement::Transition:
            case SceneElement::Shot:
            case SceneElement::Action:
                break; // do nothing for these
            case SceneElement::Parenthetical: {
                QTextBlock previousBlock = block.previous();
                ScreenplayParagraphBlockData *previousBlockData =
                        ScreenplayParagraphBlockData::get(previousBlock);
                if (previousBlockData) {
                    if (previousBlockData->elementType() == SceneElement::Character) {
                        previousBlock = previousBlock.previous();
                        insertPageBreakAfter(previousBlock);
                    } else if (previousBlockData->elementType() == SceneElement::Dialogue) {
                        insertMarkers(previousBlock);
                    }
                }
            } break;
            case SceneElement::Dialogue:
                if (block.position() + block.length() - 1 > lastPosition
                    || !blockData->doesDialogueFinish()) {
                    const QString blockText = block.text();
                    const SceneElement *dialogElement = blockData->element();

                    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor, 1);

                    QString blockTextPart1 = cursor.selectedText();
                    blockTextPart1.chop(nrCharsPerDialogLine);
                    while (blockTextPart1.length()
                           && !blockTextPart1.at(blockTextPart1.length() - 1).isSpace())
                        blockTextPart1.chop(1);
                    blockTextPart1.chop(1);

                    // if(blockTextPart1.length() < nrCharsPerDialogLine) {
                    if (blockTextPart1.isEmpty()) {
                        QTextBlock previousBlock = block.previous();
                        ScreenplayParagraphBlockData *previousBlockData =
                                ScreenplayParagraphBlockData::get(previousBlock);
                        if (previousBlockData->elementType() == SceneElement::Character) {
                            if (previousBlockData->isFirstElementInScene())
                                previousBlock = previousBlock.previous();
                            insertPageBreakAfter(previousBlock.previous());
                            break;
                        }
                    }

                    QString blockTextPart2 = blockTextPart1.isEmpty()
                            ? blockText
                            : blockText.mid(blockTextPart1.length() + 1);
                    // Why +1? Because we want to skip the space
                    // between blockTextPart1 & blockTextPart2.

                    cursor.clearSelection();
                    cursor.select(QTextCursor::BlockUnderCursor);
                    cursor.removeSelectedText();

                    QTextBlockFormat dialogBlockFormat =
                            dialogueFormat->createBlockFormat(Qt::Alignment());
                    QTextCharFormat dialogCharFormat = dialogueFormat->createCharFormat();
                    cursor.insertBlock(dialogBlockFormat, dialogCharFormat);
                    if (m_purpose == ForDisplay)
                        cursor.insertText(blockTextPart1);
                    else
                        TransliterationUtils::polishFontsAndInsertTextAtCursor(
                                cursor, blockTextPart1, dialogElement->textFormats());
                    block = cursor.block();
                    block.setUserData(new ScreenplayParagraphBlockData(dialogElement));

                    cursor.insertBlock(dialogBlockFormat, dialogCharFormat);
                    if (m_purpose == ForDisplay)
                        cursor.insertText(blockTextPart2);
                    else
                        TransliterationUtils::polishFontsAndInsertTextAtCursor(
                                cursor, blockTextPart2,
                                [](const QVector<QTextLayout::FormatRange> &formats, int position) {
                                    if (position == 0)
                                        return formats;

                                    QVector<QTextLayout::FormatRange> ret;
                                    for (const QTextLayout::FormatRange &formatRange : formats) {
                                        if (formatRange.start + formatRange.length - 1 < position)
                                            continue;

                                        QTextLayout::FormatRange newFormatRange;
                                        newFormatRange.start = formatRange.start - position;
                                        newFormatRange.length =
                                                formatRange.length + qMin(newFormatRange.start, 0);
                                        newFormatRange.start = qMax(0, newFormatRange.start);
                                        newFormatRange.format = formatRange.format;
                                        ret.append(newFormatRange);
                                    }

                                    return ret;
                                }(dialogElement->textFormats(), blockTextPart1.length() + 1));
                    block = cursor.block();
                    block.setUserData(new ScreenplayParagraphBlockData(dialogElement));

                    insertMarkers(block.previous());
                }
                break;
            }
        }

        nrPages = layout->pageCount();
        ++pageIndex;
    }
}

void ScreenplayTextDocument::loadScreenplayLater()
{
    if (m_textDocument != nullptr)
        m_textDocument->clear();

    if (m_syncEnabled) {
        this->disconnectFromScreenplaySignals();
        this->disconnectFromScreenplayFormatSignals();

        const bool updateWasScheduled = m_loadScreenplayTimer.isActive();
        m_loadScreenplayTimer.start(50, this);
        if (!updateWasScheduled)
            emit updateScheduled();
    }
}

void ScreenplayTextDocument::resetScreenplay()
{
    m_screenplay = nullptr;
    this->loadScreenplayLater();
    emit screenplayChanged();
}

void ScreenplayTextDocument::connectToScreenplaySignals()
{
    if (m_screenplay == nullptr || !m_syncEnabled || m_connectedToScreenplaySignals)
        return;

    connect(m_screenplay, &Screenplay::elementMoved, this, &ScreenplayTextDocument::onSceneMoved,
            Qt::UniqueConnection);
    connect(m_screenplay, &Screenplay::modelReset, this, &ScreenplayTextDocument::onScreenplayReset,
            Qt::UniqueConnection);
    connect(m_screenplay, &Screenplay::elementRemoved, this,
            &ScreenplayTextDocument::onSceneRemoved, Qt::UniqueConnection);
    connect(m_screenplay, &Screenplay::elementInserted, this,
            &ScreenplayTextDocument::onSceneInserted, Qt::UniqueConnection);
    connect(m_screenplay, &Screenplay::activeSceneChanged, this,
            &ScreenplayTextDocument::onActiveSceneChanged, Qt::UniqueConnection);
    connect(m_screenplay, &Screenplay::modelAboutToBeReset, this,
            &ScreenplayTextDocument::onScreenplayAboutToReset, Qt::UniqueConnection);
    connect(m_screenplay, &Screenplay::elementOmitted, this,
            &ScreenplayTextDocument::onSceneOmitted, Qt::UniqueConnection);
    connect(m_screenplay, &Screenplay::elementIncluded, this,
            &ScreenplayTextDocument::onSceneIncluded, Qt::UniqueConnection);

    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        Scene *scene = element->scene();
        if (scene == nullptr)
            continue;

        this->connectToSceneSignals(scene);
    }

    this->onActiveSceneChanged();

    m_connectedToScreenplaySignals = true;
}

void ScreenplayTextDocument::connectToScreenplayFormatSignals()
{
    if (m_formatting == nullptr || !m_syncEnabled || m_connectedToFormattingSignals)
        return;

    connect(m_formatting, &ScreenplayFormat::defaultFontChanged, this,
            &ScreenplayTextDocument::onDefaultFontChanged, Qt::UniqueConnection);
    connect(m_formatting, &ScreenplayFormat::screenChanged, this,
            &ScreenplayTextDocument::onFormatScreenChanged, Qt::UniqueConnection);
    connect(m_formatting, &ScreenplayFormat::fontPointSizeDeltaChanged, this,
            &ScreenplayTextDocument::onFormatFontPointSizeDeltaChanged, Qt::UniqueConnection);

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        SceneElementFormat *elementFormat = m_formatting->elementFormat(i);
        connect(elementFormat, &SceneElementFormat::elementFormatChanged, this,
                &ScreenplayTextDocument::onElementFormatChanged, Qt::UniqueConnection);
    }

    m_connectedToFormattingSignals = true;
}

void ScreenplayTextDocument::disconnectFromScreenplaySignals()
{
    if (m_screenplay == nullptr || !m_connectedToScreenplaySignals)
        return;

    disconnect(m_screenplay, &Screenplay::elementMoved, this,
               &ScreenplayTextDocument::onSceneMoved);
    disconnect(m_screenplay, &Screenplay::modelReset, this,
               &ScreenplayTextDocument::onScreenplayReset);
    disconnect(m_screenplay, &Screenplay::elementRemoved, this,
               &ScreenplayTextDocument::onSceneRemoved);
    disconnect(m_screenplay, &Screenplay::elementInserted, this,
               &ScreenplayTextDocument::onSceneInserted);
    disconnect(m_screenplay, &Screenplay::activeSceneChanged, this,
               &ScreenplayTextDocument::onActiveSceneChanged);
    disconnect(m_screenplay, &Screenplay::modelAboutToBeReset, this,
               &ScreenplayTextDocument::onScreenplayAboutToReset);
    disconnect(m_screenplay, &Screenplay::elementOmitted, this,
               &ScreenplayTextDocument::onSceneOmitted);
    disconnect(m_screenplay, &Screenplay::elementIncluded, this,
               &ScreenplayTextDocument::onSceneIncluded);

    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        Scene *scene = element->scene();
        if (scene == nullptr)
            continue;

        this->disconnectFromSceneSignals(scene);
    }

    m_connectedToScreenplaySignals = false;
}

void ScreenplayTextDocument::disconnectFromScreenplayFormatSignals()
{
    if (m_formatting == nullptr || !m_connectedToFormattingSignals)
        return;

    disconnect(m_formatting, &ScreenplayFormat::defaultFontChanged, this,
               &ScreenplayTextDocument::onDefaultFontChanged);
    disconnect(m_formatting, &ScreenplayFormat::screenChanged, this,
               &ScreenplayTextDocument::onFormatScreenChanged);
    disconnect(m_formatting, &ScreenplayFormat::fontPointSizeDeltaChanged, this,
               &ScreenplayTextDocument::onFormatFontPointSizeDeltaChanged);

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        SceneElementFormat *elementFormat = m_formatting->elementFormat(i);
        disconnect(elementFormat, &SceneElementFormat::elementFormatChanged, this,
                   &ScreenplayTextDocument::onElementFormatChanged);
    }

    m_connectedToFormattingSignals = false;
}

void ScreenplayTextDocument::connectToSceneSignals(Scene *scene)
{
    if (scene == nullptr)
        return;

    connect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset,
            Qt::UniqueConnection);
    connect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel,
            Qt::UniqueConnection);
    connect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed,
            Qt::UniqueConnection);
    connect(scene, &Scene::elementCountChanged, this, &ScreenplayTextDocument::onSceneReset,
            Qt::UniqueConnection);
    connect(scene, &Scene::sceneAboutToReset, this, &ScreenplayTextDocument::onSceneAboutToReset,
            Qt::UniqueConnection);
    connect(scene, &Scene::sceneElementChanged, this,
            &ScreenplayTextDocument::onSceneElementChanged, Qt::UniqueConnection);
    connect(scene, &Scene::modelAboutToBeReset, this,
            &ScreenplayTextDocument::onSceneAboutToResetModel, Qt::UniqueConnection);

    SceneHeading *heading = scene->heading();
    connect(heading, &SceneHeading::textChanged, this,
            &ScreenplayTextDocument::onSceneHeadingChanged, Qt::UniqueConnection);
    connect(heading, &SceneHeading::enabledChanged, this,
            &ScreenplayTextDocument::onSceneHeadingChanged, Qt::UniqueConnection);
}

void ScreenplayTextDocument::disconnectFromSceneSignals(Scene *scene)
{
    if (scene == nullptr)
        return;

    disconnect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
    disconnect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel);
    disconnect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed);
    disconnect(scene, &Scene::elementCountChanged, this, &ScreenplayTextDocument::onSceneReset);
    disconnect(scene, &Scene::sceneAboutToReset, this,
               &ScreenplayTextDocument::onSceneAboutToReset);
    disconnect(scene, &Scene::sceneElementChanged, this,
               &ScreenplayTextDocument::onSceneElementChanged);
    disconnect(scene, &Scene::modelAboutToBeReset, this,
               &ScreenplayTextDocument::onSceneAboutToResetModel);

    SceneHeading *heading = scene->heading();
    disconnect(heading, &SceneHeading::textChanged, this,
               &ScreenplayTextDocument::onSceneHeadingChanged);
    disconnect(heading, &SceneHeading::enabledChanged, this,
               &ScreenplayTextDocument::onSceneHeadingChanged);
}

void ScreenplayTextDocument::onScreenplayAboutToReset()
{
    m_screenplayIsBeingReset = true;
}

void ScreenplayTextDocument::onScreenplayReset()
{
    m_screenplayIsBeingReset = false;
    this->loadScreenplay();
}

void ScreenplayTextDocument::onSceneMoved(ScreenplayElement *element, int from, int to)
{
    this->onSceneRemoved(element, from);
    this->onSceneInserted(element, to);
}

void ScreenplayTextDocument::onSceneRemoved(ScreenplayElement *element, int index)
{
    if (m_screenplayIsBeingReset)
        return;

    Q_UNUSED(index)
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument",
               "Document was updating while new scene was removed.");

    Scene *scene = element->scene();
    if (scene == nullptr)
        return;

    QTextFrame *frame = this->findTextFrame(element);
#ifdef QT_NO_DEBUG_OUTPUT
    if (frame == nullptr)
        return;
#else
    Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument",
               "Attempting to remove a scene before it was included in the text "
               "document.");
#endif

    ScreenplayTextDocumentUpdate update(this);

    QTextCursor cursor = frame->firstCursorPosition();
    cursor.movePosition(QTextCursor::Up);
    cursor.setPosition(frame->lastPosition(), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    this->removeTextFrame(element);

    if (m_sceneResetList.removeOne(scene))
        m_sceneResetTimer.start(100, this);

    this->disconnectFromSceneSignals(scene);

    if (m_syncEnabled)
        this->evaluatePageBoundariesLater();
}

void ScreenplayTextDocument::onSceneInserted(ScreenplayElement *element, int index)
{
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument",
               "Document was updating while new scene was inserted.");

    Scene *scene = element->scene();
    if (scene == nullptr)
        return;

    ScreenplayTextDocumentUpdate update(this);
    QTextFrame *rootFrame = m_textDocument->rootFrame();

    QTextCursor cursor(m_textDocument);
    if (index == m_screenplay->elementCount() - 1)
        cursor = rootFrame->lastCursorPosition();
    else if (index > 0) {
        const int sindex = index;
        ScreenplayElement *before = nullptr;
        while (before == nullptr && index >= 0) {
            before = m_screenplay->elementAt(--index);
            if (before == nullptr || before->scene() == nullptr) {
                before = nullptr;
                continue;
            }
        }
        index = sindex;

        if (before != nullptr) {
            QTextFrame *beforeFrame = this->findTextFrame(before);
#ifdef QT_NO_DEBUG_OUTPUT
            // This cannot be fixed in the next update cycle. The document will
            // be completely out of sync with the scenes. So we should take some
            // time now and deal with it.
            if (beforeFrame == nullptr) {
                this->loadScreenplay();

                beforeFrame = this->findTextFrame(before);

                // Okay, if this is happening again - then something is seriously wrong.
                // We better let the user move on for now and not block the UI.
                if (beforeFrame == nullptr)
                    return;
            }
#else
            Q_ASSERT_X(beforeFrame != nullptr, "ScreenplayTextDocument",
                       "Attempting to insert scene before screenplay is loaded.");
#endif
            cursor = beforeFrame->lastCursorPosition();
            cursor.movePosition(QTextCursor::Down);
        }
    }

    QTextFrameFormat frameFormat = m_sceneFrameFormat;
    const int fsi = m_screenplay->firstSceneIndex();
    if (index > fsi) {
        SceneElement::Type firstParaType = SceneElement::Heading;
        if (!scene->heading()->isEnabled() && scene->elementCount()) {
            SceneElement *firstPara = scene->elementAt(0);
            firstParaType = firstPara->type();
        }

        const SceneElementFormat *firstParaFormat = m_formatting->elementFormat(firstParaType);
        const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
        const QTextBlockFormat blockFormat =
                firstParaFormat->createBlockFormat(Qt::Alignment(), &pageWidth);
        frameFormat.setTopMargin(blockFormat.topMargin());
    }

    QTextFrame *frame = cursor.insertFrame(frameFormat);
    this->registerTextFrame(element, frame);
    this->loadScreenplayElement(element, cursor);

    if (m_syncEnabled) {
        this->connectToSceneSignals(scene);
        this->evaluatePageBoundariesLater();
    }
}

void ScreenplayTextDocument::onSceneOmitted(ScreenplayElement *element, int index)
{
    this->onSceneRemoved(element, index);
    this->onSceneInserted(element, index);
}

void ScreenplayTextDocument::onSceneIncluded(ScreenplayElement *element, int index)
{
    this->onSceneRemoved(element, index);
    this->onSceneInserted(element, index);
}

void ScreenplayTextDocument::onSceneReset()
{
    Scene *scene = qobject_cast<Scene *>(this->sender());
    if (scene == nullptr) {
        scene = this->sender() && this->sender()->parent()
                ? qobject_cast<Scene *>(this->sender()->parent())
                : nullptr;
        if (scene == nullptr)
            return;
    }

    this->addToSceneResetList(scene);
}

void ScreenplayTextDocument::onSceneRefreshed()
{
    this->onSceneReset();
}

void ScreenplayTextDocument::onSceneAboutToReset()
{
    Scene *scene = qobject_cast<Scene *>(this->sender());
    if (scene == nullptr)
        return;

    this->disconnectFromSceneSignals(scene);
    connect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
}

void ScreenplayTextDocument::onSceneHeadingChanged()
{
    this->onSceneReset();
}

void ScreenplayTextDocument::onSceneElementChanged(SceneElement *para,
                                                   Scene::SceneElementChangeType type)
{
    Scene *scene = qobject_cast<Scene *>(this->sender());
    if (scene == nullptr)
        return;

    Q_ASSERT_X(para->scene() == scene, "ScreenplayTextDocument",
               "Attempting to modify paragraph from outside the scene.");
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument",
               "Document was updating while a scene's paragraph was changed.");

    const int paraIndex = scene->indexOfElement(para);
    if (paraIndex < 0)
        return; // This can happen when the paragraph is not part of the scene
                // text, but it exists as a way to capture a mute-character in the
                // scene.

    ScreenplayTextDocumentUpdate update(this);

    QList<ScreenplayElement *> elements = m_screenplay->sceneElements(scene);
    for (ScreenplayElement *element : qAsConst(elements)) {
        QTextFrame *frame = this->findTextFrame(element);
#ifdef QT_NO_DEBUG_OUTPUT
        // This will probably get updated in the next cycle. Trying to fix
        // this here & right now will likely lead to slow UI updates, which
        // is much worse than having to live with dirty text document.
        if (frame == nullptr)
            continue;
#else
        Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument",
                   "Attempting to update a scene before it was included in the "
                   "text document.");
#endif
        const int nrBlocks = paraIndex + (scene->heading()->isEnabled() ? 1 : 0);

        QTextCursor cursor = frame->firstCursorPosition();
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, nrBlocks);

        QTextBlock block = cursor.block();
        ScreenplayParagraphBlockData *data = ScreenplayParagraphBlockData::get(block);
        if (data && data->contains(para)) {
            if (type == Scene::ElementTypeChange)
                this->formatBlock(block);
            else if (type == Scene::ElementTextChange) {
                if (m_purpose == ForDisplay) {
                    SceneElementBlockTextUpdater *paraUpdater =
                            para->findChild<SceneElementBlockTextUpdater *>(
                                    QString(), Qt::FindDirectChildrenOnly);
                    if (!paraUpdater)
                        paraUpdater = new SceneElementBlockTextUpdater(this, para);
                    paraUpdater->schedule();
                } else
                    this->formatBlock(block, para->text());
            }
        } else
            this->addToSceneResetList(scene);
    }
}

void ScreenplayTextDocument::onSceneAboutToResetModel()
{
    Scene *scene = qobject_cast<Scene *>(this->sender());
    if (scene == nullptr)
        return;

    this->disconnectFromSceneSignals(scene);
    connect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel);
}

void ScreenplayTextDocument::onSceneResetModel()
{
    Scene *scene = qobject_cast<Scene *>(this->sender());
    if (scene == nullptr)
        return;

    disconnect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel);
    this->onSceneReset();
}

void ScreenplayTextDocument::onElementFormatChanged()
{
    SceneElementFormat *seformat = qobject_cast<SceneElementFormat *>(this->sender());
    if (seformat && seformat->isInTransaction())
        return;

    // It is less time consuming to reload the whole document than it is
    // to apply formatting. This is mostly because iterating over text blocks
    // in a document is more expensive than just creating them from scratch
    this->loadScreenplayLater();
}

void ScreenplayTextDocument::onDefaultFontChanged()
{
#if 0
    if(m_updating)
        return;

    ScreenplayTextDocumentUpdate update(this);

    m_textDocument->setDefaultFont(m_formatting->defaultFont());
    this->formatAllBlocks();
#else
    // It is less time consuming to reload the whole document than it is
    // to apply formatting. This is mostly because iterating over text blocks
    // in a document is more expensive than just creating them from scratch
    this->loadScreenplayLater();
#endif
}

void ScreenplayTextDocument::onFormatScreenChanged()
{
    this->evaluatePageBoundariesLater();
}

void ScreenplayTextDocument::onFormatFontPointSizeDeltaChanged()
{
    this->evaluatePageBoundariesLater();
}

void ScreenplayTextDocument::onActiveSceneChanged()
{
    Scene *activeScene = m_screenplay->activeScene();
    if (m_activeScene != activeScene) {
        if (m_activeScene) {
            disconnect(m_activeScene, &Scene::aboutToDelete, this,
                       &ScreenplayTextDocument::onActiveSceneDestroyed);
            disconnect(m_activeScene, &Scene::cursorPositionChanged, this,
                       &ScreenplayTextDocument::onActiveSceneCursorPositionChanged);
        }

        m_activeScene = activeScene;

        if (m_activeScene) {
            connect(m_activeScene, &Scene::aboutToDelete, this,
                    &ScreenplayTextDocument::onActiveSceneDestroyed);
            connect(m_activeScene, &Scene::cursorPositionChanged, this,
                    &ScreenplayTextDocument::onActiveSceneCursorPositionChanged);
        }
    }

    this->evaluateCurrentPageAndPosition();
}

void ScreenplayTextDocument::onActiveSceneDestroyed(Scene *ptr)
{
    if (ptr == m_activeScene)
        m_activeScene = nullptr;
}

void ScreenplayTextDocument::onActiveSceneCursorPositionChanged()
{
    this->evaluateCurrentPageAndPosition();
}

void ScreenplayTextDocument::evaluateCurrentPageAndPosition()
{
    if (m_screenplay == nullptr || m_activeScene == nullptr || m_textDocument == nullptr
        || m_formatting == nullptr) {
        this->setCurrentPageAndPosition(0, 0);
        return;
    }

    if (m_screenplay->currentElementIndex() < 0 || m_textDocument->isEmpty()) {
        this->setCurrentPageAndPosition(0, 0);
        return;
    }

    ScreenplayElement *element = m_screenplay->elementAt(m_screenplay->currentElementIndex());
    QTextFrame *frame = element && element->scene() && element->scene() == m_activeScene
            ? this->findTextFrame(element)
            : nullptr;
    if (frame == nullptr) {
        this->setCurrentPageAndPosition(0, 0);
        return;
    }

    QTextCursor endCursor(m_textDocument);
    endCursor.movePosition(QTextCursor::End);

    if (endCursor.position() == 0) {
        this->setCurrentPageAndPosition(0, 0);
        return;
    }

    const int documentLength = endCursor.position();
    if (documentLength > 0 && m_pageBoundaries.isEmpty())
        this->evaluatePageBoundaries(false);

    QTextCursor userCursor = frame->firstCursorPosition();
    QTextBlock block = userCursor.block();
    ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
    if (blockData && blockData->elementType() == SceneElement::Heading)
        block = block.next();

    const int cursorPosition = m_activeScene->cursorPosition() + block.position();
    for (int i = 0; i < m_pageBoundaries.size(); i++) {
        const QPair<int, int> pgBoundary = m_pageBoundaries.at(i);
        if (cursorPosition >= pgBoundary.first - 1 && cursorPosition < pgBoundary.second) {
            this->setCurrentPageAndPosition(i + 1, qreal(cursorPosition) / qreal(documentLength));
            return;
        }
    }

    // If we are here, then the cursor position was not found anywhere in the
    // pageBoundaries. So, we estimate the current page to be the last page.
    this->setCurrentPageAndPosition(m_pageCount, 1.0);
}

void ScreenplayTextDocument::evaluatePageBoundaries(bool revalCurrentPageAndPosition)
{
    // NOTE: Please do not call this function from anywhere other than
    // timerEvent(), while handling m_pageBoundaryEvalTimer
    QList<QPair<int, int>> pgBoundaries;

    if (m_formatting != nullptr && m_textDocument != nullptr && m_screenplay != nullptr) {
        m_textDocument->setDefaultFont(m_formatting->defaultFont());
        m_formatting->pageLayout()->configure(m_textDocument);

        const ScreenplayPageLayout *pageLayout = m_formatting->pageLayout();
        const QMarginsF pageMargins = pageLayout->margins();

        QRectF paperRect = pageLayout->paperRect();
        QAbstractTextDocumentLayout *layout = m_textDocument->documentLayout();

        const int endCursorPosition = m_textDocument->characterCount() - 1;

        qreal fpageCount = 0.1;

        const int pageCount = m_textDocument->pageCount();
        int pageIndex = 0;
        while (pageIndex < pageCount) {
            paperRect = QRectF(0, pageIndex * paperRect.height(), paperRect.width(),
                               paperRect.height());
            const QRectF contentsRect =
                    paperRect.adjusted(pageMargins.left(), pageMargins.top(), -pageMargins.right(),
                                       -pageMargins.bottom());
            const int firstPosition = pgBoundaries.isEmpty()
                    ? layout->hitTest(contentsRect.topLeft(), Qt::FuzzyHit)
                    : pgBoundaries.last().second + 1;
            const int lastPosition = pageIndex == pageCount - 1
                    ? endCursorPosition
                    : layout->hitTest(contentsRect.bottomRight(), Qt::FuzzyHit);
            pgBoundaries << qMakePair(firstPosition,
                                      lastPosition >= 0 ? lastPosition : endCursorPosition);

            ++pageIndex;

            if (pageIndex == pageCount) {
                ScreenplayElement *lastElement =
                        m_screenplay->elementAt(m_screenplay->elementCount() - 1);
                if (lastElement == nullptr)
                    fpageCount = 0.01;
                else {
                    QTextFrame *lastFrame = this->findTextFrame(lastElement);
                    if (lastFrame == nullptr)
                        fpageCount = pageCount;
                    else {
                        const QRectF lastFrameRect = layout->frameBoundingRect(lastFrame);
                        fpageCount = pageCount - 1;
                        fpageCount += (lastFrameRect.bottom() - contentsRect.top())
                                / contentsRect.height();
                    }
                }
            }
        }

        this->setPageCount(fpageCount);
    }

    m_pageBoundaries = pgBoundaries;
    emit pageBoundariesChanged();

    if (revalCurrentPageAndPosition)
        this->evaluateCurrentPageAndPosition();
}

void ScreenplayTextDocument::evaluatePageBoundariesLater()
{
    m_pageBoundaryEvalTimer.start(500, this);
}

void ScreenplayTextDocument::formatAllBlocks()
{
    if (m_screenplay == nullptr || m_formatting == nullptr || m_updating || !m_componentComplete
        || m_textDocument == nullptr || m_textDocument->isEmpty())
        return;

    QTextCursor cursor(m_textDocument);
    QTextBlock block = cursor.block();
    while (block.isValid()) {
        this->formatBlock(block);
        block = block.next();
    }
}

bool ScreenplayTextDocument::updateFromScreenplayElement(const ScreenplayElement *element)
{
    QTextFrame *frame = this->findTextFrame(element);
    if (frame == nullptr)
        return false;

    Scene *scene = element->scene();
    if (scene == nullptr)
        return false;

    int nrParagraphs = scene->elementCount() + 1;

    // We should have a text-block for each scene element (paragraph),
    // and one for the scene heading
    QList<QPair<SceneElement *, QTextBlock>> paraBlocks;
    paraBlocks.reserve(nrParagraphs);
    paraBlocks << qMakePair(nullptr, QTextBlock());
    for (int i = 0; i < scene->elementCount(); i++)
        paraBlocks << qMakePair(scene->elementAt(i), QTextBlock());

    // Lets go over the whole frame and take stock of what exists
    // in the frame already. While we are at it, lets make note
    // of all blocks that we must remove.
    QTextFrame::iterator it = frame->begin();
    QTextFrame::iterator end = frame->end();
    QVector<QTextBlock> blocksToRemove;
    while (it != end) {
        QTextBlock block = it.currentBlock();
        ScreenplayParagraphBlockData *data = ScreenplayParagraphBlockData::get(block);
        if (data) {
            if (data->element()) {
                SceneElement *para = const_cast<SceneElement *>(data->element());
                int paraIndex = scene->indexOfElement(para);
                if (paraIndex < 0)
                    // This block is no longer backed by an
                    // actual paragraph in the scene. So we should
                    // remove the block.
                    blocksToRemove.append(block);
                else {
                    // This block is required. Lets retain it, but
                    // place it at the appropriate position in the
                    // frame.
                    ++paraIndex;
                    paraBlocks[paraIndex].second = block;
                }
            } else if (frame->begin() == it && data->elementType() == SceneElement::Heading)
                paraBlocks.first().second = block;
            else
                // We honestly dont know what this block is doing.
                blocksToRemove.append(block);
        } else
            // We honestly dont know what this block is doing.
            blocksToRemove.append(block);

        ++it;
    }

    // Remove blocks that are created for paragraphs that no longer exist
    // in the scene.
    while (!blocksToRemove.isEmpty()) {
        QTextCursor cursor(blocksToRemove.takeLast());
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.deleteChar();
    }

    QMap<SceneElement::Type, QPair<QTextCharFormat, QTextBlockFormat>> formatMap;

    auto applyFormattingOnCursor = [&](QTextCursor &cursor, SceneElement::Type paraType,
                                       Qt::Alignment overrideAlignment, bool firstParagraph) {
        QTextBlockFormat blockFormat;
        QTextCharFormat charFormat;

        if (!formatMap.contains(paraType)) {
            const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
            const SceneElementFormat *format = m_formatting->elementFormat(paraType);
            blockFormat = format->createBlockFormat(overrideAlignment, &pageWidth);
            charFormat = format->createCharFormat(&pageWidth);
            formatMap[paraType] = qMakePair(charFormat, blockFormat);
        } else {
            auto formatPair = formatMap.value(paraType);
            charFormat = formatPair.first;
            blockFormat = formatPair.second;
        }

        if (firstParagraph)
            blockFormat.setTopMargin(0);
        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
    };

    // Go over paragraphs and ensure that they exist.
    int position = frame->firstPosition();
#if 0
    SceneElement::Type lastParaType = SceneElement::Heading;
#endif
    for (int i = 0; i < paraBlocks.size(); i++) {
        const QPair<SceneElement *, QTextBlock> item = paraBlocks.at(i);

        QTextBlock block = item.second;
        SceneElement *para = item.first;
        bool newBlock = !block.isValid();
        if (newBlock) {
            QTextCursor cursor(m_textDocument);
            cursor.setPosition(position);
            cursor.insertBlock();
            applyFormattingOnCursor(cursor, para ? para->type() : SceneElement::Heading,
                                    para ? para->alignment() : Qt::Alignment(), i == 0);
            block = cursor.block();
            block.setUserData(new ScreenplayParagraphBlockData(para));
        }

        const QString paraText = para
                ? para->text()
                : (scene->heading()->isEnabled() ? scene->heading()->text()
                                                 : QStringLiteral("NO SCENE HEADING"));

        ScreenplayParagraphBlockData *data = ScreenplayParagraphBlockData::get(block);

        QTextCursor cursor(block);
        if (!newBlock)
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        if (data->isModified())
            cursor.insertText(paraText);
#if 0
        if(lastParaType != data->elementType())
        {
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            applyFormattingOnCursor(cursor, data->elementType(), data->element()->alignment(), i==0);
        }
#endif

        cursor.movePosition(QTextCursor::EndOfBlock);

        position = cursor.position();
#if 0
        lastParaType = data->elementType();
#endif
    }

    return true;
}

void ScreenplayTextDocument::loadScreenplayElement(const ScreenplayElement *element,
                                                   QTextCursor &cursor)
{
    static const QRegularExpression newlinesRegEx("\n+");
    static const QString newline = QStringLiteral("\n");

    Q_ASSERT_X(cursor.currentFrame() == this->findTextFrame(element), "ScreenplayTextDocument",
               "Screenplay element can be loaded only after a frame for it has "
               "been created");

    QTextCharFormat highlightCharFormat;
    highlightCharFormat.setBackground(Qt::yellow);

    const Scene *scene = element->scene();
    if (scene != nullptr) {
        bool insertBlock = false; // the newly inserted frame has a default first block.
                                  // its only from the second paragraph, that we need a new block.

        auto prepareCursor = [=](QTextCursor &cursor, SceneElement::Type paraType,
                                 Qt::Alignment overrideAlignment, bool firstParagraph) {
            const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
            const SceneElementFormat *format = m_formatting->elementFormat(paraType);
            QTextBlockFormat blockFormat = format->createBlockFormat(overrideAlignment, &pageWidth);
            QTextCharFormat charFormat = format->createCharFormat(&pageWidth);
            if (firstParagraph)
                blockFormat.setTopMargin(0);
            cursor.setCharFormat(charFormat);
            cursor.setBlockFormat(blockFormat);
        };

        const SceneHeading *heading = scene->heading();
        const bool headingEnabled =
                heading->isEnabled() || (m_purpose == ForDisplay) || element->isOmitted();
        if (headingEnabled) {
            if (insertBlock)
                cursor.insertBlock();

            QTextBlock block = cursor.block();
            block.setUserData(new ScreenplayParagraphBlockData(nullptr));

            if (m_sceneIcons && m_purpose == ForPrinting) {
                QTextCharFormat sceneIconFormat;
                sceneIconFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
                sceneIconFormat.setFont(m_formatting->elementFormat(SceneElement::Heading)->font());
                sceneIconFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty,
                                            ScreenplayTextObjectInterface::SceneIconType);
                sceneIconFormat.setProperty(ScreenplayTextObjectInterface::DataProperty,
                                            scene->type());
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), sceneIconFormat);
            }

            if (m_sceneNumbers && m_purpose == ForPrinting) {
                QTextCharFormat sceneNumberFormat;
                sceneNumberFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
                sceneNumberFormat.setFont(
                        m_formatting->elementFormat(SceneElement::Heading)->font());
                sceneNumberFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty,
                                              ScreenplayTextObjectInterface::SceneNumberType);
                const QVariantList data = QVariantList()
                        << element->resolvedSceneNumber() << scene->heading()->text()
                        << m_screenplay->indexOfElement(const_cast<ScreenplayElement *>(element));
                sceneNumberFormat.setProperty(ScreenplayTextObjectInterface::DataProperty, data);
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), sceneNumberFormat);
            }

            prepareCursor(cursor, SceneElement::Heading, Qt::Alignment(), !insertBlock);

            if (m_sceneColors) {
                const QColor bgColor = scene->color();

                if (!qFuzzyIsNull(bgColor.alphaF())) {
                    QTextBlockFormat sceneColorFormat;
                    sceneColorFormat.setBackground(Application::tintedColor(bgColor, 0.8));
                    cursor.mergeBlockFormat(sceneColorFormat);
                }
            }

            if (element->isOmitted()) {
                if (m_purpose == ForDisplay && heading->isEnabled() && m_sceneNumbers)
                    cursor.insertText(element->resolvedSceneNumber() + QStringLiteral(". "));

                cursor.insertText(QStringLiteral("[OMITTED] "));

                insertBlock = true;
            }
        }

        if (!element->isOmitted()) {
            if (m_purpose == ForPrinting) {
                if (heading->isEnabled()) {
                    TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor,
                                                                           heading->locationType());
                    cursor.insertText(QStringLiteral(". "));
                    TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor,
                                                                           heading->location());
                    cursor.insertText(QStringLiteral(" - "));
                    TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor,
                                                                           heading->moment());

                    insertBlock = true;
                } /*else {
                    cursor.insertText(QStringLiteral("NO SCENE HEADING"));
                }*/
            } else {
                if (heading->isEnabled()) {
                    if (m_sceneNumbers)
                        cursor.insertText(element->resolvedSceneNumber() + QStringLiteral(". "));
                    cursor.insertText(heading->locationType());
                    cursor.insertText(QStringLiteral(". "));
                    cursor.insertText(heading->location());
                    cursor.insertText(QStringLiteral(" - "));
                    cursor.insertText(heading->moment());

                    insertBlock = true;
                } else {
                    cursor.insertText(QStringLiteral("NO SCENE HEADING"));

                    insertBlock = true;
                }
            }
        }

        if (element->isOmitted())
            return;

        AbstractScreenplayTextDocumentInjectionInterface *injection =
                qobject_cast<AbstractScreenplayTextDocumentInjectionInterface *>(m_injection);
        if (injection != nullptr) {
            injection->setScreenplayElement(element);
            injection->inject(cursor,
                              AbstractScreenplayTextDocumentInjectionInterface::AfterSceneHeading);
            injection->setScreenplayElement(nullptr);
        }

        if (m_listSceneCharacters) {
            const QStringList sceneCharacters = scene->characterNames();
            if (!sceneCharacters.isEmpty()) {
                if (insertBlock)
                    cursor.insertBlock();

                prepareCursor(cursor, SceneElement::Action, Qt::Alignment(), !insertBlock);

                if (m_purpose == ForDisplay) {
                    cursor.insertHtml(QStringLiteral("<strong>Characters: </strong>"));
                    cursor.insertHtml(sceneCharacters.join(QStringLiteral(", ")));
                } else {
                    QFont font = m_textDocument->defaultFont();
                    font.setPointSize(font.pointSize() - 2);

                    const QFontMetrics fontMetrics(font);
                    const qreal ipr = 2.0; // Image Pixel Ratio
                    const int padding = 6;

                    QVariantMap characterImageResourceUrls =
                            m_textDocument->property("#characterImageResourceUrls").toMap();

                    auto insertTextAsImage = [&](const QString &name, const QString &text,
                                                 bool withBackground) {
                        QUrl url;

                        if (characterImageResourceUrls.contains(name)) {
                            url = characterImageResourceUrls.value(name).toUrl();
                        } else {
                            url = QStringLiteral("scrite://character_")
                                    + QString::number(characterImageResourceUrls.size())
                                    + QStringLiteral(".png");

                            QRect textRect = fontMetrics.boundingRect(text);
                            textRect.moveTopLeft(QPoint(0, 0));
                            textRect.setWidth(textRect.width()
                                              + 2 * fontMetrics.averageCharWidth());
                            textRect.adjust(padding, padding, padding, padding);

                            const QSize imageSize =
                                    (textRect.size() + QSize(padding, padding) * 2) * ipr;

                            QImage image(imageSize, QImage::Format_ARGB32);
                            image.setDevicePixelRatio(ipr);
                            image.fill(Qt::transparent);

                            const QRect bgRect = textRect.adjusted(-padding / 2, -padding / 2,
                                                                   padding / 2, padding / 2);

                            QPainter paint(&image);
                            paint.setRenderHint(QPainter::Antialiasing);
                            paint.setRenderHint(QPainter::TextAntialiasing);
                            paint.setFont(font);
                            paint.setPen(QPen(Qt::black, 0.5));
                            if (withBackground) {
                                paint.setBrush(QColor(245, 245, 245));
                                paint.drawRoundedRect(bgRect, bgRect.height() / 2,
                                                      bgRect.height() / 2);
                            }
                            paint.drawText(textRect, Qt::AlignCenter, text);
                            paint.end();

                            characterImageResourceUrls.insert(name, url);
                            m_textDocument->addResource(QTextDocument::ImageResource, url,
                                                        QVariant::fromValue<QImage>(image));
                        }

                        QTextImageFormat imageFormat;
                        imageFormat.setName(url.toString());
                        cursor.insertImage(imageFormat);

                        return url;
                    };

                    insertTextAsImage(QString(), QStringLiteral("Characters:"), false);
                    for (const QString &sceneCharacter : sceneCharacters)
                        insertTextAsImage(sceneCharacter, sceneCharacter, true);

                    m_textDocument->setProperty("#characterImageResourceUrls",
                                                characterImageResourceUrls);
                }

                insertBlock = true;
            }
        }

        if (m_includeSceneFeaturedImage) {
            const Attachments *sceneAttachments = scene->attachments();
            const Attachment *featuredAttachment =
                    sceneAttachments ? sceneAttachments->featuredAttachment() : nullptr;
            const Attachment *featuredImage =
                    featuredAttachment && featuredAttachment->type() == Attachment::Photo
                    ? featuredAttachment
                    : nullptr;

            if (featuredImage) {
                const QUrl url(QStringLiteral("scrite://") + featuredImage->filePath());
                const QImage image(featuredImage->fileSource().toLocalFile());

                m_textDocument->addResource(QTextDocument::ImageResource, url,
                                            QVariant::fromValue<QImage>(image));

                const QSizeF imageSize = image.size().scaled(QSize(320, 240), Qt::KeepAspectRatio);

                QTextBlockFormat blockFormat;
                blockFormat.setTopMargin(10);
                blockFormat.setAlignment(Qt::AlignHCenter);
                cursor.insertBlock(blockFormat);

                insertBlock = true;

                QTextImageFormat imageFormat;
                imageFormat.setName(url.toString());
                imageFormat.setWidth(imageSize.width());
                imageFormat.setHeight(imageSize.height());
                cursor.insertImage(imageFormat);
            }
        }

        if (m_includeSceneSynopsis) {
            const StructureElement *structureElement = scene->structureElement();
            const QString title = structureElement ? structureElement->nativeTitle() : QString();

            QString synopsis = scene->synopsis();
            synopsis = synopsis.replace(newlinesRegEx, newline);

            const bool includingSomething = !title.isEmpty() || !synopsis.isEmpty();

            QTextFrame *frame = cursor.currentFrame();

            if (includingSomething) {
                if (insertBlock)
                    cursor.insertBlock();

                QTextFrameFormat format;
                format.setPadding(5);
                format.setBottomMargin(10);
                format.setBorder(1);
                format.setBorderBrush(scene->color().darker());

                QColor sceneColor = scene->color().lighter(175);
                sceneColor.setAlphaF(0.25);
                format.setBackground(sceneColor);

                cursor.insertFrame(format);

                prepareCursor(cursor, SceneElement::Action, Qt::Alignment(), false);

                if (!title.isEmpty()) {
                    QTextBlockFormat format;
                    format.setTopMargin(m_textDocument->defaultFont().pointSize() / 4);
                    format.setBottomMargin(m_textDocument->defaultFont().pointSize());
                    cursor.mergeBlockFormat(format);

                    QTextCharFormat chFormat;
                    chFormat.setFontWeight(QFont::Bold);
                    cursor.mergeCharFormat(chFormat);

                    TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, title);

                    if (!synopsis.isEmpty())
                        cursor.insertBlock();
                }

                if (!synopsis.isEmpty()) {
                    QTextBlockFormat format;
                    format.setTopMargin(m_textDocument->defaultFont().pointSize() / 4);
                    format.setBottomMargin(m_textDocument->defaultFont().pointSize());
                    cursor.mergeBlockFormat(format);

                    QTextCharFormat chFormat;
                    chFormat.setFontWeight(QFont::Normal);
                    cursor.mergeCharFormat(chFormat);

                    TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, synopsis);
                }

                cursor = frame->lastCursorPosition();
                insertBlock = false;
            } else
                insertBlock = true;
        }

        if (m_includeSceneComments) {
            QString comments = scene->comments().trimmed();
            if (!comments.isEmpty()) {
                comments.replace(newlinesRegEx, newline);

                comments = QLatin1String("Comments: ") + comments;

                QColor sceneColor = scene->color().lighter(175);
                sceneColor.setAlphaF(0.5);

                QTextBlockFormat blockFormat;
                blockFormat.setTopMargin(m_includeSceneSynopsis ? 0 : 10);
                blockFormat.setLeftMargin(15);
                blockFormat.setRightMargin(15);
                blockFormat.setBackground(sceneColor);

                QTextCharFormat charFormat;
                charFormat.setFont(cursor.document()->defaultFont());

                cursor.insertBlock(blockFormat, charFormat);
                TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, comments);

                insertBlock = true;
            }
        }

        bool highlightParagraph = false;
        for (int j = 0; j < scene->elementCount(); j++) {
            const SceneElement *para = scene->elementAt(j);
            if (injection != nullptr) {
                injection->setSceneElement(para);
                injection->inject(
                        cursor,
                        AbstractScreenplayTextDocumentInjectionInterface::BeforeSceneElement);
                if (injection->filterSceneElement()) {
                    injection->inject(
                            cursor,
                            AbstractScreenplayTextDocumentInjectionInterface::AfterSceneElement);
                    continue;
                }
            }

            if (insertBlock)
                cursor.insertBlock();

            QTextBlock block = cursor.block();
            block.setUserData(new ScreenplayParagraphBlockData(para));
            prepareCursor(cursor, para->type(), para->alignment(), !insertBlock);

            if (!m_highlightDialoguesOf.isEmpty()) {
                if (para->type() == SceneElement::Character) {
                    const QString chName = para->text().section('(', 0, 0).trimmed();
                    highlightParagraph =
                            (m_highlightDialoguesOf.contains(chName, Qt::CaseInsensitive));
                } else if (para->type() != SceneElement::Parenthetical
                           && para->type() != SceneElement::Dialogue)
                    highlightParagraph = false;
            }

            if (highlightParagraph)
                cursor.mergeCharFormat(highlightCharFormat);

            const QString text = para->text();
            if (m_purpose == ForPrinting)
                TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, text,
                                                                       para->textFormats());
            else
                cursor.insertText(text);

            if (injection != nullptr)
                injection->inject(
                        cursor,
                        AbstractScreenplayTextDocumentInjectionInterface::AfterSceneElement);

            insertBlock = true;
        }
    }
}

void ScreenplayTextDocument::formatBlock(const QTextBlock &block, const QString &text)
{
    if (m_formatting == nullptr)
        return;

    ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
    if (blockData == nullptr)
        return;

    const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
    const SceneElementFormat *format = m_formatting->elementFormat(blockData->elementType());
    const QTextBlockFormat blockFormat =
            format->createBlockFormat(blockData->element()->alignment(), &pageWidth);
    const QTextCharFormat charFormat = format->createCharFormat(&pageWidth);

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.setBlockFormat(blockFormat);
    cursor.setCharFormat(charFormat);
    if (!text.isEmpty())
        cursor.insertText(text);
}

void ScreenplayTextDocument::removeTextFrame(const ScreenplayElement *element)
{
    this->registerTextFrame(element, nullptr);
}

void ScreenplayTextDocument::registerTextFrame(const ScreenplayElement *element, QTextFrame *frame)
{
    QTextFrame *existingFrame = m_elementFrameMap.value(element, nullptr);
    if (existingFrame != nullptr && existingFrame != frame) {
        m_elementFrameMap.remove(element);
        m_frameElementMap.remove(existingFrame);
        disconnect(existingFrame, &QTextFrame::destroyed, this,
                   &ScreenplayTextDocument::onTextFrameDestroyed);
    }

    if (frame != nullptr) {
        m_elementFrameMap[element] = frame;
        m_frameElementMap[frame] = element;
        connect(frame, &QTextFrame::destroyed, this, &ScreenplayTextDocument::onTextFrameDestroyed);
    }
}

QTextFrame *ScreenplayTextDocument::findTextFrame(const ScreenplayElement *element) const
{
    QTextFrame *frame = m_elementFrameMap.value(element, nullptr);
    if (frame != nullptr) {
        const ScreenplayElement *frameElement = m_frameElementMap.value(frame, nullptr);
        if (frameElement == element)
            return frame;
    }

    return nullptr;
}

void ScreenplayTextDocument::onTextFrameDestroyed(QObject *object)
{
    const ScreenplayElement *element = m_frameElementMap.value(object, nullptr);
    if (element != nullptr)
        m_frameElementMap.remove(object);

    m_elementFrameMap.remove(element);
}

void ScreenplayTextDocument::clearTextFrames()
{
    m_elementFrameMap.clear();

    const QList<QObject *> textFrames = m_frameElementMap.keys();
    for (QObject *textFrame : textFrames)
        disconnect(textFrame, &QTextFrame::destroyed, this,
                   &ScreenplayTextDocument::onTextFrameDestroyed);
    m_frameElementMap.clear();
}

void ScreenplayTextDocument::addToSceneResetList(Scene *scene)
{
    if (scene == nullptr)
        return;

    if (!m_sceneResetList.contains(scene))
        m_sceneResetList.append(scene);

    if (m_sceneResetList.isEmpty())
        return;

    m_sceneResetTimer.start(100, this);
    if (!m_sceneResetHasTriggeredUpdateScheduled) {
        int nrBlocks = 0;
        for (Scene *s : qAsConst(m_sceneResetList)) {
            const QList<int> sil = s->screenplayElementIndexList();
            for (int si : sil) {
                ScreenplayElement *e = m_screenplay->elementAt(si);
                if (e) {
                    int fblocks = 0;

                    QTextFrame *f = m_elementFrameMap.value(e, nullptr);
                    if (f) {
                        const int lastPosition = f->lastPosition();
                        QTextCursor cursor = f->firstCursorPosition();
                        while (cursor.currentFrame() && cursor.currentFrame() == f
                               && cursor.position() <= lastPosition) {
                            ++fblocks;
                            if (!cursor.movePosition(QTextCursor::NextBlock))
                                break;
                        }

                        nrBlocks += qAbs(s->elementCount() - fblocks);
                    }
                }
            }
        }

        if (nrBlocks >= 3) {
            emit updateScheduled();
            m_sceneResetHasTriggeredUpdateScheduled = true;
        }
    }
}

void ScreenplayTextDocument::processSceneResetList()
{
    m_sceneResetHasTriggeredUpdateScheduled = false;

    if (m_sceneResetList.isEmpty())
        return;

    ScreenplayTextDocumentUpdate update(this);

    QList<Scene *> scenes = m_sceneResetList;
    m_sceneResetList.clear();

    while (!scenes.isEmpty()) {
        Scene *scene = scenes.takeFirst();

        const QList<ScreenplayElement *> elements = m_screenplay->sceneElements(scene);
        for (ScreenplayElement *element : elements) {
            QTextFrame *frame = this->findTextFrame(element);
#ifdef QT_NO_DEBUG_OUTPUT
            // This will probably get updated in the next cycle. Trying to fix
            // this here & right now will likely lead to slow UI updates, which
            // is much worse than having to live with dirty text document.
            if (frame == nullptr)
                continue;
#else
            Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument",
                       "Attempting to update a scene before it was included in the "
                       "text document.");
#endif
            if (m_purpose == ForDisplay) {
                if (this->updateFromScreenplayElement(element))
                    continue;
            }

            QTextCursor cursor = frame->firstCursorPosition();
            cursor.setPosition(frame->lastPosition(), QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            this->loadScreenplayElement(element, cursor);
        }

        disconnect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
        this->connectToSceneSignals(scene);
    }

    this->evaluatePageBoundariesLater();
}

void ScreenplayTextDocument::resetInjection()
{
    m_injection = nullptr;
    this->loadScreenplayLater();
    emit injectionChanged();
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayElementPageBreaks::ScreenplayElementPageBreaks(QObject *parent)
    : QObject(parent),
      m_screenplayElement(this, "screenplayElement"),
      m_screenplayDocument(this, "screenplayDocument")
{
}

ScreenplayElementPageBreaks::~ScreenplayElementPageBreaks() { }

void ScreenplayElementPageBreaks::setScreenplayDocument(ScreenplayTextDocument *val)
{
    if (m_screenplayDocument == val)
        return;

    if (m_screenplayDocument != nullptr)
        disconnect(m_screenplayDocument, &ScreenplayTextDocument::pageBoundariesChanged, this,
                   &ScreenplayElementPageBreaks::updatePageBreaks);

    m_screenplayDocument = val;

    if (m_screenplayDocument != nullptr)
        connect(m_screenplayDocument, &ScreenplayTextDocument::pageBoundariesChanged, this,
                &ScreenplayElementPageBreaks::updatePageBreaks);

    emit screenplayDocumentChanged();

    this->updatePageBreaks();
}

void ScreenplayElementPageBreaks::setScreenplayElement(ScreenplayElement *val)
{
    if (m_screenplayElement == val)
        return;

    m_screenplayElement = val;
    emit screenplayElementChanged();

    this->updatePageBreaks();
}

void ScreenplayElementPageBreaks::resetScreenplayDocument()
{
    m_screenplayDocument = nullptr;
    emit screenplayDocumentChanged();
    this->updatePageBreaks();
}

void ScreenplayElementPageBreaks::resetScreenplayElement()
{
    m_screenplayElement = nullptr;
    emit screenplayElementChanged();
    this->updatePageBreaks();
}

void ScreenplayElementPageBreaks::updatePageBreaks()
{
    QJsonArray breaks;

    if (m_screenplayDocument != nullptr && m_screenplayElement != nullptr) {
        const QString positionKey = QStringLiteral("position");
        const QString pageNumberKey = QStringLiteral("pageNumber");

        const QList<QPair<int, int>> ibreaks =
                m_screenplayDocument->pageBreaksFor(m_screenplayElement);
        for (const QPair<int, int> &ibreak : ibreaks) {
            QJsonObject item;
            item.insert(positionKey, ibreak.first);
            item.insert(pageNumberKey, ibreak.second);
            breaks.append(item);
        }
    }

    this->setPageBreaks(breaks);
}

void ScreenplayElementPageBreaks::setPageBreaks(const QJsonArray &val)
{
    if (m_pageBreaks == val)
        return;

    m_pageBreaks = val;
    emit pageBreaksChanged();
}

///////////////////////////////////////////////////////////////////////////////
Q_DECL_IMPORT int qt_defaultDpi();

ScreenplayTitlePageObjectInterface::ScreenplayTitlePageObjectInterface(QObject *parent)
    : QObject(parent)
{
}

ScreenplayTitlePageObjectInterface::~ScreenplayTitlePageObjectInterface() { }

QSizeF ScreenplayTitlePageObjectInterface::intrinsicSize(QTextDocument *doc, int posInDocument,
                                                         const QTextFormat &format)
{
    Q_UNUSED(format)
    Q_UNUSED(posInDocument)

    const QSizeF pageSize = doc->pageSize();
    const QTextFrameFormat rootFrameFormat = doc->rootFrame()->frameFormat();
    const QSizeF ret(
            (pageSize.width() - rootFrameFormat.leftMargin() - rootFrameFormat.rightMargin()),
            (pageSize.height() - rootFrameFormat.topMargin() - rootFrameFormat.bottomMargin()));
    return ret;
}

void ScreenplayTitlePageObjectInterface::drawObject(QPainter *painter, const QRectF &givenRect,
                                                    QTextDocument *doc, int posInDocument,
                                                    const QTextFormat &format)
{
    Q_UNUSED(posInDocument)

    /**
      The title page now consists of 3 frames.

       In the center of the page, we will have the first frame, with the following
                TITLE        <----- Bold
               VERSION       <----- Normal

               Written by     <----- Normal
                AUTHORS       <----- Normal

       In the bottom left, we will have the second frame, with the following

        COPYRIGHT OWNER
        ADDRESS
        PHONE
        EMAIL
        URL

       On the bottom right, we will have the third frame with the following
      information Written or Generated using Scrite https://www.scrite.io
       */
    auto evaluateCenteredPaintRect = [=]() {
        const QTextFrameFormat rootFrameFormat = doc->rootFrame()->frameFormat();
        const qreal margin = (rootFrameFormat.rightMargin() + rootFrameFormat.leftMargin()) / 2;
        QRectF ret = givenRect;
        ret.moveLeft(margin);
        return ret;
    };
    const QRectF rectOnPage =
            format.property(TitlePageIsCentered).toBool() ? evaluateCenteredPaintRect() : givenRect;
    const QRectF sceneRect = QRectF(0, 0, rectOnPage.width(), rectOnPage.height());

    const Screenplay *screenplay =
            qobject_cast<Screenplay *>(format.property(ScreenplayProperty).value<QObject *>());
    if (screenplay == nullptr)
        return;

    const Screenplay *masterScreenplay = ScriteDocument::instance()->screenplay();
    const Screenplay *coverPageImageScreenplay = screenplay;
    if (screenplay->property("#useDocumentScreenplayForCoverPagePhoto").toBool() == true)
        coverPageImageScreenplay = masterScreenplay;

    auto fetch = [](const QString &given, const QString &defaultValue = QString()) {
        const QString val = given.trimmed();
        return val.isEmpty() ? defaultValue : val;
    };

    const QString title = fetch(screenplay->title(), QStringLiteral("Untitled Screenplay"));
    const QString subtitle = screenplay->subtitle();
    const QString writtenBy = QStringLiteral("Written By");
    const QString basedOn = screenplay->basedOn();
    const QString version = fetch(screenplay->version());
    const QString authors = fetch(screenplay->author());
    const QString contact = fetch(screenplay->contact());
    const QString address = screenplay->address();
    const QString phoneNumber = screenplay->phoneNumber();
    const QString email = screenplay->email();
    const QString website = screenplay->website();
    const QString logline = screenplay->logline();
    const QFont normalFont = doc->defaultFont();

    auto itemRect = [](const QGraphicsItem *item) {
        return item->mapToScene(item->boundingRect()).boundingRect();
    };

    QGraphicsScene scene;
    scene.setSceneRect(sceneRect);

    // Place the cover photo
    GraphicsImageRectItem *coverPagePhotoItem = nullptr;
    if (!coverPageImageScreenplay->coverPagePhoto().isEmpty()) {
        QImage photo(coverPageImageScreenplay->coverPagePhoto());
        QRectF photoRect = photo.rect();
        QSizeF photoSize = photoRect.size();

        QRectF spaceAvailable = sceneRect;
        spaceAvailable.setBottom(sceneRect.center().y());
        photoSize.scale(spaceAvailable.size(), Qt::KeepAspectRatio);

        switch (coverPageImageScreenplay->coverPagePhotoSize()) {
        case Screenplay::LargeCoverPhoto:
            break;
        case Screenplay::MediumCoverPhoto:
            photoSize /= 2.0;
            break;
        case Screenplay::SmallCoverPhoto:
            photoSize /= 4.0;
            break;
        }

        photoRect = QRectF(0, 0, photoSize.width(), photoSize.height());
        photoRect.moveCenter(spaceAvailable.center());
        photoRect.moveTop(spaceAvailable.top());

        coverPagePhotoItem = new GraphicsImageRectItem;
        scene.addItem(coverPagePhotoItem);
        coverPagePhotoItem->setImage(photo);
        coverPagePhotoItem->setRect(0, 0, photoRect.width(), photoRect.height());
        coverPagePhotoItem->setPos(photoRect.topLeft());
    }

    // Place title, subtitle, based on, written by and version into a card
    // and place them in the middle of the page or under the cover page.
    QGraphicsTextItem *titleCardItem = new QGraphicsTextItem;
    scene.addItem(titleCardItem);
    titleCardItem->setFont(normalFont);
    titleCardItem->setTextWidth(sceneRect.width());
    QString titleHtml;
    {
        QTextStream ts(&titleHtml, QIODevice::WriteOnly);
        ts << "<center>";
        ts << "<font size=\"+2\"><strong>" << title << "</strong></font>";
        if (!subtitle.isEmpty())
            ts << "<br/>" << subtitle;
        if (!authors.isEmpty())
            ts << "<br/><br/>" << writtenBy << "<br/>" << authors;
        if (!basedOn.isEmpty())
            ts << "<br/><br/>" << basedOn;
        if (!version.isEmpty())
            ts << "<br/><br/>" << version;

        const QSettings *settings = Application::instance()->settings();
        const bool includeTimestamp =
                settings->value(QStringLiteral("TitlePage/includeTimestamp"), false).toBool();
        if (includeTimestamp) {
            ts << "<br/><br/><font size=\"-1\">Generated on ";
            ts << QDateTime::currentDateTime().toString(Qt::TextDate);
            ts << "</font>";
        }

        ts << "</center>";
    }
    titleCardItem->setHtml(titleHtml);

    QRectF titleCardRect = titleCardItem->boundingRect();
    if (coverPagePhotoItem) {
        const QRectF cpr = itemRect(coverPagePhotoItem);
        titleCardRect.moveTopLeft(QPointF(0, cpr.bottom() + sceneRect.height() * 0.05));
    } else {
        titleCardRect.moveCenter(sceneRect.center());
    }
    titleCardItem->setPos(titleCardRect.topLeft());

    // Place contact, address, phoneNumber, email, website, marketing in a card
    // and place them on the bottom left corner
    QGraphicsTextItem *contactCardItem = nullptr;
    QStringList contactCardFields({ contact, address, phoneNumber, email, website });
    contactCardFields.removeAll(QString());
    if (!contactCardFields.isEmpty()) {
        QString contactHtml;
        contactCardFields.prepend(QStringLiteral("Contact:"));
        contactHtml = contactCardFields.join(QStringLiteral("<br/>"));
        contactCardItem = new QGraphicsTextItem;
        scene.addItem(contactCardItem);
        contactCardItem->setTextWidth(sceneRect.width());
        contactCardItem->setFont(normalFont);
        contactCardItem->setHtml(contactHtml);

        QRectF contactCardRect = contactCardItem->boundingRect();
        contactCardRect.moveBottomLeft(sceneRect.bottomLeft());
        contactCardItem->setPos(contactCardRect.topLeft());
    }

    // Place logline, on the title page if asked for.
    if (!logline.isEmpty() && doc->property("#includeLoglineInTitlePage").toBool()) {
        const qreal textWidth = sceneRect.width() * 0.9;
        const qreal ymin = titleCardRect.bottom();
        const qreal ymax = contactCardItem ? contactCardItem->pos().y() : sceneRect.bottom();
        const qreal margin = (ymax - ymin) * 0.1;

        QRectF loglineCardRect(0, 0, textWidth, 1);
        loglineCardRect.moveTop(ymin + margin);
        loglineCardRect.setBottom(ymax - margin);
        loglineCardRect.moveCenter(QPointF(sceneRect.center().x(), (ymin + ymax) / 2));

        QGraphicsRectItem *loglineCardContainer = new QGraphicsRectItem;
        scene.addItem(loglineCardContainer);
        loglineCardContainer->setPen(Qt::NoPen);
        loglineCardContainer->setBrush(Qt::NoBrush);
        loglineCardContainer->setOpacity(0.1);
        loglineCardContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
        loglineCardContainer->setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);

        QFont loglineFont = normalFont;
        loglineFont.setPointSize(loglineFont.pointSize() - 2);

        QGraphicsTextItem *loglineCard = new QGraphicsTextItem(loglineCardContainer);
        loglineCard->setFont(loglineFont);
        loglineCard->setTextWidth(textWidth);
        loglineCardContainer->setPos(0, 0);

        const QStringList loglineParas = logline.split(QLatin1String("\n"), Qt::SkipEmptyParts);
        QString loglineHtml;
        const QString openP = QLatin1String("<p>");
        const QString closeP = QLatin1String("</p>");
        for (const QString &loglinePara : qAsConst(loglineParas)) {
            if (loglineHtml.isEmpty())
                loglineHtml =
                        openP + QLatin1String("<strong>Logline:</strong> ") + loglinePara + closeP;
            else
                loglineHtml += openP + loglinePara + closeP;
        }
        loglineCard->setHtml(loglineHtml);

        QTextDocument *loglineDoc = loglineCard->document();
        QTextCursor cursor(loglineDoc);
        while (!cursor.atEnd()) {
            QTextBlockFormat format;
            format.setAlignment(Qt::AlignJustify);
            cursor.mergeBlockFormat(format);
            if (!cursor.movePosition(QTextCursor::NextBlock))
                break;
        }

        loglineCardRect.setHeight(
                qMin(loglineCard->boundingRect().height(), loglineCardRect.height()));
        loglineCardContainer->setRect(
                QRectF(0, 0, loglineCardRect.width(), loglineCardRect.height()));
        loglineCardContainer->setPos(loglineCardRect.topLeft());
    }

    scene.render(painter, rectOnPage, sceneRect, Qt::IgnoreAspectRatio);
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayTextObjectInterface::ScreenplayTextObjectInterface(QObject *parent) : QObject(parent) { }

ScreenplayTextObjectInterface::~ScreenplayTextObjectInterface() { }

QSizeF ScreenplayTextObjectInterface::intrinsicSize(QTextDocument *doc, int posInDocument,
                                                    const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const QFont font(format.property(QTextFormat::FontFamily).toString(),
                     format.property(QTextFormat::FontPointSize).toInt());
    const QFontMetricsF fontMetrics(font);
    return QSizeF(0, fontMetrics.lineSpacing() - fontMetrics.descent());
}

void ScreenplayTextObjectInterface::drawObject(QPainter *painter, const QRectF &givenRect,
                                               QTextDocument *doc, int posInDocument,
                                               const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const QFont font(format.property(QTextFormat::FontFamily).toString(),
                     format.property(QTextFormat::FontPointSize).toInt());
    painter->setFont(font);

    int type = format.property(TypeProperty).toInt();
    switch (type) {
    case SceneNumberType:
        this->drawSceneNumber(painter, givenRect, doc, posInDocument, format);
        break;
    case ContdMarkerType:
    case MoreMarkerType:
        this->drawMoreMarker(painter, givenRect, doc, posInDocument, format);
        break;
    case SceneIconType:
        this->drawSceneIcon(painter, givenRect, doc, posInDocument, format);
        break;
    }
}

void ScreenplayTextObjectInterface::drawSceneNumber(QPainter *painter, const QRectF &givenRect,
                                                    QTextDocument *doc, int posInDocument,
                                                    const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const QVariantList data = format.property(DataProperty).toList();
    const QString sceneNumber = data.at(0).toString();
    if (sceneNumber.isEmpty())
        return;

    const QTextFrameFormat rootFrameFormat = doc->rootFrame()->frameFormat();

    QRectF rect = givenRect;
    rect.setLeft(rootFrameFormat.leftMargin() * 0.55);

    const QString sceneNumberText = sceneNumber + QStringLiteral(".");
    this->drawText(painter, rect, sceneNumberText);
}

void ScreenplayTextObjectInterface::drawMoreMarker(QPainter *painter, const QRectF &givenRect,
                                                   QTextDocument *doc, int posInDocument,
                                                   const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const QString text = format.property(DataProperty).toString();
    if (text.isEmpty())
        return;

    QFontMetricsF fm(painter->font());
    QRectF rect = fm.boundingRect(text);
    rect.moveLeft(givenRect.right());
    rect.moveBottom(givenRect.bottom());

    const QPen oldPen = painter->pen();

    QColor textColor = format.foreground().color();
    textColor.setAlphaF(textColor.alphaF() * 0.75);
    painter->setPen(textColor);
    this->drawText(painter, rect, text);

    painter->setPen(oldPen);
}

void ScreenplayTextObjectInterface::drawSceneIcon(QPainter *painter, const QRectF &givenRect,
                                                  QTextDocument *doc, int posInDocument,
                                                  const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const int sceneType = format.property(DataProperty).toInt();
    if (sceneType == Scene::Standard)
        return;

    static const QJsonArray sceneTypeModel = Application::instance()->enumerationModelForType(
            QStringLiteral("Scene"), QStringLiteral("Type"));
    if (sceneType < 0 || sceneType >= sceneTypeModel.size())
        return;

    const QJsonObject sceneTypeInfo = sceneTypeModel.at(sceneType).toObject();

    const qreal iconSize = givenRect.height();
    QString iconFile = sceneTypeInfo.value(QStringLiteral("icon")).toString();
    if (iconFile.isEmpty())
        return;

    if (iconFile.startsWith(QStringLiteral("qrc:/")))
        iconFile.remove(0, 3);

    const QImage icon(iconFile);
    if (icon.isNull())
        return;

    const QTextFrameFormat rootFrameFormat = doc->rootFrame()->frameFormat();

    QRectF rect = givenRect;
    rect.setLeft(rootFrameFormat.leftMargin() * 0.45);
    rect.moveBottom(rect.bottom() + iconSize * 0.15);
    rect = QRectF(rect.left() - iconSize, rect.bottom() - iconSize, iconSize, iconSize);

    const bool flag = painter->renderHints().testFlag(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(rect, icon);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, flag);

    const QString iconKey = sceneTypeInfo.value(QStringLiteral("key")).toString();
    QRectF iconKeyRect = rect;
    iconKeyRect.moveTop(rect.bottom() + rect.height() * 0.5);
    painter->save();
    painter->setFont(QFont(painter->font().family(), painter->font().pointSize() - 4));
    this->drawText(painter, iconKeyRect, QStringLiteral("(") + iconKey + QStringLiteral(")"));
    painter->restore();
}

void ScreenplayTextObjectInterface::drawText(QPainter *painter, const QRectF &rect,
                                             const QString &text)
{
    const bool isPdfDevice = painter->device()->paintEngine()->type() == QPaintEngine::Pdf;

    if (isPdfDevice) {
        const qreal invScaleX = qreal(qt_defaultDpi()) / painter->device()->logicalDpiX();
        const qreal invScaleY = qreal(qt_defaultDpi()) / painter->device()->logicalDpiY();

        painter->save();
        painter->translate(rect.left(), rect.bottom());
        painter->scale(invScaleX, invScaleY);
        painter->drawText(0, 0, text);
        painter->restore();
    } else
        painter->drawText(rect.bottomLeft(), text);
}

///////////////////////////////////////////////////////////////////////////////

Q_GLOBAL_STATIC(QList<SceneElementBlockTextUpdater *>, SceneElementBlockTextUpdaterList)

void SceneElementBlockTextUpdater::completeOthers(SceneElementBlockTextUpdater *than)
{
    for (SceneElementBlockTextUpdater *updater : qAsConst(*SceneElementBlockTextUpdaterList)) {
        if (updater != than)
            updater->update();
    }
}

SceneElementBlockTextUpdater::SceneElementBlockTextUpdater(ScreenplayTextDocument *document,
                                                           SceneElement *para)
    : QObject(para), m_sceneElement(para), m_document(document)
{
    SceneElementBlockTextUpdaterList->append(this);
    if (!m_document.isNull())
        connect(m_document, &ScreenplayTextDocument::destroyed, this,
                &SceneElementBlockTextUpdater::abort);
}

SceneElementBlockTextUpdater::~SceneElementBlockTextUpdater()
{
    m_timer.stop();
    SceneElementBlockTextUpdaterList->removeOne(this);
}

void SceneElementBlockTextUpdater::schedule()
{
    if (m_sceneElement.isNull())
        return;

    m_timer.stop();
    m_timer.start(500, this);

    completeOthers(this);
}

void SceneElementBlockTextUpdater::abort()
{
    m_timer.stop();
    this->deleteLater();
}

void SceneElementBlockTextUpdater::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_timer.timerId()) {
        QObject::timerEvent(event);
        return;
    }

    this->update();
}

void SceneElementBlockTextUpdater::update()
{
    this->deleteLater();
    if (!m_timer.isActive())
        return;

    m_timer.stop();
    if (m_sceneElement.isNull() || m_document.isNull())
        return;

    Scene *scene = m_sceneElement->scene();
    if (scene == nullptr)
        return;

    Screenplay *screenplay = m_document->screenplay();
    if (screenplay == nullptr)
        return;

    ScreenplayTextDocumentUpdate update(m_document);

    const int paraIndex = scene->indexOfElement(m_sceneElement);
    if (paraIndex < 0)
        return; // This can happen when the paragraph is not part of the scene
                // text, but it exists as a way to capture a mute-character in the
                // scene.

    QList<ScreenplayElement *> elements = screenplay->sceneElements(scene);
    for (ScreenplayElement *element : qAsConst(elements)) {
        QTextFrame *frame = m_document->findTextFrame(element);
        if (frame == nullptr)
            continue;

        const int nrBlocks = paraIndex + (scene->heading()->isEnabled() ? 1 : 0);

        QTextCursor cursor = frame->firstCursorPosition();
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, nrBlocks);

        QTextBlock block = cursor.block();
        m_document->formatBlock(block, m_sceneElement->text());
    }
}
