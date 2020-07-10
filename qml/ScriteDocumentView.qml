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
    }

    Settings {
        id: screenplayEditorSettings
        fileName: app.settingsFilePath
        category: "Screenplay Editor"
        property bool displaySceneCharacters: true
        property int mainEditorZoomValue: -1
        property int embeddedEditorZoomValue: -1
        property bool includeTitlePageInPreview: true
        property bool enableSpellCheck: true
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

    Rectangle {
        id: appToolBarArea
        anchors.left: parent.left
        anchors.right: parent.right
        height: appToolBar.height + 10
        color: primaryColors.c50.background

        Item {
            id: appToolBar

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            height: appFileTools.height

            function saveQuestionText() {
                if(scriteDocument.fileName === "")
                    return "Do you want to save this document first?"
                return "Do you want to save changes to <strong>" + app.fileName(scriteDocument.fileName) + "</strong> first?"
            }

            Row {
                id: appFileTools
                spacing: 2

                ToolButton3 {
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
                            }, this)
                        else
                            resetContentAnimation.start()
                    }
                }

                ToolButton3 {
                    id: fileOpenButton
                    iconSource: "../icons/file/folder_open.png"
                    text: "Open"
                    shortcut: "Ctrl+O"
                    shortcutText: "O"
                    down: recentFilesMenu.visible
                    onClicked: recentFilesMenu.recentFiles.length > 0 ? recentFilesMenu.open() : doOpen()
                    function doOpen(filePath) {
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
                                }, this)
                        else {
                            recentFilesMenu.close()
                            fileDialog.launch("OPEN", filePath)
                        }
                    }

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

                            MenuItem2 {
                                text: "Open Another"
                                onClicked: fileOpenButton.doOpen()
                            }

                            MenuSeparator {
                                visible: true
                            }

                            FontMetrics {
                                id: recentFilesFontMetrics
                            }

                            onAboutToShow: {
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
                }

                ToolButton3 {
                    text: "Save As"
                    shortcut: "Ctrl+Shift+S"
                    shortcutText: "Shift+S"
                    iconSource: "../icons/content/archive.png"
                    enabled: scriteDocument.structure.elementCount > 0
                    onClicked: fileDialog.launch("SAVE")
                    visible: documentUI.width > 1460
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
                    enabled: app.canUndo
                    onClicked: app.undoGroup.undo()
                    ToolTip.text: "Undo" + "\t" + app.polishShortcutTextForDisplay(shortcut)
                }

                ToolButton3 {
                    shortcut: app.isMacOSPlatform ? "Ctrl+Shift+Z" : "Ctrl+Y"
                    shortcutText: app.isMacOSPlatform ? "Shift+Z" : "Y"
                    iconSource: "../icons/content/redo.png"
                    enabled: app.canRedo
                    onClicked: app.undoGroup.redo()
                    ToolTip.text: "Redo" + "\t" + app.polishShortcutTextForDisplay(shortcut)
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
                                        text: modelData
                                        onClicked: {
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
                                                    }, this)
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
                                        onClicked: exportTimer.formatName = format
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

                                        onTriggered: {
                                            reportGeneratorTimer.reportArgs = modelData.name
                                        }
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
                    iconSource: "../icons/action/settings_applications.png"
                    text: "Settings"
                    shortcut: "Ctrl+,"
                    shortcutText: ","
                    onClicked: {
                        modalDialog.popupSource = this
                        modalDialog.sourceComponent = optionsDialogComponent
                        modalDialog.active = true
                    }
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
                        anchors.horizontalCenter: parent.horizontalCenter

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
        }

        Row {
            anchors.right: parent.right
            height: parent.height

            ScreenplayEditorToolbar {
                id: globalScreenplayEditorToolbar
                property Item sceneEditor
                readonly property bool editInFullscreen: true
                anchors.verticalCenter: parent.verticalCenter
                binder: sceneEditor ? sceneEditor.binder : null
                editor: sceneEditor ? sceneEditor.editor : null
            }

            Item {
                height: parent.height
                width: appToolBarArea.width * 0.01
            }

            // We move the main-tab bar here based on UI/UX suggestions from Surya Vasishta
            Row {
                id: mainTabBar
                height: parent.height

                property Item currentTab: currentIndex >= 0 ? mainTabBarRepeater.itemAt(currentIndex) : null
                property int currentIndex: -1
                property var tabs: ["Screenplay", "Structure", "Notebook"]
                property var currentTabP1: currentTabExtents.value.p1
                property var currentTabP2: currentTabExtents.value.p2
                property color activeTabColor: primaryColors.windowColor

                ResetOnChange {
                    id: currentTabExtents
                    trackChangesOn: appToolBarArea.width
                    from: {
                        "p1": { "x": 0, "y": 0 },
                        "p2": { "x": 0, "y": 0 }
                    }
                    to: {
                        "p1": mainTabBar.mapFromItem(mainTabBar.currentTab, 0, 0),
                        "p2": mainTabBar.mapFromItem(mainTabBar.currentTab, mainTabBar.currentTab.width, 0)
                    }
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

            Item {
                height: parent.height
                width: appToolBarArea.width * 0.01
            }

            Image {
                id: appLogo
                source: "../images/teriflix_logo.png"
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
        active: true
        sourceComponent: uiLayoutComponent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: appToolBarArea.bottom
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

    Loader {
        id: openingAnimationLoader
        active: splashLoader.active === false
        sourceComponent: SequentialAnimation {
            running: true

            PauseAnimation {
                duration: 500
            }

            ScriptAction {
                script: appLogo.ToolTip.visible = true
            }

            PropertyAnimation {
                target: appLogo
                properties: "scale"
                from: 1; to: 1.5
                duration: 1000
            }

            PropertyAnimation {
                target: appLogo
                properties: "scale"
                from: 1.5; to: 1
                duration: 1000
            }

            ScriptAction {
                script: {
                    appLogo.ToolTip.visible = false
                    openingAnimationLoader.active = false
                }
            }
        }
    }

    property bool handleCloseEvent: true
    Connections {
        target: qmlWindow
        onClosing: {
            if(handleCloseEvent) {
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
                        handleCloseEvent = false
                        qmlWindow.close()
                    }
                }, documentUI)
            } else
                close.accepted = true
        }
    }
}
