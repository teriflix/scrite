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
    visibility: ApplicationWindow.Windowed

    color: Runtime.colors.primary.windowColor

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    palette.window:          Runtime.colors.palette.window
    palette.windowText:      Runtime.colors.palette.windowText
    palette.base:            Runtime.colors.palette.base
    palette.text:            Runtime.colors.palette.text
    palette.button:          Runtime.colors.palette.button
    palette.buttonText:      Runtime.colors.palette.buttonText
    palette.highlight:       Runtime.colors.palette.highlight
    palette.highlightedText: Runtime.colors.palette.highlightedText
    palette.light:           Runtime.colors.palette.light
    palette.midlight:        Runtime.colors.palette.midlight
    palette.mid:             Runtime.colors.palette.mid
    palette.dark:            Runtime.colors.palette.dark
    palette.shadow:          Runtime.colors.palette.shadow
    palette.alternateBase:   Runtime.colors.palette.alternateBase
    palette.toolTipBase:     Runtime.colors.palette.toolTipBase
    palette.toolTipText:     Runtime.colors.palette.toolTipText
    palette.placeholderText: Runtime.colors.palette.placeholderText
    palette.brightText:      Runtime.colors.palette.brightText
    palette.link:            Runtime.colors.palette.link
    palette.linkVisited:     Runtime.colors.palette.linkVisited

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

            // Check for legacy NSIS install (production Windows only).
            // Scrite.prelaunchChecks() returns false if one is found; QML shows the error.
            if (!Scrite.prelaunchChecks()) {
                MessageBox.information(
                    "Previous Version Detected",
                    "A previous version of Scrite is installed on this computer.\n\n" +
                    "Please uninstall it via \"Add or Remove Programs\" in Windows Settings " +
                    "before running this version.",
                    Qt.quit)
                return
            }

            // Show the license dialog on first launch of each new version.
            if (!Scrite.isLicenseAccepted()) {
                var dlg = LicenseDialog.launch()
                if (dlg)
                    dlg.accepted.connect(_private.continueAfterLicense)
                else
                    _private.continueAfterLicense()
                return
            }

            _private.continueAfterLicense()
        }

        function continueAfterLicense() {
            // Show initial UI — user login check happens here
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
                let textItemObj = Qt.createQmlObject("import QtQuick; Text { text: \"Welcome to Scrite\" }", _contentLoader)
                let textItem = textItemObj as Text
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

