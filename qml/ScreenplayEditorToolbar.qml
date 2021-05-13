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
        enabled: screenplayTextDocument.editor !== null
    }

    ToolButton3 {
        id: findAndReplaceButton
        iconSource: "../icons/action/search.png"
        shortcut: "Ctrl+F"
        ToolTip.text: "Toggles the search & replace panel in screenplay editor.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        checkable: true
        checked: false
        enabled: !showScreenplayPreview && screenplayTextDocument.editor !== null

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

    property int breakInsertIndex: {
        var idx = scriteDocument.screenplay.currentElementIndex
        if(idx < 0)
            return -1

        ++idx
        while(idx < scriteDocument.screenplay.elementCount) {
            var e = scriteDocument.screenplay.elementAt(idx)
            if(e === null)
                break
            if(e.elementType === ScreenplayElement.BreakElementType)
                ++idx
            else
                break
        }

        return idx
    }

    ToolButton3 {
        iconSource: "../icons/content/add_box.png"
        shortcut: "Ctrl+Shift+B"
        shortcutText: ""
        ToolTip.text: "Creates an act break after the current scene in the screenplay.\t(" + app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly
        onClicked: {
            requestScreenplayEditor()
            if(breakInsertIndex < 0)
                scriteDocument.screenplay.addBreakElement(Screenplay.Act)
            else
                scriteDocument.screenplay.insertBreakElement(Screenplay.Act, breakInsertIndex)
        }

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Act Break" : "Insert Act Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: shortcut
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+P"
        enabled: !showScreenplayPreview && !scriteDocument.readOnly

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Episode Break" : "Insert Episode Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence

        onActivated:  {
            requestScreenplayEditor()
            if(breakInsertIndex < 0)
                scriteDocument.screenplay.addBreakElement(Screenplay.Episode)
            else
                scriteDocument.screenplay.insertBreakElement(Screenplay.Episode, breakInsertIndex)
        }
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
