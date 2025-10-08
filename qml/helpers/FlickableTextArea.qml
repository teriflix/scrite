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

    property int tabSequenceIndex: 0

    property bool undoRedoEnabled: true
    property bool spellCheckEnabled: true
    property bool scrollBarRequired: contentHeight > height
    property bool tabSequenceEnabled: true
    property bool enforceDefaultFont: true
    property bool enforceHeadingFontSize: false
    property bool adjustTextWidthBasedOnScrollBar: true

    property alias text: _textArea.text
    property alias font: _textArea.font
    property alias color: _textArea.color
    property alias readOnly: _textArea.readOnly
    property alias background: _textArea.background
    property alias textDocument: _textArea.textDocument
    property alias placeholderText: _textArea.placeholderText

    property Item tabItem
    property Item textArea: _textArea
    property Item backTabItem

    property SyntaxHighlighter syntaxHighlighter: _textArea.SyntaxHighlighter
    property TabSequenceManager tabSequenceManager

    signal editingFinished()

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    ScrollBar.vertical: VclScrollBar { flickable: root }

    contentWidth: _textArea.width
    contentHeight: _textArea.height

    clip: true

    TextArea {
        id: _textArea

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

        SyntaxHighlighter.delegates: [
            LanguageFontSyntaxHighlighterDelegate {
                enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                defaultFont: _textArea.font
                enforceDefaultFont: root.enforceDefaultFont
            },

            HeadingFontSyntaxHighlighterDelegate {
                enabled: root.enforceHeadingFontSize
                Component.onCompleted: initializeWithNormalFontAs(_textArea.font)
            },

            SpellCheckSyntaxHighlighterDelegate {
                id: _spellChecker
                enabled: root.spellCheckEnabled
                cursorPosition: _textArea.cursorPosition
            }
        ]
        SyntaxHighlighter.textDocument: textDocument
        SyntaxHighlighter.textDocumentUndoRedoEnabled: undoRedoEnabled

        LanguageTransliterator.popup: LanguageTransliteratorPopup {
            editorFont: _textArea.font
        }
        LanguageTransliterator.option: Runtime.language.activeTransliterationOption
        LanguageTransliterator.enabled: !readOnly

        ContextMenuEvent.mode: ContextMenuEvent.GlobalEventFilterMode
        ContextMenuEvent.active: !_spellChecker.wordUnderCursorIsMisspelled
        ContextMenuEvent.onPopup: (mouse) => {
            if(!_textArea.activeFocus) {
                _textArea.forceActiveFocus()
                _textArea.cursorPosition = _textArea.positionAt(mouse.x, mouse.y)
            }
            _contextMenu.popup()
        }

        TabSequenceItem.manager: tabSequenceManager
        TabSequenceItem.enabled: tabSequenceEnabled
        TabSequenceItem.sequence: tabSequenceIndex

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
            let cr = cursorRectangle
            cr = Qt.rect(cr.x, cr.y-4, cr.width, cr.height+8)

            let cy = root.contentY
            let ch = root.height
            if(cr.y < cy)
                cy = Math.max(cr.y, 0)
            else if(cr.y + cr.height > cy + ch)
                cy = Math.min(cr.y + cr.height - ch, height-ch)
            else
                return
            root.contentY = cy
        }

        onEditingFinished: root.editingFinished()
    }

    VclMenu {
        id: _contextMenu

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
