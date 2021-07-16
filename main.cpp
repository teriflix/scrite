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

#include "application.h"

#include <QMenuBar>
#include <QShortcut>
#include <QUndoStack>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QOperatingSystemVersion>

#include "form.h"
#include "notes.h"
#include "fileinfo.h"
#include "undoredo.h"
#include "ruleritem.h"
#include "autoupdate.h"
#include "automation.h"
#include "trackobject.h"
#include "aggregation.h"
#include "eventfilter.h"
#include "timeprofiler.h"
#include "announcement.h"
#include "imageprinter.h"
#include "focustracker.h"
#include "notification.h"
#include "searchengine.h"
#include "propertyalias.h"
#include "standardpaths.h"
#include "urlattributes.h"
#include "textshapeitem.h"
#include "resetonchange.h"
#include "notebookmodel.h"
#include "scritedocument.h"
#include "shortcutsmodel.h"
#include "materialcolors.h"
#include "completionmodel.h"
#include "painterpathitem.h"
#include "transliteration.h"
#include "openfromlibrary.h"
#include "abstractexporter.h"
#include "textdocumentitem.h"
#include "simpletabbaritem.h"
#include "genericarraymodel.h"
#include "screenplayadapter.h"
#include "spellcheckservice.h"
#include "colorimageprovider.h"
#include "itempositionmapper.h"
#include "tabsequencemanager.h"
#include "gridbackgrounditem.h"
#include "notificationmanager.h"
#include "boundingboxevaluator.h"
#include "delayedpropertybinder.h"
#include "screenplaytextdocument.h"
#include "abstractreportgenerator.h"
#include "objectlistpropertymodel.h"
#include "qtextdocumentpagedprinter.h"
#include "characterrelationshipsgraph.h"
#include "screenplaytextdocumentoffsets.h"

void ScriteQtMessageHandler(QtMsgType type, const QMessageLogContext & context, const QString &message)
{
#ifdef QT_NO_DEBUG
    Q_UNUSED(type)
    Q_UNUSED(context)
    Q_UNUSED(message)
#else
    QString logMessage;

    QTextStream ts(&logMessage, QIODevice::WriteOnly);
    switch(type)
    {
    case QtDebugMsg: ts << "Debug: "; break;
    case QtWarningMsg: ts << "Warning: "; break;
    case QtCriticalMsg: ts << "Critical: "; break;
    case QtFatalMsg: ts << "Fatal: "; break;
    case QtInfoMsg: ts << "Info: "; break;
    }

    const char *where = context.function ? context.function : context.file;
    static const char *somewhere = "Somewhere";
    if(where == nullptr)
        where = somewhere;

    ts << "[" << where << " / " << context.line << "] - ";
    ts << message;
    ts.flush();

    fprintf(stderr, "%s\n", qPrintable(logMessage));
#endif
}

int main(int argc, char **argv)
{
    const QVersionNumber applicationVersion(0, 6, 9);
    Application::setApplicationName(QStringLiteral("Scrite"));
    Application::setOrganizationName(QStringLiteral("TERIFLIX"));
    Application::setOrganizationDomain(QStringLiteral("teriflix.com"));

#ifdef Q_OS_MAC
    Application::setApplicationVersion(applicationVersion.toString() + QStringLiteral("-beta"));
    if(QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSCatalina)
        qputenv("QT_MAC_WANTS_LAYER", QByteArrayLiteral("1"));
#else
    if(QSysInfo::WordSize == 32)
        Application::setApplicationVersion(applicationVersion.toString() + "-beta-x86");
    else
        Application::setApplicationVersion(applicationVersion.toString() + "-beta-x64");
#endif

#ifdef Q_OS_WIN
    // Maybe helps address https://www.github.com/teriflix/scrite/issues/247
    const QByteArray dpiMode = qgetenv("SCRITE_DPI_MODE");
    if( dpiMode == QByteArrayLiteral("HIGH_DPI") )
    {
        Application::setAttribute(Qt::AA_EnableHighDpiScaling);
        Application::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }
    else if( dpiMode == QByteArrayLiteral("96_DPI_ONLY") )
    {
        Application::setAttribute(Qt::AA_Use96Dpi);
        Application::setAttribute(Qt::AA_DisableHighDpiScaling);
    }
    else
        Application::setAttribute(Qt::AA_Use96Dpi);
    Application::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    qInstallMessageHandler(ScriteQtMessageHandler);

    Application a(argc, argv, applicationVersion);
    a.setWindowIcon(QIcon(QStringLiteral(":/images/appicon.png")));
    a.computeIdealFontPointSize();

    QPalette palette = Application::palette();
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor::fromRgbF(0,0.4,1));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(Qt::white));
    palette.setColor(QPalette::Active, QPalette::Text, QColor(Qt::black));
    Application::setPalette(palette);

    const char *scriteModuleUri = "Scrite";
    const QString apreason = QStringLiteral("Use as attached property.");
    const QString reason = QStringLiteral("Instantiation from QML not allowed.");

    qmlRegisterSingletonType<Aggregation>(scriteModuleUri, 1, 0, "Aggregation", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        return new Aggregation(engine);
    });

    qmlRegisterSingletonType<StandardPaths>(scriteModuleUri, 1, 0, "StandardPaths", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        return new StandardPaths(engine);
    });

#ifdef ENABLE_TIME_PROFILING
    qmlRegisterUncreatableType<ProfilerItem>(scriteModuleUri, 1, 0, "Profiler", apreason);
#endif

    qmlRegisterUncreatableType<ScriteDocument>(scriteModuleUri, 1, 0, "ScriteDocument", reason);

    qmlRegisterType<Scene>(scriteModuleUri, 1, 0, "Scene");
    qmlRegisterType<SceneGroup>(scriteModuleUri, 1, 0, "SceneGroup");
    qmlRegisterType<SceneSizeHintItem>(scriteModuleUri, 1, 0, "SceneSizeHint");
    qmlRegisterUncreatableType<SceneHeading>(scriteModuleUri, 1, 0, "SceneHeading", reason);
    qmlRegisterType<SceneElement>(scriteModuleUri, 1, 0, "SceneElement");

    qmlRegisterUncreatableType<Screenplay>(scriteModuleUri, 1, 0, "Screenplay", reason);
    qmlRegisterType<ScreenplayElement>(scriteModuleUri, 1, 0, "ScreenplayElement");
    qmlRegisterType<ScreenplayTracks>(scriteModuleUri, 1, 0, "ScreenplayTracks");

    qmlRegisterUncreatableType<Structure>(scriteModuleUri, 1, 0, "Structure", reason);
    qmlRegisterType<StructureElement>(scriteModuleUri, 1, 0, "StructureElement");
    qmlRegisterType<StructureElementConnector>(scriteModuleUri, 1, 0, "StructureElementConnector");
    qmlRegisterType<StructureCanvasViewportFilterModel>(scriteModuleUri, 1, 0, "StructureCanvasViewportFilterModel");
    qmlRegisterUncreatableType<StructureElementStack>(scriteModuleUri, 1, 0, "StructureElementStack", reason);
    qmlRegisterUncreatableType<StructureElementStacks>(scriteModuleUri, 1, 0, "StructureElementStacks", reason);

    qmlRegisterUncreatableType<Form>(scriteModuleUri, 1, 0, "Form", reason);
    qmlRegisterUncreatableType<Forms>(scriteModuleUri, 1, 0, "Forms", reason);
    qmlRegisterUncreatableType<FormQuestion>(scriteModuleUri, 1, 0, "FormQuestion", reason);

    qmlRegisterUncreatableType<Note>(scriteModuleUri, 1, 0, "Note", reason);
    qmlRegisterUncreatableType<Notes>(scriteModuleUri, 1, 0, "Notes", reason);
    qmlRegisterUncreatableType<Attachment>(scriteModuleUri, 1, 0, "Attachment", reason);
    qmlRegisterUncreatableType<Attachments>(scriteModuleUri, 1, 0, "Attachments", reason);
    qmlRegisterType<AttachmentsDropArea>(scriteModuleUri, 1, 0, "AttachmentsDropArea");
    qmlRegisterType<Relationship>(scriteModuleUri, 1, 0, "Relationship");
    qmlRegisterUncreatableType<Character>(scriteModuleUri, 1, 0, "Character", reason);
    qmlRegisterType<CharacterRelationshipsGraph>(scriteModuleUri, 1, 0, "CharacterRelationshipsGraph");

    qmlRegisterType<NotebookModel>(scriteModuleUri, 1, 0, "NotebookModel");
    qmlRegisterUncreatableType<BookmarkedNotes>(scriteModuleUri, 1, 0, "BookmarkedNotes", reason);

    qmlRegisterUncreatableType<ScriteDocument>(scriteModuleUri, 1, 0, "ScriteDocument", reason);
    qmlRegisterUncreatableType<ScreenplayFormat>(scriteModuleUri, 1, 0, "ScreenplayFormat", reason);
    qmlRegisterUncreatableType<SceneElementFormat>(scriteModuleUri, 1, 0, "SceneElementFormat", reason);
    qmlRegisterUncreatableType<ScreenplayPageLayout>(scriteModuleUri, 1, 0, "ScreenplayPageLayout", reason);

    qmlRegisterType<SceneDocumentBinder>(scriteModuleUri, 1, 0, "SceneDocumentBinder");
    qmlRegisterUncreatableType<TextFormat>(scriteModuleUri, 1, 0, "TextFormat", "Use the instance provided by SceneDocumentBinder.textFormat property.");

    qmlRegisterType<GridBackgroundItem>(scriteModuleUri, 1, 0, "GridBackground");
    qmlRegisterUncreatableType<GridBackgroundItemBorder>(scriteModuleUri, 1, 0, "GridBackgroundItemBorder", reason);
    qmlRegisterType<CompletionModel>(scriteModuleUri, 1, 0, "CompletionModel");

    qmlRegisterUncreatableType<EventFilterResult>(scriteModuleUri, 1, 0, "EventFilterResult", "Use the instance provided by EventFilter.onFilter signal.");
    qmlRegisterUncreatableType<EventFilter>(scriteModuleUri, 1, 0, "EventFilter", apreason);

    qmlRegisterType<PainterPathItem>(scriteModuleUri, 1, 0, "PainterPathItem");
    qmlRegisterUncreatableType<AbstractPathElement>(scriteModuleUri, 1, 0, "PathElement", "Use subclasses of AbstractPathElement.");
    qmlRegisterType<PainterPath>(scriteModuleUri, 1, 0, "PainterPath");
    qmlRegisterType<MoveToElement>(scriteModuleUri, 1, 0, "MoveTo");
    qmlRegisterType<LineToElement>(scriteModuleUri, 1, 0, "LineTo");
    qmlRegisterType<CloseSubpathElement>(scriteModuleUri, 1, 0, "CloseSubpath");
    qmlRegisterType<CubicToElement>(scriteModuleUri, 1, 0, "CubicTo");
    qmlRegisterType<QuadToElement>(scriteModuleUri, 1, 0, "QuadTo");
    qmlRegisterType<ArcToElement>(scriteModuleUri, 1, 0, "ArcTo");
    qmlRegisterType<TextShapeItem>(scriteModuleUri, 1, 0, "TextShapeItem");
    qmlRegisterType<UndoStack>(scriteModuleUri, 1, 0, "UndoStack");
    qmlRegisterType<UndoHandler>(scriteModuleUri, 1, 0, "UndoHandler");
    qmlRegisterUncreatableType<UndoResult>(scriteModuleUri, 1, 0, "UndoResult", "Use the instance provided by UndoHandler.onUndoRequest or UndoHandler.onReduRequest signal.");

    qmlRegisterType<SearchEngine>(scriteModuleUri, 1, 0, "SearchEngine");
    qmlRegisterType<TextDocumentSearch>(scriteModuleUri, 1, 0, "TextDocumentSearch");
    qmlRegisterUncreatableType<SearchAgent>(scriteModuleUri, 1, 0, "SearchAgent", apreason);

    qmlRegisterUncreatableType<Notification>(scriteModuleUri, 1, 0, "Notification", apreason);
    qmlRegisterUncreatableType<NotificationManager>(scriteModuleUri, 1, 0, "NotificationManager", "Use notificationManager instead.");

    qmlRegisterUncreatableType<ErrorReport>(scriteModuleUri, 1, 0, "ErrorReport", reason);
    qmlRegisterUncreatableType<ProgressReport>(scriteModuleUri, 1, 0, "ProgressReport", reason);

    qmlRegisterUncreatableType<TransliterationEngine>(scriteModuleUri, 1, 0, "TransliterationEngine", "Use app.transliterationEngine instead.");
    qmlRegisterUncreatableType<Transliterator>(scriteModuleUri, 1, 0, "Transliterator", apreason);
    qmlRegisterType<TransliteratedText>(scriteModuleUri, 1, 0, "TransliteratedText");

    qmlRegisterUncreatableType<AbstractExporter>(scriteModuleUri, 1, 0, "AbstractExporter", reason);
    qmlRegisterUncreatableType<AbstractReportGenerator>(scriteModuleUri, 1, 0, "AbstractReportGenerator", reason);

    qmlRegisterUncreatableType<FocusTracker>(scriteModuleUri, 1, 0, "FocusTracker", reason);
    qmlRegisterUncreatableType<FocusTrackerIndicator>(scriteModuleUri, 1, 0, "FocusTrackerIndicator", reason);

    qmlRegisterUncreatableType<Application>(scriteModuleUri, 1, 0, "Application", reason);
    qmlRegisterType<Annotation>(scriteModuleUri, 1, 0, "Annotation");
    qmlRegisterType<DelayedPropertyBinder>(scriteModuleUri, 1, 0, "DelayedPropertyBinder");
    qmlRegisterType<ResetOnChange>(scriteModuleUri, 1, 0, "ResetOnChange");

    qmlRegisterUncreatableType<HeaderFooter>(scriteModuleUri, 1, 0, "HeaderFooter", reason);
    qmlRegisterUncreatableType<QTextDocumentPagedPrinter>(scriteModuleUri, 1, 0, "QTextDocumentPagedPrinter", reason);

    qmlRegisterUncreatableType<AutoUpdate>(scriteModuleUri, 1, 0, "AutoUpdate", reason);

    qmlRegisterType<MaterialColors>(scriteModuleUri, 1, 0, "MaterialColors");

    qmlRegisterType<GenericArrayModel>(scriteModuleUri, 1, 0, "GenericArrayModel");
    qmlRegisterType<GenericArraySortFilterProxyModel>(scriteModuleUri, 1, 0, "GenericArraySortFilterProxyModel");

    qmlRegisterUncreatableType<AbstractObjectTracker>(scriteModuleUri, 1, 0, "AbstractTracker", reason);
    qmlRegisterType<TrackProperty>(scriteModuleUri, 1, 0, "TrackProperty");
    qmlRegisterType<TrackSignal>(scriteModuleUri, 1, 0, "TrackSignal");
    qmlRegisterType<TrackModelRow>(scriteModuleUri, 1, 0, "TrackModelRow");
    qmlRegisterType<TrackerPack>(scriteModuleUri, 1, 0, "TrackerPack");
    qmlRegisterType<PropertyAlias>(scriteModuleUri, 1, 0, "PropertyAlias");

    qmlRegisterType<ScreenplayAdapter>(scriteModuleUri, 1, 0, "ScreenplayAdapter");
    qmlRegisterType<ScreenplayTextDocument>(scriteModuleUri, 1, 0, "ScreenplayTextDocument");
    qmlRegisterType<ScreenplayElementPageBreaks>(scriteModuleUri, 1, 0, "ScreenplayElementPageBreaks");
    qmlRegisterType<ImagePrinter>(scriteModuleUri, 1, 0, "ImagePrinter");
    qmlRegisterType<TextDocumentItem>(scriteModuleUri, 1, 0, "TextDocumentItem");
    qmlRegisterType<ScreenplayTextDocumentOffsets>(scriteModuleUri, 1, 0, "ScreenplayTextDocumentOffsets");

    qmlRegisterType<RulerItem>(scriteModuleUri, 1, 0, "RulerItem");
    qmlRegisterType<SimpleTabBarItem>(scriteModuleUri, 1, 0, "SimpleTabBarItem");

    qmlRegisterType<SpellCheckService>(scriteModuleUri, 1, 0, "SpellCheckService");

    qmlRegisterType<BoundingBoxEvaluator>(scriteModuleUri, 1, 0, "BoundingBoxEvaluator");
    qmlRegisterType<BoundingBoxPreview>(scriteModuleUri, 1, 0, "BoundingBoxPreview");
    qmlRegisterUncreatableType<BoundingBoxItem>(scriteModuleUri, 1, 0, "BoundingBoxItem", apreason);

    qmlRegisterType<FileInfo>(scriteModuleUri, 1, 0, "FileInfo");

    qmlRegisterUncreatableType<ShortcutsModelItem>(scriteModuleUri, 1, 0, "ShortcutsModelItem", apreason);

    qmlRegisterType<LibraryService>(scriteModuleUri, 1, 0, "LibraryService");
    qmlRegisterUncreatableType<Library>(scriteModuleUri, 1, 0, "Library", "Use from LibraryService.library");

    qmlRegisterType<UrlAttributes>(scriteModuleUri, 1, 0, "UrlAttributes");

    qmlRegisterUncreatableType<QAbstractItemModel>(scriteModuleUri, 1, 0, "Model", "Base type of models (QAbstractItemModel)");

    qmlRegisterType<TabSequenceManager>(scriteModuleUri, 1, 0, "TabSequenceManager");
    qmlRegisterUncreatableType<TabSequenceItem>(scriteModuleUri, 1, 0, "TabSequenceItem", apreason);

    qmlRegisterUncreatableType<Announcement>(scriteModuleUri, 1, 0, "Announcement", apreason);

    qmlRegisterType<ItemPositionMapper>(scriteModuleUri, 1, 0, "ItemPositionMapper");

    qmlRegisterType<SortFilterObjectListModel>(scriteModuleUri, 1, 0, "SortFilterObjectListModel");

    NotificationManager notificationManager;

    DocumentFileSystem::setMarker( QByteArrayLiteral("SCRITE") );

    ShortcutsModel::instance()->setGroups( QStringList() << QStringLiteral("Application") <<
        QStringLiteral("Formatting") << QStringLiteral("Settings") << QStringLiteral("Language") <<
        QStringLiteral("File") << QStringLiteral("Edit") );

    ScriteDocument *scriteDocument = ScriteDocument::instance();

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    const QByteArray envOpenGLMultisampling = qgetenv("SCRITE_OPENGL_MULTISAMPLING").toUpper().trimmed();
    if(envOpenGLMultisampling == QByteArrayLiteral("FULL"))
        format.setSamples(4);
    else if(envOpenGLMultisampling == QByteArrayLiteral("EXTREME"))
        format.setSamples(8);
    else if(envOpenGLMultisampling == QByteArrayLiteral("NONE"))
        format.setSamples(-1);
    else
        format.setSamples(2); // default

    const QScreen *primaryScreen = a.primaryScreen();
    const QSize primaryScreenSize = primaryScreen->availableSize();

#ifdef Q_OS_MAC
    QMenuBar *menuBar = new QMenuBar(nullptr);
    QAction *quitAction = menuBar->addMenu("File")->addAction("Quit");
#endif

    QQuickStyle::setStyle("Material");

    QQuickView qmlView;
#ifdef Q_OS_MAC
    qmlView.setFlag(Qt::WindowFullscreenButtonHint); // [0.5.2 All] Full Screen Mode #194
#endif
    qmlView.setObjectName(QStringLiteral("ScriteQmlWindow"));
    qmlView.setFormat(format);
#ifdef Q_OS_WIN
    if( QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10 )
        qmlView.setSceneGraphBackend(QSGRendererInterface::Direct3D12);
    else
        qmlView.setSceneGraphBackend(QSGRendererInterface::Software);
#endif
    scriteDocument->formatting()->setSreeenFromWindow(&qmlView);
    scriteDocument->clearModified();
    a.initializeStandardColors(qmlView.engine());
    qmlView.setTitle(scriteDocument->documentWindowTitle());
    QObject::connect(scriteDocument, &ScriteDocument::documentWindowTitleChanged, &qmlView, &QQuickView::setTitle);
    qmlView.engine()->addImageProvider(QStringLiteral("color"), new ColorImageProvider);
    qmlView.engine()->addImageProvider(QStringLiteral("fileIcon"), new FileIconProvider);
    qmlView.engine()->rootContext()->setContextProperty("app", &a);
    qmlView.engine()->rootContext()->setContextProperty("qmlWindow", &qmlView);
    qmlView.engine()->rootContext()->setContextProperty("scriteDocument", scriteDocument);
    qmlView.engine()->rootContext()->setContextProperty("shortcutsModel", ShortcutsModel::instance());
    qmlView.engine()->rootContext()->setContextProperty("notificationManager", &notificationManager);

    QString fileNameToOpen;

#ifdef Q_OS_MAC
    if(!a.fileToOpen().isEmpty())
        fileNameToOpen = a.fileToOpen();
    a.setHandleFileOpenEvents(true);
#else
    if(a.arguments().size() > 1)
    {
        bool hasOptions = false;
        Q_FOREACH(QString arg, a.arguments())
        {
            if(arg.startsWith(QStringLiteral("--")))
            {
                hasOptions = true;
                break;
            }
        }

        if(!hasOptions)
        {
#ifdef Q_OS_WIN
            fileNameToOpen = a.arguments().last();
#else
            QStringList args = a.arguments();
            args.takeFirst();
            fileNameToOpen = args.join( QStringLiteral(" ") );
#endif
        }
    }
#endif

    qmlView.rootContext()->setContextProperty(QStringLiteral("fileNameToOpen"), fileNameToOpen);
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
    // qmlView.setTextRenderType(QQuickView::NativeTextRendering);
    Automation::init(&qmlView);
    qmlView.setSource(QUrl("qrc:/main.qml"));
    qmlView.setMinimumSize(QSize(qMin(600,primaryScreenSize.width()), qMin(375,primaryScreenSize.height())));

#if 0
    const QByteArray windowSize = qgetenv("SCRITE_WINDOW_SIZE");
    if(windowSize.isEmpty())
        qmlView.showMaximized();
    else
    {
        QTextStream ts(windowSize);
        int width = 0, height = 0;
        ts >> width >> height;
        if(width == 0 || height == 0)
            qmlView.showFullScreen();
        else
        {
            qmlView.resize(width, height);
            qmlView.show();
        }
    }
#else
    qmlView.show();
#endif
    qmlView.raise();

#ifdef Q_OS_MAC
    QObject::connect(quitAction, &QAction::triggered, &qmlView, &QQuickView::close);
#endif

    QObject::connect(&a, &Application::minimizeWindowRequest, &qmlView, &QQuickView::showMinimized);

    return a.exec();
}

