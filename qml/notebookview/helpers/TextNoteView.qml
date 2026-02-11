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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    property Note note

    property real maxTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 80
    property real minTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 20

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

        VclTextField {
            id: _title

            TabSequenceItem.manager: _tabManager
            TabSequenceItem.sequence: 0

            anchors.horizontalCenter: parent.horizontalCenter

            placeholderText: "Heading"
            text: note ? note.title : ""
            width: parent.width >= root.maxTextAreaSize+20 ? root.maxTextAreaSize : parent.width-20
            wrapMode: Text.WordWrap

            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

            onTextChanged: {
                if(note)
                    note.title = text
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

            lod: Runtime.notebookSettings.richTextNotesEnabled ? LodLoader.LOD.High : LodLoader.LOD.Low
            resetHeightBeforeLodChange: false
            resetWidthBeforeLodChange: false
            sanctioned: note

            lowDetailComponent: FlickableTextArea {
                DeltaDocument {
                    id: _noteContent
                    content: note.content
                }

                text: _noteContent.plainText
                placeholderText: "Content"
                tabSequenceIndex: 1
                tabSequenceManager: _tabManager

                background: Rectangle {
                    color: Runtime.colors.primary.windowColor
                    opacity: 0.15
                }

                onTextChanged: if(textArea.activeFocus) note.content = text

                function assumeFocus() {
                    textArea.forceActiveFocus()
                }
            }

            highDetailComponent: RichTextEdit {
                function assumeFocus() {
                    textArea.forceActiveFocus()
                }

                adjustTextWidthBasedOnScrollBar: false
                placeholderText: "Content"
                tabSequenceIndex: 1
                tabSequenceManager: _tabManager
                text: note.content

                onTextChanged: {
                    note.content = text
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

        attachments: note ? note.attachments : null
        orientation: ListView.Horizontal
    }

    AttachmentsDropArea {
        id: _attachmentsDropArea

        anchors.fill: parent

        allowMultiple: true
        target: note ? note.attachments : null
    }

    onNoteChanged: {
        if(note.objectName === "_newNote")
            _title.forceActiveFocus()
        else if(note.objectName === "_focusNote") {
            if(_fieldLoader.item)
                _fieldLoader.item.assumeFocus()
        }
        note.objectName = ""
    }
}
