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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/"
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notifications"

ScreenplayEditor {
    id: root

    HelpTipNotification {
        tipName: "screenplay"
    }

    BasicAttachmentsDropArea {
        id: _dropArea

        property string droppedFilePath
        property string droppedFileName

        anchors.fill: parent

        allowedType: Attachments.NoMedia
        allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]

        onDropped: {
            if(Scrite.document.empty)
                Scrite.document.openOrImport(attachment.filePath)
            else {
                droppedFilePath = attachment.filePath
                droppedFileName = attachment.originalFileName
            }

            Announcement.shout(Runtime.announcementIds.closeDialogBoxRequest, undefined)
        }

        Loader {
            id: fileOpenDropAreaNotification

            Component.onDestruction: _header.enabled = true

            anchors.fill: _dropArea

            active: _dropArea.active || _dropArea.droppedFilePath !== ""
            onActiveChanged: _header.enabled = !active

            sourceComponent: Rectangle {
                color: Scrite.app.translucent(Runtime.colors.primary.c500.background, 0.5)

                Rectangle {
                    anchors.fill: fileOpenDropAreaNotice
                    anchors.margins: -30

                    color: Runtime.colors.primary.c700.background
                    radius: 4
                }

                Column {
                    id: fileOpenDropAreaNotice

                    anchors.centerIn: parent

                    width: parent.width * 0.5
                    spacing: 20

                    VclLabel {
                        width: parent.width

                        text: _dropArea.active ? _dropArea.attachment.originalFileName : _dropArea.droppedFileName
                        color: Runtime.colors.primary.c700.text
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter

                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    }

                    VclLabel {
                        width: parent.width

                        text: _dropArea.active ? "Drop the file here to open/import it." : "Do you want to open, import or cancel?"
                        color: Runtime.colors.primary.c700.text
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter

                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    }

                    VclLabel {
                        width: parent.width

                        text: "NOTE: Any unsaved changes in the currently open document will be discarded."
                        color: Runtime.colors.primary.c700.text
                        visible: !Scrite.document.empty || Scrite.document.fileName !== ""
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter

                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter

                        spacing: 20
                        visible: !Scrite.document.empty

                        VclButton {
                            text: "Open/Import"

                            onClicked: {
                                Scrite.document.openOrImport(_dropArea.droppedFilePath)
                                _dropArea.droppedFileName = ""
                                _dropArea.droppedFilePath = ""
                            }
                        }

                        VclButton {
                            text: "Cancel"

                            onClicked:  {
                                _dropArea.droppedFileName = ""
                                _dropArea.droppedFilePath = ""
                            }
                        }
                    }
                }
            }
        }
    }
}
