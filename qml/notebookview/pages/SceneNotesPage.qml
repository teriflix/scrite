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

    backgroundColor: Runtime.colors.tint(_private.scene.color, Runtime.colors.currentNoteTint)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 11

        TextTabBar {
            id: _tabBar

            Layout.fillWidth: true

            name: root.pageData ? root.pageData.notebookItemTitle.substr(0, root.pageData.notebookItemTitle.indexOf(']')+1) : "Scene"
            tabs: ["Synopsis", "Relationships", "Notes", "Comments"]
            currentTab: Runtime.notebookSettings.sceneNotesPageTab
            switchTabHandlerEnabled: true

            onCurrentTabChanged: Runtime.notebookSettings.sceneNotesPageTab = currentTab
        }

        StackLayout {
            id: _contentArea

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            currentIndex: _tabBar.currentTab

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
        }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: true
        tooltip: "Export current scene report as a PDF or ODT."

        onTriggered: (source) => {
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
            selectedColor: _private.scene.color

            onMenuItemClicked: (color) => {
                                   _private.scene.color = color
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
