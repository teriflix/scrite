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

import Scrite 1.0
import QtQuick 2.13
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13
// import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.12

import "./qml" as UI

Rectangle {
    id: window
    width: 1366
    height: 700
    color: primaryColors.windowColor

    MaterialColors {
        id: primaryColors
        name: "Gray"
        readonly property int key: Material.Grey
        readonly property color windowColor: c300.background
        readonly property color borderColor: c400.background
        readonly property color separatorColor: c400.background
        readonly property var highlight: c400
        readonly property var button: c200
    }

    MaterialColors {
        id: accentColors
        name: "Blue Gray"
        readonly property int key: Material.BlueGrey
        readonly property color windowColor: c300.background
        readonly property color borderColor: c400.background
        readonly property color separatorColor: c400.background
        readonly property var button: c200
    }

    Material.primary: primaryColors.key
    Material.accent: accentColors.key
    Material.theme: Material.Light
    Material.background: accentColors.c700.background

    UndoStack {
        id: mainUndoStack
        objectName: "MainUndoStack"
        property bool screenplayEditorActive: false
        property bool timelineEditorActive: false
        property bool structureEditorActive: false
        property bool sceneEditorActive: false
        active: screenplayEditorActive || timelineEditorActive || structureEditorActive || sceneEditorActive
    }

    UI.ScriteDocumentView {
        id: ui
        anchors.fill: parent
    }

    Loader {
        id: statusText
        active: false
        anchors.fill: parent
        property string text
        property bool enableAnimations: true
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
                property real t: statusText.enableAnimations ? 0 : 1
                scale: 0.5 + t/1.0
                opacity: statusText.enableAnimations ? (1.0 - t*0.75) : 0.8
            }

            SequentialAnimation {
                running: true

                NumberAnimation {
                    target: textItem
                    properties: "t"
                    from: 0
                    to: 1
                    duration: statusText.enableAnimations ? 250 : 0
                    easing.type: Easing.OutQuint
                }

                PauseAnimation {
                    duration: statusText.enableAnimations ? 0 : 250
                }

                ScriptAction {
                    script: statusText.active = false
                }
            }
        }
    }

    Item {
        id: blur
        anchors.fill: ui
        property color color: primaryColors.windowColor

        property int visibilityCounter: 0
        function show() {
            visible = true
            visibilityCounter = Math.max(visibilityCounter+1,1)
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
                color = primaryColors.windowColor
                visibilityCounter = 0
            }
        }

        /*
          // There are many graphics card drivers out there that are simply unable to fund
          // this blur effect. We have to aim for the lowest common denominator unfortunately.

        FastBlur {
            anchors.fill: parent
            source: ui
            radius: parent.radius
        }
        */

        Rectangle {
            anchors.fill: parent
            color: parent.color
            // opacity: 0.6 * (parent.radius/parent.maxRadius)
            opacity: 0.9 * (parent.radius/parent.maxRadius)
        }
    }

    Item {
        anchors.fill: blur
        visible: scriteDocument.busy
        opacity: 0.9
        onVisibleChanged: {
            if(visible) {
                blur.radius = blur.maxRadius
                blur.show()
            } else {
                blur.hide()
            }
        }
    }

    Settings {
        id: scrollAreaSettings
        fileName: app.settingsFilePath
        category: "ScrollArea"
        property real zoomFactor: 0.05
    }

    Settings {
        id: structureCanvasSettings
        fileName: app.settingsFilePath
        category: "Structure Tab"

        property bool showGrid: true
        property color gridColor: primaryColors.c400.background
        property color canvasColor: accentColors.c50.background
        property bool showPreview: true
        property bool displayAnnotationProperties: true
        property real connectorLineWidth: 2
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
        enabled: notificationManager.count === 0
        onCloseRequest: {
            active = false
            closeable = true
            closeUponClickOutsideContentArea = false
        }
        property var arguments
        property var initItemCallback
        onDialogItemChanged: {
            if(initItemCallback)
                initItemCallback(dialogItem)
            initItemCallback = undefined
        }
        opacity: !enabled || scriteDocument.busy ? 0.5 : 1
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
                    color: accentColors.c50.text
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
                    color: accentColors.c50.text
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
        active: scriteDocument.busy
        anchors.fill: parent
        sourceComponent: Item {
            Rectangle {
                anchors.fill: indication
                anchors.margins: -30
                radius: 4
                color: primaryColors.c600.background
            }

            Row {
                id: indication
                anchors.centerIn: parent
                spacing: 20
                width: Math.min(parent.width * 0.4, implicitWidth)
                property real maxWidth: parent.width*0.4

                BusyIndicator {
                    id: busyIndicator
                    anchors.verticalCenter: parent.verticalCenter
                    running: true
                    width: 50; height: 50
                    Material.accent: primaryColors.c600.text
                }

                Text {
                    width: Math.min(parent.maxWidth - busyIndicator.width - parent.spacing, contentWidth)
                    anchors.verticalCenter: parent.verticalCenter
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: scriteDocument.busyMessage
                    font.pixelSize: 16
                    color: primaryColors.c600.text
                }
            }

            MouseArea {
                anchors.fill: parent
            }

            EventFilter.target: app
            EventFilter.events: [6,7]
            EventFilter.onFilter: {
                result.filter = true
            }
        }
    }

    Item {
        property AutoUpdate autoUpdate: app.autoUpdate

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
        Notification.onButtonClicked: {
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

    UI.NotificationsView {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: -1
        width: parent.width * 0.7
        onVisibleChanged: {
            if(visible)
                blur.show()
            else
                blur.hide()
        }
    }

    Connections {
        target: qmlWindow
        onScreenChanged: scriteDocument.formatting.setSreeenFromWindow(qmlWindow)
        // onActiveFocusItemChanged: console.log("PA: " + qmlWindow.activeFocusItem)
    }

    Loader {
        id: splashLoader
        anchors.fill: parent
        sourceComponent: UI.SplashScreen {
            Component.onCompleted: blur.show()
            Component.onDestruction: blur.hide()
            onDone: {
                splashLoader.active = false
                if(app.isWindowsPlatform && app.isNotWindows10)
                    showInformation({
                        "message": "The Windows version of Scrite works best on Windows 10. While it may work on earlier versions of Windows, we don't actively test on them. We recommend that you use Scrite on Windows 10 PCs."
                    })
            }
        }
    }

    property var lastSnapshotTimestamp: 0
    EventFilter.active: app.getEnvironmentVariable("SCRITE_SNAPSHOT_CAPTURE") === "YES"
    EventFilter.target: app
    EventFilter.events: [6]
    EventFilter.onFilter: {
        if(event.key === Qt.Key_F6) {
            var timestamp = (new Date()).getTime()
            if(timestamp - lastSnapshotTimestamp > 500) {
                lastSnapshotTimestamp = timestamp
                windowCapture.start()
            }
        }
    }

    WindowCapture {
        id: windowCapture
        fileName: "scrite-window-capture.jpg"
        format: WindowCapture.JPGFormat
        forceCounterInFileName: true
        window: qmlWindow
        maxImageSize: Qt.size(1920, 1080)
    }

    Component.onCompleted: qmlWindow.raise()
}

