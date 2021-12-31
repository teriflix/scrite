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
    property string fileName: Scrite.app.fileName( Scrite.app.urlToLocalFile(source) ) + ".pdf"
    property bool allowFileSave: true

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
            property real idealPageScale: evaluatePageScale(Math.min(nrPages,2))
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
                                    width: Math.min(pdfDoc.maxPageWidth*2,pdfView.pdfPageWidth)
                                    height: Math.min(pdfDoc.maxPageHeight*2,pdfView.pdfPageHeight)
                                }
                                asynchronous: true
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                mipmap: true
                                source: pdfDoc.source
                                currentFrame: startPageIndex+index
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
            }

            Text {
                text: "Zoom: "
                font.pointSize: Scrite.app.idealFontPointSize
                anchors.verticalCenter: parent.verticalCenter
                color: accentColors.c900.text
            }

            ComboBox2 {
                Material.foreground: accentColors.c500.text
                Material.background: accentColors.c500.background
                currentIndex: Math.max(pdfDoc.pagesPerRow-1,0)
                model: {
                    const nrPages = Math.min(3, pdfDoc.pageCount)
                    var ret = []
                    for(var i=0; i<nrPages; i++) {
                        if(i)
                            ret.push("" + (i+1) + " Pages")
                        else
                            ret.push("1 Page")
                    }
                    return ret
                }
                onModelChanged: currentIndex = Qt.binding( function() { return Math.max(pdfDoc.pagesPerRow-1,0) } )
                onCurrentIndexChanged: pageScaleSlider.value = pdfDoc.evaluatePageScale(currentIndex+1,pdfDoc.pageCount)
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                width: 1
                height: parent.height
                color: accentColors.c100.background
                visible: allowFileSave
            }

            ToolButton2 {
                visible: allowFileSave
                icon.source: "../icons/content/save_as_inverted.png"
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    const downloadedFilePath = Scrite.app.copyFile( Scrite.app.urlToLocalFile(pdfDoc.source),
                            StandardPaths.writableLocation(StandardPaths.DownloadLocation) + "/" + fileName )
                    if(downloadedFilePath !== "")
                        Scrite.app.revealFileOnDesktop(downloadedFilePath)
                }
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

        Slider {
            id: pageScaleSlider
            orientation: Qt.Horizontal
            width: 200
            from: pdfDoc.minPageScale
            to: pdfDoc.maxPageScale
            value: 1
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
        }
    }
}
