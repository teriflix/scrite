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

// This QML component extends SpellingSuggestionsMenu so that it can easily
// be used within a TextArea that employs a SyntaxHighlighter with a
// SpellCheckSyntaxHighlighterDelegate delegate that's enabled.

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

SpellingSuggestionsMenu {
    id: root

    // Just setting this property should be enough
    property var textArea

    anchors.bottom: parent.bottom

    Component.onCompleted: { Qt.callLater(_private.update) }

    onTextAreaChanged: Qt.callLater(_private.update)

    onMenuAboutToShow: () => {
                           _private.cursorPosition = _private.textArea.cursorPosition
                           _private.textArea.persistentSelection = true
                       }

    onMenuAboutToHide: () => {
                           _private.textArea.persistentSelection = false
                           _private.textArea.forceActiveFocus()
                           _private.textArea.cursorPosition = _private.cursorPosition
                       }

    onReplaceRequest: (suggestion) => {
                          if(_private.cursorPosition >= 0) {
                              _private.spellCheck.replaceWordAt(_private.cursorPosition, suggestion)
                              _private.textArea.cursorPosition = _private.cursorPosition
                          }
                    }

    onAddToDictionaryRequest: () => {
                                  _private.spellCheck.addWordAtPositionToDictionary(_private.cursorPosition)
                              }

    onAddToIgnoreListRequest: () => {
                                  _private.spellCheck.addWordAtPositionToIgnoreList(_private.cursorPosition)
                              }

    MouseArea {
        parent: _private.textArea ? _private.textArea : root

        anchors.fill: parent

        enabled: _private.textArea && _private.textArea.activeFocus && _private.spellCheck && _private.spellCheck.enabled
        cursorShape: Qt.IBeamCursor
        acceptedButtons: Qt.RightButton

        onClicked: (mouse) => {
                       mouse.accepted = false

                       _private.textArea.persistentSelection = true
                       if(_private.textArea.selectedText === "") {
                           _private.textArea.cursorPosition = _private.textArea.positionAt(mouse.x, mouse.y)
                           if(_private.spellCheck.wordUnderCursorIsMisspelled) {
                               root.spellingSuggestions = _private.spellCheck.spellingSuggestionsForWordUnderCursor
                               root.popup()
                               mouse.accepted = true
                               return
                           }
                       }

                       _private.textArea.persistentSelection = false
                   }
    }

    // These are implied from textArea
    QtObject {
        id: _private

        property var spellCheck    // of type SpellCheckSyntaxHighlighterDelegate
        property int cursorPosition: -1
        property var syntaxHighlighter // of type SyntaxHighlighter
        property TextArea textArea: root.textArea as TextArea

        function update() {
            if(_private.textArea === null)
                root.textArea = Object.firstParentByType(root, "QQuickTextArea")

            syntaxHighlighter = Object.firstChildByType(_private.textArea, "SyntaxHighlighter")
            if(syntaxHighlighter)
                spellCheck = syntaxHighlighter.findDelegate("SpellCheckSyntaxHighlighterDelegate")

            cursorPosition = -1
        }
    }
}

