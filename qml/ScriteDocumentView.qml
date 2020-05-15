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
    width: 1470
    height: 865

    // onWidthChanged: console.log(width)

    FontMetrics {
        id: sceneEditorFontMetrics
        readonly property SceneElementFormat format: scriteDocument.formatting.elementFormat(SceneElement.Action)
        readonly property int lettersPerLine: globalSceneEditorToolbar.editInFullscreen ? 70 : 60
        readonly property int marginLetters: 5
        readonly property real paragraphWidth: Math.ceil(lettersPerLine*averageCharacterWidth)
        readonly property real paragraphMargin: Math.ceil(marginLetters*averageCharacterWidth)
        readonly property real pageWidth: Math.ceil(paragraphWidth + 2*paragraphMargin)

        Component.onCompleted: {
            font = format.font
            font.pointSize = font.pointSize + scriteDocument.formatting.fontPointSizeDelta
        }
    }

    Settings {
        id: workspaceSettings
        fileName: app.settingsFilePath
        category: "Workspace"
        property var workspaceHeight
        property var structureEditorWidth
        property bool editInFullscreen: true
    }

    Rectangle {
        id: appToolBarArea
        anchors.left: parent.left
        anchors.right: parent.right
        height: appToolBar.height + 10
        // color: primaryColors.windowColor
        gradient: Gradient {
            GradientStop { position: 0; color: primaryColors.c50.background }
            GradientStop { position: 1; color: primaryColors.windowColor }
        }

        Item {
            id: appToolBar

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            height: appFileTools.height

            Row {
                id: appFileTools
                spacing: 5

                ToolButton2 {
                    icon.source: "../icons/action/description.png"
                    text: "New"
                    shortcut: "Ctrl+N"
                    shortcutText: "N"
                    display: AbstractButton.IconOnly
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
                                                resetContentAnimation.start()
                                            }
                                        }, this)
                        else
                            resetContentAnimation.start()
                    }
                }

                ToolButton2 {
                    id: fileOpenButton
                    icon.source: "../icons/file/folder_open.png"
                    text: "Open"
                    shortcut: "Ctrl+O"
                    shortcutText: "O"
                    display: documentUI.width > 1590 ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly
                    down: recentFilesMenu.visible
                    onClicked: recentFilesMenu.recentFiles.length > 0 ? recentFilesMenu.open() : doOpen()
                    function doOpen(filePath) {
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

                            Menu2 {
                                id: recentFilesSubMenu
                                title: "Recent Files"
                                width: 400

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
                                        text: recentFilesFontMetrics.elidedText("" + (index+1) + ". " + app.fileInfo(filePath).baseName, Qt.ElideMiddle, recentFilesSubMenu.width)
                                        ToolTip.text: filePath
                                        ToolTip.visible: hovered
                                        onClicked: fileOpenButton.doOpen(filePath)
                                    }
                                }
                            }
                        }
                    }
                }

                ToolButton2 {
                    id: cmdSave
                    icon.source: "../icons/content/save.png"
                    text: "Save"
                    shortcut: "Ctrl+S"
                    shortcutText: "S"
                    display: AbstractButton.IconOnly
                    enabled: scriteDocument.structure.elementCount > 0
                    onClicked: doClick()
                    function doClick() {
                        if(scriteDocument.fileName === "")
                            fileDialog.launch("SAVE")
                        else
                            scriteDocument.save()
                    }
                }

                ToolButton2 {
                    display: AbstractButton.TextBesideIcon
                    text: "Save As"
                    shortcut: "Ctrl+Shift+S"
                    shortcutText: "Shift+S"
                    enabled: scriteDocument.structure.elementCount > 0
                    onClicked: fileDialog.launch("SAVE")
                    visible: documentUI.width > 1460
                }

                Rectangle {
                    width: 1
                    height: parent.height
                    color: app.palette.mid
                }

                ToolButton2 {
                    shortcut: "Ctrl+Z"
                    shortcutText: "Z"
                    icon.source: "../icons/content/undo.png"
                    enabled: app.canUndo
                    onClicked: app.undoGroup.undo()
                    ToolTip.text: app.undoText + "\t" + app.polishShortcutTextForDisplay(shortcut)
                }

                ToolButton2 {
                    shortcut: app.isMacOSPlatform ? "Ctrl+Shift+Z" : "Ctrl+Y"
                    shortcutText: app.isMacOSPlatform ? "Shift+Z" : "Y"
                    icon.source: "../icons/content/redo.png"
                    enabled: app.canRedo
                    onClicked: app.undoGroup.redo()
                    ToolTip.text: app.redoText + "\t" + app.polishShortcutTextForDisplay(shortcut)
                }

                Rectangle {
                    width: 1
                    height: parent.height
                    color: app.palette.mid
                }

                ToolButton2 {
                    id: importExportButton
                    icon.source: "../icons/file/import_export.png"
                    text: "Import, Export & Reports"
                    display: AbstractButton.IconOnly
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
                                width: 250

                                Repeater {
                                    model: scriteDocument.supportedReports

                                    MenuItem2 {
                                        text: modelData
                                        onTriggered: {
                                            reportGeneratorTimer.reportArgs = modelData
                                        }
                                    }
                                }
                            }
                        }

                        Timer {
                            id: exportTimer
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
                            property var reportArgs
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
                                    modalDialog.popupSource = importExportButton
                                    modalDialog.active = true
                                }
                                reportArgs = ""
                            }
                        }
                    }
                }

                Rectangle {
                    width: 1
                    height: parent.height
                    color: app.palette.mid
                }

                ToolButton2 {
                    icon.source: "../icons/action/settings_applications.png"
                    text: "Settings"
                    shortcut: "Ctrl+,"
                    shortcutText: ","
                    display: documentUI.width > 1528 ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly
                    onClicked: {
                        modalDialog.popupSource = this
                        modalDialog.sourceComponent = optionsDialogComponent
                        modalDialog.active = true
                    }
                }

                ToolButton2 {
                    icon.source: "../icons/content/language.png"
                    text: app.transliterationEngine.languageAsString
                    shortcut: "Ctrl+L"
                    shortcutText: "L"
                    // display: AbstractButton.IconOnly
                    ToolTip.text: app.polishShortcutTextForDisplay("Language Transliteration" + "\t" + shortcut)
                    onClicked: languageMenu.visible = true
                    down: languageMenu.visible

                    Item {
                        anchors.top: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter

                        Menu2 {
                            id: languageMenu
                            width: 250

                            Repeater {
                                model: app.enumerationModel(app.transliterationEngine, "Language")

                                MenuItem2 {
                                    property string baseText: modelData.key
                                    property string shortcutKey: app.transliterationEngine.shortcutLetter(modelData.value)
                                    text: baseText + " (" + app.polishShortcutTextForDisplay("Alt+"+shortcutKey) + ")"
                                    onClicked: app.transliterationEngine.language = modelData.value
                                    checkable: true
                                    checked: app.transliterationEngine.language === modelData.value
                                }
                            }

                            MenuSeparator { }

                            MenuItem2 {
                                text: "Next-Language (F10)"
                                checkable: true
                                checked: false
                                onClicked: app.transliterationEngine.cycleLanguage()
                            }
                        }

                        Repeater {
                            model: app.enumerationModel(app.transliterationEngine, "Language")

                            Item {
                                Shortcut {
                                    property string shortcutKey: app.transliterationEngine.shortcutLetter(modelData.value)
                                    context: Qt.ApplicationShortcut
                                    sequence: "Alt+"+shortcutKey
                                    onActivated: app.transliterationEngine.language = modelData.value
                                }
                            }
                        }

                        Shortcut {
                            context: Qt.ApplicationShortcut
                            sequence: "F10"
                            onActivated: app.transliterationEngine.cycleLanguage()
                        }
                    }
                }

                ToolButton2 {
                    icon.source: down ? "../icons/hardware/keyboard_hide.png" : "../icons/hardware/keyboard.png"
                    ToolTip.text: "Show English to " + app.transliterationEngine.languageAsString + " alphabet mappings.\t" + app.polishShortcutTextForDisplay(shortcut)
                    shortcut: "Ctrl+K"
                    shortcutText: "K"
                    display: AbstractButton.IconOnly
                    onClicked: alphabetMappingsPopup.visible = true
                    down: alphabetMappingsPopup.visible
                    visible: app.transliterationEngine.language !== TransliterationEngine.English
                    enabled: visible

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
            }
        }

        Item {
            anchors.right: parent.right
            width: parent.width - appFileTools.width - 20
            height: parent.height

            SceneEditorToolbar {
                id: globalSceneEditorToolbar
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: appLogo.left
                binder: sceneEditor ? sceneEditor.binder : null
                editor: sceneEditor ? sceneEditor.editor : null
                property Item sceneEditor
                editInFullscreen: workspaceSettings.editInFullscreen
                onEditInFullscreenChanged: workspaceSettings.editInFullscreen = editInFullscreen
            }

            Image {
                id: appLogo
                source: "../images/teriflix_logo.png"
                height: parent.height
                smooth: true
                fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
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
        sourceComponent: globalSceneEditorToolbar.editInFullscreen ? uiLayout2Component : uiLayout1Component
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: appToolBarArea.bottom
        anchors.bottom: parent.bottom
        onActiveChanged: {
            globalSceneEditorToolbar.sceneEditor = null
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
        id: uiLayout1Component

        SplitView {
            orientation: Qt.Vertical
            Material.background: Qt.darker(primaryColors.windowColor, 1.1)

            Item {
                SplitView.preferredHeight: workspaceSettings.workspaceHeight ? documentUI.height*workspaceSettings.workspaceHeight : documentUI.height*0.75
                SplitView.minimumHeight: documentUI.height * 0.5
                onHeightChanged: workspaceSettings.workspaceHeight = height/documentUI.height

                SplitView {
                    anchors.fill: parent
                    anchors.margins: 2
                    orientation: Qt.Horizontal

                    Rectangle {
                        SplitView.preferredWidth: workspaceSettings.structureEditorWidth ? documentUI.width*workspaceSettings.structureEditorWidth : documentUI.width*0.4
                        onWidthChanged: workspaceSettings.structureEditorWidth = width/documentUI.width
                        border {
                            width: 1
                            color: primaryColors.borderColor
                        }
                        radius: 5
                        color: primaryColors.windowColor

                        Item {
                            id: structureEditor
                            anchors.fill: parent

                            property var tabs: ["Structure", "Notebook"]

                            Item {
                                id: structureEditorTabs
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.margins: 5
                                height: 28
                                property int currentIndex: 0

                                Rectangle {
                                    width: parent.width
                                    height: 2
                                    anchors.bottom: parent.bottom
                                    color: primaryColors.borderColor
                                }

                                Row {
                                    height: parent.height
                                    anchors.centerIn: parent
                                    spacing: -height*0.75

                                    Repeater {
                                        id: structureEditorTabGenerator
                                        model: structureEditor.tabs

                                        TabBarTab {
                                            id: tabItem
                                            text: modelData
                                            width: tabTextWidth + 120
                                            height: structureEditorTabs.height
                                            tabIndex: index
                                            tabCount: structureEditor.tabs.length
                                            currentTabIndex: structureEditorTabs.currentIndex
                                            onRequestActivation: structureEditorTabs.currentIndex = index
                                        }
                                    }
                                }
                            }

                            SwipeView {
                                id: structureEditorContent
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: structureEditorTabs.bottom
                                anchors.bottom: parent.bottom
                                anchors.margins: 5
                                clip: true

                                interactive: false
                                currentIndex: structureEditorTabs.currentIndex

                                StructureView {
                                    id: structureView
                                    onRequestEditor: {
                                        if(scriteDocument.structure.currentElementIndex >= 0)
                                            editorLoader.sourceComponent = sceneEditorComponent
                                        else
                                            editorLoader.sourceComponent = screenplayEditorComponent
                                    }
                                    onReleaseEditor: editorLoader.sourceComponent = screenplayEditorComponent
                                }

                                NotebookView {
                                    id: notebookView
                                }
                            }
                        }
                    }

                    Rectangle {
                        SplitView.preferredWidth: documentUI.width * 0.6
                        color: primaryColors.windowColor
                        border {
                            width: 1
                            color: primaryColors.borderColor
                        }
                        radius: 5

                        Loader {
                            width: parent.width*0.7
                            anchors.centerIn: parent
                            active: editorLoader.item == null
                            sourceComponent: TextArea {
                                readOnly: true
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                                font.pixelSize: 30
                                enabled: false
                                // renderType: Text.NativeRendering
                                text: "Select a scene on the structure canvas or in the timeline to edit its content here."
                            }
                        }

                        Loader {
                            id: editorLoader
                            anchors.fill: parent
                            sourceComponent: scriteDocument.screenplay.elementCount > 0 ? screenplayEditorComponent : null
                            property bool emptyDocument: scriteDocument.structure.elementCount === 0
                            onEmptyDocumentChanged: {
                                if(emptyDocument)
                                    sourceComponent = null
                            }
                        }

                        Connections {
                            target: globalSceneEditorToolbar
                            onRequestScreenplayEditor: editorLoader.sourceComponent = screenplayEditorComponent
                        }
                    }
                }
            }

            Item {
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    color: accentColors.c200.background
                    border { width: 1; color: accentColors.borderColor }
                    radius: 6

                    ScreenplayView {
                        id: screenplayView
                        anchors.fill: parent
                        anchors.margins: 5
                        onRequestEditor: editorLoader.sourceComponent = screenplayEditorComponent
                    }
                }
            }
        }
    }

    Component {
        id: uiLayout2Component

        Item {
            Item {
                id: uiLayout2TabBar
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 5
                height: 28
                property int currentIndex: 0
                property var tabs: ["Screenplay Only", "Structure & Timeline", "Notebook"]

                Rectangle {
                    width: parent.width
                    height: 2
                    anchors.bottom: parent.bottom
                    color: primaryColors.borderColor
                }

                Row {
                    height: parent.height
                    anchors.centerIn: parent
                    spacing: -height*0.75

                    Repeater {
                        model: uiLayout2TabBar.tabs

                        TabBarTab {
                            id: tabItem
                            text: modelData
                            width: tabTextWidth + 120
                            height: uiLayout2TabBar.height
                            tabCount: uiLayout2TabBar.tabs.length
                            tabIndex: index
                            currentTabIndex: uiLayout2TabBar.currentIndex
                            onRequestActivation: uiLayout2TabBar.currentIndex = index
                        }
                    }
                }
            }

            SwipeView {
                id: uiLayout2TabView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: uiLayout2TabBar.bottom
                anchors.bottom: parent.bottom
                anchors.margins: 5
                clip: true

                interactive: false
                currentIndex: uiLayout2TabBar.currentIndex

                Loader {
                    sourceComponent: screenplayEditorComponent
                }

                SplitView {
                    orientation: Qt.Vertical
                    Material.background: Qt.darker(primaryColors.windowColor, 1.1)

                    Item {
                        SplitView.fillHeight: true

                        Rectangle {
                            anchors.fill: parent
                            color: primaryColors.c10.background
                            border {
                                width: 1
                                color: primaryColors.borderColor
                            }
                            radius: 4

                            StructureView {
                                anchors.fill: parent
                                anchors.margins: 2

                                onRequestEditor: {
                                    if(scriteDocument.structure.currentElementIndex >= 0) {
                                        var selement = scriteDocument.structure.elementAt(scriteDocument.structure.currentElementIndex)
                                        var index = scriteDocument.screenplay.firstIndexOfScene(selement.scene)
                                        scriteDocument.screenplay.currentElementIndex = index
                                    }
                                }
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
                            radius: 6

                            ScreenplayView {
                                id: screenplayView
                                anchors.fill: parent
                                anchors.margins: 5
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
                            radius: 6

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

        Rectangle {
            id: screenplayEditorItem
            clip: true
            color: globalSceneEditorToolbar.editInFullscreen && scriteDocument.screenplay.elementCount === 0 ? primaryColors.windowColor : primaryColors.c50.background

            Loader {
                width: parent.width*0.7
                anchors.centerIn: parent
                active: globalSceneEditorToolbar.editInFullscreen && scriteDocument.screenplay.elementCount === 0
                sourceComponent: TextArea {
                    readOnly: true
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 30
                    enabled: false
                    // renderType: Text.NativeRendering
                    text: "Click on the add new scene button on the toolbar or press " + app.polishShortcutTextForDisplay("Ctrl+Shift+N") + " to create a new scene."
                }
            }

            ScreenplayEditor {
                id: screenplayEditor
                anchors.fill: parent
                onCurrentSceneEditorChanged: globalSceneEditorToolbar.sceneEditor = currentSceneEditor
                displaySceneNumbers: globalSceneEditorToolbar.editInFullscreen
                displaySceneMenu: globalSceneEditorToolbar.editInFullscreen
            }
        }
    }

    Component {
        id: sceneEditorComponent

        Rectangle {
            id: sceneEditorView
            color: sceneEditor.backgroundColor

            SearchBar {
                id: searchBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
            }

//            FocusIndicator {
//                id: focusIndicator
//                active: mainUndoStack.active
//                anchors.fill: sceneEditor
//                anchors.margins: -3
//            }

            SceneEditor {
                id: sceneEditor
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.top: searchBar.bottom
                anchors.margins: 3
                clip: true
                property StructureElement element: scriteDocument.structure.elementAt(scriteDocument.structure.currentElementIndex)
                scene: element ? element.scene : null

                onSplitSceneRequest: {
                    showInformation({
                                        "message": "You can split a scene only when its edited in the context of a screenplay, not when edited as an independent scene.",
                                    }, this)
                }

                SearchAgent.engine: searchBar.searchEngine
                SearchAgent.textDocument: editor.textDocument
                SearchAgent.onHighlightText: {
                    editor.cursorPosition = start
                    editor.select(start, end)
                }
                SearchAgent.onClearSearchRequest: {
                    editor.deselect()
                }

                FocusTracker.window: qmlWindow
                FocusTracker.indicator.target: mainUndoStack
                FocusTracker.indicator.property: "sceneEditorActive"
            }

            Component.onCompleted: globalSceneEditorToolbar.sceneEditor = sceneEditor
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
                processFile(filePath)
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
        active: true
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
