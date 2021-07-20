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

import QtQml 2.13
import QtQuick 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13

import Scrite 1.0

Row {
    id: screenplayEditorToolbar
    property SceneDocumentBinder binder
    property TextArea editor
    property alias showScreenplayPreview: screenplayPreviewButton.checked
    property alias showFind: findButton.checked
    property bool showReplace: false

    signal requestScreenplayEditor()

    height: 45
    clip: true

    property var tools: [
        { "value": SceneElement.Action, "display": "Action", "icon": "../icons/screenplay/action.png" },
        { "value": SceneElement.Character, "display": "Character", "icon": "../icons/screenplay/character.png" },
        { "value": SceneElement.Dialogue, "display": "Dialogue", "icon": "../icons/screenplay/dialogue.png" },
        { "value": SceneElement.Parenthetical, "display": "Parenthetical", "icon": "../icons/screenplay/parenthetical.png" },
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
        enabled: screenplayTextDocument.editor !== null
    }

    ToolButton3 {
        id: findButton
        iconSource: "../icons/action/search.png"
        shortcut: "Ctrl+F"
        ToolTip.text: "Toggles the search & replace panel in screenplay editor.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        checkable: true
        checked: false
        enabled: !showScreenplayPreview && screenplayTextDocument.editor !== null
        onToggled: {
            if(!checked)
                showReplace = false
        }

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find"
        ShortcutsModelItem.shortcut: shortcut
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+F"
        onActivated: {
            if(showReplace)
                showReplace = false
            else {
                findButton.checked = true
                showReplace = !showReplace
            }
        }
        enabled: !showScreenplayPreview && screenplayTextDocument.editor !== null

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find & Replace"
        ShortcutsModelItem.shortcut: sequence
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

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Shift+F5"
        onActivated: screenplayTextDocument.reload()
        enabled: !showScreenplayPreview && screenplayTextDocument.editor !== null

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Redo Page Layout"
        ShortcutsModelItem.shortcut: sequence
    }

    Rectangle {
        width: 1
        height: parent.height
        color: primaryColors.separatorColor
        opacity: 0.5
    }

    property int breakInsertIndex: {
        var idx = scriteDocument.screenplay.currentElementIndex
        if(idx < 0)
            return -1

        ++idx

        if(mainTabBar.currentIndex == 0 || mainUndoStack.screenplayEditorActive) {
            while(idx < scriteDocument.screenplay.elementCount) {
                var e = scriteDocument.screenplay.elementAt(idx)
                if(e === null)
                    break
                if(e.elementType === ScreenplayElement.BreakElementType)
                    ++idx
                else
                    break
            }
        }

        return idx
    }

    function addEpisode() {
        requestScreenplayEditor()
        if(breakInsertIndex < 0)
            scriteDocument.screenplay.addBreakElement(Screenplay.Episode)
        else
            scriteDocument.screenplay.insertBreakElement(Screenplay.Episode, breakInsertIndex)
    }

    ToolButton3 {
        iconSource: "../icons/action/add_episode.png"
        shortcut: "Ctrl+Shift+P"
        shortcutText: ""
        ToolTip.text: "Creates an episode break after the current scene in the screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly
        onClicked: addEpisode()
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Episode Break" : "Insert Episode Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: shortcut
    }

    function addAct() {
        requestScreenplayEditor()
        if(breakInsertIndex < 0)
            scriteDocument.screenplay.addBreakElement(Screenplay.Act)
        else
            scriteDocument.screenplay.insertBreakElement(Screenplay.Act, breakInsertIndex)
    }

    ToolButton3 {
        iconSource: "../icons/action/add_act.png"
        shortcut: "Ctrl+Shift+B"
        shortcutText: ""
        ToolTip.text: "Creates an act break after the current scene in the screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly
        onClicked: addAct()
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Act Break" : "Insert Act Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: shortcut
    }

    function addScene() {
        requestScreenplayEditor()
        if(!scriteDocument.readOnly)
            scriteDocument.createNewScene(mainTabBar.currentIndex > 0 ? mainUndoStack.screenplayEditorActive : false)
    }

    ToolButton3 {
        iconSource: "../icons/action/add_scene.png"
        shortcut: "Ctrl+Shift+N"
        shortcutText: ""
        ToolTip.text: "Creates a new scene and adds it to both structure and screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly
        onClicked: addScene()
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Create New Scene"
        ShortcutsModelItem.enabled: !scriteDocument.readOnly
        ShortcutsModelItem.shortcut: shortcut
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+L"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Interval Break" : "Insert Interval Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence

        onActivated:  {
            requestScreenplayEditor()
            if(breakInsertIndex < 0)
                scriteDocument.screenplay.addBreakElement(Screenplay.Interval)
            else
                scriteDocument.screenplay.insertBreakElement(Screenplay.Interval, breakInsertIndex)
        }
    }

    Rectangle {
        width: 1
        height: parent.height
        color: primaryColors.separatorColor
        opacity: 0.5
    }

    QtObject {
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Current Scene Heading"
        ShortcutsModelItem.shortcut: "Ctrl+0"
        ShortcutsModelItem.enabled: enabled
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
