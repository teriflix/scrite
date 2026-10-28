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
import "../../dialogs"
import "../../helpers"
import "../../controls"
import ".."
import "../tabs"

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 11

        TextTabBar {
            id: _tabBar

            Layout.fillWidth: true

            name: "Screenplay"
            tabs: ["Title Page", "Logline", "Notes", "Stats", "Attributes"].concat(_private.singletonNotes.map(note => note.title))
            currentTab: Runtime.notebookSettings.screenplayPageTab
            switchTabHandlerEnabled: true

            onCurrentTabChanged: Runtime.notebookSettings.screenplayPageTab = currentTab
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            currentIndex: Math.min(_tabBar.currentTab, _tabBar.tabs.length - 1)

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: ScreenplayTitlePageTab {
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: ScreenplayLoglineTab {
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: NotesTab {
                    notes: _private.structure.notes

                    onSwitchRequest: (item) => { root.switchRequest(item) }
                    onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: ScreenplayStatsTab { }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible

                sourceComponent: AttributesTab {
                    notes: _private.structure.notes
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
        tooltip: "Export entire screenplay notes as PDF or ODT."

        onTriggered: () => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = _private.screenplay
                         ReportConfigurationDialog.launch(generator)
                     }
    }

    QtObject {
        id: _private

        property Structure structure: Scrite.document.structure
        property Screenplay screenplay: Scrite.document.screenplay
        property var singletonNotes: structure.notes.singletonNotes

        function deleteSingletonNote(note) {
            root.askDeleteConfirmation("Are you sure you want to delete this note?", () => {
                                           Qt.callLater(() => { _private.structure.notes.removeNote(note) })
                                       })
        }
    }
}
