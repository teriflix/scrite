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

Item {
    id: textNoteView
    property Note note
    clip: true

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: {
        EventFilter.forwardEventTo(contentField)
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

        TextField2 {
            id: titleField
            text: note ? note.title : ""
            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
            font.bold: true
            font.pointSize: Scrite.app.idealFontPointSize + 2
            placeholderText: "Heading"
            TabSequenceItem.manager: noteTabManager
            TabSequenceItem.sequence: 0
            onTextChanged: {
                if(note)
                    note.title = text
            }
            onActiveFocusChanged: {
                if(activeFocus)
                    noteFlickable.contentY = 0
            }
        }

        RichTextEdit {
            id: contentField
            text: note ? note.content : ""
            placeholderText: "Content"
            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
            readOnly: Scrite.document.readOnly
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height - titleField.height - parent.spacing
            tabSequenceManager: noteTabManager
            tabSequenceIndex: 1
            adjustTextWidthBasedOnScrollBar: false
            ScrollBar.vertical: noteVScrollBar
            onTextChanged: {
                if(note)
                    note.content = text
            }
            background: Rectangle {
                color: primaryColors.windowColor
                opacity: 0.15
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

    AttachmentsDropArea2 {
        id: attachmentsDropArea
        anchors.fill: parent
        target: note ? note.attachments : null
    }

    onNoteChanged: {
        if(note.objectName === "_newNote")
            titleField.forceActiveFocus()
        else if(note.objectName === "_focusNote")
            contentField.textArea.forceActiveFocus()
        note.objectName = ""
    }
}
