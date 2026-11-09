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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../../globals"
import "../../helpers"
import "../../dialogs"
import "../../controls"
import ".."
import "../tabs"
import "../menus"

AbstractNotebookTabPage {
    id: root

    hostUri: "Scrite.App.Notebook.CharacterPage"

    notes: _private.notes

    readonly property Character character: owner as Character
    tabBarName: _private.character.name
    currentTabUri: Runtime.notebookSettings.characterPageTabUri

    backgroundColor: Runtime.colors.tintTx(_private.character.color, Runtime.colors.sceneHeadingTint)

    NotebookPageTab {
        uri: "Scrite.NotebookTab.CharacterPage.InformationTab"
        title: "Information"
        order: 100

        CharacterInformationTab {
            character: _private.character
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.CharacterPage.RelationshipsTab"
        title: "Relationships"
        order: 200

        CharacterRelationshipsTab {
            character: _private.character

            onSwitchRequest: (item) => { root.switchRequest(item) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.CharacterPage.NotesTab"
        title: "Notes"
        order: 300

        NotesTab {
            notes: _private.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
            onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.CharacterPage.AttributesTab"
        title: "Attributes"
        order: 400

        AttributesTab {
            notes: _private.notes
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: true
        tooltip: "Export current character report as a PDF or ODT."

        onTriggered: () => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = _private.character
                         ReportConfigurationDialog.launch(generator)
                     }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("delete")

        enabled: true
        tooltip: "Delete current character."

        onTriggered: () => {
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

    onCurrentTabUriChanged: Runtime.notebookSettings.characterPageTabUri = currentTabUri

    QtObject {
        id: _private

        property Notes notes: character ? character.notes : null

        property Character character: root.pageData ? root.pageData.notebookItemObject.character : null

        readonly property Component newNoteMenu: NewNoteMenu {
            notes: _private.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
        }

        function popupNewNoteMenu(source) {
            let menu = newNoteMenu.createObject(source) as NewNoteMenu
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
            let menu = colorMenu.createObject(source) as ColorMenu
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
            return menu
        }
    }
}
