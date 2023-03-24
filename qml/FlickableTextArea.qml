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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

Flickable {
    property Item textArea: __textArea
    property bool scrollBarRequired: contentHeight > height
    property bool adjustTextWidthBasedOnScrollBar: true
    property bool undoRedoEnabled: true
    property alias text: __textArea.text
    property alias font: __textArea.font
    property alias textDocument: __textArea.textDocument
    property Item tabItem
    property Item backTabItem
    property alias readonly: __textArea.readOnly
    property alias placeholderText: __textArea.placeholderText
    property alias readOnly: __textArea.readOnly
    property alias background: __textArea.background
    property bool enforceDefaultFont: true
    property bool enforceHeadingFontSize: false
    property bool spellCheckEnabled: true
    property TabSequenceManager tabSequenceManager
    property int tabSequenceIndex: 0
    property alias syntaxHighlighter: __textArea.syntaxHighlighter
    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

    signal editingFinished()

    id: textAreaFlickable
    clip: true
    contentWidth: __textArea.width
    contentHeight: __textArea.height
    ScrollBar.vertical: ScrollBar2 { flickable: textAreaFlickable }

    TextArea {
        id: __textArea
        property SyntaxHighlighter syntaxHighlighter: Transliterator.highlighter
        property var spellChecker: syntaxHighlighter.findDelegate("SpellCheckSyntaxHighlighterDelegate")
        width: textAreaFlickable.width - (textAreaFlickable.scrollBarRequired && textAreaFlickable.adjustTextWidthBasedOnScrollBar ? 20 : 0)
        height: Math.max(textAreaFlickable.height-topPadding-bottomPadding, contentHeight+20)
        font.pointSize: Scrite.app.idealFontPointSize
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        selectByMouse: true
        selectByKeyboard: true
        leftPadding: 5; rightPadding: 5
        topPadding: 5; bottomPadding: 5
        Transliterator.defaultFont: font
        Transliterator.textDocument: textDocument
        Transliterator.cursorPosition: cursorPosition
        Transliterator.hasActiveFocus: activeFocus
        Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
        Transliterator.textDocumentUndoRedoEnabled: undoRedoEnabled
        Transliterator.spellCheckEnabled: textAreaFlickable.spellCheckEnabled
        Transliterator.enforeDefaultFont: textAreaFlickable.enforceDefaultFont
        Transliterator.enforceHeadingFontSize: textAreaFlickable.enforceHeadingFontSize
        readOnly: Scrite.document.readOnly
        KeyNavigation.tab: textAreaFlickable.tabItem
        KeyNavigation.backtab: textAreaFlickable.backTabItem
        KeyNavigation.priority: KeyNavigation.AfterItem
        background: Item { }
        SpecialSymbolsSupport {
            anchors.top: parent.bottom
            anchors.left: parent.left
            textEditor: __textArea
            textEditorHasCursorInterface: true
            enabled: !Scrite.document.readOnly
        }
        UndoHandler {
            enabled: !__textArea.readOnly && __textArea.activeFocus && textAreaFlickable.undoRedoEnabled
            canUndo: __textArea.canUndo
            canRedo: __textArea.canRedo
            onUndoRequest: __textArea.undo()
            onRedoRequest: __textArea.redo()
        }
        SpellingSuggestionsMenu2 { }
        onCursorRectangleChanged: {
            var cr = cursorRectangle
            cr = Qt.rect(cr.x, cr.y-4, cr.width, cr.height+8)

            var cy = textAreaFlickable.contentY
            var ch = textAreaFlickable.height
            if(cr.y < cy)
                cy = Math.max(cr.y, 0)
            else if(cr.y + cr.height > cy + ch)
                cy = Math.min(cr.y + cr.height - ch, height-ch)
            else
                return
            textAreaFlickable.contentY = cy
        }
        onEditingFinished: textAreaFlickable.editingFinished()
        TabSequenceItem.manager: tabSequenceManager
        TabSequenceItem.sequence: tabSequenceIndex
    }

    ContextMenuEvent.active: __textArea.spellChecker ? !__textArea.spellChecker.wordUnderCursorIsMisspelled : true
    ContextMenuEvent.mode: ContextMenuEvent.GlobalEventFilterMode
    ContextMenuEvent.onPopup: (mouse) => {
        if(!__textArea.activeFocus) {
            __textArea.forceActiveFocus()
            __textArea.cursorPosition = __textArea.positionAt(mouse.x, mouse.y)
        }
        __contextMenu.popup()
    }

    Menu2 {
        id: __contextMenu

        property bool __persistentSelection: false
        onAboutToShow: {
            __persistentSelection = __textArea.persistentSelection
            __textArea.persistentSelection = true
        }
        onAboutToHide: __textArea.persistentSelection = __persistentSelection

        MenuItem2 {
            text: "Cut\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+X")
            enabled: __textArea.selectedText !== ""
            onClicked: __textArea.cut()
            focusPolicy: Qt.NoFocus
        }

        MenuItem2 {
            text: "Copy\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
            enabled: __textArea.selectedText !== ""
            onClicked: __textArea.copy()
            focusPolicy: Qt.NoFocus
        }

        MenuItem2 {
            text: "Paste\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+V")
            onClicked: __textArea.paste()
            focusPolicy: Qt.NoFocus
        }
    }
}
