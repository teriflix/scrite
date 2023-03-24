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

TextArea {
    id: txtAreaInput
    property bool undoRedoEnabled: false
    property bool spellCheckEnabled: false

    palette: Scrite.app.palette
    selectByKeyboard: true
    selectByMouse: true
    // renderType: Text.NativeRendering
    Material.primary: primaryColors.key
    Material.accent: accentColors.key
    selectedTextColor: accentColors.c700.text
    selectionColor: accentColors.c700.background
    background: Rectangle {
        color: enabled ? primaryColors.c10.background : primaryColors.button.background

        Rectangle {
            width: parent.width
            height: txtAreaInput.activeFocus ? 2 : 1
            color: accentColors.c700.background
            visible: txtAreaInput.enabled
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4
        }
    }
    Keys.onReturnPressed: Transliterator.transliterateLastWord()
    Transliterator.defaultFont: font
    Transliterator.textDocument: textDocument
    Transliterator.cursorPosition: cursorPosition
    Transliterator.hasActiveFocus: activeFocus
    Transliterator.applyLanguageFonts: screenplayEditorSettings.applyUserDefinedLanguageFonts
    Transliterator.textDocumentUndoRedoEnabled: undoRedoEnabled
    Transliterator.spellCheckEnabled: spellCheckEnabled

    SpecialSymbolsSupport {
        anchors.top: parent.bottom
        anchors.left: parent.left
        textEditor: txtAreaInput
        textEditorHasCursorInterface: true
        enabled: !Scrite.document.readOnly
    }

    UndoHandler {
        enabled: !txtAreaInput.readOnly && txtAreaInput.activeFocus && txtAreaInput.undoRedoEnabled
        canUndo: txtAreaInput.canUndo
        canRedo: txtAreaInput.canRedo
        onUndoRequest: txtAreaInput.undo()
        onRedoRequest: txtAreaInput.redo()
    }

    SpellingSuggestionsMenu2 { }

    property var spellChecker: Transliterator.highlighter.findDelegate("SpellCheckSyntaxHighlighterDelegate")
    ContextMenuEvent.active: spellChecker ? !spellChecker.wordUnderCursorIsMisspelled : true
    ContextMenuEvent.mode: ContextMenuEvent.GlobalEventFilterMode
    ContextMenuEvent.onPopup: (mouse) => {
        if(!txtAreaInput.activeFocus) {
            txtAreaInput.forceActiveFocus()
            txtAreaInput.cursorPosition = txtAreaInput.positionAt(mouse.x, mouse.y)
        }
        __contextMenu.popup()
    }

    Menu2 {
        id: __contextMenu
        focus: false

        property bool __persistentSelection: false
        onAboutToShow: {
            __persistentSelection = txtAreaInput.persistentSelection
            txtAreaInput.persistentSelection = true
        }
        onAboutToHide: txtAreaInput.persistentSelection = __persistentSelection

        MenuItem2 {
            text: "Cut\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+X")
            enabled: txtAreaInput.selectedText !== ""
            onClicked: txtAreaInput.cut()
            focusPolicy: Qt.NoFocus
        }

        MenuItem2 {
            text: "Copy\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
            enabled: txtAreaInput.selectedText !== ""
            onClicked: txtAreaInput.copy()
            focusPolicy: Qt.NoFocus
        }

        MenuItem2 {
            text: "Paste\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+V")
            onClicked: txtAreaInput.paste()
            focusPolicy: Qt.NoFocus
        }
    }
}
