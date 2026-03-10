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

pragma Singleton
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../tasks"

import "../globals"
import "../helpers"
import "../controls"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "BackupsDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

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
                        id: _backupFilesView
                        clip: true
                        anchors.fill: parent
                        anchors.margins: 1
                        model: Scrite.document.backupFilesModel
                        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                        currentIndex: -1
                        ScrollBar.vertical: VclScrollBar { flickable: _backupFilesView }
                        highlight: Rectangle {
                            color: Runtime.colors.primary.highlight.background
                        }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0
                        property string currentBackupFilePath
                        delegate: Item {
                            id: _backupFilesViewDelegate

                            required property int index
                            required property int timestamp
                            required property int fileSize

                            required property var metaData

                            required property string timestampAsString
                            required property string relativeTime
                            required property string fileName
                            required property string filePath

                            width: _backupFilesView.width
                            height: _rowLayout.height + 10

                            Row {
                                id: _rowLayout
                                width: parent.width-20
                                anchors.verticalCenter: parent.verticalCenter

                                VclLabel {
                                    width: parent.width * 0.75
                                    text: relativeTime + "<br/><font size=\"-2\">" + _backupFilesViewDelegate.timestampAsString + "</font>"
                                    padding: 5
                                    leftPadding: 12
                                    elide: Text.ElideRight
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    anchors.top: parent.top
                                }

                                VclLabel {
                                    width: parent.width * 0.25
                                    anchors.top: parent.top
                                    property string fileSizeInfo: {
                                        if(_backupFilesViewDelegate.fileSize < 1024)
                                            return _backupFilesViewDelegate.fileSize + " B"
                                        if(_backupFilesViewDelegate.fileSize < 1024*1024)
                                            return Math.round(_backupFilesViewDelegate.fileSize / 1024, 2) + " KB"
                                        return Math.round(_backupFilesViewDelegate.fileSize / (1024*1024), 2) + " MB"
                                    }
                                    property string metaDataInfo: {
                                        if(_backupFilesViewDelegate.metaData.loaded)
                                            return _backupFilesViewDelegate.metaData.sceneCount + (_backupFilesViewDelegate.metaData.sceneCount === 1 ? " Scene" : " Scenes");
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
                                    _backupFilesView.currentBackupFilePath = _backupFilesViewDelegate.filePath
                                    _backupFilesView.currentIndex = _backupFilesViewDelegate.index
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
                        enabled: _backupFilesView.currentIndex >= 0
                        hoverEnabled: true
                        toolTipText: "Closes the current document and loads the selected backup."

                        onClicked: {
                            var task = OpenFileTask.openAnonymously(_backupFilesView.currentBackupFilePath)
                            task.finished.connect(_dialog.close)
                        }
                    }

                    VclButton {
                        text: "Open in New Window"
                        enabled: _backupFilesView.currentIndex >= 0
                        hoverEnabled: true
                        toolTipText: "Loads the selected backup in a new window."

                        onClicked: {
                            const filePath = _backupFilesView.currentBackupFilePath

                            var waitDialog = WaitDialog.launch()
                            Scrite.app.launchNewInstanceAndOpenAnonymously(Scrite.window, filePath)
                            Runtime.execLater(_dialog, 1500, () => {
                                                Qt.callLater(_dialog.close)
                                                if(waitDialog)
                                                waitDialog.close()
                                            } )
                        }
                    }
                }
            }
        }
    }
}
