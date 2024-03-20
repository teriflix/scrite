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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch() {
        var dlg = dialogComponent.createObject(root)
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        Scrite.app.log("Couldn't launch BackupsDialog")
        return null
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            title: "Select a Backup to Load"
            width: 640
            height: Math.min(Scrite.window.height*0.9, 550)

            content: Item {
                ColumnLayout {
                    spacing: 20
                    anchors.fill: parent
                    anchors.margins: 20

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        color: Runtime.colors.primary.c200.background
                        border.width: 1
                        border.color: Runtime.colors.primary.borderColor

                        ListView {
                            id: backupFilesView
                            clip: true
                            anchors.fill: parent
                            anchors.margins: 1
                            model: Scrite.document.backupFilesModel
                            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                            currentIndex: -1
                            ScrollBar.vertical: VclScrollBar { flickable: backupFilesView }
                            highlight: Rectangle {
                                color: Runtime.colors.primary.highlight.background
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

                                    VclText {
                                        width: parent.width * 0.75
                                        text: relativeTime + "<br/><font size=\"-2\">" + timestampAsString + "</font>"
                                        padding: 5
                                        leftPadding: 12
                                        elide: Text.ElideRight
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                        anchors.top: parent.top
                                    }

                                    VclText {
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
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
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

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 20

                        VclButton {
                            text: "Open in This Window"
                            enabled: backupFilesView.currentIndex >= 0
                            hoverEnabled: true
                            ToolTip.visible: hovered
                            ToolTip.text: "Closes the current document and loads the selected backup."
                            onClicked: {
                                const filePath = backupFilesView.currentBackupFilePath

                                Runtime.loadMainUiContent = false
                                var waitDialog = WaitDialog.launch()
                                Scrite.document.openAnonymously(filePath)
                                waitDialog.closed.connect( () => {
                                                                 Runtime.loadMainUiContent = true
                                                                 dialog.close()
                                                             } )
                                waitDialog.closeLater(50)
                            }
                        }

                        VclButton {
                            text: "Open in New Window"
                            enabled: backupFilesView.currentIndex >= 0
                            hoverEnabled: true
                            ToolTip.visible: hovered
                            ToolTip.text: "Loads the selected backup in a new window."
                            onClicked: {
                                const filePath = backupFilesView.currentBackupFilePath

                                var waitDialog = WaitDialog.launch()
                                Scrite.app.launchNewInstanceAndOpenAnonymously(Scrite.window, filePath)
                                waitDialog.closed.connect( () => {
                                                                 dialog.close()
                                                             } )
                                waitDialog.closeLater(1500)
                            }
                        }
                    }
                }
            }
        }
    }
}
