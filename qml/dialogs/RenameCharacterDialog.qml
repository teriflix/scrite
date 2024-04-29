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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch(character) {
        if(!character || !Scrite.app.verifyType(character, "Character")) {
            console.log("Couldn't launch RenameCharacterDialog: invalid character specified.")
            return null
        }

        var dlg = dialogComponent.createObject(root, {"character": character})
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch RenameCharacterDialog")
        return null
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            property Character character

            width: 680
            height: 300
            title: "Rename/Merge Character: " + _private.orignalCharacterName

            content: Item {
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
                        font.capitalization: Font.AllUppercase
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

                                const allCharacterNames = Scrite.document.structure.allCharacterNames()
                                if(allCharacterNames.indexOf(_private.newCharacterName) >= 0) {
                                    MessageBox.question("Rename Confirmation",
                                                        "Are you sure you want to merge " + _private.orignalCharacterName + " with <b>" + _private.newCharacterName + "</b>?",
                                                        ["Yes", "No", "Cancel"],
                                                        (answer) => {
                                                            if(answer === "Yes")
                                                                renameJob.start()
                                                            else if(answer === "Cancel")
                                                                Qt.callLater(dialog.close)
                                                        })
                                }
                                else
                                    renameJob.start()
                            }
                        }
                    }
                }

                SequentialAnimation {
                    id: renameJob

                    running: false

                    // Launch wait dialog ..
                    ScriptAction {
                        script: {
                            _private.waitDialog = WaitDialog.launch("Renaming '" + _private.orignalCharacterName + "' to '" + _private.newCharacterName + "' ...")
                        }
                    }

                    // Wait for it to show up on the UI ...
                    PauseAnimation {
                        duration: 200
                    }

                    // Perform the rename ...
                    ScriptAction {
                        script: {
                            dialog.character.clearRenameError()
                            _private.renameWasSuccessful = dialog.character.rename(_private.newCharacterName)
                        }
                    }

                    // Wait for the UI to update after renaming was done ...
                    PauseAnimation {
                        duration: 200
                    }

                    // Cleanup and complete
                    ScriptAction {
                        script: {
                            Qt.callLater(_private.waitDialog.close)
                            _private.waitDialog = null

                            if(_private.renameWasSuccessful)
                                Qt.callLater(dialog.close)
                            else
                                MessageBox.information("Rename Error", dialog.character.renameError, () => {
                                                           dialog.character.clearRenameError()
                                                           Qt.callLater(dialog.close)
                                                       } )
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

            QtObject {
                id: _private

                property string orignalCharacterName
                property string newCharacterName
                property bool renameWasSuccessful: false
                property VclDialog waitDialog
            }
        }
    }
}
