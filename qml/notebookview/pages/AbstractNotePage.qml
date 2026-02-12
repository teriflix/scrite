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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/dialogs"
import "qrc:/qml/notebookview"
import "qrc:/qml/notebookview/menus"
import "qrc:/qml/notebookview/helpers"

AbstractNotebookPage {
    id: root

    readonly property alias note: _private.note

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.

    backgroundColor: Runtime.colors.tint(_private.note.color, Runtime.colors.sceneHeadingTint)

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: true
        tooltip: "Export current text note as a PDF or ODT."

        onTriggered: (source) => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = _private.note
                         ReportConfigurationDialog.launch(generator)
                     }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("delete")

        enabled: true
        tooltip: "Delete current text note."

        onTriggered: (source) => {
                         root.askDeleteConfirmation("Are you sure you want to delete this text note?", confirmDeleteLater)
                     }

        function confirmDeleteLater() {
            Qt.callLater(confirmDelete)
        }

        function confirmDelete() {
            let notes = _private.note.notes
            notes.removeNote(_private.note)
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
        iconSource: "image://color/" + _private.note.color + "/1"

        onTriggered: (source) => {
                         if(colorMenu)
                            colorMenu.popup()
                         else
                            colorMenu = _private.popupColorMenu(source)
                     }
    }

    QtObject {
        id: _private

        property Note note: root.pageData ? root.pageData.notebookItemObject : null

        readonly property Component newNoteMenu: NewNoteMenu {
            notes: _private.note.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
        }

        function popupNewNoteMenu(source) {
            let menu = newNoteMenu.createObject(source)
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
            return menu
        }

        readonly property Component colorMenu: ColorMenu {
            selectedColor: _private.note.color

            onMenuItemClicked: (color) => {
                                   _private.note.color = color
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
