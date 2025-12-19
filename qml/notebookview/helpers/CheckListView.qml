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

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview/helpers"

Item {
    id: root

    property Note note

    property real maxTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 80
    property real minTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 20

    ColumnLayout {
        anchors.centerIn: parent

        width: Math.max(root.minTextAreaSize, Math.min(parent.width-20, root.maxTextAreaSize))
        height: parent.height

        spacing: 20

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }

        VclTextField {
            id: _title

            Layout.fillWidth: true

            placeholderText: "Title"
            tabItem: _description
            text: note ? note.title : ""
            wrapMode: Text.WordWrap

            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

            onTextChanged: {
                if(note)
                    note.title = text
            }
        }

        VclTextField {
            id: _description

            Layout.fillWidth: true

            backTabItem: _title
            placeholderText: "Description"
            tabItem: _checkListView
            text: note ? note.summary : ""
            wrapMode: Text.WordWrap

            font.pointSize: Runtime.idealFontMetrics.font.pointSize

            onTextChanged: {
                if(note)
                    note.summary = text
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Qt.rgba(0,0,0,0)
            border.width: _checkListView.height < _checkListView.contentHeight ? 1 : 0
            border.color: Runtime.colors.primary.borderColor

            ListView {
                id: _checkListView

                signal assumeFocusRequest(int focusItemIndex, bool tabReason)

                function switchFocusTo(index, tabReason) {
                    index = Math.min(Math.max(-1, index), _checkListModel.count)
                    if(index < 0) {
                        currentIndex = 0
                        _description.forceActiveFocus()
                        return
                    }

                    if(index === _checkListModel.count) {
                        currentIndex = -1
                        positionViewAtEnd()
                    } else {
                        currentIndex = index
                    }
                    Qt.callLater(assumeFocusRequest, currentIndex, tabReason)
                }

                ScrollBar.vertical: _vscrollBar

                anchors.fill: parent
                anchors.margins: 1

                model: _checkListModel
                clip: true

                delegate: CheckListItem {
                    id: _delegate

                    required property int index
                    required property bool _checked
                    required property string _text

                    Connections {
                        target: _checkListView

                        function onAssumeFocusRequest(focusItemIndex, tabReason) {
                            if(focusItemIndex === index)
                                _delegate.assumeFocus(tabReason)
                        }
                    }

                    text: _text
                    checked: _checked
                    width: _checkListView.width - (_checkListView.height < _checkListView.contentHeight ? 20 : 1)

                    onCheckedChanged: () => {
                                          _checked = checked
                                          _checkListModel.setProperty(index, "_checked", _checked)
                                          _checkListModel.modelUpdated()
                                      }

                    onTextEdited: (text) => {
                                      _text = text
                                      _checkListModel.setProperty(index, "_text", _text)
                                      _checkListModel.modelUpdated()
                                  }

                    onEditingFinished: () => {
                                           const newItem = {
                                               _checked: false,
                                               _text: ""
                                           }
                                           _checkListModel.insert(index+1, newItem)
                                           _checkListView.switchFocusTo(index+1)
                                           _checkListModel.modelUpdated()
                                       }

                    onScrollToNextItem: (tabReason) => {
                                            _checkListView.switchFocusTo(index+1, tabReason)
                                        }

                    onScrollToPreviousItem: (tabReason) => {
                                                if(index > 0 || tabReason)
                                                    _checkListView.switchFocusTo(index-1, tabReason)
                                            }
                }

                footer: CheckListItem {
                    id: _footer

                    function commit() {
                        if(text === "")
                            return

                        const newItem = {
                            _checked: checked,
                            _text: text
                        }
                        _checkListModel.append(newItem)
                        _checkListModel.modelUpdated()
                        Qt.callLater(reset)
                    }

                    function reset() {
                        checked = false
                        text = ""
                    }

                    Connections {
                        target: _checkListView
                        function onAssumeFocusRequest(focusItemIndex, tabReason) {
                            if(focusItemIndex < 0)
                                _footer.assumeFocus(tabReason)
                        }
                    }

                    width: _checkListView.width - (_checkListView.height < _checkListView.contentHeight ? 20 : 1)

                    opacity: userIsInteracting ? 1 : (text === "" ? 0.25 : 0.85)

                    onEditingFinished: {
                        commit()
                        _checkListView.switchFocusTo(_checkListView.count)
                    }

                    onScrollToPreviousItem: (tabReason) => {
                                                _checkListView.switchFocusTo(_checkListModel.count-1, tabReason)
                                            }
                }

                onCurrentIndexChanged: {
                    _checkListModel.modelUpdated()
                }

                onFocusChanged: {
                    if(activeFocus) {
                        currentIndex = 0
                        switchFocusTo(0, true)
                    }
                }

            }
        }
    }

    VclScrollBar {
        id: _vscrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        flickable: _checkListView
    }

    component CheckListItem : Item {
        id: _checkListItem

        property real topPadding: 2
        property real leftPadding: 2
        property real rightPadding: 2
        property real bottomPadding: 2

        property bool checked
        property bool userIsInteracting: _textField.activeFocus || _checkBox.activeFocus

        property string text

        signal textEdited(string text)
        signal editingFinished()
        signal scrollToPreviousItem(bool tabReason)
        signal scrollToNextItem(bool tabReason)

        function assumeFocus(tabReason) {
            _textField.forceActiveFocus()
            if(tabReason)
                _textField.selectAll()
        }

        height: _checkListItemLayout.height + topPadding + leftPadding

        RowLayout {
            id: _checkListItemLayout

            anchors.verticalCenter: parent.verticalCenter

            width: parent.width - parent.leftPadding - parent.rightPadding
            height: Math.max(_checkBox.height, _textField.height)

            spacing: 2

            CheckBox {
                id: _checkBox

                Layout.alignment: _textField.lineCount > 1 ? Qt.AlignTop : Qt.AlignBaseline

                checked: _checkListItem.checked
                onToggled: _checkListItem.checked = checked
            }

            TextAreaInput {
                id: _textField

                Layout.alignment: lineCount > 1 ? Qt.AlignTop : Qt.AlignVCenter
                Layout.fillWidth: true

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

                spellCheckEnabled: true
                text: _checkListItem.text
                undoRedoEnabled: true
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                onTextChanged: {
                    _checkListItem.text = text
                    if(activeFocus)
                        _checkListItem.textEdited(text)
                }
            }
        }
    }

    ListModel {
        id: _checkListModel

        Component.onDestruction: commitPendingItems()

        function modelUpdated() {
            dirty = true
            Runtime.execLater(_checkListModel, 250, saveUpdates)
        }

        function saveUpdates() {
            if(dirty && root.note) {
                var newContent = []
                for(var i=count-1; i>=0; i--) {
                    const item = get(i)
                    if(item._text === undefined || item._text === "") {
                        if(_checkListView.focus && i === _checkListView.currentIndex)
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

        function commitPendingItems() {
            if(_checkListView.footerItem)
                _checkListView.footerItem.commit()

            _checkListModel.saveUpdates()
        }

        function loadFromNote() {
            clear()

            const list = root.note ? root.note.content : []
            list.forEach( (item) => { append(item) } )
        }

        property bool dirty: false
    }

    onNoteChanged: _checkListModel.loadFromNote()
}
