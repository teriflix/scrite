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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml"
import "qrc:/qml/modules"
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/overlays"
import "qrc:/qml/notifications"
import "qrc:/qml/floatingdockpanels"

Rectangle {
    id: scriteRoot
    width: 1366
    height: 700
    color: Runtime.colors.primary.windowColor

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Material.Light
    Material.background: Runtime.colors.accent.c700.background

    ScriteMainWindow {
        anchors.fill: parent
        enabled: !NotificationsView.visible && Runtime.allowAppUsage
    }

    // Private Section
    Component.onCompleted: _private.initialize()

    QtObject {
        id: _private

        function initialize() {
            // Initialize runtime
            Runtime.init(scriteRoot)

            // Determine font size provided by QML
            determineDefaultFontSize()

            // Initialize layers
            LoginWorkflow.init(scriteRoot)
            FloatingDockLayer.init(scriteRoot)
            OverlaysLayer.init(scriteRoot)
            NotificationsLayer.init(scriteRoot)

            // Raise window
            Scrite.window.raise()

            // Show initial UI
            if(Scrite.user.loggedIn)
                showHomeScreenOrOpenFile()
            else {
                var splashScreen = SplashScreen.launch()
                if(splashScreen)
                    splashScreen.closed.connect(_private.splashScreenWasClosed)
                else
                    splashScreenWasClosed()
            }
        }

        function determineDefaultFontSize() {
            if( Scrite.app.customFontPointSize === 0) {
                var textItem = Qt.createQmlObject("import QtQuick 2.15; Text { text: \"Welcome to Scrite\" }", scriteRoot)
                if(textItem) {
                    Scrite.app.customFontPointSize = textItem.font.pointSize
                    textItem.destroy()
                }
            }
        }

        function splashScreenWasClosed() {
            if(Scrite.app.isWindowsPlatform && Scrite.app.isNotWindows10) {
                MessageBox.information("",
                    "The Windows version of Scrite works best on Windows 10 or higher. While it may work on earlier versions of Windows, we don't actively test on them. We recommend that you use Scrite on PCs with Windows 10 or higher.",
                    _private.showHomeScreenOrOpenFile
                )
            } else if(Scrite.user.loggedIn)
                showHomeScreenOrOpenFile()
            else
                Announcement.shout(Runtime.announcementIds.loginRequest, undefined)
        }

        function showHomeScreenOrOpenFile() {
            if(Scrite.fileNameToOpen === "") {
                if(!Scrite.app.maybeOpenAnonymously())
                    HomeScreen.launch()
            } else
                Scrite.document.open(Scrite.fileNameToOpen)

            Scrite.user.forceLoginRequest.connect(userForceLoginRequest)
        }

        function userForceLoginRequest() {
            Announcement.shout(Runtime.announcementIds.loginRequest, undefined)
        }
    }
}

