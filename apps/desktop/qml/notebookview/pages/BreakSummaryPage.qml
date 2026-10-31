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
import QtQuick.Controls
import QtQuick.Layouts

import Scrite.App

import "../../globals"
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

        visible: _private.breakElement !== null

        TextTabBar {
            id: _tabBar

            Layout.topMargin: 11
            Layout.leftMargin: 11
            Layout.rightMargin: 11
            Layout.fillWidth: true

            name: _private.breakElement ? _private.breakElement.breakTitle : ""
            tabs: ["Information", "Notes", "Attributes"].concat(_private.singletonNotes.map(note => note.title))
            currentTab: Runtime.notebookSettings.breakPageTab
            switchTabHandlerEnabled: true

            onCurrentTabChanged: Runtime.notebookSettings.breakPageTab = currentTab
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            currentIndex: Math.min(_tabBar.currentTab, _tabBar.tabs.length - 1)

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible && _private.breakElement !== null

                sourceComponent: BreakInformationTab {
                    breakElement: _private.breakElement
                    breakKind: _private.breakKind
                    maxTextAreaSize: root.maxTextAreaSize
                    minTextAreaSize: root.minTextAreaSize
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible && _private.notes !== null

                sourceComponent: NotesTab {
                    notes: _private.notes

                    onSwitchRequest: (item) => { root.switchRequest(item) }
                    onDeleteNoteRequest: (note) => { root.deleteNoteRequest(note) }
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true

                active: visible && _private.notes !== null

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

    SctLabel {
        anchors.centerIn: parent

        width: parent.width * 0.6

        horizontalAlignment: Text.AlignHCenter
        text: "Create " + _private.breakKind.toLowerCase() + " break in the screenplay to capture a summary for it."
        visible: _private.breakElement === null
        wrapMode: Text.WordWrap

        font.bold: true
        font.pointSize: Runtime.notebookFontPointSize + 3
    }

    QtObject {
        id: _private

        property ScreenplayElement breakElement: root.pageData
        property Notes notes: breakElement ? breakElement.notes : null
        property string breakKind: breakElement ? (breakElement.notebookItemType === NotebookModel.EpisodeBreakType ? "Episode" : "Act") : "a"
        property var singletonNotes: notes ? notes.singletonNotes : []

        function deleteSingletonNote(note) {
            root.askDeleteConfirmation("Are you sure you want to delete this note?", () => {
                                           Qt.callLater(() => { _private.notes.removeNote(note) })
                                       })
        }
    }
}
