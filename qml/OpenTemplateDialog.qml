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
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Item {
    id: newFileDialog
    width: documentUI.width * 0.75
    height: documentUI.height * 0.85

    signal importStarted()
    signal importFinished()
    signal importCancelled()

    Component.onCompleted: modalDialog.closeOnEscape = true

    AppFeature {
        id: templateAppFeature
        feature: Scrite.TemplateFeature
    }

    LibraryService {
        id: libraryService
        onImportStarted: {
            busyOverlay.busyMessage = "Loading \"" + libraryService.templates.recordAt(index).name + "\" Template ..."
            busyOverlay.visible = true
            newFileDialog.importStarted()
        }
        onImportFinished: {
            newFileDialog.importFinished()
            Scrite.app.execLater(libraryService, 250, function() {
                modalDialog.close()
            })
        }
    }

    Item {
        anchors.fill: parent
        anchors.margins: 20

        Column {
            id: titleText
            spacing: 10
            width: parent.width

            Text {
                font.pointSize: 24
                text: "Select A Template"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                font.pointSize: Scrite.app.idealFontPointSize-2
                text: "Templates in Scriptalay capture popular structures of screenplays so you can build your own work by leveraging those structures. If you want to contribute templates, please post a message on our Discord server."
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                width: parent.width
            }
        }

        Rectangle {
            color: primaryColors.c50.background
            border.width: 1
            border.color: primaryColors.borderColor
            anchors.top: titleText.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonRow.top
            anchors.topMargin: 10
            anchors.bottomMargin: 10

            GridView {
                id: templatesGridView
                enabled: templateAppFeature.enabled
                opacity: enabled ? 1 : 0.5
                anchors.fill: parent
                anchors.margins: 2
                model: libraryService.templates
                rightMargin: contentHeight > height ? 15 : 0
                highlightMoveDuration: 0
                clip: true
                ScrollBar.vertical: ScrollBar2 { flickable: templatesGridView }
                property real availableWidth: width-rightMargin
                readonly property real posterWidth: 80 * 1.75
                readonly property real posterHeight: 120 * 1.5
                readonly property real minCellWidth: posterWidth * 1.5
                property int nrCells: Math.floor(availableWidth/minCellWidth)
                cellWidth: availableWidth / nrCells
                cellHeight: posterHeight + 20

                highlight: Rectangle {
                    color: primaryColors.highlight.background
                }

                delegate: Item {
                    width: templatesGridView.cellWidth
                    height: templatesGridView.cellHeight
                    property color textColor: templatesGridView.currentIndex === index ? primaryColors.highlight.text : "black"
                    z: mouseArea.containsMouse ? 2 : 1

                    Rectangle {
                        visible: toolTipVisibility.get
                        width: templatesGridView.cellWidth * 2.5
                        height: description.contentHeight + 20
                        color: primaryColors.c600.background

                        DelayedPropertyBinder {
                            id: toolTipVisibility
                            initial: false
                            set: mouseArea.containsMouse
                            delay: mouseArea.containsMouse && templatesGridView.currentIndex !== index  ? 1000 : 0
                        }

                        onVisibleChanged: {
                            if(visible === false)
                                return

                            var referencePoint = parent.mapToItem(templatesGridView, parent.width/2,0)
                            if(referencePoint.y - height >= 0)
                                y = -height
                            else
                                y = parent.height

                            var hasSpaceOnLeft = referencePoint.x - width/2 > 0
                            var hasSpaceOnRight = referencePoint.x + width/2 < templatesGridView.width
                            if(hasSpaceOnLeft && hasSpaceOnRight)
                                x = (parent.width - width)/2
                            else if(!hasSpaceOnLeft)
                                x = 0
                            else
                                x = parent.width - width
                        }

                        Text {
                            id: description
                            width: parent.width-20
                            font.pointSize: Scrite.app.idealFontPointSize
                            anchors.centerIn: parent
                            wrapMode: Text.WordWrap
                            color: primaryColors.c600.text
                            text: record.description
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: primaryColors.c200.background
                        visible: mouseArea.containsMouse && templatesGridView.currentIndex !== index
                        border.width: 1
                        border.color: primaryColors.borderColor
                    }

                    MouseArea {
                        id: mouseArea
                        hoverEnabled: true
                        anchors.fill: parent
                        onClicked: templatesGridView.currentIndex = index
                        onDoubleClicked: {
                            templatesGridView.currentIndex = index
                            newFileDialog.enabled = false
                            libraryService.openTemplateAt(templatesGridView.currentIndex)
                        }
                    }

                    Column {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 10
                        width: parent.width-20
                        clip: true
                        onHeightChanged: templatesGridView.cellHeight = Math.max(templatesGridView.cellHeight,height+20)
                        spacing: 10

                        Image {
                            id: poster
                            width: templatesGridView.posterWidth
                            height: templatesGridView.posterHeight
                            fillMode: Image.PreserveAspectCrop
                            smooth: true
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: index === 0 ? record.poster : libraryService.templates.baseUrl + "/" + record.poster

                            Rectangle {
                                anchors.fill: parent
                                color: primaryColors.button.background
                                visible: parent.status !== Image.Ready
                            }

                            BusyIcon {
                                anchors.centerIn: parent
                                running: parent.status === Image.Loading
                                opacity: 0.5
                            }
                        }

                        Column {
                            id: metaData
                            width: parent.width
                            spacing: 5

                            Text {
                                font.pointSize: Scrite.app.idealFontPointSize
                                font.bold: true
                                text: record.name
                                width: parent.width
                                wrapMode: Text.WordWrap
                                color: textColor
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                font.pointSize: Scrite.app.idealFontPointSize-1
                                text: record.authors
                                width: parent.width
                                wrapMode: Text.WordWrap
                                color: textColor
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                font.pointSize: Scrite.app.idealFontPointSize-3
                                text: "More Info"
                                font.underline: true
                                width: parent.width
                                elide: Text.ElideRight
                                color: "blue"
                                horizontalAlignment: Text.AlignHCenter
                                visible: record.more_info !== ""

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    hoverEnabled: true
                                    onClicked: Qt.openUrlExternally(record.more_info)
                                }
                            }
                        }
                    }
                }
            }

            DisabledFeatureNotice {
                anchors.fill: parent
                color: Qt.rgba(1,1,1,0.9)
                featureName: "New Document From Template"
                visible: !templateAppFeature.enabled
            }
        }

        Item {
            id: buttonRow
            height: Math.max(reloadButton.height, okCancelButtons.height)
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Button2 {
                id: reloadButton
                text: "Reload"
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                onClicked: libraryService.reload()
            }

            Row {
                id: okCancelButtons
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right

                Button2 {
                    text: "Cancel"
                    onClicked: {
                        newFileDialog.importCancelled()
                        modalDialog.close()
                    }
                }

                Button2 {
                    text: "OK"
                    enabled: templatesGridView.currentIndex === 0 || templateAppFeature.enabled
                    onClicked: {
                        newFileDialog.enabled = false
                        libraryService.openTemplateAt(templatesGridView.currentIndex)
                    }
                }
            }
        }
    }

    enabled: !libraryService.busy

    BusyOverlay {
        id: busyOverlay
        anchors.fill: parent
        busyMessage: "Loading templates..."
        visible: libraryService.busy
    }
}
