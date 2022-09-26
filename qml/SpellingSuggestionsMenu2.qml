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
// be used within a TextArea that employs Transliterator.spellCheckEnabled = true

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

SpellingSuggestionsMenu {
    id: root
    anchors.bottom: parent.bottom

    // Just setting this property should be enough
    property TextArea textArea

    // These are implied from textArea
    QtObject {
        id: _private
        property var transliterator // of type Transliterator (see notes below)
        property var highlighter    // of type FontSyntaxHighlighter
        property int cursorPosition: -1

        function update() {
            transliterator = root.textArea ? root.textArea.Transliterator : null
            highlighter = transliterator ? transliterator.highlighter : null
            cursorPosition = -1
        }
    }

    onTextAreaChanged: Qt.callLater(_private.update)
    Component.onCompleted: {
        if(!textArea && Scrite.app.verifyType(parent, "QQuickTextArea"))
            textArea = parent

        Qt.callLater(_private.update)
    }

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
                              _private.highlighter.replaceWordAt(_private.cursorPosition, suggestion)
                              textArea.cursorPosition = _private.cursorPosition
                          }
                    }

    onAddToDictionaryRequest: () => {
                                  _private.highlighter.addWordAtPositionToDictionary(_private.cursorPosition)
                              }

    onAddToIgnoreListRequest: () => {
                                  _private.highlighter.addWordAtPositionToIgnoreList(_private.cursorPosition)
                              }

    MouseArea {
        parent: textArea ? textArea : root
        anchors.fill: parent
        enabled: textArea && textArea.activeFocus && _private.transliterator && _private.transliterator.spellCheckEnabled
        acceptedButtons: Qt.RightButton
        cursorShape: Qt.IBeamCursor
        onClicked: (mouse) => {
                       mouse.accepted = false

                       textArea.persistentSelection = true
                       if(!textArea.hasSelection) {
                           textArea.cursorPosition = textArea.positionAt(mouse.x, mouse.y)
                           if(_private.highlighter.wordUnderCursorIsMisspelled) {
                               root.spellingSuggestions = _private.highlighter.spellingSuggestionsForWordUnderCursor
                               root.popup()
                               mouse.accepted = true
                               return
                           }
                       }

                       textArea.persistentSelection = false
                   }
    }
}

/**
  NOTES:

    In _private data, I am unable to specify type of 'transliterator' as Transliterator, because
    textArea.Transliterator is a QObject* and it wont automatically typcast to Transliterator.

    Although I can specify type of 'highlighter' as 'FontSyntaxHighlighter', I did not want to
    do that because it would spoil the consistency of types in _private data object. Somehow it
    wouldn't look visually pleasent to me.

    QtObject {
        id: _private
        property Transliterator transliterator
        property FontSyntaxHighlighter highlighter
        ....
    }
  */
