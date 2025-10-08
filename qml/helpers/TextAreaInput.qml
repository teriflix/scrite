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

TextArea {
    id: root

    property bool undoRedoEnabled: false
    property bool spellCheckEnabled: Runtime.screenplayEditorSettings.enableSpellCheck

    Material.accent: Runtime.colors.accent.key
    Material.primary: Runtime.colors.primary.key

    SyntaxHighlighter.delegates: [
        LanguageFontSyntaxHighlighterDelegate {
            enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
            defaultFont: root.font
        },

        SpellCheckSyntaxHighlighterDelegate {
            id: _spellChecker
            enabled: root.spellCheckEnabled
            cursorPosition: root.cursorPosition
        }
    ]
    SyntaxHighlighter.textDocument: textDocument
    SyntaxHighlighter.textDocumentUndoRedoEnabled: undoRedoEnabled

    LanguageTransliterator.popup: LanguageTransliteratorPopup {
        editorFont: root.font
    }
    LanguageTransliterator.option: Runtime.language.activeTransliterationOption
    LanguageTransliterator.enabled: !readOnly

    ContextMenuEvent.mode: ContextMenuEvent.GlobalEventFilterMode
    ContextMenuEvent.active: !_spellChecker.wordUnderCursorIsMisspelled
    ContextMenuEvent.onPopup: (mouse) => {
        if(!root.activeFocus) {
            root.forceActiveFocus()
            root.cursorPosition = root.positionAt(mouse.x, mouse.y)
        }
        _contextMenu.popup()
    }

    palette: Scrite.app.palette
    selectByMouse: true
    selectByKeyboard: true

    // renderType: Text.NativeRendering
    selectedTextColor: Runtime.colors.accent.c700.text
    selectionColor: Runtime.colors.accent.c700.background
    background: Rectangle {
        color: enabled ? Runtime.colors.primary.c10.background : Runtime.colors.primary.button.background

        Rectangle {
            width: parent.width
            height: root.activeFocus ? 2 : 1
            color: Runtime.colors.accent.c700.background
            visible: root.enabled
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4
        }
    }

    SpecialSymbolsSupport {
        anchors.top: parent.bottom
        anchors.left: parent.left
        textEditor: root
        textEditorHasCursorInterface: true
        enabled: !Scrite.document.readOnly
    }

    UndoHandler {
        enabled: !root.readOnly && root.activeFocus && root.undoRedoEnabled
        canUndo: root.canUndo
        canRedo: root.canRedo
        onUndoRequest: root.undo()
        onRedoRequest: root.redo()
    }

    TextAreaSpellingSuggestionsMenu { }

    VclMenu {
        id: _contextMenu

        focus: false

        property bool __persistentSelection: false

        onAboutToShow: {
            __persistentSelection = root.persistentSelection
            root.persistentSelection = true
        }
        onAboutToHide: root.persistentSelection = __persistentSelection

        VclMenuItem {
            text: "Cut\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+X")
            enabled: root.selectedText !== ""
            onClicked: root.cut()
            focusPolicy: Qt.NoFocus
        }

        VclMenuItem {
            text: "Copy\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
            enabled: root.selectedText !== ""
            onClicked: root.copy()
            focusPolicy: Qt.NoFocus
        }

        VclMenuItem {
            text: "Paste\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+V")
            onClicked: root.paste()
            focusPolicy: Qt.NoFocus
        }
    }
}
