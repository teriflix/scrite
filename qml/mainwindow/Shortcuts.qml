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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/tasks"
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"

Item {
    id: root

    readonly property alias notebook: _notebook
    readonly property alias toggleTagging: _toggleTagging
    readonly property alias toggleSynopsis: _toggleSynopsis
    readonly property alias toggleComments: _toggleComments
    readonly property alias toggleSpellCheck: _toggleSpellCheck
    readonly property alias toggleSceneCharacters: _toggleSceneCharacters

    readonly property url helpUrl: "https://www.scrite.io/index.php/help/"

    Shortcut {
        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Save As"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+S"

        onActivated: activate()

        function activate() { SaveFileTask.saveAs() }
    }

    Shortcut {
        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "New"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+N"

        onActivated: activate()

        function activate() { HomeScreen.launch() }
    }

    Shortcut {
        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Open"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+O"

        onActivated: activate()

        function activate() { HomeScreen.launch() }
    }

    Shortcut {
        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Scriptalay"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+O"

        onActivated: activate()

        function activate() { HomeScreen.launch("Scriptalay") }
    }

    Shortcut {
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Export To PDF"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+P"

        onActivated: activate()

        function activate() { ExportConfigurationDialog.launch("Screenplay/Adobe PDF") }
    }

    Shortcut {
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Help"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "F1"

        onActivated: activate()

        function activate() { Qt.openUrlExternally(helpUrl) }
    }

    Shortcut {
        id: _toggleSceneCharacters

        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.displaySceneCharacters ? "Hide Scene Characters, Tags" : "Show Scene Characters, Tags"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+C"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditorSettings.displaySceneCharacters = !Runtime.screenplayEditorSettings.displaySceneCharacters
        }
    }

    Shortcut {
        id: _toggleSynopsis

        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.displaySceneSynopsis ? "Hide Synopsis" : "Show Synopsis"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+S"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditorSettings.displaySceneSynopsis = !Runtime.screenplayEditorSettings.displaySceneSynopsis
        }
    }

    Shortcut {
        id: _toggleComments

        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.displaySceneComments ? "Hide Comments" : "Show Comments"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+M"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditorSettings.displaySceneComments = !Runtime.screenplayEditorSettings.displaySceneComments
        }
    }

    Shortcut {
        id: _toggleTagging

        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.allowTaggingOfScenes ? "Allow Tagging" : "Disable Tagging"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+G"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditorSettings.allowTaggingOfScenes = !Runtime.screenplayEditorSettings.allowTaggingOfScenes
        }
    }

    Shortcut {
        id: _toggleSpellCheck

        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.enableSpellCheck ? "Disable Spellcheck" : "Enable Spellcheck"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+L"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditorSettings.enableSpellCheck = !Runtime.screenplayEditorSettings.enableSpellCheck
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.applicationSettings.enableAnimations ? "Disable Animations" : "Enable Animations"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+A"

        onActivated: activate()

        function activate() {
            Runtime.applicationSettings.enableAnimations = !Runtime.applicationSettings.enableAnimations
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.highlightCurrentLine ? "Line Highlight Off" : "Line Highlight On"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+H"

        onActivated: activate()

        function activate() {
            Runtime.screenplayEditorSettings.highlightCurrentLine = !Runtime.screenplayEditorSettings.highlightCurrentLine
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "New Scrite Window"
        ShortcutsModelItem.enabled: true
        ShortcutsModelItem.visible: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+M"

        onActivated: activate()

        function activate() {
            Scrite.app.launchNewInstance(Scrite.window)
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Screenplay"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+1"

        onActivated: activate()

        function activate() {
            Runtime.activateMainWindowTab(Runtime.e_ScreenplayTab)
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Structure"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+2"

        onActivated: activate()

        function activate() {
            Runtime.activateMainWindowTab(Runtime.e_StructureTab)
            if(Runtime.showNotebookInStructure)
                Announcement.shout(Runtime.announcementIds.tabRequest, "Structure")
        }
    }

    Shortcut {
        id: _notebook

        property bool notebookTabVisible: mainTabBar.currentIndex === (Runtime.showNotebookInStructure ? 1 : 2)

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Notebook"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+3"
        onActivated: activate()

        function activate() {
            if(Runtime.showNotebookInStructure) {
                if(Runtime.mainWindowTab === Runtime.e_StructureTab)
                    Announcement.shout(Runtime.announcementIds.tabRequest, "Notebook")
                else {
                    Runtime.activateMainWindowTab(Runtime.e_StructureTab)
                    Utils.execLater(mainTabBar, 250, function() {
                        Announcement.shout(Runtime.announcementIds.tabRequest, "Notebook")
                    })
                }
            } else
                Runtime.activateMainWindowTab(Runtime.e_NotebookTab)
        }

        function showBookmarkedNotes() {
            showNotes("Notebook Bookmarks")
        }

        function showStoryNotes() {
            showNotes("Notebook Story")
        }

        function showCharacterNotes() {
            showNotes("Notebook Characters")
        }

        function showNotes(type) {
            var nbt = Runtime.showNotebookInStructure ? 1 : 2
            if(mainTabBar.currentIndex !== nbt) {
                Runtime.activateMainWindowTab(nbt)
                Utils.execLater(mainTabBar, 250, function() {
                    Announcement.shout(Runtime.announcementIds.tabRequest, type)
                })
            } else
                Announcement.shout(Runtime.announcementIds.tabRequest, type)
        }
    }

    Shortcut {
        ShortcutsModelItem.group: _notebook.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Bookmarked Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+K"

        onActivated: activate()

        function activate() {
            _notebook.showBookmarkedNotes()
        }
    }

    Shortcut {
        ShortcutsModelItem.group: _notebook.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Charater Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+R"

        onActivated: activate()

        function activate() {
            _notebook.showCharacterNotes()
        }
    }

    Shortcut {
        ShortcutsModelItem.group: _notebook.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Story Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        context: Qt.ApplicationShortcut
        enabled: Runtime.allowAppUsage
        sequence: "Ctrl+Shift+Y"

        onActivated: activate()

        function activate() {
            _notebook.showStoryNotes()
        }
    }

    Shortcut {
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Scrited"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        context: Qt.ApplicationShortcut
        enabled: Runtime.workspaceSettings.showScritedTab && Runtime.allowAppUsage
        sequence: "Alt+4"

        onActivated: activate()

        function activate() {
            Runtime.activateMainWindowTab(Runtime.e_ScritedTab)
        }
    }

    ShortcutsModelRecord {
        group: "Formatting"
        title: "Symbols & Smileys"
        enabled: Scrite.app.isTextInputItem(Scrite.window.activeFocusItem)
        priority: 10
        shortcut: "F3"
    }

    /*
      Most users already know that Ctrl+Z is undo and Ctrl+Y is redo.
      Therefore simply listing these shortcuts in shortcuts dockwidget
      should be sufficient to establish their existence. Showing these
      toolbuttons is robbing us of some really good screenspace.
    */
    ShortcutsModelRecord {
        group: "Edit"
        title: "Undo"
        enabled: Scrite.app.canUndo && !Scrite.document.readOnly // enabled
        shortcut: "Ctrl+Z" // shortcut
    }

    ShortcutsModelRecord {
        group: "Edit"
        title: "Redo"
        enabled: Scrite.app.canRedo && !Scrite.document.readOnly // enabled
        shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Shift+Z" : "Ctrl+Y" // shortcut
    }
}
