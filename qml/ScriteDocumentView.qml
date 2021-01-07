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

    Settings {
        id: workspaceSettings
        fileName: app.settingsFilePath
        category: "Workspace"
        property real workspaceHeight
        property real screenplayEditorWidth: -1
        property bool scriptalayIntroduced: false
        property bool showNotebookInStructure: true
        property bool mouseWheelZoomsInCharacterGraph: app.isWindowsPlatform || app.isLinuxPlatform
        property bool mouseWheelZoomsInStructureCanvas: app.isWindowsPlatform || app.isLinuxPlatform
        property string lastOpenFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenPhotosFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        property string lastOpenImportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenExportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenReportsFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        property var customColors: []

        onShowNotebookInStructureChanged: {
            app.execLater(workspaceSettings, 100, function() {
                mainTabBar.currentIndex = mainTabBar.currentIndex % (showNotebookInStructure ? 2 : 3)
            })
        }
    }

    Settings {
        id: screenplayEditorSettings
        fileName: app.settingsFilePath
        category: "Screenplay Editor"
        property bool displaySceneCharacters: true
        property bool displaySceneSynopsis: true
        property bool displaySceneComments: false
        property int mainEditorZoomValue: -1
        property int embeddedEditorZoomValue: -1
        property bool includeTitlePageInPreview: true
        property bool enableSpellCheck: false // until we can fix https://github.com/teriflix/scrite/issues/138
        property bool enableAnimations: true
        onEnableAnimationsChanged: {
            modalDialog.animationsEnabled = enableAnimations
            statusText.enableAnimations = enableAnimations
        }

        property real textFormatDockWidgetX: -1
        property real textFormatDockWidgetY: -1
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
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+C"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.displaySceneCharacters ? "Hide Scene Characters" : "Show Scene Characters"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.displaySceneCharacters = !screenplayEditorSettings.displaySceneCharacters
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+S"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.displaySceneSynopsis ? "Hide Synopsis" : "Show Synopsis"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.displaySceneSynopsis = !screenplayEditorSettings.displaySceneSynopsis
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Alt+M"
        ShortcutsModelItem.group: "Settings"
        ShortcutsModelItem.title: screenplayEditorSettings.displaySceneComments ? "Hide Comments" : "Show Comments"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.displaySceneComments = !screenplayEditorSettings.displaySceneComments
    }

    Shortcut {
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
        ShortcutsModelItem.enabled: mainTabBar.currentIndex !== 0
        ShortcutsModelItem.shortcut: sequence
        onActivated: mainTabBar.currentIndex = 0
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Alt+2"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Structure"
        ShortcutsModelItem.enabled: mainTabBar.currentIndex !== 1
        ShortcutsModelItem.shortcut: sequence
        onActivated: mainTabBar.currentIndex = 1
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Alt+3"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Notebook"
        ShortcutsModelItem.enabled: enabled && mainTabBar.currentIndex !== 2
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.visible: enabled
        enabled: !workspaceSettings.showNotebookInStructure
        onActivated: mainTabBar.currentIndex = 2
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Alt+4"
        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Scrited"
        ShortcutsModelItem.enabled: enabled && mainTabBar.currentIndex !== 3
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.visible: enabled
        onActivated: mainTabBar.currentIndex = 3
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
            anchors.leftMargin: 10
            visible: appToolBarArea.width >= 1366
            onVisibleChanged: {
                if(!visible)
                    mainTabBar.currentIndex = 0
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
                                resetContentAnimation.start()
                            }
                        }, fileNewButton)
                    else
                        resetContentAnimation.start()
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
                                    fileDialog.launch("OPEN", filePath)
                                }
                            }, fileOpenButton)
                    else {
                        recentFilesMenu.close()
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

                        property var recentFiles: []
                        function add(filePath) {
                            var r = recentFiles
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
                            text: recentFilesMenu.recentFiles.length > 0 ? "Open Another" : "Open"
                            onClicked: fileOpenButton.doOpen()
                        }

                        MenuSeparator { visible: true }

                        FontMetrics {
                            id: recentFilesFontMetrics
                        }

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
                    else
                        scriteDocument.save()
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
                iconSource: "../icons/content/archive.png"
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
                    resetContentAnimation.filePath = ""
                    resetContentAnimation.openFileDialog = false
                    resetContentAnimation.callback = undefined
                    resetContentAnimation.start()

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
                            text: "Settings\t" + app.polishShortcutTextForDisplay("Ctrl+,")
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
                            text: "Shortcuts\t" + app.polishShortcutTextForDisplay("Ctrl+E")
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
                    }
                }
            }

            ToolButton3 {
                id: helpButton
                iconSource: "../icons/action/help.png"
                text: "Help"
                shortcut: "F1"
                onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/help/")

                ShortcutsModelItem.group: "Application"
                ShortcutsModelItem.title: "Help"
                ShortcutsModelItem.shortcut: "F1"
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
                                onTriggered: mainTabBar.currentIndex = 0
                                font.bold: mainTabBar.currentIndex === 0
                            }

                            MenuItem2 {
                                text: "Structure (" + app.polishShortcutTextForDisplay("Alt+2") + ")"
                                onTriggered: mainTabBar.currentIndex = 1
                                font.bold: mainTabBar.currentIndex === 1
                            }

                            MenuItem2 {
                                text: "Notebook (" + app.polishShortcutTextForDisplay("Alt+3") + ")"
                                onTriggered: mainTabBar.currentIndex = 2
                                font.bold: mainTabBar.currentIndex === 2
                                enabled: !workspaceSettings.showNotebookInStructure
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

        Item {
            id: globalTimeDisplay
            anchors.left: appToolBar.visible ? appToolBar.right : appToolsMenu.right
            anchors.right: editTools.visible ? editTools.left : parent.right
            anchors.margins: 10
            height: parent.height
            property ScreenplayTextDocument screenplayTextDocument
            visible: screenplayTextDocument !== null
            property alias visibleToUser: currentTimeDisplay.visible
            property real contentWidth: currentTimeLabel.visible ? currentTimeLabel.width + 10 : 0

            Rectangle {
                visible: currentTimeDisplay.visible
                anchors.fill: currentTimeDisplay
                anchors.margins: -5
                color: primaryColors.c800.background
                border.color: primaryColors.c300.background
                border.width: 1
                radius: 3

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    ToolTip.text: "Time estimates are approximate, assuming " + globalTimeDisplay.screenplayTextDocument.timePerPageAsString + " per page."
                    ToolTip.delay: 1000
                    ToolTip.visible: containsMouse
                }
            }

            Column {
                id: currentTimeDisplay
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                visible: width <= parent.width

                Text {
                    id: currentTimeLabel
                    font.pixelSize: globalTimeDisplay.height*0.45
                    font.family: scriteDocument.formatting.defaultFont.family
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: primaryColors.c800.text
                    text: {
                        if(globalTimeDisplay.screenplayTextDocument === null)
                            return "00:00"
                        if(globalTimeDisplay.screenplayTextDocument.totalTime.getHours() > 0)
                            return Qt.formatTime(globalTimeDisplay.screenplayTextDocument.currentTime, "H:mm:ss")
                        return Qt.formatTime(globalTimeDisplay.screenplayTextDocument.currentTime, "mm:ss")
                    }
                }

                Text {
                    font.pixelSize: globalTimeDisplay.height*0.15
                    color: primaryColors.c800.text
                    text: "( Current Time )"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }

        ScritedToolbar {
            id: scritedToolbar
            anchors.left: appToolBar.visible ? appToolBar.right : appToolsMenu.right
            anchors.right: editTools.visible ? editTools.left : parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10
            visible: scritedView !== null
        }

        Row {
            id: editTools
            x: appToolBar.visible ? (parent.width - appLogo.width - width) : (appToolsMenu.x + (parent.width - width - appToolsMenu.width - appToolsMenu.x)/2 + (globalTimeDisplay.visible ? globalTimeDisplay.contentWidth/2 : 0))
            height: parent.height
            spacing: 20

            ScreenplayEditorToolbar {
                id: globalScreenplayEditorToolbar
                property Item sceneEditor
                readonly property bool editInFullscreen: true
                anchors.verticalCenter: parent.verticalCenter
                binder: sceneEditor ? sceneEditor.binder : null
                editor: sceneEditor ? sceneEditor.editor : null
                visible: mainTabBar.currentIndex === 1 || mainTabBar.currentIndex === 0
            }

            Row {
                id: mainTabBar
                height: parent.height
                visible: appToolBar.visible

                property Item currentTab: currentIndex >= 0 && mainTabBarRepeater.count === tabs.length ? mainTabBarRepeater.itemAt(currentIndex) : null
                property int currentIndex: -1
                readonly property var tabs: [
                    { "name": "Screenplay", "icon": "../icons/navigation/screenplay_tab.png", "visible": true },
                    { "name": "Structure", "icon": "../icons/navigation/structure_tab.png", "visible": true },
                    { "name": "Notebook", "icon": "../icons/navigation/notebook_tab.png", "visible": !workspaceSettings.showNotebookInStructure },
                    { "name": "Scrited", "icon": "../icons/navigation/scrited_tab.png", "visible": true }
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
                            onClicked: mainTabBar.currentIndex = index
                            ToolTip.text: modelData.name + "\t" + app.polishShortcutTextForDisplay("Alt+"+(index+1))
                            ToolTip.delay: 1000
                            ToolTip.visible: containsMouse
                        }
                    }
                }
            }
        }

        Image {
            id: appLogo
            anchors.right: parent.right
            source: documentUI.width >= 1440 ? "../images/teriflix_logo.png" : "../images/teriflix_icon.png"
            height: parent.height
            smooth: true
            mipmap: true
            fillMode: Image.PreserveAspectFit
            anchors.verticalCenter: parent.verticalCenter
            transformOrigin: Item.Right
            ToolTip.text: "Click here to provide feedback"

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

    Loader {
        id: contentLoader
        active: !scriteDocument.loading
        sourceComponent: uiLayoutComponent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: appToolBarArea.visible ? appToolBarArea.bottom : parent.top
        anchors.bottom: parent.bottom
        onActiveChanged: {
            globalScreenplayEditorToolbar.sceneEditor = null
        }
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
                anchors.margins: 5
                clip: true
                readonly property int screenplayZoomLevelModifier: 0
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
            zoomLevelModifier: screenplayZoomLevelModifier
            additionalCharacterMenuItems: {
                if(mainTabBar.currentIndex === 1) {
                    if(workspaceSettings.showNotebookInStructure)
                        return [{"name": "Character Notes", "description": "Create/switch to notes for the character in notebook"}]
                }
                return []
            }
            additionalSceneMenuItems: {
                if(mainTabBar.currentIndex === 1) {
                    if(workspaceSettings.showNotebookInStructure)
                        return ["Scene Notes"]
                }
                return []
            }
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }

            source: {
                if(mainTabBar.currentIndex !== 0 &&
                   scriteDocument.structure.elementCount > 0 &&
                   scriteDocument.screenplay.currentElementIndex < 0) {
                    var index = scriteDocument.structure.currentElementIndex
                    var element = scriteDocument.structure.elementAt(index)
                    if(scriteDocument.screenplay.firstIndexOfScene(element.scene) >= 0)
                        return scriteDocument.loading ? null : scriteDocument.screenplay
                    return element ? element.scene : scriteDocument.screenplay
                }
                return scriteDocument.loading ? null : scriteDocument.screenplay
            }

            onAdditionalCharacterMenuItemClicked: {
                if(menuItemName === "Character Notes" && workspaceSettings.showNotebookInStructure) {
                    var ch = scriteDocument.structure.findCharacter(characterName)
                    if(ch === null)
                        scriteDocument.structure.addCharacter(characterName)
                    Announcement.shout("7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8", characterName)
                }
            }

            onAdditionalSceneMenuItemClicked: {
                if(menuItemName === "Scene Notes")
                    Announcement.shout("41EE5E06-FF97-4DB6-B32D-F938418C9529", scene)
            }
        }
    }

    Component {
        id: sceneEditorComponent

        ScreenplayEditor {
            source: scriteDocument.structure.elementAt(scriteDocument.structure.currentElementIndex).scene
            zoomLevelModifier: screenplayZoomLevelModifier
        }
    }

    Component {
        id: structureEditorComponent

        SplitView {
            orientation: Qt.Vertical
            Material.background: Qt.darker(primaryColors.windowColor, 1.1)

            Rectangle {
                SplitView.fillHeight: true
                color: primaryColors.c10.background

                SplitView {
                    orientation: Qt.Horizontal
                    Material.background: Qt.darker(primaryColors.windowColor, 1.1)
                    anchors.fill: parent

                    Rectangle {
                        SplitView.fillWidth: true
                        color: primaryColors.c10.background
                        border {
                            width: workspaceSettings.showNotebookInStructure ? 0 : 1
                            color: primaryColors.borderColor
                        }

                        TabView3 {
                            id: structureEditorTabs
                            anchors.fill: parent
                            anchors.margins: 1
                            tabNames: workspaceSettings.showNotebookInStructure ? ["Canvas", "Notebook"] : ["Canvas"]
                            tabColor: primaryColors.c700.background
                            tabBarVisible: workspaceSettings.showNotebookInStructure
                            currentTabContent: Item {
                                Announcement.onIncoming: {
                                    if(workspaceSettings.showNotebookInStructure) {
                                        if(structureEditorTabs.currentTabIndex === 0)
                                            structureEditorTabs.currentTabIndex = 1

                                        if(type === "7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8")
                                            app.execLater(notebookViewLoader, 100, function() {
                                                notebookViewLoader.item.switchToCharacterTab(data)
                                            })
                                        else if(type === "41EE5E06-FF97-4DB6-B32D-F938418C9529")
                                            app.execLater(notebookViewLoader, 100, function() {
                                                notebookViewLoader.item.switchToSceneTab(data)
                                            })
                                    }
                                }

                                Loader {
                                    id: structureViewLoader
                                    anchors.fill: parent
                                    visible: !workspaceSettings.showNotebookInStructure || structureEditorTabs.currentTabIndex === 0
                                    sourceComponent: StructureView { }
                                }

                                Loader {
                                    id: notebookViewLoader
                                    anchors.fill: parent
                                    active: false
                                    visible: workspaceSettings.showNotebookInStructure && structureEditorTabs.currentTabIndex === 1
                                    onVisibleChanged: {
                                        if(visible && !active)
                                            active = true
                                    }
                                    sourceComponent: NotebookView { }
                                }
                            }
                        }
                    }

                    Loader {
                        id: screenplayEditor2
                        SplitView.preferredWidth: workspaceSettings.screenplayEditorWidth < 0 ? ui.width * 0.5 : workspaceSettings.screenplayEditorWidth
                        onWidthChanged: workspaceSettings.screenplayEditorWidth = width
                        readonly property int screenplayZoomLevelModifier: -3
                        active: width >= 50
                        sourceComponent: mainTabBar.currentIndex === 1 ? screenplayEditorComponent : null
                    }
                }
            }

            Loader {
                SplitView.preferredHeight: 155
                SplitView.minimumHeight: SplitView.preferredHeight
                SplitView.maximumHeight: SplitView.preferredHeight
                active: height >= 50
                sourceComponent: Rectangle {
                    color: accentColors.c200.background
                    border { width: 1; color: accentColors.borderColor }

                    ScreenplayView {
                        anchors.fill: parent
                        anchors.margins: 5
                        showNotesIcon: workspaceSettings.showNotebookInStructure
                    }
                }
            }
        }
    }

    Component {
        id: notebookEditorComponent

        SplitView {
            orientation: Qt.Vertical
            Material.background: Qt.darker(primaryColors.windowColor, 1.1)

            NotebookView {
                SplitView.fillHeight: true
            }

            Loader {
                SplitView.preferredHeight: 155
                SplitView.minimumHeight: SplitView.preferredHeight
                SplitView.maximumHeight: SplitView.preferredHeight
                active: height >= 50

                sourceComponent: Rectangle {
                    color: accentColors.c200.background
                    border { width: 1; color: accentColors.borderColor }

                    ScreenplayView {
                        anchors.fill: parent
                        anchors.margins: 5
                        showNotesIcon: true
                        enableDragDrop: false
                    }
                }
            }
        }
    }

    Component {
        id: scritedComponent

        ScritedView {

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
        Notification.onDismissed: errorReport.clear()

        Component.onCompleted: {
            var availableModes = {
                "OPEN": {
                    "nameFilters": ["Scrite Projects (*.scrite)"],
                    "selectExisting": true,
                    "callback": function(path) {
                        scriteDocument.open(path)
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
                        scriteDocument.importFile(path, format)
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

        OpenFromLibrary { }
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
    }
}
