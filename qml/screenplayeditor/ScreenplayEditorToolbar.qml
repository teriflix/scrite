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
import "qrc:/qml/floatingdockpanels"

Row {
    id: root

    property TextEdit sceneTextEditor
    property SceneDocumentBinder sceneDocumentBinder

    signal requestScreenplayEditor()

    function set(_editor, _binder) {
        sceneTextEditor = _editor
        sceneDocumentBinder = _binder
    }

    function reset(_editor, _binder) {
        if(sceneTextEditor === _editor)
            sceneTextEditor = null
        if(sceneDocumentBinder === _binder) {
            sceneDocumentBinder = null
        }
    }

    height: 45

    clip: true

    FlatToolButton {
        id: _findButton

        ToolTip.text: "Toggles the search & replace panel in screenplay editor.\t(" + Scrite.app.polishShortcutTextForDisplay(_findShortcut.sequence) + ")"

        down: Runtime.screenplayEditor ? Runtime.screenplayEditor.searchBarVisible : false
        enabled: Runtime.screenplayEditor
        iconSource: "qrc:/icons/action/search.png"

        onClicked: Runtime.screenplayEditor.searchBarVisible = !Runtime.screenplayEditor.searchBarVisible
    }

    Shortcut {
        id: _findShortcut

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        context: Qt.ApplicationShortcut
        enabled: Runtime.screenplayEditor && Runtime.allowAppUsage
        sequence: "Ctrl+F"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditor.toggleSearchBar(false)
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find & Replace"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        context: Qt.ApplicationShortcut
        enabled: Runtime.screenplayEditor && Runtime.allowAppUsage
        sequence: "Ctrl+Shift+F"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditor.toggleSearchBar(true)
        }
    }

    FlatToolButton {
        id: _screenplayViewOptions

        ToolTip.text: "Screenplay Editor Options"

        down: _screenplayViewOptionsMenu.visible
        objectName: "screenplayViewOptionsButton"
        iconSource: "qrc:/icons/content/view_options.png"

        onClicked: _screenplayViewOptionsMenu.open()

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            VclMenu {
                id: _screenplayViewOptionsMenu

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
                    property bool __sceneBlocksVisible: Runtime.screenplayEditorSettings.spaceBetweenScenes > 0

                    text: "Show Scene Blocks"
                    icon.source: __sceneBlocksVisible ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

                    onTriggered: Runtime.screenplayEditorSettings.spaceBetweenScenes = __sceneBlocksVisible ? 0 : 40
                }

                VclMenuItem {
                    property bool __toolsVisible: Runtime.screenplayEditorSettings.markupToolsDockVisible

                    text: "Show Markup Tools"
                    icon.source: __toolsVisible ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

                    onTriggered: Runtime.screenplayEditorSettings.markupToolsDockVisible = !__toolsVisible
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
                    text: "Scan For Mute Characters"
                    enabled: !Scrite.document.readOnly && Runtime.screenplayEditorSettings.displaySceneCharacters
                    icon.source: "qrc:/icons/content/blank.png"

                    onClicked: Scrite.document.structure.scanForMuteCharacters()
                }

                VclMenuItem {
                    text: "Reset Scene Numbers"
                    enabled: !Scrite.document.readOnly
                    icon.source: "qrc:/icons/content/blank.png"

                    onClicked: Scrite.document.screenplay.resetSceneNumbers()
                }
            }
        }
    }

    FlatToolButton {
        ToolTip.text: "Reloads formatting for this scene.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Refresh"
        ShortcutsModelItem.shortcut: shortcut
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: sceneDocumentBinder ? true : false
        shortcut: "F5"
        iconSource: "qrc:/icons/navigation/refresh.png"

        onClicked: activate()

        function activate() {
            var cp = sceneTextEditor.cursorPosition
            sceneDocumentBinder.preserveScrollAndReload()
            if(cp >= 0)
                sceneTextEditor.cursorPosition = cp
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Redo Page Layout"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        context: Qt.ApplicationShortcut
        enabled: Runtime.screenplayEditor && Runtime.allowAppUsage
        sequence: "Shift+F5"

        onActivated: activate()

        function activate() {
            Runtime.screenplayTextDocument.reload()
        }
    }

    Rectangle {
        width: 1
        height: parent.height

        color: Runtime.colors.primary.separatorColor
        opacity: 0.5
    }

    FlatToolButton {
        ToolTip.text: "Creates an episode break after the current scene in the screenplay.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: _private.breakInsertIndex < 0 ? "Add Episode Break" : "Insert Episode Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: shortcut
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: !Scrite.document.readOnly
        shortcut: "Ctrl+Shift+P"
        iconSource: "qrc:/icons/action/add_episode.png"

        onClicked: activate()

        function activate() {
            _private.addEpisode()
        }
    }

    FlatToolButton {
        ToolTip.text: "Creates an act break after the current scene in the screenplay.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: _private.breakInsertIndex < 0 ? "Add Act Break" : "Insert Act Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: shortcut
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: !Scrite.document.readOnly
        shortcut: "Ctrl+Shift+B"
        iconSource: "qrc:/icons/action/add_act.png"

        onClicked: activate()

        function activate() {
            _private.addAct()
        }
    }

    FlatToolButton {
        ToolTip.text: "Creates a new scene and adds it to both structure and screenplay.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Create New Scene"
        ShortcutsModelItem.enabled: !Scrite.document.readOnly
        ShortcutsModelItem.shortcut: shortcut
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: !Scrite.document.readOnly
        shortcut: "Ctrl+Shift+N"
        iconSource: "qrc:/icons/action/add_scene.png"

        onClicked: activate()

        function activate() { _private.addScene() }
    }

    Shortcut {
        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: _private.breakInsertIndex < 0 ? "Add Interval Break" : "Insert Interval Break"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        context: Qt.ApplicationShortcut
        enabled: !Scrite.document.readOnly && Runtime.allowAppUsage
        sequence: "Ctrl+Shift+L"

        onActivated: activate()

        function activate() {
            requestScreenplayEditor()
            if(_private.breakInsertIndex < 0)
                Scrite.document.screenplay.addBreakElement(Screenplay.Interval)
            else
                Scrite.document.screenplay.insertBreakElement(Screenplay.Interval, _private.breakInsertIndex)
        }
    }

    Rectangle {
        width: 1
        height: parent.height

        color: Runtime.colors.primary.separatorColor
        opacity: 0.5
    }

    Repeater {
        model: _private.tools

        FlatToolButton {
            required property int index
            required property var modelData // { value: int, display: string, icon: string }

            ToolTip.text: Scrite.app.polishShortcutTextForDisplay(modelData.display + "\t" + shortcut)
            ToolTip.visible: containsMouse

            ShortcutsModelItem.group: "Formatting"
            ShortcutsModelItem.title: modelData.display
            ShortcutsModelItem.enabled: enabled
            ShortcutsModelItem.priority: -index
            ShortcutsModelItem.shortcut: shortcut
            ShortcutsModelItem.canActivate: true
            ShortcutsModelItem.onActivated: activate()

            down: sceneDocumentBinder ? (sceneDocumentBinder.currentElement ? sceneDocumentBinder.currentElement.type === modelData.value : false) : false
            enabled: _private.formattable
            shortcut: "Ctrl+" + index
            iconSource: modelData.icon

            onClicked: activate()

            function activate() {
                if(index === 0) {
                    if(!sceneDocumentBinder.scene.heading.enabled)
                        sceneDocumentBinder.scene.heading.enabled = true
                    Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneHeading)
                } else
                    sceneDocumentBinder.currentElement.type = modelData.value
            }
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: Runtime.announcementData.focusOptions.addMuteCharacter
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: _private.formattable && Runtime.screenplayEditorSettings.displaySceneCharacters
        ShortcutsModelItem.priority: -7
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        sequence: "Ctrl+7"
        enabled: Runtime.allowAppUsage

        onActivated: activate()

        function activate() {
            Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.addMuteCharacter)
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: Runtime.announcementData.focusOptions.sceneSynopsis
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: _private.formattable && Runtime.screenplayEditorSettings.displaySceneSynopsis
        ShortcutsModelItem.priority: -8
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        sequence: "Ctrl+8"
        enabled: Runtime.allowAppUsage

        onActivated: activate()

        function activate() {
            Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneSynopsis)
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: Runtime.announcementData.focusOptions.sceneNumber
        ShortcutsModelItem.enabled: sceneDocumentBinder && sceneDocumentBinder.currentElement && sceneDocumentBinder.currentElement.scene.heading.enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.priority: -9
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        sequence: "Ctrl+9"
        enabled: Runtime.allowAppUsage

        onActivated: activate()

        function activate() {
            Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneNumber)
        }
    }

    QtObject {
        id: _private

        property bool formattable: Scrite.document.readOnly ? false : (sceneDocumentBinder && sceneDocumentBinder.currentElement)

        property int breakInsertIndex: {
            var idx = Scrite.document.screenplay.currentElementIndex
            if(idx < 0)
                return -1

            ++idx

            if(Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab || Runtime.undoStack.screenplayEditorActive) {
                while(idx < Scrite.document.screenplay.elementCount) {
                    const e = Scrite.document.screenplay.elementAt(idx)
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

        readonly property var tools: [
            { "value": SceneElement.Heading, "display": "Current Scene Heading", "icon": "qrc:/icons/screenplay/heading.png" },
            { "value": SceneElement.Action, "display": "Action", "icon": "qrc:/icons/screenplay/action.png" },
            { "value": SceneElement.Character, "display": "Character", "icon": "qrc:/icons/screenplay/character.png" },
            { "value": SceneElement.Dialogue, "display": "Dialogue", "icon": "qrc:/icons/screenplay/dialogue.png" },
            { "value": SceneElement.Parenthetical, "display": "Parenthetical", "icon": "qrc:/icons/screenplay/parenthetical.png" },
            { "value": SceneElement.Shot, "display": "Shot", "icon": "qrc:/icons/screenplay/shot.png" },
            { "value": SceneElement.Transition, "display": "Transition", "icon": "qrc:/icons/screenplay/transition.png" }
        ]

        function addAct() {
            if(Scrite.document.readOnly)
                return

            root.requestScreenplayEditor()

            if(breakInsertIndex < 0)
                Scrite.document.screenplay.addBreakElement(Screenplay.Act)
            else
                Scrite.document.screenplay.insertBreakElement(Screenplay.Act, _private.breakInsertIndex)
        }

        function addScene() {
            if(Scrite.document.readOnly)
                return

            root.requestScreenplayEditor()

            Scrite.document.createNewScene(Runtime.mainWindowTab !== Runtime.MainWindowTab.ScreenplayTab ? Runtime.undoStack.screenplayEditorActive : false)
        }

        function addEpisode() {
            if(Scrite.document.readOnly)
                return

            root.requestScreenplayEditor()

            if(_private.breakInsertIndex < 0)
                Scrite.document.screenplay.addBreakElement(Screenplay.Episode)
            else
                Scrite.document.screenplay.insertBreakElement(Screenplay.Episode, _private.breakInsertIndex)
        }
    }
}
