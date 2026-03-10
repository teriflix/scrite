/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

Item {
    id: root

    property alias folder: _folderPathDialog.currentFolder
    property alias absoluteFilePath: _fileInfo.absoluteFilePath

    property var nameFilters // Unused, but here for backward compatibility
    property var selectedExtension
    property var allowedExtensions: []

    property string label: "Select a file to export into"
    property string filePathPrefix: "File will be saved as: "

    property TabSequenceManager tabSequenceManager

    implicitWidth: 400
    implicitHeight: _layout.height

    BasicFileInfo {
        id: _fileInfo
    }

    FolderDialog {
        id: _folderPathDialog

        currentFolder: {
            if(_fileInfo.absolutePath !== "") {
                if(_fileInfo.exists)
                    return Url.fromPath(_fileInfo.absolutePath)
            }
            return Url.fromPath(StandardPaths.writableLocation(StandardPaths.DownloadLocation))
        }

        onAccepted: _fileInfo.absolutePath = Url.toPath(selectedFolder)
         // The default Ctrl+U interfers with underline
    }

    Column {
        id: _layout
        spacing: 10
        width: parent.width

        VclLabel {
            id: _labelText

            width: parent.width

            enabled: visible
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            lineHeight: 1.2
            lineHeightMode: Text.ProportionalHeight
            text: "<b>" + label + ":</b><br/>(" + filePathPrefix + "<u>" + _fileInfo.absoluteFilePath + "</u>. <a href=\"change\">Change path</a>.)</font>"
            visible: selectedExtension && selectedExtension.value !== AbstractReportGenerator.PdfFormat
            wrapMode: Text.WordWrap

            MouseArea {
                anchors.fill: parent

                hoverEnabled: true
                cursorShape: parent.linkAt(mouseX, mouseY) === "change" ? Qt.PointingHandCursor : Qt.ArrowCursor

                onClicked: (mouse) => {
                    if(parent.linkAt(mouse.x, mouse.y) === "change")
                        _folderPathDialog.open()
                }
            }
        }

        VclTextField {
            TabSequenceItem.manager: tabSequenceManager

            width: parent.width

            enabled: visible
            placeholderText: "File Name"
            text: _fileInfo.baseName
            visible: selectedExtension.value !== AbstractReportGenerator.PdfFormat

            onTextEdited: _fileInfo.baseName = text
        }

        Row {
            spacing: 20

            Repeater {
                model: allowedExtensions

                delegate: VclRadioButton {
                    id: _delegate

                    required property int index
                    required property var modelData

                    text: modelData.label + " (." + modelData.suffix + ")"
                    checked: selectedExtension.value === modelData.value
                    enabled: modelData.enabled ? modelData.enabled === true : true

                    onClicked: {
                        selectedExtension = modelData
                        _fileInfo.suffix = selectedExtension.suffix
                    }
                }
            }
        }
    }
}
