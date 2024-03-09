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

import "../js/utils.js" as Utils
import "./globals"

Item {
    id: scriteDocumentViewItem
    width: 1350
    height: 700

    readonly property url helpUrl: "https://www.scrite.io/index.php/help/"

    enabled: !Scrite.document.loading

    property bool canShowNotebookInStructure: width > 1600
    property bool showNotebookInStructure: ScriteRuntime.workspaceSettings.showNotebookInStructure && canShowNotebookInStructure
    onShowNotebookInStructureChanged: {
        Utils.execLater(ScriteRuntime.workspaceSettings, 100, function() {
            mainTabBar.currentIndex = mainTabBar.currentIndex % (showNotebookInStructure ? 2 : 3)
        })
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+S"
        onActivated: saveFileDialog.launch("SAVE_AS")

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Save As"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+N"
        onActivated: showHomeScreen(homeButton)

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "New"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+O"
        onActivated: showHomeScreen(homeButton)

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Open"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+O"
        onActivated: {
            showHomeScreen(homeButton)
            Utils.execLater(modalDialog, 500, () => {
                                Announcement.shout("710A08E7-9F60-4D36-9DEA-0993EEBA7DCA", "Scriptalay")
                            })
        }

        ShortcutsModelItem.group: "File"
        ShortcutsModelItem.title: "Scriptalay"
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+P"

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Export To PDF"
        ShortcutsModelItem.shortcut: sequence
        onActivated: showExportWorkflow("Screenplay/Adobe PDF")
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "F1"

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Help"
        ShortcutsModelItem.shortcut: sequence
        onActivated: Qt.openUrlExternally(helpUrl)
    }

    Shortcut {
        id: sceneCharactersToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+C"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: ScriteRuntime.screenplayEditorSettings.displaySceneCharacters ? "Hide Scene Characters, Tags" : "Show Scene Characters, Tags"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ScriteRuntime.screenplayEditorSettings.displaySceneCharacters = !ScriteRuntime.screenplayEditorSettings.displaySceneCharacters
    }

    Shortcut {
        id: synopsisToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+S"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: ScriteRuntime.screenplayEditorSettings.displaySceneSynopsis ? "Hide Synopsis" : "Show Synopsis"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ScriteRuntime.screenplayEditorSettings.displaySceneSynopsis = !ScriteRuntime.screenplayEditorSettings.displaySceneSynopsis
    }

    Shortcut {
        id: commentsToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+M"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: ScriteRuntime.screenplayEditorSettings.displaySceneComments ? "Hide Comments" : "Show Comments"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ScriteRuntime.screenplayEditorSettings.displaySceneComments = !ScriteRuntime.screenplayEditorSettings.displaySceneComments
    }

    Shortcut {
        id: taggingToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+G"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: ScriteRuntime.screenplayEditorSettings.allowTaggingOfScenes ? "Allow Tagging" : "Disable Tagging"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ScriteRuntime.screenplayEditorSettings.allowTaggingOfScenes = !ScriteRuntime.screenplayEditorSettings.allowTaggingOfScenes
    }

    Shortcut {
        id: spellCheckToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+L"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: ScriteRuntime.screenplayEditorSettings.enableSpellCheck ? "Disable Spellcheck" : "Enable Spellcheck"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ScriteRuntime.screenplayEditorSettings.enableSpellCheck = !ScriteRuntime.screenplayEditorSettings.enableSpellCheck
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+A"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: ScriteRuntime.applicationSettings.enableAnimations ? "Disable Animations" : "Enable Animations"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ScriteRuntime.applicationSettings.enableAnimations = !ScriteRuntime.applicationSettings.enableAnimations
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+H"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: ScriteRuntime.screenplayEditorSettings.highlightCurrentLine ? "Line Highlight Off" : "Line Highlight On"
        ShortcutsModelItem.shortcut: sequence
        onActivated: ScriteRuntime.screenplayEditorSettings.highlightCurrentLine = !ScriteRuntime.screenplayEditorSettings.highlightCurrentLine
    }

    Shortcut {
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
        context: Qt.ApplicationShortcut
        sequence: "Alt+1"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Screenplay"
        ShortcutsModelItem.shortcut: sequence
        onActivated: mainTabBar.activateTab(0)
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Alt+2"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Structure"
        ShortcutsModelItem.shortcut: sequence
        onActivated: {
            mainTabBar.activateTab(1)
            if(showNotebookInStructure)
                Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Structure")
        }
    }

    Shortcut {
        id: notebookShortcut
        context: Qt.ApplicationShortcut
        sequence: "Alt+3"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Notebook"
        ShortcutsModelItem.shortcut: sequence
        onActivated: {
            if(showNotebookInStructure) {
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

        property bool notebookTabVisible: mainTabBar.currentIndex === (showNotebookInStructure ? 1 : 2)

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
            var nbt = showNotebookInStructure ? 1 : 2
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
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+K"
        onActivated: notebookShortcut.showBookmarkedNotes()

        ShortcutsModelItem.group: notebookShortcut.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Bookmarked Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
    }

    Shortcut {
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
        enabled: ScriteRuntime.workspaceSettings.showScritedTab
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Scrited"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: enabled
        onActivated: mainTabBar.activateTab(3)
    }

    Connections {
        target: Scrite.document

        function onJustReset() {
            ScriteRuntime.screenplayEditorSettings.firstSwitchToStructureTab = true
            appBusyOverlay.ref()
            ScriteRuntime.screenplayAdapter.initialLoadTreshold = 25
            Utils.execLater(ScriteRuntime.screenplayAdapter, 250, () => {
                                appBusyOverlay.deref()
                                ScriteRuntime.screenplayAdapter.sessionId = Scrite.document.sessionId
                            })
        }

        function onJustLoaded() {
            ScriteRuntime.screenplayEditorSettings.firstSwitchToStructureTab = true
            var firstElement = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.firstSceneIndex())
            if(firstElement) {
                var editorHints = firstElement.editorHints
                if(editorHints)
                    ScriteRuntime.screenplayAdapter.initialLoadTreshold = -1
            }
        }
    }

    // Refactor QML TODO: Get rid of this stuff when we move to overlays and ApplicationMainWindow
    QtObject {
        property bool overlayRefCountModified: false
        property bool requiresAppBusyOverlay: ScriteRuntime.undoStack.screenplayEditorActive || ScriteRuntime.undoStack.sceneEditorActive

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
            ScriteRuntime.screenplayTextDocument.onUpdateScheduled.connect(onUpdateScheduled)
            ScriteRuntime.screenplayTextDocument.onUpdateFinished.connect(onUpdateFinished)
        }
    }

    Rectangle {
        id: appToolBarArea
        anchors.left: parent.left
        anchors.right: parent.right
        height: 53
        color: ScriteRuntime.colors.primary.c50.background
        visible: !pdfViewer.active
        enabled: visible

        Row {
            id: appToolBar
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 5
            visible: appToolBarArea.width >= 1150
            onVisibleChanged: {
                if(enabled && !visible)
                    mainTabBar.activateTab(0)
            }

            // spacing: scriteDocumentViewItem.width >= 1440 ? 2 : 0

            ToolButton3 {
                id: homeButton
                iconSource: "../icons/action/home.png"
                text: "Home"
                onClicked: showHomeScreen()
            }

            ToolButton3 {
                id: backupOpenButton
                iconSource: "../icons/file/backup_open.png"
                text: "Open Backup"
                visible: Scrite.document.backupFilesModel.count > 0
                onClicked: {
                    modalDialog.closeable = false
                    modalDialog.closeOnEscape = true
                    modalDialog.popupSource = backupOpenButton
                    modalDialog.sourceComponent = backupsDialogBoxComponent
                    modalDialog.active = true
                }

                ToolTip.text: "Open any of the " + Scrite.document.backupFilesModel.count + " backup(s) available for this file."

                Text {
                    id: backupCountHint
                    font.pixelSize: parent.height * 0.2
                    font.bold: true
                    text: Scrite.document.backupFilesModel.count
                    padding: 2
                    color: ScriteRuntime.colors.primary.highlight.text
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                }
            }

            ToolButton3 {
                id: cmdSave
                iconSource: "../icons/content/save.png"
                text: "Save"
                shortcut: "Ctrl+S"
                enabled: Scrite.document.modified && !Scrite.document.readOnly
                onClicked: saveFileDialog.launch()

                ShortcutsModelItem.group: "File"
                ShortcutsModelItem.title: text
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: shortcut
            }

            ToolButton3 {
                id: cmdExport
                text: "Export to ..."
                iconSource: "../icons/action/share.png"
                enabled: appToolsMenu.visible === false
                onClicked: exportMenu.open()
                down: exportMenu.visible

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    Menu2 {
                        id: exportMenu
                        width: 300

                        Repeater {
                            model: Scrite.document.supportedExportFormats

                            MenuItem2 {
                                required property var modelData
                                text: modelData.name
                                icon.source: "qrc" + modelData.icon
                                onClicked: showExportWorkflow(modelData.key)

                                ToolTip {
                                    text: modelData.description + "\n\nCategory: " + modelData.category
                                    width: 300
                                    visible: parent.hovered
                                    delay: Qt.styleHints.mousePressAndHoldInterval
                                }
                            }
                        }

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Scrite"
                            icon.source: "qrc:/icons/exporter/scrite.png"
                            onClicked: saveFileDialog.launch("SAVE_AS")
                        }
                    }
                }
            }

            ToolButton3 {
                id: cmdReports
                iconSource: "../icons/reports/reports_menu_item.png"
                ToolTip.text: "Reports"
                checkable: false
                checked: false
                onClicked: reportsMenu.open()
                down: reportsMenu.visible
                // visible: scriteDocumentViewItem.width >= 1400 || !appToolBar.visible

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    Menu2 {
                        id: reportsMenu
                        width: 350

                        Repeater {
                            model: Scrite.document.supportedReports

                            MenuItem2 {
                                required property var modelData
                                text: modelData.name
                                icon.source: "qrc" + modelData.icon
                                onClicked: showReportWorkflow(modelData.name)

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
                color: ScriteRuntime.colors.primary.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                id: settingsAndShortcutsButton
                iconSource: "../icons/action/settings_applications.png"
                text: "Settings & Shortcuts"
                down: settingsAndShortcutsMenu.visible
                onClicked: settingsAndShortcutsMenu.visible = true

                Item {
                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    Menu2 {
                        id: settingsAndShortcutsMenu
                        width: 300

                        MenuItem2 {
                            id: settingsMenuItem
                            text: "Settings\t\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+,")
                            icon.source: "../icons/action/settings_applications.png"
                            onClicked: activate()
                            enabled: appToolBar.visible

                            function activate() {
                                modalDialog.popupSource = settingsAndShortcutsButton
                                modalDialog.sourceComponent = optionsDialogComponent
                                modalDialog.active = true
                            }

                            ShortcutsModelItem.group: "Application"
                            ShortcutsModelItem.title: "Settings"
                            ShortcutsModelItem.shortcut: "Ctrl+,"
                            ShortcutsModelItem.enabled: appToolBar.visible

                            Shortcut {
                                context: Qt.ApplicationShortcut
                                sequence: "Ctrl+,"
                                onActivated: settingsMenuItem.activate()
                            }
                        }

                        MenuItem2 {
                            id: shortcutsMenuItem
                            text: "Shortcuts\t\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+E")
                            icon.source: {
                                if(Scrite.app.isMacOSPlatform)
                                    return "../icons/navigation/shortcuts_macos.png"
                                if(Scrite.app.isWindowsPlatform)
                                    return "../icons/navigation/shortcuts_windows.png"
                                return "../icons/navigation/shortcuts_linux.png"
                            }
                            onClicked: activate()
                            enabled: appToolBar.visible

                            ShortcutsModelItem.group: "Application"
                            ShortcutsModelItem.title: shortcutsDockWidget.visible ? "Hide Shortcuts" : "Show Shortcuts"
                            ShortcutsModelItem.shortcut: "Ctrl+E"
                            ShortcutsModelItem.enabled: appToolBar.visible

                            function activate() {
                                shortcutsDockWidget.toggle()
                            }

                            Shortcut {
                                context: Qt.ApplicationShortcut
                                sequence: "Ctrl+E"
                                onActivated: shortcutsMenuItem.activate()
                            }
                        }

                        MenuItem2 {
                            icon.source: "../icons/action/help.png"
                            text: "Help\t\tF1"
                            onClicked: Qt.openUrlExternally(helpUrl)
                        }

                        MenuItem2 {
                            icon.source: "../icons/action/info.png"
                            text: "About"
                            onClicked: showAboutDialog()

                            Announcement.onIncoming: (type,data) => {
                                const stype = "" + type
                                const idata = data
                                if(stype === "72892ED6-BA58-47EC-B045-E92D9EC1C47A") {
                                    if(idata && typeof idata === "number")
                                        Utils.execLater(mainScriteDocumentView, idata, showAboutDialog)
                                    else
                                        showAboutDialog()
                                }
                            }

                            function showAboutDialog() {
                                modalDialog.sourceComponent = aboutBoxComponent
                                modalDialog.popupSource = parent
                                modalDialog.active = true
                            }
                        }

                        MenuItem2 {
                            text: "Toggle Fullscreen\tF7"
                            icon.source: "../icons/navigation/fullscreen.png"
                            onClicked: Utils.execLater(Scrite.app, 100, function() { Scrite.app.toggleFullscreen(Scrite.window) })
                            ShortcutsModelItem.group: "Application"
                            ShortcutsModelItem.title: "Toggle Fullscreen"
                            ShortcutsModelItem.shortcut: "F7"
                            Shortcut {
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
                color: ScriteRuntime.colors.primary.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                id: languageToolButton
                iconSource: "../icons/content/language.png"
                text: Scrite.app.transliterationEngine.languageAsString
                shortcut: "Ctrl+L"
                ToolTip.text: Scrite.app.polishShortcutTextForDisplay("Language Transliteration" + "\t" + shortcut)
                onClicked: languageMenu.visible = true
                down: languageMenu.visible
                visible: mainTabBar.currentIndex <= 2

                Item {
                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    Menu2 {
                        id: languageMenu
                        width: 250

                        Repeater {
                            model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                            MenuItem2 {
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
                                    ScriteRuntime.paragraphLanguageSettings.defaultLanguage = modelData.key
                                }
                            }
                        }

                        MenuSeparator {
                            focusPolicy: Qt.NoFocus
                        }

                        MenuItem2 {
                            text: "Next-Language\tF10"
                            focusPolicy: Qt.NoFocus
                            onClicked: {
                                Scrite.app.transliterationEngine.cycleLanguage()
                                Scrite.document.formatting.defaultLanguage = Scrite.app.transliterationEngine.language
                                ScriteRuntime.paragraphLanguageSettings.defaultLanguage = Scrite.app.transliterationEngine.languageAsString
                            }
                        }
                    }

                    Repeater {
                        model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                        Item {
                            Shortcut {
                                property string shortcutKey: Scrite.app.transliterationEngine.shortcutLetter(modelData.value)
                                context: Qt.ApplicationShortcut
                                sequence: "Alt+"+shortcutKey
                                onActivated: {
                                    Scrite.app.transliterationEngine.language = modelData.value
                                    Scrite.document.formatting.defaultLanguage = modelData.value
                                    ScriteRuntime.paragraphLanguageSettings.defaultLanguage = modelData.key
                                }
                                enabled: Scrite.app.transliterationEngine.enabledLanguages.indexOf(modelData.value) >= 0

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
                        onActivated: {
                            Scrite.app.transliterationEngine.cycleLanguage()
                            Scrite.document.formatting.defaultLanguage = Scrite.app.transliterationEngine.language
                            ScriteRuntime.paragraphLanguageSettings.defaultLanguage = Scrite.app.transliterationEngine.languageAsString
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

            ToolButton3 {
                iconSource: down ? "../icons/hardware/keyboard_hide.png" : "../icons/hardware/keyboard.png"
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
                                    color: ScriteRuntime.colors.primary.c300.background
                                    opacity: 0.9
                                    anchors.fill: parent

                                    Text {
                                        width: parent.width * 0.75
                                        font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize + 5
                                        anchors.centerIn: parent
                                        horizontalAlignment: Text.AlignHCenter
                                        color: ScriteRuntime.colors.primary.c300.text
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

            Text {
                id: languageDescLabel
                anchors.verticalCenter: parent.verticalCenter
                text: Scrite.app.transliterationEngine.languageAsString
                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
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

            ToolButton3 {
                text: "File"
                iconSource: "../icons/navigation/menu.png"
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
                    menu: Menu2 {
                        width: 300

                        MenuItem2 {
                            text: "Home"
                            onTriggered: showHomeScreen()
                        }

                        MenuItem2 {
                            text: "Save"
                            onTriggered: cmdSave.doClick()
                        }

                        MenuSeparator { }

                        Menu2 {
                            id: exportMenu2
                            title: "Share"
                            width: 250

                            Repeater {
                                model: Scrite.document.supportedExportFormats

                                MenuItem2 {
                                    required property var modelData
                                    text: modelData.name
                                    icon.source: "qrc" + modelData.icon
                                    onClicked: showExportWorkflow(modelData.key)
                                }
                            }

                            MenuSeparator { }

                            MenuItem2 {
                                text: "Scrite"
                                icon.source: "qrc:/icons/exporter/scrite.png"
                                onClicked: saveFileDialog.launch("SAVE_AS")
                            }
                        }

                        Menu2 {
                            title: "Reports"
                            width: 300

                            Repeater {
                                model: Scrite.document.supportedReports

                                MenuItem2 {
                                    required property var modelData
                                    text: modelData.name
                                    icon.source: "qrc" + modelData.icon
                                    onClicked: reportsMenu.itemAt(index).click()
                                    // enabled: scriteDocumentViewItem.width >= 800
                                }
                            }
                        }

                        MenuSeparator { }

                        Menu2 {
                            // FIXME: This is a duplicate of the languageMenu.
                            // We should remove this when we build an ActionManager.
                            title: "Language"

                            Repeater {
                                model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")

                                MenuItem2 {
                                    property string baseText: modelData.key
                                    property string shortcutKey: Scrite.app.transliterationEngine.shortcutLetter(modelData.value)
                                    text: baseText + " (" + Scrite.app.polishShortcutTextForDisplay("Alt+"+shortcutKey) + ")"
                                    font.bold: Scrite.app.transliterationEngine.language === modelData.value
                                    onClicked: {
                                        Scrite.app.transliterationEngine.language = modelData.value
                                        Scrite.document.formatting.defaultLanguage = modelData.value
                                        ScriteRuntime.paragraphLanguageSettings.defaultLanguage = modelData.key
                                    }
                                }
                            }

                            MenuSeparator { }

                            MenuItem2 {
                                text: "Next-Language (F10)"
                                onClicked: {
                                    Scrite.app.transliterationEngine.cycleLanguage()
                                    Scrite.document.formatting.defaultLanguage = Scrite.app.transliterationEngine.language
                                    ScriteRuntime.paragraphLanguageSettings.defaultLanguage = Scrite.app.transliterationEngine.languageAsString
                                }
                            }
                        }

                        MenuItem2 {
                            text: "Alphabet Mappings For " + Scrite.app.transliterationEngine.languageAsString
                            enabled: Scrite.app.transliterationEngine.language !== TransliterationEngine.English
                            onClicked: alphabetMappingsPopup.visible = !alphabetMappingsPopup.visible
                        }

                        MenuSeparator { }

                        Menu2 {
                            title: "View"
                            width: 250

                            MenuItem2 {
                                text: "Screenplay (" + Scrite.app.polishShortcutTextForDisplay("Alt+1") + ")"
                                onTriggered: mainTabBar.activateTab(0)
                                font.bold: mainTabBar.currentIndex === 0
                            }

                            MenuItem2 {
                                text: "Structure (" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
                                onTriggered: mainTabBar.activateTab(1)
                                font.bold: mainTabBar.currentIndex === 1
                            }

                            MenuItem2 {
                                text: "Notebook (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                                onTriggered: mainTabBar.activateTab(2)
                                font.bold: mainTabBar.currentIndex === 2
                                enabled: !showNotebookInStructure
                            }

                            MenuItem2 {
                                text: "Scrited (" + Scrite.app.polishShortcutTextForDisplay("Alt+4") + ")"
                                onTriggered: mainTabBar.currentIndex = 3
                                font.bold: mainTabBar.currentIndex === 3
                            }
                        }

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Settings"
                            // enabled: scriteDocumentViewItem.width >= 1100
                            onTriggered: settingsMenuItem.activate()
                        }

                        MenuItem2 {
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
                id: globalScreenplayEditorToolbar
                property Item sceneEditor
                anchors.verticalCenter: parent.verticalCenter
                binder: sceneEditor ? sceneEditor.binder : null
                editor: sceneEditor ? sceneEditor.editor : null
                visible: {
                    var min = 0
                    var max = showNotebookInStructure ? 1 : 2
                    return mainTabBar.currentIndex >= min && mainTabBar.currentIndex <= max
                }
            }

            Row {
                id: mainTabBar
                height: parent.height
                visible: appToolBar.visible

                readonly property var tabs: [
                    { "name": "Screenplay", "icon": "../icons/navigation/screenplay_tab.png", "visible": true, "tab": ScriteRuntime.e_ScreenplayTab },
                    { "name": "Structure", "icon": "../icons/navigation/structure_tab.png", "visible": true, "tab": ScriteRuntime.e_StructureTab },
                    { "name": "Notebook", "icon": "../icons/navigation/notebook_tab.png", "visible": !showNotebookInStructure, "tab": ScriteRuntime.e_NotebookTab },
                    { "name": "Scrited", "icon": "../icons/navigation/scrited_tab.png", "visible": ScriteRuntime.workspaceSettings.showScritedTab, "tab": ScriteRuntime.e_ScritedTab }
                ]
                readonly property color activeTabColor: ScriteRuntime.colors.primary.windowColor
                function indexOfTab(_ScriteRuntime_TabType) {
                    for(var i=0; i<tabs.length; i++) {
                        if(tabs[i].tab === _ScriteRuntime_TabType) {
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
                    if(index < 0 || index >= tabs.length || pdfViewer.active)
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
                    ScriteRuntime.mainWindowTab = tabs[currentIndex].tab
                }
                Component.onCompleted: {
                    ScriteRuntime.mainWindowTab = ScriteRuntime.e_ScreenplayTab
                    currentIndex = indexOfTab(ScriteRuntime.mainWindowTab)

                    const syncCurrentIndex = ()=>{
                        const idx = indexOfTab(ScriteRuntime.mainWindowTab)
                        if(currentIndex !== idx)
                            currentIndex = idx
                    }
                    ScriteRuntime.mainWindowTabChanged.connect( () => {
                                                                   Qt.callLater(syncCurrentIndex)
                                                               } )

                    ScriteRuntime.activateMainWindowTab.connect( (tabType) => {
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
                            fillColor: parent.active ? mainTabBar.activeTabColor : ScriteRuntime.colors.primary.c10.background
                            outlineColor: ScriteRuntime.colors.primary.borderColor
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
                            font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                        }

                        Image {
                            source: modelData.icon
                            width: parent.active ? 32 : 24; height: width
                            Behavior on width {
                                enabled: ScriteRuntime.applicationSettings.enableAnimations
                                NumberAnimation { duration: 250 }
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

        UserLogin {
            id: userLogin
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Loader {
        id: mainUiContentLoader
        active: allowContent && !Scrite.document.loading
        visible: !pdfViewer.active
        sourceComponent: uiLayoutComponent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: appToolBarArea.visible ? appToolBarArea.bottom : parent.top
        anchors.bottom: parent.bottom
        onActiveChanged: {
            globalScreenplayEditorToolbar.sceneEditor = null
        }

        property bool allowContent: true
        property string sessionId

        function reset(callback) {
            active = false
            Qt.callLater( (callback) => {
                             if(callback)
                                 callback()
                             mainUiContentLoader.active = true
                         }, callback )
        }
    }

    Rectangle {
        id: pdfViewerToolBar
        anchors.left: parent.left
        anchors.right: parent.right
        height: 53
        color: ScriteRuntime.colors.primary.c50.background
        visible: pdfViewer.active
        enabled: visible && !notificationsView.visible

        Text {
            text: pdfViewer.pdfTitle
            color: ScriteRuntime.colors.accent.c50.text
            elide: Text.ElideMiddle
            anchors.centerIn: parent
            width: parent.width * 0.8
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize + 2
            font.bold: true
        }

        Rectangle {
            width: parent.width
            height: 1
            color: ScriteRuntime.colors.primary.borderColor
            anchors.bottom: parent.bottom
        }
    }

    Loader {
        id: pdfViewer
        active: false
        visible: active
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: pdfViewerToolBar.visible ? pdfViewerToolBar.bottom : parent.top
        anchors.bottom: parent.bottom
        property string pdfFilePath
        property string pdfDownloadFilePath
        property string pdfTitle
        property int pdfPagesPerRow: 2
        property bool pdfSaveAllowed: true
        enabled: !notificationsView.visible
        onActiveChanged: {
            if(!active) {
                pdfPagesPerRow = 2
                pdfDownloadFilePath = ""
                pdfFilePath = ""
                pdfTitle = ""
            }
        }

        function show(title, filePath, dlFilePath, pagesPerRow, allowSave) {
            active = false
            pdfTitle = title
            pdfPagesPerRow = pagesPerRow
            pdfDownloadFilePath = dlFilePath
            pdfFilePath = filePath
            pdfSaveAllowed = allowSave === undefined ? true : allowSave
            Qt.callLater( function() {
                pdfViewer.active = true
            })
        }

        Connections {
            target: Scrite.document
            function onAboutToReset() {
                pdfViewer.active = false
            }
        }

        sourceComponent: PdfView {
            source: Scrite.app.localFileToUrl(pdfViewer.pdfFilePath)
            saveFilePath: pdfViewer.pdfDownloadFilePath
            allowFileSave: pdfViewer.pdfSaveAllowed
            saveFeatureDisabled: !pdfViewer.pdfSaveAllowed
            pagesPerRow: pdfViewer.pdfPagesPerRow
            allowFileReveal: false

            Component.onCompleted: forceActiveFocus()

            // While this PDF view is active, we don't want shortcuts to be
            // processed by any other part of the application.
            EventFilter.target: Scrite.app
            EventFilter.events: [EventFilter.KeyPress,EventFilter.Shortcut,EventFilter.ShortcutOverride]
            EventFilter.onFilter: (object,event,result) => {
                                      result.filter = true
                                      result.acceptEvent = true
                                      if(event.type === EventFilter.KeyPress) {
                                          if(event.key === Qt.Key_Escape) {
                                              pdfViewer.active = false
                                              return
                                          }
                                      }
                                  }

            FileManager {
                autoDeleteList: [pdfViewer.pdfFilePath]
            }

            onCloseRequest: pdfViewer.active = false
        }
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
                outlineColor: ScriteRuntime.colors.primary.borderColor
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

            // zoomLevelModifier: mainTabBar.currentIndex > 0 ? -3 : 0
            Component.onCompleted: {
                const evalZoomLevelModifierFn = () => {
                    screenplayFormat.pageLayout.evaluateRectsNow()

                    const pageLayout = screenplayFormat.pageLayout
                    const zoomLevels = screenplayFormat.fontZoomLevels
                    const indexOfZoomLevel = (val) => {
                        for(var i=0; i<zoomLevels.length; i++) {
                            if(zoomLevels[i] === val)
                                return i
                        }
                        return -1
                    }
                    const _oneValue = indexOfZoomLevel(1)

                    const availableWidth = mainTabBar.currentIndex === 0 ? width-500 : width
                    var _value = _oneValue
                    var zl = zoomLevels[_value]
                    var pageWidth = pageLayout.paperWidth * zl * Screen.devicePixelRatio
                    var totalMargin = availableWidth - pageWidth
                    if(totalMargin < 0) {
                        while(totalMargin < 20) { // 20 is width of vertical scrollbar.
                            if(_value-1 < 0)
                                break
                            _value = _value - 1
                            zl = zoomLevels[_value]
                            pageWidth = pageLayout.paperWidth * zl * Screen.devicePixelRatio
                            totalMargin = availableWidth - pageWidth
                        }
                    } else if(totalMargin > pageWidth/2) {
                        while(totalMargin > pageWidth/2) {
                            if(_value >= zoomLevels.length-1)
                                break
                            _value = _value + 1
                            zl = zoomLevels[_value]
                            pageWidth = pageLayout.paperWidth * zl * Screen.devicePixelRatio
                            totalMargin = availableWidth - pageWidth
                        }
                    }

                    return _value - _oneValue
                }

                if(ScriteRuntime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor)
                    zoomLevelModifier = evalZoomLevelModifierFn()
                else {
                    const zlms = ScriteRuntime.screenplayEditorSettings.zoomLevelModifiers
                    const zlm = zlms["tab"+mainTabBar.currentIndex]
                    if(zlm !== undefined)
                        zoomLevelModifier = zlm
                }

                trackZoomLevelChanges.enabled = true
            }

            Connections {
                id: trackZoomLevelChanges
                enabled: false
                target: screenplayEditor
                function onZoomLevelChanged() {
                    var zlms = ScriteRuntime.screenplayEditorSettings.zoomLevelModifiers
                    zlms["tab"+mainTabBar.currentIndex] = zoomLevelModifierToApply()
                    ScriteRuntime.screenplayEditorSettings.zoomLevelModifiers = zlms
                }
            }

            additionalCharacterMenuItems: {
                if(mainTabBar.currentIndex === 1) {
                    if(showNotebookInStructure)
                        return [
                                    {
                                        "name": "Character Notes",
                                        "description": "Create/switch to notes for the character in notebook",
                                        "icon": ":/icons/content/note.png"
                                    }
                                ]
                }
                return []
            }
            additionalSceneMenuItems: {
                if(mainTabBar.currentIndex === 1) {
                    if(showNotebookInStructure)
                        return ["Scene Notes"]
                }
                return []
            }
            Behavior on opacity {
                enabled: ScriteRuntime.applicationSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }

            enableSceneListPanel: mainTabBar.currentIndex === 0

            onAdditionalCharacterMenuItemClicked: (characterName,menuItemName) => {
                if(menuItemName === "Character Notes" && showNotebookInStructure) {
                    var ch = Scrite.document.structure.findCharacter(characterName)
                    if(ch === null)
                        Scrite.document.structure.addCharacter(characterName)
                    Announcement.shout("7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8", characterName)
                }
            }

            onAdditionalSceneMenuItemClicked: {
                if(menuItemName === "Scene Notes")
                    Announcement.shout("41EE5E06-FF97-4DB6-B32D-F938418C9529", undefined)
            }

            AttachmentsDropArea {
                id: fileOpenDropArea
                anchors.fill: parent
                enabled: !modalDialog.active
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
                }

                Loader {
                    id: fileOpenDropAreaNotification
                    anchors.fill: fileOpenDropArea
                    active: fileOpenDropArea.active || fileOpenDropArea.droppedFilePath !== ""
                    onActiveChanged: appToolBarArea.enabled = !active
                    Component.onDestruction: appToolBarArea.enabled = true
                    sourceComponent: Rectangle {
                        color: Scrite.app.translucent(ScriteRuntime.colors.primary.c500.background, 0.5)

                        Rectangle {
                            anchors.fill: fileOpenDropAreaNotice
                            anchors.margins: -30
                            radius: 4
                            color: ScriteRuntime.colors.primary.c700.background
                        }

                        Column {
                            id: fileOpenDropAreaNotice
                            anchors.centerIn: parent
                            width: parent.width * 0.5
                            spacing: 20

                            Text {
                                wrapMode: Text.WordWrap
                                width: parent.width
                                color: ScriteRuntime.colors.primary.c700.text
                                font.bold: true
                                text: fileOpenDropArea.active ? fileOpenDropArea.attachment.originalFileName : fileOpenDropArea.droppedFileName
                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                            }

                            Text {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                color: ScriteRuntime.colors.primary.c700.text
                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                                text: fileOpenDropArea.active ? "Drop the file here to open/import it." : "Do you want to open, import or cancel?"
                            }

                            Text {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                color: ScriteRuntime.colors.primary.c700.text
                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                                visible: !Scrite.document.empty || Scrite.document.fileName !== ""
                                text: "NOTE: Any unsaved changes in the currently open document will be discarded."
                            }

                            Row {
                                spacing: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                                visible: !Scrite.document.empty

                                Button2 {
                                    text: "Open/Import"
                                    onClicked: {
                                        Scrite.document.openOrImport(fileOpenDropArea.droppedFilePath)
                                        fileOpenDropArea.droppedFileName = ""
                                        fileOpenDropArea.droppedFilePath = ""
                                    }
                                }

                                Button2 {
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
            Material.background: Qt.darker(ScriteRuntime.colors.primary.windowColor, 1.1)

            Rectangle {
                id: structureEditorRow1
                SplitView.fillHeight: true
                color: ScriteRuntime.colors.primary.c10.background

                SplitView {
                    id: structureEditorSplitView2
                    orientation: Qt.Horizontal
                    Material.background: Qt.darker(ScriteRuntime.colors.primary.windowColor, 1.1)
                    anchors.fill: parent

                    Rectangle {
                        SplitView.fillWidth: true
                        SplitView.minimumWidth: 80
                        color: ScriteRuntime.colors.primary.c10.background
                        border {
                            width: showNotebookInStructure ? 0 : 1
                            color: ScriteRuntime.colors.primary.borderColor
                        }

                        Item {
                            id: structureEditorTabs
                            anchors.fill: parent
                            property int currentTabIndex: 0

                            Announcement.onIncoming: (type,data) => {
                                var sdata = "" + data
                                var stype = "" + type
                                if(showNotebookInStructure) {
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
                                    } else if(stype === "7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8") {
                                        structureEditorTabs.currentTabIndex = 1
                                        Utils.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToCharacterTab(data)
                                        })
                                    }
                                    else if(stype === "41EE5E06-FF97-4DB6-B32D-F938418C9529") {
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
                                // active: !ScriteAppFeatures.structure.enabled && ui.showNotebookInStructure
                                active: {
                                    if(structureEditorTabs.currentTabIndex === 0)
                                        return !ScriteAppFeatures.structure.enabled && mainScriteDocumentView.showNotebookInStructure
                                    else if(structureEditorTabs.currentTabIndex === 1)
                                        return !ScriteAppFeatures.notebook.enabled && mainScriteDocumentView.showNotebookInStructure
                                    return false
                                }
                                visible: active
                                sourceComponent: Rectangle {
                                    color: ScriteRuntime.colors.primary.c100.background
                                    width: appToolBar.height+4

                                    Column {
                                        anchors.horizontalCenter: parent.horizontalCenter

                                        ToolButton3 {
                                            down: structureEditorTabs.currentTabIndex === 0
                                            visible: mainScriteDocumentView.showNotebookInStructure
                                            iconSource: "../icons/navigation/structure_tab.png"
                                            ToolTip.text: "Structure\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
                                            onClicked: Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Structure")
                                        }

                                        ToolButton3 {
                                            down: structureEditorTabs.currentTabIndex === 1
                                            visible: mainScriteDocumentView.showNotebookInStructure
                                            iconSource: "../icons/navigation/notebook_tab.png"
                                            ToolTip.text: "Notebook Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                                            onClicked: Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Notebook")
                                        }
                                    }

                                    Rectangle {
                                        width: 1
                                        height: parent.height
                                        anchors.right: parent.right
                                        color: ScriteRuntime.colors.primary.borderColor
                                    }
                                }
                            }

                            Loader {
                                id: structureViewLoader
                                anchors.top: parent.top
                                anchors.left: structureEditorTabBar.active ? structureEditorTabBar.right : parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                visible: !showNotebookInStructure || structureEditorTabs.currentTabIndex === 0
                                active: ScriteAppFeatures.structure.enabled
                                sourceComponent: StructureView {
                                    HelpTipNotification {
                                        tipName: "structure"
                                        enabled: structureViewLoader.visible
                                    }
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
                                visible: showNotebookInStructure && structureEditorTabs.currentTabIndex === 1
                                active: visible && ScriteAppFeatures.notebook.enabled
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
                                color: Scrite.app.translucent(ScriteRuntime.colors.primary.button, 0.5)

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
                                        color: ScriteRuntime.colors.primary.windowColor
                                        visible: screenplayEditorHandleAnimation.running
                                    }

                                    Text {
                                        color: ScriteRuntime.colors.primary.c50.background
                                        text: "Pull this handle to view the screenplay editor."
                                        font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize + 2
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
                                        color: ScriteRuntime.colors.primary.windowColor
                                        visible: timelineViewHandleAnimation.running
                                    }

                                    Text {
                                        color: ScriteRuntime.colors.primary.c50.background
                                        text: "Pull this handle to get the timeline view."
                                        font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
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
                        SplitView.preferredWidth: mainScriteDocumentView.width * 0.5
                        SplitView.minimumWidth: 16
                        onWidthChanged: ScriteRuntime.workspaceSettings.screenplayEditorWidth = width
                        active: width >= 50
                        sourceComponent: mainTabBar.currentIndex === 1 ? screenplayEditorComponent : null

                        Rectangle {
                            visible: !parent.active
                            anchors.fill: parent
                            color: ScriteRuntime.colors.primary.c400.background
                        }
                    }
                }
            }

            Loader {
                id: structureEditorRow2
                SplitView.preferredHeight: 140 + ScriteRuntime.minimumFontMetrics.height*ScriteRuntime.screenplayTracks.trackCount
                SplitView.minimumHeight: 16
                SplitView.maximumHeight: SplitView.preferredHeight
                active: height >= 50
                sourceComponent: Rectangle {
                    color: FocusTracker.hasFocus ? ScriteRuntime.colors.accent.c100.background : ScriteRuntime.colors.accent.c50.background
                    FocusTracker.window: Scrite.window

                    Behavior on color {
                        enabled: ScriteRuntime.applicationSettings.enableAnimations
                        ColorAnimation { duration: 250 }
                    }

                    ScreenplayView {
                        anchors.fill: parent
                        showNotesIcon: showNotebookInStructure
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: Qt.rgba(0,0,0,0)
                        border { width: 1; color: ScriteRuntime.colors.accent.borderColor }
                    }
                }

                Rectangle {
                    visible: !parent.active
                    anchors.fill: parent
                    color: ScriteRuntime.colors.primary.c400.background
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

                if(ScriteRuntime.structureCanvasSettings.showPullHandleAnimation && mainUiContentLoader.sessionId !== Scrite.document.sessionId) {
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
            active: ScriteAppFeatures.notebook.enabled
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
            active: ScriteAppFeatures.scrited.enabled
            sourceComponent: ScritedView {

            }

            DisabledFeatureNotice {
                anchors.fill: parent
                visible: !parent.active
                featureName: "Scrited"
            }
        }
    }

    function showHomeScreenLater(ps, delay) {
        Utils.execLater(scriteDocumentViewItem, delay, () => {
                            showHomeScreen(ps)
                        })
    }

    function showHomeScreen(ps) {
        modalDialog.popupSource = ps === undefined ? homeButton : ps
        modalDialog.sourceComponent = homeScreenComponent
        modalDialog.closeable = true
        modalDialog.active = true
    }

    function showExportWorkflow(formatName) {
        if(formatName !== "") {
            modalDialog.closeable = false
            modalDialog.arguments = formatName
            modalDialog.sourceComponent = exporterConfigurationComponent
            modalDialog.popupSource = cmdExport
            modalDialog.active = true
        }
    }

    function showReportWorkflow(reportName) {
        if(reportName !== "") {
            modalDialog.closeable = false
            modalDialog.arguments = reportName
            modalDialog.sourceComponent = reportGeneratorConfigurationComponent
            modalDialog.popupSource = cmdReports
            modalDialog.active = true
        }
    }

    Component {
        id: homeScreenComponent

        HomeScreen { }
    }

    Component {
        id: aboutBoxComponent

        AboutBox { }
    }

    Component {
        id: optionsDialogComponent

        OptionsDialog { }
    }

    Item {
        property ErrorReport applicationErrors: Aggregation.findErrorReport(Scrite.app)
        Notification.active: applicationErrors ? applicationErrors.hasError : false
        Notification.title: "Scrite Error"
        Notification.text: applicationErrors ? applicationErrors.errorMessage : ""
        Notification.autoClose: false
    }

    Component {
        id: reportGeneratorConfigurationComponent

        ReportGeneratorConfiguration { }
    }

    Component {
        id: exporterConfigurationComponent

        ExporterConfiguration { }
    }

    Component {
        id: backupsDialogBoxComponent

        BackupsDialogBox {
            onOpenInThisWindow: {
                mainUiContentLoader.allowContent = false
                Scrite.document.openAnonymously(filePath)
                Utils.execLater(mainUiContentLoader, 50, function() {
                    mainUiContentLoader.allowContent = true
                    modalDialog.close()
                })
            }
            onOpenInNewWindow: {
                Scrite.app.launchNewInstanceAndOpenAnonymously(Scrite.window, filePath)
                Utils.execLater(modalDialog, 4000, function() {
                    modalDialog.close()
                })
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
                if(closeEventHandler.handleCloseEvent) {
                    Scrite.app.saveWindowGeometry(Scrite.window, "Workspace")

                    if(!Scrite.document.modified || Scrite.document.empty) {
                        close.accepted = true
                        return
                    }

                    if(Scrite.document.autoSave && Scrite.document.fileName !== "") {
                        Scrite.document.save()
                        close.accepted = true
                        return
                    }

                    close.accepted = false
                    askQuestion({
                        "question": "Do you want to save your current project before closing?",
                        "okButtonText": "Yes",
                        "cancelButtonText": "No",
                        "abortButtonText": "Cancel",
                        "callback": function(val) {
                            if(val) {
                                if(Scrite.document.fileName !== "")
                                    Scrite.document.save()
                                else {
                                    saveFileDialog.launch()
                                    return
                                }
                            }
                            closeEventHandler.handleCloseEvent = false
                            Scrite.window.close()
                        }
                    }, closeEventHandler)
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

    DockWidget {
        id: shortcutsDockWidget
        title: "Shortcuts"
        anchors.fill: parent
        contentX: ScriteRuntime.shortcutsDockWidgetSettings.contentX
        contentY: ScriteRuntime.shortcutsDockWidgetSettings.contentY
        contentWidth: 375
        contentHeight: (parent.height - appToolBarArea.height - 30) * 0.85
        visible: ScriteRuntime.shortcutsDockWidgetSettings.visible
        sourceItem: settingsAndShortcutsButton
        content: ShortcutsView { }
        onCloseRequest: hide()

        onContentXChanged: ScriteRuntime.shortcutsDockWidgetSettings.contentX = contentX
        onContentYChanged: ScriteRuntime.shortcutsDockWidgetSettings.contentY = contentY
        onVisibleChanged: ScriteRuntime.shortcutsDockWidgetSettings.visible = visible

        PropertyAlias {
            sourceObject: ScriteRuntime
            sourceProperty: "mainWindowTab"
            onValueChanged: {
                if(value !== ScriteRuntime.e_ScreenplayTab)
                    shortcutsDockWidget.hide()
            }
        }

        Connections {
            target: splashLoader
            function onActiveChanged() {
                if(splashLoader.active)
                    return
                if(shortcutsDockWidget.contentX < 0 || shortcutsDockWidget.contentX + shortcutsDockWidget.contentWidth > scriteDocumentViewItem.width)
                    shortcutsDockWidget.contentX = scriteDocumentViewItem.width - 40 - shortcutsDockWidget.contentWidth
                if(shortcutsDockWidget.contentY < 0 || shortcutsDockWidget.contentY + shortcutsDockWidget.contentHeight > scriteDocumentViewItem.height)
                    shortcutsDockWidget.contentY = (scriteDocumentViewItem.height - shortcutsDockWidget.contentHeight)/2
            }
        }
    }

    DockWidget {
        id: floatingDockWidget
        title: "Floating"
        anchors.fill: parent
        visible: false
        contentX: -1
        contentY: -1
        contentWidth: 375
        contentHeight: scriteDocumentViewItem.height * 0.6
        onCloseRequest: hide()

        function display(titleText, contentComponent) {
            title = titleText
            content = contentComponent
            show()
        }

        onVisibleChanged: {
            if(visible === false) {
                title = "Floating"
                content = null
            }
        }
    }

    Component.onCompleted: {
        if(!Scrite.app.restoreWindowGeometry(Scrite.window, "Workspace"))
            ScriteRuntime.workspaceSettings.screenplayEditorWidth = -1
        if(Scrite.app.maybeOpenAnonymously())
            splashLoader.active = false
        ScriteRuntime.screenplayAdapter.sessionId = Scrite.document.sessionId
        Qt.callLater( function() {
            Announcement.shout("{f4048da2-775d-11ec-90d6-0242ac120003}", "restoreWindowGeometryDone")
        })
    }

    BusyOverlay {
        id: appBusyOverlay
        anchors.fill: parent
        busyMessage: "Computing Page Layout, Evaluating Page Count & Time ..."
        visible: RefCounter.isReffed
        function ref() {
            RefCounter.ref()
        }
        function deref() {
            RefCounter.deref()
        }
    }

    HelpTipNotification {
        id: htNotification
        enabled: tipName !== ""

        Component.onCompleted: {
            Qt.callLater( () => {
                             if(ScriteRuntime.helpNotificationSettings.dayZero === "")
                                ScriteRuntime.helpNotificationSettings.dayZero = new Date()

                             const days = ScriteRuntime.helpNotificationSettings.daysSinceZero()
                             if(days >= 2) {
                                 if(!ScriteRuntime.helpNotificationSettings.isTipShown("discord"))
                                     htNotification.tipName = "discord"
                                 else if(!ScriteRuntime.helpNotificationSettings.isTipShown("subscription") && days >= 5)
                                     htNotification.tipName = "subscription"
                             }
                         })
        }
    }

    function openAnonymously(filePath, onCompleted) {
        mainUiContentLoader.allowContent = false
        Scrite.document.openAnonymously(filePath)
        Utils.execLater(mainUiContentLoader, 50, function() {
            mainUiContentLoader.allowContent = true
            if(onCompleted)
                onCompleted()
        })
    }

    FileDialog {
        id: saveFileDialog

        title: "Save Scrite Document As"
        nameFilters: ["Scrite Documents (*.scrite)"]
        selectFolder: false
        selectMultiple: false
        objectName: "Save File Dialog"
        dirUpAction.shortcut: "Ctrl+Shift+U"
        folder: ScriteRuntime.workspaceSettings.lastOpenFolderUrl
        onFolderChanged: ScriteRuntime.workspaceSettings.lastOpenFolderUrl = folder
        sidebarVisible: true
        selectExisting: false

        function launch(mode) {
            if(Scrite.document.empty)
                return

            if(mode === "SAVE_AS") {
                open()
                return
            }

            if(!Scrite.document.modified || Scrite.document.readOnly)
                 return;

            if(Scrite.document.fileName === "") {
                open()
                return
            }

            Scrite.document.save()

            const fileName = Scrite.document.fileName
            const fileInfo = Scrite.app.fileInfo(fileName)
            ScriteRuntime.recentFiles.add(fileInfo.filePath)
            return
        }

        onAccepted: {
            const path = Scrite.app.urlToLocalFile(fileUrl)
            Scrite.document.saveAs(path)

            ScriteRuntime.recentFiles.add(path)

            const fileInfo = Scrite.app.fileInfo(path)
            ScriteRuntime.workspaceSettings.lastOpenFolderUrl = folder
        }
    }

    QtObject {
        id: documentLoadErrors

        property ErrorReport errorReport: Aggregation.findErrorReport(Scrite.document)
        Notification.title: "Document Error"
        Notification.text: errorReport.errorMessage
        Notification.active: errorReport.hasError
        Notification.autoClose: false
        Notification.onDismissed: {
            if(errorReport.details && errorReport.details.revealOnDesktopRequest)
                Scrite.app.revealFileOnDesktop(errorReport.details.revealOnDesktopRequest)
            errorReport.clear()
        }
    }
}
