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
import io.scrite.components 1.0

Rectangle {
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
    color: primaryColors.c100.background

    Rectangle {
        id: symbolsPanel
        width: 100
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        color: primaryColors.c700.background
        property int currentIndex: 0
        property bool currentIndexIsEmoji: symbols[symbolsPanel.currentIndex].title === "Emoji"

        Column {
            width: parent.width

            Repeater {
                model: symbols
                delegate: Rectangle {
                    width: symbolsPanel.width
                    height: 40
                    color: symbolsPanel.currentIndex === index ? primaryColors.windowColor : Qt.rgba(0,0,0,0)

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        text: modelData.title
                        font.pointSize: Scrite.app.idealFontPointSize
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
        anchors.top: parent.top
        anchors.left: symbolsPanel.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 5
        clip: true
        ScrollBar.vertical: ScrollBar2 { flickable: symbolsGridView }
        rightMargin: 14

        cellWidth: symbolsPanel.currentIndexIsEmoji ? 50 : 40
        cellHeight: cellWidth
        model: symbols[symbolsPanel.currentIndex].symbols
        header: Item {
            width: symbolsGridView.width-14
            height: symbolsPanel.currentIndexIsEmoji ? 35 : 0

            Text {
                visible: symbolsPanel.currentIndexIsEmoji
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                anchors.centerIn: parent
                font.pointSize: Scrite.app.idealFontPointSize
                text: includeEmojis ? "Emojis may not be included in PDF exports." : "Emojis are not supported in this text area."
            }
        }

        delegate: Item {
            width: symbolsGridView.cellWidth
            height: symbolsGridView.cellHeight
            enabled: !symbolsPanel.currentIndexIsEmoji || includeEmojis
            opacity: enabled ? 1 : 0.5

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                border.width: 1
                border.color: primaryColors.borderColor
                opacity: 0.5
            }

            Text {
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

