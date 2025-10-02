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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Flickable {
    id: root

    property Item textArea: _textArea
    property bool scrollBarRequired: contentHeight > height
    property bool adjustTextWidthBasedOnScrollBar: true
    property bool undoRedoEnabled: true
    property alias text: _textArea.text
    property alias font: _textArea.font
    property alias textDocument: _textArea.textDocument
    property Item tabItem
    property Item backTabItem
    property alias readonly: _textArea.readOnly
    property alias placeholderText: _textArea.placeholderText
    property alias readOnly: _textArea.readOnly
    property alias background: _textArea.background
    property alias color: _textArea.color
    property bool enforceDefaultFont: true
    property bool enforceHeadingFontSize: false
    property bool spellCheckEnabled: true
    property TabSequenceManager tabSequenceManager
    property int tabSequenceIndex: 0
    property bool tabSequenceEnabled: true
    property alias syntaxHighlighter: _textArea.syntaxHighlighter
    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    signal editingFinished()

    clip: true
    contentWidth: _textArea.width
    contentHeight: _textArea.height
    ScrollBar.vertical: VclScrollBar { flickable: root }

    TextArea {
        id: _textArea

        property SyntaxHighlighter syntaxHighlighter: Transliterator.highlighter
        property var spellChecker: syntaxHighlighter.findDelegate("SpellCheckSyntaxHighlighterDelegate")

        width: root.width - (root.scrollBarRequired && root.adjustTextWidthBasedOnScrollBar ? 20 : 0)
        height: Math.max(root.height-topPadding-bottomPadding, contentHeight+20)

        topPadding: 5
        leftPadding: 5
        rightPadding: 5
        bottomPadding: 5

        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        selectByMouse: true
        selectByKeyboard: true

        font.pointSize: Runtime.idealFontMetrics.font.pointSize

        KeyNavigation.tab: root.tabItem
        KeyNavigation.backtab: root.backTabItem
        KeyNavigation.priority: KeyNavigation.AfterItem

        Transliterator.enabled: false
        Transliterator.defaultFont: font
        Transliterator.textDocument: textDocument
        Transliterator.cursorPosition: cursorPosition
        Transliterator.hasActiveFocus: activeFocus
        Transliterator.applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
        Transliterator.textDocumentUndoRedoEnabled: undoRedoEnabled
        Transliterator.spellCheckEnabled: root.spellCheckEnabled
        Transliterator.enforeDefaultFont: root.enforceDefaultFont
        Transliterator.enforceHeadingFontSize: root.enforceHeadingFontSize

        ImTransliterator.popup: ImTransliteratorPopup {
            editorFont: _textArea.font
        }
        ImTransliterator.enabled: !readOnly

        readOnly: Scrite.document.readOnly
        background: Item { }

        SpecialSymbolsSupport {
            anchors.top: parent.bottom
            anchors.left: parent.left
            textEditor: _textArea
            textEditorHasCursorInterface: true
            enabled: !Scrite.document.readOnly
        }
        UndoHandler {
            enabled: !_textArea.readOnly && _textArea.activeFocus && root.undoRedoEnabled
            canUndo: _textArea.canUndo
            canRedo: _textArea.canRedo
            onUndoRequest: _textArea.undo()
            onRedoRequest: _textArea.redo()
        }
        TextAreaSpellingSuggestionsMenu { }
        onCursorRectangleChanged: {
            var cr = cursorRectangle
            cr = Qt.rect(cr.x, cr.y-4, cr.width, cr.height+8)

            var cy = root.contentY
            var ch = root.height
            if(cr.y < cy)
                cy = Math.max(cr.y, 0)
            else if(cr.y + cr.height > cy + ch)
                cy = Math.min(cr.y + cr.height - ch, height-ch)
            else
                return
            root.contentY = cy
        }
        onEditingFinished: root.editingFinished()
        TabSequenceItem.manager: tabSequenceManager
        TabSequenceItem.enabled: tabSequenceEnabled
        TabSequenceItem.sequence: tabSequenceIndex
    }

    ContextMenuEvent.active: _textArea.spellChecker ? !_textArea.spellChecker.wordUnderCursorIsMisspelled : true
    ContextMenuEvent.mode: ContextMenuEvent.GlobalEventFilterMode
    ContextMenuEvent.onPopup: (mouse) => {
        if(!_textArea.activeFocus) {
            _textArea.forceActiveFocus()
            _textArea.cursorPosition = _textArea.positionAt(mouse.x, mouse.y)
        }
        __contextMenu.popup()
    }

    VclMenu {
        id: __contextMenu

        property bool __persistentSelection: false
        onAboutToShow: {
            __persistentSelection = _textArea.persistentSelection
            _textArea.persistentSelection = true
        }
        onAboutToHide: _textArea.persistentSelection = __persistentSelection

        VclMenuItem {
            text: "Cut\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+X")
            enabled: _textArea.selectedText !== ""
            onClicked: _textArea.cut()
            focusPolicy: Qt.NoFocus
        }

        VclMenuItem {
            text: "Copy\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
            enabled: _textArea.selectedText !== ""
            onClicked: _textArea.copy()
            focusPolicy: Qt.NoFocus
        }

        VclMenuItem {
            text: "Paste\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+V")
            onClicked: _textArea.paste()
            focusPolicy: Qt.NoFocus
        }
    }
}
