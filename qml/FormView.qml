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
    id: formView
    property Form form: note ? note.form : null
    property Note note
    property int nrQuestionDigits: form ? evalNrQuestionDigits() : 2
    clip: true

    function evalNrQuestionDigits() {
        var nrQs = form.questionCount
        if(nrQs < 10)
            return 1
        if(nrQs < 100)
            return 2
        return 3
    }

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: {
        EventFilter.forwardEventTo(formFlickable)
        result.filter = true
        result.accepted = true
    }

    SortFilterObjectListModel {
        id: formQuestionsModel
        sourceModel: form.questionsModel
        filterFunction: formFilterFunction

        property bool filterForms: !notebookSettings.showAllFormQuestions
        onFilterFormsChanged: invalidate()

        function formFilterFunction(form) {
            if(filterForms)
                return note.getFormData(form.id) !== ""
            return true;
        }
    }

    Flickable {
        id: formFlickable
        width: Math.max(minTextAreaSize, Math.min(parent.width-17, maxTextAreaSize))
        height: parent.height
        contentWidth: formContentLayout.width
        contentHeight: formContentLayout.height
        anchors.centerIn: parent
        ScrollBar.vertical: formVScrollBar
        ScrollBar.horizontal: formHScrollBar
        flickableDirection: Flickable.VerticalFlick
        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

        Column {
            id: formContentLayout
            width: formVScrollBar.needed ? formFlickable.width - 17 : formFlickable.width
            spacing: 20

            Column {
                width: parent.width
                spacing: parent.spacing
                property bool visibleToUser: Scrite.app.doRectanglesIntersect( Qt.rect(x,y,width,height),
                                                    Qt.rect(0,formFlickable.contentY,width,formFlickable.height) )
                opacity: visibleToUser ? 1 : 0

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
                    font.pointSize: Scrite.app.idealFontPointSize + 2
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
                    font.pointSize: Scrite.app.idealFontPointSize
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

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    text: form.moreInfoUrl == "" ? "" : "To learn more about this form, visit <a href=\"" + form.moreInfoUrl + "\">" + form.moreInfoUrl + "</a>"
                    visible: text !== ""
                    onLinkActivated: Qt.openUrlExternally(form.moreInfoUrl)
                }
            }

            Row {
                spacing: 10

                Label {
                    text: "View"
                    anchors.verticalCenter: parent.verticalCenter
                }

                RadioButton2 {
                    text: "All"
                    checked: notebookSettings.showAllFormQuestions
                    onToggled: notebookSettings.showAllFormQuestions = true
                }

                RadioButton2 {
                    text: "Answered"
                    checked: !notebookSettings.showAllFormQuestions
                    onToggled: notebookSettings.showAllFormQuestions = false
                }
            }

            Repeater {
                id: formFieldsRepeater
                model: formQuestionsModel

                function switchToNextField(index) {
                    if(index === count-1)
                        return

                    const item = itemAt(index+1)
                    item.assumeFocus(0)
                }

                function switchToPreviousField(index) {
                    if(index === 0)
                        return

                    const item = itemAt(index-1)
                    item.assumeFocus(-1)
                }

                FormField {
                    anchors.right: parent.right
                    width: parent.width
                    indentation: objectItem.indentation*50
                    spacing: parent.spacing/2
                    questionKey: objectItem.id
                    questionNumber: objectItem.number
                    question: objectItem.questionText
                    placeholderText: objectItem.answerHint === "" ? "Your answer ..." : objectItem.answerHint
                    answerLength: objectItem.type
                    tabSequenceManager: formTabManager
                    tabSequenceIndex: 2+index
                    nrQuestionDigits: formView.nrQuestionDigits
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
                    property bool visibleToUser: Scrite.app.doRectanglesIntersect( Qt.rect(x,y,width,height),
                                                        Qt.rect(0,formFlickable.contentY,width,formFlickable.height) )
                    opacity: visibleToUser ? 1 : 0

                    onFocusNextRequest: formFieldsRepeater.switchToNextField(index)
                    onFocusPreviousRequest: formFieldsRepeater.switchToPreviousField(index)
                }
            }

            Item {
                width: parent.width
                height: 20
                property bool visibleToUser: Scrite.app.doRectanglesIntersect( Qt.rect(x,y,width,height),
                                                    Qt.rect(0,formFlickable.contentY,width,formFlickable.height) )
                opacity: visibleToUser ? 1 : 0
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

    onNoteChanged: {
        if(note.objectName === "_newNote")
            descriptionField.forceActiveFocus()
        else if(note.objectName === "_focusNote") {
            descriptionField.forceActiveFocus()
            formTabManager.focusNext()
        }
        note.objectName = ""
    }
}
