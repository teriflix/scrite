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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Rectangle {
    id: root

    color: Runtime.colors.primary.c50.background

    implicitHeight: _layout.height

    RowLayout {
        id: _layout

        width: parent.width

        ToolButton {
            Layout.rightMargin: Runtime.iconImageSize * 2

            display: ToolButton.TextBesideIcon
            down: _mainMenu.visible
            text: "Scrite"
            visible: Runtime.mainWindowTab !== Runtime.MainWindowTab.ScritedTab && !_group1.visible

            icon.source: "qrc:/icons/exporter/scrite.png"

            onClicked: _mainMenu.popup()

            Menu {
                id: _mainMenu

                ActionManagerMenu {
                    actionManager: ActionHub.fileOperations
                }

                ActionManagerMenu {
                    actionManager: ActionHub.languageOptions
                }

                ActionManagerMenu {
                    actionManager: ActionHub.exportOptions
                }

                ActionManagerMenu {
                    actionManager: ActionHub.reportOptions
                }

                ActionManagerMenu {
                    actionManager: ActionHub.appOptions
                }
            }
        }

        RowLayout {
            id: _group1

            visible: Runtime.mainWindowTab !== Runtime.MainWindowTab.ScritedTab &&
                     root.width > _group1.width + _group2.width + _mainTabs.width + _userAccount.width

            ActionManagerToolBar {
                actionManager: ActionHub.fileOperations
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                color: Runtime.colors.primary.borderColor
            }

            ActionManagerToolButton {
                actionManager: ActionHub.exportOptions
            }

            ActionManagerToolButton {
                actionManager: ActionHub.reportOptions
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                color: Runtime.colors.primary.borderColor
            }

            ActionManagerToolButton {
                actionManager: ActionHub.appOptions
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                color: Runtime.colors.primary.borderColor
            }

            ActionManagerToolButton {
                actionManager: ActionHub.languageOptions
            }

            ActionToolButton {
                action: _alphabetMappingsHandler.action
                down: _alphabetMappingsPopup.visible
            }

            VclLabel {
                Layout.preferredWidth: contentWidth + rightPadding

                text: Runtime.language.active.name
                rightPadding: Runtime.minimumFontMetrics.averageCharacterWidth * 2

                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                color: Runtime.colors.primary.borderColor
            }
        }

        RowLayout {
            id: _group2

            visible: Runtime.mainWindowTab !== Runtime.MainWindowTab.ScritedTab

            ActionManagerToolBar {
                actionManager: ActionHub.paragraphFormats
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                color: Runtime.colors.primary.borderColor
            }

            ActionToolButton {
                action: ActionHub.editOptions.find("find")
            }

            ActionToolButton {
                action: ActionHub.editOptions.find("reload")
            }

            ActionToolButton {
                action: ActionHub.editOptions.find("splitScene")
            }

            ActionToolButton {
                action: ActionHub.editOptions.find("mergeScene")
            }

            ActionManagerToolButton {
                actionManager: ActionHub.screenplayEditorOptions
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                color: Runtime.colors.primary.borderColor
            }

            ActionManagerToolBar {
                actionManager: ActionHub.screenplayOperations
            }
        }

        RowLayout {
            id: _scritedGroup

            visible: Runtime.mainWindowTab === Runtime.MainWindowTab.ScritedTab

            ActionToolButton {
                action: ActionHub.fileOperations.find("fileOpen")
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                color: Runtime.colors.primary.borderColor
            }

            ActionManagerToolBar {
                actionManager: ActionHub.scritedOptions
            }
        }

        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            id: _mainTabs

            Repeater {
                model: ActionHub.mainWindowTabs

                ToolButton {
                    required property var qmlAction

                    Material.theme: Runtime.colors.theme
                    Material.accent: Runtime.colors.accent.key
                    Material.primary: Runtime.colors.primary.key

                    ToolTip.text: {
                        const tt = qmlAction.tooltip !== undefined ? qmlAction.tooltip : qmlAction.text
                        const sc = Gui.nativeShortcut(qmlAction.shortcut)
                        return sc === "" ? tt : (tt + " (" + sc + " )")
                    }
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    ToolTip.visible: ToolTip.text !== "" && hovered

                    action: qmlAction
                    down: qmlAction.down
                    display: down ? Button.TextBesideIcon : Button.IconOnly
                    flat: true
                    visible: qmlAction.visible !== undefined ? qmlAction.visible === true : true

                    background: Item {
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -5

                            color: qmlAction.down ? Runtime.colors.primary.c300.background : Runtime.colors.transparent
                        }
                    }
                }
            }
        }

        UserAccountToolButton {
            id: _userAccount
        }
    }

    ActionHandler {
        id: _alphabetMappingsHandler

        anchors.top: parent.bottom

        width: _alphabetMappingsPopup.width

        action: ActionHub.inputOptions.find("alphabetMappings")
        onTriggered: _alphabetMappingsPopup.open()

        Popup {
            id: _alphabetMappingsPopup

            width: _alphabetMappingsLoader.width + 30
            height: _alphabetMappingsLoader.height + 30

            modal: false
            focus: false
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

            Loader {
                id: _alphabetMappingsLoader

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
