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

import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    id: importFromLibraryUi
    width: documentUI.width * 0.75
    height: documentUI.height * 0.85

    EventFilter.target: app
    EventFilter.events: [6] // KeyPress
    EventFilter.onFilter: {
        if( event.key === Qt.Key_Escape) {
            result.acceptEvent = true
            result.filter = true
            modalDialog.close()
        }
    }

    Column {
        id: titleBar
        width: parent.width
        y: 30

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -height*0.22
                height: subtitle.height * 1.75
                fillMode: Image.PreserveAspectFit
                smooth: true; mipmap: true
                source: "../images/library.png"
            }

            Text {
                id: subtitle
                font.pointSize: Screen.devicePixelRatio > 1 ? 22 : 18
                text: "-  Repository of Screenplays in Scrite Format"
                color: primaryColors.c300.text
            }
        }

        Text {
            text: "The repository consists of curated screenplays either directly contributed by their respective copyright owners or sourced from publicly available screenplay repositories. In all cases, <u>the copyright of the works rests with its respective owners only</u>. Read the complete <a href=\"disclaimer\">disclaimer</a> here."
            font.pointSize: app.idealFontPointSize-2
            width: parent.width * 0.9
            wrapMode: Text.WordWrap
            anchors.horizontalCenter: parent.horizontalCenter
            color: primaryColors.c300.text

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if(parent.linkAt(mouse.x, mouse.y) === "disclaimer") {
                        Qt.openUrlExternally("https://www.scrite.io/index.php/disclaimer/")
                    }
                }
            }
        }
    }

    LibraryService {
        id: libraryService
        onImported: {
            app.execLater(libraryService, 250, function() {
                modalDialog.close()
            })
        }
    }

    Rectangle {
        anchors.fill: libraryGridView
        color: primaryColors.c50.background
        border.width: 1
        border.color: primaryColors.borderColor
    }

    GridView {
        id: libraryGridView
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: buttonsRow.top
        anchors.margins: 15
        model: libraryService.library
        rightMargin: contentHeight > height ? 15 : 0
        highlightMoveDuration: 0
        clip: true
        ScrollBar.vertical: ScrollBar {
            policy: (libraryGridView.contentHeight > libraryGridView.height) ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.25)
                dark: Qt.rgba(0,0,0,0.75)
            }
            opacity: active ? 1 : 0.2
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }
        }

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
            width: libraryGridView.cellWidth
            height: libraryGridView.cellHeight
            property color textColor: libraryGridView.currentIndex === index ? primaryColors.highlight.text : "black"
            z: mouseArea.containsMouse ? 2 : 1

            Rectangle {
                visible: toolTipVisibility.get
                width: libraryGridView.cellWidth * 2.5
                height: description.contentHeight + 20
                color: primaryColors.c600.background

                DelayedPropertyBinder {
                    id: toolTipVisibility
                    initial: false
                    set: mouseArea.containsMouse
                    delay: mouseArea.containsMouse && libraryGridView.currentIndex !== index  ? 1000 : 0
                }

                onVisibleChanged: {
                    if(visible === false)
                        return

                    var referencePoint = parent.mapToItem(libraryGridView, parent.width/2,0)
                    if(referencePoint.y - height >= 0)
                        y = -height
                    else
                        y = parent.height

                    var hasSpaceOnLeft = referencePoint.x - width/2 > 0
                    var hasSpaceOnRight = referencePoint.x + width/2 < libraryGridView.width
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
                    font.pointSize: app.idealFontPointSize
                    anchors.centerIn: parent
                    wrapMode: Text.WordWrap
                    color: primaryColors.c600.text
                    text: record.logline + "<br/><br/>" +
                          "<strong>Revision:</strong> " + record.revision + "<br/>" +
                          "<strong>Copyright:</strong> " + record.copyright + "<br/>" +
                          "<strong>Source:</strong> " + record.source
                }
            }

            Rectangle {
                anchors.fill: parent
                color: primaryColors.c200.background
                visible: mouseArea.containsMouse && libraryGridView.currentIndex !== index
                border.width: 1
                border.color: primaryColors.borderColor
            }

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 10
                width: parent.width-20
                clip: true
                onHeightChanged: libraryGridView.cellHeight = Math.max(libraryGridView.cellHeight,height+20)
                spacing: 10

                Image {
                    id: poster
                    width: libraryGridView.posterWidth
                    height: libraryGridView.posterHeight
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: libraryService.library.baseUrl + "/" + record.poster

                    Rectangle {
                        anchors.fill: parent
                        color: primaryColors.button.background
                        visible: parent.status !== Image.Ready
                    }

                    BusyIndicator {
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
                        font.pointSize: app.idealFontPointSize
                        font.bold: true
                        text: record.name
                        width: parent.width
                        wrapMode: Text.WordWrap
                        color: textColor
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        font.pointSize: app.idealFontPointSize-1
                        text: record.authors
                        width: parent.width
                        wrapMode: Text.WordWrap
                        color: textColor
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        font.pointSize: app.idealFontPointSize-3
                        text: record.pageCount + " Pages"
                        width: parent.width
                        elide: Text.ElideRight
                        color: textColor
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            MouseArea {
                id: mouseArea
                hoverEnabled: true
                anchors.fill: parent
                onClicked: libraryGridView.currentIndex = index
                onDoubleClicked: {
                    libraryGridView.currentIndex = index
                    openButton.click()
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: libraryGridView
        running: libraryService.library.busy
    }

    Item {
        id: buttonsRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Math.max(rightButtons.height, leftButtons.height)
        anchors.margins: 20

        Row {
            id: leftButtons
            spacing: 20
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left

            Button2 {
                text: "Reload"
                onClicked: {
                    libraryGridView.currentIndex = -1
                    libraryService.library.reload()
                }
            }
        }

        Text {
            text: "" + libraryService.library.count + " Screenplays Available"
            font.pointSize: app.idealFontPointSize
            anchors.left: leftButtons.right
            anchors.right: rightButtons.left
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignHCenter
            visible: !libraryService.library.busy && libraryGridView.contentHeight > libraryGridView.height
        }

        Row {
            id: rightButtons
            spacing: 20
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right

            Button2 {
                text: "Cancel"
                onClicked: modalDialog.close()
            }

            Button2 {
                id: openButton
                text: "Open"
                enabled: libraryGridView.currentIndex >= 0 && !libraryService.library.busy
                function click() {
                    importFromLibraryUi.enabled = false
                    libraryService.openLibraryRecordAt(libraryGridView.currentIndex)
                }
                onClicked: click()
            }
        }
    }

    Rectangle {
        color: primaryColors.windowColor
        opacity: 0.5
        visible: !parent.enabled
        anchors.fill: parent

        BusyIndicator {
            anchors.centerIn: parent
        }
    }
}
