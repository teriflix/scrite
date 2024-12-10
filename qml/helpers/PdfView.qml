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

import QtQuick 2.15
import QtQuick.Pdf 5.15
import QtQuick.Window 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    property alias source: pdfDoc.source
    property alias pagesPerRow: pdfDoc.pagesPerRow
    property string saveFilePath
    property string saveFileName: Scrite.app.fileName( Scrite.app.urlToLocalFile(source) ) + ".pdf"
    property bool closable: true
    property bool allowFileSave: true
    property bool allowFileReveal: false
    property bool saveFeatureDisabled: false
    property bool displayRefreshButton: false

    signal closeRequest()
    signal refreshRequest()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c300.background

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
                ScrollBar.vertical: VclScrollBar {
                    flickable: pdfView
                }
                ScrollBar.horizontal: VclScrollBar {
                    flickable: pdfView
                }
                FlickScrollSpeedControl.flickable: pdfView
                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

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
                                        const dpr = Scrite.app.isMacOSPlatform ? Scrite.document.displayFormat.devicePixelRatio : 1.0
                                        sourceSize = Qt.size(dpr*bound(pdfDoc.maxPageWidth,pdfView.pdfPageWidth,pdfDoc.maxPageWidth*2),
                                                             dpr*bound(pdfDoc.maxPageHeight,pdfView.pdfPageHeight,pdfDoc.maxPageHeight*2))
                                    }

                                    Connections {
                                        target: pageScaleSlider
                                        function onPressedChanged() {
                                            if(!pageScaleSlider.pressed)
                                                pdfPage.configureSourceSize()
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

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            color: Runtime.colors.primary.separatorColor
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            spacing: 20

            // Page Count
            VclLabel {
                text: pdfDoc.pageCount + (pdfDoc.pageCount > 1 ? " Pages" : " Page")
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.preferredHeight: 30

                color: Runtime.colors.primary.separatorColor
                visible: pdfDoc.pageCount > 1
            }

            // Pages per row:
            VclComboBox {
                Layout.preferredWidth: Runtime.idealFontMetrics.boundingRect("View 3 Page(s)").width + 40

                model: {
                    const nrPages = Math.min(3, pdfDoc.pageCount)
                    var ret = ["1 Page"]
                    for(var i=1; i<nrPages; i++)
                        ret.push("" + (i+1) + " Pages")
                    return ret
                }
                visible: pdfDoc.pageCount > 1
                displayText: "View: " + currentText

                onModelChanged: Qt.callLater( function() {
                    currentIndex = Qt.binding( function() { return Math.max(pdfDoc.pagesPerRow-1,0) } )
                })
                onCurrentIndexChanged: pageScaleSlider.value = pdfDoc.evaluatePageScale(currentIndex+1,pdfDoc.pageCount)
            }

            Item {
                Layout.fillWidth: true
            }

            // Download & Refresh buttons
            VclToolButton {
                ToolTip.text: "Regenerates this PDF and refreshes its content."

                text: "Refresh"
                visible: displayRefreshButton
                icon.source: "qrc:/icons/navigation/refresh.png"

                onClicked: refreshRequest()
            }

            VclToolButton {
                ToolTip.text: "Save this PDF to your computer."

                text: "Save PDF"
                down: saveMenu.visible
                visible: (allowFileSave || saveFeatureDisabled)
                icon.source: "qrc:/icons/file/file_download.png"

                onClicked: {
                    if(saveFeatureDisabled)
                        saveDisabledNotice.open()
                    else
                        saveMenu.open()
                }

                Item {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: -saveMenu.height

                    height: 1

                    VclMenu {
                        id: saveMenu

                        width: 325

                        VclMenuItem {
                            text: "To 'Downloads' folder"
                            property string targetFolder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

                            onClicked: _private.savePdf(targetFolder)
                        }

                        VclMenuItem {
                            text: Scrite.document.fileName === "" ?
                                      "To 'Desktop' folder" :
                                      "To the Scrite document folder"
                            property string targetFolder: Scrite.document.fileName === "" ? StandardPaths.writableLocation(StandardPaths.DesktopLocation) : Scrite.app.filePath(Scrite.document.fileName)

                            onClicked: _private.savePdf(targetFolder)
                        }

                        VclMenuItem {
                            text: "Other ..."

                            onClicked: saveFileDialog.open()
                        }
                    }
                }
            }

            VclToolButton {
                id: revealFileButton

                ToolTip.text: "Reveal the location of this PDF on your computer."

                text: "Reveal"
                visible: allowFileReveal
                icon.source: "qrc:/icons/file/folder_open.png"

                onClicked: Scrite.app.revealFileOnDesktop( Scrite.app.urlToLocalFile(pdfDoc.source) )
            }

            Item {
                Layout.fillWidth: true
            }

            // Zoom Slider
            ZoomSlider {
                id: pageScaleSlider

                Layout.preferredHeight: 20
                Layout.preferredWidth: implicitWidth

                from: pdfDoc.minPageScale
                to: pdfDoc.maxPageScale
                value: 1
            }
        }
    }

    // Private Stuff
    PdfDocument {
        id: pdfDoc
        property int  pagesPerRow: 1
        property real minPageScale: evaluatePageScale(4)
        property real idealPageScale: evaluatePageScale(Math.min(pageCount,2))
        property real maxPageScale: evaluatePageScale(0.5)

        source: root.source

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

    VclFileDialog {
        id: saveFileDialog
        nameFilters: ["Adobe PDF Files (*.pdf)"]
        folder: Scrite.app.localFileToUrl(StandardPaths.writableLocation(StandardPaths.DownloadLocation))
        selectFolder: false
        selectMultiple: false
        selectExisting: false
        sidebarVisible: true
         // The default Ctrl+U interfers with underline
        onAccepted: {
            const targetFilePath = Scrite.app.urlToLocalFile(saveFileDialog.fileUrl)
            const downloadedFilePath = Scrite.app.copyFile( Scrite.app.urlToLocalFile(pdfDoc.source), targetFilePath )
            if(downloadedFilePath !== "")
                Scrite.app.revealFileOnDesktop(downloadedFilePath)
        }
    }

    VclDialog {
        id: saveDisabledNotice

        width: 640
        height: 480
        title: "Feature Missing"

        content: DisabledFeatureNotice {
            featureName: "Saving PDF Files"
        }
    }

    QtObject {
        id: _private

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
}
