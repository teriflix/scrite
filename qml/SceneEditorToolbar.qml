/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
import QtQuick.Layouts 1.13
import Scrite 1.0

ScrollView {
    id: toolbarArea
    property SceneDocumentBinder binder
    property TextArea editor

    height: 45
    clip: true
    contentWidth: toolbarContainer.width
    contentHeight: toolbarContainer.height

    Item {
        id: toolbarContainer
        width: Math.max(toolbarArea.width, toolbar.toolButtonsWidth)
        height: toolbar.height

        Rectangle {
            id: toolbar
            anchors.centerIn: parent
            color: binder && binder.scene ? Qt.tint(binder.scene.color, "#C0FFFFFF") : "lightgray"
            property var tools: [
                { "value": SceneElement.Action, "display": "Action", "icon": "../icons/screenplay/action.png" },
                { "value": SceneElement.Character, "display": "Character", "icon": "../icons/screenplay/character.png" },
                { "value": SceneElement.Dialogue, "display": "Dialogue", "icon": "../icons/screenplay/dialogue.png" },
                { "value": SceneElement.Parenthetical, "display": "Parenthetical", "icon": "../icons/screenplay/paranthetical.png" },
                { "value": SceneElement.Shot, "display": "Shot", "icon": "../icons/screenplay/shot.png" },
                { "value": SceneElement.Transition, "display": "Transition", "icon": "../icons/screenplay/transition.png" }
            ]
            property real toolButtonWidth: 70
            property real toolButtonHeight: 45
            property real toolButtonSpacing: 5
            property real toolButtonsWidth: toolButtonWidth*tools.length + (tools.length-1)*toolButtonSpacing

            RowLayout {
                spacing: toolbar.toolButtonSpacing
                anchors.horizontalCenter: parent.horizontalCenter

                ToolButton2 {
                    icon.source: "../icons/content/language.png"
                    suggestedHeight: toolbar.toolButtonHeight
                    shortcut: "Ctrl+L"
                    shortcutText: "L"
                    text: binder ? binder.transliterationLanguageAsString : "Language"
                    ToolTip.text: app.polishShortcutTextForDisplay("Language Transliteration" + "\t" + shortcut)
                    enabled: binder ? true : false
                    onClicked: languageMenu.visible = true
                    down: languageMenu.visible

                    Item {
                        anchors.top: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter

                        Menu {
                            id: languageMenu

                            Repeater {
                                model: binder ? app.enumerationModel(binder, "TransliterationLanguage") : 0

                                MenuItem {
                                    property string baseText: binder.transliterationLanguageAsMenuItemText(modelData.value)
                                    property string shortcutKey: baseText[baseText.indexOf('&')+1].toUpperCase()
                                    text: baseText + " \t" + app.polishShortcutTextForDisplay("Ctrl+Alt+"+shortcutKey)
                                    onClicked: binder.transliterationLanguage = modelData.value
                                    checkable: true
                                    checked: binder.transliterationLanguage === modelData.value
                                }
                            }
                        }
                    }
                }

                Repeater {
                    model: toolbar.tools

                    ToolButton2 {
                        icon.source: modelData.icon
                        suggestedWidth: toolbar.toolButtonWidth
                        suggestedHeight: toolbar.toolButtonHeight
                        shortcut: "Ctrl+" + (index+1)
                        shortcutText: (index+1)
                        ToolTip.text: app.polishShortcutTextForDisplay(modelData.display + "\t" + shortcut)
                        enabled: binder && editor ? (binder.currentElement !== null && editor.activeFocus) : false
                        down: binder ? (binder.currentElement === null ? false : binder.currentElement.type === modelData.value) : false
                        onClicked: {
                            binder.currentElement.type = modelData.value
                            var pos = editor.cursorPosition
                            editor.cursorPosition = pos-1
                            editor.cursorPosition = pos
                        }
                    }
                }
            }
        }
    }
}
