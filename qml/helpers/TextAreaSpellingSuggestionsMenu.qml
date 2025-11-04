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

// This QML component extends SpellingSuggestionsMenu so that it can easily
// be used within a TextArea that employs a SyntaxHighlighter with a
// SpellCheckSyntaxHighlighterDelegate delegate that's enabled.

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

SpellingSuggestionsMenu {
    id: root

    // Just setting this property should be enough
    property TextArea textArea

    anchors.bottom: parent.bottom

    Component.onCompleted: { Qt.callLater(_private.update) }

    onTextAreaChanged: Qt.callLater(_private.update)

    onMenuAboutToShow: () => {
                           _private.cursorPosition = textArea.cursorPosition
                           textArea.persistentSelection = true
                       }

    onMenuAboutToHide: () => {
                           textArea.persistentSelection = false
                           textArea.forceActiveFocus()
                           textArea.cursorPosition = _private.cursorPosition
                       }

    onReplaceRequest: (suggestion) => {
                          if(_private.cursorPosition >= 0) {
                              _private.spellCheck.replaceWordAt(_private.cursorPosition, suggestion)
                              textArea.cursorPosition = _private.cursorPosition
                          }
                    }

    onAddToDictionaryRequest: () => {
                                  _private.spellCheck.addWordAtPositionToDictionary(_private.cursorPosition)
                              }

    onAddToIgnoreListRequest: () => {
                                  _private.spellCheck.addWordAtPositionToIgnoreList(_private.cursorPosition)
                              }

    MouseArea {
        parent: textArea ? textArea : root

        anchors.fill: parent

        enabled: textArea && textArea.activeFocus && _private.spellCheck && _private.spellCheck.enabled
        cursorShape: Qt.IBeamCursor
        acceptedButtons: Qt.RightButton

        onClicked: (mouse) => {
                       mouse.accepted = false

                       textArea.persistentSelection = true
                       if(!textArea.hasSelection) {
                           textArea.cursorPosition = textArea.positionAt(mouse.x, mouse.y)
                           if(_private.spellCheck.wordUnderCursorIsMisspelled) {
                               root.spellingSuggestions = _private.spellCheck.spellingSuggestionsForWordUnderCursor
                               root.popup()
                               mouse.accepted = true
                               return
                           }
                       }

                       textArea.persistentSelection = false
                   }
    }

    // These are implied from textArea
    QtObject {
        id: _private

        property var spellCheck    // of type SpellCheckSyntaxHighlighterDelegate
        property int cursorPosition: -1
        property var syntaxHighlighter // of type SyntaxHighlighter

        function update() {
            if(root.textArea === null)
                root.textArea = Object.firstParentByType(root, "QQuickTextArea")

            syntaxHighlighter = Object.firstChildByType(root.textArea, "SyntaxHighlighter")
            if(syntaxHighlighter)
                spellCheck = syntaxHighlighter.findDelegate("SpellCheckSyntaxHighlighterDelegate")

            cursorPosition = -1
        }
    }
}

