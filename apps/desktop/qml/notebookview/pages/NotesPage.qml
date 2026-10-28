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
import "../../controls"
import ".."
import "../tabs"

AbstractNotebookPage {
    id: root

    // If true, the page has tabs for the notes, the attributes, and each singleton note.
    // Otherwise it just shows the notes.
    property bool showTabs: false

    signal deleteNoteRequest(Note note)
    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.

    // Selects the tab of the singleton note. Returns false if this page does not have one for it.
    function showNote(note) {
        const index = _private.singletonNotes.indexOf(note)
        if(!root.showTabs || index < 0)
            return false

        _tabBar.currentTab = _tabBar.tabs.length - _private.singletonNotes.length + index
        return true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.showTabs ? 11 : 0

        TextTabBar {
            id: _tabBar

            Layout.fillWidth: true

            currentTab: 0
            name: _private.notes ? _private.notes.title : "Notes"
            switchTabHandlerEnabled: root.showTabs
            tabs: ["Notes", "Attributes"].concat(_private.singletonNotes.map(note => note.title))
            visible: root.showTabs
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            currentIndex: root.showTabs ? Math.min(_tabBar.currentTab, _tabBar.tabs.length - 1) : 0

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

                active: visible && root.showTabs

                sourceComponent: AttributesTab {
                    notes: _private.notes
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Repeater {
                model: root.showTabs ? _private.singletonNotes : []

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

    QtObject {
        id: _private

        property Notes notes: root.pageData ? root.pageData.notebookItemObject : null
        property var singletonNotes: notes ? notes.singletonNotes : []

        function deleteSingletonNote(note) {
            root.askDeleteConfirmation("Are you sure you want to delete this note?", () => {
                                           Qt.callLater(() => { _private.notes.removeNote(note) })
                                       })
        }
    }
}
