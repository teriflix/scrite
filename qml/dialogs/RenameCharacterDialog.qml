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

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

DialogLauncher {
    id: root

    function launch(character) {
        if(!character || !Object.isOfType(character, "Character")) {
            console.log("Couldn't launch " + name + ": invalid character specified.")
            return null
        }

        return doLaunch({"character": character})
    }

    name: "RenameCharacterDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        property Character character

        width: 680
        height: 300

        handleLanguageShortcuts: true
        title: "Rename/Merge Character: " + _private.orignalCharacterName

        content: Item {
            Component.onCompleted: Qt.callLater(newNameField.forceActiveFocus)

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
                    onReturnPressed: {
                        if(renameButton.enabled)
                            renameButton.clicked()
                    }
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
                                let question = "Merging " + _private.orignalCharacterName + " with <b>" + _private.newCharacterName + "</b>"

                                const originalCh = Scrite.document.structure.findCharacter(_private.orignalCharacterName)
                                const newCh = Scrite.document.structure.findCharacter(_private.newCharacterName)
                                if(originalCh) {
                                    let points = []

                                    const ntos = (number) => {
                                        const nos = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"]
                                        if(number > 10)
                                        return ""+number
                                        return nos[number]
                                    }

                                    if(originalCh.notes.noteCount > 0)
                                    points.push(ntos(originalCh.notes.noteCount) + " note(s)")
                                    if(originalCh.attachments.attachmentCount > 0)
                                    points.push(ntos(originalCh.attachments.attachmentCount) + " attachment(s)")
                                    if(originalCh.photos.length > 0)
                                    points.push(ntos(originalCh.photos.length) + " photo(s)")
                                    if(originalCh.relationshipCount > 0)
                                    points.push(ntos(originalCh.relationshipCount) + " relationship(s)")

                                    if(points.length > 0) {
                                        question += ", along with "

                                        if(points.length > 1) {
                                            question += "<br/><ul>"

                                            for(let p=0; p<points.length; p++) {
                                                let suffix = "."
                                                if(points.length > 1) {
                                                    if(p < points.length-2)
                                                    suffix = ","
                                                    else if(p === points.length-2)
                                                    suffix = ", and"
                                                }
                                                question += "<li>" + points[p] + suffix + "</li>"
                                            }
                                            question += "</ul>"
                                        } else
                                        question += points[0] + ".<br/>"
                                    } else {
                                        question += ".<br/>"
                                    }
                                } else {
                                    question += ".<br/>"
                                }

                                question += "<br/>Are you sure you want to do this?"

                                MessageBox.question("Merge Confirmation",
                                                    question,
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

                        if(_private.renameWasSuccessful) {
                            const tabs = Runtime.showNotebookInStructure ? [Runtime.StructureTab, Runtime.NotebookTab] : [Runtime.NotebookTab]
                            if(tabs.indexOf(Runtime.mainWindowTab) >= 0) {
                                let characterNotes = ActionHub.notebookOperations.find("characterNotes")
                                characterNotes.characterName = _private.newCharacterName
                                characterNotes.trigger()
                            }
                            Qt.callLater(dialog.close)
                        } else {
                            MessageBox.information("Rename Error", dialog.character.renameError, () => {
                                                       dialog.character.clearRenameError()
                                                       Qt.callLater(dialog.close)
                                                   } )
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

        QtObject {
            id: _private

            property string orignalCharacterName
            property string newCharacterName
            property bool renameWasSuccessful: false
            property VclDialog waitDialog
        }
    }
}
