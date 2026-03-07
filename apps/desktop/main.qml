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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "./qml"
import "./qml/globals"
import "./qml/helpers"
import "./qml/dialogs"
import "./qml/controls"
import "./qml/overlays"
import "./qml/commandcenter"
import "./qml/notifications"
import "./qml/floatingdockpanels"

ApplicationWindow {
    id: root

    property bool closeButtonVisible: true

    AppWindow.closeButtonVisible: closeButtonVisible
    AppWindow.onInitialize: _private.initialize()

    width: 1366
    height: 700
    visible: true
    visibility: ApplicationWindow.Maximized

    color: Runtime.colors.primary.windowColor

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    Loader {
        id: _contentLoader

        anchors.fill: parent

        active: false
        sourceComponent: ScriteMainWindowContent {
            enabled: !NotificationsView.visible && Runtime.allowAppUsage
        }
    }

    // Private Section
    QtObject {
        id: _private

        function initialize() {
            // Initialize runtime
            Runtime.init(_contentLoader)
            ActionHub.init(_contentLoader)
            HelpCenter.init(_contentLoader)
            CommandCenter.init(_contentLoader)
            SubscriptionPlanOperations.init(_contentLoader)

            // Determine font size provided by QML
            determineDefaultFontSize()

            // Show the main-window content
            _contentLoader.active = true

            // Initialize layers
            BusyOverlay.init(_contentLoader)
            SubscriptionDetailsDialog.init()
            SubscriptionPlanComparisonDialog.init()
            UserAccountDialog.init(_contentLoader)
            FloatingDockLayer.init(_contentLoader)
            OverlaysLayer.init(_contentLoader)
            NotificationsLayer.init(_contentLoader)

            // Raise window
            Scrite.window.raise()

            // Show initial UI
            if(Scrite.user.loggedIn) {
                if(Runtime.allowAppUsage)
                    showHomeScreenOrOpenFile()
                else
                    UserAccountDialog.launch()
            } else {
                var splashScreen = SplashScreen.launch()
                if(splashScreen)
                    splashScreen.closed.connect(_private.splashScreenWasClosed)
                else
                    splashScreenWasClosed()
            }
        }

        function determineDefaultFontSize() {
            if( Scrite.app.customFontPointSize === 0) {
                var textItem = Qt.createQmlObject("import QtQuick 2.15; Text { text: \"Welcome to Scrite\" }", _contentLoader)
                if(textItem) {
                    Scrite.app.customFontPointSize = textItem.font.pointSize
                    textItem.destroy()
                }
            }
        }

        function splashScreenWasClosed() {
            if(Platform.isWindowsDesktop && Platform.osMajorVersion < 10) {
                MessageBox.information("",
                    "The Windows version of Scrite works best on Windows 10 or higher. While it may work on earlier versions of Windows, we don't actively test on them. We recommend that you use Scrite on PCs with Windows 10 or higher.",
                    _private.showHomeScreenOrOpenFile
                )
            } else if(Runtime.allowAppUsage)
                showHomeScreenOrOpenFile()
            else
                UserAccountDialog.launch()
        }

        function showHomeScreenOrOpenFile() {
            if(Scrite.fileNameToOpen === "") {
                if(!Scrite.app.maybeOpenAnonymously())
                    HomeScreen.launch()
            } else
                Scrite.document.open(Scrite.fileNameToOpen)

            Runtime.execLater(_private, 2000, _private.maybeOnboardUserSurvey)
        }

        function maybeOnboardUserSurvey() {
            if(Runtime.userAccountDialogSettings.userOnboardingStatus === "required") {
                UserOnboardingDialog.launch()
            }
        }
    }
}

