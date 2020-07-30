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
    }

    Settings {
        id: screenplayEditorSettings
        fileName: app.settingsFilePath
        category: "Screenplay Editor"
        property bool displaySceneCharacters: true
        property bool displaySceneNotes: true
        property int mainEditorZoomValue: -1
        property int embeddedEditorZoomValue: -1
        property bool includeTitlePageInPreview: true
        property bool enableSpellCheck: false // until we can fix https://github.com/teriflix/scrite/issues/138
        property bool enableAnimations: true
        onEnableAnimationsChanged: modalDialog.animationsEnabled = enableAnimations
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
        ShortcutsModelItem.title: screenplayEditorSettings.displaySceneNotes ? "Hide Synopsis" : "Show Synopsis"
        ShortcutsModelItem.shortcut: sequence
        onActivated: screenplayEditorSettings.displaySceneNotes = !screenplayEditorSettings.displaySceneNotes
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
            visible: appToolBarArea.width >= 1326
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
                function click() {
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
                onClicked: click()

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
                enabled: scriteDocument.structure.elementCount > 0 && !scriteDocument.readOnly
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
                enabled: scriteDocument.structure.elementCount > 0
                onClicked: fileDialog.launch("SAVE")

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

                function click() {
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

                onClicked: click()

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

            ToolButton3 {
                shortcut: "Ctrl+Z"
                shortcutText: "Z"
                iconSource: "../icons/content/undo.png"
                enabled: app.canUndo && !scriteDocument.readOnly
                onClicked: app.undoGroup.undo()
                ToolTip.text: "Undo" + "\t" + app.polishShortcutTextForDisplay(shortcut)

                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Undo"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: shortcut
            }

            ToolButton3 {
                shortcut: app.isMacOSPlatform ? "Ctrl+Shift+Z" : "Ctrl+Y"
                shortcutText: app.isMacOSPlatform ? "Shift+Z" : "Y"
                iconSource: "../icons/content/redo.png"
                enabled: app.canRedo && !scriteDocument.readOnly
                onClicked: app.undoGroup.redo()
                ToolTip.text: "Redo" + "\t" + app.polishShortcutTextForDisplay(shortcut)

                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Redo"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: shortcut

            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
                opacity: 0.5
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

                Item {
                    anchors.top: parent.bottom
                    anchors.left: parent.left

                    Menu2 {
                        id: languageMenu
                        width: 250

                        ButtonGroup { id: languageMenuGroup }

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
                onClicked: alphabetMappingsPopup.visible = true
                down: alphabetMappingsPopup.visible
                enabled: app.transliterationEngine.language !== TransliterationEngine.English

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
                            sourceComponent: AlphabetMappings { }
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
                        MenuItem2 {
                            text: "New File"
                            onTriggered: fileNewButton.click()
                        }

                        MenuItem2 {
                            text: "Open File"
                            onTriggered: fileOpenButton.doOpen()
                        }

                        Menu2 {
                            title: "Open Recent"
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

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Scriptalay"
                            enabled: documentUI.width >= 858
                            onTriggered: openFromLibrary.click()
                        }

                        MenuSeparator { }

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

                        MenuSeparator { }

                        MenuItem2 {
                            text: "Settings"
                            enabled: documentUI.width >= 1100
                            onTriggered: settingsMenuItem.activate()
                        }
                    }
                }
            }
        }

        Row {
            id: editTools
            x: appToolBar.visible ? (parent.width - appLogo.width - width) : (appToolsMenu.x + (parent.width - width - appToolsMenu.width - appToolsMenu.x) / 2)
            height: parent.height

            ScreenplayEditorToolbar {
                id: globalScreenplayEditorToolbar
                property Item sceneEditor
                readonly property bool editInFullscreen: true
                anchors.verticalCenter: parent.verticalCenter
                binder: sceneEditor ? sceneEditor.binder : null
                editor: sceneEditor ? sceneEditor.editor : null
            }

            // We move the main-tab bar here based on UI/UX suggestions from Surya Vasishta
            Row {
                id: mainTabBar
                height: parent.height
                visible: appToolBar.visible
                onVisibleChanged: currentIndex = 0

                property Item currentTab: currentIndex >= 0 ? mainTabBarRepeater.itemAt(currentIndex) : null
                property int currentIndex: -1
                property var tabs: ["Screenplay", "Structure", "Notebook"]
                property var currentTabP1: currentTabExtents.value.p1
                property var currentTabP2: currentTabExtents.value.p2
                property color activeTabColor: primaryColors.windowColor

                onCurrentIndexChanged: {
                    if(currentIndex !== 0)
                        shortcutsDockWidget.hide()
                }

                ResetOnChange {
                    id: currentTabExtents
                    trackChangesOn: appToolBarArea.width
                    from: {
                        "p1": { "x": 0, "y": 0 },
                        "p2": { "x": 0, "y": 0 }
                    }
                    to: mainTabBar.visible && mainTabBar.currentTab ? {
                        "p1": mainTabBar.mapFromItem(mainTabBar.currentTab, 0, 0),
                        "p2": mainTabBar.mapFromItem(mainTabBar.currentTab, mainTabBar.currentTab.width, 0)
                    } : from
                }

                Component.onCompleted: currentIndex = 0

                Repeater {
                    id: mainTabBarRepeater
                    model: mainTabBar.tabs

                    Item {
                        property bool active: mainTabBar.currentIndex === index
                        height: mainTabBar.height
                        width: tabBarFontMetrics.advanceWidth(modelData) + 30

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

                        Text {
                            id: tabBarText
                            text: modelData
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset:parent.active ? 0 : 1
                            font.pointSize: app.idealFontPointSize
                            font.bold: parent.active
                        }

                        MouseArea {
                            id: tabBarMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: mainTabBar.currentIndex = index
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
        active: true
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

            StackLayout {
                id: uiLayoutTabView
                anchors.fill: parent
                anchors.margins: 5
                clip: true
                currentIndex: mainTabBar.currentIndex

                Loader {
                    readonly property bool editCurrentSceneInStructure: false
                    readonly property int screenplayZoomLevelModifier: 0
                    sourceComponent: mainTabBar.currentIndex === 0 ? screenplayEditorComponent : null
                }

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
                                    width: 1
                                    color: primaryColors.borderColor
                                }

                                StructureView {
                                    anchors.fill: parent
                                    anchors.margins: 1
                                    onRequestEditor: screenplayEditor2.editCurrentSceneInStructure = true
                                    onReleaseEditor: screenplayEditor2.editCurrentSceneInStructure = false
                                }
                            }

                            Loader {
                                id: screenplayEditor2
                                SplitView.preferredWidth: workspaceSettings.screenplayEditorWidth < 0 ? scriteDocument.formatting.pageLayout.paperWidth * 1.4 : workspaceSettings.screenplayEditorWidth
                                onWidthChanged: workspaceSettings.screenplayEditorWidth = width
                                property bool editCurrentSceneInStructure: true
                                readonly property int screenplayZoomLevelModifier: -3
                                active: screenplayEditor2Active.value
                                sourceComponent: mainTabBar.currentIndex === 1 ? screenplayEditorComponent : null
                            }

                            ResetOnChange {
                                id: screenplayEditor2Active
                                trackChangesOn: screenplayEditor2.editCurrentSceneInStructure
                                from: false; to: true
                            }
                        }
                    }

                    Item {
                        SplitView.preferredHeight: screenplayView.preferredHeight + 40
                        SplitView.minimumHeight: SplitView.preferredHeight
                        SplitView.maximumHeight: SplitView.preferredHeight

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            color: accentColors.c200.background
                            border { width: 1; color: accentColors.borderColor }

                            ScreenplayView {
                                id: screenplayView
                                anchors.fill: parent
                                anchors.margins: 5
                                onRequestEditor: screenplayEditor2.editCurrentSceneInStructure = false
                            }
                        }
                    }
                }

                SplitView {
                    orientation: Qt.Vertical
                    Material.background: Qt.darker(primaryColors.windowColor, 1.1)

                    NotebookView {
                        SplitView.fillHeight: true
                    }

                    Item {
                        SplitView.preferredHeight: screenplayView2.preferredHeight + 40
                        SplitView.minimumHeight: SplitView.preferredHeight
                        SplitView.maximumHeight: SplitView.preferredHeight

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            color: accentColors.c200.background
                            border { width: 1; color: accentColors.borderColor }

                            ScreenplayView {
                                id: screenplayView2
                                anchors.fill: parent
                                anchors.margins: 5
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: screenplayEditorComponent

        ScreenplayEditor {
            zoomLevelModifier: screenplayZoomLevelModifier
            source: sourceBinder.get

            DelayedPropertyBinder {
                id: sourceBinder
                initial: null
                set: {
                    if(editCurrentSceneInStructure) {
                        var index = scriteDocument.structure.currentElementIndex
                        var element = scriteDocument.structure.elementAt(index)
                        return element ? element.scene : null
                    }
                    return scriteDocument.loading ? null : scriteDocument.screenplay
                }
                delay: 50
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
        folder: {
            if(scriteDocument.fileName !== "") {
                var fileInfo = app.fileInfo(scriteDocument.fileName)
                if(fileInfo.exists)
                    return "file:///" + fileInfo.absolutePath
            }
            return "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
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

    Item {
        property ProgressReport progressReport: Aggregation.findProgressReport(app)
        Notification.active: progressReport ? progressReport.progress < 1 : false
        Notification.title: progressReport ? progressReport.progressText : ""
        Notification.text: progressReport ? ("Progress: " + Math.floor(progressReport.progress*100) + "%") : ""
        Notification.autoCloseDelay: 1000
        Notification.autoClose: true
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

    Loader {
        id: openingAnimationLoader
        active: splashLoader.active === false
        anchors.fill: parent
        sourceComponent: SequentialAnimation {
            running: true

            PauseAnimation {
                duration: screenplayEditorSettings.enableAnimations ? 500 : 0
            }

            ScriptAction {
                script: appLogo.ToolTip.visible = true
            }

            PropertyAnimation {
                target: appLogo
                properties: "scale"
                from: 1; to: 1.5
                duration: screenplayEditorSettings.enableAnimations ? 1000 : 0
            }

            PropertyAnimation {
                target: appLogo
                properties: "scale"
                from: 1.5; to: 1
                duration: screenplayEditorSettings.enableAnimations ? 1000 : 0
            }

            PauseAnimation {
                duration: screenplayEditorSettings.enableAnimations ? 0 : 2000
            }

            ScriptAction {
                script: {
                    appLogo.ToolTip.visible = false
                    if(workspaceSettings.scriptalayIntroduced)
                        openingAnimationLoader.active = false
                    else {
                        var r = openingAnimationLoader.mapFromItem(openFromLibrary, 0, 0, openFromLibrary.width, openFromLibrary.height)
                        openFromLibraryIntro.parent.x = r.x
                        openFromLibraryIntro.parent.y = r.y
                        openFromLibraryIntro.parent.width = r.width
                        openFromLibraryIntro.parent.height = r.height
                        openFromLibraryIntro.visible = true
                    }
                }
            }

            PropertyAnimation {
                target: openFromLibrary.toolButtonImage
                properties: "scale"
                from: 1; to: 3
                duration: screenplayEditorSettings.enableAnimations ? 1000 : 0
            }

            PropertyAnimation {
                target: openFromLibrary.toolButtonImage
                properties: "scale"
                from: 3; to: 1
                duration: screenplayEditorSettings.enableAnimations ? 1000 : 0
            }

            PauseAnimation {
                duration: 5000
            }

            ScriptAction {
                script: {
                    openFromLibraryIntro.visible = false
                    openingAnimationLoader.active = false
                    workspaceSettings.scriptalayIntroduced = true
                }
            }
        }

        Item {
            Rectangle {
                id: openFromLibraryIntro
                anchors.top: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: openFromLibraryIntroText.width + 40
                height: openFromLibraryIntroText.height + 40
                color: primaryColors.c600.background
                visible: false

                Text {
                    id: openFromLibraryIntroText
                    anchors.centerIn: parent
                    width: 250
                    text: "Introducing <strong>Scriptalay</strong>! Click this button to browse through and download from a repository of screenpalys in Scrite format."
                    wrapMode: Text.WordWrap
                    font.pointSize: app.idealFontPointSize
                    color: primaryColors.c600.text
                }
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
        ShortcutsModelItem.enabled: qmlWindow.activeFocusItem !== null
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
}
