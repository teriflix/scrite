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

#include "hourglass.h"
#include "application.h"
#include "imageprinter.h"
#include "timeprofiler.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "screenplaytextdocument.h"

#include <QDate>
#include <QtMath>
#include <QtDebug>
#include <QPainter>
#include <QDateTime>
#include <QQmlEngine>
#include <QTextBlock>
#include <QPdfWriter>
#include <QTextTable>
#include <QTextCursor>
#include <QPaintEngine>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextBlockUserData>
#include <QScopedValueRollback>
#include <QAbstractTextDocumentLayout>

class ScreenplayParagraphBlockData : public QTextBlockUserData
{
public:
    ScreenplayParagraphBlockData(const SceneElement *element);
    ~ScreenplayParagraphBlockData();

    bool contains(const SceneElement *other) const;
    SceneElement::Type elementType() const;
    QString elementText() const;
    const SceneElement *element() const { return m_element; }
    bool isFirstElementInScene() const;

    const SceneElement *getCharacterElement() const;
    QString getCharacterElementText() const;

    static ScreenplayParagraphBlockData *get(const QTextBlock &block);
    static ScreenplayParagraphBlockData *get(QTextBlockUserData *userData);

private:
    const SceneElement *m_element = nullptr;
};

ScreenplayParagraphBlockData::ScreenplayParagraphBlockData(const SceneElement *element)
    : m_element(element) { }

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
    if(m_element)
        return m_element->scene()->elementAt(0) == m_element;
    return false;
}

const SceneElement *ScreenplayParagraphBlockData::getCharacterElement() const
{
    if(m_element && (m_element->type() == SceneElement::Dialogue || m_element->type() == SceneElement::Parenthetical))
    {
        Scene *scene = m_element->scene();
        SceneElement *element = const_cast<SceneElement*>(m_element);
        int index = m_element->scene()->indexOfElement(element) - 1;
        while(index >= 0)
        {
            element = scene->elementAt(index);
            if(element->type() == SceneElement::Character)
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
    if(userData == nullptr)
        return nullptr;

    ScreenplayParagraphBlockData *userData2 = reinterpret_cast<ScreenplayParagraphBlockData*>(userData);
    return userData2;
}

///////////////////////////////////////////////////////////////////////////////

class ScreenplayTextDocumentUpdate
{
public:
    ScreenplayTextDocumentUpdate(ScreenplayTextDocument *document)
        : m_document(document) {
        if(m_document)
            m_document->setUpdating(true);
    }
    ~ScreenplayTextDocumentUpdate() {
        if(m_document)
            m_document->setUpdating(false);
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
    if(m_textDocument != nullptr && m_textDocument->parent() == this)
        m_textDocument->setUndoRedoEnabled(true);
}

void ScreenplayTextDocument::setTextDocument(QTextDocument *val)
{
    if(m_textDocument != nullptr && m_textDocument == val)
        return;

    if(m_textDocument != nullptr && m_textDocument->parent() == this)
        delete m_textDocument;

    m_textDocument = val ? val : new QTextDocument(this);
    m_textDocument->setUndoRedoEnabled(false);
    this->loadScreenplayLater();

    emit textDocumentChanged();
}

void ScreenplayTextDocument::setScreenplay(Screenplay *val)
{
    if(m_screenplay == val)
        return;

    this->disconnectFromScreenplaySignals();

    if(m_screenplay)
        disconnect(m_screenplay, &Screenplay::aboutToDelete, this, &ScreenplayTextDocument::resetScreenplay);

    m_screenplay = val;

    if(m_screenplay)
        connect(m_screenplay, &Screenplay::aboutToDelete, this, &ScreenplayTextDocument::resetScreenplay);

    this->loadScreenplayLater();

    emit screenplayChanged();
}

void ScreenplayTextDocument::setFormatting(ScreenplayFormat *val)
{
    if(m_formatting == val)
        return;

    this->disconnectFromScreenplayFormatSignals();

    if(m_formatting && m_formatting->parent() == this)
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
    if(m_titlePage == val)
        return;

    m_titlePage = val;
    emit titlePageChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSceneNumbers(bool val)
{
    if(m_sceneNumbers == val)
        return;

    m_sceneNumbers = val;
    emit sceneNumbersChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSceneIcons(bool val)
{
    if(m_sceneIcons == val)
        return;

    m_sceneIcons = val;
    emit sceneIconsChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setSyncEnabled(bool val)
{
    if(m_syncEnabled == val)
        return;

    m_syncEnabled = val;
    m_loadScreenplayTimer.stop();

    if(m_syncEnabled)
    {
        this->connectToScreenplaySignals();
        this->connectToScreenplayFormatSignals();
        this->syncNow();
    }
    else
    {
        this->disconnectFromScreenplaySignals();
        this->disconnectFromScreenplayFormatSignals();
    }

    emit syncEnabledChanged();
}

void ScreenplayTextDocument::setListSceneCharacters(bool val)
{
    if(m_listSceneCharacters == val)
        return;

    m_listSceneCharacters = val;
    emit listSceneCharactersChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setHighlightDialoguesOf(QStringList val)
{
    if(m_highlightDialoguesOf == val)
        return;

    m_highlightDialoguesOf = val;
    emit highlightDialoguesOfChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setIncludeSceneSynopsis(bool val)
{
    if(m_includeSceneSynopsis == val)
        return;

    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::setPurpose(ScreenplayTextDocument::Purpose val)
{
    if(m_purpose == val)
        return;

    m_purpose = val;
    emit purposeChanged();
}

void ScreenplayTextDocument::setPrintEachSceneOnANewPage(bool val)
{
    if(m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::print(QObject *printerObject)
{
    HourGlass hourGlass;

    if(m_textDocument == nullptr || m_screenplay == nullptr || m_formatting == nullptr)
        return;

    if(m_loadScreenplayTimer.isActive() || m_textDocument->isEmpty())
        this->syncNow();

    QPagedPaintDevice *printer = nullptr;

    QPdfWriter *pdfWriter = qobject_cast<QPdfWriter*>(printerObject);
    if(pdfWriter)
    {
        printer = pdfWriter;

        pdfWriter->setTitle(m_screenplay->title());
        pdfWriter->setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
    }

    ImagePrinter *imagePrinter = qobject_cast<ImagePrinter*>(printerObject);
    if(imagePrinter)
        printer = imagePrinter;

    if(printer)
    {
        m_formatting->pageLayout()->configure(printer);
        printer->setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);
    }

    QTextDocumentPagedPrinter docPrinter;
    docPrinter.header()->setVisibleFromPageOne(!m_titlePage);
    docPrinter.footer()->setVisibleFromPageOne(!m_titlePage);
    docPrinter.watermark()->setVisibleFromPageOne(!m_titlePage);
    docPrinter.print(m_textDocument, printer);
}

QList< QPair<int,int> > ScreenplayTextDocument::pageBreaksFor(ScreenplayElement *element) const
{
    QList< QPair<int,int> > ret;
    if(element == nullptr)
        return ret;

    QTextFrame *frame = this->findTextFrame(element);
    if(frame == nullptr)
        return ret;

    // We need to know three positions within each scene frame.
    // 1. Start of the frame
    // 2. Start of the first paragraph in the scene (after the scene heading)
    // 3. End of the scene frame
    QTextCursor cursor = frame->firstCursorPosition();
    QTextBlock block = cursor.block();
    ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
    if(blockData && blockData->elementType() == SceneElement::Heading)
        block = block.next();

    // This is the range of cursor positions inside the frame
    int sceneHeadingStart = frame->firstPosition();
    int paragraphStart = block.position();
    int paragraphEnd = frame->lastPosition();

    // This method includes 'pageBorderPosition' and 'pageNumber' in the returned list
    // If pageBorderPosition lies within the frame, then it is included in the list.
    auto checkAndAdd = [sceneHeadingStart,paragraphStart,paragraphEnd,&ret](int pageBorderPosition, int pageNumber) {
        if(pageBorderPosition >= sceneHeadingStart && pageBorderPosition <= paragraphEnd) {
            const int offset = qMax(pageBorderPosition - paragraphStart, -1);
            if(ret.isEmpty() || ret.last().first != offset)
                ret << qMakePair(offset, pageNumber);
        }
    };

    // Special case for page #1
    if(element == m_screenplay->elementAt(0))
        checkAndAdd(sceneHeadingStart, 1);

    // Now loop through all pages and gather all pages that lie within the scene boundaries
    for(int i=0; i<m_pageBoundaries.count(); i++)
    {
        const QPair<int,int> pgBoundary = m_pageBoundaries.at(i);
        if(pgBoundary.first > paragraphEnd)
            break;

        checkAndAdd(pgBoundary.first, i+1);
    }

    return ret;
}

qreal ScreenplayTextDocument::lengthInPixels(ScreenplayElement *element) const
{
    QTextFrame *frame = this->findTextFrame(element);
    if(frame == nullptr)
        return 0;

    QAbstractTextDocumentLayout *layout = m_textDocument->documentLayout();
    return layout->frameBoundingRect(frame).height();
}

qreal ScreenplayTextDocument::lengthInPages(ScreenplayElement *element) const
{
    const qreal pxLength = this->lengthInPixels(element);
    if( qFuzzyIsNull(pxLength) )
        return 0;

    const QTextFrameFormat rootFrameFormat = m_textDocument->rootFrame()->frameFormat();
    const qreal topMargin = rootFrameFormat.topMargin();
    const qreal bottomMargin = rootFrameFormat.bottomMargin();
    const qreal pageLength = m_textDocument->pageSize().height() - topMargin - bottomMargin;
    if( qFuzzyIsNull(pageLength) )
        return 0;

    return pxLength / pageLength;
}

void ScreenplayTextDocument::setInjection(QObject *val)
{
    if(m_injection == val)
        return;

    if(m_injection != val)
        disconnect(m_injection, &QObject::destroyed, this, &ScreenplayTextDocument::resetInjection);

    m_injection = val;

    if(m_injection != val)
        connect(m_injection, &QObject::destroyed, this, &ScreenplayTextDocument::resetInjection);

    emit injectionChanged();

    this->loadScreenplayLater();
}

void ScreenplayTextDocument::syncNow()
{
    this->loadScreenplay();
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
    if(event->timerId() == m_loadScreenplayTimer.timerId())
    {
        this->syncNow();
        this->connectToScreenplaySignals();
        this->connectToScreenplayFormatSignals();
    }
    else if(event->timerId() == m_pageBoundaryEvalTimer.timerId())
    {
        m_pageBoundaryEvalTimer.stop();
        this->evaluatePageBoundaries();
    }
    else if(event->timerId() == m_sceneResetTimer.timerId())
    {
        m_sceneResetTimer.stop();
        this->processSceneResetList();
    }
}

void ScreenplayTextDocument::init()
{
    if(m_textDocument == nullptr)
        m_textDocument = new QTextDocument(this);
}

void ScreenplayTextDocument::setUpdating(bool val)
{
    if(m_updating == val)
        return;

    m_updating = val;
    emit updatingChanged();

    if(val)
        emit updateStarted();
    else
    {
        this->evaluatePageBoundariesLater();
        emit updateFinished();
    }
}

void ScreenplayTextDocument::setPageCount(int val)
{
    if(m_pageCount == val)
        return;

    m_pageCount = val;
    emit pageCountChanged();
}

void ScreenplayTextDocument::setCurrentPage(int val)
{
    val = m_pageCount > 0 ? qBound(1, val, m_pageCount) : 0;
    if(m_currentPage == val)
        return;

    m_currentPage = val;
    emit currentPageChanged();
}

inline void polishFontsAndInsertTextAtCursor(QTextCursor &cursor, const QString &text)
{
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

void ScreenplayTextDocument::loadScreenplay()
{
    HourGlass hourGlass;

    if(m_updating || !m_componentComplete) // so that we avoid recursive updates
        return;

    if(!m_screenplayModificationTracker.isModified(m_screenplay) && !m_formattingModificationTracker.isModified(m_formatting))
        return;

    ScreenplayTextDocumentUpdate update(this);

    // Here we discard anything we have previously loaded and load the entire
    // document fresh from the start.
    this->clearTextFrames();
    m_sceneResetList.clear();
    m_textDocument->clear();
    m_sceneResetTimer.stop();
    m_pageBoundaryEvalTimer.stop();

    if(m_screenplay == nullptr)
        return;

    if(m_screenplay->elementCount() == 0)
        return;

    if(m_formatting == nullptr)
        this->setFormatting(new ScreenplayFormat(this));

    m_textDocument->setDefaultFont(m_formatting->defaultFont());
    m_formatting->pageLayout()->configure(m_textDocument);
    m_textDocument->setIndentWidth(10);

    if(m_sceneNumbers || (m_purpose == ForPrinting && m_syncEnabled) || m_sceneIcons)
    {
        ScreenplayTextObjectInterface *toi = m_textDocument->findChild<ScreenplayTextObjectInterface*>();
        if(toi == nullptr)
        {
            toi = new ScreenplayTextObjectInterface(m_textDocument);
            m_textDocument->documentLayout()->registerHandler(ScreenplayTextObjectInterface::Kind, toi);
        }
    }

    const QTextFrameFormat rootFrameFormat = m_textDocument->rootFrame()->frameFormat();

    // So that QTextDocumentPrinter can pick up this for header and footer fields.
    m_textDocument->setProperty("#title", m_screenplay->title());
    m_textDocument->setProperty("#subtitle", m_screenplay->subtitle());
    m_textDocument->setProperty("#author", m_screenplay->author());
    m_textDocument->setProperty("#contact", m_screenplay->contact());
    m_textDocument->setProperty("#version", m_screenplay->version());
    m_textDocument->setProperty("#phone", m_screenplay->phoneNumber());
    m_textDocument->setProperty("#email", m_screenplay->email());
    m_textDocument->setProperty("#website", m_screenplay->website());

    QTextBlockFormat frameBoundaryBlockFormat;
    frameBoundaryBlockFormat.setLineHeight(0, QTextBlockFormat::FixedHeight);

    QTextCursor cursor(m_textDocument);

    // Title Page
    if(m_titlePage)
    {
        ScreenplayTitlePageObjectInterface *tpoi = m_textDocument->findChild<ScreenplayTitlePageObjectInterface*>();
        if(tpoi == nullptr)
        {
            tpoi = new ScreenplayTitlePageObjectInterface(m_textDocument);
            m_textDocument->documentLayout()->registerHandler(ScreenplayTitlePageObjectInterface::Kind, tpoi);
        }

        QTextBlockFormat pageBreakFormat;
        pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
        cursor.setBlockFormat(pageBreakFormat);

        QTextCharFormat titlePageFormat;
        titlePageFormat.setObjectType(ScreenplayTitlePageObjectInterface::Kind);
        titlePageFormat.setProperty(ScreenplayTitlePageObjectInterface::ScreenplayProperty, QVariant::fromValue<QObject*>(m_screenplay));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), titlePageFormat);
    }

    AbstractScreenplayTextDocumentInjectionInterface *injection = qobject_cast<AbstractScreenplayTextDocumentInjectionInterface*>(m_injection);
    if(injection != nullptr)
        injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::AfterTitlePage);

    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        const ScreenplayElement *element = m_screenplay->elementAt(i);
        if(element->elementType() != ScreenplayElement::SceneElementType)
            continue;

        QTextFrameFormat frameFormat = m_sceneFrameFormat;
        const Scene *scene = element->scene();
        if(scene != nullptr && i > 0)
        {
            SceneElement::Type firstParaType = SceneElement::Heading;
            if(!scene->heading()->isEnabled() && scene->elementCount())
            {
                SceneElement *firstPara = scene->elementAt(0);
                firstParaType = firstPara->type();
            }

            const SceneElementFormat *firstParaFormat = m_formatting->elementFormat(firstParaType);
            const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
            const QTextBlockFormat blockFormat = firstParaFormat->createBlockFormat(&pageWidth);
            frameFormat.setTopMargin(blockFormat.topMargin());
        }

        if(i > 0 && m_printEachSceneOnANewPage)
            frameFormat.setPageBreakPolicy(QTextFrameFormat::PageBreak_AlwaysBefore);

        // Each screenplay element (or scene) has its own frame. That makes
        // moving them in one bunch easy.
        QTextFrame *frame = cursor.insertFrame(frameFormat);
        this->registerTextFrame(element, frame);
        this->loadScreenplayElement(element, cursor);

        // We have to move the cursor out of the frame we created for the scene
        // https://doc.qt.io/qt-5/richtext-cursor.html#frames
        cursor = m_textDocument->rootFrame()->lastCursorPosition();
        cursor.setBlockFormat(frameBoundaryBlockFormat);
    }

    if(injection != nullptr)
        injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::AfterLastScene);

    this->includeMoreAndContdMarkers();
    this->evaluatePageBoundariesLater();
}

void ScreenplayTextDocument::includeMoreAndContdMarkers()
{
    if(m_purpose != ForPrinting || m_syncEnabled)
        return;

    /**
      When we print a screenplay, we expect it to do the following

      1. Slug line or Scene Heading cannot come on the last line of the page
      2. Character name cannot be on the last line of the page
      3. If only one line of the dialogue can be squeezed into the last line of the page, then
         we must move it to the next page along with the charactername.
      4. If a dialogue spans across page break, then we must insert MORE and CONT'D markers, with character name.
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
        cursor.setPosition(block.position()+block.length()-1, QTextCursor::KeepAnchor);
        cursor.mergeBlockFormat(blockFormat);
        cursor.clearSelection();
    };

    const ScreenplayFormat *format = m_formatting;
    const SceneElementFormat *characterFormat = format->elementFormat(SceneElement::Character);
    const SceneElementFormat *dialogueFormat = format->elementFormat(SceneElement::Dialogue);
    const QFontMetricsF dialogFontMetrics(dialogueFormat->font());
    const int nrCharsPerDialogLine = int(qCeil((pageLayout->contentWidth()-pageLayout->leftMargin()-pageLayout->rightMargin()-dialogueFormat->leftMargin()-dialogueFormat->rightMargin())/dialogFontMetrics.averageCharWidth()));

    auto insertMarkers = [=](const QTextBlock &block) {
        ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
        if(blockData == nullptr)
            return;

        const QString characterName = blockData ? blockData->getCharacterElementText() : QString();
        if(characterName.isEmpty())
            return;

        SceneElementFormat *elementFormat = format->elementFormat(blockData->elementType());
        const QFont font = elementFormat->font();

        QTextCursor cursor(block);
        cursor.setPosition(block.position()+block.length()-1, QTextCursor::KeepAnchor);

        const QTextBlockFormat restoreBlockFormat = cursor.blockFormat();
        const QTextCharFormat restoreCharFormat = cursor.charFormat();

        QTextBlockFormat pageBreakFormat;
        pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
        cursor.mergeBlockFormat(pageBreakFormat);
        cursor.clearSelection();

        QTextCharFormat moreMarkerFormat;
        moreMarkerFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
        moreMarkerFormat.setFont(font);
        moreMarkerFormat.setForeground(elementFormat->textColor());
        moreMarkerFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty, ScreenplayTextObjectInterface::MoreMarkerType);
        moreMarkerFormat.setProperty(ScreenplayTextObjectInterface::DataProperty, QStringLiteral("  (MORE)"));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), moreMarkerFormat);

        QTextBlockFormat characterBlockFormat = characterFormat->createBlockFormat();
        QTextCharFormat characterCharFormat = characterFormat->createCharFormat();
        characterBlockFormat.setTopMargin(0);
        cursor.insertBlock(characterBlockFormat, characterCharFormat);
        // cursor.insertText(characterName);
        if(m_purpose == ForDisplay)
            cursor.insertText(characterName);
        else
            polishFontsAndInsertTextAtCursor(cursor, characterName);

        QTextCharFormat contdMarkerFormat;
        contdMarkerFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
        contdMarkerFormat.setFont(characterFormat->font());
        contdMarkerFormat.setForeground(characterFormat->textColor());
        contdMarkerFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty, ScreenplayTextObjectInterface::ContdMarkerType);
        contdMarkerFormat.setProperty(ScreenplayTextObjectInterface::DataProperty, QStringLiteral(" (CONT'D)"));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), contdMarkerFormat);
    };

    while(pageIndex < nrPages)
    {
        if(pageIndex == 38)
            qDebug("Check this.");

        paperRect = QRectF(0, pageIndex*paperRect.height(), paperRect.width(), paperRect.height());
        const QRectF contentsRect = paperRect.adjusted(pageMargins.left(), pageMargins.top(), -pageMargins.right(), -pageMargins.bottom());
        const int lastPosition = pageIndex == nrPages-1 ? endCursor.position() : layout->hitTest(contentsRect.bottomRight(), Qt::FuzzyHit);

        QTextCursor cursor(m_textDocument);
        cursor.setPosition(lastPosition-1);

        QTextBlock block = cursor.block();
        ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
        if(blockData == nullptr)
        {
            block = block.previous();
            blockData = ScreenplayParagraphBlockData::get(block);
        }

        if(blockData)
        {
            switch(blockData->elementType())
            {
            case SceneElement::Character: {
                const SceneElement *element = blockData->element();
                if(element->scene()->elementAt(0) == element)
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
                ScreenplayParagraphBlockData *previousBlockData = ScreenplayParagraphBlockData::get(previousBlock);
                if(previousBlockData) {
                    if(previousBlockData->elementType() == SceneElement::Character) {
                        previousBlock = previousBlock.previous();
                        insertPageBreakAfter(previousBlock);
                    } else if(previousBlockData->elementType() == SceneElement::Dialogue) {
                        insertMarkers(previousBlock);
                    }
                }
                } break;
            case SceneElement::Dialogue:
                if(block.position()+block.length()-1 > lastPosition) {
                    const QString blockText = block.text();
                    const SceneElement *dialogElement = blockData->element();

                    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor, 1);

                    QString blockTextPart1 = cursor.selectedText();
                    blockTextPart1.chop(nrCharsPerDialogLine);
                    while(blockTextPart1.length() && !blockTextPart1.at(blockTextPart1.length()-1).isSpace())
                        blockTextPart1.chop(1);
                    blockTextPart1.chop(1);

                    // if(blockTextPart1.length() < nrCharsPerDialogLine) {
                    if(blockTextPart1.isEmpty()) {
                        QTextBlock previousBlock = block.previous();
                        ScreenplayParagraphBlockData *previousBlockData = ScreenplayParagraphBlockData::get(previousBlock);
                        if(previousBlockData->elementType() == SceneElement::Character) {
                            if(previousBlockData->isFirstElementInScene())
                                previousBlock = previousBlock.previous();
                            insertPageBreakAfter(previousBlock.previous());
                            break;
                        }
                    }

                    QString blockTextPart2 = blockTextPart1.isEmpty() ? blockText :
                                             blockText.mid(blockTextPart1.length()+1);
                                             // Why +1? Because we want to skip the space
                                             // between blockTextPart1 & blockTextPart2.

                    cursor.clearSelection();
                    cursor.select(QTextCursor::BlockUnderCursor);
                    cursor.removeSelectedText();

                    QTextBlockFormat dialogBlockFormat = dialogueFormat->createBlockFormat();
                    QTextCharFormat dialogCharFormat = dialogueFormat->createCharFormat();
                    cursor.insertBlock(dialogBlockFormat, dialogCharFormat);
                    if(m_purpose == ForDisplay)
                        cursor.insertText(blockTextPart1);
                    else
                        polishFontsAndInsertTextAtCursor(cursor, blockTextPart1);
                    block = cursor.block();
                    block.setUserData(new ScreenplayParagraphBlockData(dialogElement));

                    cursor.insertBlock(dialogBlockFormat, dialogCharFormat);
                    if(m_purpose == ForDisplay)
                        cursor.insertText(blockTextPart2);
                    else
                        polishFontsAndInsertTextAtCursor(cursor, blockTextPart2);
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
    if(m_textDocument != nullptr)
        m_textDocument->clear();

    if(m_syncEnabled)
    {
        this->disconnectFromScreenplaySignals();
        this->disconnectFromScreenplayFormatSignals();

        const bool updateWasScheduled = m_loadScreenplayTimer.isActive();
        m_loadScreenplayTimer.start(100, this);
        if(!updateWasScheduled)
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
    if(m_screenplay == nullptr || !m_syncEnabled || m_connectedToScreenplaySignals)
        return;

    connect(m_screenplay, &Screenplay::elementMoved, this, &ScreenplayTextDocument::onSceneMoved);
    connect(m_screenplay, &Screenplay::modelReset, this, &ScreenplayTextDocument::onScreenplayReset);
    connect(m_screenplay, &Screenplay::elementRemoved, this, &ScreenplayTextDocument::onSceneRemoved);
    connect(m_screenplay, &Screenplay::elementInserted, this, &ScreenplayTextDocument::onSceneInserted);
    connect(m_screenplay, &Screenplay::activeSceneChanged, this, &ScreenplayTextDocument::onActiveSceneChanged);
    connect(m_screenplay, &Screenplay::modelAboutToBeReset, this, &ScreenplayTextDocument::onScreenplayAboutToReset);

    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        Scene *scene = element->scene();
        if(scene == nullptr)
            continue;

        this->connectToSceneSignals(scene);
    }

    this->onActiveSceneChanged();

    m_connectedToScreenplaySignals = true;
}

void ScreenplayTextDocument::connectToScreenplayFormatSignals()
{
    if(m_formatting == nullptr || !m_syncEnabled || m_connectedToFormattingSignals)
        return;

    connect(m_formatting, &ScreenplayFormat::defaultFontChanged, this, &ScreenplayTextDocument::onDefaultFontChanged);
    connect(m_formatting, &ScreenplayFormat::screenChanged, this, &ScreenplayTextDocument::onFormatScreenChanged);
    connect(m_formatting, &ScreenplayFormat::fontPointSizeDeltaChanged, this, &ScreenplayTextDocument::onFormatFontPointSizeDeltaChanged);

    for(int i=SceneElement::Min; i<=SceneElement::Max; i++)
    {
        SceneElementFormat *elementFormat = m_formatting->elementFormat(i);
        connect(elementFormat, &SceneElementFormat::elementFormatChanged, this, &ScreenplayTextDocument::onElementFormatChanged);
    }

    m_connectedToFormattingSignals = true;
}

void ScreenplayTextDocument::disconnectFromScreenplaySignals()
{
    if(m_screenplay == nullptr || !m_connectedToScreenplaySignals)
        return;

    disconnect(m_screenplay, &Screenplay::elementMoved, this, &ScreenplayTextDocument::onSceneMoved);
    disconnect(m_screenplay, &Screenplay::modelReset, this, &ScreenplayTextDocument::onScreenplayReset);
    disconnect(m_screenplay, &Screenplay::elementRemoved, this, &ScreenplayTextDocument::onSceneRemoved);
    disconnect(m_screenplay, &Screenplay::elementInserted, this, &ScreenplayTextDocument::onSceneInserted);
    disconnect(m_screenplay, &Screenplay::activeSceneChanged, this, &ScreenplayTextDocument::onActiveSceneChanged);
    disconnect(m_screenplay, &Screenplay::modelAboutToBeReset, this, &ScreenplayTextDocument::onScreenplayAboutToReset);

    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        Scene *scene = element->scene();
        if(scene == nullptr)
            continue;

        this->disconnectFromSceneSignals(scene);
    }

    m_connectedToScreenplaySignals = false;
}

void ScreenplayTextDocument::disconnectFromScreenplayFormatSignals()
{
    if(m_formatting == nullptr || !m_connectedToFormattingSignals)
        return;

    disconnect(m_formatting, &ScreenplayFormat::defaultFontChanged, this, &ScreenplayTextDocument::onDefaultFontChanged);
    disconnect(m_formatting, &ScreenplayFormat::screenChanged, this, &ScreenplayTextDocument::onFormatScreenChanged);
    disconnect(m_formatting, &ScreenplayFormat::fontPointSizeDeltaChanged, this, &ScreenplayTextDocument::onFormatFontPointSizeDeltaChanged);

    for(int i=SceneElement::Min; i<=SceneElement::Max; i++)
    {
        SceneElementFormat *elementFormat = m_formatting->elementFormat(i);
        disconnect(elementFormat, &SceneElementFormat::elementFormatChanged, this, &ScreenplayTextDocument::onElementFormatChanged);
    }

    m_connectedToFormattingSignals = false;
}

void ScreenplayTextDocument::connectToSceneSignals(Scene *scene)
{
    if(scene == nullptr)
        return;

    connect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset, Qt::UniqueConnection);
    connect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel, Qt::UniqueConnection);
    connect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed, Qt::UniqueConnection);
    connect(scene, &Scene::sceneAboutToReset, this, &ScreenplayTextDocument::onSceneAboutToReset, Qt::UniqueConnection);
    connect(scene, &Scene::sceneElementChanged, this, &ScreenplayTextDocument::onSceneElementChanged, Qt::UniqueConnection);
    connect(scene, &Scene::modelAboutToBeReset, this, &ScreenplayTextDocument::onSceneAboutToResetModel, Qt::UniqueConnection);

    SceneHeading *heading = scene->heading();
    connect(heading, &SceneHeading::textChanged, this, &ScreenplayTextDocument::onSceneHeadingChanged, Qt::UniqueConnection);
}

void ScreenplayTextDocument::disconnectFromSceneSignals(Scene *scene)
{
    if(scene == nullptr)
        return;

    disconnect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
    disconnect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel);
    disconnect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed);
    disconnect(scene, &Scene::sceneAboutToReset, this, &ScreenplayTextDocument::onSceneAboutToReset);
    disconnect(scene, &Scene::sceneElementChanged, this, &ScreenplayTextDocument::onSceneElementChanged);
    disconnect(scene, &Scene::modelAboutToBeReset, this, &ScreenplayTextDocument::onSceneAboutToResetModel);

    SceneHeading *heading = scene->heading();
    disconnect(heading, &SceneHeading::textChanged, this, &ScreenplayTextDocument::onSceneHeadingChanged);
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
    if(m_screenplayIsBeingReset)
        return;

    Q_UNUSED(index)
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating while new scene was removed.");

    Scene *scene = element->scene();
    if(scene == nullptr)
        return;

    ScreenplayTextDocumentUpdate update(this);

    QTextFrame *frame = this->findTextFrame(element);
    Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument", "Attempting to remove a scene before it was included in the text document.");

    QTextCursor cursor = frame->firstCursorPosition();
    cursor.movePosition(QTextCursor::Up);
    cursor.setPosition(frame->lastPosition(), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    this->removeTextFrame(element);

    if(m_sceneResetList.removeOne(scene))
        m_sceneResetTimer.start(100, this);

    this->disconnectFromSceneSignals(scene);
}

void ScreenplayTextDocument::onSceneInserted(ScreenplayElement *element, int index)
{
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating while new scene was inserted.");

    Scene *scene = element->scene();
    if(scene == nullptr)
        return;

    ScreenplayTextDocumentUpdate update(this);

    QTextCursor cursor(m_textDocument);
    if(index == m_screenplay->elementCount()-1)
        cursor = m_textDocument->rootFrame()->lastCursorPosition();
    else if(index > 0)
    {
        ScreenplayElement *before = nullptr;
        while(before == nullptr)
        {
            before = m_screenplay->elementAt(--index);
            if(before->scene() == nullptr)
            {
                before = nullptr;
                continue;
            }
        }

        if(before != nullptr)
        {
            QTextFrame *beforeFrame = this->findTextFrame(before);
            Q_ASSERT_X(beforeFrame != nullptr, "ScreenplayTextDocument", "Attempting to insert scene before screenplay is loaded.");
            cursor = beforeFrame->lastCursorPosition();
            cursor.movePosition(QTextCursor::Down);
        }
    }

    QTextFrame *frame = cursor.insertFrame(m_sceneFrameFormat);
    this->registerTextFrame(element, frame);
    this->loadScreenplayElement(element, cursor);

    if(m_syncEnabled)
        this->connectToSceneSignals(scene);
}

void ScreenplayTextDocument::onSceneReset()
{
    Scene *scene = qobject_cast<Scene*>(this->sender());
    if(scene == nullptr)
        return;

    this->addToSceneResetList(scene);
}

void ScreenplayTextDocument::onSceneRefreshed()
{
    this->onSceneReset();
}

void ScreenplayTextDocument::onSceneAboutToReset()
{
    Scene *scene = qobject_cast<Scene*>(this->sender());
    if(scene == nullptr)
        return;

    this->disconnectFromSceneSignals(scene);
    connect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
}

void ScreenplayTextDocument::onSceneHeadingChanged()
{
#if 1
    this->onSceneReset();
#else
    SceneHeading *heading = qobject_cast<SceneHeading*>(this->sender());
    Scene *scene = heading ? heading->scene() : nullptr;
    if(scene == nullptr)
        return;

    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating scene heading changed.");

    ScreenplayTextDocumentUpdate update(this);

    QList<ScreenplayElement*> elements = m_screenplay->sceneElements(scene);
    Q_FOREACH(ScreenplayElement *element, elements)
    {
        QTextFrame *frame = this->findTextFrame(element);
        Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument", "Attempting to update a scene before it was included in the text document.");

        QTextCursor cursor = frame->firstCursorPosition();
        QTextBlock block = cursor.block();
        ScreenplayParagraphBlockData *data = ScreenplayParagraphBlockData::get(block);
        if(data->elementType() == SceneElement::Heading)
        {
            if(heading->isEnabled())
                this->formatBlock(block, heading->text());
            else
            {
                cursor.select(QTextCursor::BlockUnderCursor);
                cursor.removeSelectedText();
            }
        }
        else if(heading->isEnabled())
        {
            cursor.insertBlock();
            this->formatBlock(cursor.block(), heading->text());
        }
    }
#endif
}

void ScreenplayTextDocument::onSceneElementChanged(SceneElement *para, Scene::SceneElementChangeType type)
{
    Scene *scene = qobject_cast<Scene*>(this->sender());
    if(scene == nullptr)
        return;

    Q_ASSERT_X(para->scene() == scene, "ScreenplayTextDocument", "Attempting to modify paragraph from outside the scene.");
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating while a scene's paragraph was changed.");

    const int paraIndex = scene->indexOfElement(para);
    if(paraIndex < 0)
        return; // This can happen when the paragraph is not part of the scene text, but
                // it exists as a way to capture a mute-character in the scene.

    ScreenplayTextDocumentUpdate update(this);

    QList<ScreenplayElement*> elements = m_screenplay->sceneElements(scene);
    Q_FOREACH(ScreenplayElement *element, elements)
    {
        QTextFrame *frame = this->findTextFrame(element);
        Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument", "Attempting to update a scene before it was included in the text document.");

        QTextCursor cursor = frame->firstCursorPosition();
        QTextBlock block = cursor.block();

        while(block.isValid())
        {
            ScreenplayParagraphBlockData *data = ScreenplayParagraphBlockData::get(block);
            if(data && data->contains(para))
            {
                if(type == Scene::ElementTypeChange)
                    this->formatBlock(block);
                else if(type == Scene::ElementTextChange)
                    this->formatBlock(block, para->text());
                break;
            }

            block = block.next();
        }
    }
}

void ScreenplayTextDocument::onSceneAboutToResetModel()
{
    Scene *scene = qobject_cast<Scene*>(this->sender());
    if(scene == nullptr)
        return;

    this->disconnectFromSceneSignals(scene);
    connect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel);
}

void ScreenplayTextDocument::onSceneResetModel()
{
    Scene *scene = qobject_cast<Scene*>(this->sender());
    if(scene == nullptr)
        return;

    disconnect(scene, &Scene::modelReset, this, &ScreenplayTextDocument::onSceneResetModel);
    this->onSceneReset();
}

void ScreenplayTextDocument::onElementFormatChanged()
{
#if 0
    if(m_updating)
        return;

    ScreenplayTextDocumentUpdate update(this);

    SceneElementFormat *format = qobject_cast<SceneElementFormat*>(this->sender());
    if(format == nullptr)
        return;

    QTextCursor cursor(m_textDocument);
    QTextBlock block = cursor.block();
    while(block.isValid())
    {
        ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
        if(blockData && blockData->elementType() == format->elementType())
            this->formatBlock(block);

        block = block.next();
    }
#else
    // It is less time consuming to reload the whole document than it is
    // to apply formatting. This is mostly because iterating over text blocks
    // in a document is more expensive than just creating them from scratch
    this->loadScreenplayLater();
#endif
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
    if(m_activeScene != activeScene)
    {
        if(m_activeScene)
        {
            disconnect(m_activeScene, &Scene::aboutToDelete, this, &ScreenplayTextDocument::onActiveSceneDestroyed);
            disconnect(m_activeScene, &Scene::cursorPositionChanged, this, &ScreenplayTextDocument::onActiveSceneCursorPositionChanged);
        }

        m_activeScene = activeScene;

        if(m_activeScene)
        {
            connect(m_activeScene, &Scene::aboutToDelete, this, &ScreenplayTextDocument::onActiveSceneDestroyed);
            connect(m_activeScene, &Scene::cursorPositionChanged, this, &ScreenplayTextDocument::onActiveSceneCursorPositionChanged);
        }
    }

    this->evaluateCurrentPage();
}

void ScreenplayTextDocument::onActiveSceneDestroyed(Scene *ptr)
{
    if(ptr == m_activeScene)
        m_activeScene = nullptr;
}

void ScreenplayTextDocument::onActiveSceneCursorPositionChanged()
{
    this->evaluateCurrentPage();
}

void ScreenplayTextDocument::evaluateCurrentPage()
{
    if(m_screenplay == nullptr || m_screenplay->currentElementIndex() < 0 ||
       m_activeScene == nullptr || m_textDocument == nullptr || m_textDocument->isEmpty() ||
       m_formatting == nullptr)
    {
        this->setCurrentPage(0);
        return;
    }

    ScreenplayElement *element = m_screenplay->elementAt(m_screenplay->currentElementIndex());
    QTextFrame *frame = element && element->scene() && element->scene() == m_activeScene ? this->findTextFrame(element) : nullptr;
    if(frame == nullptr)
    {
        this->setCurrentPage(0);
        return;
    }

    QTextCursor endCursor(m_textDocument);
    endCursor.movePosition(QTextCursor::End);

    if(endCursor.position() == 0)
    {
        this->setCurrentPage(0);
        return;
    }

    QTextCursor userCursor = frame->firstCursorPosition();
    QTextBlock block = userCursor.block();
    ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
    if(blockData && blockData->elementType() == SceneElement::Heading)
        block = block.next();

    const int cursorPosition = m_activeScene->cursorPosition() + block.position();
    for(int i=0; i<m_pageBoundaries.size(); i++)
    {
        const QPair<int,int> pgBoundary = m_pageBoundaries.at(i);
        if(cursorPosition >= pgBoundary.first-1 && cursorPosition < pgBoundary.second)
        {
            this->setCurrentPage(i+1);
            return;
        }
    }

    // If we are here, then the cursor position was not found anywhere in the pageBoundaries.
    // So, we estimate the current page to be the last page.
    this->setCurrentPage(m_pageCount);
}

void ScreenplayTextDocument::evaluatePageBoundaries()
{
    // NOTE: Please do not call this function from anywhere other than
    // timerEvent(), while handling m_pageBoundaryEvalTimer
    QList< QPair<int,int> > pgBoundaries;

    if(m_formatting != nullptr && m_textDocument != nullptr)
    {
        m_textDocument->setDefaultFont(m_formatting->defaultFont());
        m_formatting->pageLayout()->configure(m_textDocument);

        this->setPageCount(m_textDocument->pageCount());

        const ScreenplayPageLayout *pageLayout = m_formatting->pageLayout();
        const QMarginsF pageMargins = pageLayout->margins();

        QRectF paperRect = pageLayout->paperRect();
        QAbstractTextDocumentLayout *layout = m_textDocument->documentLayout();

        QTextCursor endCursor(m_textDocument);
        endCursor.movePosition(QTextCursor::End);

        const int pageCount = m_textDocument->pageCount();
        int pageIndex = 0;
        while(pageIndex < pageCount)
        {
            paperRect = QRectF(0, pageIndex*paperRect.height(), paperRect.width(), paperRect.height());
            const QRectF contentsRect = paperRect.adjusted(pageMargins.left(), pageMargins.top(), -pageMargins.right(), -pageMargins.bottom());
            const int firstPosition = pgBoundaries.isEmpty() ? layout->hitTest(contentsRect.topLeft(), Qt::FuzzyHit) : pgBoundaries.last().second+1;
            const int lastPosition = pageIndex == pageCount-1 ? endCursor.position() : layout->hitTest(contentsRect.bottomRight(), Qt::FuzzyHit);
            pgBoundaries << qMakePair(firstPosition, lastPosition >= 0 ? lastPosition : endCursor.position());

            ++pageIndex;
        }
    }

    m_pageBoundaries = pgBoundaries;
    emit pageBoundariesChanged();

    this->evaluateCurrentPage();
}

void ScreenplayTextDocument::evaluatePageBoundariesLater()
{
    m_pageBoundaryEvalTimer.start(500, this);
}

void ScreenplayTextDocument::formatAllBlocks()
{
    if(m_screenplay == nullptr || m_formatting == nullptr || m_updating || !m_componentComplete || m_textDocument == nullptr || m_textDocument->isEmpty())
        return;

    QTextCursor cursor(m_textDocument);
    QTextBlock block = cursor.block();
    while(block.isValid())
    {
        this->formatBlock(block);
        block = block.next();
    }
}

void ScreenplayTextDocument::loadScreenplayElement(const ScreenplayElement *element, QTextCursor &cursor)
{
    Q_ASSERT_X(cursor.currentFrame() == this->findTextFrame(element),
               "ScreenplayTextDocument", "Screenplay element can be loaded only after a frame for it has been created");

    QTextCharFormat highlightCharFormat;
    highlightCharFormat.setBackground(Qt::yellow);

    const Scene *scene = element->scene();
    if(scene != nullptr)
    {
        bool insertBlock = false; // the newly inserted frame has a default first block.
                                  // its only from the second paragraph, that we need a new block.

        auto prepareCursor = [=](QTextCursor &cursor, SceneElement::Type paraType, bool firstParagraph) {
            const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
            const SceneElementFormat *format = m_formatting->elementFormat(paraType);
            QTextBlockFormat blockFormat = format->createBlockFormat(&pageWidth);
            QTextCharFormat charFormat = format->createCharFormat(&pageWidth);
            if(firstParagraph)
                blockFormat.setTopMargin(0);
            cursor.setCharFormat(charFormat);
            cursor.setBlockFormat(blockFormat);
        };

        const SceneHeading *heading = scene->heading();
        if(heading->isEnabled())
        {
            if(insertBlock)
                cursor.insertBlock();

            QTextBlock block = cursor.block();
            block.setUserData(new ScreenplayParagraphBlockData(nullptr));

            if(m_sceneIcons)
            {
                QTextCharFormat sceneIconFormat;
                sceneIconFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
                sceneIconFormat.setFont(m_formatting->elementFormat(SceneElement::Heading)->font());
                sceneIconFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty, ScreenplayTextObjectInterface::SceneIconType);
                sceneIconFormat.setProperty(ScreenplayTextObjectInterface::DataProperty, scene->type());
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), sceneIconFormat);
            }

            if(m_sceneNumbers)
            {
                QTextCharFormat sceneNumberFormat;
                sceneNumberFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
                sceneNumberFormat.setFont(m_formatting->elementFormat(SceneElement::Heading)->font());
                sceneNumberFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty, ScreenplayTextObjectInterface::SceneNumberType);
                sceneNumberFormat.setProperty(ScreenplayTextObjectInterface::DataProperty, element->sceneNumber());
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), sceneNumberFormat);
            }

            prepareCursor(cursor, SceneElement::Heading, !insertBlock);
            if(m_purpose == ForPrinting)
                polishFontsAndInsertTextAtCursor(cursor, heading->text());
            else
                cursor.insertText(heading->text());
            insertBlock = true;
        }

        AbstractScreenplayTextDocumentInjectionInterface *injection = qobject_cast<AbstractScreenplayTextDocumentInjectionInterface*>(m_injection);
        if(injection != nullptr)
        {
            injection->setScreenplayElement(element);
            injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::AfterSceneHeading);
            injection->setScreenplayElement(nullptr);
        }

        if(m_listSceneCharacters)
        {
            const QStringList sceneCharacters = scene->characterNames();
            if(!sceneCharacters.isEmpty())
            {
                if(insertBlock)
                    cursor.insertBlock();

                prepareCursor(cursor, SceneElement::Heading, !insertBlock);
                cursor.mergeCharFormat(highlightCharFormat);
                cursor.insertText( QStringLiteral("{") + sceneCharacters.join(", ") + QStringLiteral("}") );
                insertBlock = true;
            }
        }

        if(m_includeSceneSynopsis && !scene->title().isEmpty())
        {
            QColor sceneColor = scene->color().lighter(175);
            sceneColor.setAlphaF(0.5);

            QTextBlockFormat blockFormat;
            blockFormat.setTopMargin(10);
            blockFormat.setBackground(sceneColor);

            QTextCharFormat charFormat;
            charFormat.setFont(Application::instance()->font());

            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(scene->title());

            insertBlock = true;
        }

        bool highlightParagraph = false;
        for(int j=0; j<scene->elementCount(); j++)
        {
            const SceneElement *para = scene->elementAt(j);
            if(injection != nullptr)
            {
                injection->setSceneElement(para);
                injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::BeforeSceneElement);
                if(injection->filterSceneElement())
                {
                    injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::AfterSceneElement);
                    continue;
                }
            }

            if(insertBlock)
                cursor.insertBlock();

            QTextBlock block = cursor.block();
            block.setUserData(new ScreenplayParagraphBlockData(para));
            prepareCursor(cursor, para->type(), !insertBlock);

            if(!m_highlightDialoguesOf.isEmpty())
            {
                if(para->type() == SceneElement::Character)
                {
                    const QString chName = para->text().section('(', 0, 0).trimmed();
                    highlightParagraph = (m_highlightDialoguesOf.contains(chName, Qt::CaseInsensitive));
                }
                else if(para->type() != SceneElement::Parenthetical && para->type() != SceneElement::Dialogue)
                    highlightParagraph = false;
            }

            if(highlightParagraph)
                cursor.mergeCharFormat(highlightCharFormat);

            const QString text = para->text();
            if(m_purpose == ForPrinting)
                polishFontsAndInsertTextAtCursor(cursor, text);
            else
                cursor.insertText(text);

            if(injection != nullptr)
                injection->inject(cursor, AbstractScreenplayTextDocumentInjectionInterface::AfterSceneElement);

            insertBlock = true;
        }
    }
}

void ScreenplayTextDocument::formatBlock(const QTextBlock &block, const QString &text)
{
    if(m_formatting == nullptr)
        return;

    ScreenplayParagraphBlockData *blockData = ScreenplayParagraphBlockData::get(block);
    if(blockData == nullptr)
        return;

    const qreal pageWidth = m_formatting->pageLayout()->contentWidth();
    const SceneElementFormat *format = m_formatting->elementFormat(blockData->elementType());
    const QTextBlockFormat blockFormat = format->createBlockFormat(&pageWidth);
    const QTextCharFormat charFormat = format->createCharFormat(&pageWidth);

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.setBlockFormat(blockFormat);
    cursor.setCharFormat(charFormat);
    if(!text.isEmpty())
        cursor.insertText(text);
}

void ScreenplayTextDocument::removeTextFrame(const ScreenplayElement *element)
{
    this->registerTextFrame(element, nullptr);
}

void ScreenplayTextDocument::registerTextFrame(const ScreenplayElement *element, QTextFrame *frame)
{
    QTextFrame *existingFrame = m_elementFrameMap.value(element, nullptr);
    if(existingFrame && existingFrame != frame)
    {
        m_elementFrameMap.remove(element);
        m_frameElementMap.remove(existingFrame);
        disconnect(existingFrame, &QTextFrame::destroyed, this, &ScreenplayTextDocument::onTextFrameDestroyed);
    }

    if(frame != nullptr)
    {
        m_elementFrameMap[element] = frame;
        m_frameElementMap[frame] = element;
        connect(frame, &QTextFrame::destroyed, this, &ScreenplayTextDocument::onTextFrameDestroyed);
    }
}

QTextFrame *ScreenplayTextDocument::findTextFrame(const ScreenplayElement *element) const
{
    return m_elementFrameMap.value(element, nullptr);
}

void ScreenplayTextDocument::onTextFrameDestroyed(QObject *object)
{
    const ScreenplayElement *element = m_frameElementMap.value(object, nullptr);
    if(element != nullptr)
    {
        m_frameElementMap.remove(object);
        m_elementFrameMap.remove(element);
    }
}

void ScreenplayTextDocument::clearTextFrames()
{
    m_elementFrameMap.clear();

    QList<QObject*> textFrames = m_frameElementMap.keys();
    Q_FOREACH(QObject *textFrame, textFrames)
        disconnect(textFrame, &QTextFrame::destroyed, this, &ScreenplayTextDocument::onTextFrameDestroyed);
    m_frameElementMap.clear();
}

void ScreenplayTextDocument::addToSceneResetList(Scene *scene)
{
    if(!m_sceneResetList.contains(scene))
        m_sceneResetList.append(scene);
    m_sceneResetTimer.start(100, this);
}

void ScreenplayTextDocument::processSceneResetList()
{
    if(m_sceneResetList.isEmpty())
        return;

    ScreenplayTextDocumentUpdate update(this);

    QList<Scene*> scenes = m_sceneResetList;
    m_sceneResetList.clear();

    while(!scenes.isEmpty())
    {
        Scene *scene = scenes.takeFirst();

        QList<ScreenplayElement*> elements = m_screenplay->sceneElements(scene);
        Q_FOREACH(ScreenplayElement *element, elements)
        {
            QTextFrame *frame = this->findTextFrame(element);
            Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument", "Attempting to update a scene before it was included in the text document.");

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

ScreenplayElementPageBreaks::~ScreenplayElementPageBreaks()
{

}

void ScreenplayElementPageBreaks::setScreenplayDocument(ScreenplayTextDocument *val)
{
    if(m_screenplayDocument == val)
        return;

    if(m_screenplayDocument != nullptr)
        disconnect(m_screenplayDocument, &ScreenplayTextDocument::pageBoundariesChanged,
                this, &ScreenplayElementPageBreaks::updatePageBreaks);

    m_screenplayDocument = val;

    if(m_screenplayDocument != nullptr)
        connect(m_screenplayDocument, &ScreenplayTextDocument::pageBoundariesChanged,
                this, &ScreenplayElementPageBreaks::updatePageBreaks);

    emit screenplayDocumentChanged();

    this->updatePageBreaks();
}

void ScreenplayElementPageBreaks::setScreenplayElement(ScreenplayElement *val)
{
    if(m_screenplayElement == val)
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
    QVariantList breaks;

    if(m_screenplayDocument != nullptr && m_screenplayElement != nullptr)
    {
        const QList< QPair<int,int> > ibreaks = m_screenplayDocument->pageBreaksFor(m_screenplayElement);
        QPair<int,int> ibreak;
        Q_FOREACH(ibreak, ibreaks)
        {
            QVariantMap item;
            item["position"] = ibreak.first;
            item["pageNumber"] = ibreak.second;
            breaks << item;
        }
    }

    this->setPageBreaks(breaks);
}

void ScreenplayElementPageBreaks::setPageBreaks(const QVariantList &val)
{
    if(m_pageBreaks == val)
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

ScreenplayTitlePageObjectInterface::~ScreenplayTitlePageObjectInterface()
{

}

QSizeF ScreenplayTitlePageObjectInterface::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(format)
    Q_UNUSED(posInDocument)

    const QSizeF pageSize = doc->pageSize();
    const QTextFrameFormat rootFrameFormat = doc->rootFrame()->frameFormat();
    const QSizeF ret( (pageSize.width() - rootFrameFormat.leftMargin() - rootFrameFormat.rightMargin()),
                   (pageSize.height() - rootFrameFormat.topMargin() - rootFrameFormat.bottomMargin()) );
    return ret;
}

void ScreenplayTitlePageObjectInterface::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
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

      On the bottom right, we will have the third frame with the following information
      Written or Generated using Scrite
      https://www.scrite.io
      */

    const Screenplay *screenplay = qobject_cast<Screenplay*>(format.property(ScreenplayProperty).value<QObject*>());
    if(screenplay == nullptr)
        return;

    const Screenplay *masterScreenplay = ScriteDocument::instance()->screenplay();
    const Screenplay *coverPageImageScreenplay = screenplay;
    if(screenplay->property("#useDocumentScreenplayForCoverPagePhoto").toBool() == true)
        coverPageImageScreenplay = masterScreenplay;

    auto fetch = [](const QString &given, const QString &defaultValue) {
        const QString val = given.trimmed();
        return val.isEmpty() ? defaultValue : val;
    };

    const QString title = fetch(screenplay->title(), QStringLiteral("Untitled Screenplay"));
    const QString subtitle = screenplay->subtitle();
    const QString writtenBy = QStringLiteral("Written By");
    const QString basedOn = screenplay->basedOn();
    const QString version = fetch(screenplay->version(), QStringLiteral("Initial Draft"));
    const QString authors = fetch(screenplay->author(), QStringLiteral("A Good Writer"));
    const QString contact = fetch(screenplay->contact(), authors);
    const QString address = screenplay->address();
    const QString phoneNumber = screenplay->phoneNumber();
    const QString email = screenplay->email();
    const QString website = screenplay->website();
    const QString marketing = QStringLiteral("Written/Generated using Scrite (www.scrite.io)");

    const QFont normalFont = doc->defaultFont();
    const QFontMetricsF normalFontMetrics(normalFont);

    QFont titleFont = normalFont;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize()+2);
    const QFontMetricsF titleFontMetrics(titleFont);

    QFont marketingFont = normalFont;
    marketingFont.setPointSize(8);
    const QFontMetricsF marketingFontMetrics(marketingFont);

    const QString newline = QStringLiteral("\n");
    const QString emptyPara = QStringLiteral(".");
    auto createParagraph = [newline,emptyPara](const QStringList &items) {
        QString ret;
        for(int i=0; i<items.size(); i++) {
            const QString item = items.at(i);
            if(item.isEmpty())
                continue;
            if(!ret.isEmpty())
                ret += newline;
            if(item != emptyPara)
                ret += item;
        }
        return ret.trimmed();
    };

    const QString centerFrameText = createParagraph( QStringList() << subtitle << emptyPara << writtenBy << authors << emptyPara << basedOn << emptyPara << version);
    QRectF centerFrameRect = normalFontMetrics.boundingRect(rect, Qt::TextWordWrap, centerFrameText);
    centerFrameRect.moveCenter(rect.center());
    centerFrameRect.moveBottom(rect.center().y());

    const QString titleFrameText = title;
    QRectF titleFrameRect = titleFontMetrics.boundingRect(rect, Qt::TextWordWrap, titleFrameText);
    titleFrameRect.moveCenter(centerFrameRect.center());
    titleFrameRect.moveBottom(centerFrameRect.top());

    const QString marketingText = marketing;
    QRectF marketingFrame = marketingFontMetrics.boundingRect(rect, Qt::TextWordWrap, marketingText);
    marketingFrame.moveCenter(rect.center());
    marketingFrame.moveTop( rect.bottom() + marketingFrame.height()*3 );

    const QString contactFrameText = createParagraph( QStringList() << contact << address << phoneNumber << email << website );
    QRectF contactFrameRect = normalFontMetrics.boundingRect(rect, Qt::TextWordWrap, contactFrameText);
    contactFrameRect.moveBottomLeft(rect.bottomLeft());

    const bool isPdfDevice = painter->device()->paintEngine()->type() == QPaintEngine::Pdf;
    const qreal defaultDpi = qt_defaultDpi();
    auto paintText = [isPdfDevice,defaultDpi](QPainter *painter, const QRectF &rect, int flags, const QString &text) {
        if(isPdfDevice) {
            const qreal invScaleX = defaultDpi / qreal(painter->device()->logicalDpiX());
            const qreal invScaleY = defaultDpi / qreal(painter->device()->logicalDpiY());
            painter->save();
            painter->translate(rect.left(), rect.top());
            painter->scale(invScaleX, invScaleY);
            const QRectF textRect(0,0,(rect.width()*1.1)/invScaleX,rect.height()/invScaleY);
            painter->drawText(textRect, flags, text);
            painter->restore();
        } else
            painter->drawText(rect, flags, text);
    };

    painter->save();

    if(!coverPageImageScreenplay->coverPagePhoto().isEmpty())
    {
        QImage photo(coverPageImageScreenplay->coverPagePhoto());
        QRectF photoRect = photo.rect();
        QSizeF photoSize = photoRect.size();

        QRectF spaceAvailable = rect;
        spaceAvailable.setBottom(titleFrameRect.top() - titleFrameRect.height());
        photoSize.scale(spaceAvailable.size(), Qt::KeepAspectRatio);

        switch(coverPageImageScreenplay->coverPagePhotoSize())
        {
        case Screenplay::LargeCoverPhoto:
            break;
        case Screenplay::MediumCoverPhoto:
            photoSize /= 2.0;
            photo = photo.scaled( photo.size()/2.0, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
            break;
        case Screenplay::SmallCoverPhoto:
            photoSize /= 4.0;
            photo = photo.scaled( photo.size()/4.0, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
            break;
        }

        photoRect.setSize(photoSize);
        photoRect.moveCenter(spaceAvailable.center());
        photoRect.moveBottom(spaceAvailable.bottom());

        const bool flag = painter->renderHints().testFlag(QPainter::SmoothPixmapTransform);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawImage(photoRect, photo);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, flag);
    }

    painter->setFont(titleFont);
    paintText(painter, titleFrameRect, Qt::AlignHCenter|Qt::TextWordWrap, titleFrameText);

    painter->setFont(normalFont);
    paintText(painter, centerFrameRect, Qt::AlignHCenter|Qt::TextWordWrap, centerFrameText);
    paintText(painter, contactFrameRect, Qt::AlignLeft|Qt::TextWordWrap, contactFrameText);

    painter->setFont(marketingFont);
    painter->setPen(Qt::darkGray);
    paintText(painter, marketingFrame, Qt::AlignRight|Qt::TextWordWrap, marketingText);

    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayTextObjectInterface::ScreenplayTextObjectInterface(QObject *parent)
    : QObject(parent)
{

}

ScreenplayTextObjectInterface::~ScreenplayTextObjectInterface()
{

}

QSizeF ScreenplayTextObjectInterface::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const QFont font( format.property(QTextFormat::FontFamily).toString(), format.property(QTextFormat::FontPointSize).toInt() );
    const QFontMetricsF fontMetrics(font);
    return QSizeF(0, fontMetrics.lineSpacing() - fontMetrics.descent());
}

void ScreenplayTextObjectInterface::drawObject(QPainter *painter, const QRectF &givenRect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const QFont font( format.property(QTextFormat::FontFamily).toString(), format.property(QTextFormat::FontPointSize).toInt() );
    painter->setFont(font);

    int type = format.property(TypeProperty).toInt();
    switch(type)
    {
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

void ScreenplayTextObjectInterface::drawSceneNumber(QPainter *painter, const QRectF &givenRect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const int sceneNumber = format.property(DataProperty).toInt();
    if(sceneNumber < 0)
        return;

    QRectF rect = givenRect;
    rect.setLeft( rect.left()*0.6 );

    const QString sceneNumberText = QString::number(sceneNumber) + QStringLiteral(".");
    this->drawText(painter, rect, sceneNumberText);
}

void ScreenplayTextObjectInterface::drawMoreMarker(QPainter *painter, const QRectF &givenRect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const QString text = format.property(DataProperty).toString();
    if(text.isEmpty())
        return;

    QFontMetricsF fm(painter->font());
    QRectF rect = fm.boundingRect(text);
    rect.moveLeft( givenRect.right() );
    rect.moveBottom( givenRect.bottom() );

    const QPen oldPen = painter->pen();

    QColor textColor = format.foreground().color();
    textColor.setAlphaF(textColor.alphaF()*0.75);
    painter->setPen(textColor);
    this->drawText(painter, rect, text);

    painter->setPen(oldPen);
}

void ScreenplayTextObjectInterface::drawSceneIcon(QPainter *painter, const QRectF &givenRect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    const int sceneType = format.property(DataProperty).toInt();
    if(sceneType == Scene::Standard)
        return;

    static const QImage musicIcon(":/icons/content/queue_mus24px.png");
    static const QImage actionIcon(":/icons/content/fight_scene.png");
    const qreal iconSize = givenRect.height();
    QImage icon = sceneType == Scene::Action ? actionIcon : musicIcon;

    QRectF rect = givenRect;
    rect.setLeft( rect.left()*0.45 );
    rect.moveBottom( rect.bottom()+iconSize*0.15 );

    const bool flag = painter->renderHints().testFlag(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(QRectF(rect.left()-iconSize, rect.bottom()-iconSize, iconSize, iconSize), icon);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, flag);
}

void ScreenplayTextObjectInterface::drawText(QPainter *painter, const QRectF &rect, const QString &text)
{
    const bool isPdfDevice = painter->device()->paintEngine()->type() == QPaintEngine::Pdf;

    if(isPdfDevice)
    {
        const qreal invScaleX = qreal(qt_defaultDpi()) / painter->device()->logicalDpiX();
        const qreal invScaleY = qreal(qt_defaultDpi()) / painter->device()->logicalDpiY();

        painter->save();
        painter->translate(rect.left(), rect.bottom());
        painter->scale(invScaleX, invScaleY);
        painter->drawText(0, 0, text);
        painter->restore();
    }
    else
        painter->drawText(rect.bottomLeft(), text);
}

