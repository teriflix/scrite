/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../globals"
import "../helpers"
import "../controls"

/**
  The toolbar of the main window. What is on it are the ToolBarGroups whose hostUri is
  Scrite.App.MainWindow.ToolBar, which are the ones below, and those of extensions. Groups
  with the lowest priority collapse first as the window narrows. Whatever is on the toolbar is also
  in the menu bar, which has everything at all times.

  Scrite.ToolBar.File, .Export, .AppOptions, .Theme, .Language, .Formats, .Find, .SplitMerge,
  .Screenplay, .ScritedFile and .ScritedOptions.
  */
Rectangle {
    id: root

    // The uri of this toolbar, which every group below gives as its hostUri.
    readonly property string uri: "Scrite.App.MainWindow.ToolBar"
    readonly property bool inScrited: Runtime.mainWindowTab === Runtime.MainWindowTab.ScritedTab

    color: Runtime.colors.primary.c50.background

    implicitHeight: _layout.height
    enabled: Runtime.dialogs.objectCount === 0

    RowLayout {
        id: _layout

        width: parent.width
        spacing: 0

        HostedToolBar {
            id: _toolBar

            Layout.fillHeight: true

            uri: root.uri

            // Tabs show their text when there is room to spare, and that is not counted in the width
            // that is available, so that showing it doesn't make the room go away.
            availableWidth: root.width - _tabIconWidth.implicitWidth * _tabsRepeater.count - _userAccount.implicitWidth - _userAccount.Layout.leftMargin
            roomyWidth: 150
        }

        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            id: _mainTabs

            Repeater {
                id: _tabsRepeater

                model: ActionHub.mainWindowTabs.visibleActions

                delegate: ToolButton {
                    id: _mainWindowTabDelegate

                    required property int index
                    required property var modelData

                    readonly property var qmlAction: modelData as Action

                    ToolTipPopup {
                        text: {
                            const tt = _mainWindowTabDelegate.qmlAction.tooltip !== undefined ? _mainWindowTabDelegate.qmlAction.tooltip : _mainWindowTabDelegate.qmlAction.text
                            const sc = Gui.nativeShortcut(_mainWindowTabDelegate.qmlAction.shortcut)
                            return sc === "" ? tt : (tt + " (" + sc + ")")
                        }
                        visible: _mainWindowTabDelegate.hovered
                    }

                    action: qmlAction
                    down: qmlAction.down
                    display: down && _toolBar.roomy ? Button.TextBesideIcon : Button.IconOnly
                    flat: true

                    background: Item {
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -5

                            color: _mainWindowTabDelegate.qmlAction.down ? Runtime.colors.primary.c300.background : Runtime.colors.transparent
                        }
                    }
                }
            }
        }

        UserAccountToolButton {
            id: _userAccount

            Layout.leftMargin: 10
        }
    }

    // The width of a tab that shows only its icon.
    ToolButton {
        id: _tabIconWidth

        visible: false
        display: ToolButton.IconOnly
        flat: true
        icon.source: "qrc:/icons/navigation/screenplay_tab.png"
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.File"
        hostUri: root.uri
        order: 10
        priority: 90
        collapsible: true
        visible: !root.inScrited
        actionManager: ActionHub.fileOperations
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.Export"
        hostUri: root.uri
        order: 20
        priority: 50
        collapsible: true
        title: "Export & Reports"
        visible: !root.inScrited
        entries: [ActionHub.exportOptions, ActionHub.reportOptions]
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.Theme"
        hostUri: root.uri
        order: 30
        priority: 10
        visible: !root.inScrited && Runtime.applicationSettings.showColorThemeIconOnToolbar

        delegate: ActionToolButton {
            action: ActionHub.appOptions.find("toggleColorTheme") as Action
            tooltip: "Switch to " + (action.isDarkMode ? "Light" : "Dark") + " Mode (" + Gui.portableShortcut(action.shortcut) + ")"
            visible: true
        }
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.Language"
        hostUri: root.uri
        order: 40
        priority: 70
        visible: !root.inScrited

        delegate: RowLayout {
            id: _language

            required property AbstractActionContext context

            spacing: 0

            ActionManagerToolButton {
                actionManager: ActionHub.languageOptions
                enabled: visible
                visible: LanguageEngine.handleLanguageSwitch
            }

            ActionToolButton {
                action: ActionHub.inputOptions.find("alphabetMappings") as Action
                down: _alphabetMappingsPopup.visible
                visible: action.visible && LanguageEngine.handleLanguageSwitch
            }

            SctLabel {
                Layout.preferredWidth: contentWidth + rightPadding

                rightPadding: Runtime.minimumFontMetrics.averageCharacterWidth
                text: Scrite.window.width < 1500 ? Runtime.language.active.shortName.toUpperCase() : Runtime.language.active.name
                visible: LanguageEngine.handleLanguageSwitch

                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            }

            ActionToolButton {
                action: ActionHub.languageOptions.find("platformLanguage") as Action
                display: ToolButton.TextBesideIcon
            }

            Item {
                id: _alphabetMappingsHost

                Layout.preferredWidth: 0
                Layout.preferredHeight: 0

                ActionHandler {
                    action: ActionHub.inputOptions.find("alphabetMappings")
                    enabled: LanguageEngine.handleLanguageSwitch

                    onTriggered: _alphabetMappingsPopup.open()

                    Popup {
                        id: _alphabetMappingsPopup

                        Material.elevation: 6
                        Material.containerStyle: Material.Filled
                        Material.roundedScale: Material.NotRounded

                        x: -_alphabetMappingsHost.x
                        y: _language.height
                        width: _alphabetMappingsLoader.width + 30
                        height: _alphabetMappingsLoader.height + 30

                        modal: false
                        focus: false
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                        onClosed: {
                            Object.resetProperty(_alphabetMappingsLoader, "width")
                            Object.resetProperty(_alphabetMappingsLoader, "height")
                        }

                        Loader {
                            id: _alphabetMappingsLoader

                            active: _alphabetMappingsPopup.visible

                            sourceComponent: AlphabetMappingsView {
                                option: Runtime.language.activeTransliterationOption

                                EventFilter.target: Scrite.app
                                EventFilter.events: [EventFilter.KeyPress]
                                EventFilter.onFilter: (watched, event, result) => {
                                                          result.acceptEvent = event.key == Qt.Key_Escape
                                                          result.filter = event.key == Qt.Key_Escape
                                                          if(event.key === Qt.Key_Escape)
                                                              _alphabetMappingsPopup.close()
                                                      }
                            }
                        }
                    }
                }
            }
        }
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.Formats"
        hostUri: root.uri
        order: 50
        priority: 60
        collapsible: true
        visible: !root.inScrited
        actionManager: ActionHub.paragraphFormats
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.Find"
        hostUri: root.uri
        order: 60
        priority: 75
        collapsible: true
        title: "Find"
        visible: !root.inScrited

        entries: [
            ActionHub.editOptions.find("find"),
            ActionHub.applicationOptions.find("configureScreenplayEditorOptions")
        ]
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.SplitMerge"
        hostUri: root.uri
        order: 61
        priority: 20
        visible: !root.inScrited

        entries: [
            ActionHub.editOptions.find("splitScene"),
            ActionHub.editOptions.find("mergeScene")
        ]
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.Screenplay"
        hostUri: root.uri
        order: 70
        priority: 40
        collapsible: true
        visible: !root.inScrited
        actionManager: ActionHub.screenplayOperations
        iconSource: "qrc:/icons/content/add_box.png"
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.ScritedFile"
        hostUri: root.uri
        order: 80
        priority: 90
        visible: root.inScrited
        entries: [ActionHub.fileOperations.find("fileOpen")]
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.ScritedOptions"
        hostUri: root.uri
        order: 81
        priority: 85
        collapsible: true
        visible: root.inScrited
        actionManager: ActionHub.scritedOptions
    }

    ToolBarGroup {
        uri: "Scrite.ToolBar.AppOptions"
        hostUri: root.uri
        order: 90
        priority: 80
        visible: !root.inScrited
        separator: false
        entries: [ActionHub.appOptions]
    }
}
