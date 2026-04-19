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

pragma ComponentBehavior: Bound

/**
  We this kind of a heavy TextField implementation because, we have to add

  - Transliteration support
  - Special symbols spport
  - Auto completion popups support
  - Cut/Copy/Paste context menu support

  None of these are available in vanilla TextField QML component from QtQuick Controls
  */

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../helpers"

TextField {
    id: root

    Material.theme: Runtime.colors.theme

    signal returnPressed()
    signal editingComplete()
    signal requestCompletion(string string)

    property var includeSuggestion: function(suggestion) { return suggestion }
    property var polishCompletionText: function(text) { return text }

    property int maxVisibleItems: maxCompletionItems
    property int minimumCompletionPrefixLength: 1

    property bool undoRedoEnabled: false
    property bool tabItemUponReturn: true
    property bool userTypedSomeText: false
    property bool labelAlwaysVisible: false
    property bool includeEmojiSymbols: true
    property bool enableTransliteration: false
    property bool singleClickAutoComplete: true

    property alias label: _label.text
    property alias labelColor: _label.color
    property alias labelTextAlign: _label.horizontalAlignment
    property alias showingSymbols: _specialSymbolSupport.showingSymbols
    property alias completionRow: _completionModel.currentRow
    property alias completionPrefix: _completionModel.completionPrefix
    property alias completionStrings: _completionModel.strings
    property alias maxCompletionItems: _completionModel.maxVisibleItems
    property alias completionSortMode: _completionModel.sortMode
    property alias completionFilterMode: _completionModel.filterMode
    property alias completionHasSuggestions: _completionModel.hasSuggestion
    property alias completionIgnoreSuffixAfter: _completionModel.ignoreSuffixAfter
    property alias completionAcceptsEnglishStringsOnly: _completionModel.acceptEnglishStringsOnly

    property Item tabItem
    property Item backTabItem

    Component.onDestruction: {
        if(activeFocus && !_editingCompleteEmitted) {
            editingComplete()
        }
    }

    Keys.onPressed: (event) => {
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

    Keys.onTabPressed: {
        autoCompleteOrFocusNext(true)
    }

    KeyNavigation.tab: tabItem
    KeyNavigation.backtab: backTabItem

    DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

    PlaceholderVisibility.visible: !activeFocus && text === ""

    LanguageTransliterator.popup: LanguageTransliteratorPopup {
        editorFont: root.font
    }
    LanguageTransliterator.option: Runtime.language.activeTransliterationOption
    LanguageTransliterator.enabled: !readOnly

    ContextMenuEvent.onPopup: (mouse) => {
        if(!root.activeFocus) {
            root.forceActiveFocus()
            root.cursorPosition = root.positionAt(mouse.x, mouse.y)
        }
        _contextMenu.popup()
    }

    selectByMouse: true
    // selectionColor: Runtime.colors.accent.c700.background
    // selectedTextColor: Runtime.colors.accent.c700.text

    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    leftPadding: 0

    background: Item {
        implicitWidth: 120
        implicitHeight: _fontMetrics.lineSpacing

        Rectangle {
            anchors.bottom: parent.bottom

            width: parent.width
            height: root.activeFocus ? 2 : 0.5

            color: root.activeFocus ? Runtime.colors.accent.c900.background : Runtime.colors.primary.c900.background
        }
    }

    CompletionModel {
        id: _completionModel

        property bool allowEnable: true
        property bool hasSuggestion: count > 0

        property string suggestion: currentCompletion

        enabled: allowEnable && root.activeFocus && root.length >= root.minimumCompletionPrefixLength
        sortStrings: false
        completionPrefix: root.text
        filterKeyStrokes: root.activeFocus

        onRequestCompletion: {
            root.autoCompleteOrFocusNext(root.tabItemUponReturn)
        }

        onHasSuggestionChanged: {
            if(hasSuggestion)
                _completionPopup.open()
            else
                _completionPopup.close()
        }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("undo")
        enabled: root.undoRedoEnabled && !root.readOnly && root.activeFocus && root.canUndo

        onTriggered: () => { root.undo() }
    }

    ActionHandler {
        action: ActionHub.editOptions.find("redo")
        enabled: root.undoRedoEnabled && !root.readOnly && root.activeFocus && root.canRedo

        onTriggered: () => { root.redo() }
    }

    SpecialSymbolsSupport {
        id: _specialSymbolSupport

        anchors.top: parent.bottom
        anchors.left: parent.left

        textEditor: root
        includeEmojis: root.includeEmojiSymbols
        textEditorHasCursorInterface: true
    }

    VclLabel {
        id: _label

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.top
        anchors.verticalCenterOffset: parent.topPadding/4

        text: root.placeholderText
        visible: root.labelAlwaysVisible ? true : root.text !== "" && root.activeFocus

        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
    }

    FontMetrics {
        id: _fontMetrics

        font: root.font
    }

    Popup {
        id: _completionPopup

        x: 0
        y: parent.height
        width: parent.width
        height: visible ? _completionView.height + topInset + bottomInset + topPadding + bottomPadding : 100

        focus: false
        closePolicy: root.length === 0 ? Popup.CloseOnPressOutside : Popup.NoAutoClose

        contentItem: ListView {
            id: _completionView

            property real delegateHeight: _fontMetrics.lineSpacing + 10

            ScrollBar.vertical: VclScrollBar { flickable: _completionView }

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            height: Math.min( Scrite.window.height*0.3, Math.min(contentHeight, root.maxVisibleItems > 0 ? delegateHeight*root.maxVisibleItems : contentHeight) )

            clip: true
            model: _completionModel
            currentIndex: _completionModel.currentRow
            keyNavigationEnabled: false

            highlightMoveDuration: 0
            highlightResizeDuration: 0
            highlight: Rectangle {
                color: Runtime.colors.primary.highlight.background
            }

            delegate: VclLabel {
                id: _completionDelegate

                required property int index
                required property var completionString

                width: _completionView.width - (_completionView.contentHeight > _completionView.height ? 20 : 1)
                height: _completionView.delegateHeight

                text: root.polishCompletionText(completionString)
                font: root.font
                padding: 5
                elide: Text.ElideRight

                MouseArea {
                    id: _textMouseArea

                    anchors.fill: parent

                    hoverEnabled: root.singleClickAutoComplete
                    cursorShape: root.singleClickAutoComplete ? Qt.PointingHandCursor : Qt.ArrowCursor

                    onClicked: {
                        if(root.singleClickAutoComplete || _completionModel.currentRow === _completionDelegate.index)
                            _completionModel.requestCompletion(parent.text)
                        else
                            _completionModel.currentRow = _completionDelegate.index
                    }

                    onDoubleClicked: {
                        _completionModel.requestCompletion(parent.text)
                    }

                    onContainsMouseChanged: {
                        if(root.singleClickAutoComplete)
                            _completionModel.currentRow = _completionDelegate.index
                    }
                }
            }
        }
    }

    VclMenu {
        id: _contextMenu

        property bool __persistentSelection: false

        focus: false

        VclMenuItem {
            text: "Cut\t" + ActionHub.editOptions.find("cut").shortcut
            enabled: root.selectedText !== ""
            focusPolicy: Qt.NoFocus

            onClicked: root.cut()
        }

        VclMenuItem {
            text: "Copy\t" + ActionHub.editOptions.find("copy").shortcut
            enabled: root.selectedText !== ""
            focusPolicy: Qt.NoFocus

            onClicked: root.copy()
        }

        VclMenuItem {
            text: "Paste\t" + ActionHub.editOptions.find("paste").shortcut
            focusPolicy: Qt.NoFocus

            onClicked: root.paste()
        }

        onAboutToShow: {
            __persistentSelection = root.persistentSelection
            root.persistentSelection = true
        }

        onAboutToHide: {
            root.persistentSelection = __persistentSelection
        }
    }

    property bool _editingCompleteEmitted: false

    function autoCompleteOrFocusNext(doTabItem) {
        if(_completionModel.hasSuggestion && _completionModel.suggestion !== text) {
            text = includeSuggestion(_completionModel.suggestion)
            editingFinished()
        } else if(tabItem && (doTabItem === undefined || doTabItem === true)) {
            editingFinished()
            tabItem.forceActiveFocus()
        } else
            editingFinished()
    }

    onTextEdited: {
        _completionModel.allowEnable = true
    }

    onFocusChanged: {
        _completionModel.allowEnable = true
    }

    onEditingFinished: {
        _completionModel.allowEnable = false
        if(!_editingCompleteEmitted) {
            _editingCompleteEmitted = true
            editingComplete()
        }
    }

    onActiveFocusChanged: {
        if(activeFocus && !readOnly) {
            selectAll()
            _editingCompleteEmitted = false
        } else {
            _completionPopup.close()
        }
    }
}
