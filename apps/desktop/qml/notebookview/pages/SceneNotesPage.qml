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

AbstractNotebookPage {
    id: root

    property alias currentTab: _tabBar.currentTab

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.
    signal deleteNoteRequest(Note note)

    // Selects the tab of the singleton note. Returns false if this page does not have one for it.
    function showNote(note) {
        const index = _private.singletonNotes.indexOf(note)
        if(index < 0)
            return false

        _tabBar.currentTab = _tabBar.tabs.length - _private.singletonNotes.length + index
        return true
    }

    backgroundColor: Runtime.colors.tintTx(_private.scene.color, Runtime.colors.currentNoteTint)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 11

        TextTabBar {
            id: _tabBar

            Layout.fillWidth: true

            name: root.pageData ? root.pageData.notebookItemTitle.substr(0, root.pageData.notebookItemTitle.indexOf(']')+1) : "Scene"
            tabs: ["Synopsis", "Relationships", "Notes", "Comments", "Attributes"].concat(_private.singletonNotes.map(note => note.title))
            currentTab: Runtime.notebookSettings.sceneNotesPageTab
            switchTabHandlerEnabled: true

            onCurrentTabChanged: Runtime.notebookSettings.sceneNotesPageTab = currentTab
        }

        StackLayout {
            id: _contentArea

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            currentIndex: Math.min(_tabBar.currentTab, _tabBar.tabs.length - 1)

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: SceneSynopsisTab {
                    scene: _private.scene
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: SceneCharacterRelationshipsTab {
                    scene: _private.scene

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

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: SceneCommentsTab {
                    scene: _private.scene
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: AttributesTab {
                    notes: _private.notes
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Repeater {
                model: _private.singletonNotes

                delegate: Loader {
                    id: _singletonLoader

                    required property var modelData

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    active: visible

                    sourceComponent: SingletonNoteTab {
                        note: _singletonLoader.modelData
                        maxTextAreaSize: root.maxTextAreaSize
                        minTextAreaSize: root.minTextAreaSize

                        onDeleteRequest: (note) => { _private.deleteSingletonNote(note) }
                    }
                }
            }
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

    QtObject {
        id: _private

        property Notes notes: root.pageData ? root.pageData.notebookItemObject : null
        property Scene scene: notes ? notes.scene : null
        property var singletonNotes: notes ? notes.singletonNotes : []

        function deleteSingletonNote(note) {
            root.askDeleteConfirmation("Are you sure you want to delete this note?", () => {
                                           Qt.callLater(() => { _private.notes.removeNote(note) })
                                       })
        }

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
