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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/notebookview/tabs"
import "qrc:/qml/notebookview/menus"

AbstractNotebookPage {
    id: root

    property alias currentTab: _tabBar.currentTab

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.
    signal deleteNoteRequest(Note note)

    backgroundColor: Runtime.colors.tint(_private.character.color, Runtime.colors.sceneHeadingTint)

    ColumnLayout {
        anchors.fill: parent

        TextTabBar {
            id: _tabBar

            Layout.topMargin: 11
            Layout.leftMargin: 11
            Layout.rightMargin: 11
            Layout.fillWidth: true

            name: _private.character.name
            tabs: ["Information", "Relationships", "Notes"]
            currentTab: Runtime.notebookSettings.characterPageTab
            switchTabHandlerEnabled: true

            onCurrentTabChanged: Runtime.notebookSettings.characterPageTab = currentTab
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            currentIndex: _tabBar.currentTab

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: CharacterInformationTab {
                    character: _private.character
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: CharacterRelationshipsTab {
                    character: _private.character

                    onSwitchRequest: (item) => { root.switchRequest(item) }
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: NotesTab {
                    notes: _private.notes

                    onSwitchRequest: (item) => { root.switchRequest(item) }
                    onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
                }
            }
        }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: true
        tooltip: "Export current character report as a PDF or ODT."

        onTriggered: (source) => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = _private.character
                         ReportConfigurationDialog.launch(generator)
                     }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("delete")

        enabled: true
        tooltip: "Delete current character."

        onTriggered: (source) => {
                         root.askDeleteConfirmation("Are you sure you want to delete this character?", confirmDeleteLater)
                     }

        function confirmDeleteLater() {
            Qt.callLater(confirmDelete)
        }

        function confirmDelete() {
            root.switchRequest("Characters")
            Scrite.document.structure.removeCharacter(_private.character)
        }
    }

    ActionHandler {
        property Menu newNoteMenu

        action: ActionHub.notebookOperations.find("addNote")

        down: newNoteMenu !== null

        onTriggered: (source) => {
                         if(newNoteMenu)
                            newNoteMenu.popup()
                         else
                            newNoteMenu = _private.popupNewNoteMenu(source)
                     }
    }

    ActionHandler {
        property Menu colorMenu

        action: ActionHub.notebookOperations.find("noteColor")

        down: colorMenu !== null
        iconSource: "image://color/" + _private.character.color + "/1"

        onTriggered: (source) => {
                         if(colorMenu)
                            colorMenu.popup()
                         else
                            colorMenu = _private.popupColorMenu(source)
                     }
    }

    QtObject {
        id: _private

        property Notes notes: character ? character.notes : null
        property Character character: root.pageData ? root.pageData.notebookItemObject.character : null

        readonly property Component newNoteMenu: NewNoteMenu {
            notes: _private.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
        }

        function popupNewNoteMenu(source) {
            let menu = newNoteMenu.createObject(source)
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
            return menu
        }

        readonly property Component colorMenu: ColorMenu {
            selectedColor: _private.character.color

            onMenuItemClicked: (color) => {
                                   _private.character.color = color
                               }
        }

        function popupColorMenu(source) {
            let menu = colorMenu.createObject(source)
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
            return menu
        }
    }
}
