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

import Scrite 1.0

Item {
    id: documentUI
    width: 1470
    height: 865

    // onWidthChanged: console.log(width)

    Rectangle {
        id: appToolBarArea
        anchors.left: parent.left
        anchors.right: parent.right
        height: appToolBar.height + 10
        color: "lightgray"

        ToolBar {
            id: appToolBar

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            background: Rectangle {
                color: appToolBarArea.color
            }

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
                                    fileDialog.launch("OPEN", filePath)
                                }
                            }, this)
                        else
                            fileDialog.launch("OPEN", filePath)
                    }

                    Item {
                        anchors.top: parent.bottom
                        anchors.left: parent.left

                        Settings {
                            fileName: app.settingsFilePath
                            category: "RecentFiles"
                            property alias files: recentFilesMenu.recentFiles
                        }

                        Menu {
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

                            MenuItem {
                                text: "Open Another"
                                onClicked: fileOpenButton.doOpen()
                            }

                            MenuSeparator {
                                visible: true
                            }

                            Menu {
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

                                    MenuItem {
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
                    icon.source: "../icons/file/file_download.png"
                    text: "Import"
                    shortcut: "Ctrl+Shift+I"
                    shortcutText: "Shift+I"
                    display: AbstractButton.IconOnly
                    down: importMenu.visible
                    onClicked: importMenu.visible = true

                    Item {
                        anchors.top: parent.bottom
                        anchors.left: parent.left

                        Menu {
                            id: importMenu

                            Repeater {
                                model: scriteDocument.supportedImportFormats

                                MenuItem {
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
                    }
                }

                ToolButton2 {
                    icon.source: "../icons/file/file_upload.png"
                    text: "Export"
                    shortcut: "Ctrl+Shift+X"
                    shortcutText: "Shift+X"
                    display: AbstractButton.IconOnly
                    down: exportMenu.visible
                    enabled: scriteDocument.screenplay.elementCount > 0
                    onClicked: exportMenu.visible = true

                    Item {
                        anchors.top: parent.bottom
                        anchors.left: parent.left

                        Menu {
                            id: exportMenu

                            Repeater {
                                model: scriteDocument.supportedExportFormats

                                MenuItem {
                                    text: modelData
                                    onClicked: fileDialog.launch("EXPORT " + modelData)
                                }
                            }
                        }
                    }
                }

                ToolButton2 {
                    id: reportsButton
                    icon.source: "../icons/file/file_pdf.png"
                    text: "Reports"
                    shortcut: "Ctrl+Shift+R"
                    shortcutText: "Shift+R"
                    display: AbstractButton.IconOnly
                    down: reportsMenu.visible
                    enabled: scriteDocument.screenplay.elementCount > 0
                    onClicked: reportsMenu.visible = true

                    Item {
                        anchors.top: parent.bottom
                        anchors.left: parent.left

                        Menu {
                            id: reportsMenu

                            Repeater {
                                model: scriteDocument.supportedReports

                                MenuItem {
                                    text: modelData
                                    onTriggered: {
                                        reportGeneratorTimer.reportName = modelData
                                    }
                                }
                            }
                        }

                        Timer {
                            id: reportGeneratorTimer
                            property string reportName
                            repeat: false
                            interval: 10
                            onReportNameChanged: {
                                if(reportName !== "")
                                    start()
                            }
                            onTriggered: {
                                if(reportName !== "")
                                    scriteDocument.generateReport(reportName)
                                reportName = ""
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
                    text: app.transliterationSettings.languageAsString
                    shortcut: "Ctrl+L"
                    shortcutText: "L"
                    // display: AbstractButton.IconOnly
                    ToolTip.text: app.polishShortcutTextForDisplay("Language Transliteration" + "\t" + shortcut)
                    onClicked: languageMenu.visible = true
                    down: languageMenu.visible

                    Item {
                        anchors.top: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter

                        Menu {
                            id: languageMenu
                            width: 250

                            Repeater {
                                model: app.enumerationModel(app.transliterationSettings, "Language")

                                MenuItem {
                                    property string baseText: modelData.key
                                    property string shortcutKey: app.transliterationSettings.shortcutLetter(modelData.value)
                                    text: baseText + " (" + app.polishShortcutTextForDisplay("Alt+"+shortcutKey) + ")"
                                    onClicked: app.transliterationSettings.language = modelData.value
                                    checkable: true
                                    checked: app.transliterationSettings.language === modelData.value
                                }
                            }

                            MenuSeparator { }

                            MenuItem {
                                text: "Next-Language (F10)"
                                checkable: true
                                checked: false
                                onClicked: app.transliterationSettings.cycleLanguage()
                            }
                        }

                        Repeater {
                            model: app.enumerationModel(app.transliterationSettings, "Language")

                            Item {
                                Shortcut {
                                    property string shortcutKey: app.transliterationSettings.shortcutLetter(modelData.value)
                                    context: Qt.ApplicationShortcut
                                    sequence: "Alt+"+shortcutKey
                                    onActivated: app.transliterationSettings.language = modelData.value
                                }
                            }
                        }

                        Shortcut {
                            context: Qt.ApplicationShortcut
                            sequence: "F10"
                            onActivated: app.transliterationSettings.cycleLanguage()
                        }
                    }
                }
            }
        }

        Item {
            anchors.right: parent.right
            width: parent.width - appFileTools.width - 20
            height: parent.height

            Rectangle {
                anchors.fill: globalSceneEditorToolbar
                anchors.margins: -5
                opacity: 0.25
                radius: 8
                border { width: 1; color: "black" }
                color: globalSceneEditorToolbar.enabled ? "white" : "black"
            }

            SceneEditorToolbar {
                id: globalSceneEditorToolbar
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: appLogo.left
                binder: sceneEditor ? sceneEditor.binder : null
                editor: sceneEditor ? sceneEditor.editor : null
                // enabled: sceneEditor ? true : false
                property Item sceneEditor
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
        sourceComponent: documentUiContentComponent
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

        PropertyAnimation {
            target: contentLoader
            properties: "opacity"
            from: 1; to: 0
            duration: 100
        }

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

        PropertyAnimation {
            target: contentLoader
            properties: "opacity"
            from: 0; to: 1
            duration: 100
        }
    }

    Component {
        id: documentUiContentComponent

        SplitView {
            orientation: Qt.Vertical

            Settings {
                id: workspaceSettings
                fileName: app.settingsFilePath
                category: "Workspace"
                property var workspaceHeight
                property var structureEditorWidth
            }

            Rectangle {
                SplitView.preferredHeight: workspaceSettings.workspaceHeight ? documentUI.height*workspaceSettings.workspaceHeight : documentUI.height*0.75
                SplitView.minimumHeight: documentUI.height * 0.5
                onHeightChanged: workspaceSettings.workspaceHeight = height/documentUI.height

                SplitView {
                    anchors.fill: parent
                    orientation: Qt.Horizontal

                    Rectangle {
                        SplitView.preferredWidth: workspaceSettings.structureEditorWidth ? documentUI.width*workspaceSettings.structureEditorWidth : documentUI.width*0.4
                        color: "lightgray"
                        onWidthChanged: workspaceSettings.structureEditorWidth = width/documentUI.width

                        Rectangle {
                            id: structureEditor
                            anchors.fill: parent
                            anchors.margins: 2
                            border { width: 1; color: "gray" }
                            radius: 5
                            color: Qt.rgba(1,1,1,0.5)

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
                                    color: "gray"
                                }

                                Row {
                                    height: parent.height
                                    anchors.centerIn: parent

                                    Repeater {
                                        id: structureEditorTabGenerator
                                        model: structureEditor.tabs

                                        Item {
                                            id: tabItem
                                            width: tabLabel.width + 120
                                            height: structureEditorTabs.height
                                            property bool isActiveTab: structureEditorTabs.currentIndex === index

                                            PainterPathItem {
                                                anchors.fill: parent
                                                anchors.margins: isActiveTab ? 0 : 1
                                                fillColor: isActiveTab ? "white" : "lightgray"
                                                outlineColor: "gray"
                                                outlineWidth: 2
                                                renderingMechanism: PainterPathItem.UseQPainter
                                                renderType: isActiveTab ? PainterPathItem.FillOnly : PainterPathItem.OutlineAndFill
                                                antialiasing: true
                                                painterPath: PainterPath {
                                                    id: tabPath
                                                    property real radius: Math.min(itemRect.width, itemRect.height)*0.2
                                                    property point c1: Qt.point(itemRect.left+itemRect.width*0.1, itemRect.top+1)
                                                    property point c2: Qt.point(itemRect.right-1-itemRect.width*0.1, itemRect.top+1)

                                                    property point p1: Qt.point(itemRect.left, itemRect.bottom)
                                                    property point p2: pointInLine(c1, p1, radius, true)
                                                    property point p3: pointInLine(c1, c2, radius, true)
                                                    property point p4: pointInLine(c2, c1, radius, true)
                                                    property point p5: pointInLine(c2, p6, radius, true)
                                                    property point p6: Qt.point(itemRect.right-1, itemRect.bottom)

                                                    MoveTo { x: tabPath.p1.x; y: tabPath.p1.y }
                                                    LineTo { x: tabPath.p2.x; y: tabPath.p2.y }
                                                    QuadTo { controlPoint: tabPath.c1; endPoint: tabPath.p3 }
                                                    LineTo { x: tabPath.p4.x; y: tabPath.p4.y }
                                                    QuadTo { controlPoint: tabPath.c2; endPoint: tabPath.p5 }
                                                    LineTo { x: tabPath.p6.x; y: tabPath.p6.y }
                                                    CloseSubpath { }
                                                }

                                                Text {
                                                    id: tabLabel
                                                    text: modelData
                                                    anchors.centerIn: parent
                                                    font.pixelSize: isActiveTab ? 16 : 14
                                                    font.bold: isActiveTab
                                                }

                                                Rectangle {
                                                    width: parent.width-3
                                                    height: 2
                                                    anchors.horizontalCenter: parent.horizontalCenter
                                                    anchors.verticalCenter: parent.bottom
                                                    color: "white"
                                                    visible: isActiveTab
                                                }
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: structureEditorTabs.currentIndex = index
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                anchors.fill: structureEditorContent
                                anchors.margins: -1
                                border { width: 1; color: "lightgray" }
                                radius: 5
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
                        color: "lightgray"

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
                                renderType: Text.NativeRendering
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
                ScreenplayView {
                    id: screenplayView
                    anchors.fill: parent
                    anchors.margins: 5
                    border { width: 1; color: "lightgray" }
                    radius: 5
                    onRequestEditor: editorLoader.sourceComponent = screenplayEditorComponent
                }
            }
        }
    }

    Component {
        id: screenplayEditorComponent

        Rectangle {
            id: screenplayEditorItem
            clip: true
            color: "lightgray"

            ScreenplayEditor {
                id: screenplayEditor
                anchors.fill: parent
                onCurrentSceneEditorChanged: globalSceneEditorToolbar.sceneEditor = currentSceneEditor
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

            FocusIndicator {
                id: focusIndicator
                active: mainUndoStack.active
                anchors.fill: sceneEditor
                anchors.margins: -3
            }

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

            scriteDocument.supportedExportFormats.forEach(function(format) {
                availableModes["EXPORT " + format] = {
                    "nameFilters": scriteDocument.exportFormatFileSuffix(format),
                    "selectExisting": false,
                    "callback": function(path) {
                        scriteDocument.exportFile(path, format)
                        app.revealFileOnDesktop(path)
                    },
                    "reset": false,
                    "notificationTitle": "Exporting Scrite project to " + format
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

    Connections {
        target: scriteDocument
        onGenerateReportRequest: {
            modalDialog.closeable = false
            modalDialog.arguments = reportName
            modalDialog.sourceComponent = reportGeneratorConfigurationComponent
            modalDialog.popupSource = reportsButton
            modalDialog.active = true
        }
    }

    Component {
        id: reportGeneratorConfigurationComponent

        ReportGeneratorConfiguration { }
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
}
