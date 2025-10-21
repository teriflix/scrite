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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

BasicAttachmentsDropArea {
    id: root

    property string droppedFilePath
    property string droppedFileName

    allowedType: Attachments.NoMedia
    allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]

    VclDialog {
        id: _dialog

        title: root.active ? root.attachment.originalFileName : root.droppedFileName
        width: Math.min(500, Scrite.window.width * 0.5)
        height: 275

        content: Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                VclLabel {
                    Layout.fillWidth: true

                    text: root.active ? "Drop the file here to open/import it." : "Do you want to open, import or cancel?"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                VclLabel {
                    Layout.fillWidth: true

                    text: "NOTE: Any unsaved changes in the currently open document will be discarded."
                    color: Runtime.colors.primary.c700.text
                    visible: !Scrite.document.empty || Scrite.document.fileName !== ""
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                }

                RowLayout {
                    spacing: 20

                    VclButton {
                        text: "Open/Import"

                        onClicked: {
                            Scrite.document.openOrImport(root.droppedFilePath)
                            root.droppedFileName = ""
                            root.droppedFilePath = ""
                            Qt.callLater(_dialog.close)
                        }
                    }

                    VclButton {
                        text: "Cancel"

                        onClicked:  {
                            root.droppedFileName = ""
                            root.droppedFilePath = ""
                            Qt.callLater(_dialog.close)
                        }
                    }
                }
            }
        }
    }

    onDropped: {
        if(Scrite.document.empty)
            Scrite.document.openOrImport(attachment.filePath)
        else {
            droppedFilePath = attachment.filePath
            droppedFileName = attachment.originalFileName

            Runtime.closeAllDialogs()

            _dialog.open()
        }
    }
}
