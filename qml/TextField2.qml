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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

TextField {
    id: textField
    property alias completionStrings: completionModel.strings
    property alias completionPrefix: completionModel.completionPrefix
    property alias maxCompletionItems: completionModel.maxVisibleItems
    property int maxVisibleItems: maxCompletionItems
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
    property bool tabItemUponReturn: true
    selectedTextColor: accentColors.c700.text
    selectionColor: accentColors.c700.background
    selectByMouse: true
    font.pointSize: Scrite.app.idealFontPointSize

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
        onRequestCompletion: autoCompleteOrFocusNext(tabItemUponReturn)
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
        autoCompleteOrFocusNext(tabItemUponReturn)
        returnPressed()
    }
    Keys.onEnterPressed: {
        autoCompleteOrFocusNext(tabItemUponReturn)
        returnPressed()
    }

    Keys.onTabPressed: autoCompleteOrFocusNext(true)

    function autoCompleteOrFocusNext(doTabItem) {
        if(completionModel.hasSuggestion && completionModel.suggestion !== text) {
            text = includeSuggestion(completionModel.suggestion)
            editingFinished()
        } else if(tabItem && (doTabItem === undefined || doTabItem === true)) {
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
            var newText = Scrite.app.transliterationEngine.transliteratedParagraph(text, includingLastWord)
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
        font.pointSize: 2*Scrite.app.idealFontPointSize/3
        anchors.left: parent.left
        anchors.verticalCenter: parent.top
        anchors.verticalCenterOffset: parent.topPadding/2
        visible: parent.labelAlwaysVisible ? true : parent.text !== ""
    }

    FontMetrics {
        id: fontMetrics
        font: textField.font
    }

    Popup {
        id: completionViewPopup
        x: 0
        y: parent.height
        width: parent.width
        height: completionView.height + topInset + bottomInset + topPadding + bottomPadding
        focus: false
        closePolicy: textField.length === 0 ? Popup.CloseOnPressOutside : Popup.NoAutoClose
        contentItem: ListView {
            id: completionView
            clip: true
            model: completionModel
            FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            keyNavigationEnabled: false
            property real delegateHeight: fontMetrics.lineSpacing + 10
            delegate: Text {
                width: completionView.width - (completionView.contentHeight > completionView.height ? 20 : 1)
                height: completionView.delegateHeight
                text: string
                padding: 5
                font: textField.font
                color: index === completionView.currentIndex ? primaryColors.highlight.text : primaryColors.c10.text
                MouseArea {
                    id: textMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: completionModel.currentRow = index
                    cursorShape: Qt.PointingHandCursor
                    onClicked: completionModel.requestCompletion(parent.text)
                }
            }
            highlight: Rectangle {
                color: primaryColors.highlight.background
            }
            currentIndex: completionModel.currentRow
            height: Math.min(contentHeight, maxVisibleItems > 0 ? delegateHeight*maxVisibleItems : contentHeight)
            ScrollBar.vertical: ScrollBar2 { flickable: completionView }
        }
    }

    Component {
        id: backgroundComponent

        Item {
            implicitWidth: textField.width
            implicitHeight: fontMetrics.lineSpacing

            Rectangle {
                width: parent.width
                height: 1
                color: textField.activeFocus ? primaryColors.c700.background : primaryColors.c300.background
                anchors.bottom: parent.bottom
            }
        }
    }

    Component.onCompleted: {
        if(!Scrite.app.usingMaterialTheme) {
            background = backgroundComponent.createObject(textField)
            topPadding = topPadding + 4
            bottomPadding = bottomPadding + 4
        }
    }
}
