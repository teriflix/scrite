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
import QtGraphicalEffects 1.0

import "./qml" as UI

Rectangle {
    id: window
    width: 1024
    height: 768
    color: "lightgray"

    UI.ScriteDocumentView {
        id: ui
        anchors.fill: parent
    }

    property SystemPalette systemPalette: SystemPalette { }

    Item {
        id: blur
        anchors.fill: ui
        property color color: systemPalette.window

        property real maxRadius: 32
        property real radius: maxRadius
        visible: false
        onVisibleChanged: {
            if(!visible)
                color = systemPalette.window
        }

        FastBlur {
            anchors.fill: parent
            source: ui
            radius: parent.radius
        }

        Rectangle {
            anchors.fill: parent
            color: parent.color
            opacity: 0.6 * (parent.radius/parent.maxRadius)
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
        category: "Structure Canvas"

        property bool showGrid: true
        property color gridColor: "darkgray"
        property color canvasColor: "#F8ECC2"
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
            item.question = params.question
            item.okButtonText = params.okButtonText
            item.cancelButtonText = params.cancelButtonText
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
        backgroundColor: "gray"
        onCloseRequest: {
            active = false
            closeable = true
        }
        property var arguments
        property var initItemCallback
        onDialogItemChanged: {
            if(initItemCallback)
                initItemCallback(dialogItem)
            initItemCallback = undefined
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
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: okButtonText
                        onClicked: {
                            if(okCallback)
                                okCallback()
                            modalDialog.closeRequest()
                        }
                    }

                    Button {
                        text: cancelButtonText
                        onClicked: {
                            if(cancelCallback)
                                cancelCallback()
                            modalDialog.closeRequest()
                        }
                    }
                }
            }
        }
    }

    Item {
        property AutoUpdate autoUpdate: app.autoUpdate

        Notification.active: autoUpdate.updateAvailable
        Notification.title: "Update Available"
        Notification.text: "Scrite " + autoUpdate.updateInfo.versionString + " is now available for download. <font size=\"-1\"><i>[<strong>What's new?</strong> " + autoUpdate.updateInfo.changeLog + "]</i></font>"
        Notification.buttons: ["Download", "Ignore"]
        Notification.onButtonClicked: {
            if(index === 0)
                Qt.openUrlExternally(autoUpdate.updateDownloadUrl)
        }
    }

    UI.NotificationsView {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.7
    }
}

