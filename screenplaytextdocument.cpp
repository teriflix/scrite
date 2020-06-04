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
#include "garbagecollector.h"
#include "screenplaytextdocument.h"

#include <QDate>
#include <QDateTime>
#include <QQmlEngine>
#include <QTextBlock>
#include <QPdfWriter>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextBlockUserData>
#include <QScopedValueRollback>

class ScreenplayParagraphBlockData : public QTextBlockUserData
{
public:
    ScreenplayParagraphBlockData(const SceneElement *element);
    ~ScreenplayParagraphBlockData();

    bool contains(const SceneElement *other) const;
    SceneElement::Type elementType() const;
    QString elementText() const;

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
      m_textDocument(new QTextDocument(this))
{
    this->init();
}

ScreenplayTextDocument::ScreenplayTextDocument(QTextDocument *document, QObject *parent)
    : QObject(parent),
      m_textDocument(document)
{
    this->init();
}

ScreenplayTextDocument::~ScreenplayTextDocument()
{
    if(m_textDocument != nullptr && m_textDocument->parent() == this)
        m_textDocument->setUndoRedoEnabled(true);
}

void ScreenplayTextDocument::setTextDocument(QTextDocument *val)
{
    if(m_textDocument == val)
        return;

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

    m_screenplay = val;

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

void ScreenplayTextDocument::setSyncEnabled(bool val)
{
    if(m_syncEnabled == val)
        return;

    m_syncEnabled = val;

    if(m_syncEnabled)
    {
        this->connectToScreenplaySignals();
        this->connectToScreenplayFormatSignals();
    }
    else
    {
        this->disconnectFromScreenplaySignals();
        this->disconnectFromScreenplayFormatSignals();
    }

    emit syncEnabledChanged();
}

void ScreenplayTextDocument::print(QObject *printerObject)
{
    HourGlass hourGlass;

    if(m_textDocument == nullptr || m_screenplay == nullptr || m_formatting == nullptr)
        return;

    QPagedPaintDevice *printer = nullptr;

    QPdfWriter *pdfWriter = qobject_cast<QPdfWriter*>(printerObject);
    if(pdfWriter)
        printer = pdfWriter;

    ImagePrinter *imagePrinter = qobject_cast<ImagePrinter*>(printerObject);
    if(imagePrinter)
    {
        printer = imagePrinter;

        QMap<HeaderFooter::Field,QString> fieldMap;
        fieldMap[HeaderFooter::AppName] = Application::instance()->applicationName();
        fieldMap[HeaderFooter::AppVersion] = Application::instance()->applicationVersion();
        fieldMap[HeaderFooter::Title] = m_screenplay->title();
        fieldMap[HeaderFooter::Subtitle] = m_screenplay->subtitle();
        fieldMap[HeaderFooter::Author] = m_screenplay->author();
        fieldMap[HeaderFooter::Contact] = m_screenplay->contact();
        fieldMap[HeaderFooter::Version] = m_screenplay->version();
        fieldMap[HeaderFooter::Date] = QDate::currentDate().toString(Qt::SystemLocaleShortDate);
        fieldMap[HeaderFooter::Time] = QTime::currentTime().toString(Qt::SystemLocaleShortDate);
        fieldMap[HeaderFooter::DateTime] = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
        fieldMap[HeaderFooter::PageNumber] = QString::number(m_textDocument->pageCount()) + ".  ";
        fieldMap[HeaderFooter::PageNumberOfCount] = QString::number(m_textDocument->pageCount()) + "/" + QString::number(m_textDocument->pageCount()) + "  ";

        imagePrinter->header()->setFont(m_textDocument->defaultFont());
        imagePrinter->footer()->setFont(m_textDocument->defaultFont());
        imagePrinter->setHeaderFooterFields(fieldMap);
    }

    if(printer)
    {
        m_formatting->pageLayout()->configure(printer);
        m_textDocument->print(printer);
    }

    if(imagePrinter)
        imagePrinter->clearHeaderFooterFields();
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
        m_loadScreenplayTimer.stop();
        this->loadScreenplay();
        this->connectToScreenplaySignals();
        this->connectToScreenplayFormatSignals();
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
        emit updateFinished();

        if(m_textDocument != nullptr)
            this->setPageCount(m_textDocument->pageCount());
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
    m_elementFrameMap.clear();
    m_textDocument->clear();

    if(m_screenplay == nullptr)
        return;

    if(m_screenplay->elementCount() == 0)
        return;

    if(m_formatting == nullptr)
        this->setFormatting(new ScreenplayFormat(this));

    m_textDocument->setDefaultFont(m_formatting->defaultFont());
    m_formatting->pageLayout()->configure(m_textDocument);

    QTextBlockFormat frameBoundaryBlockFormat;
    frameBoundaryBlockFormat.setLineHeight(0, QTextBlockFormat::FixedHeight);

    QTextCursor cursor(m_textDocument);
    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        const ScreenplayElement *element = m_screenplay->elementAt(i);

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

        // Each screenplay element (or scene) has its own frame. That makes
        // moving them in one bunch easy.
        QTextFrame *frame = cursor.insertFrame(frameFormat);
        m_elementFrameMap[element] = frame;
        this->loadScreenplayElement(element, cursor);

        // We have to move the cursor out of the frame we created for the scene
        // https://doc.qt.io/qt-5/richtext-cursor.html#frames
        cursor = m_textDocument->rootFrame()->lastCursorPosition();
        cursor.setBlockFormat(frameBoundaryBlockFormat);
    }
}

void ScreenplayTextDocument::loadScreenplayLater()
{
    this->disconnectFromScreenplaySignals();
    this->disconnectFromScreenplayFormatSignals();

    m_loadScreenplayTimer.start(100, this);
}

void ScreenplayTextDocument::connectToScreenplaySignals()
{
    if(m_screenplay == nullptr || !m_syncEnabled || m_connectedToScreenplaySignals)
        return;

    connect(m_screenplay, &Screenplay::elementMoved, this, &ScreenplayTextDocument::onSceneMoved);
    connect(m_screenplay, &Screenplay::elementRemoved, this, &ScreenplayTextDocument::onSceneRemoved);
    connect(m_screenplay, &Screenplay::elementInserted, this, &ScreenplayTextDocument::onSceneInserted);
    connect(m_screenplay, &Screenplay::activeSceneChanged, this, &ScreenplayTextDocument::onActiveSceneChanged);

    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        Scene *scene = element->scene();
        if(scene == nullptr)
            continue;

        connect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
        connect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed);
        connect(scene, &Scene::sceneElementChanged, this, &ScreenplayTextDocument::onSceneElementChanged);

        SceneHeading *heading = scene->heading();
        connect(heading, &SceneHeading::textChanged, this, &ScreenplayTextDocument::onSceneHeadingChanged);
    }

    this->onActiveSceneChanged();

    m_connectedToScreenplaySignals = true;
}

void ScreenplayTextDocument::connectToScreenplayFormatSignals()
{
    if(m_formatting == nullptr || !m_syncEnabled || m_connectedToFormattingSignals)
        return;

    connect(m_formatting, &ScreenplayFormat::defaultFontChanged, this, &ScreenplayTextDocument::onDefaultFontChanged);
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
    disconnect(m_screenplay, &Screenplay::elementRemoved, this, &ScreenplayTextDocument::onSceneRemoved);
    disconnect(m_screenplay, &Screenplay::elementInserted, this, &ScreenplayTextDocument::onSceneInserted);

    for(int i=0; i<m_screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        Scene *scene = element->scene();
        if(scene == nullptr)
            continue;

        disconnect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
        disconnect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed);
        disconnect(scene, &Scene::sceneElementChanged, this, &ScreenplayTextDocument::onSceneElementChanged);

        SceneHeading *heading = scene->heading();
        disconnect(heading, &SceneHeading::textChanged, this, &ScreenplayTextDocument::onSceneHeadingChanged);
    }

    m_connectedToScreenplaySignals = false;
}

void ScreenplayTextDocument::disconnectFromScreenplayFormatSignals()
{
    if(m_formatting == nullptr || !m_connectedToFormattingSignals)
        return;

    disconnect(m_formatting, &ScreenplayFormat::defaultFontChanged, this, &ScreenplayTextDocument::onDefaultFontChanged);
    for(int i=SceneElement::Min; i<=SceneElement::Max; i++)
    {
        SceneElementFormat *elementFormat = m_formatting->elementFormat(i);
        disconnect(elementFormat, &SceneElementFormat::elementFormatChanged, this, &ScreenplayTextDocument::onElementFormatChanged);
    }

    m_connectedToFormattingSignals = false;
}

void ScreenplayTextDocument::onSceneMoved(ScreenplayElement *element, int from, int to)
{
    this->onSceneRemoved(element, from);
    this->onSceneInserted(element, to);
}

void ScreenplayTextDocument::onSceneRemoved(ScreenplayElement *element, int index)
{
    Q_UNUSED(index)
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating while new scene was removed.");

    Scene *scene = element->scene();
    if(scene == nullptr)
        return;

    ScreenplayTextDocumentUpdate update(this);

    QTextFrame *frame = m_elementFrameMap.value(element, nullptr);
    Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument", "Attempting to remove a scene before it was included in the text document.");

    QTextCursor cursor = frame->firstCursorPosition();
    cursor.movePosition(QTextCursor::Up);
    cursor.setPosition(frame->lastPosition(), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_elementFrameMap.remove(element);

    disconnect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
    disconnect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed);
    disconnect(scene, &Scene::sceneElementChanged, this, &ScreenplayTextDocument::onSceneElementChanged);

    SceneHeading *heading = scene->heading();
    disconnect(heading, &SceneHeading::textChanged, this, &ScreenplayTextDocument::onSceneHeadingChanged);
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
        ScreenplayElement *before = m_screenplay->elementAt(index-1);
        QTextFrame *beforeFrame = m_elementFrameMap.value(before, nullptr);
        Q_ASSERT_X(beforeFrame != nullptr, "ScreenplayTextDocument", "Attempting to insert scene before screenplay is loaded.");
        cursor = beforeFrame->lastCursorPosition();
        cursor.movePosition(QTextCursor::Down);
    }

    QTextFrame *frame = cursor.insertFrame(m_sceneFrameFormat);
    m_elementFrameMap[element] = frame;
    this->loadScreenplayElement(element, cursor);

    if(m_syncEnabled)
    {
        connect(scene, &Scene::sceneReset, this, &ScreenplayTextDocument::onSceneReset);
        connect(scene, &Scene::sceneRefreshed, this, &ScreenplayTextDocument::onSceneRefreshed);
        connect(scene, &Scene::sceneElementChanged, this, &ScreenplayTextDocument::onSceneElementChanged);

        SceneHeading *heading = scene->heading();
        connect(heading, &SceneHeading::textChanged, this, &ScreenplayTextDocument::onSceneHeadingChanged);
    }
}

void ScreenplayTextDocument::onSceneReset()
{
    Scene *scene = qobject_cast<Scene*>(this->sender());
    if(scene == nullptr)
        return;

    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating while new scene was reset or refreshed.");

    ScreenplayTextDocumentUpdate update(this);

    QList<ScreenplayElement*> elements = m_screenplay->sceneElements(scene);
    Q_FOREACH(ScreenplayElement *element, elements)
    {
        QTextFrame *frame = m_elementFrameMap.value(element, nullptr);
        Q_ASSERT_X(frame != nullptr, "ScreenplayTextDocument", "Attempting to update a scene before it was included in the text document.");

        QTextCursor cursor = frame->firstCursorPosition();
        cursor.setPosition(frame->lastPosition(), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        this->loadScreenplayElement(element, cursor);
    }
}

void ScreenplayTextDocument::onSceneRefreshed()
{
    this->onSceneReset();
}

void ScreenplayTextDocument::onSceneHeadingChanged()
{
    SceneHeading *heading = qobject_cast<SceneHeading*>(this->sender());
    Scene *scene = heading ? heading->scene() : nullptr;
    if(scene == nullptr)
        return;

    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating scene heading changed.");

    ScreenplayTextDocumentUpdate update(this);

    QList<ScreenplayElement*> elements = m_screenplay->sceneElements(scene);
    Q_FOREACH(ScreenplayElement *element, elements)
    {
        QTextFrame *frame = m_elementFrameMap.value(element, nullptr);
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
    }
}

void ScreenplayTextDocument::onSceneElementChanged(SceneElement *para, Scene::SceneElementChangeType type)
{
    Scene *scene = qobject_cast<Scene*>(this->sender());
    if(scene == nullptr)
        return;

    Q_ASSERT_X(para->scene() == scene, "ScreenplayTextDocument", "Attempting to modify paragraph from outside the scene.");
    Q_ASSERT_X(m_updating == false, "ScreenplayTextDocument", "Document was updating while a scene's paragraph was changed.");

    const int paraIndex = scene->indexOfElement(para);
    Q_ASSERT_X(paraIndex >= 0, "ScreenplayTextDocument", "Attempting to modify a non-existent paragraph from within the scene.");

    ScreenplayTextDocumentUpdate update(this);

    QList<ScreenplayElement*> elements = m_screenplay->sceneElements(scene);
    Q_FOREACH(ScreenplayElement *element, elements)
    {
        QTextFrame *frame = m_elementFrameMap.value(element, nullptr);
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
       m_activeScene == nullptr || m_textDocument == nullptr || m_textDocument->isEmpty())
    {
        this->setCurrentPage(0);
        return;
    }

    ScreenplayElement *element = m_screenplay->elementAt(m_screenplay->currentElementIndex());
    QTextFrame *frame = element && element->scene() && element->scene() == m_activeScene ? m_elementFrameMap.value(element) : nullptr;
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
    userCursor.setPosition(userCursor.position() + m_activeScene->cursorPosition());

    // As of now there is no way to accurately determine the page number
    // in which a cursor-position would come inside of. So, we make an
    // aproximate guess right now.
    const int pageNr = 1 + int(qreal(userCursor.position())/qreal(endCursor.position()) * qreal(m_textDocument->pageCount()));
    this->setCurrentPage(pageNr);
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
    Q_ASSERT_X(cursor.currentFrame() == m_elementFrameMap.value(element, nullptr),
               "ScreenplayTextDocument", "Screenplay element can be loaded only after a frame for it has been created");

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
            prepareCursor(cursor, SceneElement::Heading, !insertBlock);
            cursor.insertText(heading->text());
            insertBlock = true;
        }

        for(int j=0; j<scene->elementCount(); j++)
        {
            const SceneElement *para = scene->elementAt(j);
            if(insertBlock)
                cursor.insertBlock();

            QTextBlock block = cursor.block();
            block.setUserData(new ScreenplayParagraphBlockData(para));
            prepareCursor(cursor, para->type(), !insertBlock);
            cursor.insertText(para->text());
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
