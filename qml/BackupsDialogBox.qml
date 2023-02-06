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

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import io.scrite.components 1.0
import "../js/utils.js" as Utils

Item {
    width: 640
    height: Math.min(documentUI.height*0.9, 550)

    signal openInThisWindow(string filePath)
    signal openInNewWindow(string filePath)

    Item {
        anchors.fill: parent
        anchors.margins: 20

        Row {
            id: title
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                source: "../icons/file/backup_open.png"
                width: 36; height: width; mipmap: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                font.pointSize: Screen.devicePixelRatio > 1 ? 22 : 18
                text: "Select A Backup To Load"
                color: primaryColors.c200.text
                anchors.verticalCenter: parent.verticalCenter
            }
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
                id: backupFilesView
                clip: true
                anchors.fill: parent
                anchors.margins: 1
                model: Scrite.document.backupFilesModel
                FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                currentIndex: -1
                ScrollBar.vertical: ScrollBar2 { flickable: backupFilesView }
                highlight: Rectangle {
                    color: primaryColors.highlight.background
                }
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                property string currentBackupFilePath
                delegate: Item {
                    width: backupFilesView.width
                    height: rowLayout.height + 10

                    Row {
                        id: rowLayout
                        width: parent.width-20
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            width: parent.width * 0.75
                            text: relativeTime + "<br/><font size=\"-2\">" + timestampAsString + "</font>"
                            padding: 5
                            leftPadding: 12
                            elide: Text.ElideRight
                            font.pointSize: Scrite.app.idealFontPointSize
                            anchors.top: parent.top
                        }

                        Text {
                            width: parent.width * 0.25
                            anchors.top: parent.top
                            property string fileSizeInfo: {
                                if(fileSize < 1024)
                                    return fileSize + " B"
                                if(fileSize < 1024*1024)
                                    return Math.round(fileSize / 1024, 2) + " KB"
                                return Math.round(fileSize / (1024*1024), 2) + " MB"
                            }
                            property string metaDataInfo: {
                                if(metaData.loaded)
                                    return metaData.sceneCount + (metaData.sceneCount === 1 ? " Scene" : " Scenes");
                                return "Loading metadata ..."
                            }
                            text: metaDataInfo + "<br/><font size=\"-2\">" + fileSizeInfo + "</font>"
                            padding: 5
                            elide: Text.ElideRight
                            font.pointSize: Scrite.app.idealFontPointSize
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            backupFilesView.currentBackupFilePath = filePath
                            backupFilesView.currentIndex = index
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

            Row {
                spacing: 20
                anchors.right: parent.right

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Open In: "
                    font.pointSize: Scrite.app.idealFontPointSize
                }

                Button2 {
                    text: "This Window"
                    enabled: backupFilesView.currentIndex >= 0
                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Closes the current document and loads the selected backup."
                    onClicked: {
                        busyOverlay.visible = true
                        Utils.execLater(busyOverlay, 50, function() {
                            openInThisWindow(backupFilesView.currentBackupFilePath)
                        })
                    }
                }

                Button2 {
                    text: "New Window"
                    enabled: backupFilesView.currentIndex >= 0
                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Loads the selected backup in a new window."
                    onClicked: {
                        busyOverlay.visible = true
                        Utils.execLater(busyOverlay, 50, function() {
                            openInNewWindow(backupFilesView.currentBackupFilePath)
                        })
                    }
                }
            }
        }
    }

    BusyOverlay {
        id: busyOverlay
        anchors.fill: parent
        busyMessage: "Opening Backup ..."
    }
}
