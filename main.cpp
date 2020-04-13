/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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

#include "logger.h"
#include "undostack.h"
#include "completer.h"
#include "aggregation.h"
#include "eventfilter.h"
#include "focustracker.h"
#include "notification.h"
#include "searchengine.h"
#include "standardpaths.h"
#include "textshapeitem.h"
#include "scritedocument.h"
#include "painterpathitem.h"
#include "transliteration.h"
#include "gridbackgrounditem.h"
#include "notificationmanager.h"
#include "delayedpropertybinder.h"
#include "abstractreportgenerator.h"

int main(int argc, char **argv)
{
    const QVersionNumber applicationVersion(0, 2, 4);
    Application::setApplicationName("scrite");
    Application::setOrganizationName("TERIFLIX");
    Application::setOrganizationDomain("teriflix.com");
    Application::setApplicationVersion(applicationVersion.toString() + "-beta");

    qInstallMessageHandler(&Logger::qtMessageHandler);

    Logger::instance()->log(
                QString("%1 %2 Version %3 Launched")
                  .arg(QApplication::organizationName())
                  .arg(QApplication::applicationName())
                  .arg(QApplication::applicationVersion()));

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
    qmlRegisterUncreatableType<SceneHeading>("Scrite", 1, 0, "SceneHeading", reason);
    qmlRegisterUncreatableType<SceneElement>("Scrite", 1, 0, "SceneElement", reason);

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

    qmlRegisterType<SceneDocumentBinder>("Scrite", 1, 0, "SceneDocumentBinder");

    qmlRegisterType<GridBackgroundItem>("Scrite", 1, 0, "GridBackground");
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

    qmlRegisterUncreatableType<TransliterationSettings>("Scrite", 1, 0, "TransliterationSettings", "Use app.transliterationSettings instead.");
    qmlRegisterUncreatableType<Transliterator>("Scrite", 1, 0, "Transliterator", "Use as attached property.");

    qmlRegisterUncreatableType<AbstractReportGenerator>("Scrite", 1, 0, "AbstractReportGenerator", reason);

    qmlRegisterUncreatableType<FocusTracker>("Scrite", 1, 0, "FocusTracker", reason);
    qmlRegisterUncreatableType<FocusTrackerIndicator>("Scrite", 1, 0, "FocusTrackerIndicator", reason);

    qmlRegisterType<Annotation>("Scrite", 1, 0, "Annotation");
    qmlRegisterType<DelayedPropertyBinder>("Scrite", 1, 0, "DelayedPropertyBinder");

    NotificationManager notificationManager;
    ScriteDocument scriteDocument;

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(2);

    QQuickView qmlView;
    qmlView.setFormat(format);
    scriteDocument.formatting()->setScreen(qmlView.screen());
    qmlView.setTitle(scriteDocument.documentWindowTitle());
    QObject::connect(&scriteDocument, &ScriteDocument::documentWindowTitleChanged, &qmlView, &QQuickView::setTitle);
    qmlView.engine()->rootContext()->setContextProperty("app", &a);
    qmlView.engine()->rootContext()->setContextProperty("qmlWindow", &qmlView);
    qmlView.engine()->rootContext()->setContextProperty("logger", Logger::instance());
    qmlView.engine()->rootContext()->setContextProperty("scriteDocument", &scriteDocument);
    qmlView.engine()->rootContext()->setContextProperty("notificationManager", &notificationManager);
    qmlView.setResizeMode(QQuickView::SizeRootObjectToView);
    qmlView.setSource(QUrl("qrc:/main.qml"));
    qmlView.setMinimumSize(QSize(1470, 865));
    qmlView.showMaximized();

    QObject::connect(&a, &Application::minimizeWindowRequest, &qmlView, &QQuickView::showMinimized);

    return a.exec();
}
