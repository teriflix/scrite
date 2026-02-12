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
import "qrc:/qml/controls"
import "qrc:/qml/dialogs"
import "qrc:/qml/notebookview"
import "qrc:/qml/notebookview/menus"

Item {
    id: root

    required property Notes notes

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.
    signal deleteNoteRequest(Note note)

    Flickable {
        id: _flickable

        property int currentIndex: 0

        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        ScrollBar.vertical: _scrollBar

        anchors.fill: parent

        clip: true
        contentWidth: width
        contentHeight: _layout.height

        Flow {
            id: _layout

            width: _flickable.width

            Repeater {
                model: notes

                delegate: Item {
                    id: _noteItem

                    required property int index
                    required property var modelData

                    property Note note: modelData

                    width: _private.noteSize
                    height: _private.noteSize

                    BoxShadow {
                        anchors.fill: _noteVisual

                        visible: _flickable.currentIndex === index

                        opacity: 0.5
                    }

                    Rectangle {
                        id: _noteVisual

                        anchors.fill: parent
                        anchors.margins: 10

                        color: _flickable.currentIndex === index ? Runtime.colors.tint(_noteItem.note.color, Runtime.colors.currentNoteTint) : Runtime.colors.tint(_noteItem.note.color, Runtime.colors.sceneHeadingTint)

                        Column {
                            anchors.fill: parent
                            anchors.margins: 16

                            spacing: 8

                            VclLabel {
                                id: _noteHeading

                                width: parent.width

                                color: Color.isLight(parent.parent.color) ? Qt.rgba(0.2,0.2,0.2,1.0) : Qt.rgba(0.9,0.9,0.9,1.0)
                                elide: Text.ElideRight
                                maximumLineCount: 1
                                text: _noteItem.note.title

                                font.bold: true
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            VclLabel {
                                width: parent.width
                                height: parent.height - _noteHeading.height - parent.spacing

                                color: _noteHeading.color
                                elide: Text.ElideRight
                                opacity: 0.75
                                text: _noteItem.note.type === Note.TextNoteType ? _deltaDoc.plainText : _noteItem.note.summary
                                wrapMode: Text.WordWrap

                                font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                                DeltaDocument {
                                    id: _deltaDoc

                                    content: _noteItem.note.type === Note.TextNoteType ? _noteItem.note.content : {}
                                }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent

                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onClicked: (mouse) => {
                                       parent.forceActiveFocus()
                                       _flickable.currentIndex = index
                                       if(mouse.button === Qt.RightButton) {
                                           _private.popupNoteMenu(_noteItem.note, _noteItem)
                                       }
                                   }

                        onDoubleClicked: (mouse) => {
                                             root.switchRequest(_noteItem.note)
                                         }
                    }
                }
            }

            Item {
                width: _private.noteSize
                height: _private.noteSize

                visible: !Scrite.document.readOnly

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 10

                    color: Color.translucent(Runtime.colors.primary.c100.background, 0.5)

                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor

                    MouseArea {
                        ToolTip.text: "Add a new text or form note."
                        ToolTip.visible: containsMouse
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

                        anchors.fill: parent

                        hoverEnabled: true

                        onClicked: _private.popupNewNoteMenu(_newNoteButton)
                    }
                }

                FlatToolButton {
                    id: _newNoteButton

                    anchors.centerIn: parent

                    iconSource: "qrc:/icons/action/note_add.png"
                    toolTipText: "Add a new text or form note."

                    onClicked: _private.popupNewNoteMenu(_newNoteButton)
                }
            }
        }
    }

    VclScrollBar {
        id: _scrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        flickable: _flickable
        orientation: Qt.Vertical
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")

        enabled: notes.ownerType === Notes.StructureOwner || notes.ownerType === Notes.SceneOwner
        tooltip: {
            switch(notes.ownerType) {
            case Notes.StructureOwner:
                return "Exports all story notes into a PDF or ODT."
            case Notes.SceneOwner:
                return "Exports all scene notes into a PDF or ODT."
            }
            return ""
        }

        onTriggered: (source) => {
                         let generator = Scrite.document.createReportGenerator("Notebook Report")
                         generator.section = notes.owner
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

    QtObject {
        id: _private

        property real noteSize: {
            const minSize = 200
            return root.width > minSize ?
                        (root.width / Math.floor(root.width/minSize)) :
                        root.width
        }

        readonly property Component noteMenu: NoteMenu {
            onDeleteNoteRequest: () => { root.deleteNoteRequest(note) }
        }

        function popupNoteMenu(note, source) {
            let menu = noteMenu.createObject(source, {"note": note})
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
            return menu
        }

        readonly property Component newNoteMenu: NewNoteMenu {
            notes: root.notes

            onSwitchRequest: (item) => { root.switchRequest(item) }
        }

        function popupNewNoteMenu(source) {
            let menu = newNoteMenu.createObject(source)
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
            return menu
        }
    }
}
