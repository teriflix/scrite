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

import QtQuick 2.15
import QtQuick.Pdf 5.15
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

Item {
    width: 1280
    height: 720

    property alias source: pdfDoc.source
    property alias pagesPerRow: pdfDoc.pagesPerRow
    property string saveFilePath
    property string saveFileName: Scrite.app.fileName( Scrite.app.urlToLocalFile(source) ) + ".pdf"
    property bool closable: true
    property bool allowFileSave: true
    property bool allowFileReveal: false
    property bool displayRefreshButton: false

    signal closeRequest()
    signal refreshRequest()

    function getRefreshButton() { return refreshButton }

    // Catch all mouse-area, which doesnt let mouse events
    // propagate to layers underneath this item.
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: statusBar.top
        color: primaryColors.windowColor

        PdfDocument {
            id: pdfDoc
            property int  pagesPerRow: 1
            property real minPageScale: evaluatePageScale(4)
            property real idealPageScale: evaluatePageScale(Math.min(pageCount,2))
            property real maxPageScale: evaluatePageScale(0.5)
            onIdealPageScaleChanged: pageScaleSlider.value = idealPageScale
            onStatusChanged: Qt.callLater( function() {
                if(pageCount === 1 && pagesPerRow > 1)
                    pageScaleSlider.value = pdfView.height/maxPageHeight
                else
                    pageScaleSlider.value = pdfDoc.evaluatePageScale(Math.min(pagesPerRow,pageCount))
            })

            function evaluatePageScale(nrPages) {
                return status === PdfDocument.Ready ?
                        Math.max(0.0000001,
                                 ((pdfView.width/nrPages)-(2+nrPages-1)*pdfView.spacing)/maxPageWidth) :
                        1
            }
        }

        TableView {
            id: pdfView
            clip: true
            anchors.fill: parent
            model: Math.ceil(pdfDoc.pageCount / pdfPagesPerRow)
            property real pdfPageScale: pageScaleSlider.value
            property real pdfPageWidth: pdfDoc.maxPageWidth * pdfPageScale
            property real pdfPageHeight: pdfDoc.maxPageHeight * pdfPageScale
            property real pdfPageCellWidth: pdfPageWidth + spacing
            property real pdfPageCellHeight: pdfPageHeight + spacing
            property real spacing: Math.min( width*0.05, 30 )
            property int  pdfPagesPerRow: Math.max(1,Math.floor(width/pdfPageCellWidth))
            onPdfPageCellWidthChanged: Qt.callLater(forceLayout)
            onPdfPageCellHeightChanged: Qt.callLater(forceLayout)
            rowHeightProvider: () => { return pdfPageCellHeight }
            columnWidthProvider: () => { return Math.max(width,pdfPageCellWidth*pdfPagesPerRow) }
            reuseItems: false
            ScrollBar.vertical: ScrollBar2 {
                flickable: pdfView
            }
            ScrollBar.horizontal: ScrollBar2 {
                flickable: pdfView
            }

            onPdfPageScaleChanged: Qt.callLater(returnToBounds)

            delegate: Item {
                width: pdfView.width
                height: pdfView.pdfPageCellHeight

                property int startPageIndex: index*pdfView.pdfPagesPerRow
                property int endPageIndex: Math.min(startPageIndex+pdfView.pdfPagesPerRow-1, pdfDoc.pageCount-1)

                Row {
                    height: pdfView.pdfPageHeight
                    anchors.centerIn: parent
                    spacing: pdfView.spacing

                    Repeater {
                        model: Math.max(1,(endPageIndex-startPageIndex)+1)

                        Rectangle {
                            width: pdfView.pdfPageWidth
                            height: pdfView.pdfPageHeight

                            Image {
                                id: pdfPage
                                anchors.fill: parent
                                sourceSize {
                                    width: pdfDoc.maPageWidth
                                    height: pdfDoc.maxPageHeight
                                }
                                asynchronous: true
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                mipmap: true
                                source: pdfDoc.source
                                currentFrame: startPageIndex+index

                                Component.onCompleted: configureSourceSize()
                                function configureSourceSize() {
                                    const bound = (min, val, max) => {
                                        return Math.min(max, Math.max(min,val))
                                    }
                                    sourceSize = Qt.size(bound(pdfDoc.maxPageWidth,pdfView.pdfPageWidth,pdfDoc.maxPageWidth*2),
                                                         bound(pdfDoc.maxPageHeight,pdfView.pdfPageHeight,pdfDoc.maxPageHeight*2))
                                }

                                Connections {
                                    target: pageScaleSlider
                                    function onPressedChanged() {
                                        if(!pageScaleSlider.pressed)
                                            Qt.callLater(() => { pdfPage.configureSourceSize() } )
                                    }
                                }

                                BusyIcon {
                                    anchors.centerIn: parent
                                    running: pdfPage.status !== Image.Ready
                                    opacity: 0.5
                                }
                            }
                        }
                    }
                }
            }
        }

        Item {
            id: pinchScaler
            scale: pageScaleSlider.value
            width: parent.width
            height: parent.height
        }

        PinchHandler {
            id: pinchHandler
            target: pinchScaler

            minimumScale: pageScaleSlider.from
            maximumScale: pageScaleSlider.to
            minimumRotation: 0
            maximumRotation: 0
            minimumPointCount: 2

            onScaleChanged: pageScaleSlider.value = activeScale
        }
    }

    BoxShadow {
        anchors.fill: floatingToolBar
    }

    Rectangle {
        id: floatingToolBar
        color: primaryColors.c100.background
        border.width: 1
        border.color: primaryColors.c500.background
        width: floatingButtonsRow.width + 10
        height: floatingButtonsRow.height + 20
        anchors.bottom: statusBar.top
        anchors.bottomMargin: height
        anchors.horizontalCenter: parent.horizontalCenter

        Row {
            id: floatingButtonsRow
            anchors.centerIn: parent

            ToolButton2 {
                icon.source: "../icons/action/close.png"
                ToolTip.text: "Closes the PDF View"
                anchors.verticalCenter: parent.verticalCenter
                onClicked: closeRequest()
                visible: closable
            }

            Item {
                width: 12
                height: parent.height
                visible: closable && pdfDoc.pageCount > 1

                Rectangle {
                    width: 1
                    height: parent.height
                    anchors.left: parent.left
                    color: primaryColors.c400.background
                }
            }

            Text {
                text: "View: "
                font.pointSize: Scrite.app.idealFontPointSize
                anchors.verticalCenter: parent.verticalCenter
                color: primaryColors.c300.text
                visible: pdfDoc.pageCount > 1
                rightPadding: 10
            }

            ComboBox2 {
                Material.foreground: primaryColors.c300.text
                Material.background: primaryColors.c300.background
                currentIndex: Math.max(pdfDoc.pagesPerRow-1,0)
                visible: pdfDoc.pageCount > 1
                model: {
                    const nrPages = Math.min(3, pdfDoc.pageCount)
                    var ret = ["1 Page"]
                    for(var i=1; i<nrPages; i++)
                        ret.push("" + (i+1) + " Pages")
                    return ret
                }
                onModelChanged: Qt.callLater( function() {
                    currentIndex = Qt.binding( function() { return Math.max(pdfDoc.pagesPerRow-1,0) } )
                })
                onCurrentIndexChanged: pageScaleSlider.value = pdfDoc.evaluatePageScale(currentIndex+1,pdfDoc.pageCount)
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: 12
                height: parent.height
                visible: pdfDoc.pageCount > 1 && (allowFileSave || allowFileReveal || displayRefreshButton)

                Rectangle {
                    width: 1
                    height: parent.height
                    anchors.right: parent.right
                    color: primaryColors.c400.background
                }
            }

            ToolButton2 {
                visible: displayRefreshButton
                icon.source: "../icons/navigation/refresh.png"
                ToolTip.text: "Refresh"
                anchors.verticalCenter: parent.verticalCenter
                onClicked: refreshRequest()
            }

            ToolButton2 {
                id: saveFileButton
                visible: allowFileSave
                icon.source: "../icons/file/file_download.png"
                ToolTip.text: "Save PDF"
                anchors.verticalCenter: parent.verticalCenter
                down: saveMenu.visible
                onClicked: saveMenu.open()

                Item {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: -saveMenu.height
                    height: 1

                    FileDialog {
                        id: folderPathDialog
                        folder: Scrite.app.localFileToUrl(StandardPaths.writableLocation(StandardPaths.DesktopLocation))
                        selectFolder: true
                        selectMultiple: false
                        selectExisting: false
                        onAccepted: saveFileButton.savePdf(Scrite.app.urlToLocalFile(folderPathDialog.folder))
                    }

                    Menu2 {
                        id: saveMenu
                        width: 325

                        MenuItem2 {
                            text: "To 'Downloads' folder"
                            property string targetFolder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
                            onClicked: saveFileButton.savePdf(targetFolder)
                        }

                        MenuItem2 {
                            text: Scrite.document.fileName === "" ?
                                  "To 'Desktop' folder" :
                                  "To the Scrite document folder"
                            property string targetFolder: Scrite.document.fileName === "" ? StandardPaths.writableLocation(StandardPaths.DesktopLocation) : Scrite.app.filePath(Scrite.document.fileName)
                            onClicked: saveFileButton.savePdf(targetFolder)
                        }

                        MenuItem2 {
                            text: "Other ..."
                            onClicked: folderPathDialog.open()
                        }
                    }
                }

                function savePdf(folderPath) {
                    var targetFilePath = ""
                    if(saveFilePath !== "")
                        targetFilePath = Scrite.app.fileName(saveFilePath) + ".pdf"
                    else
                        targetFilePath = saveFileName
                    targetFilePath = folderPath + "/" + targetFilePath

                    const downloadedFilePath = Scrite.app.copyFile( Scrite.app.urlToLocalFile(pdfDoc.source), targetFilePath)
                    if(downloadedFilePath !== "")
                        Scrite.app.revealFileOnDesktop(downloadedFilePath)
                }
            }

            ToolButton2 {
                visible: allowFileReveal
                icon.source: "../icons/file/folder_open.png"
                ToolTip.text: "Reveal the location of this PDF on your computer."
                anchors.verticalCenter: parent.verticalCenter
                onClicked: Scrite.app.revealFileOnDesktop( Scrite.app.urlToLocalFile(pdfDoc.source) )
            }
        }
    }

    Rectangle {
        id: statusBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 30
        color: primaryColors.c300.background
        border.width: 1
        border.color: primaryColors.c400.background

        Text {
            text: pdfDoc.pageCount + (pdfDoc.pageCount > 1 ? " Pages" : " Page")
            font.pixelSize: statusBar.height * 0.5
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            color: accentColors.c300.text
        }

        ZoomSlider {
            id: pageScaleSlider
            from: pdfDoc.minPageScale
            to: pdfDoc.maxPageScale
            value: 1
            height: parent.height-6
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
        }
    }
}
