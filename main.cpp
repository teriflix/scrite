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

#include "fileinfo.h"
#include "undoredo.h"
#include "completer.h"
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
#include "standardpaths.h"
#include "urlattributes.h"
#include "textshapeitem.h"
#include "resetonchange.h"
#include "scritedocument.h"
#include "shortcutsmodel.h"
#include "materialcolors.h"
#include "painterpathitem.h"
#include "transliteration.h"
#include "openfromlibrary.h"
#include "abstractexporter.h"
#include "textdocumentitem.h"
#include "notebooktabmodel.h"
#include "genericarraymodel.h"
#include "screenplayadapter.h"
#include "spellcheckservice.h"
#include "colorimageprovider.h"
#include "tabsequencemanager.h"
#include "gridbackgrounditem.h"
#include "notificationmanager.h"
#include "boundingboxevaluator.h"
#include "delayedpropertybinder.h"
#include "screenplaytextdocument.h"
#include "abstractreportgenerator.h"
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
    const QVersionNumber applicationVersion(0, 5, 8);
    Application::setApplicationName("Scrite");
    Application::setOrganizationName("TERIFLIX");
    Application::setOrganizationDomain("teriflix.com");

#ifdef Q_OS_MAC
    Application::setApplicationVersion(applicationVersion.toString() + "-beta");
    if(QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSCatalina)
        qputenv("QT_MAC_WANTS_LAYER", QByteArrayLiteral("1"));
#else
    if(QSysInfo::WordSize == 32)
        Application::setApplicationVersion(applicationVersion.toString() + "-beta-x86");
    else
        Application::setApplicationVersion(applicationVersion.toString() + "-beta-x64");
#endif

#ifdef Q_OS_WIN
    Application::setAttribute(Qt::AA_Use96Dpi);
    Application::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    qInstallMessageHandler(ScriteQtMessageHandler);

    Application a(argc, argv, applicationVersion);
    a.setWindowIcon(QIcon(":/images/appicon.png"));
    a.computeIdealFontPointSize();

    QPalette palette = Application::palette();
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor::fromRgbF(0,0.4,1));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor("white"));
    palette.setColor(QPalette::Active, QPalette::Text, QColor("black"));
    Application::setPalette(palette);

    qmlRegisterSingletonType<Aggregation>("Scrite", 1, 0, "Aggregation", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        return new Aggregation(engine);
    });

    qmlRegisterSingletonType<StandardPaths>("Scrite", 1, 0, "StandardPaths", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        return new StandardPaths(engine);
    });

    const QString apreason("Use as attached property.");
    const QString reason("Instantiation from QML not allowed.");

#ifdef ENABLE_TIME_PROFILING
    qmlRegisterUncreatableType<ProfilerItem>("Scrite", 1, 0, "Profiler", apreason);
#endif

    qmlRegisterUncreatableType<ScriteDocument>("Scrite", 1, 0, "ScriteDocument", reason);

    qmlRegisterType<Scene>("Scrite", 1, 0, "Scene");
    qmlRegisterType<SceneSizeHintItem>("Scrite", 1, 0, "SceneSizeHint");
    qmlRegisterUncreatableType<SceneHeading>("Scrite", 1, 0, "SceneHeading", reason);
    qmlRegisterType<SceneElement>("Scrite", 1, 0, "SceneElement");

    qmlRegisterUncreatableType<Screenplay>("Scrite", 1, 0, "Screenplay", reason);
    qmlRegisterType<ScreenplayElement>("Scrite", 1, 0, "ScreenplayElement");

    qmlRegisterUncreatableType<Structure>("Scrite", 1, 0, "Structure", reason);
    qmlRegisterType<StructureElement>("Scrite", 1, 0, "StructureElement");
    qmlRegisterType<StructureElementConnector>("Scrite", 1, 0, "StructureElementConnector");
    qmlRegisterType<StructureCanvasViewportFilterModel>("Scrite", 1, 0, "StructureCanvasViewportFilterModel");

    qmlRegisterType<Note>("Scrite", 1, 0, "Note");
    qmlRegisterType<Relationship>("Scrite", 1, 0, "Relationship");
    qmlRegisterUncreatableType<Character>("Scrite", 1, 0, "Character", reason);
    qmlRegisterType<CharacterRelationshipsGraph>("Scrite", 1, 0, "CharacterRelationshipsGraph");

    qmlRegisterUncreatableType<ScriteDocument>("Scrite", 1, 0, "ScriteDocument", reason);
    qmlRegisterUncreatableType<ScreenplayFormat>("Scrite", 1, 0, "ScreenplayFormat", reason);
    qmlRegisterUncreatableType<SceneElementFormat>("Scrite", 1, 0, "SceneElementFormat", reason);
    qmlRegisterUncreatableType<ScreenplayPageLayout>("Scrite", 1, 0, "ScreenplayPageLayout", reason);

    qmlRegisterType<SceneDocumentBinder>("Scrite", 1, 0, "SceneDocumentBinder");
    qmlRegisterUncreatableType<TextFormat>("Scrite", 1, 0, "TextFormat", "Use the instance provided by SceneDocumentBinder.textFormat property.");

    qmlRegisterType<GridBackgroundItem>("Scrite", 1, 0, "GridBackground");
    qmlRegisterUncreatableType<GridBackgroundItemBorder>("Scrite", 1, 0, "GridBackgroundItemBorder", reason);
    qmlRegisterType<Completer>("Scrite", 1, 0, "Completer");

    qmlRegisterUncreatableType<EventFilterResult>("Scrite", 1, 0, "EventFilterResult", "Use the instance provided by EventFilter.onFilter signal.");
    qmlRegisterUncreatableType<EventFilter>("Scrite", 1, 0, "EventFilter", apreason);

    qmlRegisterType<PainterPathItem>("Scrite", 1, 0, "PainterPathItem");
    qmlRegisterUncreatableType<AbstractPathElement>("Scrite", 1, 0, "PathElement", "Use subclasses of AbstractPathElement.");
    qmlRegisterType<PainterPath>("Scrite", 1, 0, "PainterPath");
    qmlRegisterType<MoveToElement>("Scrite", 1, 0, "MoveTo");
    qmlRegisterType<LineToElement>("Scrite", 1, 0, "LineTo");
    qmlRegisterType<CloseSubpathElement>("Scrite", 1, 0, "CloseSubpath");
    qmlRegisterType<CubicToElement>("Scrite", 1, 0, "CubicTo");
    qmlRegisterType<QuadToElement>("Scrite", 1, 0, "QuadTo");
    qmlRegisterType<ArcToElement>("Scrite", 1, 0, "ArcTo");
    qmlRegisterType<TextShapeItem>("Scrite", 1, 0, "TextShapeItem");
    qmlRegisterType<UndoStack>("Scrite", 1, 0, "UndoStack");

    qmlRegisterType<SearchEngine>("Scrite", 1, 0, "SearchEngine");
    qmlRegisterType<TextDocumentSearch>("Scrite", 1, 0, "TextDocumentSearch");
    qmlRegisterUncreatableType<SearchAgent>("Scrite", 1, 0, "SearchAgent", apreason);

    qmlRegisterUncreatableType<Notification>("Scrite", 1, 0, "Notification", apreason);
    qmlRegisterUncreatableType<NotificationManager>("Scrite", 1, 0, "NotificationManager", "Use notificationManager instead.");

    qmlRegisterUncreatableType<ErrorReport>("Scrite", 1, 0, "ErrorReport", reason);
    qmlRegisterUncreatableType<ProgressReport>("Scrite", 1, 0, "ProgressReport", reason);

    qmlRegisterUncreatableType<TransliterationEngine>("Scrite", 1, 0, "TransliterationEngine", "Use app.transliterationEngine instead.");
    qmlRegisterUncreatableType<Transliterator>("Scrite", 1, 0, "Transliterator", apreason);
    qmlRegisterType<TransliteratedText>("Scrite", 1, 0, "TransliteratedText");

    qmlRegisterUncreatableType<AbstractExporter>("Scrite", 1, 0, "AbstractExporter", reason);
    qmlRegisterUncreatableType<AbstractReportGenerator>("Scrite", 1, 0, "AbstractReportGenerator", reason);

    qmlRegisterUncreatableType<FocusTracker>("Scrite", 1, 0, "FocusTracker", reason);
    qmlRegisterUncreatableType<FocusTrackerIndicator>("Scrite", 1, 0, "FocusTrackerIndicator", reason);

    qmlRegisterUncreatableType<Application>("Scrite", 1, 0, "Application", reason);
    qmlRegisterType<Annotation>("Scrite", 1, 0, "Annotation");
    qmlRegisterType<DelayedPropertyBinder>("Scrite", 1, 0, "DelayedPropertyBinder");
    qmlRegisterType<ResetOnChange>("Scrite", 1, 0, "ResetOnChange");

    qmlRegisterUncreatableType<HeaderFooter>("Scrite", 1, 0, "HeaderFooter", reason);
    qmlRegisterUncreatableType<QTextDocumentPagedPrinter>("Scrite", 1, 0, "QTextDocumentPagedPrinter", reason);

    qmlRegisterUncreatableType<AutoUpdate>("Scrite", 1, 0, "AutoUpdate", reason);

    qmlRegisterType<MaterialColors>("Scrite", 1, 0, "MaterialColors");

    qmlRegisterType<GenericArrayModel>("Scrite", 1, 0, "GenericArrayModel");
    qmlRegisterType<GenericArraySortFilterProxyModel>("Scrite", 1, 0, "GenericArraySortFilterProxyModel");

    qmlRegisterUncreatableType<AbstractObjectTracker>("Scrite", 1, 0, "AbstractTracker", reason);
    qmlRegisterType<TrackProperty>("Scrite", 1, 0, "TrackProperty");
    qmlRegisterType<TrackSignal>("Scrite", 1, 0, "TrackSignal");
    qmlRegisterType<TrackModelRow>("Scrite", 1, 0, "TrackModelRow");
    qmlRegisterType<TrackerPack>("Scrite", 1, 0, "TrackerPack");

    qmlRegisterType<ScreenplayAdapter>("Scrite", 1, 0, "ScreenplayAdapter");
    qmlRegisterType<ScreenplayTextDocument>("Scrite", 1, 0, "ScreenplayTextDocument");
    qmlRegisterType<ScreenplayElementPageBreaks>("Scrite", 1, 0, "ScreenplayElementPageBreaks");
    qmlRegisterType<ImagePrinter>("Scrite", 1, 0, "ImagePrinter");
    qmlRegisterType<TextDocumentItem>("Scrite", 1, 0, "TextDocumentItem");
    qmlRegisterType<ScreenplayTextDocumentOffsets>("Scrite", 1, 0, "ScreenplayTextDocumentOffsets");

    qmlRegisterType<RulerItem>("Scrite", 1, 0, "RulerItem");

    qmlRegisterType<SpellCheckService>("Scrite", 1, 0, "SpellCheckService");

    qmlRegisterType<BoundingBoxEvaluator>("Scrite", 1, 0, "BoundingBoxEvaluator");
    qmlRegisterType<BoundingBoxPreview>("Scrite", 1, 0, "BoundingBoxPreview");
    qmlRegisterUncreatableType<BoundingBoxItem>("Scrite", 1, 0, "BoundingBoxItem", apreason);

    qmlRegisterType<FileInfo>("Scrite", 1, 0, "FileInfo");

    qmlRegisterUncreatableType<ShortcutsModelItem>("Scrite", 1, 0, "ShortcutsModelItem", apreason);

    qmlRegisterType<LibraryService>("Scrite", 1, 0, "LibraryService");
    qmlRegisterUncreatableType<Library>("Scrite", 1, 0, "Library", "Use from LibraryService.library");

    qmlRegisterType<UrlAttributes>("Scrite", 1, 0, "UrlAttributes");

    qmlRegisterUncreatableType<QAbstractItemModel>("Scrite", 1, 0, "Model", "Base type of models (QAbstractItemModel)");

    qmlRegisterType<TabSequenceManager>("Scrite", 1, 0, "TabSequenceManager");
    qmlRegisterUncreatableType<TabSequenceItem>("Scrite", 1, 0, "TabSequenceItem", apreason);

    qmlRegisterUncreatableType<Announcement>("Scrite", 1, 0, "Announcement", apreason);

    qmlRegisterType<NotebookTabModel>("Scrite", 1, 0, "NotebookTabModel");

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

