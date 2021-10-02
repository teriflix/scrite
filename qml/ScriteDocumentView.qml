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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.13
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

import Scrite 1.0

Item {
    id: documentUI
    width: 1350
    height: 700

    readonly property url helpUrl: "https://www.scrite.io/index.php/help/"

    enabled: !scriteDocument.loading

    FontMetrics {
        id: sceneEditorFontMetrics
        readonly property SceneElementFormat format: scriteDocument.formatting.elementFormat(SceneElement.Action)
        readonly property int lettersPerLine: globalScreenplayEditorToolbar.editInFullscreen ? 70 : 60
        readonly property int marginLetters: 5
        readonly property real paragraphWidth: Math.ceil(lettersPerLine*averageCharacterWidth)
        readonly property real paragraphMargin: Math.ceil(marginLetters*averageCharacterWidth)
        readonly property real pageWidth: Math.ceil(paragraphWidth + 2*paragraphMargin)
        font: format ? format.font2 : scriteDocument.formatting.defaultFont2
    }

    property bool canShowNotebookInStructure: width > 1600
    property bool showNotebookInStructure: workspaceSettings.showNotebookInStructure && canShowNotebookInStructure
    onShowNotebookInStructureChanged: {
        app.execLater(workspaceSettings, 100, function() {
            mainTabBar.currentIndex = mainTabBar.currentIndex % (showNotebookInStructure ? 2 : 3)
        })
    }

    Settings {
        id: workspaceSettings
        fileName: app.settingsFilePath
        category: "Workspace"
        property real workspaceHeight
        property real screenplayEditorWidth: -1
        property bool scriptalayIntroduced: false
        property bool showNotebookInStructure: true
        property bool syncCurrentSceneOnNotebook: true
        property bool animateStructureIcon: true
        property bool animateNotebookIcon: true
        property real flickScrollSpeedFactor: 1.0
        property bool showScritedTab: false
        property bool mouseWheelZoomsInCharacterGraph: app.isWindowsPlatform || app.isLinuxPlatform
        property bool mouseWheelZoomsInStructureCanvas: app.isWindowsPlatform || app.isLinuxPlatform
        property string lastOpenFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenPhotosFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        property string lastOpenImportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenExportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenReportsFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        property var customColors: []
    }

    Settings {
        id: screenplayEditorSettings
        fileName: app.settingsFilePath
        category: "Screenplay Editor"
        property bool displayRuler: true
        property bool displaySceneCharacters: true
        property bool displaySceneSynopsis: true
        property bool displaySceneComments: false
        property int mainEditorZoomValue: -1
        property int embeddedEditorZoomValue: -1
        property bool includeTitlePageInPreview: true
        property bool enableSpellCheck: false // until we can fix https://github.com/teriflix/scrite/issues/138
        property bool enableAnimations: true
        property int lastLanguageRefreshNoticeBoxTimestamp: 0
        property int lastSpellCheckRefreshNoticeBoxTimestamp: 0
        property bool showLanguageRefreshNoticeBox: true
        property bool showSpellCheckRefreshNoticeBox: true
        property bool showLoglineEditor: false
        property bool allowTaggingOfScenes: false
        property real spaceBetweenScenes: 0
        onEnableAnimationsChanged: {
            modalDialog.animationsEnabled = enableAnimations
            statusText.enableAnimations = enableAnimations
        }

        property real textFormatDockWidgetX: -1
        property real textFormatDockWidgetY: -1

        property bool pausePageAndTimeComputation: false
        property bool highlightCurrentLine: true
    }

    Settings {
        id: paragraphLanguageSettings
        fileName: app.settingsFilePath
        category: "Paragraph Language"

        property string shotLanguage: "Default"
        property string actionLanguage: "Default"
        property string defaultLanguage: "English"
        property string dialogueLanguage: "Default"
        property string characterLanguage: "Default"
        property string transitionLanguage: "Default"
        property string parentheticalLanguage: "Default"
    }

    Settings {
        id: notebookSettings
        fileName: app.settingsFilePath
        category: "Notebook"
        property int activeTab: 0 // 0 = Relationships, 1 = Notes
        property int graphLayoutMaxTime: 1000
        property int graphLayoutMaxIterations: 50000
        property bool showAllFormQuestions: true
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+P"

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Export To PDF"
        ShortcutsModelItem.shortcut: sequence
        onActivated: exportTimer.formatName = "Screenplay/Adobe PDF"
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
        ShortcutsModelItem.title: screenplayEditorSettings.displaySceneCharacters ? "Hide Scene Characters, Tags" : "Show Scene Characters, Tags"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.displaySceneCharacters = !screenplayEditorSettings.displaySceneCharacters
    }

    Shortcut {
        id: synopsisToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+S"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.displaySceneSynopsis ? "Hide Synopsis" : "Show Synopsis"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.displaySceneSynopsis = !screenplayEditorSettings.displaySceneSynopsis
    }

    Shortcut {
        id: commentsToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+M"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.displaySceneComments ? "Hide Comments" : "Show Comments"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.displaySceneComments = !screenplayEditorSettings.displaySceneComments
    }

    Shortcut {
        id: taggingToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+G"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.allowTaggingOfScenes ? "Allow Tagging" : "Disable Tagging"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.allowTaggingOfScenes = !screenplayEditorSettings.allowTaggingOfScenes
    }

    Shortcut {
        id: spellCheckToggleShortcut
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+L"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.enableSpellCheck ? "Disable Spellcheck" : "Enable Spellcheck"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.enableSpellCheck = !screenplayEditorSettings.enableSpellCheck
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+A"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.enableAnimations ? "Disable Animations" : "Enable Animations"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.enableAnimations = !screenplayEditorSettings.enableAnimations
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+H"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.highlightCurrentLine ? "Line Highlight Off" : "Line Highlight On"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.highlightCurrentLine = !screenplayEditorSettings.highlightCurrentLine
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+M"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "New Scrite Window"
        ShortcutsModelItem.enabled: true
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.visible: enabled
        onActivated: app.launchNewInstance(qmlWindow)
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
                    app.execLater(mainTabBar, 250, function() {
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
                app.execLater(mainTabBar, 250, function() {
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
        enabled: workspaceSettings.showScritedTab
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Scrited"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.enabled: enabled
        onActivated: mainTabBar.activateTab(3)
    }

    Connections {
        target: scriteDocument
        onJustReset: {
            appBusyOverlay.refCount = appBusyOverlay.refCount+1
            screenplayAdapter.initialLoadTreshold = 25
            app.execLater(screenplayAdapter, 250, function() {
                screenplayAdapter.sessionId = scriteDocument.sessionId
                appBusyOverlay.refCount = Math.max(appBusyOverlay.refCount-1,0)
            })
        }
        onJustLoaded: {
            var firstElement = scriteDocument.screenplay.elementAt(scriteDocument.screenplay.firstSceneIndex())
            if(firstElement) {
                var editorHints = firstElement.editorHints
                if(editorHints) {
                    screenplayAdapter.initialLoadTreshold = -1
                    screenplayEditorSettings.displaySceneCharacters = editorHints.displaySceneCharacters
                    screenplayEditorSettings.displaySceneSynopsis = editorHints.displaySceneSynopsis
                    return
                }
            }
        }
    }

    ScreenplayAdapter {
        id: screenplayAdapter
        onSourceChanged: globalScreenplayEditorToolbar.showScreenplayPreview = false
        property string sessionId
        source: {
            if(scriteDocument.sessionId !== sessionId)
                return null

            if(mainTabBar.currentIndex === 0)
                return scriteDocument.screenplay

            if(scriteDocument.screenplay.currentElementIndex < 0) {
                var index = scriteDocument.structure.currentElementIndex
                var element = scriteDocument.structure.elementAt(index)
                if(element) {
                    if(element.scene.addedToScreenplay) {
                        scriteDocument.screenplay.currentElementIndex = element.scene.screenplayElementIndexList[0]
                        return scriteDocument.screenplay
                    }
                    return element.scene
                }
            }

            return scriteDocument.screenplay
        }
    }

    ScreenplayTextDocument {
        id: screenplayTextDocument
        screenplay: scriteDocument.loading || paused ? null : (editor ? screenplayAdapter.screenplay : null)
        formatting: scriteDocument.loading || paused ? null : (editor ? scriteDocument.printFormat : null)
        property bool paused: screenplayEditorSettings.pausePageAndTimeComputation
        onPausedChanged: Qt.callLater( function() {
            screenplayEditorSettings.pausePageAndTimeComputation = screenplayTextDocument.paused
        })
        syncEnabled: true
        sceneNumbers: false
        titlePage: false
        sceneIcons: false
        listSceneCharacters: false
        includeSceneSynopsis: false
        printEachSceneOnANewPage: false
        secondsPerPage: scriteDocument.printFormat.secondsPerPage
        property Item editor
        property bool overlayRefCountModified: false
        onUpdateScheduled: {
            if(mainUndoStack.screenplayEditorActive || mainUndoStack.sceneEditorActive) {
                appBusyOverlay.refCount = appBusyOverlay.refCount+1
                overlayRefCountModified = true
            }
        }
        onUpdateFinished: {
            if(overlayRefCountModified)
                appBusyOverlay.refCount = Math.max(appBusyOverlay.refCount-1,0)
            overlayRefCountModified = false
        }
        Component.onCompleted: app.registerObject(screenplayTextDocument, "screenplayTextDocument")
    }

    Rectangle {
        id: appToolBarArea
        anchors.left: parent.left
        anchors.right: parent.right
        height: 53
        color: primaryColors.c50.background

        Row {
            id: appToolBar
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 5
            visible: appToolBarArea.width >= 1200
            onVisibleChanged: {
                if(!visible)
                    mainTabBar.activateTab(0)
            }

            function saveQuestionText() {
                if(scriteDocument.fileName === "")
                    return "Do you want to save this document first?"
                return "Do you want to save changes to <strong>" + app.fileName(scriteDocument.fileName) + "</strong> first?"
            }

            spacing: documentUI.width >= 1440 ? 2 : 0

            ToolButton3 {
                id: fileNewButton
                iconSource: "../icons/action/description.png"
                text: "New"
                shortcut: "Ctrl+N"
                shortcutText: "N"
                onClicked: {
                    if(scriteDocument.modified)
                        askQuestion({
                            "question": appToolBar.saveQuestionText(),
                            "okButtonText": "Yes",
                            "cancelButtonText": "No",
                            "abortButtonText": "Cancel",
                            "callback": function(val) {
                                if(val) {
                                    if(scriteDocument.fileName !== "")
                                        scriteDocument.save()
                                    else {
                                        cmdSave.doClick()
                                        return
                                    }
                                }
                                contentLoader.allowContent = false
                                scriteDocument.reset()
                                contentLoader.allowContent = true
                                app.execLater(fileNewButton, 250, newFromTemplate)
                            }
                        }, fileNewButton)
                    else
                        newFromTemplate()
                }

                ShortcutsModelItem.group: "File"
                ShortcutsModelItem.title: text
                ShortcutsModelItem.shortcut: shortcut
            }

            ToolButton3 {
                id: fileOpenButton
                iconSource: "../icons/file/folder_open.png"
                text: "Open"
                shortcut: "Ctrl+O"
                shortcutText: "O"
                down: recentFilesMenu.visible
                onClicked: recentFilesMenu.open()

                function doOpen(filePath) {
                    if(filePath === scriteDocument.fileName)
                        return

                    if(scriteDocument.modified)
                        askQuestion({
                                "question": appToolBar.saveQuestionText(),
                                "okButtonText": "Yes",
                                "cancelButtonText": "No",
                                "abortButtonText": "Cancel",
                                "callback": function(val) {
                                    if(val) {
                                        if(scriteDocument.fileName !== "")
                                            scriteDocument.save()
                                        else {
                                            cmdSave.doClick()
                                            return
                                        }
                                    }
                                    recentFilesMenu.close()
                                    if(filePath === "#TEMPLATE")
                                        app.execLater(fileOpenButton, 250, newFromTemplate)
                                    else
                                        fileDialog.launch("OPEN", filePath)
                                }
                            }, fileOpenButton)
                    else {
                        recentFilesMenu.close()
                        if(filePath === "#TEMPLATE")
                            app.execLater(fileOpenButton, 250, newFromTemplate)
                        else
                            fileDialog.launch("OPEN", filePath)
                    }
                }

                Connections {
                    target: app
                    onOpenFileRequest: fileOpenButton.doOpen(filePath)
                }

                ShortcutsModelItem.group: "File"
                ShortcutsModelItem.title: text
                ShortcutsModelItem.shortcut: shortcut

                Item {
                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    Settings {
                        fileName: app.settingsFilePath
                        category: "RecentFiles"
                        property alias files: recentFilesMenu.recentFiles
                    }

                    Menu2 {
                        id: recentFilesMenu
                        width: recentFiles.length > 1 ? 400 : 200

                        Connections {
                            target: scriteDocument
                            onJustLoaded: recentFilesMenu.add(scriteDocument.fileName)
                        }

                        property int nrRecentFiles: recentFiles.length
                        property var recentFiles: []
                        function add(filePath) {
                            if(filePath === "")
                                return
                            var r = recentFiles
                            if(r.length > 0 && r[r.length-1] === filePath)
                                return
                            for(var i=0; i<r.length; i++) {
                                if(r[i] === filePath)
                                    r.splice(i,1);
                            }
                            r.push(filePath)
                            if(r.length > 10)
                                r.splice(0, r.length-10)
                            recentFiles = r
                        }

                        function prepareRecentFilesList() {
                            var newFiles = []
                            var filesDropped = false
                            recentFilesMenu.recentFiles.forEach(function(filePath) {
                                var fi = app.fileInfo(filePath)
                                if(fi.exists)
                                    newFiles.push(filePath)
                                else
                                    filesDropped = true
                            })
                            if(filesDropped)
                                recentFilesMenu.recentFiles = newFiles
                        }

                        onAboutToShow: prepareRecentFilesList()

                        MenuItem2 {
                            text: "Open..."
                            onClicked: fileOpenButton.doOpen()

                            Rectangle {
                                width: parent.width; height: 1
                                anchors.bottom: parent.bottom
                                color: primaryColors.borderColor
                            }
                        }

                        FontMetrics {
                            id: recentFilesFontMetrics
                        }

                        Repeater {
                            model: recentFilesMenu.recentFiles

                            MenuItem2 {
                                property string filePath: recentFilesMenu.recentFiles[recentFilesMenu.nrRecentFiles-index-1]
                                property var fileInfo: app.fileInfo(filePath)
                                text: recentFilesFontMetrics.elidedText(fileInfo.baseName, Qt.ElideMiddle, recentFilesMenu.width)
                                ToolTip.text: filePath
                                ToolTip.visible: hovered
                                onClicked: fileOpenButton.doOpen(filePath)
                            }
                        }
                    }
                }
            }

            ToolButton3 {
                id: backupOpenButton
                iconSource: "../icons/file/backup_open.png"
                text: "Open Backup"
                visible: scriteDocument.backupFilesModel.count > 0
                onClicked: {
                    modalDialog.closeable = false
                    modalDialog.closeOnEscape = true
                    modalDialog.popupSource = backupOpenButton
                    modalDialog.sourceComponent = backupsDialogBoxComponent
                    modalDialog.active = true
                }

                ToolTip.text: "Open any of the " + scriteDocument.backupFilesModel.count + " backup(s) available for this file."

                Text {
                    id: backupCountHint
                    font.pixelSize: parent.height * 0.2
                    font.bold: true
                    text: scriteDocument.backupFilesModel.count
                    padding: 2
                    color: primaryColors.highlight.text
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                }
            }

            ToolButton3 {
                id: cmdSave
                iconSource: "../icons/content/save.png"
                text: "Save"
                shortcut: "Ctrl+S"
                shortcutText: "S"
                enabled: scriteDocument.modified && !scriteDocument.readOnly
                onClicked: doClick()
                function doClick() {
                    if(scriteDocument.fileName === "")
                        fileDialog.launch("SAVE")
                    else {
                        fileDialog.mode = "SAVE"
                        scriteDocument.save()
                    }
                }

                ShortcutsModelItem.group: "File"
                ShortcutsModelItem.title: text
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: shortcut
            }

            ToolButton3 {
                text: "Save As"
                shortcut: "Ctrl+Shift+S"
                shortcutText: "Shift+S"
                iconSource: "../icons/content/save_as.png"
                onClicked: fileDialog.launch("SAVE")
                enabled: scriteDocument.structure.elementCount > 0 ||
                         scriteDocument.structure.noteCount > 0 ||
                         scriteDocument.structure.annotationCount > 0 ||
                         scriteDocument.screenplay.elementCount > 0
                ShortcutsModelItem.group: "File"
                ShortcutsModelItem.title: text
                ShortcutsModelItem.shortcut: shortcut
            }

            ToolButton3 {
                id: openFromLibrary
                iconSource: "../icons/action/library.png"
                text: "<img src=\"qrc:/images/library_woicon_inverted.png\" height=\"30\" width=\"107\">\t&nbsp;"
                shortcut: "Ctrl+Shift+O"
                shortcutText: "Shift+O"
                function go() {
                    modalDialog.closeable = false
                    modalDialog.popupSource = openFromLibrary
                    modalDialog.sourceComponent = openFromLibraryComponent
                    modalDialog.active = true
                }

                onClicked: {
                    if(scriteDocument.modified)
                        askQuestion({
                            "question": appToolBar.saveQuestionText(),
                            "okButtonText": "Yes",
                            "cancelButtonText": "No",
                            "abortButtonText": "Cancel",
                            "callback": function(val) {
                                if(val) {
                                    if(scriteDocument.fileName !== "")
                                        scriteDocument.save()
                                    else {
                                        cmdSave.doClick()
                                        return
                                    }
                                }
                                app.execLater(openFromLibrary, 250, function() { openFromLibrary.go() })
                            }
                        }, fileNewButton)
                    else
                        openFromLibrary.go()
                }

                ShortcutsModelItem.group: "File"
                ShortcutsModelItem.title: "<img src=\"qrc:/images/library_woicon.png\" height=\"30\" width=\"107\">"
                ShortcutsModelItem.shortcut: shortcut
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
                opacity: 0.5
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
                ShortcutsModelItem.enabled: app.canUndo && !scriteDocument.readOnly // enabled
                ShortcutsModelItem.shortcut: "Ctrl+Z" // shortcut
            }

            QtObject {
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Redo"
                ShortcutsModelItem.enabled: app.canRedo && !scriteDocument.readOnly // enabled
                ShortcutsModelItem.shortcut: app.isMacOSPlatform ? "Ctrl+Shift+Z" : "Ctrl+Y" // shortcut
            }

            ToolButton3 {
                id: importExportButton
                iconSource: "../icons/file/import_export.png"
                text: "Import, Export & Reports"
                onClicked: importExportMenu.visible = true
                down: importExportMenu.visible

                Item {
                    anchors.top: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right

                    Menu2 {
                        id: importExportMenu

                        Menu2 {
                            id: importMenu
                            title: "Import"

                            Repeater {
                                model: scriteDocument.supportedImportFormats

                                MenuItem2 {
                                    id: importMenuItem
                                    text: modelData
                                    onClicked: click()
                                    function click() {
                                        if(scriteDocument.modified)
                                            askQuestion({
                                                    "question": "Do you want to save your current project first?",
                                                    "okButtonText": "Yes",
                                                    "cancelButtonText": "No",
                                                    "callback": function(val) {
                                                        if(val) {
                                                            if(scriteDocument.fileName !== "")
                                                                scriteDocument.save()
                                                            else {
                                                                cmdSave.doClick()
                                                                return
                                                            }
                                                        }
                                                        fileDialog.launch("IMPORT " + modelData)
                                                    }
                                                }, importMenuItem)
                                        else
                                            fileDialog.launch("IMPORT " + modelData)
                                    }
                                }
                            }
                        }

                        Menu2 {
                            id: exportMenu
                            title: "Export"
                            width: 250

                            Component {
                                id: menuItemComponent
                                MenuItem2 {
                                    property string format
                                    text: {
                                        var fields = format.split("/")
                                        return fields[fields.length-1]
                                    }
                                    function click() { exportTimer.formatName = format }
                                    onClicked: click()
                                }
                            }

                            Component {
                                id: menuSeparatorComponent
                                MenuSeparator { }
                            }

                            Component.onCompleted: {
                                var formats = scriteDocument.supportedExportFormats
                                for(var i=0; i<formats.length; i++) {
                                    var format = formats[i]
                                    if(format === "")
                                        exportMenu.addItem(menuSeparatorComponent.createObject(exportMenu))
                                    else
                                        exportMenu.addItem(menuItemComponent.createObject(exportMenu, {"format": format}))
                                }
                            }
                        }

                        Menu2 {
                            id: reportsMenu
                            title: "Reports"
                            enabled: scriteDocument.screenplay.elementCount > 0
                            width: 300

                            Repeater {
                                model: scriteDocument.supportedReports

                                MenuItem2 {
                                    leftPadding: 15
                                    rightPadding: 15
                                    topPadding: 5
                                    bottomPadding: 5
                                    width: reportsMenu.width
                                    height: 65
                                    contentItem: Column {
                                        id: menuContent
                                        width: reportsMenu.width - 30
                                        spacing: 5

                                        Text {
                                            font.bold: true
                                            font.pixelSize: 16
                                            text: modelData.name
                                        }

                                        Text {
                                            text: modelData.description
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                            font.pixelSize: 12
                                            font.italic: true
                                        }
                                    }

                                    function click() {
                                        reportGeneratorTimer.reportArgs = modelData.name
                                    }

                                    onTriggered: click()
                                }
                            }
                        }
                    }

                    Timer {
                        id: exportTimer
                        objectName: "ScriteDocumentView.exportTimer"
                        property string formatName
                        repeat: false
                        interval: 10
                        onFormatNameChanged: {
                            if(formatName !== "")
                                start()
                        }
                        onTriggered: {
                            if(formatName !== "") {
                                modalDialog.closeable = false
                                modalDialog.arguments = formatName
                                modalDialog.sourceComponent = exporterConfigurationComponent
                                modalDialog.popupSource = importExportButton
                                modalDialog.active = true
                            }
                            formatName = ""
                        }
                    }

                    Timer {
                        id: reportGeneratorTimer
                        objectName: "ScriteDocumentView.reportGeneratorTimer"
                        property var reportArgs
                        property Item requestSource
                        repeat: false
                        interval: 10
                        onReportArgsChanged: {
                            if(reportArgs !== "")
                                start()
                        }
                        onTriggered: {
                            if(reportArgs !== "") {
                                modalDialog.closeable = false
                                modalDialog.arguments = reportArgs
                                modalDialog.sourceComponent = reportGeneratorConfigurationComponent
                                modalDialog.popupSource = requestSource === null ? importExportButton : requestSource
                                modalDialog.active = true
                            }
                            reportArgs = ""
                            requestSource = null
                        }
                    }
                }
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
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
                            text: "Settings\t\t" + app.polishShortcutTextForDisplay("Ctrl+,")
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
                            text: "Shortcuts\t\t" + app.polishShortcutTextForDisplay("Ctrl+E")
                            icon.source: {
                                if(app.isMacOSPlatform)
                                    return "../icons/navigation/shortcuts_macos.png"
                                if(app.isWindowsPlatform)
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
                            text: "Toggle Fullscreen\tF7"
                            icon.source: "../icons/navigation/fullscreen.png"
                            onClicked: app.execLater(app, 100, function() { app.toggleFullscreen(qmlWindow) })
                            ShortcutsModelItem.group: "Application"
                            ShortcutsModelItem.title: "Toggle Fullscreen"
                            ShortcutsModelItem.shortcut: "F7"
                            Shortcut {
                                context: Qt.ApplicationShortcut
                                sequence: "F7"
                                onActivated: app.execLater(app, 100, function() { app.toggleFullscreen(qmlWindow) })
                            }
                        }

                        MenuSeparator {

                        }

                        Menu2 {
                            id: sceneCharactersMenu
                            width: 400
                            title: "Screenplay Options"

                            MenuItem2 {
                                icon.source: "../icons/content/blank.png"
                                text: "Scan For Mute Characters"
                                onClicked: scriteDocument.structure.scanForMuteCharacters()
                                enabled: !scriteDocument.readOnly && screenplayEditorSettings.displaySceneCharacters
                            }

                            MenuSeparator {  }

                            MenuItem2 {
                                text: "Logline Editor"
                                icon.source: screenplayEditorSettings.showLoglineEditor ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                                onTriggered: screenplayEditorSettings.showLoglineEditor = !screenplayEditorSettings.showLoglineEditor
                            }

                            MenuItem2 {
                                text: "Scene Synopsis\t\t" + app.polishShortcutTextForDisplay(synopsisToggleShortcut.ShortcutsModelItem.shortcut)
                                icon.source: screenplayEditorSettings.displaySceneSynopsis && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                                onTriggered: screenplayEditorSettings.displaySceneSynopsis = !screenplayEditorSettings.displaySceneSynopsis
                            }

                            MenuItem2 {
                                text: "Scene Comments\t\t" + app.polishShortcutTextForDisplay(commentsToggleShortcut.ShortcutsModelItem.shortcut)
                                icon.source: screenplayEditorSettings.displaySceneComments && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                                onTriggered: screenplayEditorSettings.displaySceneComments = !screenplayEditorSettings.displaySceneComments
                            }

                            MenuItem2 {
                                text: "Scene Characters and Tags\t" + app.polishShortcutTextForDisplay(sceneCharactersToggleShortcut.ShortcutsModelItem.shortcut)
                                icon.source: screenplayEditorSettings.displaySceneCharacters ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                                onTriggered: screenplayEditorSettings.displaySceneCharacters = !screenplayEditorSettings.displaySceneCharacters
                            }

                            MenuItem2 {
                                text: "Enable Tagging Of Scenes\t" +app.polishShortcutTextForDisplay(taggingToggleShortcut.ShortcutsModelItem.shortcut)
                                icon.source: screenplayEditorSettings.allowTaggingOfScenes && enabled ? "../icons/navigation/check.png" : "../icons/content/blank.png"
                                onTriggered: screenplayEditorSettings.allowTaggingOfScenes = !screenplayEditorSettings.allowTaggingOfScenes
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                id: languageToolButton
                iconSource: "../icons/content/language.png"
                text: app.transliterationEngine.languageAsString
                shortcut: "Ctrl+L"
                shortcutText: "L"
                ToolTip.text: app.polishShortcutTextForDisplay("Language Transliteration" + "\t" + shortcut)
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
                            model: app.enumerationModel(app.transliterationEngine, "Language")

                            MenuItem2 {
                                property string baseText: modelData.key
                                property string shortcutKey: app.transliterationEngine.shortcutLetter(modelData.value)
                                text: baseText + " (" + app.polishShortcutTextForDisplay("Alt+"+shortcutKey) + ")"
                                font.bold: app.transliterationEngine.language === modelData.value
                                onClicked: {
                                    app.transliterationEngine.language = modelData.value
                                    scriteDocument.formatting.defaultLanguage = modelData.value
                                    paragraphLanguageSettings.defaultLanguage = modelData.key
                                }
                            }
                        }

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Next-Language (F10)"
                            onClicked: {
                                app.transliterationEngine.cycleLanguage()
                                scriteDocument.formatting.defaultLanguage = app.transliterationEngine.language
                                paragraphLanguageSettings.defaultLanguage = app.transliterationEngine.languageAsString
                            }
                        }
                    }

                    Repeater {
                        model: app.enumerationModel(app.transliterationEngine, "Language")

                        Item {
                            Shortcut {
                                property string shortcutKey: app.transliterationEngine.shortcutLetter(modelData.value)
                                context: Qt.ApplicationShortcut
                                sequence: "Alt+"+shortcutKey
                                onActivated: {
                                    app.transliterationEngine.language = modelData.value
                                    scriteDocument.formatting.defaultLanguage = modelData.value
                                    paragraphLanguageSettings.defaultLanguage = modelData.key
                                }

                                ShortcutsModelItem.priority: 0
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
                            app.transliterationEngine.cycleLanguage()
                            scriteDocument.formatting.defaultLanguage = app.transliterationEngine.language
                            paragraphLanguageSettings.defaultLanguage = app.transliterationEngine.languageAsString
                        }

                        ShortcutsModelItem.priority: 1
                        ShortcutsModelItem.title: "Next Language"
                        ShortcutsModelItem.group: "Language"
                        ShortcutsModelItem.shortcut: "F10"
                    }
                }
            }

            ToolButton3 {
                iconSource: down ? "../icons/hardware/keyboard_hide.png" : "../icons/hardware/keyboard.png"
                ToolTip.text: "Show English to " + app.transliterationEngine.languageAsString + " alphabet mappings.\t" + app.polishShortcutTextForDisplay(shortcut)
                shortcut: "Ctrl+K"
                shortcutText: "K"
                onClicked: alphabetMappingsPopup.visible = !alphabetMappingsPopup.visible
                down: alphabetMappingsPopup.visible
                enabled: app.transliterationEngine.language !== TransliterationEngine.English
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
                                enabled: app.transliterationEngine.textInputSourceIdForLanguage(app.transliterationEngine.language) === ""

                                Rectangle {
                                    visible: !parent.enabled
                                    color: primaryColors.c300.background
                                    opacity: 0.9
                                    anchors.fill: parent

                                    Text {
                                        width: parent.width * 0.75
                                        font.pointSize: app.idealFontPointSize + 5
                                        anchors.centerIn: parent
                                        horizontalAlignment: Text.AlignHCenter
                                        color: primaryColors.c300.text
                                        text: {
                                            if(app.isMacOSPlatform)
                                                return "Scrite is using an input source from macOS while typing in " + app.transliterationEngine.languageAsString + "."
                                            return "Scrite is using an input method & keyboard layout from Windows while typing in " + app.transliterationEngine.languageAsString + "."
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: documentUI.width > 1470 ? fullText : fullText.substring(0, 2)
                font.pointSize: app.idealFontPointSize-2
                property string fullText: app.transliterationEngine.languageAsString
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

                        Menu2 {
                            title: "File"

                            MenuItem2 {
                                text: "New"
                                onTriggered: fileNewButton.click()
                            }

                            MenuItem2 {
                                text: "Open"
                                onTriggered: fileOpenButton.doOpen()
                            }


                            Menu2 {
                                title: "Recent"
                                onAboutToShow: recentFilesMenu.prepareRecentFilesList()
                                width: Math.min(documentUI.width * 0.75, 350)

                                Repeater {
                                    model: recentFilesMenu.recentFiles

                                    MenuItem2 {
                                        property string filePath: recentFilesMenu.recentFiles[recentFilesMenu.recentFiles.length-index-1]
                                        text: recentFilesFontMetrics.elidedText("" + (index+1) + ". " + app.fileInfo(filePath).baseName, Qt.ElideMiddle, recentFilesMenu.width)
                                        ToolTip.text: filePath
                                        ToolTip.visible: hovered
                                        onClicked: fileOpenButton.doOpen(filePath)
                                    }
                                }
                            }

                            MenuItem2 {
                                text: "Save"
                                onTriggered: cmdSave.doClick()
                            }

                            MenuItem2 {
                                text: "Save As"
                                onTriggered: fileDialog.launch("SAVE")
                            }
                        }

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Scriptalay"
                            enabled: documentUI.width >= 858
                            onTriggered: openFromLibrary.click()
                        }

                        MenuSeparator { }

                        Menu2 {
                            title: "Import, Export, Reports"

                            Menu2 {
                                title: "Import"

                                Repeater {
                                    model: scriteDocument.supportedImportFormats

                                    MenuItem2 {
                                        text: modelData
                                        onClicked: importMenu.itemAt(index).click()
                                    }
                                }
                            }

                            Menu2 {
                                id: exportMenu2
                                title: "Export"
                                width: 250

                                Component.onCompleted: {
                                    var formats = scriteDocument.supportedExportFormats
                                    for(var i=0; i<formats.length; i++) {
                                        var format = formats[i]
                                        if(format === "")
                                            exportMenu2.addItem(menuSeparatorComponent.createObject(exportMenu))
                                        else
                                            exportMenu2.addItem(menuItemComponent.createObject(exportMenu, {"format": format}))
                                    }
                                }
                            }

                            Menu2 {
                                title: "Reports"
                                width: 300

                                Repeater {
                                    model: scriteDocument.supportedReports

                                    MenuItem2 {
                                        text: modelData.name
                                        onClicked: reportsMenu.itemAt(index).click()
                                        enabled: documentUI.width >= 800
                                    }
                                }
                            }
                        }

                        MenuSeparator { }

                        Menu2 {
                            // FIXME: This is a duplicate of the languageMenu.
                            // We should remove this when we build an ActionManager.
                            title: "Language"

                            Repeater {
                                model: app.enumerationModel(app.transliterationEngine, "Language")

                                MenuItem2 {
                                    property string baseText: modelData.key
                                    property string shortcutKey: app.transliterationEngine.shortcutLetter(modelData.value)
                                    text: baseText + " (" + app.polishShortcutTextForDisplay("Alt+"+shortcutKey) + ")"
                                    font.bold: app.transliterationEngine.language === modelData.value
                                    onClicked: {
                                        app.transliterationEngine.language = modelData.value
                                        scriteDocument.formatting.defaultLanguage = modelData.value
                                        paragraphLanguageSettings.defaultLanguage = modelData.key
                                    }
                                }
                            }

                            MenuSeparator { }

                            MenuItem2 {
                                text: "Next-Language (F10)"
                                onClicked: {
                                    app.transliterationEngine.cycleLanguage()
                                    scriteDocument.formatting.defaultLanguage = app.transliterationEngine.language
                                    paragraphLanguageSettings.defaultLanguage = app.transliterationEngine.languageAsString
                                }
                            }
                        }

                        MenuItem2 {
                            text: "Alphabet Mappings For " + app.transliterationEngine.languageAsString
                            enabled: app.transliterationEngine.language !== TransliterationEngine.English
                            onClicked: alphabetMappingsPopup.visible = !alphabetMappingsPopup.visible
                        }

                        MenuSeparator { }

                        Menu {
                            title: "View"
                            width: 250

                            MenuItem2 {
                                text: "Screenplay (" + app.polishShortcutTextForDisplay("Alt+1") + ")"
                                onTriggered: mainTabBar.activateTab(0)
                                font.bold: mainTabBar.currentIndex === 0
                            }

                            MenuItem2 {
                                text: "Structure (" + app.polishShortcutTextForDisplay("Alt+2") + ")"
                                onTriggered: mainTabBar.activateTab(1)
                                font.bold: mainTabBar.currentIndex === 1
                            }

                            MenuItem2 {
                                text: "Notebook (" + app.polishShortcutTextForDisplay("Alt+3") + ")"
                                onTriggered: mainTabBar.activateTab(2)
                                font.bold: mainTabBar.currentIndex === 2
                                enabled: !showNotebookInStructure
                            }

                            MenuItem2 {
                                text: "Scrited (" + app.polishShortcutTextForDisplay("Alt+4") + ")"
                                onTriggered: mainTabBar.currentIndex = 3
                                font.bold: mainTabBar.currentIndex === 3
                            }
                        }

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Settings"
                            enabled: documentUI.width >= 1100
                            onTriggered: settingsMenuItem.activate()
                        }

                        MenuItem2 {
                            text: "Help"
                            onTriggered: helpButton.click()
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
            x: appToolBar.visible ? (parent.width - appLogo.width - width) : (appToolsMenu.x + (parent.width - width - appToolsMenu.width - appToolsMenu.x)/2)
            height: parent.height
            spacing: 20

            ScreenplayEditorToolbar {
                id: globalScreenplayEditorToolbar
                property Item sceneEditor
                readonly property bool editInFullscreen: true
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

                Connections {
                    target: scriteDocument
                    onJustReset: mainTabBar.activateTab(0)
                    onAboutToSave: {
                        var userData = scriteDocument.userData
                        userData["mainTabBar"] = {
                            "version": 0,
                            "currentIndex": mainTabBar.currentIndex
                        }
                        scriteDocument.userData = userData
                    }
                    onJustLoaded: {
                        var userData = scriteDocument.userData
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
                    if(index < 0 || index >= tabs.length)
                        return
                    var tab = tabs[index]
                    if(!tab.visible)
                        index = 0
                    var message = "Preparing the <b>" + tabs[index].name + "</b> tab, just a few seconds ..."
                    scriteDocument.setBusyMessage(message)
                    app.execLater(mainTabBar, 100, function() {
                        mainTabBar.currentIndex = index
                        scriteDocument.clearBusyMessage()
                    })
                }

                property Item currentTab: currentIndex >= 0 && mainTabBarRepeater.count === tabs.length ? mainTabBarRepeater.itemAt(currentIndex) : null
                property int currentIndex: -1
                readonly property var tabs: [
                    { "name": "Screenplay", "icon": "../icons/navigation/screenplay_tab.png", "visible": true },
                    { "name": "Structure", "icon": "../icons/navigation/structure_tab.png", "visible": true },
                    { "name": "Notebook", "icon": "../icons/navigation/notebook_tab.png", "visible": !showNotebookInStructure },
                    { "name": "Scrited", "icon": "../icons/navigation/scrited_tab.png", "visible": workspaceSettings.showScritedTab }
                ]
                property var currentTabP1: currentTabExtents.value.p1
                property var currentTabP2: currentTabExtents.value.p2
                readonly property color activeTabColor: primaryColors.windowColor

                onCurrentIndexChanged: {
                    if(currentIndex !== 0)
                        shortcutsDockWidget.hide()
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

                Component.onCompleted: currentIndex = 0

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
                            fillColor: parent.active ? mainTabBar.activeTabColor : primaryColors.c10.background
                            outlineColor: primaryColors.borderColor
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
                            font.pointSize: app.idealFontPointSize
                        }

                        Image {
                            source: modelData.icon
                            width: parent.active ? 32 : 24; height: width
                            Behavior on width {
                                enabled: screenplayEditorSettings.enableAnimations
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
                            ToolTip.text: modelData.name + "\t" + app.polishShortcutTextForDisplay("Alt+"+(index+1))
                            ToolTip.delay: 1000
                            ToolTip.visible: containsMouse
                        }
                    }
                }
            }
        }

        Item {
            id: appLogo
            anchors.right: parent.right
            width: 32+20+10
            height: 32
            anchors.verticalCenter: parent.verticalCenter

            Image {
                // source: documentUI.width >= 1440 ? "../images/teriflix_logo.png" : "../images/teriflix_icon.png"
                source: "../images/appicon.png"
                x: 20
                height: parent.height
                width: parent.height
                smooth: true
                mipmap: true
                fillMode: Image.PreserveAspectFit
                transformOrigin: Item.Right
                ToolTip.text: "About, Help & Feedback"

                MouseArea {
                    hoverEnabled: true
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onEntered: parent.ToolTip.visible = true
                    onExited: parent.ToolTip.visible = false
                    enabled: appToolBar.visible
                    onClicked: {
                        modalDialog.sourceComponent = aboutBoxComponent
                        modalDialog.popupSource = parent
                        modalDialog.active = true
                    }
                }
            }
        }
    }

    Loader {
        id: contentLoader
        active: allowContent && !scriteDocument.loading
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
    }

    SequentialAnimation {
        id: resetContentAnimation
        property string filePath
        property var callback
        property bool openFileDialog: false

        ScriptAction {
            script: {
                contentLoader.active = false
            }
        }

        PauseAnimation {
            duration: 100
        }

        ScriptAction {
            script: {
                if(resetContentAnimation.filePath === "")
                    scriteDocument.reset()
                else
                    resetContentAnimation.callback(resetContentAnimation.filePath)
                resetContentAnimation.filePath = ""
                resetContentAnimation.callback = undefined
                contentLoader.active = true

                if(resetContentAnimation.openFileDialog)
                    fileDialog.open()
                resetContentAnimation.openFileDialog = false
            }
        }
    }

    ScreenplayTracks {
        id: screenplayTracks
        screenplay: scriteDocument.screenplay
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
                outlineColor: primaryColors.borderColor
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
            }
        }
    }

    Component {
        id: screenplayEditorComponent

        ScreenplayEditor {
            zoomLevelModifier: mainTabBar.currentIndex > 0 ? -3 : 0
            additionalCharacterMenuItems: {
                if(mainTabBar.currentIndex === 1) {
                    if(showNotebookInStructure)
                        return [{"name": "Character Notes", "description": "Create/switch to notes for the character in notebook"}]
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
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }

            enableSceneListPanel: mainTabBar.currentIndex === 0

            onAdditionalCharacterMenuItemClicked: {
                if(menuItemName === "Character Notes" && showNotebookInStructure) {
                    var ch = scriteDocument.structure.findCharacter(characterName)
                    if(ch === null)
                        scriteDocument.structure.addCharacter(characterName)
                    Announcement.shout("7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8", characterName)
                }
            }

            onAdditionalSceneMenuItemClicked: {
                if(menuItemName === "Scene Notes")
                    Announcement.shout("41EE5E06-FF97-4DB6-B32D-F938418C9529", undefined)
            }
        }
    }

    Component {
        id: structureEditorComponent

        SplitView {
            id: structureEditorSplitView1
            orientation: Qt.Vertical
            Material.background: Qt.darker(primaryColors.windowColor, 1.1)

            Rectangle {
                id: structureEditorRow1
                SplitView.fillHeight: true
                color: primaryColors.c10.background

                SplitView {
                    id: structureEditorSplitView2
                    orientation: Qt.Horizontal
                    Material.background: Qt.darker(primaryColors.windowColor, 1.1)
                    anchors.fill: parent

                    Rectangle {
                        SplitView.fillWidth: true
                        SplitView.minimumWidth: 80
                        color: primaryColors.c10.background
                        border {
                            width: showNotebookInStructure ? 0 : 1
                            color: primaryColors.borderColor
                        }

                        Item {
                            id: structureEditorTabs
                            anchors.fill: parent
                            property int currentTabIndex: 0

                            Announcement.onIncoming: {
                                var sdata = "" + data
                                var stype = "" + type
                                if(showNotebookInStructure) {
                                    if(stype === "190B821B-50FE-4E47-A4B2-BDBB2A13B72C") {
                                        if(sdata === "Structure")
                                            structureEditorTabs.currentTabIndex = 0
                                        else if(sdata.startsWith("Notebook")) {
                                            structureEditorTabs.currentTabIndex = 1
                                            if(sdata !== "Notebook")
                                                app.execLater(notebookViewLoader, 100, function() {
                                                    notebookViewLoader.item.switchTo(sdata)
                                                })
                                        }
                                    } else if(stype === "7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8") {
                                        structureEditorTabs.currentTabIndex = 1
                                        app.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToCharacterTab(data)
                                        })
                                    }
                                    else if(stype === "41EE5E06-FF97-4DB6-B32D-F938418C9529") {
                                        structureEditorTabs.currentTabIndex = 1
                                        app.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToSceneTab(data)
                                        })
                                    }
                                }
                            }

                            Loader {
                                id: structureViewLoader
                                anchors.fill: parent
                                visible: !showNotebookInStructure || structureEditorTabs.currentTabIndex === 0
                                sourceComponent: StructureView { }
                            }

                            Loader {
                                id: notebookViewLoader
                                anchors.fill: parent
                                active: showNotebookInStructure && structureEditorTabs.currentTabIndex === 1
                                sourceComponent: NotebookView {
                                    toolbarSize: appToolBar.height+4
                                    toolbarSpacing: appToolBar.spacing
                                    toolbarLeftMargin: appToolBar.anchors.leftMargin
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
                                color: app.translucent(primaryColors.button, 0.5)

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
                                        color: primaryColors.windowColor
                                        visible: screenplayEditorHandleAnimation.running
                                    }

                                    Text {
                                        color: primaryColors.c50.background
                                        text: "Pull this handle to view the screenplay editor."
                                        font.pointSize: app.idealFontPointSize + 2
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
                                        color: primaryColors.windowColor
                                        visible: timelineViewHandleAnimation.running
                                    }

                                    Text {
                                        color: primaryColors.c50.background
                                        text: "Pull this handle to get the timeline view."
                                        font.pointSize: app.idealFontPointSize
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
                        SplitView.preferredWidth: ui.width * 0.5
                        SplitView.minimumWidth: 16
                        onWidthChanged: workspaceSettings.screenplayEditorWidth = width
                        active: width >= 50
                        sourceComponent: mainTabBar.currentIndex === 1 ? screenplayEditorComponent : null

                        Rectangle {
                            visible: !parent.active
                            anchors.fill: parent
                            color: primaryColors.c400.background
                        }
                    }
                }
            }

            Loader {
                id: structureEditorRow2
                SplitView.preferredHeight: 140 + minimumAppFontMetrics.height*screenplayTracks.trackCount
                SplitView.minimumHeight: 16
                SplitView.maximumHeight: SplitView.preferredHeight
                active: height >= 50
                sourceComponent: Rectangle {
                    color: FocusTracker.hasFocus ? accentColors.c300.background : accentColors.c200.background
                    FocusTracker.window: qmlWindow

                    Behavior on color {
                        enabled: screenplayEditorSettings.enableAnimations
                        ColorAnimation { duration: 250 }
                    }

                    ScreenplayView {
                        anchors.fill: parent
                        showNotesIcon: showNotebookInStructure
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: Qt.rgba(0,0,0,0)
                        border { width: 1; color: accentColors.borderColor }
                    }
                }

                Rectangle {
                    visible: !parent.active
                    anchors.fill: parent
                    color: primaryColors.c400.background
                }
            }

            Connections {
                target: scriteDocument
                onAboutToSave: structureEditorSplitView1.saveLayoutDetails()
                onJustLoaded: structureEditorSplitView1.restoreLayoutDetails()
            }

            Component.onCompleted: restoreLayoutDetails()
            Component.onDestruction: saveLayoutDetails()

            function saveLayoutDetails() {
                var userData = scriteDocument.userData
                userData["structureTab"] = {
                    "version": 0,
                    "screenplayEditorWidth": screenplayEditor2.width/structureEditorRow1.width,
                    "timelineViewHeight": structureEditorRow2.height
                }
                scriteDocument.userData = userData
            }

            function restoreLayoutDetails() {
                var userData = scriteDocument.userData
                if(userData.structureTab && userData.structureTab.version === 0) {
                    structureEditorRow2.SplitView.preferredHeight = userData.structureTab.timelineViewHeight
                    structureEditorRow2.height = structureEditorRow2.SplitView.preferredHeight
                    screenplayEditor2.SplitView.preferredWidth = structureEditorRow1.width*userData.structureTab.screenplayEditorWidth
                    screenplayEditor2.width = screenplayEditor2.SplitView.preferredWidth
                }

                if(structureCanvasSettings.showPullHandleAnimation && contentLoader.sessionId !== scriteDocument.sessionId) {
                    app.execLater(splitViewAnimationLoader, 250, function() {
                        splitViewAnimationLoader.active = !screenplayEditor2.active || !structureEditorRow2.active
                    })
                    contentLoader.sessionId = scriteDocument.sessionId
                }
            }
        }
    }

    Component {
        id: notebookEditorComponent

        NotebookView {
            Announcement.onIncoming: {
                var stype = "" + "190B821B-50FE-4E47-A4B2-BDBB2A13B72C"
                var sdata = "" + data
                if(stype === "190B821B-50FE-4E47-A4B2-BDBB2A13B72C")
                    switchTo(sdata)
            }
        }
    }

    Component {
        id: scritedComponent

        ScritedView {

        }
    }

    function newFromTemplate() {
        if(!scriteDocument.empty) {
            contentLoader.allowContent = false
            scriteDocument.reset()
            contentLoader.allowContent = true
        }
        if(app.internetAvailable) {
            modalDialog.popupSource = fileNewButton
            modalDialog.sourceComponent = openTemplateDialogComponent
            modalDialog.closeable = false
            modalDialog.active = true
        }
    }

    Component {
        id: openTemplateDialogComponent

        OpenTemplateDialog {
            onImportStarted: contentLoader.allowContent = false
            onImportFinished: contentLoader.allowContent = true
            onImportCancelled: {
                contentLoader.allowContent = false
                scriteDocument.reset()
                contentLoader.allowContent = true
            }
        }
    }

    Component {
        id: aboutBoxComponent

        AboutBox { }
    }

    Component {
        id: optionsDialogComponent

        OptionsDialog { }
    }

    FileDialog {
        id: fileDialog
        nameFilters: modes[mode].nameFilters
        selectFolder: false
        selectMultiple: false
        onFolderChanged: {
            if(mode === "OPEN")
                workspaceSettings.lastOpenFolderUrl = folder
            else
                workspaceSettings.lastOpenImportFolderUrl = folder
        }
        sidebarVisible: true
        selectExisting: modes[mode].selectExisting
        property string mode: "OPEN"

        property ErrorReport errorReport: Aggregation.findErrorReport(scriteDocument)
        Notification.title: modes[mode].notificationTitle
        Notification.text: errorReport.errorMessage
        Notification.active: errorReport.hasError
        Notification.autoClose: false
        Notification.onDismissed: {
            if(errorReport.details && errorReport.details.revealOnDesktopRequest)
                app.revealFileOnDesktop(errorReport.details.revealOnDesktopRequest)
            errorReport.clear()
        }

        Component.onCompleted: {
            var availableModes = {
                "OPEN": {
                    "nameFilters": ["Scrite Projects (*.scrite)"],
                    "selectExisting": true,
                    "callback": function(path) {
                        contentLoader.allowContent = false
                        scriteDocument.open(path)
                        contentLoader.allowContent = true
                        recentFilesMenu.add(path)
                    },
                    "reset": true,
                    "notificationTitle": "Opening Scrite Project"
                },
                "SAVE": {
                    "nameFilters": ["Scrite Projects (*.scrite)"],
                    "selectExisting": false,
                    "callback": function(path) {
                        scriteDocument.saveAs(path)
                        recentFilesMenu.add(path)
                    },
                    "reset": false,
                    "notificationTitle": "Saving Scrite Project"
                }
            }

            scriteDocument.supportedImportFormats.forEach(function(format) {
                availableModes["IMPORT " + format] = {
                    "nameFilters": scriteDocument.importFormatFileSuffix(format),
                    "selectExisting": true,
                    "callback": function(path) {
                        contentLoader.allowContent = false
                        scriteDocument.importFile(path, format)
                        contentLoader.allowContent = true
                    },
                    "reset": true,
                    "notificationTitle": "Creating Scrite project from " + format
                }
            })

            modes = availableModes
        }

        property var modes

        function launch(launchMode, filePath) {
            mode = launchMode
            folder = mode === "IMPORT" ? workspaceSettings.lastOpenImportFolderUrl : workspaceSettings.lastOpenFolderUrl

            if(filePath)
                app.execLater(qmlWindow, 250, function() { processFile(filePath) } )
            else {
                var modeInfo = modes[mode]
                if(modeInfo["reset"] === true) {
                    resetContentAnimation.openFileDialog = true
                    resetContentAnimation.start()
                } else
                    open()
            }
        }

        onAccepted: processFile()

        function processFile(filePath) {
            var modeInfo = modes[mode]
            if(modeInfo["reset"] === true) {
                resetContentAnimation.filePath = filePath ? filePath : app.urlToLocalFile(fileUrl)
                resetContentAnimation.callback = modeInfo.callback
                resetContentAnimation.start()
            } else {
                modeInfo.callback(app.urlToLocalFile(fileUrl))
            }
        }
    }

    Item {
        property ErrorReport applicationErrors: Aggregation.findErrorReport(app)
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
        id: openFromLibraryComponent

        OpenFromLibrary {
            onImportStarted: contentLoader.allowContent = false
            onImportFinished: contentLoader.allowContent = true
        }
    }

    Component {
        id: backupsDialogBoxComponent

        BackupsDialogBox {
            onOpenInThisWindow: {
                contentLoader.allowContent = false
                scriteDocument.openAnonymously(filePath)
                modalDialog.close()
                app.execLater(contentLoader, 300, function() {
                    contentLoader.allowContent = true
                })
            }
            onOpenInNewWindow: {
                app.launchNewInstanceAndOpenAnonymously(qmlWindow, filePath)
                app.execLater(modalDialog, 4000, function() {
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
            target: qmlWindow
            onClosing: {
                if(closeEventHandler.handleCloseEvent) {
                    app.saveWindowGeometry(qmlWindow, "Workspace")

                    if(!scriteDocument.modified) {
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
                                if(scriteDocument.fileName !== "")
                                    scriteDocument.save()
                                else {
                                    cmdSave.doClick()
                                    return
                                }
                            }
                            closeEventHandler.handleCloseEvent = false
                            qmlWindow.close()
                        }
                    }, closeEventHandler)
                } else
                    close.accepted = true
            }
        }
    }

    QtObject {
        ShortcutsModelItem.enabled: app.isTextInputItem(qmlWindow.activeFocusItem)
        ShortcutsModelItem.priority: 10
        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: "Symbols & Smileys"
        ShortcutsModelItem.shortcut: "F3"
    }

    DockWidget {
        id: shortcutsDockWidget
        title: "Shortcuts"
        anchors.fill: parent
        contentX: -1
        contentY: -1
        contentWidth: 375
        contentHeight: (parent.height - appToolBarArea.height - 30) * 0.85
        visible: shortcutsDockWidgetSettings.visible
        sourceItem: settingsAndShortcutsButton
        content: ShortcutsView { }
        onCloseRequest: hide()

        Settings {
            id: shortcutsDockWidgetSettings
            fileName: app.settingsFilePath
            category: "Shortcuts Dock Widget"
            property alias contentX: shortcutsDockWidget.contentX
            property alias contentY: shortcutsDockWidget.contentY
            property alias visible: shortcutsDockWidget.visible
        }

        Connections {
            target: splashLoader
            onActiveChanged: {
                if(splashLoader.active)
                    return
                if(shortcutsDockWidget.contentX < 0 || shortcutsDockWidget.contentX + shortcutsDockWidget.contentWidth > documentUI.width)
                    shortcutsDockWidget.contentX = documentUI.width - 40 - shortcutsDockWidget.contentWidth
                if(shortcutsDockWidget.contentY < 0 || shortcutsDockWidget.contentY + shortcutsDockWidget.contentHeight > documentUI.height)
                    shortcutsDockWidget.contentY = (documentUI.height - shortcutsDockWidget.contentHeight)/2
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
        contentHeight: documentUI.height * 0.6
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

    Loader {
        active: automationScript !== ""
        source: automationScript
        onSourceChanged: console.log("PA: " + source)
    }

    Component.onCompleted: {
        if(!app.restoreWindowGeometry(qmlWindow, "Workspace"))
            workspaceSettings.screenplayEditorWidth = -1
        if(app.maybeOpenAnonymously())
            splashLoader.active = false
        screenplayAdapter.sessionId = scriteDocument.sessionId
    }

    BusyOverlay {
        id: appBusyOverlay
        anchors.fill: parent
        busyMessage: "Computing Page Layout, Evaluating Page Count & Time ..."
        visible: refCount > 0
        property int refCount: 0
    }
}
