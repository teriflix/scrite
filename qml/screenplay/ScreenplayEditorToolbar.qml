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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Row {
    id: root

    property SceneDocumentBinder binder
    property TextArea editor

    signal requestScreenplayEditor()

    function set(_editor, _binder) {
        editor = _editor
        binder = _binder
    }
    function reset(_editor, _binder) {
        if(editor === _editor)
            editor = null
        if(binder === _binder)
            binder = null
    }

    height: 45
    clip: true

    property var tools: [
        { "value": SceneElement.Heading, "display": "Current Scene Heading", "icon": "qrc:/icons/screenplay/heading.png" },
        { "value": SceneElement.Action, "display": "Action", "icon": "qrc:/icons/screenplay/action.png" },
        { "value": SceneElement.Character, "display": "Character", "icon": "qrc:/icons/screenplay/character.png" },
        { "value": SceneElement.Dialogue, "display": "Dialogue", "icon": "qrc:/icons/screenplay/dialogue.png" },
        { "value": SceneElement.Parenthetical, "display": "Parenthetical", "icon": "qrc:/icons/screenplay/parenthetical.png" },
        { "value": SceneElement.Shot, "display": "Shot", "icon": "qrc:/icons/screenplay/shot.png" },
        { "value": SceneElement.Transition, "display": "Transition", "icon": "qrc:/icons/screenplay/transition.png" }
    ]

    FlatToolButton {
        id: findButton
        iconSource: "qrc:/icons/action/search.png"
        ToolTip.text: "Toggles the search & replace panel in screenplay editor.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"
        down: Runtime.screenplayEditor ? Runtime.screenplayEditor.searchBarVisible : false
        enabled: Runtime.screenplayEditor
        onClicked: Runtime.screenplayEditor.searchBarVisible = !Runtime.screenplayEditor.searchBarVisible
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+F"
        onActivated: Runtime.screenplayEditor.toggleSearchBar(false)
        enabled: Runtime.screenplayEditor

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+F"
        onActivated: Runtime.screenplayEditor.toggleSearchBar(true)
        enabled: Runtime.screenplayEditor

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find & Replace"
        ShortcutsModelItem.shortcut: sequence
    }

    FlatToolButton {
        id: screenplayViewOptions
        iconSource: "qrc:/icons/content/view_options.png"
        ToolTip.text: "Screenplay Editor Options"
        down: screenplayViewOptionsMenu.visible
        onClicked: screenplayViewOptionsMenu.open()
        objectName: "screenplayViewOptionsButton"

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            VclMenu {
                id: screenplayViewOptionsMenu
                width: 500
                title: "Screenplay Options"

                VclMenuItem {
                    text: "Show Logline Editor"
                    icon.source: Runtime.screenplayEditorSettings.showLoglineEditor ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.showLoglineEditor = !Runtime.screenplayEditorSettings.showLoglineEditor
                }

                VclMenuItem {
                    text: "Show Ruler"
                    icon.source: Runtime.screenplayEditorSettings.displayRuler ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.displayRuler = !Runtime.screenplayEditorSettings.displayRuler
                }

                VclMenuItem {
                    text: "Show Empty Title Card"
                    icon.source: Runtime.screenplayEditorSettings.displayEmptyTitleCard ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.displayEmptyTitleCard = !Runtime.screenplayEditorSettings.displayEmptyTitleCard
                }

                VclMenuItem {
                    text: "Show Add Scene Controls"
                    icon.source: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons = !Runtime.screenplayEditorSettings.displayAddSceneBreakButtons
                }

                VclMenuItem {
                    text: "Show Scene Blocks"
                    property bool sceneBlocksVisible: Runtime.screenplayEditorSettings.spaceBetweenScenes > 0
                    icon.source: sceneBlocksVisible ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.spaceBetweenScenes = sceneBlocksVisible ? 0 : 40
                }

                VclMenuItem {
                    text: "Show Markup Tools"
                    property bool toolsVisible: Runtime.screenplayEditorSettings.markupToolsDockVisible
                    icon.source: toolsVisible ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.markupToolsDockVisible = !toolsVisible
                }

                VclMenuItem {
                    text: "Show Scene Synopsis\t\t" + Scrite.app.polishShortcutTextForDisplay(synopsisToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: Runtime.screenplayEditorSettings.displaySceneSynopsis && enabled ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.displaySceneSynopsis = !Runtime.screenplayEditorSettings.displaySceneSynopsis
                }

                VclMenuItem {
                    text: "Show Scene Comments\t\t" + Scrite.app.polishShortcutTextForDisplay(commentsToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: Runtime.screenplayEditorSettings.displaySceneComments && enabled ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.displaySceneComments = !Runtime.screenplayEditorSettings.displaySceneComments
                }

                VclMenuItem {
                    text: "Show Index Card Fields"
                    enabled: Runtime.screenplayEditorSettings.displaySceneComments
                    icon.source: Runtime.screenplayEditorSettings.displayIndexCardFields && enabled ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.displayIndexCardFields = !Runtime.screenplayEditorSettings.displayIndexCardFields
                }

                VclMenuItem {
                    text: "Show Scene Characters and Tags\t" + Scrite.app.polishShortcutTextForDisplay(sceneCharactersToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: Runtime.screenplayEditorSettings.displaySceneCharacters ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.displaySceneCharacters = !Runtime.screenplayEditorSettings.displaySceneCharacters
                }

                VclMenuItem {
                    text: "Enable Tagging Of Scenes\t\t" +Scrite.app.polishShortcutTextForDisplay(taggingToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: Runtime.screenplayEditorSettings.allowTaggingOfScenes && enabled ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                    onTriggered: Runtime.screenplayEditorSettings.allowTaggingOfScenes = !Runtime.screenplayEditorSettings.allowTaggingOfScenes
                }

                MenuSeparator {  }

                VclMenuItem {
                    icon.source: "qrc:/icons/content/blank.png"
                    text: "Scan For Mute Characters"
                    onClicked: Scrite.document.structure.scanForMuteCharacters()
                    enabled: !Scrite.document.readOnly && Runtime.screenplayEditorSettings.displaySceneCharacters
                }

                VclMenuItem {
                    icon.source: "qrc:/icons/content/blank.png"
                    text: "Reset Scene Numbers"
                    onClicked: Scrite.document.screenplay.resetSceneNumbers()
                    enabled: !Scrite.document.readOnly
                }
            }
        }
    }

    FlatToolButton {
        iconSource: "qrc:/icons/navigation/refresh.png"
        shortcut: "F5"
        ToolTip.text: "Reloads formatting for this scene.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: binder ? true : false
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
        onActivated: Runtime.screenplayTextDocument.reload()
        enabled: Runtime.screenplayEditor

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Redo Page Layout"
        ShortcutsModelItem.shortcut: sequence
    }

    Rectangle {
        width: 1
        height: parent.height
        color: Runtime.colors.primary.separatorColor
        opacity: 0.5
    }

    property int breakInsertIndex: {
        var idx = Scrite.document.screenplay.currentElementIndex
        if(idx < 0)
            return -1

        ++idx

        if(Runtime.mainWindowTab === Runtime.e_ScreenplayTab || Runtime.undoStack.screenplayEditorActive) {
            while(idx < Scrite.document.screenplay.elementCount) {
                var e = Scrite.document.screenplay.elementAt(idx)
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
            Scrite.document.screenplay.addBreakElement(Screenplay.Episode)
        else
            Scrite.document.screenplay.insertBreakElement(Screenplay.Episode, breakInsertIndex)
    }

    FlatToolButton {
        iconSource: "qrc:/icons/action/add_episode.png"
        shortcut: "Ctrl+Shift+P"
        ToolTip.text: "Creates an episode break after the current scene in the screenplay.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !Scrite.document.readOnly
        onClicked: addEpisode()
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Episode Break" : "Insert Episode Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: shortcut
    }

    function addAct() {
        requestScreenplayEditor()
        if(breakInsertIndex < 0)
            Scrite.document.screenplay.addBreakElement(Screenplay.Act)
        else
            Scrite.document.screenplay.insertBreakElement(Screenplay.Act, breakInsertIndex)
    }

    FlatToolButton {
        iconSource: "qrc:/icons/action/add_act.png"
        shortcut: "Ctrl+Shift+B"
        ToolTip.text: "Creates an act break after the current scene in the screenplay.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !Scrite.document.readOnly
        onClicked: addAct()
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Act Break" : "Insert Act Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: shortcut
    }

    function addScene() {
        requestScreenplayEditor()
        if(!Scrite.document.readOnly)
            Scrite.document.createNewScene(Runtime.mainWindowTab !== Runtime.e_ScreenplayTab ? Runtime.undoStack.screenplayEditorActive : false)
    }

    FlatToolButton {
        iconSource: "qrc:/icons/action/add_scene.png"
        shortcut: "Ctrl+Shift+N"
        ToolTip.text: "Creates a new scene and adds it to both structure and screenplay.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"
        enabled: !Scrite.document.readOnly
        onClicked: addScene()
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Create New Scene"
        ShortcutsModelItem.enabled: !Scrite.document.readOnly
        ShortcutsModelItem.shortcut: shortcut
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+L"
        enabled: !Scrite.document.readOnly

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: breakInsertIndex < 0 ? "Add Interval Break" : "Insert Interval Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence

        onActivated:  {
            requestScreenplayEditor()
            if(breakInsertIndex < 0)
                Scrite.document.screenplay.addBreakElement(Screenplay.Interval)
            else
                Scrite.document.screenplay.insertBreakElement(Screenplay.Interval, breakInsertIndex)
        }
    }

    Rectangle {
        width: 1
        height: parent.height
        color: Runtime.colors.primary.separatorColor
        opacity: 0.5
    }

    Repeater {
        model: root.tools

        FlatToolButton {
            iconSource: modelData.icon
            shortcut: "Ctrl+" + index
            ToolTip.visible: containsMouse
            ToolTip.text: Scrite.app.polishShortcutTextForDisplay(modelData.display + "\t" + shortcut)
            enabled: _private.formattable
            down: binder ? (binder.currentElement ? binder.currentElement.type === modelData.value : false) : false
            onClicked: {
                if(index === 0) {
                    if(!binder.scene.heading.enabled)
                        binder.scene.heading.enabled = true
                    Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneHeading)
                } else
                    binder.currentElement.type = modelData.value
            }

            ShortcutsModelItem.group: "Formatting"
            ShortcutsModelItem.title: modelData.display
            ShortcutsModelItem.shortcut: shortcut
            ShortcutsModelItem.enabled: enabled
            ShortcutsModelItem.priority: -index
        }
    }

    Shortcut {
        sequence: "Ctrl+7"

        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: Runtime.announcementData.focusOptions.addMuteCharacter
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: _private.formattable && Runtime.screenplayEditorSettings.displaySceneCharacters
        ShortcutsModelItem.priority: -7

        onActivated: Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.addMuteCharacter)
    }

    Shortcut {
        sequence: "Ctrl+8"

        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: Runtime.announcementData.focusOptions.sceneSynopsis
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: _private.formattable && Runtime.screenplayEditorSettings.displaySceneSynopsis
        ShortcutsModelItem.priority: -8

        onActivated: Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneSynopsis)
    }

    Shortcut {
        sequence: "Ctrl+9"

        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: Runtime.announcementData.focusOptions.sceneNumber
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: binder && binder.currentElement && binder.currentElement.scene.heading.enabled
        ShortcutsModelItem.priority: -9

        onActivated: Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneNumber)
    }

    QtObject {
        id: _private

        property bool formattable: Scrite.document.readOnly ? false : (binder && binder.currentElement)
    }
}
