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

import "qrc:/qml/tasks"

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "EditRecentFilesDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        title: "Modify Recent Files"
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
                        id: recentFilesView

                        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                        ScrollBar.vertical: VclScrollBar {
                            id: recentFilesViewVScrollBar
                            flickable: recentFilesView
                        }

                        anchors.fill: parent
                        anchors.margins: 1

                        clip: true
                        model: Runtime.recentFiles
                        spacing: 10
                        currentIndex: 0

                        highlight: Rectangle {
                            color: Runtime.colors.primary.highlight.background
                        }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0

                        delegate: Item {
                            required property int index
                            required property var fileInfo

                            width: recentFilesView.width
                            height: delegateLayout.height+20

                            MouseArea {
                                anchors.fill: parent
                                onClicked: recentFilesView.currentIndex = index
                            }

                            RowLayout {
                                id: delegateLayout

                                anchors.left: parent.left
                                anchors.leftMargin: 10
                                anchors.verticalCenter: parent.verticalCenter

                                width: parent.width-(recentFilesViewVScrollBar.needed ? 30 : 15)
                                spacing: 10

                                Rectangle {
                                    Layout.preferredWidth: 48
                                    Layout.preferredHeight: 64

                                    color: Qt.rgba(0,0,0,0)
                                    border.width: 1
                                    border.color: Runtime.colors.primary.borderColor

                                    StackLayout {
                                        anchors.fill: parent
                                        anchors.margins: 1

                                        currentIndex: fileInfo.hasCoverPage ? 1 : 0

                                        Image {
                                            source: "qrc:/icons/filetype/document.png"
                                            fillMode: Image.PreserveAspectFit
                                        }

                                        QImageItem {
                                            image: fileInfo.hasCoverPage ? fileInfo.coverPageImage : Scrite.app.emptyQImage
                                            fillMode: QImageItem.PreserveAspectFit
                                            useSoftwareRenderer: Runtime.currentUseSoftwareRenderer
                                        }
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true

                                    VclLabel {
                                        Layout.fillWidth: true

                                        font.bold: true
                                        text: {
                                            const title = fileInfo.title === "" ? "Untitled Screenplay" : fileInfo.title + ""
                                            const version = fileInfo.version
                                            return version === "" ? title : (title + " <font size=\"-1\">(" + version + ")</font>")
                                        }
                                        elide: Text.ElideRight
                                    }

                                    VclLabel {
                                        Layout.fillWidth: true

                                        font.italic: true
                                        text: fontInfo.author === "" ? "<Unknown Authors>" : fileInfo.author
                                        elide: Text.ElideRight
                                    }

                                    Link {
                                        Layout.fillWidth: true

                                        text: fileInfo.filePath
                                        elide: Text.ElideMiddle
                                        enabled: !Platform.isLinuxDesktop
                                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                                        onClicked: {
                                            recentFilesView.currentIndex = index
                                            File.revealOnDesktop(fileInfo.filePath)
                                            Scrite.notifications.dismissNotification(0)
                                        }
                                    }
                                }

                                VclToolButton {
                                    icon.source: "qrc:/icons/action/delete.png"
                                    hoverEnabled: true
                                    opacity: hovered ? 1 : 0.25

                                    onClicked: Runtime.recentFiles.removeAt(index)
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    VclLabel {
                        text: "Display Mode: "
                    }

                    VclComboBox {
                        Layout.preferredWidth: 250

                        model: ["Prefer Title (Version)", "File Name"]
                        currentIndex: Runtime.recentFiles.preferTitleVersionText ? 0 : 1
                        onActivated: (index) => { Runtime.recentFiles.preferTitleVersionText = index === 0 }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    VclButton {
                        text: "Clear"

                        onClicked: {
                            MessageBox.question("Clear Confirmation", "Are you sure you want to clear your recent files list?",
                                                ["Yes", "No"], (answer) => {
                                                    if(answer === "Yes")
                                                        Runtime.recentFiles.clear()
                                                })
                        }
                    }
                }
            }
        }
    }
}
