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

#include "user.h"
#include "fileinfo.h"
#include "appwindow.h"
#include "automation.h"
#include "application.h"
#include "shortcutsmodel.h"
#include "scritedocument.h"
#include "colorimageprovider.h"
#include "notificationmanager.h"

#include <QMenuBar>
#include <QQuickStyle>
#include <QQmlContext>
#include <QOperatingSystemVersion>

static AppWindow *AppWindowInstance = nullptr;

AppWindow *AppWindow::instance()
{
    return ::AppWindowInstance;
}

AppWindow::AppWindow()
{
    ::AppWindowInstance = this;

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    const QByteArray envOpenGLMultisampling =
            qgetenv("SCRITE_OPENGL_MULTISAMPLING").toUpper().trimmed();
    if (envOpenGLMultisampling == QByteArrayLiteral("FULL"))
        format.setSamples(4);
    else if (envOpenGLMultisampling == QByteArrayLiteral("EXTREME"))
        format.setSamples(8);
    else if (envOpenGLMultisampling == QByteArrayLiteral("NONE"))
        format.setSamples(-1);
    else
        format.setSamples(2); // default

#ifdef Q_OS_MAC
    this->setFlag(Qt::WindowFullscreenButtonHint); // [0.5.2 All] Full Screen Mode #194
#endif
    this->setObjectName(QStringLiteral("ScriteWindow"));
    this->setFormat(format);
#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10)
        this->setSceneGraphBackend(QSGRendererInterface::Direct3D12);
    else
        this->setSceneGraphBackend(QSGRendererInterface::Software);
#endif

#ifdef Q_OS_MAC
    QMenuBar *menuBar = new QMenuBar(nullptr);
    QAction *quitAction = menuBar->addMenu("File")->addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, this, &QQuickView::close);
#endif

    // Handle minimize window request from application to minimize this window
    Application &scriteApp = *Application::instance();
    QObject::connect(&scriteApp, &Application::minimizeWindowRequest, this,
                     &QQuickView::showMinimized);

    // Hook up to Scrite Document
    ScriteDocument *scriteDocument = ScriteDocument::instance();
    scriteDocument->formatting()->setSreeenFromWindow(this);
    scriteDocument->clearModified();
    scriteApp.initializeStandardColors(this->engine());
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
    this->engine()->addImageProvider(QStringLiteral("fileIcon"), new FileIconProvider);
    this->engine()->addImageProvider(QStringLiteral("userIcon"), new UserIconProvider);

    // Force registration of QML types in io.scrite.components
    extern void qml_register_types_io_scrite_components();
    qml_register_types_io_scrite_components();

    // Register singletons
    const char *uri = "io.scrite.models";
    qmlRegisterUncreatableType<QAbstractItemModel>(uri, 1, 0, "Model",
                                                   "Base type of models (QAbstractItemModel)");
}

AppWindow::~AppWindow()
{
    ::AppWindowInstance = nullptr;
}

void AppWindow::initializeFileNameToOpen()
{
    Application &scriteApp = *Application::instance();

    QString fileNameToOpen;
#ifdef Q_OS_MAC
    if (!scriteApp.fileToOpen().isEmpty())
        fileNameToOpen = scriteApp.fileToOpen();
    scriteApp.setHandleFileOpenEvents(true);
#else
    const QStringList appArgs = scriteApp.arguments();
    if (appArgs.size() > 1) {
        bool hasOptions = false;
        for (const QString &arg : appArgs) {
            if (arg.startsWith(QStringLiteral("--"))) {
                hasOptions = true;
                break;
            }
        }

        if (!hasOptions) {
#ifdef Q_OS_WIN
            fileNameToOpen = appArgs.last();
#else
            QStringList args = appArgs;
            args.takeFirst();
            fileNameToOpen = args.join(QStringLiteral(" "));
#endif
        }
    }
#endif
    this->rootContext()->setContextProperty(QStringLiteral("fileNameToOpen"), fileNameToOpen);
}
