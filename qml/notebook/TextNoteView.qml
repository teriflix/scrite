/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
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
    id: textNoteView
    property Note note
    clip: true

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: {
        EventFilter.forwardEventTo(contentFieldLoader.item)
        result.filter = true
        result.accepted = true
    }

    Column {
        id: noteArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: attachmentsArea.top
        anchors.margins: 10
        anchors.bottomMargin: 0
        spacing: 10

        VclTextField {
            id: titleField
            text: note ? note.title : ""
            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
            placeholderText: "Heading"
            TabSequenceItem.manager: noteTabManager
            TabSequenceItem.sequence: 0
            onTextChanged: {
                if(note)
                    note.title = text
            }
            onActiveFocusChanged: {
                if(activeFocus) {
                    if(contentFieldLoader.item && contentFieldLoader.lod === contentFieldLoader.LodLoader.LOD.Low)
                        contentFieldLoader.item.contentY = 0
                }
            }
        }

        LodLoader {
            id: contentFieldLoader

            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
            height: parent.height - titleField.height - parent.spacing

            anchors.horizontalCenter: parent.horizontalCenter

            lod: Runtime.notebookSettings.richTextNotesEnabled ? LodLoader.LOD.High : LodLoader.LOD.Low
            sanctioned: note
            resetWidthBeforeLodChange: false
            resetHeightBeforeLodChange: false

            lowDetailComponent: FlickableTextArea {
                DeltaDocument {
                    id: noteContent
                    content: note.content
                }

                text: noteContent.plainText
                placeholderText: "Content"
                tabSequenceIndex: 1
                tabSequenceManager: noteTabManager

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
                text: note.content
                placeholderText: "Content"
                tabSequenceIndex: 1
                tabSequenceManager: noteTabManager
                adjustTextWidthBasedOnScrollBar: false

                background: Rectangle {
                    color: Runtime.colors.primary.windowColor
                    opacity: 0.15
                }

                onTextChanged: note.content = text

                function assumeFocus() {
                    textArea.forceActiveFocus()
                }
            }
        }
    }

    TabSequenceManager {
        id: noteTabManager
        wrapAround: true
    }

    AttachmentsView {
        id: attachmentsArea
        attachments: note ? note.attachments : null
        orientation: ListView.Horizontal
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    AttachmentsDropArea {
        id: attachmentsDropArea
        anchors.fill: parent
        allowMultiple: true
        target: note ? note.attachments : null
    }

    onNoteChanged: {
        if(note.objectName === "_newNote")
            titleField.forceActiveFocus()
        else if(note.objectName === "_focusNote") {
            if(contentFieldLoader.item)
                contentFieldLoader.item.assumeFocus()
        }
        note.objectName = ""
    }
}
