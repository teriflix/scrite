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

#include "user.h"
#include "scrite.h"
#include "appwindow.h"
#include "callgraph.h"
#include "automation.h"
#include "application.h"
#include "scritedocument.h"
#include "languageengine.h"
#include "colorimageprovider.h"
#include "basicfileiconprovider.h"

#include <QMenuBar>
#include <QSettings>
#include <QQuickStyle>
#include <QQmlContext>
#include <QOperatingSystemVersion>

static AppWindow *AppWindowInstance = nullptr;

AppWindow *AppWindow::instance()
{
    // CAPTURE_FIRST_CALL_GRAPH;
    return ::AppWindowInstance;
}

AppWindow::AppWindow()
{
    // CAPTURE_CALL_GRAPH;
    ::AppWindowInstance = this;

    this->setFormat(QSurfaceFormat::defaultFormat());

#ifdef Q_OS_MAC
    this->setFlag(Qt::WindowFullscreenButtonHint); // [0.5.2 All] Full Screen Mode #194
#endif
    this->setObjectName(QStringLiteral("ScriteWindow"));

#ifdef Q_OS_MAC
    QMenuBar *menuBar = new QMenuBar(nullptr);
    QAction *quitAction = menuBar->addMenu("File")->addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, this, &QQuickView::close);
#endif

    // Handle minimize window request from application to minimize this window
    Application &scriteApp = *Application::instance();
    QObject::connect(&scriteApp, &Application::minimizeWindowRequest, this,
                     &QQuickView::showMinimized);
    QObject::connect(this->engine(), &QQmlEngine::quit, &scriteApp, &Application::quit);

    // Hook up to Scrite Document
    ScriteDocument *scriteDocument = ScriteDocument::instance();
    scriteDocument->formatting()->setSreeenFromWindow(this);
    scriteDocument->clearModified();
    this->setTitle(scriteDocument->documentWindowTitle());
    QObject::connect(scriteDocument, &ScriteDocument::documentWindowTitleChanged, this,
                     &QQuickView::setTitle);

    // Configure minimum size of the application window
    const QScreen *screen = scriteApp.primaryScreen();
    const QSize screenSize = screen->availableSize();
    this->setMinimumSize(QSize(qMin(600, screenSize.width()), qMin(375, screenSize.height())));

    // If supplied in args, load the file-name
    this->initializeFileNameToOpen();

    // Other inits
    this->setResizeMode(QQuickView::SizeRootObjectToView);
    // this->setTextRenderType(QQuickView::NativeTextRendering);
    Automation::init(this);

    // Register image providers
    this->engine()->addImageProvider(QStringLiteral("color"), new ColorImageProvider);
    this->engine()->addImageProvider(QStringLiteral("fileIcon"), new BasicFileIconProvider);

    // Force registration of QML types in io.scrite.components
    extern void qml_register_types_io_scrite_components();
    qml_register_types_io_scrite_components();

    // Register singletons
    const char *uri = "io.scrite.models";
    qmlRegisterUncreatableType<QAbstractItemModel>(uri, 1, 0, "Model",
                                                   "Base type of models (QAbstractItemModel)");
    qmlRegisterUncreatableType<QFontDatabase>(uri, 1, 0, "FontDatabase",
                                              "Refers to a QFontDatabase instance.");

    const bool useNativeTextRendering = [=]() -> bool {
#ifdef Q_OS_WIN
        if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10)
            return true;
#endif
        const QSettings *settings = Application::instance()->settings();
        return settings
                ? settings->value(QStringLiteral("Application/useNativeTextRendering"), false)
                          .toBool()
                : true;
    }();
    setTextRenderType(useNativeTextRendering ? NativeTextRendering : QtTextRendering);

    m_defaultWindowFlags = this->flags();

    LanguageEngine::init("io.scrite.components", this->engine());

    this->setMinimumSize(QSize(1366, 700));
}

AppWindow::~AppWindow()
{
    ::AppWindowInstance = nullptr;
}

void AppWindow::setCloseButtonVisible(bool val)
{
    if (m_closeButtonVisible == val)
        return;

    m_closeButtonVisible = val;

    Qt::WindowFlags newFlags = m_defaultWindowFlags;
    if (!val)
        newFlags |= Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;
    this->setFlags(newFlags);

    emit closeButtonVisibleChanged();
}

static inline QString getFileNameToOpenFromAppArgs()
{
    QString ret;

    QStringList appArgs = qApp->arguments();
    appArgs.takeFirst(); // Get rid of application file name

    if (!appArgs.isEmpty()) {
        if (appArgs.contains("--openAnonymously"))
            return ret;

        auto removeArgs = [&appArgs](int from, int count) {
            for (int i = 0; i < count; i++)
                appArgs.removeAt(from);
        };

        int index = appArgs.indexOf("--sessionToken");
        if (index >= 0)
            removeArgs(index, 2);

        index = appArgs.indexOf("--windowGeometry");
        if (index >= 0)
            removeArgs(index, 5);

#ifdef Q_OS_WIN
        ret = appArgs.isEmpty() ? QString() : appArgs.last();
#else
        ret = appArgs.join(QStringLiteral(" "));
#endif
    }

    if (!ret.isEmpty()) {
        QFileInfo fi(ret);
        if (fi.exists() && fi.isReadable())
            return ret;
    }

    return QString();
}

void AppWindow::initializeFileNameToOpen()
{
    QString fileNameToOpen;

#ifdef Q_OS_MAC
    if (Application::instance()->fileToOpen().isEmpty())
        fileNameToOpen = getFileNameToOpenFromAppArgs();
    else
        fileNameToOpen = Application::instance()->fileToOpen();
    Application::instance()->setHandleFileOpenEvents(true);
#else
    fileNameToOpen = getFileNameToOpenFromAppArgs();
#endif

    Scrite::setFileNameToOpen(fileNameToOpen);
}
