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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

VclDialog {
    id: root

    property Character character
    property bool renameWasSuccessful: false

    width: 640
    height: 300
    title: "Rename Character: " + _private.orignalCharacterName

    content: character ? renameCharacterDialogContent : characterNotSpecifiedError

    Component {
        id: characterNotSpecifiedError

        VclText {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            text: "No character was supplied for renaming."
        }
    }

    Component {
        id: renameCharacterDialogContent

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                VclTextField {
                    id: newNameField
                    Layout.fillWidth: true

                    placeholderText: "New name"
                    label: ""
                    focus: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                    onReturnPressed: renameButton.click()
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20

                    VclCheckBox {
                        id: chkNotice
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        padding: 0
                        text: "I understand that the rename operation cannot be undone."
                    }

                    VclButton {
                        id: renameButton
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        text: "Rename"
                        enabled: chkNotice.checked && newNameField.length > 0 && newNameField.text.toUpperCase() !== character.name
                        onClicked: {
                            _private.newCharacterName = newNameField.text.toUpperCase()
                            renameProgressDialog.open()
                        }
                    }
                }
            }

            VclDialog {
                id: renameProgressDialog

                width: root.width * 0.8
                height: root.height * 0.8
                title: "Renaming ..."
                titleBarButtons: null

                onDismissed: {
                    character.clearRenameError()
                }

                content: VclText {
                    id: renameProgressDialogContent
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: "Renaming '" + _private.orignalCharacterName + "' to '" + _private.newCharacterName + "', please wait ..."
                    wrapMode: Text.WordWrap

                    Component.onCompleted: {
                        renameWasSuccessful = root.character.rename(_private.newCharacterName)
                        if(renameWasSuccessful) {
                            closeLater()
                        } else {
                            color = "red"
                            text = character.renameError
                        }
                    }

                    SequentialAnimation {
                        id: renameAnimation
                        running: true

                        PauseAnimation {
                            duration: 200
                        }

                        ScriptAction {
                            script: {
                                root.renameWasSuccessful = root.character.rename(_private.newCharacterName)
                            }
                        }

                        PauseAnimation {
                            duration: 200
                        }

                        ScriptAction {
                            script: {
                                if(root.renameWasSuccessful) {
                                    Qt.callLater(root.close)
                                    renameProgressDialogContent.close()
                                } else {
                                    renameProgressDialogContent.color = "red"
                                    renameProgressDialogContent.text = character.renameError
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    onCharacterChanged: {
        if(character)
            _private.orignalCharacterName = character.name
        else
            _private.orignalCharacterName = "<unknown>"
        _private.newCharacterName = ""
    }

    onClosed: character = null

    QtObject {
        id: _private

        property string orignalCharacterName
        property string newCharacterName
    }
}
