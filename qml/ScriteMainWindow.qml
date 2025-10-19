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

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/scrited"
import "qrc:/qml/controls"
import "qrc:/qml/mainwindow"
import "qrc:/qml/notifications"
import "qrc:/qml/screenplayeditor"
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
            Runtime.execLater(mainTabBar, 100, function() {
                mainTabBar.currentIndex = mainTabBar.currentIndex % (Runtime.showNotebookInStructure ? 2 : 3)
            })
        }
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+1"
        onActivated: activate()

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Screenplay"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        function activate() {
            Runtime.activateMainWindowTab(0)
        }
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+2"
        onActivated: activate()

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Structure"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        function activate() {
            Runtime.activateMainWindowTab(1)
            if(Runtime.showNotebookInStructure)
                Announcement.shout(Runtime.announcementIds.tabRequest, "Structure")
        }
    }

    Shortcut {
        id: notebookShortcut

        property bool notebookTabVisible: mainTabBar.currentIndex === (Runtime.showNotebookInStructure ? 1 : 2)

        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Alt+3"
        onActivated: activate()

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Notebook"
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        function activate() {
            if(Runtime.showNotebookInStructure) {
                if(Runtime.mainWindowTab === Runtime.MainWindowTab.StructureTab)
                    Announcement.shout(Runtime.announcementIds.tabRequest, "Notebook")
                else {
                    Runtime.activateMainWindowTab(Runtime.MainWindowTab.StructureTab)
                    Runtime.execLater(mainTabBar, 250, function() {
                        Announcement.shout(Runtime.announcementIds.tabRequest, "Notebook")
                    })
                }
            } else
                Runtime.activateMainWindowTab(Runtime.MainWindowTab.NotebookTab)
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
                Runtime.execLater(mainTabBar, 250, function() {
                    Announcement.shout(Runtime.announcementIds.tabRequest, type)
                })
            } else
                Announcement.shout(Runtime.announcementIds.tabRequest, type)
        }
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+K"
        onActivated: activate()

        ShortcutsModelItem.group: notebookShortcut.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Bookmarked Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        function activate() {
            notebookShortcut.showBookmarkedNotes()
        }
    }

    Shortcut {
        enabled: Runtime.allowAppUsage
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+R"
        onActivated: activate()

        ShortcutsModelItem.group: notebookShortcut.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Charater Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        function activate() {
            notebookShortcut.showCharacterNotes()
        }
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        enabled: Runtime.allowAppUsage
        sequence: "Ctrl+Shift+Y"
        onActivated: activate()

        ShortcutsModelItem.group: notebookShortcut.notebookTabVisible ? "Notebook" : "Application"
        ShortcutsModelItem.title: "Story Notes"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        function activate() {
            notebookShortcut.showStoryNotes()
        }
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        enabled: Runtime.workspaceSettings.showScritedTab && Runtime.allowAppUsage
        sequence: "Alt+4"
        onActivated: activate()

        ShortcutsModelItem.group: "Application"
        ShortcutsModelItem.title: "Scrited"
        ShortcutsModelItem.enabled: enabled
        ShortcutsModelItem.shortcut: sequence
        ShortcutsModelItem.canActivate: true
        ShortcutsModelItem.onActivated: activate()

        function activate() {
            Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScritedTab)
        }
    }

    Connections {
        target: Scrite.document

        function onJustReset() {
            Runtime.screenplayEditorSettings.firstSwitchToStructureTab = true
            appBusyOverlay.ref()
            // Runtime.screenplayAdapter.initialLoadTreshold = 25
            reloadScriteDocumentTimer.stop()
            Runtime.execLater(Runtime.screenplayAdapter, 250, () => {
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
        id: _header
        anchors.left: parent.left
        anchors.right: parent.right

        z: 1
        height: 53
        color: Runtime.colors.primary.c50.background
        enabled: visible

        MainToolBar {
            id: _mainToolbar

            anchors.left: parent.left
            anchors.right: editTools.left
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            anchors.verticalCenter: parent.verticalCenter

            visible: Runtime.mainWindowTab !== Runtime.MainWindowTab.ScritedTab

            // onVisibleChanged: {
            //     if(enabled && !visible)
            //         Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScreenplayTab)
            // }
        }

        HelpTipNotification {
            tipName: Scrite.app.isWindowsPlatform ? "language_windows" : (Scrite.app.isMacOSPlatform ? "language_macos" : "language_linux")
            enabled: Runtime.language.activeCode !== QtLocale.English
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: _scritedToolbar.height

            ScritedToolbar {
                id: _scritedToolbar

                anchors.horizontalCenter: parent.horizontalCenter

                visible: Runtime.mainWindowTab === Runtime.MainWindowTab.ScritedTab
            }
        }

        Row {
            id: editTools

            x: _mainToolbar.visible ? (parent.width - userLogin.width - width) : (appToolsMenu.x + (parent.width - width - appToolsMenu.width - appToolsMenu.x)/2)
            height: parent.height

            spacing: 20

            Row {
                id: mainTabBar

                height: parent.height

                visible: _mainToolbar.visible

                readonly property var tabs: [
                    { "name": "Screenplay", "icon": "qrc:/icons/navigation/screenplay_tab.png", "visible": true, "tab": Runtime.MainWindowTab.ScreenplayTab },
                    { "name": "Structure", "icon": "qrc:/icons/navigation/structure_tab.png", "visible": true, "tab": Runtime.MainWindowTab.StructureTab },
                    { "name": "Notebook", "icon": "qrc:/icons/navigation/notebook_tab.png", "visible": !Runtime.showNotebookInStructure, "tab": Runtime.MainWindowTab.NotebookTab },
                    { "name": "Scrited", "icon": "qrc:/icons/navigation/scrited_tab.png", "visible": Runtime.workspaceSettings.showScritedTab, "tab": Runtime.MainWindowTab.ScritedTab }
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

                    function onJustReset() {
                        Runtime.activateMainWindowTab(0)
                    }

                    function onAboutToSave() {
                        let userData = Scrite.document.userData
                        userData["mainTabBar"] = {
                            "version": 0,
                            "currentIndex": mainTabBar.currentIndex
                        }
                        Scrite.document.userData = userData
                    }

                    function onJustLoaded() {
                        let userData = Scrite.document.userData
                        if(userData.mainTabBar) {
                            var ci = userData.mainTabBar.currentIndex
                            if(ci >= 0 && ci <= 2)
                                Runtime.activateMainWindowTab(ci)
                            else
                                Runtime.activateMainWindowTab(0)
                        } else
                            Runtime.activateMainWindowTab(0)
                    }
                }

                function activateTab(index) {
                    if(index < 0 || index >= tabs.length || index === mainTabBar.currentIndex)
                        return

                    let tab = tabs[index]
                    if(!tab.visible)
                        index = 0

                    const message = "Preparing the <b>" + tabs[index].name + "</b> tab, just a few seconds ..."

                    Scrite.document.setBusyMessage(message)
                    Scrite.document.screenplay.clearSelection()

                    Runtime.execLater(mainTabBar, 100, function() {
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
                    Runtime.mainWindowTab = Runtime.MainWindowTab.ScreenplayTab
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
                    TrackProperty { target: _header; property: "width" }
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

                        width: height
                        height: mainTabBar.height

                        visible: modelData.visible
                        enabled: modelData.visible

                        PainterPathItem {
                            anchors.fill: parent

                            fillColor: parent.active ? mainTabBar.activeTabColor : Runtime.colors.primary.c10.background
                            renderType: parent.active ? PainterPathItem.OutlineAndFill : PainterPathItem.FillOnly
                            outlineColor: Runtime.colors.primary.borderColor
                            outlineWidth: 1
                            renderingMechanism: PainterPathItem.UseQPainter

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
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: parent.active ? 0 : 1

                            width: parent.active ? 32 : 24
                            height: width
                            source: modelData.icon
                            opacity: parent.active ? 1 : 0.75
                            fillMode: Image.PreserveAspectFit

                            Behavior on width {
                                enabled: Runtime.applicationSettings.enableAnimations

                                NumberAnimation { duration: Runtime.stdAnimationDuration }
                            }

                        }

                        MouseArea {
                            ToolTip.text: modelData.name + "\t" + Scrite.app.polishShortcutTextForDisplay("Alt+"+(index+1))
                            ToolTip.delay: 1000
                            ToolTip.visible: containsMouse

                            anchors.fill: parent

                            hoverEnabled: true

                            onClicked: Runtime.activateMainWindowTab(index)
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

        property bool allowContent: Runtime.loadMainUiContent
        property string sessionId

        anchors.top: _header.visible ? _header.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        active: allowContent && !Scrite.document.loading
        opacity: 0
        sourceComponent: uiLayoutComponent

        Announcement.onIncoming: (type, data) => {
                                               if(type === Runtime.announcementIds.reloadMainUiRequest) {
                                                    mainUiContentLoader.active = false

                                                    const delay = data && typeof data === "number" ? data : 100
                                                    Runtime.execLater(mainUiContentLoader, delay, () => {
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

        Component.onCompleted: Runtime.execLater(mainUiContentLoader, 200, () => { mainUiContentLoader.opacity = 1 } )
    }

    Component {
        id: uiLayoutComponent

        Rectangle {
            color: mainTabBar.activeTabColor

            PainterPathItem {
                id: tabBarSeparator

                anchors.left: parent.left
                anchors.right: parent.right

                height: 1

                visible: mainTabBar.visible
                renderType: PainterPathItem.OutlineOnly
                outlineColor: Runtime.colors.primary.borderColor
                outlineWidth: height
                renderingMechanism: PainterPathItem.UseQPainter

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
                                                 Runtime.execLater(uiLoader, 250, function() {
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

                property string droppedFilePath
                property string droppedFileName

                anchors.fill: parent

                allowedType: Attachments.NoMedia
                allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]

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

                    Component.onDestruction: _header.enabled = true

                    anchors.fill: fileOpenDropArea

                    active: fileOpenDropArea.active || fileOpenDropArea.droppedFilePath !== ""
                    onActiveChanged: _header.enabled = !active

                    sourceComponent: Rectangle {
                        color: Scrite.app.translucent(Runtime.colors.primary.c500.background, 0.5)

                        Rectangle {
                            anchors.fill: fileOpenDropAreaNotice
                            anchors.margins: -30

                            color: Runtime.colors.primary.c700.background
                            radius: 4
                        }

                        Column {
                            id: fileOpenDropAreaNotice

                            anchors.centerIn: parent

                            width: parent.width * 0.5
                            spacing: 20

                            VclLabel {
                                width: parent.width

                                text: fileOpenDropArea.active ? fileOpenDropArea.attachment.originalFileName : fileOpenDropArea.droppedFileName
                                color: Runtime.colors.primary.c700.text
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter

                                font.bold: true
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            VclLabel {
                                width: parent.width

                                text: fileOpenDropArea.active ? "Drop the file here to open/import it." : "Do you want to open, import or cancel?"
                                color: Runtime.colors.primary.c700.text
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter

                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            VclLabel {
                                width: parent.width

                                text: "NOTE: Any unsaved changes in the currently open document will be discarded."
                                color: Runtime.colors.primary.c700.text
                                visible: !Scrite.document.empty || Scrite.document.fileName !== ""
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter

                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            Row {
                                anchors.horizontalCenter: parent.horizontalCenter

                                spacing: 20
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

            Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

            orientation: Qt.Vertical

            Rectangle {
                id: structureEditorRow1

                SplitView.fillHeight: true

                color: Runtime.colors.primary.c10.background

                SplitView {
                    id: structureEditorSplitView2

                    Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

                    anchors.fill: parent

                    orientation: Qt.Horizontal

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

                            property int currentTabIndex: 0

                            anchors.fill: parent

                            Announcement.onIncoming: (type,data) => {
                                var sdata = "" + data
                                var stype = "" + type
                                if(Runtime.showNotebookInStructure) {
                                    if(stype === Runtime.announcementIds.tabRequest) {
                                        if(sdata === "Structure")
                                            structureEditorTabs.currentTabIndex = 0
                                        else if(sdata.startsWith("Notebook")) {
                                            structureEditorTabs.currentTabIndex = 1
                                            if(sdata !== "Notebook")
                                                Runtime.execLater(notebookViewLoader, 100, function() {
                                                    notebookViewLoader.item.switchTo(sdata)
                                                })
                                        }
                                    } else if(stype === Runtime.announcementIds.characterNotesRequest) {
                                        structureEditorTabs.currentTabIndex = 1
                                        Runtime.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToCharacterTab(data)
                                        })
                                    }
                                    else if(stype === Runtime.announcementIds.sceneNotesRequest) {
                                        structureEditorTabs.currentTabIndex = 1
                                        Runtime.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToSceneTab(data)
                                        })
                                    }
                                }
                            }

                            Loader {
                                id: structureEditorTabBar

                                anchors.top: parent.top
                                anchors.left: parent.left
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
                                    width: _mainToolbar.height+4

                                    color: Runtime.colors.primary.c100.background

                                    Column {
                                        anchors.horizontalCenter: parent.horizontalCenter

                                        FlatToolButton {
                                            ToolTip.text: "Structure\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"

                                            down: structureEditorTabs.currentTabIndex === 0
                                            visible: Runtime.showNotebookInStructure
                                            iconSource: "qrc:/icons/navigation/structure_tab.png"

                                            onClicked: Announcement.shout(Runtime.announcementIds.tabRequest, "Structure")
                                        }

                                        FlatToolButton {
                                            ToolTip.text: "Notebook Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"

                                            down: structureEditorTabs.currentTabIndex === 1
                                            visible: Runtime.showNotebookInStructure
                                            iconSource: "qrc:/icons/navigation/notebook_tab.png"

                                            onClicked: Announcement.shout(Runtime.announcementIds.tabRequest, "Notebook")
                                        }
                                    }

                                    Rectangle {
                                        anchors.right: parent.right

                                        width: 1
                                        height: parent.height

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

                                active: Runtime.appFeatures.structure.enabled
                                visible: !Runtime.showNotebookInStructure || structureEditorTabs.currentTabIndex === 0
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

                                active: visible && Runtime.appFeatures.notebook.enabled
                                visible: Runtime.showNotebookInStructure && structureEditorTabs.currentTabIndex === 1

                                sourceComponent: NotebookView {
                                    toolbarSize: _mainToolbar.height+4
                                    toolbarSpacing: _mainToolbar.spacing
                                    toolbarLeftMargin: _mainToolbar.anchors.leftMargin
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

                            property string sessionId

                            anchors.fill: parent

                            active: false

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
                    Runtime.execLater(splitViewAnimationLoader, 250, function() {
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
                    var stype = "" + Runtime.announcementIds.tabRequest
                    var sdata = "" + data
                    if(stype === Runtime.announcementIds.tabRequest)
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
        ShortcutsModelItem.group: "Formatting"
        ShortcutsModelItem.title: "Symbols & Smileys"
        ShortcutsModelItem.enabled: Scrite.app.isTextInputItem(Scrite.window.activeFocusItem)
        ShortcutsModelItem.priority: 10
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
