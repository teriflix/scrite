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

    hostUri: "Scrite.App.Notebook.SceneNotesPage.Tabs"

    notes: _private.notes
    tabBarName: root.pageData ? root.pageData.notebookItemTitle.substr(0, root.pageData.notebookItemTitle.indexOf(']')+1) : "Scene"
    currentTabUri: Runtime.notebookSettings.sceneNotesPageTabUri

    backgroundColor: Runtime.colors.tintTx(_private.scene.color, Runtime.colors.currentNoteTint)

    NotebookPageTab {
        uri: "Scrite.NotebookTab.Scene.Synopsis"
        title: "Synopsis"
        order: 100

        SceneSynopsisTab {
            scene: _private.scene
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.Scene.Relationships"
        title: "Relationships"
        order: 200

        SceneCharacterRelationshipsTab {
            scene: _private.scene

            onSwitchRequest: (item) => { root.switchRequest(item) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.Scene.Notes"
        title: "Notes"
        order: 300

        NotesTab {
            notes: _private.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
            onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.Scene.Comments"
        title: "Comments"
        order: 400

        SceneCommentsTab {
            scene: _private.scene
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    NotebookPageTab {
        uri: "Scrite.NotebookTab.Scene.Attributes"
        title: "Attributes"
        order: 500

        AttributesTab {
            notes: _private.notes
            maxTextAreaSize: root.maxTextAreaSize
            minTextAreaSize: root.minTextAreaSize
        }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: true
        tooltip: "Export current scene report as a PDF or ODT."

        onTriggered: () => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = _private.scene
                         ReportConfigurationDialog.launch(generator)
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
        iconSource: "image://color/" + _private.scene.color + "/1"

        onTriggered: (source) => {
                         if(colorMenu)
                            colorMenu.popup()
                         else
                            colorMenu = _private.popupColorMenu(source)
                     }
    }

    onCurrentTabUriChanged: Runtime.notebookSettings.sceneNotesPageTabUri = currentTabUri

    QtObject {
        id: _private

        property Notes notes: root.pageData ? root.pageData.notebookItemObject : null

        property Scene scene: notes ? notes.scene : null

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
            selectedColor: _private.scene.color

            onMenuItemClicked: (color) => {
                                   _private.scene.color = color
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
