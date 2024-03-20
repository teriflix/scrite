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

    Item {
        id: floatingDockLayer
        anchors.fill: parent
        Component.onCompleted: {
            Runtime.floatingDockLayer = floatingDockLayer
            Qt.callLater(initFloatingDockPanels)
        }

        function initFloatingDockPanels() {
            FloatingMarkupToolsDock.init()
            FloatingShortcutsDock.init()
        }
    }

    Loader {
        id: statusText
        active: false
        anchors.fill: parent
        property string text
        property bool animationsEnabled: Runtime.applicationSettings.enableAnimations
        function show(t) {
            text = t
            active = true
        }
        onActiveChanged: {
            if(!active)
                text = ""
        }
        sourceComponent: Item {
            VclText {
                id: textItem
                anchors.centerIn: parent
                font.pixelSize: parent.height * 0.075
                text: statusText.text
                property real t: statusText.animationsEnabled ? 0 : 1
                scale: 0.5 + t/1.0
                opacity: statusText.animationsEnabled ? (1.0 - t*0.75) : 0.8
            }

            SequentialAnimation {
                running: true

                NumberAnimation {
                    target: textItem
                    properties: "t"
                    from: 0
                    to: 1
                    duration: statusText.animationsEnabled ? 250 : 0
                    easing.type: Easing.OutQuint
                }

                PauseAnimation {
                    duration: statusText.animationsEnabled ? 0 : 250
                }

                ScriptAction {
                    script: statusText.active = false
                }
            }
        }

        Connections {
            target: Runtime.applicationSettings
            function onEnableAnimationsChanged() {
                statusText.animationsEnabled = Runtime.applicationSettings.enableAnimations
            }
        }
    }

    Loader {
        active: Scrite.document.busy
        anchors.fill: parent
        sourceComponent: Item {
            Rectangle {
                anchors.fill: indication
                anchors.margins: -30
                radius: 4
                color: Runtime.colors.primary.c600.background
            }

            Row {
                id: indication
                anchors.centerIn: parent
                spacing: 20
                width: Math.min(parent.width * 0.4, implicitWidth)
                property real maxWidth: parent.width*0.4

                BusyIcon {
                    id: busyIndicator
                    anchors.verticalCenter: parent.verticalCenter
                    running: true
                    width: 50; height: 50
                    forDarkBackground: true
                }

                VclText {
                    width: Math.min(parent.maxWidth - busyIndicator.width - parent.spacing, contentWidth)
                    anchors.verticalCenter: parent.verticalCenter
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: Scrite.document.busyMessage
                    font.pixelSize: 16
                    color: Runtime.colors.primary.c600.text
                }
            }

            MouseArea {
                anchors.fill: parent
            }

            EventFilter.target: Scrite.app
            EventFilter.events: [6,7]
            EventFilter.onFilter: {
                result.filter = true
            }
        }
    }

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

    Connections {
        target: Scrite.window
        function onScreenChanged() { Scrite.document.formatting.setSreeenFromWindow(Scrite.window) }
        // function onActiveFocusItemChanged() { console.log("PA: " + Scrite.window.activeFocusItem) }
    }

    Loader {
        id: splashLoader
        anchors.fill: parent
        sourceComponent: UI.SplashScreen {
            onDone: {
                const launchHomeScreen = function() {
                    if(Scrite.user.loggedIn)
                        HomeScreenDialog.launch()
                }
                splashLoader.active = false
                if(Scrite.app.isWindowsPlatform && Scrite.app.isNotWindows10)
                    MessageBox.information("",
                        "The Windows version of Scrite works best on Windows 10 or higher. While it may work on earlier versions of Windows, we don't actively test on them. We recommend that you use Scrite on PCs with Windows 10 or higher.",
                        () => { launchHomeScreen() }
                    )
                else
                    launchHomeScreen()
                if(fileNameToOpen !== "")
                    Scrite.document.open(fileNameToOpen)
            }
        }
    }

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

    property int lastSnapshotTimestamp: 0
    EventFilter.active: Scrite.app.getEnvironmentVariable("SCRITE_SNAPSHOT_CAPTURE") === "YES"
    EventFilter.target: Scrite.app
    EventFilter.events: [6]
    EventFilter.onFilter: {
        if(event.key === Qt.Key_F6) {
            var timestamp = (new Date()).getTime()
            if(timestamp - lastSnapshotTimestamp > 500) {
                lastSnapshotTimestamp = timestamp
                windowCapture.capture()
            }
        }
    }

    WindowCapture {
        id: windowCapture
        fileName: "scrite-window-capture.jpg"
        format: WindowCapture.JPGFormat
        forceCounterInFileName: true
        window: Scrite.window
        captureMode: WindowCapture.FileAndClipboard
    }

    Component.onCompleted: Scrite.window.raise()
}

