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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Item {
    width: 640
    height: Math.min(documentUI.height*0.9, 550)

    signal openRequest(string filePath)

    Item {
        anchors.fill: parent
        anchors.margins: 20

        Text {
            id: title
            font.pointSize: Screen.devicePixelRatio > 1 ? 22 : 18
            text: "Select A Document To Restore"
            color: primaryColors.c200.text
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            anchors.top: title.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonBar.top
            anchors.topMargin: 20
            anchors.bottomMargin: 20
            color: primaryColors.c200.background
            border.width: 1
            border.color: primaryColors.borderColor

            ListView {
                id: documentsView
                clip: true
                anchors.fill: parent
                anchors.margins: 1
                model: Scrite.vault
                FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                currentIndex: -1
                ScrollBar.vertical: ScrollBar2 { flickable: documentsView }
                highlight: Rectangle {
                    color: primaryColors.highlight.background
                }
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                property string currentDocumentFilePath: ""
                delegate: Item {
                    width: documentsView.width
                    height: textLabel.height + 20

                    Text {
                        id: textLabel
                        width: parent.width-20
                        anchors.verticalCenter: parent.verticalCenter
                        property string fileSizeInfo: {
                            if(fileSize < 1024)
                                return fileSize + " B"
                            if(fileSize < 1024*1024)
                                return Math.round(fileSize / 1024, 2) + " KB"
                            return Math.round(fileSize / (1024*1024), 2) + " MB"
                        }
                        property string metaDataInfo: "<b>" + screenplayTitle + "</b> (" + numberOfScenes + (numberOfScenes === 1 ? " Scene" : " Scenes") + ")";
                        text: metaDataInfo + "<br/><font size=\"-1\">" + fileSizeInfo + ", " + relativeTime + " @ " + timestampAsString + "</font>"
                        leftPadding: 10
                        rightPadding: 10
                        elide: Text.ElideRight
                        font.pointSize: Scrite.app.idealFontPointSize
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            documentsView.currentDocumentFilePath = filePath
                            documentsView.currentIndex = index
                        }
                        onDoubleClicked: {
                            documentsView.currentDocumentFilePath = filePath
                            documentsView.currentIndex = index
                            busyOverlay.visible = true
                            Qt.callLater(openRequest, documentsView.currentDocumentFilePath)
                        }
                    }
                }
            }
        }

        Item {
            id: buttonBar
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: parent.width
            height: cancelButton.height

            Button2 {
                id: cancelButton
                text: "Cancel"
                onClicked: modalDialog.close()
                anchors.left: parent.left
            }

            Button2 {
                text: "Clear"
                visible: Scrite.vault.documentCount > 0
                anchors.centerIn: parent
                onClicked: clearConfirmationOverlay.visible = true
            }

            Button2 {
                anchors.right: parent.right
                text: "Restore"
                enabled: documentsView.currentDocumentFilePath !== ""
                onClicked: {
                    busyOverlay.visible = true
                    Qt.callLater(openRequest, documentsView.currentDocumentFilePath)
                }
            }
        }
    }

    Item {
        id: clearConfirmationOverlay
        anchors.fill: parent
        visible: false

        Rectangle {
            anchors.fill: parent
            color: primaryColors.windowColor
            opacity: 0.9
        }

        Column {
            width: parent.width * 0.8
            spacing: 30
            anchors.centerIn: parent

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Scrite.app.idealFontPointSize + 2
                font.bold: true
                text: "Are you sure you want to clear all restore points?"
            }

            Row {
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                Button2 {
                    text: "Yes"
                    onClicked: {
                        Scrite.vault.clearAllDocuments()
                        clearConfirmationOverlay.visible = false
                    }
                }

                Button2 {
                    text: "No"
                    onClicked: clearConfirmationOverlay.visible = false
                }
            }
        }
    }

    BusyOverlay {
        id: busyOverlay
        anchors.fill: parent
        busyMessage: "Restoring ..."
    }
}
