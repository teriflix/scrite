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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "AddCharactersDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        width: Math.min(750, Scrite.window.width*0.8)
        height: Math.min(600, Scrite.window.height*0.9)
        title: "Add Existing Characters"

        contentItem: Item {
            enabled: !Scrite.document.readOnly

            GenericArrayModel {
                id: _charactersModel

                Component.onCompleted: {
                    const allCharacters = Scrite.document.structure.allCharacterNames()
                    let characters = []
                    allCharacters.forEach( (character) => {
                                              const ch = Scrite.document.structure.findCharacter(character)
                                              if(!ch)
                                              characters.push(character)
                                          })
                    array = characters

                    if(count === 0)
                        Qt.callLater(reportAllCharactersAlreadyAdded)
                }

                function reportAllCharactersAlreadyAdded() {
                    MessageBox.information("No characters to add",
                                           "All existing characters have already been added to the notebook.",
                                           () => { Qt.callLater(dialog.close) })
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                VclLabel {
                    Layout.fillWidth: true

                    text: "Here are characters in your screenplay who don't already have a page in the Notebook."
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: Runtime.colors.primary.c100.background
                    border.color: Runtime.colors.primary.borderColor
                    border.width: 1

                    Flickable {
                        id: _charactersFlick

                        anchors.fill: parent
                        anchors.margins: 1

                        ScrollBar.vertical: VclScrollBar { }

                        clip: contentHeight > height
                        contentWidth: _charactersComboBoxLayout.width
                        contentHeight: _charactersComboBoxLayout.height
                        flickableDirection: Flickable.VerticalFlick

                        GridLayout {
                            id: _charactersComboBoxLayout

                            width: _charactersFlick.ScrollBar.vertical.needed ? _charactersFlick.width-20 : _charactersFlick.width
                            columns: 2
                            rowSpacing: 10
                            columnSpacing: 10

                            Repeater {
                                id: _charactersCheckBoxes

                                model: _charactersModel

                                delegate: VclCheckBox {
                                    required property int index
                                    required property string modelData

                                    text: modelData
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20

                    VclButton {
                        text: "Select All"
                        enabled: _charactersModel.count > 0

                        onClicked: {
                            for(let i=0; i<_charactersCheckBoxes.count; i++) {
                                let checkBox = _charactersCheckBoxes.itemAt(i)
                                checkBox.checked = true
                            }
                        }
                    }

                    VclButton {
                        text: "Unselect All"
                        enabled: _charactersModel.count > 0

                        onClicked: {
                            for(let i=0; i<_charactersCheckBoxes.count; i++) {
                                let checkBox = _charactersCheckBoxes.itemAt(i)
                                checkBox.checked = false
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    VclButton {
                        text: "Add Selected"
                        enabled: _charactersModel.count > 0

                        onClicked: {
                            let count = 0
                            for(let i=0; i<_charactersCheckBoxes.count; i++) {
                                let checkBox = _charactersCheckBoxes.itemAt(i)
                                if(checkBox.checked) {
                                    const ch = Scrite.document.structure.addCharacter(checkBox.text)
                                    if(ch)
                                    count = count+1
                                }
                            }

                            if(count > 0) {
                                MessageBox.information("Characters Added",
                                                       "A total of " + (count === 1 ? "one character was" : count + " characters were") + " added.",
                                                       dialog.close)
                            } else {
                                MessageBox.information("No Characters Added",
                                                       "No characters were added.",
                                                       dialog.close)
                            }

                            let notebookView = ObjectRegistry.find("notebookView")
                            if(notebookView)
                                notebookView.scheduleSwitchTo("Characters")
                        }
                    }
                }
            }
        }
    }
}
