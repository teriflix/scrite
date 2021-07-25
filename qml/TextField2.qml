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
    property alias completionStrings: completionModel.strings
    property alias completionPrefix: completionModel.completionPrefix
    property int minimumCompletionPrefixLength: 1
    signal requestCompletion(string string)

    property Item tabItem
    property Item backTabItem
    property bool labelAlwaysVisible: false
    property alias label: labelText.text
    property bool enableTransliteration: false
    property bool includeEmojiSymbols: true
    property bool undoRedoEnabled: false
    property alias showingSymbols: specialSymbolSupport.showingSymbols
    property var includeSuggestion: function(suggestion) {
        return suggestion
    }
    selectedTextColor: accentColors.c700.text
    selectionColor: accentColors.c700.background
    selectByMouse: true
    font.pointSize: app.idealFontPointSize

    signal editingComplete()
    signal returnPressed()

    onEditingFinished: {
        transliterate(true)
        completionModel.allowEnable = false
        editingComplete()
    }

    Component.onDestruction: {
        transliterate(true)
        editingComplete()
    }

    onActiveFocusChanged: {
        if(activeFocus)
            selectAll()
        else
            completionViewPopup.close()
    }

    CompletionModel {
        id: completionModel
        property bool allowEnable: true
        property string suggestion: currentCompletion
        property bool hasSuggestion: count > 0
        enabled: allowEnable && textField.activeFocus
        sortStrings: false
        completionPrefix: textField.length >= textField.minimumCompletionPrefixLength ? textField.text : ""
        filterKeyStrokes: textField.activeFocus
        onRequestCompletion: autoCompleteOrFocusNext()
        property bool hasItems: count > 0
        onHasItemsChanged: {
            if(hasItems)
                completionViewPopup.open()
            else
                completionViewPopup.close()
        }
    }

    UndoHandler {
        enabled: undoRedoEnabled && !textField.readOnly && textField.activeFocus
        canUndo: textField.canUndo
        canRedo: textField.canRedo
        onUndoRequest: textField.undo()
        onRedoRequest: textField.redo()
    }

    onTextEdited: {
        transliterate(false)
        completionModel.allowEnable = true
    }

    onFocusChanged: completionModel.allowEnable = true

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
        if(completionModel.hasSuggestion && completionModel.suggestion !== text) {
            text = includeSuggestion(completionModel.suggestion)
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

    Popup {
        id: completionViewPopup
        x: 0
        y: parent.height
        width: parent.width
        height: completionView.contentHeight + topInset + bottomInset + topPadding + bottomPadding
        focus: false
        closePolicy: textField.length === 0 ? Popup.CloseOnPressOutside : Popup.NoAutoClose
        contentItem: ListView {
            id: completionView
            model: completionModel
            FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
            delegate: Text {
                width: completionView.width-1
                text: string
                padding: 5
                font: textField.font
                color: index === completionView.currentIndex ? primaryColors.highlight.text : primaryColors.c10.text
            }
            highlight: Rectangle {
                color: primaryColors.highlight.background
            }
            currentIndex: completionModel.currentRow
            height: contentHeight
        }
    }
}
