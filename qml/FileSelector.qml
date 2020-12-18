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
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.13

Item {
    id: fileSelector
    property string label: "Select a file to export into"
    property alias absoluteFilePath: fileInfo.absoluteFilePath
    property var allowedExtensions: []
    property var selectedExtension
    property string filePathPrefix: "File will be saved as: "
    property alias nameFilters: folderPathDialog.nameFilters
    property url folder

    width: 400
    height: layout.height

    onAllowedExtensionsChanged: {
        if(allowedExtensions.length > 0)
            selectedExtension = allowedExtensions[0]
    }

    FileInfo {
        id: fileInfo
    }

    FileDialog {
        id: folderPathDialog
        folder: {
            if(fileInfo.absolutePath !== "") {
                if(fileInfo.exists)
                    return "file:///" + fileInfo.absolutePath
            }
            return fileSelector.folder
        }
        onFolderChanged: fileSelector.folder = folder
        selectFolder: true
        selectMultiple: false
        selectExisting: false
        onAccepted: fileInfo.absolutePath = app.urlToLocalFile(fileUrl)
    }

    Column {
        id: layout
        spacing: 5
        width: parent.width

        Text {
            id: labelText
            width: parent.width
            wrapMode: Text.WordWrap
            lineHeight: 1.2
            lineHeightMode: Text.ProportionalHeight
            text: "<font size=\"+1\">" + label + ":</font><br/>(" + filePathPrefix + "<u>" + fileInfo.absoluteFilePath + "</u>. <a href=\"change\">Change path</a>.)"

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

        TextField2 {
            placeholderText: "file name"
            text: fileInfo.baseName
            width: parent.width
            onTextChanged: fileInfo.baseName = text
        }

        Row {
            spacing: 20

            Repeater {
                model: allowedExtensions

                RadioButton2 {
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
