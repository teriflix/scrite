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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../../globals"
import "../../controls"
import "../../helpers"
import ".."

/**
Lets the user pick the parts of the Notebook to print: notes, scenes, characters, acts and episodes
are found by typing in the search field, and are added to a list below it, in which they can be
dragged into the order of printing, or removed. An empty list prints the entire Notebook.

The list belongs to the report, and so it is still there when the dialog goes to another page and
comes back, which makes this editor again. If the report was given a section by whoever launched the
dialog, then there is nothing to pick.
*/

ColumnLayout {
    id: root

    property scriteObjectConfigField fieldInfo
    property AbstractReportGenerator report

    // What is to be printed, in the order of printing. It is also the section of the report.
    property NotebookCheckedItems items

    // True if whoever launched the dialog gave the report a section.
    property bool sectionPreset: false

    function getReady() {
        if(!root.report)
            return

        const section = root.report.section
        if(section && Object.isOfType(section, "NotebookCheckedItems")) {
            root.items = section
            return
        }

        if(section) {
            root.sectionPreset = true
            return
        }

        root.items = _itemsComponent.createObject(root.report)
        root.report.section = root.items
    }

    spacing: 5

    SctLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap

        text: root.fieldInfo.label
    }

    SctLabel {
        Layout.fillWidth: true

        visible: root.sectionPreset
        wrapMode: Text.WordWrap
        font.italic: true

        text: "A specific section of the Notebook has already been chosen for this report."
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.rightMargin: 20

        visible: !root.sectionPreset

        spacing: 5

        SctTextField {
            id: _search

            Layout.fillWidth: true

            label: ""
            placeholderText: "Type to find a note, scene, character, act or episode to add ..."

            maxVisibleItems: 10
            maxCompletionItems: 10
            completionStrings: _lens.labels
            completionFilterMode: CompletionModel.ContainsPrefix
            completionAcceptsEnglishStringsOnly: false

            // Choosing a suggestion fills the field with it, which is when it is added.
            onEditingComplete: {
                _private.addByLabel(text)
                clear()
            }

            Keys.onShortcutOverride: (event) => {
                if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                    event.accepted = _search.completionHasSuggestions || _lens.keyOf(_search.text) !== ""
            }
        }

        // Picks notes from the whole Notebook, as it is shown in the Notebook.
        FlatToolButton {
            iconSource: Runtime.themedIcon("qrc:/icons/content/add_circle_outline.png")
            toolTipText: "Pick notes from the Notebook"
            enabled: root.items !== null

            onClicked: _private.pickNotes()
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: 320

        visible: !root.sectionPreset
        padding: 1 // keeps the rows inside the border of the background

        background: Rectangle {
            color: Runtime.colors.primary.c50.background
            border.width: 1
            border.color: Runtime.colors.primary.c50.text

            SctLabel {
                anchors.centerIn: parent

                visible: _listView.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                width: parent.width * 0.8

                text: "Nothing is chosen. The entire Notebook will be exported. Find what to print in the field above, and drag the rows to change the order of printing."
            }
        }

        ListView {
            id: _listView

            clip: true
            model: root.items

            moveDisplaced: Transition {
                NumberAnimation { properties: "y"; duration: 150 }
            }

            delegate: DropArea {
                id: _delegate

                required property int index
                required property string itemLabel

                width: _listView.width
                height: Runtime.idealFontMetrics.height + 20

                // Whatever is dragged over this row takes its place.
                onEntered: (drag) => {
                    const from = drag.source.modelIndex
                    if(from !== _delegate.index)
                        root.items.moveObject(from, _delegate.index)
                }

                Rectangle {
                    id: _row

                    // Follows the row in the model, which changes as the row is dragged over others.
                    readonly property int modelIndex: _delegate.index

                    width: _delegate.width
                    height: _delegate.height

                    z: _dragHandler.active ? 10 : 0
                    color: _dragHandler.active ? Runtime.colors.primary.highlight.background : Runtime.colors.primary.editor.background

                    Drag.active: _dragHandler.active
                    Drag.source: _row
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2

                    DragHandler {
                        id: _dragHandler

                        xAxis.enabled: false
                        cursorShape: Qt.OpenHandCursor
                    }

                    SctLabel {
                        anchors.left: parent.left
                        anchors.right: _removeLabel.left
                        anchors.verticalCenter: parent.verticalCenter

                        color: Color.textColorFor(_row.color)
                        elide: Text.ElideRight
                        padding: 5
                        leftPadding: 10
                        text: "≡   " + (_delegate.index + 1) + ". " + _delegate.itemLabel

                        font.family: Runtime.idealFontMetrics.font.family
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        font.capitalization: Runtime.idealFontMetrics.font.capitalization
                    }

                    SctLabel {
                        id: _removeLabel

                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        color: Color.textColorFor(_row.color)
                        padding: 10
                        rightPadding: 15
                        text: "✕"

                        MouseArea {
                            anchors.fill: parent

                            cursorShape: Qt.PointingHandCursor

                            onClicked: root.items.removeObject(_delegate.index)
                        }
                    }

                    // While dragging, the row is taken out of its delegate, so that it can be moved
                    // over the other rows.
                    states: State {
                        when: _dragHandler.active

                        ParentChange {
                            target: _row
                            parent: _listView.contentItem
                        }
                    }
                }
            }
        }
    }

    SctLabel {
        Layout.fillWidth: true

        visible: !root.sectionPreset
        padding: 5

        text: _listView.count === 0 ? "Entire Notebook" : ("" + _listView.count + " Item(s) Chosen")
    }

    Component {
        id: _itemsComponent

        NotebookCheckedItems { }
    }

    NotebookFlatLens {
        id: _lens

        sourceModel: root.items ? root.items.notebook : null
        kinds: ["note", "scene", "character", "break"]
    }

    QtObject {
        id: _private

        // Adds the notes that the user picks in the dialog box, in the order in which they were picked.
        function pickNotes() {
            NotebookNotesPickerDialog.launch(root.items.notebook, (keys) => {
                                                 for(let i=0; i<keys.length; i++)
                                                     root.items.addKey(keys[i])
                                             })
        }

        function addByLabel(label) {
            const key = _lens.keyOf(label)
            if(key === "" || !root.items)
                return

            root.items.addKey(key)
            _search.text = ""
        }
    }
}
