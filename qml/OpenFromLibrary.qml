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
    id: importFromLibraryUi
    width: documentUI.width * 0.75
    height: documentUI.height * 0.85

    signal importStarted()
    signal importFinished()
    signal importCancelled()

    Component.onCompleted: modalDialog.closeOnEscape = true

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: pageView.top
        color: primaryColors.c200.background
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
                text: "-  Repository of Screenplays & Templates in Scrite Format"
                color: primaryColors.c200.text
            }
        }

        Text {
            text: pageView.pagesArray[pageView.currentIndex].disclaimer
            font.pointSize: Scrite.app.idealFontPointSize-2
            width: parent.width * 0.9
            wrapMode: Text.WordWrap
            anchors.horizontalCenter: parent.horizontalCenter
            color: primaryColors.c200.text

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var linkUrl = parent.linkAt(mouse.x, mouse.y)
                    Qt.openUrlExternally(linkUrl)
                }
            }
        }
    }

    LibraryService {
        id: libraryService
        onImportStarted: {
            var library = pageView.currentIndex === 0 ? libraryService.screenplays : libraryService.templates
            busyOverlay.busyMessage = "Loading \"" + library.recordAt(index).name + "\" " + pageView.pagesArray[pageView.currentIndex].kind + " ..."
            busyOverlay.visible = true
            importFromLibraryUi.importStarted()
        }
        onImportFinished: {
            Scrite.app.execLater(libraryService, 250, function() {
                importFromLibraryUi.importFinished()
                modalDialog.close()
            })
        }
    }

    PageView {
        id: pageView
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 18
        pageListWidth: 180

        pagesArray: [
            { "kind": "Screenplay", "title": "Screenplays", "disclaimer": "Screenplays in Scriptalay consists of curated works either directly contributed by their respective copyright owners or sourced from publicly available screenplay repositories. In all cases, <u>the copyright of the works rests with its respective owners only</u> - <a href=\"https://www.scrite.io/index.php/disclaimer/\">disclaimer</a>.", appFeature: Scrite.ScriptalayFeature },
            { "kind": "Template", "title": "Templates", "disclaimer": "Templates in Scriptalay capture popular structures of screenplays so you can build your own work by leveraging those structures. If you want to contribute templates, please post a message on our Discord server.", appFeature: Scrite.TemplateFeature }
        ]

        pageTitleRole: "title"

        currentIndex: 0

        cornerContent: Item {
            Button2 {
                text: "Reload"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                onClicked: {
                    if(pageView.currentIndex === 0)
                        libraryService.screenplays.reload()
                    else
                        libraryService.templates.reload()
                }
            }
        }

        pageContent: Item {
            height: pageView.height

            AppFeature {
                id: appFeatureCheck
                feature: pageView.pagesArray[pageView.currentIndex].appFeature
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: buttonsRow.top
                anchors.right: parent.right
                anchors.margins: 20
                anchors.leftMargin: 0
                color: primaryColors.c50.background
                border.color: primaryColors.borderColor
                border.width: 1

                Loader {
                    id: loader
                    anchors.fill: parent
                    anchors.margins: 3
                    enabled: appFeatureCheck.enabled
                    opacity: enabled ? 1 : 0.5
                    sourceComponent: {
                        switch(pageView.currentIndex) {
                        case 0: return scriptalayComponent
                        case 1: return templatesComponent
                        }
                    }
                }

                DisabledFeatureNotice {
                    anchors.fill: parent
                    color: Qt.rgba(1,1,1,0.9)
                    featureName: "Loading " + pageView.pagesArray[pageView.currentIndex].kind + " From Scriptalay"
                    visible: !appFeatureCheck.enabled
                }
            }

            Item {
                id: buttonsRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: rightButtons.height
                anchors.margins: 20

                Text {
                    text: loader.item.statusText
                    font.pointSize: Scrite.app.idealFontPointSize
                    anchors.left: parent.left
                    anchors.right: rightButtons.left
                    anchors.verticalCenter: parent.verticalCenter
                    visible: !libraryService.busy
                }

                Row {
                    id: rightButtons
                    spacing: 20
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right

                    Button2 {
                        text: "Cancel"
                        onClicked: {
                            importFromLibraryUi.importCancelled()
                            modalDialog.close()
                        }
                    }

                    Button2 {
                        id: openButton
                        text: "Open"
                        enabled: (loader.item && loader.item.somethingIsSelected) && !libraryService.busy && loader.enabled
                        function click() {
                            importFromLibraryUi.enabled = false
                            loader.item.openSelected()
                        }
                        onClicked: click()
                    }
                }
            }


        }
    }

    Component {
        id: scriptalayComponent

        Item {
            property bool somethingIsSelected: libraryGridView.currentIndex >= 0

            property string statusText: "" + libraryService.screenplays.count + " Screenplays Available"

            function openSelected() {
                libraryService.openScreenplayAt(libraryGridView.currentIndex)
            }

            GridView {
                id: libraryGridView
                anchors.fill: parent
                model: libraryService.screenplays
                rightMargin: contentHeight > height ? 15 : 0
                highlightMoveDuration: 0
                clip: true
                ScrollBar.vertical: ScrollBar2 { flickable: libraryGridView }
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
                    z: mouseArea.containsMouse || libraryGridView.currentIndex === index ? 2 : 1

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
                            font.pointSize: Scrite.app.idealFontPointSize
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
                            source: libraryService.screenplays.baseUrl + "/" + record.poster

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
                            importFromLibraryUi.enabled = false
                            libraryService.openScreenplayAt(libraryGridView.currentIndex)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: templatesComponent

        Item {
            property bool somethingIsSelected: templatesGridView.currentIndex >= 0

            property string statusText: "" + libraryService.templates.count + " Templates Available"

            function openSelected() {
                libraryService.openTemplateAt(templatesGridView.currentIndex)
            }

            GridView {
                id: templatesGridView
                anchors.fill: parent
                model: libraryService.templates
                rightMargin: contentHeight > height ? 15 : 0
                highlightMoveDuration: 0
                clip: true
                ScrollBar.vertical: ScrollBar2 { flickable: templatesGridView  }

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
                    z: mouseArea.containsMouse || templatesGridView.currentIndex === index ? 2 : 1

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
                            importFromLibraryUi.enabled = false
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
        }
    }

    BusyOverlay {
        id: busyOverlay
        anchors.fill: parent
        busyMessage: "Loading library..."
        visible: libraryService.busy
    }
}
