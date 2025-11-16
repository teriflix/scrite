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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

FocusScope {
    id: root

    enum FilterMethod { EditableShortcutsOnly, AllShortcuts }

    property int filterMethod: ApplicationShortcutsPage.FilterMethod.EditableShortcutsOnly
    property real availableHeight: 500

    height: availableHeight

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 11

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: _filterText

                Layout.fillWidth: true

                focus: true
                placeholderText: "Filter by name"
            }

            TextField {
                id: _filterGroup

                Layout.fillWidth: true

                focus: true
                placeholderText: "Filter by group"
            }

            ToolButton {
                id: _filterMethod

                ToolTip.text: "Filter Options"
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                ToolTip.visible: hovered

                icon.source: "qrc:/icons/content/view_options.png"

                onClicked: {
                    _optionsMenu.popup()
                }

                VclMenu {
                    id: _optionsMenu

                    width: Math.max( Runtime.idealFontMetrics.boundingRect(_option1.text).width,
                                    Runtime.idealFontMetrics.boundingRect(_option2.text).width ) + 100

                    VclMenuItem {
                        id: _option1

                        checkable: true
                        checked: root.filterMethod === ApplicationShortcutsPage.FilterMethod.EditableShortcutsOnly
                        text: "Only those with editable shortcuts"

                        onClicked: root.filterMethod = ApplicationShortcutsPage.FilterMethod.EditableShortcutsOnly
                    }

                    VclMenuItem {
                        id: _option2

                        checkable: true
                        checked: root.filterMethod === ApplicationShortcutsPage.FilterMethod.AllShortcuts
                        text: "All shortcuts"

                        onClicked: root.filterMethod = ApplicationShortcutsPage.FilterMethod.AllShortcuts
                    }
                }
            }

            ToolButton {
                flat: true

                ToolTip.text: "Restore default shortcuts to all"
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                ToolTip.visible: hovered

                icon.source: "qrc:/icons/content/undo.png"

                onClicked: {
                    const restoreCount = _actionsModel.restoreAllActionShortcuts()
                    if(restoreCount > 0) {
                        MessageBox.information("Shortcuts Restored",
                                               "Default shortcut was restored on " + restoreCount + " action(s).")
                    } else {
                        MessageBox.information("None Required",
                                               "No shortcut required restoration. All are already set to defaults.")
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c10.background
            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            ListView {
                id: _actionsView

                anchors.fill: parent
                anchors.margins: 1

                ScrollBar.vertical: VclScrollBar { }

                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: ListView.StopAtBounds
                clip: true
                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                keyNavigationEnabled: false
                model: _actionsModel

                section.property: "groupName"
                section.criteria: ViewSection.FullString
                section.labelPositioning: ViewSection.InlineLabels
                section.delegate: VclLabel {
                    required property string section

                    width: _actionsView.width

                    text: section
                    color: Runtime.colors.accent.c100.text
                    padding: 12

                    font.bold: true

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft

                    background: Rectangle {
                        color: Runtime.colors.accent.c100.background
                    }
                }

                delegate: Item {
                    required property int index
                    required property var qmlAction
                    required property var actionManager
                    required property bool shortcutIsEditable
                    required property string groupName

                    width: _actionsView.height < _actionsView.contentHeight ? _actionsView.width - 17 : _actionsView.width
                    height: _delegateLayout.height

                    MouseArea {
                        anchors.fill: parent

                        onClicked: _actionsView.currentIndex = index
                    }

                    RowLayout {
                        id: _delegateLayout

                        width: parent.width

                        Image {
                            Layout.leftMargin: 12
                            Layout.preferredHeight: Runtime.iconImageSize
                            Layout.preferredWidth: Runtime.iconImageSize

                            fillMode: Image.PreserveAspectFit
                            source: qmlAction.icon.source !== "" ? qmlAction.icon.source : "qrc:/icons/content/blank.png"
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            text: qmlAction.text
                            elide: Text.ElideRight
                            padding: 10
                        }

                        ShortcutField {
                            Layout.preferredWidth: _delegateLayout.width * 0.3

                            enabled: shortcutIsEditable
                            opacity: enabled ? 1 : 0.5
                            shortcut: Gui.nativeShortcut(qmlAction.shortcut)
                            placeholderText: qmlAction.defaultShortcut !== undefined ? ("Default: " + Gui.nativeShortcut(qmlAction.defaultShortcut)) : ""

                            onActiveFocusChanged: {
                                if(activeFocus) {
                                    _actionsView.currentIndex = index
                                }
                            }

                            onShortcutEdited: (newShortcut) => {
                                                  const conflictingAction = _actionsModel.findActionForShortcut(newShortcut)
                                                  if(conflictingAction) {
                                                      MessageBox.information("Shortcut Conflict",
                                                                             Gui.nativeShortcut(newShortcut) + " is already mapped to <b>" + conflictingAction.text + "</b>.")
                                                  } else {
                                                    qmlAction.shortcut = newShortcut
                                                  }
                                              }
                        }

                        ToolButton {
                            flat: true

                            enabled: visible && shortcutIsEditable && Gui.nativeShortcut(qmlAction.defaultShortcut) !== Gui.nativeShortcut(qmlAction.shortcut)
                            opacity: enabled ? 1 : (shortcutIsEditable ? 0.5 : 0)
                            icon.source: "qrc:/icons/content/undo.png"

                            onClicked: _actionsModel.restoreActionShortcut(qmlAction)
                        }
                    }
                }

                highlight: Rectangle {
                    color: Runtime.colors.primary.highlight.background
                }
            }
        }

    }

    ActionsModelFilter {
        id: _actionsModel

        filters: root.filterMethod === ApplicationShortcutsPage.FilterMethod.EditableShortcutsOnly ?
                     ActionsModelFilter.ShortcutsEditorFilters : ActionsModelFilter.ShortcutsDockFilters
        actionText: _filterText.text
        actionManagerTitle: _filterGroup.text

        onModelReset: _actionsView.currentIndex = -1
        onRowsRemoved: _actionsView.currentIndex = -1
        onRowsInserted: _actionsView.currentIndex = -1
    }
}
