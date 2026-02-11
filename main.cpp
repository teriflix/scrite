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

#include "user.h"
#include "utils.h"
#include "scrite.h"
#include "appwindow.h"
#include "application.h"
#include "languageengine.h"
#include "scritedocument.h"
#include "crashpadmodule.h"
#include "documentfilesystem.h"
#include "scritedocumentvault.h"
#include "notificationmanager.h"
#include "systemrequirements.h"

#include <QQuickStyle>

int main(int argc, char **argv)
{
    if (CrashpadModule::isAvailable()) {
        if (!CrashpadModule::prepare())
            return 0;

        if (CrashpadModule::initialize()) {
#ifdef ENABLE_CRASHPAD_CRASH_TEST
            qInfo() << "Crashpad Initialized";
#endif
        }
    }

#ifndef Q_OS_WINDOWS
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    Application scriteApp(argc, argv, Application::prepare());

    if (!SystemRequirements::checkAndReport())
        return -1;

    Utils::registerTypes();
    User::instance();
    LanguageEngine::instance();
    NotificationManager::instance();
    DocumentFileSystem::setMarker(QByteArrayLiteral("SCRITE"));
    UndoHub::instance();
    ScriteDocument::instance();
    ScriteDocumentVault::instance();

    AppWindow scriteWindow;
    QTimer::singleShot(0, &scriteWindow, [&scriteWindow]() {
        scriteWindow.setSource(QUrl("qrc:/main.qml"));
        scriteWindow.show();
    });

    return scriteApp.exec();
}
