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
import QtQuick.Controls.Material

import Scrite.App

import "../../globals"
import "../../controls"
import "../../helpers"

Item {
    id: root

    property Note note

    required property real maxTextAreaSize
    required property real minTextAreaSize

    clip: true

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object, event, result) => {
                              EventFilter.forwardEventTo(_fieldLoader.item)
                              result.filter = true
                              result.accepted = true
                          }

    Column {
        id: _layout

        anchors.bottom: _attachmentsArea.top
        anchors.bottomMargin: 0
        anchors.left: parent.left
        anchors.margins: 10
        anchors.right: parent.right
        anchors.top: parent.top

        spacing: 10

        SctTextField {
            id: _title

            TabSequenceItem.manager: _tabManager
            TabSequenceItem.sequence: 0

            anchors.horizontalCenter: parent.horizontalCenter

            placeholderText: "Heading"
            text: root.note ? root.note.title : ""
            width: parent.width >= root.maxTextAreaSize+20 ? root.maxTextAreaSize : parent.width-20
            wrapMode: Text.WordWrap

            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

            onTextEdited: {
                if(root.note)
                    root.note.title = text
            }

            onActiveFocusChanged: {
                if(activeFocus) {
                    if(_fieldLoader.item && _fieldLoader.lod === _fieldLoader.LodLoader.LOD.Low)
                        _fieldLoader.item.contentY = 0
                }
            }
        }

        LodLoader {
            id: _fieldLoader

            anchors.horizontalCenter: parent.horizontalCenter

            width: parent.width >= root.maxTextAreaSize+20 ? root.maxTextAreaSize : parent.width-20
            height: parent.height - _title.height - parent.spacing

            lod: Runtime.notebookSettings.richTextNotesEnabled && !root.note.preferPlainTextUi ? LodLoader.LOD.High : LodLoader.LOD.Low
            resetHeightBeforeLodChange: false
            resetWidthBeforeLodChange: false
            sanctioned: root.note

            lowDetailComponent: root.note.hasMarkdownContent ? _markdownTextNoteComponent : _plainTextNoteComponent

            highDetailComponent: RichTextEdit {
                function assumeFocus() {
                    textArea.forceActiveFocus()
                }

                adjustTextWidthBasedOnScrollBar: false
                placeholderText: "Content"
                tabSequenceIndex: 1
                tabSequenceManager: _tabManager
                text: root.note.content

                onTextEdited: {
                    root.note.content = text
                }
            }
        }
    }

    TabSequenceManager {
        id: _tabManager

        wrapAround: true
    }

    AttachmentsView {
        id: _attachmentsArea

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        attachments: root.note ? root.note.attachments : null
        orientation: ListView.Horizontal
    }

    AttachmentsDropArea {
        id: _attachmentsDropArea

        anchors.fill: parent

        allowMultiple: true
        target: root.note ? root.note.attachments : null
    }

    Component {
        id: _plainTextNoteComponent

        Rectangle {
            color: Runtime.colors.primary.c200.background

            FlickableTextArea {
                anchors.fill: parent
                anchors.margins: 4

                DeltaDocument {
                    id: _noteContent
                    content: root.note.content
                }

                text: _noteContent.plainText
                placeholderText: "Content"
                tabSequenceIndex: 1
                tabSequenceManager: _tabManager
                background.visible: false

                onTextEdited: root.note.content = text

                function assumeFocus() {
                    textArea.forceActiveFocus()
                }
            }
        }
    }

    Component {
        id: _markdownTextNoteComponent

        Rectangle {
            id: _markdownRoot

            property bool editMode: false

            color: Runtime.colors.primary.c200.background

            Connections {
                target: root

                function onNoteChanged() {
                    _markdownRoot.editMode = false
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 4

                spacing: 0

                RowLayout {
                    Layout.fillWidth: true

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: _markdownToolbar.implicitHeight

                        Row {
                            id: _markdownToolbar

                            anchors.verticalCenter: parent.verticalCenter

                            FeatureHost.uri: "Scrite.App.Notebook.TextNote.MarkdownToolbar"
                            FeatureHost.onFeatureAdded: (object) => {
                                                            const item = object as Item
                                                            if(item && item.note !== undefined)
                                                                item.note = Qt.binding( () => { return root.note } )
                                                       }
                        }
                    }

                    Switch {
                        visible: !Scrite.document.readOnly
                        text: _markdownRoot.editMode ? "Edit" : "View"
                        checked: _markdownRoot.editMode

                        onToggled: {
                            _markdownRoot.editMode = checked
                            if(checked)
                                _markdownTextArea.textArea.forceActiveFocus()
                        }
                    }
                }

                FlickableTextArea {
                    id: _markdownTextArea

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    DeltaDocument {
                        id: _noteContent
                        content: root.note.content
                    }

                    text: _noteContent.plainText
                    placeholderText: "Content"
                    tabSequenceIndex: 1
                    tabSequenceManager: _tabManager
                    readOnly: Scrite.document.readOnly || !_markdownRoot.editMode
                    textFormat: _markdownRoot.editMode ? TextEdit.PlainText : TextEdit.MarkdownText
                    markdownSyntaxEnabled: _markdownRoot.editMode
                    spellCheckEnabled: _markdownRoot.editMode
                    background.visible: false

                    onTextEdited: if(_markdownRoot.editMode) root.note.content = text

                    function assumeFocus() {
                        _markdownRoot.editMode = true
                        textArea.forceActiveFocus()
                    }
                }
            }

            function assumeFocus() {
                _markdownTextArea.assumeFocus()
            }
        }
    }

    onNoteChanged: {
        if(note) {
            if(note.objectName === "_newNote")
                _title.forceActiveFocus()
            else if(note.objectName === "_focusNote") {
                if(_fieldLoader.item)
                    _fieldLoader.item.assumeFocus()
            }
            note.objectName = ""
        }
    }
}
