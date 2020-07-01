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

#include <QShortcut>
#include <QUndoStack>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>

#include "undoredo.h"
#include "completer.h"
#include "ruleritem.h"
#include "autoupdate.h"
#include "trackobject.h"
#include "aggregation.h"
#include "eventfilter.h"
#include "imageprinter.h"
#include "focustracker.h"
#include "notification.h"
#include "searchengine.h"
#include "standardpaths.h"
#include "textshapeitem.h"
#include "resetonchange.h"
#include "scritedocument.h"
#include "materialcolors.h"
#include "painterpathitem.h"
#include "transliteration.h"
#include "abstractexporter.h"
#include "tightboundingbox.h"
#include "genericarraymodel.h"
#include "screenplayadapter.h"
#include "spellcheckservice.h"
#include "gridbackgrounditem.h"
#include "notificationmanager.h"
#include "delayedpropertybinder.h"
#include "screenplaytextdocument.h"
#include "abstractreportgenerator.h"
#include "qtextdocumentpagedprinter.h"

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
    const QVersionNumber applicationVersion(0, 4, 5);
    Application::setApplicationName("Scrite");
    Application::setOrganizationName("TERIFLIX");
    Application::setOrganizationDomain("teriflix.com");
#ifdef Q_OS_MAC
    Application::setApplicationVersion(applicationVersion.toString() + "-dev-beta");
#else
    if(QSysInfo::WordSize == 32)
        Application::setApplicationVersion(applicationVersion.toString() + "-beta-x86");
    else
        Application::setApplicationVersion(applicationVersion.toString() + "-beta-x64");
#endif

    qInstallMessageHandler(ScriteQtMessageHandler);

    Application a(argc, argv, applicationVersion);

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

    const QString reason("Instantiation from QML not allowed.");
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

    qmlRegisterType<Note>("Scrite", 1, 0, "Note");
    qmlRegisterUncreatableType<Character>("Scrite", 1, 0, "Character", reason);

    qmlRegisterUncreatableType<ScriteDocument>("Scrite", 1, 0, "ScriteDocument", reason);
    qmlRegisterUncreatableType<ScreenplayFormat>("Scrite", 1, 0, "ScreenplayFormat", reason);
    qmlRegisterUncreatableType<SceneElementFormat>("Scrite", 1, 0, "SceneElementFormat", reason);
    qmlRegisterUncreatableType<ScreenplayPageLayout>("Scrite", 1, 0, "ScreenplayPageLayout", reason);

    qmlRegisterType<SceneDocumentBinder>("Scrite", 1, 0, "SceneDocumentBinder");

    qmlRegisterType<GridBackgroundItem>("Scrite", 1, 0, "GridBackground");
    qmlRegisterUncreatableType<GridBackgroundItemBorder>("Scrite", 1, 0, "GridBackgroundItemBorder", reason);
    qmlRegisterType<Completer>("Scrite", 1, 0, "Completer");

    qmlRegisterUncreatableType<EventFilterResult>("Scrite", 1, 0, "EventFilterResult", "Use the instance provided by EventFilter.onFilter signal.");
    qmlRegisterUncreatableType<EventFilter>("Scrite", 1, 0, "EventFilter", "Use as attached property.");

    qmlRegisterType<PainterPathItem>("Scrite", 1, 0, "PainterPathItem");
    qmlRegisterUncreatableType<AbstractPathElement>("Scrite", 1, 0, "PathElement", "Use subclasses of AbstractPathElement.");
    qmlRegisterType<PainterPath>("Scrite", 1, 0, "PainterPath");
    qmlRegisterType<MoveToElement>("Scrite", 1, 0, "MoveTo");
    qmlRegisterType<LineToElement>("Scrite", 1, 0, "LineTo");
    qmlRegisterType<CloseSubpathElement>("Scrite", 1, 0, "CloseSubpath");
    qmlRegisterType<CubicToElement>("Scrite", 1, 0, "CubicTo");
    qmlRegisterType<QuadToElement>("Scrite", 1, 0, "QuadTo");
    qmlRegisterType<TextShapeItem>("Scrite", 1, 0, "TextShapeItem");
    qmlRegisterType<UndoStack>("Scrite", 1, 0, "UndoStack");

    qmlRegisterType<SearchEngine>("Scrite", 1, 0, "SearchEngine");
    qmlRegisterType<TextDocumentSearch>("Scrite", 1, 0, "TextDocumentSearch");
    qmlRegisterUncreatableType<SearchAgent>("Scrite", 1, 0, "SearchAgent", "Use as attached property.");

    qmlRegisterUncreatableType<Notification>("Scrite", 1, 0, "Notification", "Use as attached property.");
    qmlRegisterUncreatableType<NotificationManager>("Scrite", 1, 0, "NotificationManager", "Use notificationManager instead.");

    qmlRegisterUncreatableType<ErrorReport>("Scrite", 1, 0, "ErrorReport", reason);
    qmlRegisterUncreatableType<ProgressReport>("Scrite", 1, 0, "ProgressReport", reason);

    qmlRegisterUncreatableType<TransliterationEngine>("Scrite", 1, 0, "TransliterationEngine", "Use app.transliterationEngine instead.");
    qmlRegisterUncreatableType<Transliterator>("Scrite", 1, 0, "Transliterator", "Use as attached property.");
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

    qmlRegisterType<RulerItem>("Scrite", 1, 0, "RulerItem");

    qmlRegisterType<SpellCheckService>("Scrite", 1, 0, "SpellCheckService");

    qmlRegisterType<TightBoundingBoxEvaluator>("Scrite", 1, 0, "TightBoundingBoxEvaluator");
    qmlRegisterUncreatableType<TightBoundingBoxItem>("Scrite", 1, 0, "TightBoundingBoxItem", "Use as attached property.");

    NotificationManager notificationManager;

    DocumentFileSystem::setMarker( QByteArrayLiteral("SCRITE") );

    ScriteDocument *scriteDocument = ScriteDocument::instance();

    if(a.arguments().size() == 2)
        scriteDocument->open( a.arguments().last() );

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

    QQuickStyle::setStyle("Material");

    QQuickView qmlView;
    qmlView.setFormat(format);
    scriteDocument->formatting()->setSreeenFromWindow(&qmlView);
    scriteDocument->clearModified();
    a.initializeStandardColors(qmlView.engine());
    qmlView.setTitle(scriteDocument->documentWindowTitle());
    QObject::connect(scriteDocument, &ScriteDocument::documentWindowTitleChanged, &qmlView, &QQuickView::setTitle);
    qmlView.engine()->rootContext()->setContextProperty("app", &a);
    qmlView.engine()->rootContext()->setContextProperty("qmlWindow", &qmlView);
    qmlView.engine()->rootContext()->setContextProperty("scriteDocument", scriteDocument);
    qmlView.engine()->rootContext()->setContextProperty("notificationManager", &notificationManager);
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
    qmlView.setSource(QUrl("qrc:/main.qml"));
    qmlView.setMinimumSize(QSize(qMin(1440,primaryScreenSize.width()), qMin(900,primaryScreenSize.height())));
    qmlView.showMaximized();
    qmlView.raise();

    QObject::connect(&a, &Application::minimizeWindowRequest, &qmlView, &QQuickView::showMinimized);

    return a.exec();
}
