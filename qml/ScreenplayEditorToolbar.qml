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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Row {
    id: screenplayEditorToolbar
    property SceneDocumentBinder binder
    property TextArea editor
    property alias showFind: findButton.checked
    property bool showReplace: false

    signal requestScreenplayEditor()

    height: 45
    clip: true

    property var tools: [
        { "value": SceneElement.Heading, "display": "Current Scene Heading", "icon": "../icons/screenplay/heading.png" },
        { "value": SceneElement.Action, "display": "Action", "icon": "../icons/screenplay/action.png" },
        { "value": SceneElement.Character, "display": "Character", "icon": "../icons/screenplay/character.png" },
        { "value": SceneElement.Dialogue, "display": "Dialogue", "icon": "../icons/screenplay/dialogue.png" },
        { "value": SceneElement.Parenthetical, "display": "Parenthetical", "icon": "../icons/screenplay/parenthetical.png" },
        { "value": SceneElement.Shot, "display": "Shot", "icon": "../icons/screenplay/shot.png" },
        { "value": SceneElement.Transition, "display": "Transition", "icon": "../icons/screenplay/transition.png" }
    ]
    spacing: documentUI.width >= 1440 ? 2 : 0

    ToolButton3 {
        id: statsReportButton
        iconSource: "../icons/content/stats.png"
        ToolTip.text: "Generate Statistics Report"
        checkable: false
        checked: false
        onClicked: Qt.callLater(generateStatsReport)
        visible: documentUI.width >= 1400 || !appToolBar.visible

        function generateStatsReport() {
            modalDialog.closeable = false
            modalDialog.arguments = "Statistics Report"
            modalDialog.sourceComponent = reportGeneratorConfigurationComponent
            modalDialog.popupSource = statsReportButton
            modalDialog.active = true
        }
    }

    ToolButton3 {
        id: screenplayPreviewButton
        iconSource: "../icons/file/generate_pdf.png"
        ToolTip.text: "Generate PDF Output"
        onClicked: Qt.callLater(generatePdf)

        function generatePdf() {
            modalDialog.closeable = false
            modalDialog.arguments = "Screenplay/Adobe PDF"
            modalDialog.sourceComponent = exporterConfigurationComponent
            modalDialog.popupSource = screenplayPreviewButton
            modalDialog.active = true
        }
    }

    ToolButton3 {
        id: findButton
        iconSource: "../icons/action/search.png"
        shortcut: "Ctrl+F"
        ToolTip.text: "Toggles the search & replace panel in screenplay editor.\t(" + Scrite.app.polishShortcutTextForDisplay(shortcut) + ")"
        checkable: true
        checked: false
        enabled: screenplayTextDocument.editor
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
        enabled: screenplayTextDocument.editor

        ShortcutsModelItem.group: "Edit"
        ShortcutsModelItem.title: "Find & Replace"
        ShortcutsModelItem.shortcut: sequence
    }

    ToolButton3 {
        id: screenplayViewOptions
        iconSource: "../icons/content/view_options.png"
        ToolTip.text: "Screenplay View Options"
        down: screenplayViewOptionsMenu.visible
        onClicked: screenplayViewOptionsMenu.open()

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Menu2 {
                id: screenplayViewOptionsMenu
                width: 500
                title: "Screenplay Options"

                MenuItem2 {
                    text: "Show Logline Editor"
                    icon.source: screenplayEditorSettings.showLoglineEditor ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.showLoglineEditor = !screenplayEditorSettings.showLoglineEditor
                }

                MenuItem2 {
                    text: "Show Scene Blocks"
                    property bool sceneBlocksVisible: screenplayEditorSettings.spaceBetweenScenes > 0
                    icon.source: sceneBlocksVisible ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.spaceBetweenScenes = sceneBlocksVisible ? 0 : 40
                }

                MenuItem2 {
                    text: "Show Scene Synopsis\t\t" + Scrite.app.polishShortcutTextForDisplay(synopsisToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: screenplayEditorSettings.displaySceneSynopsis && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.displaySceneSynopsis = !screenplayEditorSettings.displaySceneSynopsis
                }

                MenuItem2 {
                    text: "Show Scene Comments\t\t" + Scrite.app.polishShortcutTextForDisplay(commentsToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: screenplayEditorSettings.displaySceneComments && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.displaySceneComments = !screenplayEditorSettings.displaySceneComments
                }

                MenuItem2 {
                    text: "Show Scene Characters and Tags\t" + Scrite.app.polishShortcutTextForDisplay(sceneCharactersToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: screenplayEditorSettings.displaySceneCharacters ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.displaySceneCharacters = !screenplayEditorSettings.displaySceneCharacters
                }

                MenuItem2 {
                    text: "Enable Tagging Of Scenes\t\t" +Scrite.app.polishShortcutTextForDisplay(taggingToggleShortcut.ShortcutsModelItem.shortcut)
                    icon.source: screenplayEditorSettings.allowTaggingOfScenes && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                    onTriggered: screenplayEditorSettings.allowTaggingOfScenes = !screenplayEditorSettings.allowTaggingOfScenes
                }

                MenuSeparator {  }

                MenuItem2 {
                    icon.source: "../icons/content/blank.png"
                    text: "Scan For Mute Characters"
                    onClicked: Scrite.document.structure.scanForMuteCharacters()
                    enabled: !Scrite.document.readOnly && screenplayEditorSettings.displaySceneCharacters
                }

            }
        }
    }

    ToolButton3 {
        iconSource: "../icons/navigation/refresh.png"
        shortcut: "F5"
        shortcutText: ""
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
        onActivated: screenplayTextDocument.reload()
        enabled: screenplayTextDocument.editor

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
        var idx = Scrite.document.screenplay.currentElementIndex
        if(idx < 0)
            return -1

        ++idx

        if(mainTabBar.currentIndex == 0 || mainUndoStack.screenplayEditorActive) {
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

    ToolButton3 {
        iconSource: "../icons/action/add_episode.png"
        shortcut: "Ctrl+Shift+P"
        shortcutText: ""
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

    ToolButton3 {
        iconSource: "../icons/action/add_act.png"
        shortcut: "Ctrl+Shift+B"
        shortcutText: ""
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
            Scrite.document.createNewScene(mainTabBar.currentIndex > 0 ? mainUndoStack.screenplayEditorActive : false)
    }

    ToolButton3 {
        iconSource: "../icons/action/add_scene.png"
        shortcut: "Ctrl+Shift+N"
        shortcutText: ""
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
        color: primaryColors.separatorColor
        opacity: 0.5
    }

    property bool formattable: {
        if(Scrite.document.readOnly)
            return false
        if(!binder)
            return false
        if(!binder.currentElement)
            return false
        return true
    }

    Repeater {
        model: screenplayEditorToolbar.tools

        ToolButton3 {
            iconSource: modelData.icon
            shortcut: "Ctrl+" + index
            shortcutText: (index+1)
            ToolTip.visible: containsMouse
            ToolTip.text: Scrite.app.polishShortcutTextForDisplay(modelData.display + "\t" + shortcut)
            enabled: screenplayEditorToolbar.formattable
            down: binder ? (binder.currentElement ? binder.currentElement.type === modelData.value : false) : false
            onClicked: {
                if(index === 0) {
                    if(!binder.scene.heading.enabled)
                        binder.scene.heading.enabled = true
                    Announcement.shout("2E3BBE4F-05FE-49EE-9C0E-3332825B72D8", "Scene Heading")
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
        ShortcutsModelItem.title: "Add Mute Character"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: screenplayEditorToolbar.formattable && screenplayEditorSettings.displaySceneCharacters
        ShortcutsModelItem.priority: -7

        onActivated: Announcement.shout("2E3BBE4F-05FE-49EE-9C0E-3332825B72D8", "Add Mute Character")
    }

    Shortcut {
        sequence: "Ctrl+8"

        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: "Synopsis"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: screenplayEditorToolbar.formattable && screenplayEditorSettings.displaySceneSynopsis
        ShortcutsModelItem.priority: -8

        onActivated: Announcement.shout("2E3BBE4F-05FE-49EE-9C0E-3332825B72D8", "Scene Synopsis")
    }

    Shortcut {
        sequence: "Ctrl+9"

        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: "Scene Number"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: binder && binder.currentElement && binder.currentElement.scene.heading.enabled
        ShortcutsModelItem.priority: -9

        onActivated: Announcement.shout("2E3BBE4F-05FE-49EE-9C0E-3332825B72D8", "Scene Number")
    }
}
