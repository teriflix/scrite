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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/tasks"
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/scrited"
import "qrc:/qml/controls"
import "qrc:/qml/mainwindow" as MainWindow
import "qrc:/qml/notifications"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/floatingdockpanels"

Rectangle {
    id: root

    required property Shortcut toggleTaggingShortcut
    required property Shortcut toggleSynopsisShortcut
    required property Shortcut toggleCommentsShortcut
    required property Shortcut toggleSceneCharactersShortcut

    implicitHeight: 55

    color: Runtime.colors.primary.c50.background
    enabled: visible

    Row {
        id: _layout

        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter

        visible: root.width >= 1200

        onVisibleChanged: {
            if(enabled && !visible)
                Runtime.activateMainWindowTab(Runtime.e_ScreenplayTab)
        }

        FlatToolButton {
            text: "Home"
            iconSource: "qrc:/icons/action/home.png"

            onClicked: HomeScreen.launch()
        }

        FlatToolButton {
            ToolTip.text: "Open any of the " + Scrite.document.backupFilesModel.count + " backup(s) available for this file."

            text: "Open Backup"
            visible: Scrite.document.backupFilesModel.count > 0
            iconSource: "qrc:/icons/file/backup_open.png"

            onClicked: BackupsDialog.launch()

            VclText {
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                text: Scrite.document.backupFilesModel.count
                color: Runtime.colors.primary.highlight.text
                padding: 2
                font.bold: true
                font.pixelSize: parent.height * 0.2
            }
        }

        FlatToolButton {
            id: _saveButton

            text: "Save"
            enabled: (Scrite.document.modified || Scrite.document.fileName === "") && !Scrite.document.readOnly
            shortcut: "Ctrl+S"
            iconSource: "qrc:/icons/content/save.png"

            onClicked: activate()

            ShortcutsModelItem.group: "File"
            ShortcutsModelItem.title: text
            ShortcutsModelItem.enabled: enabled
            ShortcutsModelItem.shortcut: shortcut
            ShortcutsModelItem.canActivate: true
            ShortcutsModelItem.onActivated: activate()

            function activate() {
                if(Scrite.document.fileName === "")
                    SaveFileTask.saveAs()
                else
                    SaveFileTask.saveSilently()
            }
        }

        FlatToolButton {
            text: "Share"
            down: _shareMenu.visible
            enabled: _appMenu.visible === false
            iconSource: "qrc:/icons/action/share.png"

            onClicked: _shareMenu.open()

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                VclMenu {
                    id: _shareMenu

                    VclMenu {
                        id: _exportMenu

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
                                    delay: Qt.styleHints.mousePressAndHoldInterval
                                    visible: parent.hovered
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
                        id: _reportsMenu

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
                                    delay: Qt.styleHints.mousePressAndHoldInterval
                                    visible: parent.hovered
                                }
                            }
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
            text: "Settings & Shortcuts"
            down: _settingsAndShortcutsMenu.visible
            onClicked: _settingsAndShortcutsMenu.visible = true
            iconSource: "qrc:/icons/action/settings_applications.png"

            Item {
                anchors.top: parent.bottom
                anchors.left: parent.left

                VclMenu {
                    id: _settingsAndShortcutsMenu
                    width: 300

                    VclMenuItem {
                        id: _settingsMenuItem

                        ShortcutsModelItem.group: "Application"
                        ShortcutsModelItem.title: "Settings"
                        ShortcutsModelItem.shortcut: "Ctrl+,"
                        ShortcutsModelItem.enabled: _layout.visible
                        ShortcutsModelItem.canActivate: true
                        ShortcutsModelItem.onActivated: activate()

                        text: "Settings\t\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+,")
                        enabled: _layout.visible
                        icon.source: "qrc:/icons/action/settings_applications.png"

                        onClicked: activate()

                        function activate() {
                            SettingsDialog.launch()
                        }

                        Shortcut {
                            enabled: Runtime.allowAppUsage
                            context: Qt.ApplicationShortcut
                            sequence: "Ctrl+,"
                            onActivated: _settingsMenuItem.activate()
                        }
                    }

                    VclMenuItem {
                        id: _shortcutsMenuItem

                        text: "Shortcuts\t\t" + Scrite.app.polishShortcutTextForDisplay("Ctrl+E")
                        enabled: _layout.visible
                        icon.source: {
                            if(Scrite.app.isMacOSPlatform)
                                return "qrc:/icons/navigation/shortcuts_macos.png"
                            if(Scrite.app.isWindowsPlatform)
                                return "qrc:/icons/navigation/shortcuts_windows.png"
                            return "qrc:/icons/navigation/shortcuts_linux.png"
                        }

                        onClicked: activate()

                        ShortcutsModelItem.group: "Application"
                        ShortcutsModelItem.title: FloatingShortcutsDock.visible ? "Hide Shortcuts" : "Show Shortcuts"
                        ShortcutsModelItem.enabled: _layout.visible
                        ShortcutsModelItem.shortcut: "Ctrl+E"
                        ShortcutsModelItem.canActivate: true
                        ShortcutsModelItem.onActivated: activate()

                        function activate() {
                            Runtime.shortcutsDockWidgetSettings.visible = !Runtime.shortcutsDockWidgetSettings.visible
                        }

                        Shortcut {
                            enabled: Runtime.allowAppUsage
                            context: Qt.ApplicationShortcut
                            sequence: "Ctrl+E"
                            onActivated: _shortcutsMenuItem.activate()
                        }
                    }

                    VclMenuItem {
                        text: "Help\t\tF1"
                        icon.source: "qrc:/icons/action/help.png"

                        onClicked: Qt.openUrlExternally(helpUrl)
                    }

                    VclMenuItem {
                        text: "About"
                        icon.source: "qrc:/icons/action/info.png"

                        onClicked: AboutDialog.launch()
                    }

                    VclMenuItem {
                        id: _toggleFullScreenMenuItem

                        ShortcutsModelItem.group: "Application"
                        ShortcutsModelItem.title: "Toggle Fullscreen"
                        ShortcutsModelItem.shortcut: "F7"
                        ShortcutsModelItem.canActivate: true
                        ShortcutsModelItem.onActivated: activate()

                        text: "Toggle Fullscreen\tF7"
                        icon.source: "qrc:/icons/navigation/fullscreen.png"

                        onClicked: activate()

                        function activate() {
                            Utils.execLater(Scrite.app, 100, function() { Scrite.app.toggleFullscreen(Scrite.window) })
                        }

                        Shortcut {
                            enabled: Runtime.allowAppUsage
                            context: Qt.ApplicationShortcut
                            sequence: "F7"
                            onActivated: _toggleFullScreenMenuItem.activate()
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
            id: _languageToolButton

            ToolTip.text: Scrite.app.polishShortcutTextForDisplay("Language Transliteration" + "\t" + shortcut)

            text: Runtime.language.active.name
            down: _languageMenu.visible
            visible: Runtime.mainWindowTab <= Runtime.e_NotebookTab
            shortcut: "Ctrl+L"
            iconSource: "qrc:/icons/content/language.png"

            onClicked: _languageMenu.visible = true

            Item {
                anchors.top: parent.bottom
                anchors.left: parent.left

                LanguageMenu {
                    id: _languageMenu
                }
            }

            /**
              What would have been ideal is if action property in the VclMenuItems created above
              actually handled global application shortcuts. But sadly, they don't.

              We are forced to create Shortcut objects separately for the same. It would have been
              awesome if we could simply create Shortcut objects in Repeater, without nesting them
              in an Item. But that's not possible either, because Repeater can only create Item
              instances, and not anything thats just QObject subclass.

              I even tried to use QShortcut in ShortcutsModelItem C++ class, but that did not work either.
              Apparently we QShortcut instances can only be created on QWidget, so that's not going
              to work for us either. AppWindow is a QQuickView, which is QWindow and not QWidget.

              We are left with no other option but to waste memory like this. :-(
              */
            Repeater {
                model: LanguageEngine.supportedLanguages

                Item {
                    required property int index
                    required property var language // This is of type Language, but we have to use var here.
                                                   // You cannot use Q_GADGET struct names as type names in QML
                                                   // that privilege is only reserved for QObject types.

                    visible: false

                    Shortcut {
                        ShortcutsModelItem.title: language.name
                        ShortcutsModelItem.group: "Language"
                        ShortcutsModelItem.priority: index+1
                        ShortcutsModelItem.enabled: enabled
                        ShortcutsModelItem.shortcut: nativeText
                        ShortcutsModelItem.canActivate: true
                        ShortcutsModelItem.onActivated: Runtime.language.setActiveCode(language.code)

                        enabled: true
                        context: Qt.ApplicationShortcut
                        sequence: language.shortcut()

                        onActivated: Runtime.language.setActiveCode(language.code)
                    }
                }
            }

            HelpTipNotification {
                tipName: Scrite.app.isWindowsPlatform ? "language_windows" : (Scrite.app.isMacOSPlatform ? "language_macos" : "language_linux")
                enabled: Runtime.language.activeCode !== QtLocale.English
            }
        }

        FlatToolButton {
            ToolTip.text: "Show English to " + Runtime.language.active.name + " alphabet mappings.\t" + Scrite.app.polishShortcutTextForDisplay(shortcut)

            down: _alphabetMappingsPopup.visible
            visible: _mainTabBar.currentIndex <= 2 && enabled
            enabled: Runtime.language.activeCode !== QtLocale.English &&
                     Runtime.language.activeTransliterator.name === DefaultTransliteration.driver &&
                     DefaultTransliteration.supportsLanguageCode(Runtime.language.activeCode)
            shortcut: "Ctrl+K"
            iconSource: down ? "qrc:/icons/hardware/keyboard_hide.png" : "qrc:/icons/hardware/keyboard.png"

            onClicked: click()

            ShortcutsModelItem.group: "Language"
            ShortcutsModelItem.title: "Alphabet Mapping"
            ShortcutsModelItem.enabled: enabled
            ShortcutsModelItem.shortcut: shortcut
            ShortcutsModelItem.priority: 0
            ShortcutsModelItem.canActivate: true
            ShortcutsModelItem.onActivated: click()

            function click() {
                if(enabled)
                    _alphabetMappingsPopup.visible = !_alphabetMappingsPopup.visible
            }

            Item {
                anchors.top: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                width: _alphabetMappingsPopup.width

                Popup {
                    id: _alphabetMappingsPopup

                    width: alphabetMappingsLoader.width + 30
                    height: alphabetMappingsLoader.height + 30

                    modal: false
                    focus: false
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                    Loader {
                        id: alphabetMappingsLoader

                        width: item ? item.width : 0
                        height: item ? item.height : 0

                        active: parent.visible

                        sourceComponent: AlphabetMappingsView {
                            language: Runtime.language.active
                        }
                    }
                }
            }
        }

        VclLabel {
            id: _languageDescLabel

            anchors.verticalCenter: parent.verticalCenter

            width: 80

            text: Runtime.language.active.name
            visible: Runtime.mainWindowTab <= Runtime.e_NotebookTab

            font.pointSize: Runtime.minimumFontMetrics.font.pointSize

            MouseArea {
                anchors.fill: parent

                onClicked: _languageToolButton.click()
            }
        }
    }

    Row {
        id: _appMenu

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        visible: !_layout.visible

        FlatToolButton {
            text: "File"
            down: _appMenuLoader.active
            iconSource: "qrc:/icons/navigation/menu.png"

            onClicked: {
                if(_appMenuLoader.active)
                    _appMenuLoader.close()
                else
                    _appMenuLoader.show()
            }

            MenuLoader {
                id: _appMenuLoader

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

                        onTriggered: _saveButton.doClick()
                    }

                    MenuSeparator { }

                    VclMenu {
                        width: 250

                        title: "Share"

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
                        width: 300

                        title: "Reports"

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

                    LanguageMenu { }

                    VclMenuItem {
                        text: "Alphabet Mappings For " + Runtime.language.active.name
                        enabled: Runtime.language.activeCode !== QtLocale.English

                        onClicked: _alphabetMappingsPopup.visible = !_alphabetMappingsPopup.visible
                    }

                    MenuSeparator { }

                    VclMenu {
                        width: 250

                        title: "View"

                        VclMenuItem {
                            text: "Screenplay (" + Scrite.app.polishShortcutTextForDisplay("Alt+1") + ")"
                            font.bold: _mainTabBar.currentIndex === 0

                            onTriggered: Runtime.activateMainWindowTab(0)
                        }

                        VclMenuItem {
                            text: "Structure (" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
                            font.bold: _mainTabBar.currentIndex === 1

                            onTriggered: Runtime.activateMainWindowTab(1)
                        }

                        VclMenuItem {
                            text: "Notebook (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                            enabled: !Runtime.showNotebookInStructure
                            font.bold: _mainTabBar.currentIndex === 2

                            onTriggered: Runtime.activateMainWindowTab(2)
                        }

                        VclMenuItem {
                            text: "Scrited (" + Scrite.app.polishShortcutTextForDisplay("Alt+4") + ")"
                            font.bold: _mainTabBar.currentIndex === 3

                            onTriggered: _mainTabBar.currentIndex = 3
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
        id: _scritedToolbar

        anchors.left: _layout.visible ? _layout.right : _appMenu.right
        anchors.right: _editTools.visible ? _editTools.left : parent.right
        anchors.margins: 10
        anchors.verticalCenter: parent.verticalCenter
    }

    Row {
        id: _editTools

        x: _layout.visible ? (parent.width - _userLogin.width - width) : (_appMenu.x + (parent.width - width - _appMenu.width - _appMenu.x)/2)
        height: parent.height

        spacing: 20

        ScreenplayEditorToolbar {
            id: screenplayEditorToolbar

            toggleTaggingShortcut: root.toggleTaggingShortcut
            toggleSynopsisShortcut: root.toggleSynopsisShortcut
            toggleCommentsShortcut: root.toggleCommentsShortcut
            toggleSceneCharactersShortcut: root.toggleSceneCharactersShortcut

            anchors.verticalCenter: parent.verticalCenter

            visible: {
                const min = 0
                const max = Runtime.showNotebookInStructure ? 1 : 2
                return _mainTabBar.currentIndex >= min && _mainTabBar.currentIndex <= max
            }

            Component.onCompleted: Runtime.screenplayEditorToolbar = screenplayEditorToolbar
        }

        Row {
            id: _mainTabBar

            readonly property var tabs: [
                { "name": "Screenplay", "icon": "qrc:/icons/navigation/screenplay_tab.png", "visible": true, "tab": Runtime.e_ScreenplayTab },
                { "name": "Structure", "icon": "qrc:/icons/navigation/structure_tab.png", "visible": true, "tab": Runtime.e_StructureTab },
                { "name": "Notebook", "icon": "qrc:/icons/navigation/notebook_tab.png", "visible": !Runtime.showNotebookInStructure, "tab": Runtime.e_NotebookTab },
                { "name": "Scrited", "icon": "qrc:/icons/navigation/scrited_tab.png", "visible": Runtime.workspaceSettings.showScritedTab, "tab": Runtime.e_ScritedTab }
            ]
            readonly property color activeTabColor: Runtime.colors.primary.windowColor

            property Item currentTab: currentIndex >= 0 && _mainTabBarRepeater.count === tabs.length ? _mainTabBarRepeater.itemAt(currentIndex) : null
            property int currentIndex: -1
            property var currentTabP1: _currentTabExtents.value.p1
            property var currentTabP2: _currentTabExtents.value.p2

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

            function indexOfTab(_Runtime_TabType) {
                for(var i=0; i<tabs.length; i++) {
                    if(tabs[i].tab === _Runtime_TabType) {
                        return i
                    }
                }
                return -1
            }

            function activateTab(index) {
                if(index < 0 || index >= tabs.length || index === _mainTabBar.currentIndex)
                    return

                let tab = tabs[index]
                if(!tab.visible)
                    index = 0

                const message = "Preparing the <b>" + tabs[index].name + "</b> tab, just a few seconds ..."

                Scrite.document.setBusyMessage(message)
                Scrite.document.screenplay.clearSelection()

                Utils.execLater(_mainTabBar, 100, function() {
                    _mainTabBar.currentIndex = index
                    Scrite.document.clearBusyMessage()
                })
            }

            height: parent.height

            visible: _layout.visible

            TrackerPack {
                id: _currentTabExtents

                property var value: fallback
                readonly property var fallback: {
                    "p1": { "x": 0, "y": 0 },
                    "p2": { "x": 0, "y": 0 }
                }

                TrackProperty { target: _mainTabBar; property: "visible" }
                TrackProperty { target: root; property: "width" }
                TrackProperty { target: _mainTabBar; property: "currentTab" }

                onTracked:  {
                    if(_mainTabBar.visible && _mainTabBar.currentTab !== null) {
                        value = {
                            "p1": _mainTabBar.mapFromItem(_mainTabBar.currentTab, 0, 0),
                            "p2": _mainTabBar.mapFromItem(_mainTabBar.currentTab, _mainTabBar.currentTab.width, 0)
                        }
                    } else
                        value = fallback
                }
            }

            Repeater {
                id: _mainTabBarRepeater

                model: _mainTabBar.tabs

                Item {
                    property bool active: _mainTabBar.currentIndex === index

                    width: height
                    height: _mainTabBar.height

                    visible: modelData.visible
                    enabled: modelData.visible

                    PainterPathItem {
                        anchors.fill: parent

                        fillColor: parent.active ? _mainTabBar.activeTabColor : Runtime.colors.primary.c10.background
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
                        id: _tabBarFontMetrics

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

            Connections {
                target: Scrite.document

                function onJustReset() {
                    Runtime.activateMainWindowTab(0)
                }

                function onAboutToSave() {
                    let userData = Scrite.document.userData
                    userData["mainTabBar"] = {
                        "version": 0,
                        "currentIndex": _mainTabBar.currentIndex
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

            onCurrentIndexChanged: {
                Runtime.mainWindowTab = tabs[currentIndex].tab
            }
        }
    }

    UserAccountToolButton {
        id: _userLogin

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
    }
}
