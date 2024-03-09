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

import "./qml" as UI
import "./qml/globals"

Rectangle {
    id: mainWindow
    width: 1366
    height: 700
    color: ScriteRuntime.colors.primary.windowColor

    Material.primary: ScriteRuntime.colors.primary.key
    Material.accent: ScriteRuntime.colors.accent.key
    Material.theme: Material.Light
    Material.background: ScriteRuntime.colors.accent.c700.background

    UI.ScriteDocumentView {
        id: mainScriteDocumentView
        anchors.fill: parent
        enabled: !dialogUnderlay.visible && !notificationsView.visible
    }

    Loader {
        id: statusText
        active: false
        anchors.fill: parent
        property string text
        property bool animationsEnabled: ScriteRuntime.applicationSettings.enableAnimations
        function show(t) {
            text = t
            active = true
        }
        onActiveChanged: {
            if(!active)
                text = ""
        }
        sourceComponent: Item {
            Text {
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
            target: ScriteRuntime.applicationSettings
            function onEnableAnimationsChanged() {
                statusText.animationsEnabled = ScriteRuntime.applicationSettings.enableAnimations
            }
        }
    }

    Item {
        id: dialogUnderlay
        anchors.fill: mainScriteDocumentView
        property color color: ScriteRuntime.colors.primary.windowColor

        property int visibilityCounter: 0
        function show() {
            visibilityCounter = Math.max(visibilityCounter+1,1)
            visible = true
        }

        function hide() {
            visibilityCounter = visibilityCounter-1
            if(visibilityCounter <= 0)
                visible = false
        }

        property real maxRadius: 32
        property real radius: maxRadius
        visible: false
        onVisibleChanged: {
            if(!visible) {
                color = ScriteRuntime.colors.primary.windowColor
                visibilityCounter = 0
            }
        }

        Rectangle {
            anchors.fill: parent
            color: parent.color
            opacity: 0.9 * (parent.radius/parent.maxRadius)
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            preventStealing: true
            propagateComposedEvents: false
            enabled: parent.visible
        }
    }

    function showInformation(params, popupSource) {
        var okCallback = function() {
            if(params.callback)
                params.callback(true)
            modalDialog.closeable = true
            modalDialog.initItemCallback = undefined
        }

        modalDialog.initItemCallback = function(item) {
            if(params.message)
                item.message = params.message
            if(params.okButtonText)
                item.okButtonText = params.okButtonText
            item.okCallback = okCallback
        }

        modalDialog.sourceComponent = infoDialogComponent
        if(popupSource)
            modalDialog.popupSource = popupSource
        modalDialog.closeable = false
        if(params.closeOnEscape !== undefined)
            modalDialog.closeOnEscape = params.closeOnEscape
        modalDialog.active = true
    }

    function askQuestion(params, popupSource) {
        var okCallback = function() {
            if(params.callback)
                params.callback(true)
            modalDialog.closeable = true
            modalDialog.initItemCallback = undefined
        }

        var cancelCallback = function() {
            if(params.callback)
                params.callback(false)
            modalDialog.closeable = true
            modalDialog.initItemCallback = undefined
        }

        modalDialog.initItemCallback = function(item) {
            if(params.question)
                item.question = params.question
            if(params.okButtonText)
                item.okButtonText = params.okButtonText
            if(params.cancelButtonText)
                item.cancelButtonText = params.cancelButtonText
            if(params.abortButtonText)
                item.abortButtonText = params.abortButtonText
            item.okCallback = okCallback
            item.cancelCallback = cancelCallback
        }

        modalDialog.sourceComponent = okCancelDialogComponent
        if(popupSource)
            modalDialog.popupSource = popupSource
        modalDialog.closeable = false
        modalDialog.active = true
    }

    UI.DialogOverlay {
        id: modalDialog
        active: false
        anchors.fill: parent
        enabled: Scrite.notifications.count === 0
        animationsEnabled: ScriteRuntime.applicationSettings.enableAnimations
        onCloseRequest: {
            active = false
            closeable = true
            closeUponClickOutsideContentArea = false
            closeOnEscape = Qt.binding( function() { return closeable || closeUponClickOutsideContentArea } )
        }
        property var arguments
        property var initItemCallback
        onDialogItemChanged: {
            if(initItemCallback)
                initItemCallback(dialogItem)
            initItemCallback = undefined
        }
        opacity: !enabled || Scrite.document.busy ? 0.5 : 1

        Connections {
            target: ScriteRuntime.applicationSettings
            function onEnableAnimationsChanged() {
                modalDialog.animationsEnabled = ScriteRuntime.applicationSettings.enableAnimations
            }
        }
    }

    Component {
        id: okCancelDialogComponent

        Item {
            width: 500
            height: 250
            property string question: "Press Ok to continue."
            property string okButtonText: "Ok"
            property string cancelButtonText: "Cancel"
            property string abortButtonText
            property var    okCallback
            property var    cancelCallback

            Column {
                width: parent.width*0.8
                spacing: 40
                anchors.centerIn: parent

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: 16
                    text: question
                    horizontalAlignment: Text.AlignHCenter
                    color: ScriteRuntime.colors.accent.c50.text
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    UI.Button2 {
                        text: okButtonText
                        onClicked: {
                            if(okCallback)
                                okCallback()
                            modalDialog.closeRequest()
                        }
                    }

                    UI.Button2 {
                        text: cancelButtonText
                        onClicked: {
                            if(cancelCallback)
                                cancelCallback()
                            modalDialog.closeRequest()
                        }
                    }

                    UI.Button2 {
                        visible: text !== ""
                        text: abortButtonText
                        onClicked: modalDialog.closeRequest()
                    }
                }
            }
        }
    }

    Component {
        id: infoDialogComponent

        Item {
            width: 500
            height: 250
            property string message: "Press Ok to continue."
            property string okButtonText: "Ok"
            property var    okCallback

            Column {
                width: parent.width*0.8
                spacing: 40
                anchors.centerIn: parent

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: 16
                    text: message
                    horizontalAlignment: Text.AlignHCenter
                    color: ScriteRuntime.colors.accent.c50.text
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    UI.Button2 {
                        text: okButtonText
                        onClicked: {
                            if(okCallback)
                                okCallback()
                            modalDialog.closeRequest()
                        }
                    }
                }
            }
        }
    }

    Loader {
        active: Scrite.document.busy
        onActiveChanged: {
            if(active) {
                dialogUnderlay.radius = dialogUnderlay.maxRadius
                dialogUnderlay.show()
            } else {
                dialogUnderlay.hide()
            }
        }
        anchors.fill: parent
        sourceComponent: Item {
            Rectangle {
                anchors.fill: indication
                anchors.margins: -30
                radius: 4
                color: ScriteRuntime.colors.primary.c600.background
            }

            Row {
                id: indication
                anchors.centerIn: parent
                spacing: 20
                width: Math.min(parent.width * 0.4, implicitWidth)
                property real maxWidth: parent.width*0.4

                UI.BusyIcon {
                    id: busyIndicator
                    anchors.verticalCenter: parent.verticalCenter
                    running: true
                    width: 50; height: 50
                    forDarkBackground: true
                }

                Text {
                    width: Math.min(parent.maxWidth - busyIndicator.width - parent.spacing, contentWidth)
                    anchors.verticalCenter: parent.verticalCenter
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: Scrite.document.busyMessage
                    font.pixelSize: 16
                    color: ScriteRuntime.colors.primary.c600.text
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
            Component.onCompleted: dialogUnderlay.show()
            Component.onDestruction: dialogUnderlay.hide()
            onDone: {
                const launchHomeScreen = function() {
                    if(Scrite.user.loggedIn)
                        mainScriteDocumentView.showHomeScreen(null)
                }
                splashLoader.active = false
                if(Scrite.app.isWindowsPlatform && Scrite.app.isNotWindows10)
                    showInformation({
                        "message": "The Windows version of Scrite works best on Windows 10 or higher. While it may work on earlier versions of Windows, we don't actively test on them. We recommend that you use Scrite on PCs with Windows 10 or higher.",
                        "callback": function(value) { launchHomeScreen() }
                    })
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
        color: Scrite.app.translucent(ScriteRuntime.colors.primary.borderColor, 0.6)

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

