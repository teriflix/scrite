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

Row {
    id: screenplayEditorToolbar
    property SceneDocumentBinder binder
    property TextArea editor
    property alias showScreenplayPreview: screenplayPreviewButton.checked
    property alias showFindAndReplace: findAndReplaceButton.checked

    signal requestScreenplayEditor()

    height: 45
    clip: true

    property var tools: [
        { "value": SceneElement.Action, "display": "Action", "icon": "../icons/screenplay/action.png" },
        { "value": SceneElement.Character, "display": "Character", "icon": "../icons/screenplay/character.png" },
        { "value": SceneElement.Dialogue, "display": "Dialogue", "icon": "../icons/screenplay/dialogue.png" },
        { "value": SceneElement.Parenthetical, "display": "Parenthetical", "icon": "../icons/screenplay/paranthetical.png" },
        { "value": SceneElement.Shot, "display": "Shot", "icon": "../icons/screenplay/shot.png" },
        { "value": SceneElement.Transition, "display": "Transition", "icon": "../icons/screenplay/transition.png" }
    ]
    spacing: 2

    ToolButton3 {
        id: screenplayPreviewButton
        iconSource: "../icons/action/preview.png"
        ToolTip.text: "Preview the screenplay in print format."
        checkable: true
        checked: false
    }

    ToolButton3 {
        iconSource: "../icons/screenplay/character.png"
        ToolTip.text: "Toggle display of character names under scene headings and scan for hidden characters in each scene."
        ToolTip.delay: 1000
        down: sceneCharactersMenu.visible
        onClicked: sceneCharactersMenu.visible = true
        enabled: !showScreenplayPreview

        Item {
            width: parent.width
            height: 1
            anchors.top: parent.bottom

            Menu2 {
                id: sceneCharactersMenu
                width: 300

                MenuItem2 {
                    text: "Display scene characters"
                    checkable: true
                    checked: screenplayEditorSettings.displaySceneCharacters
                    onToggled: screenplayEditorSettings.displaySceneCharacters = checked
                }

                MenuItem2 {
                    text: "Scan for mute characters"
                    onClicked: scriteDocument.structure.scanForMuteCharacters()
                }
            }
        }
    }

    ToolButton3 {
        id: findAndReplaceButton
        iconSource: "../icons/action/search.png"
        shortcut: "Ctrl+F"
        ToolTip.text: "Toggles the search & repleace panel in screenplay editor."
        checkable: true
        checked: false
        enabled: !showScreenplayPreview
    }

    Rectangle {
        width: 1
        height: parent.height
        color: primaryColors.borderColor
    }

    ToolButton3 {
        iconSource: "../icons/content/add_circle_outline.png"
        shortcut: "Ctrl+Shift+N"
        shortcutText: ""
        ToolTip.text: "Creates a new scene and adds it to both structure and screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !showScreenplayPreview
        onClicked: {
            requestScreenplayEditor()
            scriteDocument.createNewScene()
        }
    }

    ToolButton3 {
        iconSource: "../icons/navigation/refresh.png"
        shortcut: "F5"
        shortcutText: ""
        ToolTip.text: "Reloads formatting for this scene.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: binder && !showScreenplayPreview ? true : false
        onClicked: binder.refresh()
    }

    Rectangle {
        width: 1
        height: parent.height
        color: primaryColors.borderColor
    }

    Repeater {
        model: screenplayEditorToolbar.tools

        ToolButton3 {
            iconSource: modelData.icon
            shortcut: "Ctrl+" + (index+1)
            shortcutText: (index+1)
            ToolTip.text: app.polishShortcutTextForDisplay(modelData.display + "\t" + shortcut)
            enabled: binder && !showScreenplayPreview ? binder.currentElement !== null : false
            down: binder ? (binder.currentElement === null ? false : binder.currentElement.type === modelData.value) : false
            onClicked: binder.currentElement.type = modelData.value
        }
    }
}
