/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "utils.h"
#include "scrite.h"
#include "appwindow.h"
#include "application.h"
#include "scritedocument.h"

#include <QMenuBar>
#include <QSettings>
#include <QQuickStyle>
#include <QQmlContext>
#include <QOperatingSystemVersion>

static AppWindow *GlobalAppWindowInstance = nullptr;

QQuickWindow *AppWindow::instance()
{
    return GlobalAppWindowInstance ? GlobalAppWindowInstance->window() : nullptr;
}

AppWindow *AppWindow::qmlAttachedProperties(QObject *object)
{
    if (GlobalAppWindowInstance != nullptr)
        return nullptr;

    QQuickWindow *window = qobject_cast<QQuickWindow *>(object);
    if (window == nullptr)
        return nullptr;

    GlobalAppWindowInstance = new AppWindow(window);
    return GlobalAppWindowInstance;
}

AppWindow::AppWindow(QQuickWindow *window) : QObject(window), m_window(window)
{
#ifdef Q_OS_MAC
    window->setFlags(Qt::Window
                     | Qt::WindowFullscreenButtonHint); // [0.5.2 All] Full Screen Mode #194
#endif
    window->setObjectName(QStringLiteral("ScriteWindow"));

#ifdef Q_OS_MAC
    QMenuBar *menuBar = new QMenuBar(nullptr);
    QAction *quitAction = menuBar->addMenu("File")->addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, window, &QQuickWindow::close);
#endif

    // Handle minimize window request from application to minimize this window
    Application &scriteApp = *Application::instance();
    QObject::connect(&scriteApp, &Application::minimizeWindowRequest, window,
                     &QQuickWindow::showMinimized);

    // Hook up to Scrite Document
    ScriteDocument *scriteDocument = ScriteDocument::instance();
    scriteDocument->formatting()->setSreeenFromWindow(window);
    scriteDocument->clearModified();
    window->setTitle(scriteDocument->documentWindowTitle());
    QObject::connect(scriteDocument, &ScriteDocument::documentWindowTitleChanged, window,
                     &QQuickWindow::setTitle);

    // Configure minimum size of the application window
    const QScreen *screen = scriteApp.primaryScreen();
    const QSize screenSize = screen->availableSize();
    window->setMinimumSize(QSize(qMin(600, screenSize.width()), qMin(375, screenSize.height())));

    // If supplied in args, load the file-name
    this->initializeFileNameToOpen();

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
    window->setTextRenderType(useNativeTextRendering ? QQuickWindow::NativeTextRendering
                                                     : QQuickWindow::QtTextRendering);

    m_defaultWindowFlags = window->flags();

    window->setMinimumSize(QSize(1366, 700));

    QTimer::singleShot(50, this, &AppWindow::initialize);
}

AppWindow::~AppWindow()
{
    GlobalAppWindowInstance = nullptr;
}

void AppWindow::setCloseButtonVisible(bool val)
{
    if (m_closeButtonVisible == val)
        return;

    m_closeButtonVisible = val;

    Qt::WindowFlags newFlags = m_defaultWindowFlags;
    if (!val)
        newFlags |= Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;
    m_window->setFlags(newFlags);

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
