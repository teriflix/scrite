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



import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: specialSymbolsSupport
    property Item textEditor
    property bool textEditorHasCursorInterface: false
    property bool includeEmojis: true
    property bool showingSymbols: symbolMenu.visible

    signal symbolSelected(string text)

    EventFilter.target: textEditor
    EventFilter.active: textEditor !== null && specialSymbolsSupport.enabled && textEditor.activeFocus
    EventFilter.events: [6]
    EventFilter.onFilter: {
        if(!specialSymbolsSupport.enabled)
            return

        if(textEditorHasCursorInterface && textEditor.readOnly)
            return

        if(event.key === Qt.Key_F3) {
            symbolMenu.visible = true
            result.filer = true
            result.accepted = true
        }
    }

    VclMenu {
        id: symbolMenu

        width: 514

        focus: false
        autoWidth: false

        VclMenuItem {
            width: symbolMenu.width
            height: 400
            focusPolicy: Qt.NoFocus
            background: Item { }
            contentItem: SpecialSymbolsPanel {
                includeEmojis: specialSymbolsSupport.includeEmojis
                onSymbolClicked: {
                    if(!specialSymbolsSupport.enabled)
                        return

                    if(textEditorHasCursorInterface) {
                        if(textEditor.readOnly)
                            return

                        var cp = textEditor.cursorPosition
                        textEditor.insert(textEditor.cursorPosition, text)
                        Utils.execLater(textEditor, 250, function() { textEditor.cursorPosition = cp + text.length })
                        symbolMenu.close()
                        textEditor.forceActiveFocus()
                    } else {
                        symbolMenu.close()
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

        id: symbolsView
        width: 500; height: 400
        color: Runtime.colors.primary.c100.background

        Rectangle {
            id: symbolsPanel

            property int currentIndex: 0
            property bool currentIndexIsEmoji: symbols[symbolsPanel.currentIndex].title === "Emoji"

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

                        width: symbolsPanel.width
                        height: 40
                        color: symbolsPanel.currentIndex === index ? Runtime.colors.primary.windowColor : Qt.rgba(0,0,0,0)

                        VclLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            text: modelData.title
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            color: symbolsPanel.currentIndex === index ? "black" : "white"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: symbolsPanel.currentIndex = index
                        }
                    }
                }
            }
        }

        GridView {
            id: symbolsGridView

            ScrollBar.vertical: VclScrollBar { flickable: symbolsGridView }

            anchors.top: parent.top
            anchors.left: symbolsPanel.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 5

            clip: true
            rightMargin: 14

            cellWidth: symbolsPanel.currentIndexIsEmoji ? 50 : 40
            cellHeight: cellWidth
            model: symbols[symbolsPanel.currentIndex].symbols
            header: Item {
                width: symbolsGridView.width-14
                height: symbolsPanel.currentIndexIsEmoji ? 35 : 0

                VclLabel {
                    visible: symbolsPanel.currentIndexIsEmoji
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

                width: symbolsGridView.cellWidth
                height: symbolsGridView.cellHeight
                enabled: !symbolsPanel.currentIndexIsEmoji || includeEmojis
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
