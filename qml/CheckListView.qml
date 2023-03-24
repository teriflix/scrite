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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0
import "../js/utils.js" as Utils

Item {
    id: root
    property Note note

    Component.onDestruction: commitPendingItems()

    function commitPendingItems() {
        if(checkListView.footerItem)
            checkListView.footerItem.commit()

        checkListModel.saveUpdates()
    }

    Connections {
        target: root
        enabled: true
        function onNoteChanged() {
            if(root.note) {
                const list = root.note ? root.note.content : []
                list.forEach( (item) => { checkListModel.append(item) } )
            }
            enabled = false
        }
    }

    ListModel {
        id: checkListModel
        property bool dirty: false

        function modelUpdated() {
            dirty = true
            Utils.execLater(checkListModel, 250, saveUpdates)
        }

        function saveUpdates() {
            if(dirty && root.note) {
                var newContent = []
                for(var i=count-1; i>=0; i--) {
                    const item = get(i)
                    if(item._text === undefined || item._text === "") {
                        if(checkListView.focus && i === checkListView.currentIndex)
                            newContent.push(get(i))
                        else
                            remove(i)
                    } else
                        newContent.push(get(i))
                }
                newContent = newContent.reverse()
                root.note.content = newContent
                dirty = false
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.max(minTextAreaSize, Math.min(parent.width-17, maxTextAreaSize))
        height: parent.height
        spacing: 20

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }

        TextField2 {
            id: titleField
            text: note ? note.title : ""
            width: parent.width
            wrapMode: Text.WordWrap
            font.bold: true
            font.pointSize: Scrite.app.idealFontPointSize + 2
            placeholderText: "Title"
            tabItem: descriptionField
            onTextChanged: {
                if(note)
                    note.title = text
            }
            Layout.fillWidth: true
        }

        TextField2 {
            id: descriptionField
            text: note ? note.content : ""
            width: parent.width
            wrapMode: Text.WordWrap
            font.pointSize: Scrite.app.idealFontPointSize
            placeholderText: "Description"
            tabItem: checkListView
            backTabItem: titleField
            onTextChanged: {
                if(note)
                    note.content = text
            }
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Qt.rgba(0,0,0,0)
            border.width: checkListView.height < checkListView.contentHeight ? 1 : 0
            border.color: primaryColors.borderColor

            ListView {
                id: checkListView
                anchors.fill: parent
                anchors.margins: 1
                model: checkListModel
                clip: true
                onCurrentIndexChanged: checkListModel.modelUpdated()

                onFocusChanged: {
                    if(activeFocus) {
                        currentIndex = 0
                        switchFocusTo(0, true)
                    }
                }

                delegate: CheckListItem {
                    id: checkListItem
                    required property string _text
                    required property bool _checked
                    required property int index
                    text: _text
                    checked: _checked
                    width: checkListView.width - (checkListView.height < checkListView.contentHeight ? 20 : 1)

                    onCheckedChanged: () => {
                                          _checked = checked
                                          checkListModel.setProperty(index, "_checked", _checked)
                                          checkListModel.modelUpdated()
                                      }
                    onTextEdited: (text) => {
                                      _text = text
                                      checkListModel.setProperty(index, "_text", _text)
                                      checkListModel.modelUpdated()
                                  }
                    onEditingFinished: () => {
                                           const newItem = {
                                               _checked: false,
                                               _text: ""
                                           }
                                           checkListModel.insert(index+1, newItem)
                                           checkListView.switchFocusTo(index+1)
                                           checkListModel.modelUpdated()
                                       }
                    onScrollToNextItem: (tabReason) => { checkListView.switchFocusTo(index+1, tabReason) }
                    onScrollToPreviousItem: (tabReason) => {
                                                if(index > 0 || tabReason)
                                                    checkListView.switchFocusTo(index-1, tabReason)
                                            }

                    Connections {
                        target: checkListView
                        function onAssumeFocusRequest(focusItemIndex, tabReason) {
                            if(focusItemIndex === index)
                                checkListItem.assumeFocus(tabReason)
                        }
                    }
                }

                footer: CheckListItem {
                    id: footerItem
                    width: checkListView.width - (checkListView.height < checkListView.contentHeight ? 20 : 1)
                    onEditingFinished: {
                        commit()
                        checkListView.switchFocusTo(checkListView.count)
                    }
                    onScrollToPreviousItem: (tabReason) => {
                                                checkListView.switchFocusTo(checkListModel.count-1, tabReason)
                                            }
                    opacity: userIsInteracting ? 1 : (text === "" ? 0.25 : 0.85)

                    function commit() {
                        if(text === "")
                            return

                        const newItem = {
                            _checked: checked,
                            _text: text
                        }
                        checkListModel.append(newItem)
                        checkListModel.modelUpdated()
                        Qt.callLater(reset)
                    }

                    function reset() {
                        checked = false
                        text = ""
                    }

                    Connections {
                        target: checkListView
                        function onAssumeFocusRequest(focusItemIndex, tabReason) {
                            if(focusItemIndex < 0)
                                footerItem.assumeFocus(tabReason)
                        }
                    }
                }

                function switchFocusTo(index, tabReason) {
                    index = Math.min(Math.max(-1, index), checkListModel.count)
                    if(index < 0) {
                        currentIndex = 0
                        descriptionField.forceActiveFocus()
                        return
                    }

                    if(index === checkListModel.count) {
                        currentIndex = -1
                        positionViewAtEnd()
                    } else {
                        currentIndex = index
                    }
                    Qt.callLater(assumeFocusRequest, currentIndex, tabReason)
                }

                signal assumeFocusRequest(int focusItemIndex, bool tabReason)
            }
        }
    }

    component CheckListItem : Item {
        id: _checkListItem
        height: rowLayout.height + topPadding + leftPadding

        property real topPadding: 2
        property real leftPadding: 2
        property real rightPadding: 2
        property real bottomPadding: 2

        property string text
        property bool checked
        property alias font: textField.font
        property alias placeholderText: textField.placeholderText
        property bool userIsInteracting: textField.activeFocus || checkBox.activeFocus

        signal textEdited(string text)
        signal editingFinished()
        signal scrollToPreviousItem(bool tabReason)
        signal scrollToNextItem(bool tabReason)

        function assumeFocus(tabReason) {
            textField.forceActiveFocus()
            if(tabReason)
                textField.selectAll()
        }

        RowLayout {
            id: rowLayout
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - parent.leftPadding - parent.rightPadding
            height: Math.max(checkBox.height, textField.height)
            spacing: 2

            CheckBox {
                id: checkBox
                Layout.alignment: textField.lineCount > 1 ? Qt.AlignTop : Qt.AlignBaseline
                checked: _checkListItem.checked
                onToggled: _checkListItem.checked = checked
            }

            TextAreaInput {
                id: textField
                Layout.alignment: lineCount > 1 ? Qt.AlignTop : Qt.AlignVCenter
                Layout.fillWidth: true

                text: _checkListItem.text
                onTextChanged: {
                    _checkListItem.text = text
                    if(activeFocus)
                        _checkListItem.textEdited(text)
                }

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                undoRedoEnabled: true
                spellCheckEnabled: true

                TextDocument.cursorPosition: cursorPosition

                Keys.priority: Keys.BeforeItem
                Keys.onReturnPressed: (event) => {
                                          if(event.modifiers & Qt.ShiftModifier) {
                                              event.accepted = false
                                              return
                                          }
                                          _checkListItem.editingFinished()
                                          event.accepted = true
                                      }
                Keys.onUpPressed: (event) => {
                                      if(TextDocument.canGoUp()) {
                                          event.accepted = false
                                          return
                                      }
                                      _checkListItem.scrollToPreviousItem(false)
                                      event.accepted = true
                                  }
                Keys.onDownPressed: (event) => {
                                        if(TextDocument.canGoDown()) {
                                            event.accepted = false
                                            return
                                        }
                                        _checkListItem.scrollToNextItem(false)
                                        event.accepted = true
                                    }
                Keys.onTabPressed: (event) => {
                                       _checkListItem.scrollToNextItem(true)
                                       event.accepted = true
                                   }
                Keys.onBacktabPressed: (event) => {
                                           _checkListItem.scrollToPreviousItem(true)
                                           event.accepted = true
                                       }
            }
        }
    }
}
