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
import "qrc:/qml/dialogs"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch() {
        var dlg = dialogComponent.createObject(root)
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch AddCharactersDialog")
        return null
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            width: Math.min(750, Scrite.window.width*0.8)
            height: Math.min(600, Scrite.window.height*0.9)
            title: "Add Existing Characters"

            contentItem: Item {
                enabled: !Scrite.document.readOnly

                GenericArrayModel {
                    id: charactersModel

                    Component.onCompleted: {
                        const allCharacters = Scrite.document.structure.allCharacterNames()
                        let characters = []
                        allCharacters.forEach( (character) => {
                                                const ch = Scrite.document.structure.findCharacter(character)
                                                if(!ch)
                                                  characters.push(character)
                                              })
                        array = characters
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
                            id: charactersFlick

                            anchors.fill: parent
                            anchors.margins: 1

                            ScrollBar.vertical: VclScrollBar { }

                            clip: contentHeight > height
                            contentWidth: charactersComboBoxLayout.width
                            contentHeight: charactersComboBoxLayout.height
                            flickableDirection: Flickable.VerticalFlick

                            GridLayout {
                                id: charactersComboBoxLayout

                                width: charactersFlick.ScrollBar.vertical.needed ? charactersFlick.width-20 : charactersFlick.width
                                columns: 2
                                rowSpacing: 10
                                columnSpacing: 10

                                Repeater {
                                    id: charactersCheckBoxes
                                    model: charactersModel

                                    VclCheckBox {
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
                            onClicked: {
                                for(let i=0; i<charactersCheckBoxes.count; i++) {
                                    let checkBox = charactersCheckBoxes.itemAt(i)
                                    checkBox.checked = true
                                }
                            }
                        }

                        VclButton {
                            text: "Unselect All"
                            onClicked: {
                                for(let i=0; i<charactersCheckBoxes.count; i++) {
                                    let checkBox = charactersCheckBoxes.itemAt(i)
                                    checkBox.checked = false
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        VclButton {
                            text: "Add Selected"
                            onClicked: {
                                for(let i=0; i<charactersCheckBoxes.count; i++) {
                                    let checkBox = charactersCheckBoxes.itemAt(i)
                                    if(checkBox.checked)
                                        Scrite.document.structure.addCharacter(checkBox.text)
                                }
                                _private.switchToCharactersTabLater()
                                Qt.callLater(dialog.close)
                            }
                        }
                    }
                }
            }
        }
    }

    QtObject {
        id: _private

        function switchToCharactersTabLater() {
            Utils.execLater(_private, 200, switchToCharactersTabNow)
        }

        function switchToCharactersTabNow() {
            Announcement.shout(Runtime.announcementIds.notebookNodeRequest, "Characters")
        }
    }
}
