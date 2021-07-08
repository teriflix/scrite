/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQml 2.13
import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

import Scrite 1.0

Item {
    property Form form: note ? note.form : null
    property Note note
    clip: true

    Flickable {
        id: formFlickable
        width: Math.max(200, Math.min(parent.width-17, 800))
        height: parent.height
        contentWidth: formContentLayout.width
        contentHeight: formContentLayout.height
        anchors.centerIn: parent
        ScrollBar.vertical: formVScrollBar
        ScrollBar.horizontal: formHScrollBar

        Column {
            id: formContentLayout
            width: formVScrollBar.required ? formFlickable.width - 17 : formFlickable.width
            spacing: 20

            Item {
                width: parent.width
                height: 1
            }

            TextField2 {
                id: titleField
                text: note ? note.title : ""
                width: parent.width
                wrapMode: Text.WordWrap
                font.bold: true
                font.pointSize: app.idealFontPointSize + 2
                placeholderText: "Title"
                TabSequenceItem.manager: formTabManager
                TabSequenceItem.sequence: 0
                onTextChanged: {
                    if(note)
                        note.title = text
                }
                onActiveFocusChanged: {
                    if(activeFocus)
                        formFlickable.contentY = 0
                }
            }

            TextField2 {
                id: descriptionField
                text: note ? note.content : ""
                width: parent.width
                wrapMode: Text.WordWrap
                font.pointSize: app.idealFontPointSize
                placeholderText: "Description"
                TabSequenceItem.manager: formTabManager
                TabSequenceItem.sequence: 1
                onTextChanged: {
                    if(note)
                        note.content = text
                }
                onActiveFocusChanged: {
                    if(activeFocus)
                        formFlickable.contentY = 0
                }
            }

            Repeater {
                model: form.questionsModel

                FormField {
                    width: parent.width
                    spacing: parent.spacing/2
                    questionKey: objectItem.id
                    questionNumber: objectItem.number
                    question: objectItem.questionText
                    placeholderText: objectItem.answerHint
                    tabSequenceManager: formTabManager
                    tabSequenceIndex: 2+index
                    onCursorRectangleChanged: {
                        if(!textFieldHasActiveFocus)
                            return
                        var cr = cursorRectangle
                        cr = mapToItem(formContentLayout, cr.x, cr.y, cr.width, cr.height)
                        cr = Qt.rect(cr.x, cr.y-4, cr.width, cr.height+8)
                        formFlickable.ensureVisible(cr)
                    }
                    onTextFieldHasActiveFocusChanged: {
                        if(!textFieldHasActiveFocus)
                            return
                        var cr = mapToItem(formContentLayout, 0, 0, width, minHeight)
                        formFlickable.ensureVisible(cr)
                    }
                    onAnswerChanged: {
                        if(note)
                            note.setFormData(objectItem.id, answer)
                    }
                    Component.onCompleted: {
                        if(note)
                            answer = note.getFormData(objectItem.id)
                    }
                }
            }

            Item {
                width: parent.width
                height: 20
            }
        }

        function ensureVisible(cr) {
            var cy = contentY
            var ch = height
            if(cr.y < cy)
                cy = Math.max(cr.y, 0)
            else if(cr.y + cr.height > cy + ch)
                cy = Math.min(cr.y + cr.height - ch, contentHeight-ch)
            else
                return
            contentY = cy
        }
    }

    TabSequenceManager {
        id: formTabManager
        wrapAround: true
    }

    ScrollBar2 {
        id: formVScrollBar
        orientation: Qt.Vertical
        flickable: formFlickable
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    ScrollBar2 {
        id: formHScrollBar
        orientation: Qt.Horizontal
        flickable: formFlickable
        anchors.left: parent.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    Component.onCompleted: descriptionField.forceActiveFocus()
}
