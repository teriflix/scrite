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
import QtQuick.Layouts 1.13
import Scrite 1.0

ScrollView {
    id: toolbarArea
    property SceneDocumentBinder binder
    property TextArea editor
    property alias editInFullscreen: editInFullscreenButton.checked

    signal requestScreenplayEditor()

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
                    id: editInFullscreenButton
                    icon.source: checked ? "../icons/navigation/fullscreen_exit.png" : "../icons/navigation/fullscreen.png"
                    suggestedWidth: toolbar.toolButtonWidth
                    suggestedHeight: toolbar.toolButtonHeight
                    shortcut: app.isMacOSPlatform ? "F2" : "F11"
                    shortcutText: ""
                    checkable: true
                    checked: false
                    ToolTip.text: "Toggles between fullscreen and paneled edit mode.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
                }

                ToolButton2 {
                    icon.source: "../icons/content/add_box.png"
                    suggestedWidth: toolbar.toolButtonWidth
                    suggestedHeight: toolbar.toolButtonHeight
                    shortcut: "Ctrl+Shift+N"
                    shortcutText: ""
                    ToolTip.text: "Creates a new scene and adds it to both structure and screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
                    onClicked: {
                        requestScreenplayEditor()
                        scriteDocument.createNewScene()
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
                        enabled: binder ? binder.currentElement !== null : false
                        down: binder ? (binder.currentElement === null ? false : binder.currentElement.type === modelData.value) : false
                        onClicked: binder.currentElement.type = modelData.value
                        focusPolicy: Qt.NoFocus
                    }
                }
            }
        }
    }
}
