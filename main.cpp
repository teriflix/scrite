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
#include "appwindow.h"
#include "shortcutsmodel.h"
#include "scritedocument.h"
#include "scritedocumentvault.h"
#include "documentfilesystem.h"
#include "notificationmanager.h"

#include <QQuickStyle>

int main(int argc, char **argv)
{
    Application scriteApp(argc, argv, Application::prepare());
    NotificationManager::instance();
    DocumentFileSystem::setMarker(QByteArrayLiteral("SCRITE"));
    ShortcutsModel::instance();
    ScriteDocument::instance();
    ScriteDocumentVault::instance();

    AppWindow scriteWindow;
    scriteWindow.setSource(QUrl("qrc:/main.qml"));
    scriteWindow.show();

    return scriteApp.exec();
}
