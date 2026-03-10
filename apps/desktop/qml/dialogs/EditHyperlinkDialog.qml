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

pragma Singleton
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../helpers"
import "../controls"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    function launch(selectedText, existingLink, callback) {
        const props = {"selectedText": selectedText, "existingLink": existingLink}
        let dlg = doLaunch(props)
        dlg.updateLinkRequest.connect(callback)
        return dlg
    }

    name: "EditHyperlinkDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        property string selectedText
        property string existingLink

        signal updateLinkRequest(string newLink)

        width: 480
        height: 240
        title: "Edit Hyperlink"


        content: Item {
            Component.onCompleted: Qt.callLater(_hyperlinkField.forceActiveFocus)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                VclLabel {
                    Layout.fillWidth: true

                    text: (_dialog.existingLink === "" ? "Add hyperlink for " : "Edit hyper link to ") +
                          "<b>" + _dialog.selectedText + "</b>"
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    maximumLineCount: 3
                }

                VclTextField {
                    id: _hyperlinkField

                    Layout.fillWidth: true

                    text: _dialog.existingLink
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                RowLayout {
                    VclButton {
                        text: "Remove Link"
                        visible: _dialog.existingLink !== ""

                        onClicked: {
                            _dialog.updateLinkRequest("")
                            Qt.callLater(_dialog.close)
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    VclButton {
                        id: _acceptButton

                        text: _dialog.existingLink === "" ? "Insert Link" : "Update Link"
                        enabled: _hyperlinkField.text !== "" && (_dialog.existingLink === "" || _dialog.existingLink !== _hyperlinkField.text)

                        onClicked: {
                            _dialog.updateLinkRequest(_hyperlinkField.text)
                            Qt.callLater(_dialog.close)
                        }

                        ActionHandler {
                            action: _dialog.acceptAction

                            onTriggered: (source) => { _acceptButton.clicked() }
                        }
                    }
                }
            }
        }
    }
}
