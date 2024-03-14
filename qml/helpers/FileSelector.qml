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
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: fileSelector
    property string label: "Select a file to export into"
    property alias absoluteFilePath: fileInfo.absoluteFilePath
    property var allowedExtensions: []
    property var selectedExtension
    property string filePathPrefix: "File will be saved as: "
    property alias nameFilters: folderPathDialog.nameFilters
    property TabSequenceManager tabSequenceManager

    width: 400
    height: layout.height

    onAllowedExtensionsChanged: {
        if(allowedExtensions.length > 0)
            selectedExtension = allowedExtensions[0]
    }

    BasicFileInfo {
        id: fileInfo
    }

    FileDialog {
        id: folderPathDialog
        folder: {
            if(fileInfo.absolutePath !== "") {
                if(fileInfo.exists)
                    return Scrite.app.localFileToUrl(fileInfo.absolutePath)
            }
            return Scrite.app.localFileToUrl(StandardPaths.writableLocation(StandardPaths.DownloadFolder))
        }
        onFolderChanged: fileSelector.folder = folder
        selectFolder: true
        selectMultiple: false
        selectExisting: false
        onAccepted: fileInfo.absolutePath = Scrite.app.urlToLocalFile(fileUrl)
        dirUpAction.shortcut: "Ctrl+Shift+U" // The default Ctrl+U interfers with underline
    }

    Column {
        id: layout
        spacing: 5
        width: parent.width

        VclText {
            id: labelText
            width: parent.width
            wrapMode: Text.WordWrap
            lineHeight: 1.2
            lineHeightMode: Text.ProportionalHeight
            text: label + ":<br/><font size=\"-2\">(" + filePathPrefix + "<u>" + fileInfo.absoluteFilePath + "</u>. <a href=\"change\">Change path</a>.)</font>"
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            visible: selectedExtension.value !== AbstractReportGenerator.AdobePDF
            enabled: visible

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: parent.linkAt(mouseX, mouseY) === "change" ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: {
                    if(parent.linkAt(mouse.x, mouse.y) === "change")
                        folderPathDialog.open()
                }
            }
        }

        VclTextField {
            placeholderText: "File Name"
            text: fileInfo.baseName
            width: parent.width
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            onTextChanged: fileInfo.baseName = text
            TabSequenceItem.manager: tabSequenceManager
            visible: selectedExtension.value !== AbstractReportGenerator.AdobePDF
            enabled: visible
        }

        Row {
            spacing: 20

            Repeater {
                model: allowedExtensions

                VclRadioButton {
                    text: modelData.label + " (." + modelData.suffix + ")"
                    checked: selectedExtension.value === modelData.value
                    onClicked: {
                        selectedExtension = modelData
                        fileInfo.suffix = selectedExtension.suffix
                    }
                    enabled: modelData.enabled ? modelData.enabled === true : true
                }
            }
        }
    }
}
