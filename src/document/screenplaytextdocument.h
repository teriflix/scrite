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

#ifndef SCREENPLAYTEXTDOCUMENT_H
#define SCREENPLAYTEXTDOCUMENT_H

#include <QTime>
#include <QtMath>
#include <QTextDocument>
#include <QQmlParserStatus>
#include <QPagedPaintDevice>
#include <QQuickTextDocument>
#include <QSequentialAnimationGroup>
#include <QAbstractTextDocumentLayout>

#include "scene.h"
#include "formatting.h"
#include "screenplay.h"
#include "qobjectproperty.h"

class ScreenplayTextDocument;
class AbstractScreenplayTextDocumentInjectionInterface
{
public:
    enum InjectLocation
    {
        AfterTitlePage,
        AfterSceneHeading,
        AfterLastScene,
        BeforeSceneElement,
        AfterSceneElement
    };
    virtual void inject(QTextCursor &, InjectLocation) { }
    virtual bool filterSceneElement() const { return false; }

    const ScreenplayElement *screenplayElement() const { return m_screenplayElement; }
    const SceneElement *sceneElement() const { return m_sceneElement; }

private:
    friend class ScreenplayTextDocument;
    void setScreenplayElement(const ScreenplayElement *element) {
        m_screenplayElement = element;
    }
    void setSceneElement(const SceneElement *element) {
        m_sceneElement = element;
    }

private:
    const ScreenplayElement *m_screenplayElement = nullptr;
    const SceneElement *m_sceneElement = nullptr;
};

#define AbstractScreenplayTextDocumentInjectionInterface_iid "io.scrite.AbstractScreenplayTextDocumentInjectionInterface"
Q_DECLARE_INTERFACE(AbstractScreenplayTextDocumentInjectionInterface, AbstractScreenplayTextDocumentInjectionInterface_iid)

class QQmlEngine;
class ScreenplayTextDocumentUpdate;
class ScreenplayTextDocument : public QObject,
                               public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    ScreenplayTextDocument(QObject *parent=nullptr);
    ScreenplayTextDocument(QTextDocument *document, QObject *parent=nullptr);
    ~ScreenplayTextDocument();

    Q_PROPERTY(QTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged RESET resetTextDocument)
    void setTextDocument(QTextDocument* val);
    QTextDocument* textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged RESET resetScreenplay)
    void setScreenplay(Screenplay* val);
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(ScreenplayFormat* formatting READ formatting WRITE setFormatting NOTIFY formattingChanged RESET resetFormatting)
    void setFormatting(ScreenplayFormat* val);
    ScreenplayFormat* formatting() const { return m_formatting; }
    Q_SIGNAL void formattingChanged();

    Q_PROPERTY(bool titlePage READ hasTitlePage WRITE setTitlePage NOTIFY titlePageChanged)
    void setTitlePage(bool val);
    bool hasTitlePage() const { return m_titlePage; }
    Q_SIGNAL void titlePageChanged();

    Q_PROPERTY(bool sceneNumbers READ hasSceneNumbers WRITE setSceneNumbers NOTIFY sceneNumbersChanged)
    void setSceneNumbers(bool val);
    bool hasSceneNumbers() const { return m_sceneNumbers; }
    Q_SIGNAL void sceneNumbersChanged();

    Q_PROPERTY(bool sceneIcons READ hasSceneIcons WRITE setSceneIcons NOTIFY sceneIconsChanged)
    void setSceneIcons(bool val);
    bool hasSceneIcons() const { return m_sceneIcons; }
    Q_SIGNAL void sceneIconsChanged();

    Q_PROPERTY(bool syncEnabled READ isSyncEnabled WRITE setSyncEnabled NOTIFY syncEnabledChanged)
    void setSyncEnabled(bool val);
    bool isSyncEnabled() const { return m_syncEnabled; }
    Q_SIGNAL void syncEnabledChanged();

    Q_PROPERTY(bool listSceneCharacters READ isListSceneCharacters WRITE setListSceneCharacters NOTIFY listSceneCharactersChanged)
    void setListSceneCharacters(bool val);
    bool isListSceneCharacters() const { return m_listSceneCharacters; }
    Q_SIGNAL void listSceneCharactersChanged();

    Q_PROPERTY(QStringList highlightDialoguesOf READ highlightDialoguesOf WRITE setHighlightDialoguesOf NOTIFY highlightDialoguesOfChanged)
    void setHighlightDialoguesOf(QStringList val);
    QStringList highlightDialoguesOf() const { return m_highlightDialoguesOf; }
    Q_SIGNAL void highlightDialoguesOfChanged();

    Q_PROPERTY(bool includeSceneSynopsis READ isIncludeSceneSynopsis WRITE setIncludeSceneSynopsis NOTIFY includeSceneSynopsisChanged)
    void setIncludeSceneSynopsis(bool val);
    bool isIncludeSceneSynopsis() const { return m_includeSceneSynopsis; }
    Q_SIGNAL void includeSceneSynopsisChanged();

    enum Purpose { ForDisplay, ForPrinting };
    Q_ENUM(Purpose)
    Q_PROPERTY(Purpose purpose READ purpose WRITE setPurpose NOTIFY purposeChanged)
    void setPurpose(Purpose val);
    Purpose purpose() const { return m_purpose; }
    Q_SIGNAL void purposeChanged();

    Q_PROPERTY(bool printEachSceneOnANewPage READ isPrintEachSceneOnANewPage WRITE setPrintEachSceneOnANewPage NOTIFY printEachSceneOnANewPageChanged)
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    Q_PROPERTY(bool titlePageIsCentered READ isTitlePageIsCentered WRITE setTitlePageIsCentered NOTIFY titlePageIsCenteredChanged)
    void setTitlePageIsCentered(bool val);
    bool isTitlePageIsCentered() const { return m_titlePageIsCentered; }
    Q_SIGNAL void titlePageIsCenteredChanged();

    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)
    bool isUpdating() const { return m_updating; }
    Q_SIGNAL void updatingChanged();

    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    int pageCount() const { return m_pageCount; }
    Q_SIGNAL void pageCountChanged();

    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    int currentPage() const { return m_currentPage; }
    Q_SIGNAL void currentPageChanged();

    Q_PROPERTY(qreal currentPosition READ currentPosition NOTIFY currentPositionChanged)
    qreal currentPosition() const { return m_currentPosition; }
    Q_SIGNAL void currentPositionChanged();

    Q_PROPERTY(int secondsPerPage READ secondsPerPage WRITE setSecondsPerPage NOTIFY timePerPageChanged)
    void setSecondsPerPage(int val);
    int secondsPerPage() const;

    Q_PROPERTY(QTime timePerPage READ timePerPage WRITE setTimePerPage NOTIFY timePerPageChanged)
    void setTimePerPage(const QTime &val);
    QTime timePerPage() const { return m_timePerPage; }
    Q_SIGNAL void timePerPageChanged();

    Q_PROPERTY(QString timePerPageAsString READ timePerPageAsString NOTIFY timePerPageChanged)
    QString timePerPageAsString() const;

    Q_PROPERTY(QTime totalTime READ totalTime NOTIFY totalTimeChanged)
    QTime totalTime() const { return m_totalTime; }
    Q_SIGNAL void totalTimeChanged();

    Q_PROPERTY(QString totalTimeAsString READ totalTimeAsString NOTIFY totalTimeChanged)
    QString totalTimeAsString() const;

    Q_PROPERTY(QTime currentTime READ currentTime NOTIFY currentTimeChanged)
    QTime currentTime() const { return m_currentTime; }
    Q_SIGNAL void currentTimeChanged();

    Q_PROPERTY(QString currentTimeAsString READ currentTimeAsString NOTIFY currentTimeChanged)
    QString currentTimeAsString() const;

    Q_INVOKABLE void print(QObject *printerObject);

    QList< QPair<int,int> > pageBreaksFor(ScreenplayElement *element) const;

    QList< QPair<int,int> > pageBoundaries() const { return m_pageBoundaries; }
    Q_SIGNAL void pageBoundariesChanged();

    Q_INVOKABLE qreal lengthInPixels(ScreenplayElement *element) const;
    Q_INVOKABLE qreal lengthInPages(ScreenplayElement *element) const;

    Q_PROPERTY(QObject* injection READ injection WRITE setInjection NOTIFY injectionChanged RESET resetInjection)
    void setInjection(QObject* val);
    QObject* injection() const { return m_injection; }
    Q_SIGNAL void injectionChanged();

    void syncNow();

signals:
    void updateScheduled();
    void updateStarted();
    void updateFinished();

protected:
    // QQmlParserStatus implementation
    void classBegin();
    void componentComplete();

    // QObject interface
    void timerEvent(QTimerEvent *event);

private:
    void init();
    void setUpdating(bool val);
    void setPageCount(qreal val);
    void setCurrentPageAndPosition(int page, qreal pos);
    void resetFormatting();
    void resetTextDocument();
    void resetQQTextDocument();

    void loadScreenplay();
    void includeMoreAndContdMarkers();
    void loadScreenplayLater();
    void resetScreenplay();

    void connectToScreenplaySignals();
    void connectToScreenplayFormatSignals();

    void disconnectFromScreenplaySignals();
    void disconnectFromScreenplayFormatSignals();

    void connectToSceneSignals(Scene *scene);
    void disconnectFromSceneSignals(Scene *scene);

    // When screenplay is being cleared
    void onScreenplayAboutToReset();
    void onScreenplayReset();

    // Hook to signals that convey change in sequencing of scenes
    void onSceneMoved(ScreenplayElement *ptr, int from, int to);
    void onSceneRemoved(ScreenplayElement *ptr, int index);
    void onSceneInserted(ScreenplayElement *element, int index);

    // Hook to signals that convey changes to a specific scene content
    void onSceneReset();
    void onSceneRefreshed();
    void onSceneAboutToReset();
    void onSceneHeadingChanged();
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    void onSceneAboutToResetModel();
    void onSceneResetModel();

    // Hook to signals that convey change in formatting
    void onElementFormatChanged();
    void onDefaultFontChanged();
    void onFormatScreenChanged();
    void onFormatFontPointSizeDeltaChanged();

    // Hook to signals to know current element and cursor position,
    // so that we can report current page number.
    void onActiveSceneChanged();
    void onActiveSceneDestroyed(Scene *ptr);
    void onActiveSceneCursorPositionChanged();

    // Other methods
    void evaluateCurrentPageAndPosition();
    void evaluatePageBoundaries();
    void evaluatePageBoundariesLater();
    void formatAllBlocks();
    void loadScreenplayElement(const ScreenplayElement *element, QTextCursor &cursor);
    void formatBlock(const QTextBlock &block, const QString &text=QString());

    void removeTextFrame(const ScreenplayElement *element);
    void registerTextFrame(const ScreenplayElement *element, QTextFrame *frame);
    QTextFrame *findTextFrame(const ScreenplayElement *element) const;
    void onTextFrameDestroyed(QObject *object);
    void clearTextFrames();

    void addToSceneResetList(Scene *scene);
    void processSceneResetList();

    void resetInjection();

private:
    int m_pageCount = 0;
    bool m_updating = false;
    bool m_titlePage = false;
    int m_currentPage = 0;
    bool m_sceneIcons = true;
    Purpose m_purpose = ForDisplay;
    QTime m_totalTime = QTime(0, 0, 0);
    bool m_syncEnabled = true;
    QTime m_timePerPage = QTime(0, 1, 0);
    QTime m_currentTime = QTime(0, 0, 0);
    bool m_sceneNumbers = true;
    Scene *m_activeScene = nullptr;
    qreal m_currentPosition = 0;
    bool m_componentComplete = true;
    bool m_titlePageIsCentered = true;
    bool m_listSceneCharacters = false;
    bool m_includeSceneSynopsis = false;
    bool m_screenplayIsBeingReset = false;
    QList<Scene*> m_sceneResetList;
    ExecLaterTimer m_sceneResetTimer;
    bool m_printEachSceneOnANewPage = false;
    ExecLaterTimer m_loadScreenplayTimer;
    QStringList m_highlightDialoguesOf;
    ExecLaterTimer m_pageBoundaryEvalTimer;
    QTextFrameFormat m_sceneFrameFormat;
    QObjectProperty<QObject> m_injection;
    bool m_connectedToScreenplaySignals = false;
    bool m_connectedToFormattingSignals = false;
    QPagedPaintDevice::PageSize m_paperSize = QPagedPaintDevice::Letter;
    QList< QPair<int,int> > m_pageBoundaries;
    QObjectProperty<Screenplay> m_screenplay;
    friend class ScreenplayTextDocumentUpdate;
    QObjectProperty<QTextDocument> m_textDocument;
    QObjectProperty<ScreenplayFormat> m_formatting;
    ModificationTracker m_screenplayModificationTracker;
    ModificationTracker m_formattingModificationTracker;
    QMap<QObject *, const ScreenplayElement*> m_frameElementMap;
    QMap<const ScreenplayElement*, QTextFrame*> m_elementFrameMap;
};

class ScreenplayElementPageBreaks : public QObject
{
    Q_OBJECT

public:
    ScreenplayElementPageBreaks(QObject *parent=nullptr);
    ~ScreenplayElementPageBreaks();

    Q_PROPERTY(ScreenplayTextDocument* screenplayDocument READ screenplayDocument WRITE setScreenplayDocument NOTIFY screenplayDocumentChanged RESET resetScreenplayDocument)
    void setScreenplayDocument(ScreenplayTextDocument* val);
    ScreenplayTextDocument* screenplayDocument() const { return m_screenplayDocument; }
    Q_SIGNAL void screenplayDocumentChanged();

    Q_PROPERTY(ScreenplayElement* screenplayElement READ screenplayElement WRITE setScreenplayElement NOTIFY screenplayElementChanged RESET resetScreenplayElement)
    void setScreenplayElement(ScreenplayElement* val);
    ScreenplayElement* screenplayElement() const { return m_screenplayElement; }
    Q_SIGNAL void screenplayElementChanged();

    Q_PROPERTY(QVariantList pageBreaks READ pageBreaks NOTIFY pageBreaksChanged)
    QVariantList pageBreaks() const { return m_pageBreaks; }
    Q_SIGNAL void pageBreaksChanged();

private:
    void resetScreenplayDocument();
    void resetScreenplayElement();
    void updatePageBreaks();
    void setPageBreaks(const QVariantList &val);

private:
    QVariantList m_pageBreaks;
    QObjectProperty<ScreenplayElement> m_screenplayElement;
    QObjectProperty<ScreenplayTextDocument> m_screenplayDocument;
};

class ScreenplayTitlePageObjectInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    ScreenplayTitlePageObjectInterface(QObject *parent=nullptr);
    ~ScreenplayTitlePageObjectInterface();

    enum { Kind=QTextFormat::UserObject+2 };
    enum Property
    {
        ScreenplayProperty = QTextFormat::UserProperty+10,
        TitlePageIsCentered
    };

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);
};

class ScreenplayTextObjectInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    ScreenplayTextObjectInterface(QObject *parent=nullptr);
    ~ScreenplayTextObjectInterface();

    enum { Kind=QTextFormat::UserObject+1 };
    enum Type { SceneNumberType, MoreMarkerType, ContdMarkerType, SceneIconType };
    enum Property
    {
        TypeProperty = QTextFormat::UserProperty+1,
        DataProperty
    };

    // QTextObjectInterface interface
    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);

private:
    void drawSceneNumber(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawMoreMarker(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawSceneIcon(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawText(QPainter *painter, const QRectF &rect, const QString &text);
};

class PrintedTextDocumentOffsets : public QAbstractListModel
{
    Q_OBJECT

public:
    PrintedTextDocumentOffsets(QObject *parent=nullptr);
    ~PrintedTextDocumentOffsets();

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    void setFileName(const QString &val);
    QString fileName() const { return m_fileName; }
    Q_SIGNAL void fileNameChanged();

    Q_INVOKABLE QString fileNameFrom(const QString &mediaFileNameOrUrl) const;

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged)
    void setScreenplay(Screenplay* val);
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    enum Type { PageOffsets, SceneOffsets };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(QTime timePerPage READ timePerPage WRITE setTimePerPage NOTIFY timePerPageChanged)
    void setTimePerPage(const QTime &val);
    QTime timePerPage() const { return m_timePerPage; }
    Q_SIGNAL void timePerPageChanged();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_offsets.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE QJsonObject offsetInfoAt(int index) const;
    Q_INVOKABLE QJsonObject offsetInfoOf(const QVariant &pageOrSceneNumber) const;
    Q_INVOKABLE QJsonObject nearestOffsetInfo(int pageNumber, qreal yOffset) const;

    Q_INVOKABLE QJsonObject offsetInfoAtTime(const QTime &time, int from=0) const;
    Q_INVOKABLE QJsonObject offsetInfoAtTimeInMillisecond(int ms, int from=0) const;

    Q_INVOKABLE void setTime(int row, const QTime &time, bool adjustFollowingRows);
    Q_INVOKABLE void setTimeInMillisecond(int row, int ms, bool adjustFollowingRows);
    Q_INVOKABLE void resetTime();

    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    QString errorMessage() const { return m_errorMessage; }
    Q_SIGNAL void errorMessageChanged();

    Q_PROPERTY(bool hasError READ hasError NOTIFY errorMessageChanged)
    bool hasError() const { return !m_errorMessage.isEmpty(); }

    Q_INVOKABLE void clearErrorMessage() { this->setErrorMessage(QString()); }

    // QAbstractItemModel interface
    enum Role { ModelDataRole=Qt::UserRole, OffsetInfoRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void setErrorMessage(const QString &val);

    void syncOffsets();
    void loadOffsets();
    void saveOffsets();

private:
    qreal m_pageScale = 1.0;
    struct _OffsetInfo
    {
        _OffsetInfo() { }
        int rowNumber = -1;
        int pageNumber = -1;
        QRectF pageRect;
        QTime pageTime;
        int sceneIndex = -1;
        QString sceneNumber;
        QString sceneHeading;
        QRectF sceneHeadingRect;
        QTime sceneTime;
        void computeTimes(const QTime &timePerPage);
        QJsonObject toJsonObject() const;
    };
    QList<_OffsetInfo> m_offsets;

    QString m_fileName;
    bool m_enabled = false;
    int m_currentPageNumber = -1;
    QString m_errorMessage;
    QRectF m_currentPageRect;
    Type m_type = SceneOffsets;
    QTime m_timePerPage = QTime(0, 1, 0);
    QObjectProperty<Screenplay> m_screenplay;
};

class PageScrollAnimation : public QSequentialAnimationGroup
{
    Q_OBJECT

public:
    PageScrollAnimation(QObject *parent=nullptr);
    ~PageScrollAnimation();

    Q_PROPERTY(QRectF pageRect READ pageRect WRITE setPageRect NOTIFY pageRectChanged)
    void setPageRect(const QRectF &val);
    QRectF pageRect() const { return m_pageRect; }
    Q_SIGNAL void pageRectChanged();

    Q_PROPERTY(QRectF contentRect READ contentRect WRITE setContentRect NOTIFY contentRectChanged)
    void setContentRect(const QRectF &val);
    QRectF contentRect() const { return m_contentRect; }
    Q_SIGNAL void contentRectChanged();

    Q_PROPERTY(qreal pageScale READ pageScale WRITE setPageScale NOTIFY pageScaleChanged)
    void setPageScale(qreal val);
    qreal pageScale() const { return m_pageScale; }
    Q_SIGNAL void pageScaleChanged();

    Q_PROPERTY(qreal pageSpacing READ pageSpacing WRITE setPageSpacing NOTIFY pageSpacingChanged)
    void setPageSpacing(const qreal &val);
    qreal pageSpacing() const { return m_pageSpacing; }
    Q_SIGNAL void pageSpacingChanged();

    Q_PROPERTY(QRectF viewportRect READ viewportRect WRITE setViewportRect NOTIFY viewportRectChanged)
    void setViewportRect(const QRectF &val);
    QRectF viewportRect() const { return m_viewportRect; }
    Q_SIGNAL void viewportRectChanged();

    Q_PROPERTY(int fromPage READ fromPage WRITE setFromPage NOTIFY fromPageChanged)
    void setFromPage(int val);
    int fromPage() const { return m_fromPage; }
    Q_SIGNAL void fromPageChanged();

    Q_PROPERTY(int toPage READ toPage WRITE setToPage NOTIFY toPageChanged)
    void setToPage(int val);
    int toPage() const { return m_toPage; }
    Q_SIGNAL void toPageChanged();

    Q_PROPERTY(qreal fromY READ fromY WRITE setFromY NOTIFY fromYChanged)
    void setFromY(qreal val);
    qreal fromY() const { return m_fromY; }
    Q_SIGNAL void fromYChanged();

    Q_PROPERTY(qreal toY READ toY WRITE setToY NOTIFY toYChanged)
    void setToY(qreal val);
    qreal toY() const { return m_toY; }
    Q_SIGNAL void toYChanged();

    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    void setDuration(int val);
    int duration() const { return m_duration; }
    Q_SIGNAL void durationChanged();

    Q_PROPERTY(int pageSkipDuration READ pageSkipDuration WRITE setPageSkipDuration NOTIFY pageSkipDurationChanged)
    void setPageSkipDuration(int val);
    int pageSkipDuration() const { return m_pageSkipDuration; }
    Q_SIGNAL void pageSkipDurationChanged();

    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged)
    void setTarget(QObject* val);
    QObject* target() const { return m_target; }
    Q_SIGNAL void targetChanged();

    Q_PROPERTY(QByteArray propertyName READ propertyName WRITE setPropertyName NOTIFY propertyNameChanged)
    void setPropertyName(const QByteArray &val);
    QByteArray propertyName() const { return m_propertyName; }
    Q_SIGNAL void propertyNameChanged();

    Q_INVOKABLE void setupNow();

signals:
    void setupRequired();

protected:
    void timerEvent(QTimerEvent *te);

private:
    void setupAnimation();
    void setupAnimationLater();

private:
    qreal m_y = 0;
    qreal m_toY = 0;
    int m_toPage = 0;
    qreal m_fromY = 0;
    int m_duration = 1000;
    int m_fromPage = 0;
    bool m_running = false;
    qreal m_pageScale = 1;
    QRectF m_pageRect;
    QRectF m_contentRect;
    qreal m_pageSpacing = 0;
    QRectF m_viewportRect;
    int m_pageSkipDuration = 50;
    QByteArray m_propertyName;
    ExecLaterTimer m_setupTimer;
    QObjectProperty<QObject> m_target;
};

#endif // SCREENPLAYTEXTDOCUMENT_H
