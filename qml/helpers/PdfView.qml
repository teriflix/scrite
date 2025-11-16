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

    property bool allowFileReveal: false
    property bool allowFileSave: true
    property bool closable: true
    property bool displayRefreshButton: false
    property bool saveFeatureDisabled: false

    property alias pagesPerRow: _pdfDoc.pagesPerRow
    property alias source: _pdfDoc.source

    property string saveFileName: File.completeBaseName( Url.toPath(source) ) + ".pdf"
    property string saveFilePath

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
                id: _pdfView

                property real pdfPageScale: _pageScaleSlider.value
                property real pdfPageWidth: _pdfDoc.maxPageWidth * pdfPageScale
                property real pdfPageHeight: _pdfDoc.maxPageHeight * pdfPageScale
                property real pdfPageCellWidth: pdfPageWidth + spacing
                property real pdfPageCellHeight: pdfPageHeight + spacing
                property real spacing: Math.min( width*0.05, 30 )
                property int  pdfPagesPerRow: Math.max(1,Math.floor(width/pdfPageCellWidth))

                ScrollBar.vertical: VclScrollBar {
                    flickable: _pdfView
                }
                ScrollBar.horizontal: VclScrollBar {
                    flickable: _pdfView
                }

                FlickScrollSpeedControl.flickable: _pdfView
                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                anchors.fill: parent

                clip: true
                model: Math.ceil(_pdfDoc.pageCount / pdfPagesPerRow)
                rowHeightProvider: () => { return pdfPageCellHeight }
                columnWidthProvider: () => { return Math.max(width,pdfPageCellWidth*pdfPagesPerRow) }
                reuseItems: false

                delegate: Item {
                    required property int index

                    property int startPageIndex: index*_pdfView.pdfPagesPerRow
                    property int endPageIndex: Math.min(startPageIndex+_pdfView.pdfPagesPerRow-1, _pdfDoc.pageCount-1)

                    width: _pdfView.width
                    height: _pdfView.pdfPageCellHeight

                    Row {
                        anchors.centerIn: parent

                        height: _pdfView.pdfPageHeight

                        spacing: _pdfView.spacing

                        Repeater {
                            model: Math.max(1,(endPageIndex-startPageIndex)+1)

                            Rectangle {
                                width: _pdfView.pdfPageWidth
                                height: _pdfView.pdfPageHeight

                                Image {
                                    id: _pdfPage

                                    function configureSourceSize() {
                                        const bound = (min, val, max) => {
                                            return Math.min(max, Math.max(min,val))
                                        }
                                        const dpr = Platform.isMacOSDesktop ? Scrite.document.displayFormat.devicePixelRatio : 1.0
                                        sourceSize = Qt.size(dpr*bound(_pdfDoc.maxPageWidth,_pdfView.pdfPageWidth,_pdfDoc.maxPageWidth*2),
                                                             dpr*bound(_pdfDoc.maxPageHeight,_pdfView.pdfPageHeight,_pdfDoc.maxPageHeight*2))
                                    }

                                    Component.onCompleted: configureSourceSize()

                                    anchors.fill: parent

                                    asynchronous: true
                                    currentFrame: startPageIndex+index
                                    fillMode: Image.PreserveAspectFit
                                    mipmap: true
                                    smooth: true
                                    source: _pdfDoc.source
                                    sourceSize { width: _pdfDoc.maPageWidth;  height: _pdfDoc.maxPageHeight }

                                    Connections {
                                        target: _pageScaleSlider

                                        function onPressedChanged() {
                                            if(!_pageScaleSlider.pressed)
                                                _pdfPage.configureSourceSize()
                                        }
                                    }

                                    BusyIcon {
                                        anchors.centerIn: parent

                                        opacity: 0.5
                                        running: _pdfPage.status !== Image.Ready
                                    }
                                }
                            }
                        }
                    }
                }

                onPdfPageScaleChanged: Qt.callLater(returnToBounds)
                onPdfPageCellWidthChanged: Qt.callLater(forceLayout)
                onPdfPageCellHeightChanged: Qt.callLater(forceLayout)
            }

            Item {
                id: _pinchScaler

                width: parent.width
                height: parent.height

                scale: _pageScaleSlider.value
            }

            PinchHandler {
                id: _pinchHandler

                target: _pinchScaler

                minimumScale: _pageScaleSlider.from
                maximumScale: _pageScaleSlider.to
                minimumRotation: 0
                maximumRotation: 0
                minimumPointCount: 2

                onScaleChanged: _pageScaleSlider.value = activeScale
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
                text: _pdfDoc.pageCount + (_pdfDoc.pageCount > 1 ? " Pages" : " Page")
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.preferredHeight: 30

                color: Runtime.colors.primary.separatorColor
                visible: _pdfDoc.pageCount > 1
            }

            // Pages per row:
            VclComboBox {
                Layout.preferredWidth: Runtime.idealFontMetrics.boundingRect("View 3 Page(s)").width + 40

                model: {
                    const nrPages = Math.min(3, _pdfDoc.pageCount)
                    var ret = ["1 Page"]
                    for(var i=1; i<nrPages; i++)
                        ret.push("" + (i+1) + " Pages")
                    return ret
                }
                visible: _pdfDoc.pageCount > 1
                displayText: "View: " + currentText

                onModelChanged: Qt.callLater( function() {
                    currentIndex = Qt.binding( function() { return Math.max(_pdfDoc.pagesPerRow-1,0) } )
                })
                onCurrentIndexChanged: _pageScaleSlider.value = _pdfDoc.evaluatePageScale(currentIndex+1,_pdfDoc.pageCount)
            }

            Item {
                Layout.fillWidth: true
            }

            // Download & Refresh buttons
            VclToolButton {
                text: "Refresh"
                visible: displayRefreshButton
                toolTipText: "Regenerates this PDF and refreshes its content."

                icon.source: "qrc:/icons/navigation/refresh.png"

                onClicked: refreshRequest()
            }

            VclToolButton {

                text: "Save PDF"
                down: _saveMenu.visible
                visible: (allowFileSave || saveFeatureDisabled)
                toolTipText: "Save this PDF to your computer."

                icon.source: "qrc:/icons/file/file_download.png"

                onClicked: {
                    if(saveFeatureDisabled)
                        _saveDisabledNotice.open()
                    else
                        _saveMenu.open()
                }

                Item {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: -_saveMenu.height

                    height: 1

                    VclMenu {
                        id: _saveMenu

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
                            property string targetFolder: Scrite.document.fileName === "" ? StandardPaths.writableLocation(StandardPaths.DesktopLocation) : File.path(Scrite.document.fileName)

                            onClicked: _private.savePdf(targetFolder)
                        }

                        VclMenuItem {
                            text: "Other ..."

                            onClicked: _saveFileDialog.open()
                        }
                    }
                }
            }

            VclToolButton {
                id: _revealFileButton

                text: "Reveal"
                visible: allowFileReveal
                toolTipText: "Reveal the location of this PDF on your computer."

                icon.source: "qrc:/icons/file/folder_open.png"

                onClicked: File.revealOnDesktop( Url.toPath(_pdfDoc.source) )
            }

            Item {
                Layout.fillWidth: true
            }

            // Zoom Slider
            ZoomSlider {
                id: _pageScaleSlider

                Layout.preferredHeight: 20
                Layout.preferredWidth: implicitWidth

                from: _pdfDoc.minPageScale
                to: _pdfDoc.maxPageScale
                value: 1
            }
        }
    }

    // Private Stuff
    PdfDocument {
        id: _pdfDoc

        property int  pagesPerRow: 1
        property real minPageScale: evaluatePageScale(4)
        property real idealPageScale: evaluatePageScale(Math.min(pageCount,2))
        property real maxPageScale: evaluatePageScale(0.5)

        function evaluatePageScale(nrPages) {
            return status === PdfDocument.Ready ?
                        Math.max(0.0000001,
                                 ((_pdfView.width/nrPages)-(2+nrPages-1)*_pdfView.spacing)/maxPageWidth) :
                        1
        }

        source: root.source

        onIdealPageScaleChanged: _pageScaleSlider.value = idealPageScale
        onStatusChanged: Qt.callLater( function() {
            if(pageCount === 1 && pagesPerRow > 1)
                _pageScaleSlider.value = _pdfView.height/maxPageHeight
            else
                _pageScaleSlider.value = _pdfDoc.evaluatePageScale(Math.min(pagesPerRow,pageCount))
        })
    }

    VclFileDialog {
        id: _saveFileDialog

        nameFilters: ["Adobe PDF Files (*.pdf)"]
        folder: Url.fromPath(StandardPaths.writableLocation(StandardPaths.DownloadLocation))
        selectFolder: false
        selectMultiple: false
        selectExisting: false
        sidebarVisible: true

         // The default Ctrl+U interfers with underline
        onAccepted: {
            const targetFilePath = Url.toPath(_saveFileDialog.fileUrl)
            const downloadedFilePath = File.copyToFolder( Url.toPath(_pdfDoc.source), targetFilePath )
            if(downloadedFilePath !== "")
                File.revealOnDesktop(downloadedFilePath)
        }
    }

    VclDialog {
        id: _saveDisabledNotice

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
            let targetFilePath = ""
            if(saveFilePath !== "")
                targetFilePath = File.completeBaseName(saveFilePath) + ".pdf"
            else
                targetFilePath = saveFileName
            targetFilePath = folderPath + "/" + targetFilePath

            const downloadedFilePath = File.copyToFolder( Url.toPath(_pdfDoc.source), targetFilePath)
            if(downloadedFilePath !== "")
                File.revealOnDesktop(downloadedFilePath)
        }
    }
}
