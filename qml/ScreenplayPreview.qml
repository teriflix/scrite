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
import QtQuick.Controls 2.13

Rectangle {
    id: previewItem
    property Screenplay screenplay: scriteDocument.loading ? null : scriteDocument.screenplay
    property ScreenplayFormat screenplayFormat: scriteDocument.loading ? null : scriteDocument.printFormat
    property alias titlePage: screenplayTextDocument.titlePage
    property alias titlePageIsCentered: screenplayTextDocument.titlePageIsCentered
    property bool fitPageToWidth: false
    property alias purpose: screenplayTextDocument.purpose
    property alias contentY: pageView.contentY
    property alias contentX: pageView.contentX
    property real contentWidth: pageView.contentWidth
    property real contentHeight: pageView.contentHeight
    property real pageHeight: pageView.cellHeight
    property real lineHeight: fontMetrics.lineSpacing * previewZoomSlider.value
    property real zoomScale: previewZoomSlider.value
    readonly property real pageSpacing: fitPageToWidth ? 1 : 40

    property PrintedTextDocumentOffsets textDocumentOffsets: PrintedTextDocumentOffsets {
        timePerPage: screenplayTextDocument.timePerPage
        screenplay: previewItem.screenplay

        Notification.title: "Time Offsets Error"
        Notification.text: errorMessage
        Notification.active: hasError
        Notification.autoClose: false
        Notification.onDismissed: clearErrorMessage()
    }

    color: primaryColors.windowColor

    signal currentOffsetChanged(int row)

    Component.onCompleted: {
        app.execLater(screenplayTextDocument, 250, function() {
            textDocumentOffsets.enabled = true
            screenplay.currentElementIndex = 0
            screenplayTextDocument.print(screenplayImagePrinter)
            textDocumentOffsets.enabled = false
            Qt.callLater( function() { pageView.scrollToCurrentScene() })
        })
    }

    ScreenplayTextDocument {
        id: screenplayTextDocument
        screenplay: previewItem.screenplay
        formatting: previewItem.screenplayFormat
        sceneNumbers: true
        purpose: ScreenplayTextDocument.ForPrinting
        secondsPerPage: formatting ? formatting.secondsPerPage : 60
        syncEnabled: true
    }

    FontMetrics {
        id: fontMetrics
        font: previewItem.screenplayFormat.defaultFont
    }

    ImagePrinter {
        id: screenplayImagePrinter
        scale: 2
    }

    Connections {
        target: scriteDocument.loading ? null : scriteDocument.screenplay
        onCurrentElementIndexChanged: pageView.scrollToCurrentScene()
    }

    Text {
        id: noticeText
        font.pixelSize: 30
        anchors.centerIn: parent
        text: "Generating preview ..."
        visible: screenplayImagePrinter.printing || (screenplayImagePrinter.pageCount === 0 && screenplayTextDocument.pageCount > 0)
    }

    Flickable {
        id: pageView
        anchors.fill: parent
        anchors.bottomMargin: statusBar.visible ? statusBar.height : 0
        contentWidth: pageViewContent.width
        contentHeight: pageViewContent.height
        clip: true

        property bool lockScrollToCurrentScene: false
        function scrollToCurrentScene() {
            if(lockScrollToCurrentScene)
                return
            var idx = scriteDocument.screenplay.currentElementIndex
            if(idx < 0) return
            var element = scriteDocument.screenplay.elementAt(idx)
            if(element === null) return
            var sceneNr = element.resolvedSceneNumber
            var info = textDocumentOffsets.offsetInfoOf(sceneNr)
            if(info) {
                lockUpdateCurrentScene = true
                pageView.currentIndex = info.pageNumber-1
                pageView.contentY = (info.pageNumber-1)*pageView.cellHeight + (info.sceneHeadingRect.y)*previewZoomSlider.value
                lockUpdateCurrentScene = false
            }
        }

        property bool lockUpdateCurrentScene: false
        function updateCurrentScene() {
            if(!fitPageToWidth || textDocumentOffsets.count === 0 || lockUpdateCurrentScene)
                return
            var pageIdx = Math.floor(contentY/cellHeight)
            var yOffset = (contentY - pageIdx*cellHeight)/previewZoomSlider.value
            var info = textDocumentOffsets.nearestOffsetInfo(pageIdx+1,yOffset)
            if(scriteDocument.screenplay.currentElementIndex !== info.sceneIndex) {
                lockScrollToCurrentScene = true
                scriteDocument.screenplay.currentElementIndex = info.sceneIndex
                currentOffsetChanged(info.row)
                lockScrollToCurrentScene = false
            }
        }

        onContentYChanged: {
            if(fitPageToWidth && !lockUpdateCurrentScene)
                Qt.callLater(updateCurrentScene)
        }

        property bool containsMouse: false

        ScrollBar.horizontal: ScrollBar {
            policy: pageLayout.width > pageView.width ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.25)
                dark: Qt.rgba(0,0,0,0.75)
            }
            opacity: pageView.containsMouse ? (active ? 1 : 0.2) : 0
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }
        }

        ScrollBar.vertical: ScrollBar {
            policy: pageLayout.height > pageView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.25)
                dark: Qt.rgba(0,0,0,0.75)
            }
            opacity: pageView.containsMouse ? (active ? 1 : 0.2) : 0
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }
        }

        property real cellWidth: screenplayImagePrinter.pageWidth*previewZoomSlider.value + pageSpacing
        property real cellHeight: screenplayImagePrinter.pageHeight*previewZoomSlider.value + pageSpacing
        property int nrColumns: Math.max(Math.floor(width/cellWidth), 1)
        property int nrRows: Math.ceil(screenplayImagePrinter.pageCount / nrColumns)
        property int currentIndex: 0

        Item {
            id: pageViewContent
            width: Math.max(pageLayout.width, pageView.width)
            height: pageLayout.height

            Flow {
                id: pageLayout
                anchors.horizontalCenter: parent.horizontalCenter
                width: pageView.cellWidth * pageView.nrColumns
                height: pageView.cellHeight * pageView.nrRows

                Repeater {
                    id: pageRepeater

                    model: screenplayImagePrinter.printing ? null : screenplayImagePrinter
                    delegate: Item {
                        readonly property int pageIndex: index
                        width: pageView.cellWidth
                        height: pageView.cellHeight

                        property bool itemIsVisible: {
                            var firstRow = Math.max(Math.floor(pageView.contentY / pageView.cellHeight), 0)
                            var lastRow = Math.min(Math.ceil( (pageView.contentY+pageView.height)/pageView.cellHeight ), pageRepeater.count-1)
                            var myRow = Math.floor(pageIndex/pageView.nrColumns)
                            return firstRow <= myRow && myRow <= lastRow;
                        }

                        BoxShadow {
                            anchors.fill: pageImage
                            opacity: pageView.currentIndex === index ? 1 : 0.15
                            visible: !fitPageToWidth
                        }

                        Rectangle {
                            anchors.fill: pageImage
                            color: "white"
                        }

                        Image {
                            id: pageImage
                            width: pageWidth*previewZoomSlider.value
                            height: pageHeight*previewZoomSlider.value
                            source: parent.itemIsVisible ? pageUrl : ""
                            anchors.centerIn: parent
                            smooth: true
                            mipmap: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: pageView.currentIndex = index
                        }
                    }
                }
            }
        }

        EventFilter.acceptHoverEvents: true
        EventFilter.events: [127,128,129] // [HoverEnter, HoverLeave, HoverMove]
        EventFilter.onFilter: pageView.containsMouse = event.type === 127 || event.type === 129
    }

    Rectangle {
        id: statusBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 30
        color: primaryColors.windowColor
        border.width: 1
        border.color: primaryColors.borderColor
        visible: !fitPageToWidth

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            text: noticeText.visible ? "Generating preview ..." : ("Page " + (Math.max(pageView.currentIndex,0)+1) + " of " + pageRepeater.count)
        }

        ZoomSlider {
            id: previewZoomSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            from: 0.5
            to: pageView.width / screenplayImagePrinter.pageWidth
            value: fitPageToWidth ? to : 1
        }
    }
}
