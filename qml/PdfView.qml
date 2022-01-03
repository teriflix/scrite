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
    property bool allowFileSave: true
    property bool allowFileReveal: false

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
                                    onPressedChanged: {
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
    }

    Rectangle {
        id: floatingToolBar
        radius: 8
        color: accentColors.c900.background
        border.width: 1
        border.color: accentColors.c100.background
        width: floatingButtonsRow.width + 30
        height: floatingButtonsRow.height + 20
        anchors.bottom: statusBar.top
        anchors.bottomMargin: height
        anchors.horizontalCenter: parent.horizontalCenter

        Row {
            id: floatingButtonsRow
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 5
            spacing: 20

            Text {
                text: pdfDoc.pageCount + (pdfDoc.pageCount > 1 ? " Pages" : " Page")
                font.pointSize: Scrite.app.idealFontPointSize
                anchors.verticalCenter: parent.verticalCenter
                color: accentColors.c900.text
            }

            Rectangle {
                width: 1
                height: parent.height
                color: accentColors.c100.background
                visible: pdfDoc.pageCount > 1
            }

            Text {
                text: "View: "
                font.pointSize: Scrite.app.idealFontPointSize
                anchors.verticalCenter: parent.verticalCenter
                color: accentColors.c900.text
                visible: pdfDoc.pageCount > 1
            }

            ComboBox2 {
                Material.foreground: accentColors.c500.text
                Material.background: accentColors.c500.background
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

            Rectangle {
                width: 1
                height: parent.height
                color: accentColors.c100.background
                visible: allowFileSave || allowFileReveal
            }

            ToolButton2 {
                visible: allowFileSave
                icon.source: "../icons/content/save_as_inverted.png"
                ToolTip.text: "Save a copy of this PDF."
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    const filePath = saveFilePath === "" ?
                                       (StandardPaths.writableLocation(StandardPaths.DownloadLocation) + "/" + saveFileName) :
                                       saveFilePath
                    const downloadedFilePath = Scrite.app.copyFile( Scrite.app.urlToLocalFile(pdfDoc.source), filePath)
                    if(downloadedFilePath !== "")
                        Scrite.app.revealFileOnDesktop(downloadedFilePath)
                }
            }

            ToolButton2 {
                visible: allowFileReveal
                icon.source: "../icons/file/folder_open_inverted.png"
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
        color: primaryColors.windowColor
        border.width: 1
        border.color: primaryColors.borderColor

        ZoomSlider {
            id: pageScaleSlider
            from: pdfDoc.minPageScale
            to: pdfDoc.maxPageScale
            value: 1
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 20
        }
    }
}
