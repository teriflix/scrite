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

// Cleanup TODO:
// Once upon a time, this class was used for both PDF/ODT generation and for
// in memory QTextDocument representation of the screenplay, for pagination.
// Now, its only used for PDF/ODT generation. So, we no longer need pagination
// code in this class.
// At some point, this whole class needs to be reviewed and trimmed of all
// unwanted fat.

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

class ProgressReport;
class ScreenplayTextDocument;
class AbstractScreenplayTextDocumentInjectionInterface
{
public:
    enum InjectLocation {
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
    void setScreenplayElement(const ScreenplayElement *element) { m_screenplayElement = element; }
    void setSceneElement(const SceneElement *element) { m_sceneElement = element; }

private:
    const ScreenplayElement *m_screenplayElement = nullptr;
    const SceneElement *m_sceneElement = nullptr;
};

#define AbstractScreenplayTextDocumentInjectionInterface_iid                                       \
    "io.scrite.AbstractScreenplayTextDocumentInjectionInterface"
Q_DECLARE_INTERFACE(AbstractScreenplayTextDocumentInjectionInterface,
                    AbstractScreenplayTextDocumentInjectionInterface_iid)

class QQmlEngine;
class ScreenplayTextDocumentUpdate;
class SceneElementBlockTextUpdater;

class ScreenplayTextDocument : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit ScreenplayTextDocument(QObject *parent = nullptr);
    explicit ScreenplayTextDocument(QTextDocument *document, QObject *parent = nullptr);
    ~ScreenplayTextDocument();

    static int headingFontPointSize(int headingLevel);

    // clang-format off
    Q_PROPERTY(QTextDocument *textDocument
               READ textDocument
               WRITE setTextDocument
               NOTIFY textDocumentChanged
               RESET resetTextDocument)
    // clang-format on
    void setTextDocument(QTextDocument *val);
    QTextDocument *textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    // clang-format off
    Q_PROPERTY(Screenplay *screenplay
               READ screenplay
               WRITE setScreenplay
               NOTIFY screenplayChanged
               RESET resetScreenplay)
    // clang-format on
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    // clang-format off
    Q_PROPERTY(ScreenplayFormat *formatting
               READ formatting
               WRITE setFormatting
               NOTIFY formattingChanged
               RESET resetFormatting)
    // clang-format on
    void setFormatting(ScreenplayFormat *val);
    ScreenplayFormat *formatting() const { return m_formatting; }
    Q_SIGNAL void formattingChanged();

    // clang-format off
    Q_PROPERTY(bool titlePage
               READ hasTitlePage
               WRITE setTitlePage
               NOTIFY titlePageChanged)
    // clang-format on
    void setTitlePage(bool val);
    bool hasTitlePage() const { return m_titlePage; }
    Q_SIGNAL void titlePageChanged();

    // clang-format off
    Q_PROPERTY(bool includeLoglineInTitlePage
               READ isIncludeLoglineInTitlePage
               WRITE setIncludeLoglineInTitlePage
               NOTIFY includeLoglineInTitlePageChanged)
    // clang-format on
    void setIncludeLoglineInTitlePage(bool val);
    bool isIncludeLoglineInTitlePage() const { return m_includeLoglineInTitlePage; }
    Q_SIGNAL void includeLoglineInTitlePageChanged();

    // clang-format off
    Q_PROPERTY(bool sceneNumbers
               READ hasSceneNumbers
               WRITE setSceneNumbers
               NOTIFY sceneNumbersChanged)
    // clang-format on
    void setSceneNumbers(bool val);
    bool hasSceneNumbers() const { return m_sceneNumbers; }
    Q_SIGNAL void sceneNumbersChanged();

    // clang-format off
    Q_PROPERTY(bool sceneIcons
               READ hasSceneIcons
               WRITE setSceneIcons
               NOTIFY sceneIconsChanged)
    // clang-format on
    void setSceneIcons(bool val);
    bool hasSceneIcons() const { return m_sceneIcons; }
    Q_SIGNAL void sceneIconsChanged();

    // clang-format off
    Q_PROPERTY(bool sceneColors
               READ hasSceneColors
               WRITE setSceneColors
               NOTIFY sceneColorsChanged)
    // clang-format on
    void setSceneColors(bool val);
    bool hasSceneColors() const { return m_sceneColors; }
    Q_SIGNAL void sceneColorsChanged();

    // clang-format off
    Q_PROPERTY(bool syncEnabled
               READ isSyncEnabled
               WRITE setSyncEnabled
               NOTIFY syncEnabledChanged)
    // clang-format on
    void setSyncEnabled(bool val);
    bool isSyncEnabled() const { return m_syncEnabled; }
    Q_SIGNAL void syncEnabledChanged();

    // clang-format off
    Q_PROPERTY(bool listSceneCharacters
               READ isListSceneCharacters
               WRITE setListSceneCharacters
               NOTIFY listSceneCharactersChanged)
    // clang-format on
    void setListSceneCharacters(bool val);
    bool isListSceneCharacters() const { return m_listSceneCharacters; }
    Q_SIGNAL void listSceneCharactersChanged();

    // clang-format off
    Q_PROPERTY(QStringList highlightDialoguesOf
               READ highlightDialoguesOf
               WRITE setHighlightDialoguesOf
               NOTIFY highlightDialoguesOfChanged)
    // clang-format on
    void setHighlightDialoguesOf(QStringList val);
    QStringList highlightDialoguesOf() const { return m_highlightDialoguesOf; }
    Q_SIGNAL void highlightDialoguesOfChanged();

    // clang-format off
    Q_PROPERTY(bool includeSceneSynopsis
               READ isIncludeSceneSynopsis
               WRITE setIncludeSceneSynopsis
               NOTIFY includeSceneSynopsisChanged)
    // clang-format on
    void setIncludeSceneSynopsis(bool val);
    bool isIncludeSceneSynopsis() const { return m_includeSceneSynopsis; }
    Q_SIGNAL void includeSceneSynopsisChanged();

    // clang-format off
    Q_PROPERTY(bool includeSceneFeaturedImage
               READ isIncludeSceneFeaturedImage
               WRITE setIncludeSceneFeaturedImage
               NOTIFY includeSceneFeaturedImageChanged)
    // clang-format on
    void setIncludeSceneFeaturedImage(bool val);
    bool isIncludeSceneFeaturedImage() const { return m_includeSceneFeaturedImage; }
    Q_SIGNAL void includeSceneFeaturedImageChanged();

    // clang-format off
    Q_PROPERTY(bool includeSceneComments
               READ isIncludeSceneComments
               WRITE setIncludeSceneComments
               NOTIFY includeSceneCommentsChanged)
    // clang-format on
    void setIncludeSceneComments(bool val);
    bool isIncludeSceneComments() const { return m_includeSceneComments; }
    Q_SIGNAL void includeSceneCommentsChanged();

    enum Purpose { ForDisplay, ForPrinting };
    Q_ENUM(Purpose)
    // clang-format off
    Q_PROPERTY(Purpose purpose
               READ purpose
               WRITE setPurpose
               NOTIFY purposeChanged)
    // clang-format on
    void setPurpose(Purpose val);
    Purpose purpose() const { return m_purpose; }
    Q_SIGNAL void purposeChanged();

    // clang-format off
    Q_PROPERTY(bool printEachSceneOnANewPage
               READ isPrintEachSceneOnANewPage
               WRITE setPrintEachSceneOnANewPage
               NOTIFY printEachSceneOnANewPageChanged)
    // clang-format on
    void setPrintEachSceneOnANewPage(bool val);
    bool isPrintEachSceneOnANewPage() const { return m_printEachSceneOnANewPage; }
    Q_SIGNAL void printEachSceneOnANewPageChanged();

    // clang-format off
    Q_PROPERTY(bool printEachActOnANewPage
               READ isPrintEachActOnANewPage
               WRITE setPrintEachActOnANewPage
               NOTIFY printEachActOnANewPageChanged)
    // clang-format on
    void setPrintEachActOnANewPage(bool val);
    bool isPrintEachActOnANewPage() const { return m_printEachActOnANewPage; }
    Q_SIGNAL void printEachActOnANewPageChanged();

    // clang-format off
    Q_PROPERTY(bool includeActBreaks
               READ isIncludeActBreaks
               WRITE setIncludeActBreaks
               NOTIFY includeActBreaksChanged)
    // clang-format on
    void setIncludeActBreaks(bool val);
    bool isIncludeActBreaks() const { return m_includeActBreaks; }
    Q_SIGNAL void includeActBreaksChanged();

    // clang-format off
    Q_PROPERTY(bool titlePageIsCentered
               READ isTitlePageIsCentered
               WRITE setTitlePageIsCentered
               NOTIFY titlePageIsCenteredChanged)
    // clang-format on
    void setTitlePageIsCentered(bool val);
    bool isTitlePageIsCentered() const { return m_titlePageIsCentered; }
    Q_SIGNAL void titlePageIsCenteredChanged();

    // NOTE: this property is referred only if this->purpose() == ForPrinting
    // clang-format off
    Q_PROPERTY(bool includeMoreAndContdMarkers
               READ isIncludeMoreAndContdMarkers
               WRITE setIncludeMoreAndContdMarkers
               NOTIFY includeMoreAndContdMarkersChanged)
    // clang-format on
    void setIncludeMoreAndContdMarkers(bool val);
    bool isIncludeMoreAndContdMarkers() const { return m_includeMoreAndContdMarkers; }
    Q_SIGNAL void includeMoreAndContdMarkersChanged();

    // clang-format off
    Q_PROPERTY(bool updating
               READ isUpdating
               NOTIFY updatingChanged)
    // clang-format on
    bool isUpdating() const { return m_updating; }
    Q_SIGNAL void updatingChanged();

    // clang-format off
    Q_PROPERTY(int pageCount
               READ pageCount
               NOTIFY pageCountChanged)
    // clang-format on
    int pageCount() const { return m_pageCount; }
    Q_SIGNAL void pageCountChanged();

    // clang-format off
    Q_PROPERTY(int currentPage
               READ currentPage
               NOTIFY currentPageChanged)
    // clang-format on
    int currentPage() const { return m_currentPage; }
    Q_SIGNAL void currentPageChanged();

    // clang-format off
    Q_PROPERTY(qreal currentPosition
               READ currentPosition
               NOTIFY currentPositionChanged)
    // clang-format on
    qreal currentPosition() const { return m_currentPosition; }
    Q_SIGNAL void currentPositionChanged();

    // clang-format off
    Q_PROPERTY(int secondsPerPage
               READ secondsPerPage
               WRITE setSecondsPerPage
               NOTIFY timePerPageChanged)
    // clang-format on
    void setSecondsPerPage(int val);
    int secondsPerPage() const;

    // clang-format off
    Q_PROPERTY(QTime timePerPage
               READ timePerPage
               WRITE setTimePerPage
               NOTIFY timePerPageChanged)
    // clang-format on
    void setTimePerPage(const QTime &val);
    QTime timePerPage() const { return m_timePerPage; }
    Q_SIGNAL void timePerPageChanged();

    // clang-format off
    Q_PROPERTY(QString timePerPageAsString
               READ timePerPageAsString
               NOTIFY timePerPageChanged)
    // clang-format on
    QString timePerPageAsString() const;

    // clang-format off
    Q_PROPERTY(QTime totalTime
               READ totalTime
               NOTIFY totalTimeChanged)
    // clang-format on
    QTime totalTime() const { return m_totalTime; }
    Q_SIGNAL void totalTimeChanged();

    // clang-format off
    Q_PROPERTY(QString totalTimeAsString
               READ totalTimeAsString
               NOTIFY totalTimeChanged)
    // clang-format on
    QString totalTimeAsString() const;

    // clang-format off
    Q_PROPERTY(QTime currentTime
               READ currentTime
               NOTIFY currentTimeChanged)
    // clang-format on
    QTime currentTime() const { return m_currentTime; }
    Q_SIGNAL void currentTimeChanged();

    // clang-format off
    Q_PROPERTY(QString currentTimeAsString
               READ currentTimeAsString
               NOTIFY currentTimeChanged)
    // clang-format on
    QString currentTimeAsString() const;

    Q_INVOKABLE void print(QObject *printerObject);

    QList<QPair<int, int>> pageBreaksFor(ScreenplayElement *element) const;

    QList<QPair<int, int>> pageBoundaries() const { return m_pageBoundaries; }
    Q_SIGNAL void pageBoundariesChanged();

    Q_INVOKABLE QTime lengthInTime(ScreenplayElement *from, ScreenplayElement *to) const;
    Q_INVOKABLE QString lengthInTimeAsString(ScreenplayElement *from, ScreenplayElement *to) const;
    Q_INVOKABLE qreal lengthInPixels(ScreenplayElement *from, ScreenplayElement *to) const;
    Q_INVOKABLE qreal lengthInPages(ScreenplayElement *from, ScreenplayElement *to) const;

    // clang-format off
    Q_PROPERTY(QObject *injection
               READ injection
               WRITE setInjection
               NOTIFY injectionChanged
               RESET resetInjection)
    // clang-format on
    void setInjection(QObject *val);
    QObject *injection() const { return m_injection; }
    Q_SIGNAL void injectionChanged();

    void syncNow(ProgressReport *progress = nullptr);

    Q_INVOKABLE void superImposeStructure(const QJsonObject &model);

    Q_INVOKABLE void reload();

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
    void onSceneOmitted(ScreenplayElement *element, int index);
    void onSceneIncluded(ScreenplayElement *element, int index);

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
    void evaluatePageBoundaries(bool revalCurrentPageAndPosition = true);
    void evaluatePageBoundariesLater();
    void formatAllBlocks();
    bool updateFromScreenplayElement(const ScreenplayElement *element);
    void loadScreenplayElement(const ScreenplayElement *element, QTextCursor &cursor);
    void formatBlock(const QTextBlock &block, const QString &text = QString());

    void removeTextFrame(const ScreenplayElement *element);
    void registerTextFrame(const ScreenplayElement *element, QTextFrame *frame);
    QTextFrame *findTextFrame(const ScreenplayElement *element) const;
    void onTextFrameDestroyed(QObject *object);
    void clearTextFrames();

    void addToSceneResetList(Scene *scene);
    void processSceneResetList();

    void resetInjection();

private:
    friend class SceneElementBlockTextUpdater;
    int m_pageCount = 0;
    bool m_updating = false;
    bool m_titlePage = false;
    bool m_includeLoglineInTitlePage = false;
    int m_currentPage = 0;
    bool m_sceneIcons = true;
    bool m_sceneColors = false;
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
    bool m_includeSceneFeaturedImage = false;
    bool m_includeSceneComments = false;
    bool m_screenplayIsBeingReset = false;
    bool m_includeMoreAndContdMarkers = true;
    QList<Scene *> m_sceneResetList;
    ExecLaterTimer m_sceneResetTimer;
    bool m_sceneResetHasTriggeredUpdateScheduled = false;
    bool m_printEachSceneOnANewPage = false;
    bool m_printEachActOnANewPage = false;
    bool m_includeActBreaks = false;
    ExecLaterTimer m_loadScreenplayTimer;
    QStringList m_highlightDialoguesOf;
    ExecLaterTimer m_pageBoundaryEvalTimer;
    QTextFrameFormat m_sceneFrameFormat;
    QObjectProperty<QObject> m_injection;
    bool m_connectedToScreenplaySignals = false;
    bool m_connectedToFormattingSignals = false;
    QPagedPaintDevice::PageSize m_paperSize = QPagedPaintDevice::Letter;
    QList<QPair<int, int>> m_pageBoundaries;
    QObjectProperty<Screenplay> m_screenplay;
    friend class ScreenplayTextDocumentUpdate;
    QObjectProperty<QTextDocument> m_textDocument;
    QObjectProperty<ScreenplayFormat> m_formatting;
    ModificationTracker m_screenplayModificationTracker;
    ModificationTracker m_formattingModificationTracker;
    QMap<QObject *, const ScreenplayElement *> m_frameElementMap;
    QMap<const ScreenplayElement *, QTextFrame *> m_elementFrameMap;
    ProgressReport *m_progressReport = nullptr;
};

class ScreenplayTitlePageObjectInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    explicit ScreenplayTitlePageObjectInterface(QObject *parent = nullptr);
    ~ScreenplayTitlePageObjectInterface();

    enum { Kind = QTextFormat::UserObject + 2 };
    enum Property { ScreenplayProperty = QTextFormat::UserProperty + 10, TitlePageIsCentered };

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument,
                    const QTextFormat &format);
};

class ScreenplayTextObjectInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    explicit ScreenplayTextObjectInterface(QObject *parent = nullptr);
    ~ScreenplayTextObjectInterface();

    enum { Kind = QTextFormat::UserObject + 1 };
    enum Type { SceneNumberType, MoreMarkerType, ContdMarkerType, SceneIconType };
    enum Property { TypeProperty = QTextFormat::UserProperty + 1, DataProperty };

    // QTextObjectInterface interface
    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument,
                    const QTextFormat &format);

private:
    void drawSceneNumber(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                         int posInDocument, const QTextFormat &format);
    void drawMoreMarker(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                        int posInDocument, const QTextFormat &format);
    void drawSceneIcon(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument,
                       const QTextFormat &format);
    void drawText(QPainter *painter, const QRectF &rect, const QString &text);
};

class SceneElementBlockTextUpdater : public QObject
{
    Q_OBJECT

public:
    static void completeOthers(SceneElementBlockTextUpdater *than);

    explicit SceneElementBlockTextUpdater(ScreenplayTextDocument *document, SceneElement *para);
    ~SceneElementBlockTextUpdater();

    void schedule();
    void abort();
    void timerEvent(QTimerEvent *event);
    void update();

private:
    QBasicTimer m_timer;
    QPointer<SceneElement> m_sceneElement;
    QPointer<ScreenplayTextDocument> m_document;
};

#endif // SCREENPLAYTEXTDOCUMENT_H
