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
import QtQuick.Controls 2.15
// import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml" as UI
import "qrc:/qml/controls"
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/overlays"
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

    UI.ScriteMainWindow {
        id: scriteMainWindow
        anchors.fill: parent
        enabled: !notificationsView.visible
    }

    // Refactoring QML TODO: Move this to a singleton
    Item {
        property AutoUpdate autoUpdate: Scrite.app.autoUpdate

        Notification.active: autoUpdate.updateAvailable || autoUpdate.surveyAvailable
        Notification.title: autoUpdate.updateAvailable ? "Update Available" : (autoUpdate.surveyAvailable ? autoUpdate.surveyInfo.title : "")
        Notification.text: {
            if(autoUpdate.updateAvailable)
                return "Scrite " + autoUpdate.updateInfo.versionString + " is now available for download. <font size=\"-1\"><i>[<strong>What's new?</strong> " + autoUpdate.updateInfo.changeLog + "]</i></font>"
            if(autoUpdate.surveyAvailable)
                return autoUpdate.surveyInfo.text
            return ""
        }
        Notification.buttons: autoUpdate.updateAvailable ? ["Download", "Ignore"] : ["Participate", "Not Now", "Dont Ask Again"]
        Notification.onButtonClicked: (index) => {
            if(autoUpdate.updateAvailable) {
                if(index === 0)
                    Qt.openUrlExternally(autoUpdate.updateDownloadUrl)
            } else if(autoUpdate.surveyAvailable) {
                if(index === 0) {
                    Qt.openUrlExternally(autoUpdate.surveyUrl)
                    autoUpdate.dontAskForSurveyAgain(true)
                } else if(index === 2)
                    autoUpdate.dontAskForSurveyAgain(true)
            }
        }
    }

    // Refactoring QML TODO: Move this to a singleton
    Rectangle {
        anchors.fill: parent
        visible: Scrite.notifications.count > 0
        color: Scrite.app.translucent(Runtime.colors.primary.borderColor, 0.6)

        UI.NotificationsView {
            id: notificationsView
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: -1
            width: parent.width * 0.7
        }
    }

    // Private Section
    Component.onCompleted: _private.initialize()

    QtObject {
        id: _private

        function initialize() {
            // Initialize layers
            FloatingDockLayer.init(scriteRoot)
            OverlaysLayer.init(scriteRoot)

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

            initFloatingDockPanels()
        }

        function userForceLoginRequest() {
            Announcement.shout(Runtime.announcementIds.loginRequest, undefined)
        }

        function initFloatingDockPanels() {
            FloatingMarkupToolsDock.init()
            FloatingShortcutsDock.init()
        }
    }
}

