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
import QtQuick.Window 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/tasks"
import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/scrited"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/notifications"
import "qrc:/qml/floatingdockpanels"

Item {
    id: scriteMainWindow
    width: 1350
    height: 700

    readonly property url helpUrl: "https://www.scrite.io/index.php/help/"

    enabled: !Scrite.document.loading

    Connections {
        target: Runtime

        function onShowNotebookInStructureChanged() {
            Utils.execLater(mainTabBar, 100, function() {
                mainTabBar.currentIndex = mainTabBar.currentIndex % (Runtime.showNotebookInStructure ? 2 : 3)
            })
        }
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+S"
        enabled: Runtime.allowAppUsage
        onActivated: SaveFileTask.saveAs()

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Save As"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+N"
        onActivated: HomeScreen.launch()

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "New"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+O"
        onActivated: HomeScreen.launch()

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Open"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+O"
        onActivated: HomeScreen.launch("Scriptalay")

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Scriptalay"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+P"

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Export To PDF"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ExportConfigurationDialog.launch("Screenplay/Adobe PDF")
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "F1"

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Help"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Qt.openUrlExternally(helpUrl)
    }

    Shortcut {
        id: sceneCharactersToggleShortcut
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+C"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.displaySceneCharacters ? "Hide Scene Characters, Tags" : "Show Scene Characters, Tags"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Runtime.screenplayEditorSettings.displaySceneCharacters = !Runtime.screenplayEditorSettings.displaySceneCharacters
    }

    Shortcut {
        id: synopsisToggleShortcut
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+S"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.displaySceneSynopsis ? "Hide Synopsis" : "Show Synopsis"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Runtime.screenplayEditorSettings.displaySceneSynopsis = !Runtime.screenplayEditorSettings.displaySceneSynopsis
    }

    Shortcut {
        id: commentsToggleShortcut
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+M"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.displaySceneComments ? "Hide Comments" : "Show Comments"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Runtime.screenplayEditorSettings.displaySceneComments = !Runtime.screenplayEditorSettings.displaySceneComments
    }

    Shortcut {
        id: taggingToggleShortcut
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+G"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.allowTaggingOfScenes ? "Allow Tagging" : "Disable Tagging"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Runtime.screenplayEditorSettings.allowTaggingOfScenes = !Runtime.screenplayEditorSettings.allowTaggingOfScenes
    }

    Shortcut {
        id: spellCheckToggleShortcut
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+L"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.enableSpellCheck ? "Disable Spellcheck" : "Enable Spellcheck"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Runtime.screenplayEditorSettings.enableSpellCheck = !Runtime.screenplayEditorSettings.enableSpellCheck
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+A"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.applicationSettings.enableAnimations ? "Disable Animations" : "Enable Animations"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Runtime.applicationSettings.enableAnimations = !Runtime.applicationSettings.enableAnimations
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+H"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: Runtime.screenplayEditorSettings.highlightCurrentLine ? "Line Highlight Off" : "Line Highlight On"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Runtime.screenplayEditorSettings.highlightCurrentLine = !Runtime.screenplayEditorSettings.highlightCurrentLine
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+M"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "New Scrite Window"
        ShortcutsModelItem.enabled: true
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.visible: enabled
        onActivated: Scrite.app.launchNewInstance(Scrite.window)
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+1"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Screenplay"
        ShortcutsModelItem.shortcut: sequence
        onActivated: mainTabBar.activateTab(0)
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+2"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Structure"
        ShortcutsModelItem.shortcut: sequence
        onActivated: {
            mainTabBar.activateTab(1)
            if(Runtime.showNotebookInStructure)
                Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Structure")
        }
    }

    Shortcut {
        id: notebookShortcut
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+3"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Notebook"
        ShortcutsModelItem.shortcut: sequence
        onActivated: {
            if(Runtime.showNotebookInStructure) {
                if(mainTabBar.currentIndex === 1)
                    Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Notebook")
                else {
                    mainTabBar.activateTab(1)
                    Utils.execLater(mainTabBar, 250, function() {
                        Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Notebook")
                    })
                }
            } else
                mainTabBar.activateTab(2)
        }

        property bool notebookTabVisible: mainTabBar.currentIndex === (Runtime.showNotebookInStructure ? 1 : 2)

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
                mainTabBar.activateTab(nbt)
                Utils.execLater(mainTabBar, 250, function() {
                    Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", type)
                })
            } else
                Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", type)
        }
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+K"
        onActivated: notebookShortcut.showBookmarkedNotes()

        ShortcutsModelItem.group: notebookShortcut.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Bookmarked Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+R"
        onActivated: notebookShortcut.showCharacterNotes()

        ShortcutsModelItem.group: notebookShortcut.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Charater Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        enabled: Runtime.allowAppUsage
        sequence: "Ctrl+Shift+Y"
        onActivated: notebookShortcut.showStoryNotes()

        ShortcutsModelItem.group: notebookShortcut.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Story Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Alt+4"
        enabled: Runtime.workspaceSettings.showScritedTab && Runtime.allowAppUsage
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Scrited"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: enabled
        onActivated: mainTabBar.activateTab(3)
    }

    Connections {
        target: Scrite.document

        function onJustReset() {
            Runtime.screenplayEditorSettings.firstSwitchToStructureTab = true
            appBusyOverlay.ref()
            // Runtime.screenplayAdapter.initialLoadTreshold = 25
            reloadScriteDocumentTimer.stop()
            Utils.execLater(Runtime.screenplayAdapter, 250, () => {
                                appBusyOverlay.deref()
                                Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
                            })
        }

        function onJustLoaded() {
            Runtime.screenplayEditorSettings.firstSwitchToStructureTab = true
            // var firstElement = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.firstSceneIndex())
            // if(firstElement) {
            //     var editorHints = firstElement.editorHints
            //     if(editorHints)
            //         Runtime.screenplayAdapter.initialLoadTreshold = -1
            // }
        }

        function onOpenedAnonymously(filePath) {
            MessageBox.question("Anonymous Open",
                   "The file you just opened is a backup of another file, and is being opened anonymously in <b>read-only</b> mode.<br/><br/>" +
                   "<b>NOTE:</b> In order to edit the file, you will need to first Save-As.",
                    ["Save As", "View Read Only"],
                    (answer) => {
                        if(answer === "Save As")
                            SaveFileTask.saveAs()
                    })
        }

        function onRequiresReload() {
            if(Runtime.applicationSettings.reloadPrompt)
                reloadScriteDocumentTimer.start()
        }
    }

    VclDialog {
        id: reloadPromptDialog

        width: Math.min(500, Scrite.window.width * 0.5)
        height: 275
        title: "Reload Required"

        titleBarButtons: null

        content: Item {
            property real preferredHeight: reloadPromptDialogLayout.height + 40

            ColumnLayout {
                id: reloadPromptDialogLayout
                anchors.fill: parent
                anchors.margins: 20

                spacing: 10

                VclLabel {
                    Layout.fillWidth: true

                    text: "Current file was modified by another process in the background. Do you want to reload?"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 20

                    VclButton {
                        text: "Yes"
                        onClicked: {
                            Scrite.document.reload()
                            reloadPromptDialog.close()
                        }
                    }

                    VclButton {
                        text: "No"
                        onClicked: reloadPromptDialog.close()
                    }
                }

                ColumnLayout {
                    spacing: 2

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: Runtime.colors.primary.borderColor
                    }

                    RowLayout {
                        VclCheckBox {
                            Layout.fillWidth: true

                            text: "Don't show this again."
                            checked: false
                            onToggled: Runtime.applicationSettings.reloadPrompt = !checked
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                        }

                        Link {
                            text: "More Info"
                            onClicked: Qt.openUrlExternally("https://www.scrite.io/version-1-2-released/#chapter6_reload_prompt")
                        }
                    }
                }
            }

            property bool autoSaveFlag: false
            Component.onCompleted: {
                autoSaveFlag = Scrite.document.autoSave
                Scrite.document.autoSave = false
            }
            Component.onDestruction: Scrite.document.autoSave = autoSaveFlag
        }
    }

    Timer {
        id: reloadScriteDocumentTimer

        interval: 500
        repeat: false

        onTriggered: {
            if(Runtime.applicationSettings.reloadPrompt)
                reloadPromptDialog.open()
        }
    }

    // Refactor QML TODO: Get rid of this stuff when we move to overlays and ApplicationMainWindow
    QtObject {
        property bool overlayRefCountModified: false
        property bool requiresAppBusyOverlay: Runtime.undoStack.screenplayEditorActive || Runtime.undoStack.sceneEditorActive

        function onUpdateScheduled() {
            if(requiresAppBusyOverlay && !overlayRefCountModified) {
                appBusyOverlay.ref()
                overlayRefCountModified = true
            }
        }

        function onUpdateFinished() {
            if(overlayRefCountModified)
                appBusyOverlay.deref()
            overlayRefCountModified = false
        }

        onRequiresAppBusyOverlayChanged: {
            if(!requiresAppBusyOverlay && overlayRefCountModified) {
                appBusyOverlay.deref()
                overlayRefCountModified = false
            }
        }

        Component.onCompleted: {
            // Cannot use Connections for this, because the Connections QML item
            // does not allow usage of custom properties
            Runtime.screenplayTextDocument.onUpdateScheduled.connect(onUpdateScheduled)
            Runtime.screenplayTextDocument.onUpdateFinished.connect(onUpdateFinished)
        }
    }

    Rectangle {
        id: appToolBarArea
        anchors.left: parent.left
        anchors.right: parent.right

        z: 1
        height: 53
        color: Runtime.colors.primary.c50.background
        enabled: visible

        Row {
            id: appToolBar
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 5
            visible: appToolBarArea.width >= 1200
            onVisibleChanged: {
                if(enabled && !visible)
                    mainTabBar.activateTab(0)
            }

            FlatToolButton {
                id: homeButton
                iconSource: "qrc:/icons/action/home.png"
                text: "Home"
                onClicked: HomeScreen.launch()
            }

            FlatToolButton {
                id: backupOpenButton
                iconSource: "qrc:/icons/file/backup_open.png"
                text: "Open Backup"
                visible: Scrite.document.backupFilesModel.count > 0
                onClicked: BackupsDialog.launch()

                ToolTip.text: "Open any of the " + Scrite.document.backupFilesModel.count + " backup(s) available for this file."

                VclText {
                    id: backupCountHint
                    font.pixelSize: parent.height * 0.2
                    font.bold: true
                    text: Scrite.document.backupFilesModel.count
                    padding: 2
                    color: Runtime.colors.primary.highlight.text
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                }
            }

            FlatToolButton {
                id: cmdSave
                iconSource: "qrc:/icons/content/save.png"
                text: "Save"
                shortcut: "Ctrl+S"
                enabled: (Scrite.document.modified || Scrite.document.fileName === "") && !Scrite.document.readOnly
                onClicked: {
                    if(Scrite.document.fileName === "")
                        SaveFileTask.saveAs()
                    else
                        SaveFileTask.saveSilently()
                }

                ShortcutsModelItem.group: "File"
                ShortcutsModelItem.title: text
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: shortcut
            }

            FlatToolButton {
                id: cmdShare
                text: "Share"
                iconSource: "qrc:/icons/action/share.png"
                enabled: appToolsMenu.visible === false
                onClicked: shareMenu.open()
                down: shareMenu.visible

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    VclMenu {
                        id: shareMenu

                        VclMenu {
                            id: exportMenu
                            width: 300
                            title: "Export"

                            Repeater {
                                model: Scrite.document.supportedExportFormats

                                VclMenuItem {
                                    required property var modelData
                                    text: modelData.name
                                    icon.source: "qrc" + modelData.icon
                                    onClicked: ExportConfigurationDialog.launch(modelData.key)

                                    ToolTip {
                                        text: modelData.description + "\n\nCategory: " + modelData.category
                                        width: 300
                                        visible: parent.hovered
                                        delay: Qt.styleHints.mousePressAndHoldInterval
                                    }
                                }
                            }

                            MenuSeparator { }

                            VclMenuItem {
                                text: "Scrite"
                                icon.source: "qrc:/icons/exporter/scrite.png"
                                onClicked: SaveFileTask.saveAs()
                            }
                        }

                        VclMenu {
                            id: reportsMenu
                            width: 350
                            title: "Reports"

                            Repeater {
                                model: Scrite.document.supportedReports

                                VclMenuItem {
                                    required property var modelData
                                    text: modelData.name
                                    icon.source: "qrc" + modelData.icon
                                    onClicked: ReportConfigurationDialog.launch(modelData.name)

                                    ToolTip {
                                        text: modelData.description
                                        width: 300
                                        visible: parent.hovered
                                        delay: Qt.styleHints.mousePressAndHoldInterval
                                    }
                                }
                            }
                        }
                    }
                }
            }

            /*
              Most users already know that Ctrl+Z is undo and Ctrl+Y is redo.
              Therefore simply listing these shortcuts in shortcuts dockwidget
              should be sufficient to establish their existence. Showing these
              toolbuttons is robbing us of some really good screenspace.
            */
            QtObject {
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Undo"
                ShortcutsModelItem.enabled: Scrite.app.canUndo && !Scrite.document.readOnly // enabled
                ShortcutsModelItem.shortcut: "Ctrl+Z" // shortcut
            }

            QtObject {
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Redo"
                ShortcutsModelItem.enabled: Scrite.app.canRedo && !Scrite.document.readOnly // enabled
                ShortcutsModelItem.shortcut: Scrite.app.isMacOSPlatform ? "Ctrl+Shift+Z" : "Ctrl+Y" // shortcut
            }

            Rectangle {
                width: 1
                height: parent.height
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            FlatToolButton {
                id: settingsAndShortcutsButton
                iconSource: "qrc:/icons/action/settings_applications.png"
                text: "Settings & Shortcuts"
                down: settingsAndShortcutsMenu.visible
                onClicked: settingsAndShortcutsMenu.visible = true

                Item {
                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    VclMenu {
                        id: settingsAndShortcutsMenu
                        width: 300

                        VclMenuItem {
                            text: "Settings\t\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+,")
                            icon.source: "qrc:/icons/action/settings_applications.png"
                            onClicked: SettingsDialog.launch()
                            enabled: appToolBar.visible

                            ShortcutsModelItem.group: "Application"
                            ShortcutsModelItem.title: "Settings"
                            ShortcutsModelItem.shortcut: "Ctrl+,"
                            ShortcutsModelItem.enabled: appToolBar.visible

                            Shortcut {
                                enabled: Runtime.allowAppUsage
                                context: Qt.ApplicationShortcut
                                sequence: "Ctrl+,"
                                onActivated: SettingsDialog.launch()
                            }
                        }

                        VclMenuItem {
                            id: shortcutsMenuItem
                            text: "Shortcuts\t\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+E")
                            icon.source: {
                                if(Scrite.app.isMacOSPlatform)
                                    return "qrc:/icons/navigation/shortcuts_macos.png"
                                if(Scrite.app.isWindowsPlatform)
                                    return "qrc:/icons/navigation/shortcuts_windows.png"
                                return "qrc:/icons/navigation/shortcuts_linux.png"
                            }
                            onClicked: Runtime.shortcutsDockWidgetSettings.visible = !Runtime.shortcutsDockWidgetSettings.visible
                            enabled: appToolBar.visible

                            ShortcutsModelItem.group: "Application"
                            ShortcutsModelItem.title: FloatingShortcutsDock.visible ? "Hide Shortcuts" : "Show Shortcuts"
                            ShortcutsModelItem.shortcut: "Ctrl+E"
                            ShortcutsModelItem.enabled: appToolBar.visible

                            Shortcut {
                                enabled: Runtime.allowAppUsage
                                context: Qt.ApplicationShortcut
                                sequence: "Ctrl+E"
                                onActivated: Runtime.shortcutsDockWidgetSettings.visible = !Runtime.shortcutsDockWidgetSettings.visible
                            }
                        }

                        VclMenuItem {
                            icon.source: "qrc:/icons/action/help.png"
                            text: "Help\t\tF1"
                            onClicked: Qt.openUrlExternally(helpUrl)
                        }

                        VclMenuItem {
                            icon.source: "qrc:/icons/action/info.png"
                            text: "About"
                            onClicked: AboutDialog.launch()
                        }

                        VclMenuItem {
                            text: "Toggle Fullscreen\tF7"
                            icon.source: "qrc:/icons/navigation/fullscreen.png"
                            onClicked: Utils.execLater(Scrite.app, 100, function() { Scrite.app.toggleFullscreen(Scrite.window) })
                            ShortcutsModelItem.group: "Application"
                            ShortcutsModelItem.title: "Toggle Fullscreen"
                            ShortcutsModelItem.shortcut: "F7"
                            Shortcut {
                                enabled: Runtime.allowAppUsage
                                context: Qt.ApplicationShortcut
                                sequence: "F7"
                                onActivated: Utils.execLater(Scrite.app, 100, function() { Scrite.app.toggleFullscreen(Scrite.window) })
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: 1
                height: parent.height
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            FlatToolButton {
                id: languageToolButton
                iconSource: "qrc:/icons/content/language.png"
                text: Scrite.app.transliterationEngine.languageAsString
                shortcut: "Ctrl+L"
                ToolTip.text: Scrite.app.polishShortcutTextForDisplay("Language Transliteration" + "\t" + shortcut)
                onClicked: languageMenu.visible = true
                down: languageMenu.visible
                visible: mainTabBar.currentIndex <= 2

                Item {
                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    VclMenu {
                        id: languageMenu
                        width: 250

                        Repeater {
                            model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                            VclMenuItem {
                                property string baseText: modelData.key
                                property string shortcutKey: Scrite.app.transliterationEngine.shortcutLetter(modelData.value)
                                property string tabs: /*Scrite.app.isWindowsPlatform ? (modelData.value === TransliterationEngine.Malayalam ? "\t" : "\t\t") : */"\t\t"
                                text: baseText + tabs + Scrite.app.polishShortcutTextForDisplay("Alt+"+shortcutKey)
                                font.bold: Scrite.app.transliterationEngine.language === modelData.value
                                focusPolicy: Qt.NoFocus
                                enabled: Scrite.app.transliterationEngine.enabledLanguages.indexOf(modelData.value) >= 0
                                onClicked: {
                                    Scrite.app.transliterationEngine.language = modelData.value
                                    Scrite.document.formatting.defaultLanguage = modelData.value
                                    Runtime.paragraphLanguageSettings.defaultLanguage = modelData.key
                                }
                            }
                        }

                        MenuSeparator {
                            focusPolicy: Qt.NoFocus
                        }

                        VclMenuItem {
                            text: "Next-Language\tF10"
                            focusPolicy: Qt.NoFocus
                            onClicked: {
                                Scrite.app.transliterationEngine.cycleLanguage()
                                Scrite.document.formatting.defaultLanguage = Scrite.app.transliterationEngine.language
                                Runtime.paragraphLanguageSettings.defaultLanguage = Scrite.app.transliterationEngine.languageAsString
                            }
                        }
                    }

                    Repeater {
                        model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                        Item {
                            Shortcut {
                                property string shortcutKey: Scrite.app.transliterationEngine.shortcutLetter(modelData.value)
                                enabled: Runtime.allowAppUsage && Scrite.app.transliterationEngine.enabledLanguages.indexOf(modelData.value) >= 0
                                context: Qt.ApplicationShortcut
                                sequence: "Alt+"+shortcutKey
                                onActivated: {
                                    Scrite.app.transliterationEngine.language = modelData.value
                                    Scrite.document.formatting.defaultLanguage = modelData.value
                                    Runtime.paragraphLanguageSettings.defaultLanguage = modelData.key
                                }

                                ShortcutsModelItem.priority: 0
                                ShortcutsModelItem.enabled: enabled
                                ShortcutsModelItem.title: modelData.key
                                ShortcutsModelItem.group: "Language"
                                ShortcutsModelItem.shortcut: sequence
                            }
                        }
                    }

                    Shortcut {
                        context: Qt.ApplicationShortcut
                        sequence: "F10"
                        enabled: Runtime.allowAppUsage
                        onActivated: {
                            Scrite.app.transliterationEngine.cycleLanguage()
                            Scrite.document.formatting.defaultLanguage = Scrite.app.transliterationEngine.language
                            Runtime.paragraphLanguageSettings.defaultLanguage = Scrite.app.transliterationEngine.languageAsString
                        }

                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.title: "Next Language"
                        ShortcutsModelItem.group: "Language"
                        ShortcutsModelItem.shortcut: "F10"
                    }
                }

                HelpTipNotification {
                    tipName: Scrite.app.isWindowsPlatform ? "language_windows" : (Scrite.app.isMacOSPlatform ? "language_macos" : "language_linux")
                    enabled: Scrite.app.transliterationEngine.language !== TransliterationEngine.English
                }
            }

            FlatToolButton {
                iconSource: down ? "qrc:/icons/hardware/keyboard_hide.png" : "qrc:/icons/hardware/keyboard.png"
                ToolTip.text: "Show English to " + Scrite.app.transliterationEngine.languageAsString + " alphabet mappings.\t" + Scrite.app.polishShortcutTextForDisplay(shortcut)
                shortcut: "Ctrl+K"
                onClicked: alphabetMappingsPopup.visible = !alphabetMappingsPopup.visible
                down: alphabetMappingsPopup.visible
                enabled: Scrite.app.transliterationEngine.language !== TransliterationEngine.English
                visible: mainTabBar.currentIndex <= 2

                ShortcutsModelItem.priority: 1
                ShortcutsModelItem.group: "Language"
                ShortcutsModelItem.title: "Alphabet Mapping"
                ShortcutsModelItem.shortcut: shortcut
                ShortcutsModelItem.enabled: enabled

                Item {
                    anchors.top: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: alphabetMappingsPopup.width

                    Popup {
                        id: alphabetMappingsPopup
                        width: alphabetMappingsLoader.width + 30
                        height: alphabetMappingsLoader.height + 30
                        modal: false
                        focus: false
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                        Loader {
                            id: alphabetMappingsLoader
                            active: parent.visible
                            width: item ? item.width : 0
                            height: item ? item.height : 0
                            sourceComponent: AlphabetMappings {
                                enabled: Scrite.app.transliterationEngine.textInputSourceIdForLanguage(Scrite.app.transliterationEngine.language) === ""

                                Rectangle {
                                    visible: !parent.enabled
                                    color: Runtime.colors.primary.c300.background
                                    opacity: 0.9
                                    anchors.fill: parent

                                    VclLabel {
                                        width: parent.width * 0.75
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 5
                                        anchors.centerIn: parent
                                        horizontalAlignment: Text.AlignHCenter
                                        color: Runtime.colors.primary.c300.text
                                        text: {
                                            if(Scrite.app.isMacOSPlatform)
                                                return "Scrite is using an input source from macOS while typing in " + Scrite.app.transliterationEngine.languageAsString + "."
                                            return "Scrite is using an input method & keyboard layout from Windows while typing in " + Scrite.app.transliterationEngine.languageAsString + "."
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            VclLabel {
                id: languageDescLabel
                anchors.verticalCenter: parent.verticalCenter
                text: Scrite.app.transliterationEngine.languageAsString
                font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                width: 80
                visible: mainTabBar.currentIndex <= 2

                MouseArea {
                    anchors.fill: parent
                    onClicked: languageToolButton.click()
                }
            }
        }

        Row {
            id: appToolsMenu
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            visible: !appToolBar.visible

            FlatToolButton {
                text: "File"
                iconSource: "qrc:/icons/navigation/menu.png"
                onClicked: {
                    if(appFileMenu.active)
                        appFileMenu.close()
                    else
                        appFileMenu.show()
                }
                down: appFileMenu.active

                MenuLoader {
                    id: appFileMenu
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    menu: VclMenu {
                        width: 300

                        VclMenuItem {
                            text: "Home"
                            onTriggered: HomeScreen.launch()
                        }

                        VclMenuItem {
                            text: "Save"
                            onTriggered: cmdSave.doClick()
                        }

                        MenuSeparator { }

                        VclMenu {
                            id: exportMenu2
                            title: "Share"
                            width: 250

                            Repeater {
                                model: Scrite.document.supportedExportFormats

                                VclMenuItem {
                                    required property var modelData
                                    text: modelData.name
                                    icon.source: "qrc" + modelData.icon
                                    onClicked: ExportConfigurationDialog.launch(modelData.key)
                                }
                            }

                            MenuSeparator { }

                            VclMenuItem {
                                text: "Scrite"
                                icon.source: "qrc:/icons/exporter/scrite.png"
                                onClicked: SaveFileTask.saveAs()
                            }
                        }

                        VclMenu {
                            title: "Reports"
                            width: 300

                            Repeater {
                                model: Scrite.document.supportedReports

                                VclMenuItem {
                                    required property var modelData
                                    text: modelData.name
                                    icon.source: "qrc" + modelData.icon
                                    onClicked: ReportConfigurationDialog.launch(modelData.name)
                                    // enabled: Scrite.window.width >= 800
                                }
                            }
                        }

                        MenuSeparator { }

                        VclMenu {
                            // FIXME: This is a duplicate of the languageMenu.
                            // We should remove this when we build an ActionManager.
                            title: "Language"

                            Repeater {
                                model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                                VclMenuItem {
                                    property string baseText: modelData.key
                                    property string shortcutKey: Scrite.app.transliterationEngine.shortcutLetter(modelData.value)
                                    text: baseText + " (" + Scrite.app.polishShortcutTextForDisplay("Alt+"+shortcutKey) + ")"
                                    font.bold: Scrite.app.transliterationEngine.language === modelData.value
                                    onClicked: {
                                        Scrite.app.transliterationEngine.language = modelData.value
                                        Scrite.document.formatting.defaultLanguage = modelData.value
                                        Runtime.paragraphLanguageSettings.defaultLanguage = modelData.key
                                    }
                                }
                            }

                            MenuSeparator { }

                            VclMenuItem {
                                text: "Next-Language (F10)"
                                onClicked: {
                                    Scrite.app.transliterationEngine.cycleLanguage()
                                    Scrite.document.formatting.defaultLanguage = Scrite.app.transliterationEngine.language
                                    Runtime.paragraphLanguageSettings.defaultLanguage = Scrite.app.transliterationEngine.languageAsString
                                }
                            }
                        }

                        VclMenuItem {
                            text: "Alphabet Mappings For " + Scrite.app.transliterationEngine.languageAsString
                            enabled: Scrite.app.transliterationEngine.language !== TransliterationEngine.English
                            onClicked: alphabetMappingsPopup.visible = !alphabetMappingsPopup.visible
                        }

                        MenuSeparator { }

                        VclMenu {
                            title: "View"
                            width: 250

                            VclMenuItem {
                                text: "Screenplay (" + Scrite.app.polishShortcutTextForDisplay("Alt+1") + ")"
                                onTriggered: mainTabBar.activateTab(0)
                                font.bold: mainTabBar.currentIndex === 0
                            }

                            VclMenuItem {
                                text: "Structure (" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
                                onTriggered: mainTabBar.activateTab(1)
                                font.bold: mainTabBar.currentIndex === 1
                            }

                            VclMenuItem {
                                text: "Notebook (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                                onTriggered: mainTabBar.activateTab(2)
                                font.bold: mainTabBar.currentIndex === 2
                                enabled: !Runtime.showNotebookInStructure
                            }

                            VclMenuItem {
                                text: "Scrited (" + Scrite.app.polishShortcutTextForDisplay("Alt+4") + ")"
                                onTriggered: mainTabBar.currentIndex = 3
                                font.bold: mainTabBar.currentIndex === 3
                            }
                        }

                        MenuSeparator { }

                        VclMenuItem {
                            text: "Settings"
                            // enabled: Scrite.window.width >= 1100
                            onTriggered: SettingsDialog.launch()
                        }

                        VclMenuItem {
                            text: "Help"
                            onTriggered: Qt.openUrlExternally(helpUrl)
                        }
                    }
                }
            }
        }

        ScritedToolbar {
            id: scritedToolbar
            anchors.left: appToolBar.visible ? appToolBar.right : appToolsMenu.right
            anchors.right: editTools.visible ? editTools.left : parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10
            visible: scritedView
        }

        Row {
            id: editTools
            x: appToolBar.visible ? (parent.width - userLogin.width - width) : (appToolsMenu.x + (parent.width - width - appToolsMenu.width - appToolsMenu.x)/2)
            height: parent.height
            spacing: 20

            ScreenplayEditorToolbar {
                id: screenplayEditorToolbar
                property Item sceneEditor
                anchors.verticalCenter: parent.verticalCenter
                binder: sceneEditor ? sceneEditor.binder : null
                editor: sceneEditor ? sceneEditor.editor : null
                visible: {
                    var min = 0
                    var max = Runtime.showNotebookInStructure ? 1 : 2
                    return mainTabBar.currentIndex >= min && mainTabBar.currentIndex <= max
                }

                Component.onCompleted: Runtime.screenplayEditorToolbar = screenplayEditorToolbar
            }

            Row {
                id: mainTabBar
                height: parent.height
                visible: appToolBar.visible

                readonly property var tabs: [
                    { "name": "Screenplay", "icon": "qrc:/icons/navigation/screenplay_tab.png", "visible": true, "tab": Runtime.e_ScreenplayTab },
                    { "name": "Structure", "icon": "qrc:/icons/navigation/structure_tab.png", "visible": true, "tab": Runtime.e_StructureTab },
                    { "name": "Notebook", "icon": "qrc:/icons/navigation/notebook_tab.png", "visible": !Runtime.showNotebookInStructure, "tab": Runtime.e_NotebookTab },
                    { "name": "Scrited", "icon": "qrc:/icons/navigation/scrited_tab.png", "visible": Runtime.workspaceSettings.showScritedTab, "tab": Runtime.e_ScritedTab }
                ]
                readonly property color activeTabColor: Runtime.colors.primary.windowColor
                function indexOfTab(_Runtime_TabType) {
                    for(var i=0; i<tabs.length; i++) {
                        if(tabs[i].tab === _Runtime_TabType) {
                            return i
                        }
                    }
                    return -1
                }

                Connections {
                    target: Scrite.document
                    function onJustReset() { mainTabBar.activateTab(0) }
                    function onAboutToSave() {
                        var userData = Scrite.document.userData
                        userData["mainTabBar"] = {
                            "version": 0,
                            "currentIndex": mainTabBar.currentIndex
                        }
                        Scrite.document.userData = userData
                    }
                    function onJustLoaded() {
                        var userData = Scrite.document.userData
                        if(userData.mainTabBar) {
                            var ci = userData.mainTabBar.currentIndex
                            if(ci >= 0 && ci <= 2)
                                mainTabBar.activateTab(ci)
                            else
                                mainTabBar.activateTab(0)
                        } else
                            mainTabBar.activateTab(0)
                    }
                }

                function activateTab(index) {
                    if(index < 0 || index >= tabs.length || index === mainTabBar.currentIndex)
                        return
                    var tab = tabs[index]
                    if(!tab.visible)
                        index = 0
                    var message = "Preparing the <b>" + tabs[index].name + "</b> tab, just a few seconds ..."
                    Scrite.document.setBusyMessage(message)
                    Scrite.document.screenplay.clearSelection()
                    Utils.execLater(mainTabBar, 100, function() {
                        mainTabBar.currentIndex = index
                        Scrite.document.clearBusyMessage()
                    })
                }

                property Item currentTab: currentIndex >= 0 && mainTabBarRepeater.count === tabs.length ? mainTabBarRepeater.itemAt(currentIndex) : null
                property int currentIndex: -1
                property var currentTabP1: currentTabExtents.value.p1
                property var currentTabP2: currentTabExtents.value.p2

                onCurrentIndexChanged: {
                    Runtime.mainWindowTab = tabs[currentIndex].tab
                }
                Component.onCompleted: {
                    Runtime.mainWindowTab = Runtime.e_ScreenplayTab
                    currentIndex = indexOfTab(Runtime.mainWindowTab)

                    const syncCurrentIndex = ()=>{
                        const idx = indexOfTab(Runtime.mainWindowTab)
                        if(currentIndex !== idx)
                            currentIndex = idx
                    }
                    Runtime.mainWindowTabChanged.connect( () => {
                                                                   Qt.callLater(syncCurrentIndex)
                                                               } )

                    Runtime.activateMainWindowTab.connect( (tabType) => {
                                                                    const tabIndex = indexOfTab(tabType)
                                                                    activateTab(tabIndex)
                                                                } )
                }

                TrackerPack {
                    id: currentTabExtents
                    TrackProperty { target: mainTabBar; property: "visible" }
                    TrackProperty { target: appToolBarArea; property: "width" }
                    TrackProperty { target: mainTabBar; property: "currentTab" }

                    property var value: fallback
                    readonly property var fallback: {
                        "p1": { "x": 0, "y": 0 },
                        "p2": { "x": 0, "y": 0 }
                    }

                    onTracked:  {
                        if(mainTabBar.visible && mainTabBar.currentTab !== null) {
                            value = {
                                "p1": mainTabBar.mapFromItem(mainTabBar.currentTab, 0, 0),
                                "p2": mainTabBar.mapFromItem(mainTabBar.currentTab, mainTabBar.currentTab.width, 0)
                            }
                        } else
                            value = fallback
                    }
                }

                Repeater {
                    id: mainTabBarRepeater
                    model: mainTabBar.tabs

                    Item {
                        property bool active: mainTabBar.currentIndex === index
                        height: mainTabBar.height
                        width: height
                        visible: modelData.visible
                        enabled: modelData.visible

                        PainterPathItem {
                            anchors.fill: parent
                            fillColor: parent.active ? mainTabBar.activeTabColor : Runtime.colors.primary.c10.background
                            outlineColor: Runtime.colors.primary.borderColor
                            outlineWidth: 1
                            renderingMechanism: PainterPathItem.UseQPainter
                            renderType: parent.active ? PainterPathItem.OutlineAndFill : PainterPathItem.FillOnly
                            painterPath: PainterPath {
                                id: tabButtonPath
                                readonly property point p1: Qt.point(itemRect.left, itemRect.bottom)
                                readonly property point p2: Qt.point(itemRect.left, itemRect.top + 3)
                                readonly property point p3: Qt.point(itemRect.right-1, itemRect.top + 3)
                                readonly property point p4: Qt.point(itemRect.right-1, itemRect.bottom)
                                MoveTo { x: tabButtonPath.p1.x; y: tabButtonPath.p1.y }
                                LineTo { x: tabButtonPath.p2.x; y: tabButtonPath.p2.y }
                                LineTo { x: tabButtonPath.p3.x; y: tabButtonPath.p3.y }
                                LineTo { x: tabButtonPath.p4.x; y: tabButtonPath.p4.y }
                            }
                        }

                        FontMetrics {
                            id: tabBarFontMetrics
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        }

                        Image {
                            source: modelData.icon
                            width: parent.active ? 32 : 24; height: width
                            Behavior on width {
                                enabled: Runtime.applicationSettings.enableAnimations
                                NumberAnimation { duration: Runtime.stdAnimationDuration }
                            }

                            fillMode: Image.PreserveAspectFit
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: parent.active ? 0 : 1
                            opacity: parent.active ? 1 : 0.75
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: mainTabBar.activateTab(index)
                            ToolTip.text: modelData.name + "\t" + Scrite.app.polishShortcutTextForDisplay("Alt+"+(index+1))
                            ToolTip.delay: 1000
                            ToolTip.visible: containsMouse
                        }
                    }
                }
            }
        }

        UserAccountToolButton {
            id: userLogin
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Loader {
        id: mainUiContentLoader
        active: allowContent && !Scrite.document.loading
        opacity: 0
        sourceComponent: uiLayoutComponent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: appToolBarArea.visible ? appToolBarArea.bottom : parent.top
        anchors.bottom: parent.bottom
        onActiveChanged: screenplayEditorToolbar.sceneEditor = null

        property bool allowContent: Runtime.loadMainUiContent
        property string sessionId

        Announcement.onIncoming: (type, data) => {
                                               if(type === Runtime.announcementIds.reloadMainUiRequest) {
                                                    mainUiContentLoader.active = false

                                                    const delay = data && typeof data === "number" ? data : 100
                                                    Utils.execLater(mainUiContentLoader, delay, () => {
                                                                        mainUiContentLoader.active = true
                                                                    })
                                                }
                                           }

        // Recfactor QML: Get rid of this function, unless its called from this file itself.
        // It encourages usage of leap-of-faith IDs, which is a bad idea.
        function reset(callback) {
            active = false
            Qt.callLater( (callback) => {
                             if(callback)
                                 callback()
                             mainUiContentLoader.active = true
                         }, callback )
        }

        Connections {
            target: Runtime

            function onResetMainWindowUi(callback) {
                mainUiContentLoader.reset(callback)
            }
        }

        Component.onCompleted: Utils.execLater(mainUiContentLoader, 200, () => { mainUiContentLoader.opacity = 1 } )
    }

    Component {
        id: uiLayoutComponent

        Rectangle {
            color: mainTabBar.activeTabColor

            PainterPathItem {
                id: tabBarSeparator
                anchors.left: parent.left
                anchors.right: parent.right
                renderingMechanism: PainterPathItem.UseQPainter
                renderType: PainterPathItem.OutlineOnly
                height: 1
                outlineColor: Runtime.colors.primary.borderColor
                outlineWidth: height
                visible: mainTabBar.visible
                painterPath: PainterPath {
                    id: tabBarSeparatorPath
                    property var currentTabP1: tabBarSeparator.mapFromItem(mainTabBar, mainTabBar.currentTabP1.x, mainTabBar.currentTabP1.y)
                    property var currentTabP2: tabBarSeparator.mapFromItem(mainTabBar, mainTabBar.currentTabP2.x, mainTabBar.currentTabP2.y)
                    property point p1: Qt.point(itemRect.left, itemRect.center.y)
                    property point p2: Qt.point(currentTabP1.x, itemRect.center.y)
                    property point p3: Qt.point(currentTabP2.x, itemRect.center.y)
                    property point p4: Qt.point(itemRect.right, itemRect.center.y)

                    MoveTo { x: tabBarSeparatorPath.p1.x; y: tabBarSeparatorPath.p1.y }
                    LineTo { x: tabBarSeparatorPath.p2.x; y: tabBarSeparatorPath.p2.y }
                    MoveTo { x: tabBarSeparatorPath.p3.x; y: tabBarSeparatorPath.p3.y }
                    LineTo { x: tabBarSeparatorPath.p4.x; y: tabBarSeparatorPath.p4.y }
                }
            }

            Loader {
                id: uiLoader
                anchors.fill: parent
                anchors.topMargin: 1
                clip: true
                sourceComponent: {
                    switch(mainTabBar.currentIndex) {
                    case 1: return structureEditorComponent
                    case 2: return notebookEditorComponent
                    case 3: return scritedComponent
                    }
                    return screenplayEditorComponent
                }

                Announcement.onIncoming: (type,data) => {
                                             const stype = "" + type
                                             if(mainTabBar.currentIndex === 0 && stype === "{f4048da2-775d-11ec-90d6-0242ac120003}") {
                                                 uiLoader.active = false
                                                 Utils.execLater(uiLoader, 250, function() {
                                                    uiLoader.active = true
                                                 })
                                             }
                                         }
            }
        }
    }

    Component {
        id: screenplayEditorComponent

        ScreenplayEditor {
            id: screenplayEditor

            HelpTipNotification {
                tipName: "screenplay"
            }

            BasicAttachmentsDropArea {
                id: fileOpenDropArea
                anchors.fill: parent
                allowedType: Attachments.NoMedia
                allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]
                property string droppedFilePath
                property string droppedFileName
                onDropped: {
                    if(Scrite.document.empty)
                        Scrite.document.openOrImport(attachment.filePath)
                    else {
                        droppedFilePath = attachment.filePath
                        droppedFileName = attachment.originalFileName
                    }

                    Announcement.shout(Runtime.announcementIds.closeDialogBoxRequest, undefined)
                }

                Loader {
                    id: fileOpenDropAreaNotification
                    anchors.fill: fileOpenDropArea
                    active: fileOpenDropArea.active || fileOpenDropArea.droppedFilePath !== ""
                    onActiveChanged: appToolBarArea.enabled = !active
                    Component.onDestruction: appToolBarArea.enabled = true
                    sourceComponent: Rectangle {
                        color: Scrite.app.translucent(Runtime.colors.primary.c500.background, 0.5)

                        Rectangle {
                            anchors.fill: fileOpenDropAreaNotice
                            anchors.margins: -30
                            radius: 4
                            color: Runtime.colors.primary.c700.background
                        }

                        Column {
                            id: fileOpenDropAreaNotice
                            anchors.centerIn: parent
                            width: parent.width * 0.5
                            spacing: 20

                            VclLabel {
                                wrapMode: Text.WordWrap
                                width: parent.width
                                color: Runtime.colors.primary.c700.text
                                font.bold: true
                                text: fileOpenDropArea.active ? fileOpenDropArea.attachment.originalFileName : fileOpenDropArea.droppedFileName
                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            VclLabel {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                color: Runtime.colors.primary.c700.text
                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                text: fileOpenDropArea.active ? "Drop the file here to open/import it." : "Do you want to open, import or cancel?"
                            }

                            VclLabel {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                color: Runtime.colors.primary.c700.text
                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                visible: !Scrite.document.empty || Scrite.document.fileName !== ""
                                text: "NOTE: Any unsaved changes in the currently open document will be discarded."
                            }

                            Row {
                                spacing: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                                visible: !Scrite.document.empty

                                VclButton {
                                    text: "Open/Import"
                                    onClicked: {
                                        Scrite.document.openOrImport(fileOpenDropArea.droppedFilePath)
                                        fileOpenDropArea.droppedFileName = ""
                                        fileOpenDropArea.droppedFilePath = ""
                                    }
                                }

                                VclButton {
                                    text: "Cancel"
                                    onClicked:  {
                                        fileOpenDropArea.droppedFileName = ""
                                        fileOpenDropArea.droppedFilePath = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: structureEditorComponent

        SplitView {
            id: structureEditorSplitView1
            orientation: Qt.Vertical
            Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

            Rectangle {
                id: structureEditorRow1
                SplitView.fillHeight: true
                color: Runtime.colors.primary.c10.background

                SplitView {
                    id: structureEditorSplitView2
                    orientation: Qt.Horizontal
                    Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)
                    anchors.fill: parent

                    Rectangle {
                        SplitView.fillWidth: true
                        SplitView.minimumWidth: 80
                        color: Runtime.colors.primary.c10.background
                        border {
                            width: Runtime.showNotebookInStructure ? 0 : 1
                            color: Runtime.colors.primary.borderColor
                        }

                        Item {
                            id: structureEditorTabs
                            anchors.fill: parent
                            property int currentTabIndex: 0

                            Announcement.onIncoming: (type,data) => {
                                var sdata = "" + data
                                var stype = "" + type
                                if(Runtime.showNotebookInStructure) {
                                    if(stype === "190B821B-50FE-4E47-A4B2-BDBB2A13B72C") {
                                        if(sdata === "Structure")
                                            structureEditorTabs.currentTabIndex = 0
                                        else if(sdata.startsWith("Notebook")) {
                                            structureEditorTabs.currentTabIndex = 1
                                            if(sdata !== "Notebook")
                                                Utils.execLater(notebookViewLoader, 100, function() {
                                                    notebookViewLoader.item.switchTo(sdata)
                                                })
                                        }
                                    } else if(stype === Runtime.announcementIds.characterNotesRequest) {
                                        structureEditorTabs.currentTabIndex = 1
                                        Utils.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToCharacterTab(data)
                                        })
                                    }
                                    else if(stype === Runtime.announcementIds.sceneNotesRequest) {
                                        structureEditorTabs.currentTabIndex = 1
                                        Utils.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToSceneTab(data)
                                        })
                                    }
                                }
                            }

                            Loader {
                                id: structureEditorTabBar
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                // active: !Runtime.appFeatures.structure.enabled && Runtime.showNotebookInStructure
                                active: {
                                    if(structureEditorTabs.currentTabIndex === 0)
                                        return !Runtime.appFeatures.structure.enabled && Runtime.showNotebookInStructure
                                    else if(structureEditorTabs.currentTabIndex === 1)
                                        return !Runtime.appFeatures.notebook.enabled && Runtime.showNotebookInStructure
                                    return false
                                }
                                visible: active
                                sourceComponent: Rectangle {
                                    color: Runtime.colors.primary.c100.background
                                    width: appToolBar.height+4

                                    Column {
                                        anchors.horizontalCenter: parent.horizontalCenter

                                        FlatToolButton {
                                            down: structureEditorTabs.currentTabIndex === 0
                                            visible: Runtime.showNotebookInStructure
                                            iconSource: "qrc:/icons/navigation/structure_tab.png"
                                            ToolTip.text: "Structure\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
                                            onClicked: Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Structure")
                                        }

                                        FlatToolButton {
                                            down: structureEditorTabs.currentTabIndex === 1
                                            visible: Runtime.showNotebookInStructure
                                            iconSource: "qrc:/icons/navigation/notebook_tab.png"
                                            ToolTip.text: "Notebook Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                                            onClicked: Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Notebook")
                                        }
                                    }

                                    Rectangle {
                                        width: 1
                                        height: parent.height
                                        anchors.right: parent.right
                                        color: Runtime.colors.primary.borderColor
                                    }
                                }
                            }

                            Loader {
                                id: structureViewLoader
                                anchors.top: parent.top
                                anchors.left: structureEditorTabBar.active ? structureEditorTabBar.right : parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                visible: !Runtime.showNotebookInStructure || structureEditorTabs.currentTabIndex === 0
                                active: Runtime.appFeatures.structure.enabled
                                sourceComponent: StructureView {
                                    HelpTipNotification {
                                        tipName: "structure"
                                        enabled: structureViewLoader.visible
                                    }

                                    onEditorRequest: { } // TODO
                                    onReleaseEditorRequest: { } // TODO
                                }

                                DisabledFeatureNotice {
                                    anchors.fill: parent
                                    visible: !parent.active
                                    featureName: "Structure"
                                }
                            }

                            Loader {
                                id: notebookViewLoader
                                anchors.top: parent.top
                                anchors.left: structureEditorTabBar.active ? structureEditorTabBar.right : parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                visible: Runtime.showNotebookInStructure && structureEditorTabs.currentTabIndex === 1
                                active: visible && Runtime.appFeatures.notebook.enabled
                                sourceComponent: NotebookView {
                                    toolbarSize: appToolBar.height+4
                                    toolbarSpacing: appToolBar.spacing
                                    toolbarLeftMargin: appToolBar.anchors.leftMargin
                                }

                                DisabledFeatureNotice {
                                    anchors.fill: parent
                                    visible: !parent.active
                                    featureName: "Notebook"
                                }
                            }
                        }

                        /**
                          Some of our users find it difficult to know that they can pull the splitter handle
                          to reveal the timeline and/or screenplay editor. So we load an animation letting them
                          know about that and get rid of it once the animation is done.
                          */
                        Loader {
                            id: splitViewAnimationLoader
                            anchors.fill: parent
                            active: false
                            property string sessionId
                            sourceComponent: Rectangle {
                                color: Scrite.app.translucent(Runtime.colors.primary.button, 0.5)

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: splitViewAnimationLoader.active = false
                                }

                                Timer {
                                    interval: 5000
                                    repeat: false
                                    running: true
                                    onTriggered: splitViewAnimationLoader.active = false
                                }

                                Item {
                                    id: screenplayEditorHandle
                                    width: 1
                                    property real marginOnTheRight: 0
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.rightMargin: marginOnTheRight
                                    visible: !screenplayEditor2.active

                                    Rectangle {
                                        height: parent.height * 0.5
                                        width: 5
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: Runtime.colors.primary.windowColor
                                        visible: screenplayEditorHandleAnimation.running
                                    }

                                    VclLabel {
                                        color: Runtime.colors.primary.c50.background
                                        text: "Pull this handle to view the screenplay editor."
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                                        anchors.right: parent.left
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.rightMargin: 20
                                    }

                                    SequentialAnimation {
                                        id: screenplayEditorHandleAnimation
                                        loops: 2
                                        running: screenplayEditorHandle.visible

                                        NumberAnimation {
                                            target: screenplayEditorHandle
                                            property: "marginOnTheRight"
                                            duration: 500
                                            from: 0; to: 50
                                        }

                                        NumberAnimation {
                                            target: screenplayEditorHandle
                                            property: "marginOnTheRight"
                                            duration: 500
                                            from: 50; to: 0
                                        }
                                    }
                                }

                                Item {
                                    id: timelineViewHandle
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: marginOnTheBottom
                                    height: 1
                                    visible: !structureEditorRow2.active
                                    property real marginOnTheBottom: 0

                                    Rectangle {
                                        width: parent.width * 0.5
                                        height: 5
                                        anchors.bottom: parent.bottom
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        color: Runtime.colors.primary.windowColor
                                        visible: timelineViewHandleAnimation.running
                                    }

                                    VclLabel {
                                        color: Runtime.colors.primary.c50.background
                                        text: "Pull this handle to get the timeline view."
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.bottom: parent.top
                                        anchors.bottomMargin: 20
                                    }

                                    SequentialAnimation {
                                        id: timelineViewHandleAnimation
                                        loops: 2
                                        running: timelineViewHandle.visible

                                        NumberAnimation {
                                            target: timelineViewHandle
                                            property: "marginOnTheBottom"
                                            duration: 500
                                            from: 0; to: 50
                                        }

                                        NumberAnimation {
                                            target: timelineViewHandle
                                            property: "marginOnTheBottom"
                                            duration: 500
                                            from: 50; to: 0
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Loader {
                        id: screenplayEditor2
                        SplitView.preferredWidth: scriteMainWindow.width * 0.5
                        SplitView.minimumWidth: 16
                        onWidthChanged: Runtime.workspaceSettings.screenplayEditorWidth = width
                        active: width >= 50
                        sourceComponent: mainTabBar.currentIndex === 1 ? screenplayEditorComponent : null

                        Rectangle {
                            visible: !parent.active
                            anchors.fill: parent
                            color: Runtime.colors.primary.c400.background
                        }
                    }
                }
            }

            Loader {
                id: structureEditorRow2
                SplitView.preferredHeight: 140 + Runtime.minimumFontMetrics.height*Runtime.screenplayTracks.trackCount
                SplitView.minimumHeight: 16
                SplitView.maximumHeight: SplitView.preferredHeight
                active: height >= 50
                sourceComponent: Rectangle {
                    color: FocusTracker.hasFocus ? Runtime.colors.accent.c100.background : Runtime.colors.accent.c50.background
                    FocusTracker.window: Scrite.window

                    Behavior on color {
                        enabled: Runtime.applicationSettings.enableAnimations
                        ColorAnimation { duration: 250 }
                    }

                    TimelineView {
                        anchors.fill: parent
                        showNotesIcon: Runtime.showNotebookInStructure
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: Qt.rgba(0,0,0,0)
                        border { width: 1; color: Runtime.colors.accent.borderColor }
                    }
                }

                Rectangle {
                    visible: !parent.active
                    anchors.fill: parent
                    color: Runtime.colors.primary.c400.background
                }
            }

            Connections {
                target: Scrite.document
                function onAboutToSave() { structureEditorSplitView1.saveLayoutDetails() }
                function onJustLoaded() { structureEditorSplitView1.restoreLayoutDetails() }
            }

            Component.onCompleted: restoreLayoutDetails()
            Component.onDestruction: saveLayoutDetails()

            function saveLayoutDetails() {
                var userData = Scrite.document.userData
                userData["structureTab"] = {
                    "version": 0,
                    "screenplayEditorWidth": screenplayEditor2.width/structureEditorRow1.width,
                    "timelineViewHeight": structureEditorRow2.height
                }
                Scrite.document.userData = userData
            }

            function restoreLayoutDetails() {
                var userData = Scrite.document.userData
                if(userData.structureTab && userData.structureTab.version === 0) {
                    structureEditorRow2.SplitView.preferredHeight = userData.structureTab.timelineViewHeight
                    structureEditorRow2.height = structureEditorRow2.SplitView.preferredHeight
                    screenplayEditor2.SplitView.preferredWidth = structureEditorRow1.width*userData.structureTab.screenplayEditorWidth
                    screenplayEditor2.width = screenplayEditor2.SplitView.preferredWidth
                }

                if(Runtime.structureCanvasSettings.showPullHandleAnimation && mainUiContentLoader.sessionId !== Scrite.document.sessionId) {
                    Utils.execLater(splitViewAnimationLoader, 250, function() {
                        splitViewAnimationLoader.active = !screenplayEditor2.active || !structureEditorRow2.active
                    })
                    mainUiContentLoader.sessionId = Scrite.document.sessionId
                }
            }
        }
    }

    Component {
        id: notebookEditorComponent

        Loader {
            active: Runtime.appFeatures.notebook.enabled
            sourceComponent: NotebookView {
                Announcement.onIncoming: (type,data) => {
                    var stype = "" + "190B821B-50FE-4E47-A4B2-BDBB2A13B72C"
                    var sdata = "" + data
                    if(stype === "190B821B-50FE-4E47-A4B2-BDBB2A13B72C")
                        switchTo(sdata)
                }
            }

            DisabledFeatureNotice {
                anchors.fill: parent
                visible: !parent.active
                featureName: "Notebook"
            }
        }
    }

    Component {
        id: scritedComponent

        Loader {
            active: Runtime.appFeatures.scrited.enabled
            sourceComponent: ScritedView {

            }

            DisabledFeatureNotice {
                anchors.fill: parent
                visible: !parent.active
                featureName: "Scrited"
            }
        }
    }

    Item {
        id: closeEventHandler
        width: 100
        height: 100
        anchors.centerIn: parent

        property bool handleCloseEvent: true

        Connections {
            target: Scrite.window

            function onClosing(close) {
                if(!Scrite.window.closeButtonVisible) {
                    close.accepted = false
                    return
                }

                if(closeEventHandler.handleCloseEvent) {
                    close.accepted = false

                    Scrite.app.saveWindowGeometry(Scrite.window, "Workspace")

                    SaveFileTask.save( () => {
                                          closeEventHandler.handleCloseEvent = false
                                          if( TrialNotActivatedDialog.launch() !== null)
                                            return
                                          Scrite.window.close()
                                      } )
                } else
                    close.accepted = true
            }
        }
    }

    QtObject {
        ShortcutsModelItem.enabled: Scrite.app.isTextInputItem(Scrite.window.activeFocusItem)
        ShortcutsModelItem.priority: 10
        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: "Symbols & Smileys"
        ShortcutsModelItem.shortcut: "F3"
    }

    Component.onCompleted: {
        if(!Scrite.app.restoreWindowGeometry(Scrite.window, "Workspace"))
            Runtime.workspaceSettings.screenplayEditorWidth = -1
        Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
        Qt.callLater( function() {
            Announcement.shout("{f4048da2-775d-11ec-90d6-0242ac120003}", "restoreWindowGeometryDone")
        })
    }

    BusyOverlay {
        id: appBusyOverlay
        anchors.fill: parent
        busyMessage: "Computing Page Layout, Evaluating Page Count & Time ..."
        visible: RefCounter.isReffed
        function ref() { RefCounter.ref() }
        function deref() { RefCounter.deref() }
    }

    HelpTipNotification {
        id: htNotification
        enabled: tipName !== ""

        Component.onCompleted: {
            Qt.callLater( () => {
                             if(Runtime.helpNotificationSettings.dayZero === "")
                                Runtime.helpNotificationSettings.dayZero = new Date()

                             const days = Runtime.helpNotificationSettings.daysSinceZero()
                             if(days >= 2) {
                                 if(!Runtime.helpNotificationSettings.isTipShown("discord"))
                                     htNotification.tipName = "discord"
                             }
                         })
        }
    }

    QtObject {
        property ErrorReport applicationErrors: Aggregation.findErrorReport(Scrite.app)
        property bool errorReportHasError: applicationErrors.hasError
        onErrorReportHasErrorChanged: {
            if(errorReportHasError)
                MessageBox.information("Scrite Error", applicationErrors.errorMessage, applicationErrors.clear)
        }
    }

    QtObject {
        property ErrorReport documentErrors: Aggregation.findErrorReport(Scrite.document)
        property bool errorReportHasError: documentErrors.hasError
        onErrorReportHasErrorChanged: {
            if(errorReportHasError) {
                var msg = documentErrors.errorMessage;

                if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                    msg += "<br/><br/>Click Ok to reveal <u>" + documentErrors.details.revealOnDesktopRequest + "</u> on your computer."

                MessageBox.information("Scrite Document Error", msg, () => {
                                           if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                                               Scrite.app.revealFileOnDesktop(documentErrors.details.revealOnDesktopRequest)
                                           documentErrors.clear()
                                       })
            }
        }
    }

    QtObject {
        id: _private

        function handleOpenFileRequest(fileName) {
            if(Scrite.app.isMacOSPlatform) {
                if(Scrite.document.empty) {
                    Announcement.shout(Runtime.announcementIds.closeHomeScreenRequest, undefined)
                    OpenFileTask.open(fileName)
                } else {
                    let fileInfo = Qt.createQmlObject("import io.scrite.components 1.0; BasicFileInfo { }", _private)
                    fileInfo.absoluteFilePath = fileName

                    const justFileName = fileInfo.baseName
                    fileInfo.destroy()

                    MessageBox.question("Open Options",
                                        "How do you want to open <b>" + justFileName + "</b>?",
                                        ["This Window", "New Window"], (answer) => {
                                            if(answer === "This Window")
                                                OpenFileTask.open(fileName)
                                            else
                                                Scrite.app.launchNewInstanceAndOpen(Scrite.window, fileName);
                                        })
                }
            }
        }

        function showHelpTip(tipName) {
            if(Runtime.helpTips[tipName] !== undefined && !Runtime.helpNotificationSettings.isTipShown(tipName)) {
                helpTipNotification.createObject(Scrite.window.contentItem, {"tipName": tipName})
            }
        }

        Announcement.onIncoming: (type, data) => {
                                     if(type === Runtime.announcementIds.showHelpTip) {
                                         _private.showHelpTip(""+data)
                                     }
                                 }

        Component.onCompleted: {
            if(Scrite.app.isMacOSPlatform)
                Scrite.app.openFileRequest.connect(handleOpenFileRequest)
        }
    }

    Component {
        id: helpTipNotification
        HelpTipNotification {
            id: helpTip
            Notification.onDismissed: helpTip.destroy()
        }
    }
}
