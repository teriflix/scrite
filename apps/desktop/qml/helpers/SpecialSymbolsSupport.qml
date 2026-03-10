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

import QtQuick
import QtQuick.Controls

import io.scrite.components



import "../globals"
import "../controls"

Item {
    id: root
    property Item textEditor
    property bool textEditorHasCursorInterface: false
    property bool includeEmojis: true
    property bool showingSymbols: _symbolMenu.visible

    signal symbolSelected(string text)

    EventFilter.target: textEditor
    EventFilter.active: textEditor !== null && root.enabled && textEditor.activeFocus
    EventFilter.events: [6]
    EventFilter.onFilter: {
        if(!root.enabled)
            return

        if(textEditorHasCursorInterface && textEditor.readOnly)
            return

        if(event.key === Qt.Key_F3) {
            _symbolMenu.visible = true
            result.filer = true
            result.accepted = true
        }
    }

    VclMenu {
        id: _symbolMenu

        width: 514

        focus: false
        autoWidth: false

        VclMenuItem {
            width: _symbolMenu.width
            height: 400
            focusPolicy: Qt.NoFocus
            background: Item { }
            contentItem: SpecialSymbolsPanel {
                includeEmojis: root.includeEmojis
                onSymbolClicked: {
                    if(!root.enabled)
                        return

                    if(textEditorHasCursorInterface) {
                        if(textEditor.readOnly)
                            return

                        var cp = textEditor.cursorPosition
                        textEditor.insert(textEditor.cursorPosition, text)
                        Utils.execLater(textEditor, 250, function() { textEditor.cursorPosition = cp + text.length })
                        _symbolMenu.close()
                        textEditor.forceActiveFocus()
                    } else {
                        _symbolMenu.close()
                        symbolSelected(text)
                    }
                }
            }
        }
    }

    component SpecialSymbolsPanel : Rectangle {
        property bool includeEmojis: true
        readonly property var symbols: [
            {
                "title": "Symbols",
                "symbols": ["¡", "¢", "£", "¤", "¥", "€", "¦", "§", "¨", "©", "ª", "«", "¬", "", "®", "¯", "°", "±", "²", "³", "´", "µ", "¶", "·", "¸", "¹", "º", "»", "¼", "½", "¾", "¿", "Α", "Β", "Γ", "Δ", "Ε", "Ζ", "Η", "Θ", "Ι", "Κ", "Λ", "Μ", "Ν", "Ξ", "Ο", "Π", "Ρ", "Σ", "Τ", "Υ", "Φ", "Χ", "Ψ", "Ω", "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "λ", "μ", "ν", "ξ", "ο", "π", "ρ", "ς", "σ", "τ", "υ", "φ", "χ", "ψ", "ω", "ϑ", "ϒ", "ϖ", "†", "‡", "•", "…", "‰", "′", "″", "‾", "⁄", "℘", "ℑ", "ℜ", "™", "ℵ", "←", "↑", "→", "↓", "↔", "↵", "⇐", "⇑", "⇒", "⇓", "⇔", "∀", "∂", "∃", "∅", "∇", "∈", "∉", "∋", "∏", "∑", "−", "∗", "√", "∝", "∞", "∠", "∧", "∨", "∩", "∪", "∫", "∴", "∼", "≅", "≈", "≠", "≡", "≤", "≥", "⊂", "⊃", "⊄", "⊆", "⊇", "⊕", "⊗", "⊥", "⋅", "⌈", "⌉", "⌊", "⌋", "⟨", "⟩", "◊", "♠", "♣", "♥", "♦"]
            },
            {
                "title": "Letter",
                "symbols": ["Â", "Ã", "Ä", "Å", "Æ", "Ç", "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï", "Ð", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", "Ø", "Ù", "Ú", "Û", "Ü", "Ý", "Þ", "ß", "à", "á", "â", "ã", "ä", "å", "æ", "ç", "è", "é", "ê", "ë", "ì", "í", "î", "ï", "ð", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", "ø", "ù", "ú", "û", "ü", "ý", "þ", "ÿ", "Œ", "œ", "Š", "š", "Ÿ", "ƒ", "ˆ", "˜"]
            },
            {
                "title": "Emoji",
                "symbols": ["😁","😂","😃","😄","😅","😆","😉","😊","😋","😌","😍","😏","😒","😔","😖","😘","😚","😜","😝","😞","😠","😡","😢","😣","😤","😥","😨","😩","😪","😫","😭","😰","😱","😲","😳","😵","😷","😇","😈","😎","😐","😶","😸","😹","😺","😻","😼","😽","😾","😿","🙀","🙅","🙆","🙇","🙈","🙉","🙊","🙋","🙌","🙍","🙎","🙏"]
            }
        ]

        signal symbolClicked(string text)

        id: _symbolsView
        width: 500; height: 400
        color: Runtime.colors.primary.c100.background

        Rectangle {
            id: _symbolsPanel

            property int currentIndex: 0
            property bool currentIndexIsEmoji: symbols[_symbolsPanel.currentIndex].title === "Emoji"

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom

            width: 100
            color: Runtime.colors.primary.c700.background

            Column {
                width: parent.width

                Repeater {
                    model: symbols

                    delegate: Rectangle {
                        required property int index
                        required property var modelData

                        width: _symbolsPanel.width
                        height: 40
                        color: _symbolsPanel.currentIndex === index ? Runtime.colors.primary.windowColor : Qt.rgba(0,0,0,0)

                        VclLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            text: modelData.title
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            color: _symbolsPanel.currentIndex === index ? "black" : "white"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: _symbolsPanel.currentIndex = index
                        }
                    }
                }
            }
        }

        GridView {
            id: _symbolsGridView

            ScrollBar.vertical: VclScrollBar { flickable: _symbolsGridView }

            anchors.top: parent.top
            anchors.left: _symbolsPanel.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 5

            clip: true
            rightMargin: 14

            cellWidth: _symbolsPanel.currentIndexIsEmoji ? 50 : 40
            cellHeight: cellWidth
            model: symbols[_symbolsPanel.currentIndex].symbols
            header: Item {
                width: _symbolsGridView.width-14
                height: _symbolsPanel.currentIndexIsEmoji ? 35 : 0

                VclLabel {
                    visible: _symbolsPanel.currentIndexIsEmoji
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    anchors.centerIn: parent
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    text: includeEmojis ? "Emojis may not be included in PDF exports." : "Emojis are not supported in this text area."
                }
            }

            delegate: Item {
                required property int index
                required property string modelData

                width: _symbolsGridView.cellWidth
                height: _symbolsGridView.cellHeight
                enabled: !_symbolsPanel.currentIndexIsEmoji || includeEmojis
                opacity: enabled ? 1 : 0.5

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 1
                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor
                    opacity: 0.5
                }

                VclText {
                    anchors.centerIn: parent
                    text: modelData
                    font.pixelSize: parent.height * 0.6
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: symbolClicked(modelData)
                }
            }
        }
    }
}
