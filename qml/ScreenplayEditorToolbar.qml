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
    spacing: documentUI.width >= 1440 ? 2 : 0

    ToolButton3 {
        id: screenplayPreviewButton
        iconSource: "../icons/action/preview.png"
        ToolTip.text: "Preview the screenplay in print format."
        checkable: true
        checked: false
    }

    ToolButton3 {
        iconSource: "../icons/action/flag.png"
        ToolTip.text: "Toggle display of character names & synopsis under scene headings and scan for hidden characters in each scene."
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
                    icon.source: "../icons/content/blank.png"
                    text: "Scan For Mute Characters"
                    onClicked: scriteDocument.structure.scanForMuteCharacters()
                    enabled: !scriteDocument.readOnly && screenplayEditorSettings.displaySceneCharacters
                }

                MenuItem2 {
                    text: "Display Scene Characters"
                    icon.source: screenplayEditorSettings.displaySceneCharacters ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.displaySceneCharacters = !screenplayEditorSettings.displaySceneCharacters
                }

                MenuSeparator {

                }

                MenuItem2 {
                    text: "Display Scene Synopsis"
                    icon.source: screenplayEditorSettings.displaySceneSynopsis && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.displaySceneSynopsis = !screenplayEditorSettings.displaySceneSynopsis
                }

                MenuItem2 {
                    text: "Display Scene Comments"
                    icon.source: screenplayEditorSettings.displaySceneComments && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.displaySceneComments = !screenplayEditorSettings.displaySceneComments
                }
            }
        }
    }

    ToolButton3 {
        id: findAndReplaceButton
        iconSource: "../icons/action/search.png"
        shortcut: "Ctrl+F"
        ToolTip.text: "Toggles the search & replace panel in screenplay editor.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        checkable: true
        checked: false
        enabled: !showScreenplayPreview

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find"
        ShortcutsModelItem.shortcut: shortcut
    }

    Rectangle {
        width: 1
        height: parent.height
        color: primaryColors.separatorColor
        opacity: 0.5
    }

    ToolButton3 {
        iconSource: "../icons/content/add_circle_outline.png"
        shortcut: "Ctrl+Shift+N"
        shortcutText: ""
        ToolTip.text: "Creates a new scene and adds it to both structure and screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly
        onClicked: {
            requestScreenplayEditor()
            scriteDocument.createNewScene()
        }

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Create New Scene"
        ShortcutsModelItem.enabled: !scriteDocument.readOnly
        ShortcutsModelItem.shortcut: shortcut
    }

    ToolButton3 {
        iconSource: "../icons/content/add_box.png"
        shortcut: "Ctrl+Shift+B"
        shortcutText: ""
        ToolTip.text: "Creates an act break after the current scene in the screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly
        onClicked: {
            requestScreenplayEditor()
            if(scriteDocument.screenplay.currentElementIndex < 0)
                scriteDocument.screenplay.addBreakElement(Screenplay.Act)
            else
                scriteDocument.screenplay.insertBreakElement(Screenplay.Act, scriteDocument.screenplay.currentElementIndex+1)
        }

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Create New Scene"
        ShortcutsModelItem.enabled: !scriteDocument.readOnly
        ShortcutsModelItem.shortcut: shortcut
    }

    ToolButton3 {
        iconSource: "../icons/navigation/refresh.png"
        shortcut: "F5"
        shortcutText: ""
        ToolTip.text: "Reloads formatting for this scene.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: binder && !showScreenplayPreview ? true : false
        onClicked: {
            var cp = editor.cursorPosition
            binder.preserveScrollAndReload()
            if(cp >= 0)
                editor.cursorPosition = cp
        }

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Refresh"
        ShortcutsModelItem.shortcut: shortcut
    }

    Rectangle {
        width: 1
        height: parent.height
        color: primaryColors.separatorColor
        opacity: 0.5
    }

    Repeater {
        model: screenplayEditorToolbar.tools

        ToolButton3 {
            iconSource: modelData.icon
            shortcut: "Ctrl+" + (index+1)
            shortcutText: (index+1)
            ToolTip.text: app.polishShortcutTextForDisplay(modelData.display + "\t" + shortcut)
            enabled: {
                if(scriteDocument.readOnly)
                    return false
                if(showScreenplayPreview)
                    return false
                if(binder === null)
                    return false
                if(binder.currentElement === null)
                    return false
                return true
            }
            down: binder ? (binder.currentElement === null ? false : binder.currentElement.type === modelData.value) : false
            onClicked: binder.currentElement.type = modelData.value

            ShortcutsModelItem.group: "Formatting"
            ShortcutsModelItem.title: modelData.display
            ShortcutsModelItem.shortcut: shortcut
            ShortcutsModelItem.enabled: enabled
            ShortcutsModelItem.priority: -index
        }
    }
}
