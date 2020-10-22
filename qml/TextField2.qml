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

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

TextField {
    id: textField
    property alias completionStrings: completer.strings
    property Item tabItem
    property Item backTabItem
    property bool labelAlwaysVisible: false
    property alias label: labelText.text
    property bool enableTransliteration: false
    property bool includeEmojiSymbols: true
    property alias showingSymbols: specialSymbolSupport.showingSymbols
    selectedTextColor: "white"
    selectionColor: "blue"
    selectByMouse: true

    signal editingComplete()
    signal returnPressed()

    onEditingFinished: {
        transliterate(true)
        editingComplete()
    }
    Component.onDestruction: {
        transliterate(true)
        editingComplete()
    }

    onActiveFocusChanged: {
        if(activeFocus)
            selectAll()
    }

    Item {
        x: parent.cursorRectangle.x
        y: parent.cursorRectangle.y
        width: parent.cursorRectangle.width
        height: parent.cursorRectangle.height

        ToolTip.visible: parent.activeFocus && completer.hasSuggestion && completer.suggestion !== parent.text
        ToolTip.text: completer.suggestion
    }

    Completer {
        id: completer
        suggestionMode: Completer.CompleteSuggestion
        completionPrefix: textField.text.toUpperCase()
    }

    onTextEdited: transliterate(false)

    property bool userTypedSomeText: false
    Keys.onPressed: {
        if(event.text !== "")
            userTypedSomeText = true
    }

    Keys.onReturnPressed: {
        autoCompleteOrFocusNext()
        returnPressed()
    }
    Keys.onEnterPressed: {
        autoCompleteOrFocusNext()
        returnPressed()
    }

    Keys.onTabPressed: autoCompleteOrFocusNext()

    function autoCompleteOrFocusNext() {
        if(completer.hasSuggestion && completer.suggestion !== text) {
            text = completer.suggestion
            editingFinished()
        } else if(tabItem) {
            editingFinished()
            tabItem.forceActiveFocus()
        } else
            editingFinished()
    }

    KeyNavigation.tab: tabItem
    KeyNavigation.backtab: backTabItem

    function transliterate(includingLastWord) {
        if(includingLastWord === undefined)
            includingLastWord = false
        if(enableTransliteration & userTypedSomeText) {
            var newText = app.transliterationEngine.transliteratedParagraph(text, includingLastWord)
            if(text === newText)
                return
            userTypedSomeText = false
            text = newText
        }
    }

    SpecialSymbolsSupport {
        id: specialSymbolSupport
        anchors.top: parent.bottom
        anchors.left: parent.left
        textEditor: textField
        includeEmojis: parent.includeEmojiSymbols
        textEditorHasCursorInterface: true
    }

    Text {
        id: labelText
        text: parent.placeholderText
        font.pointSize: app.idealFontPointSize/2
        anchors.left: parent.left
        anchors.verticalCenter: parent.top
        visible: parent.labelAlwaysVisible ? true : parent.text !== ""
    }
}
