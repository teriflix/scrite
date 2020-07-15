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

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Rectangle {
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
            "title": "Smileys",
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
        property bool currentIndexIsSmileys: symbols[symbolsPanel.currentIndex].title === "Smileys"

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
                        font.pointSize: app.idealFontPointSize
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
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.25)
                dark: Qt.rgba(0,0,0,0.75)
            }
            opacity: active ? 1 : 0.2
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }
        }
        rightMargin: 14

        cellWidth: symbolsPanel.currentIndexIsSmileys ? 50 : 40
        cellHeight: cellWidth
        model: symbols[symbolsPanel.currentIndex].symbols
        header: Item {
            width: symbolsGridView.width-14
            height: symbolsPanel.currentIndexIsSmileys ? 50 : 0

            Text {
                visible: symbolsPanel.currentIndexIsSmileys
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                anchors.centerIn: parent
                font.pointSize: app.idealFontPointSize
                text: "Smileys may not be included in PDF exports."
            }
        }

        delegate: Item {
            width: symbolsGridView.cellWidth
            height: symbolsGridView.cellHeight

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

