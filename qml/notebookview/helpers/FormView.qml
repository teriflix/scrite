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

Item {
    id: root

    property int nrQuestionDigits: form ? evalNrQuestionDigits() : 2

    property real maxTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 80
    property real minTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 20

    property Form form: note ? note.form : null
    property Note note

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
        EventFilter.forwardEventTo(_flickable)
        result.filter = true
        result.accepted = true
    }

    clip: true

    SortFilterObjectListModel {
        id: _formQuestionsModel

        property bool filterForms: !Runtime.notebookSettings.showAllFormQuestions

        function formFilterFunction(form) {
            if(filterForms)
                return note.getFormData(form.id) !== ""
            return true;
        }

        sourceModel: form.questionsModel
        filterFunction: formFilterFunction

        onFilterFormsChanged: invalidate()
    }

    Flickable {
        id: _flickable

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

        ScrollBar.vertical: _vscrollBar
        ScrollBar.horizontal: _hscrollBar

        flickableDirection: Flickable.VerticalFlick
        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        anchors.centerIn: parent

        width: Math.max(root.minTextAreaSize, Math.min(parent.width-20, root.maxTextAreaSize))
        height: parent.height

        contentWidth: _formLayout.width
        contentHeight: _formLayout.height

        Column {
            id: _formLayout

            width: _vscrollBar.needed ? _flickable.width-20 : _flickable.width
            spacing: 20

            Column {
                property bool visibleToUser: GMath.doRectanglesIntersect( Qt.rect(x,y,width,height),
                                                    Qt.rect(0,_flickable.contentY,width,_flickable.height) )

                width: parent.width
                spacing: parent.spacing

                opacity: visibleToUser ? 1 : 0

                Item {
                    width: parent.width
                    height: 1
                }

                VclTextField {
                    id: _title

                    TabSequenceItem.manager: _tabManager
                    TabSequenceItem.sequence: 0

                    width: parent.width

                    placeholderText: "Title"
                    text: note ? note.title : ""
                    wrapMode: Text.WordWrap

                    font.bold: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

                    onTextChanged: {
                        if(note)
                            note.title = text
                    }

                    onActiveFocusChanged: {
                        if(activeFocus)
                            _flickable.contentY = 0
                    }
                }

                VclTextField {
                    id: _description

                    TabSequenceItem.manager: _tabManager
                    TabSequenceItem.sequence: 1

                    width: parent.width

                    placeholderText: "Description"
                    text: note ? note.summary : ""
                    wrapMode: Text.WordWrap

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                    onTextChanged: {
                        if(note)
                            note.summary = text
                    }

                    onActiveFocusChanged: {
                        if(activeFocus)
                            _flickable.contentY = 0
                    }
                }

                VclLabel {
                    width: parent.width

                    elide: Text.ElideRight
                    maximumLineCount: 2
                    text: form.moreInfoUrl == "" ? "" : "To learn more about this form, visit <a href=\"" + form.moreInfoUrl + "\">" + form.moreInfoUrl + "</a>"
                    visible: text !== ""
                    wrapMode: Text.WordWrap

                    onLinkActivated: Qt.openUrlExternally(form.moreInfoUrl)
                }
            }

            Row {
                spacing: 10

                VclLabel {
                    text: "View"

                    anchors.verticalCenter: parent.verticalCenter
                }

                VclRadioButton {
                    text: "All"
                    checked: Runtime.notebookSettings.showAllFormQuestions

                    onToggled: Runtime.notebookSettings.showAllFormQuestions = true
                }

                VclRadioButton {
                    text: "Answered"
                    checked: !Runtime.notebookSettings.showAllFormQuestions

                    onToggled: Runtime.notebookSettings.showAllFormQuestions = false
                }
            }

            Repeater {
                id: _fieldsRepeater

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

                model: _formQuestionsModel

                delegate: FormField {
                    required property int index
                    required property QtObject objectItem

                    property bool visibleToUser: GMath.doRectanglesIntersect( Qt.rect(x,y,width,height),
                                                        Qt.rect(0,_flickable.contentY,width,_flickable.height) )

                    Component.onCompleted: {
                        if(note)
                            answer = note.getFormData(objectItem.id)
                    }

                    anchors.right: parent.right

                    width: parent.width

                    answerLength: objectItem.type
                    indentation: objectItem.indentation*50
                    nrQuestionDigits: root.nrQuestionDigits
                    opacity: visibleToUser ? 1 : 0
                    placeholderText: objectItem.answerHint === "" ? "Your answer ..." : objectItem.answerHint
                    question: objectItem.questionText
                    questionKey: objectItem.id
                    questionNumber: objectItem.number
                    spacing: parent.spacing/2
                    tabSequenceIndex: 2+index
                    tabSequenceManager: _tabManager

                    onCursorRectangleChanged: {
                        if(!textFieldHasActiveFocus)
                            return
                        var cr = cursorRectangle
                        cr = mapToItem(_formLayout, cr.x, cr.y, cr.width, cr.height)
                        cr = Qt.rect(cr.x, cr.y-4, cr.width, cr.height+8)
                        _flickable.ensureVisible(cr)
                    }

                    onTextFieldHasActiveFocusChanged: {
                        if(!textFieldHasActiveFocus)
                            return
                        var cr = mapToItem(_formLayout, 0, 0, width, minHeight)
                        _flickable.ensureVisible(cr)
                    }

                    onAnswerChanged: {
                        if(note)
                            note.setFormData(objectItem.id, answer)
                    }

                    onFocusNextRequest: {
                        _fieldsRepeater.switchToNextField(index)
                    }

                    onFocusPreviousRequest: {
                        _fieldsRepeater.switchToPreviousField(index)
                    }
                }
            }

            Item {
                property bool visibleToUser: GMath.doRectanglesIntersect( Qt.rect(x,y,width,height),
                                                    Qt.rect(0,_flickable.contentY,width,_flickable.height) )

                width: parent.width
                height: 20

                opacity: visibleToUser ? 1 : 0
            }
        }
    }

    TabSequenceManager {
        id: _tabManager

        wrapAround: true
    }

    VclScrollBar {
        id: _vscrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        flickable: _flickable
        orientation: Qt.Vertical
    }

    VclScrollBar {
        id: _hscrollBar

        anchors.left: parent.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        flickable: _flickable
        orientation: Qt.Horizontal
    }

    onNoteChanged: {
        if(note.objectName === "_newNote")
            _description.forceActiveFocus()
        else if(note.objectName === "_focusNote") {
            _description.forceActiveFocus()
            _tabManager.focusNext()
        }
        note.objectName = ""
    }
}
