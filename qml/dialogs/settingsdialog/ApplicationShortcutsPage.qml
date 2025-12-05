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

    property real availableHeight: 500
    property alias lookup: _filterText.text

    Component.onCompleted: {
        forceActiveFocus()
        if(_filterText.text !== "") {
            _actionsModel.filter()
            if(_actionsView.count === 1) {
                _actionsView.itemAtIndex(0).editShortcut()
            }
        }
    }

    height: availableHeight

    focus: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 11

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: _filterText

                Layout.fillWidth: true

                Keys.onUpPressed: _actionsView.currentIndex = Math.max(_actionsView.currentIndex-1,0)
                Keys.onDownPressed: _actionsView.currentIndex = Math.min(_actionsView.currentIndex+1, _actionsView.count-1)
                Keys.onEnterPressed: _actionsView.editCurrentItem()
                Keys.onReturnPressed: _actionsView.editCurrentItem()

                focus: true
                placeholderText: "Filter by name"

                onTextEdited: _actionsModel.filter()
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

                function resetCurrentItem() {
                    currentIndex = count > 0 ? 0 : -1
                }

                function editCurrentItem() {
                    if(currentItem != null) {
                        currentItem.editShortcut()
                    }
                }

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

                    function editShortcut() {
                        _shortcutField.editShortcut()
                    }

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
                            Layout.alignment: _descriptionLabel.visible ? Qt.AlignTop : Qt.AlignVCenter
                            Layout.topMargin: _descriptionLabel.visible ? 10 : 0
                            Layout.leftMargin: 12
                            Layout.preferredHeight: _nameLabel.height * 0.5
                            Layout.preferredWidth: _nameLabel.height * 0.5

                            fillMode: Image.PreserveAspectFit
                            source: qmlAction.icon.source !== "" ? qmlAction.icon.source : "qrc:/icons/content/blank.png"
                        }

                        ColumnLayout {
                            Layout.alignment: _descriptionLabel.visible ? Qt.AlignTop : Qt.AlignVCenter
                            Layout.fillWidth: true

                            spacing: 0

                            VclLabel {
                                id: _nameLabel

                                Layout.fillWidth: true

                                elide: Text.ElideRight
                                font: Runtime.idealFontMetrics.font
                                padding: 10
                                bottomPadding: _descriptionLabel.visible ? 2 : 10
                                text: qmlAction.text + (qmlAction.checkable & qmlAction.checked ? " ✔" : "")
                            }

                            VclLabel {
                                id: _descriptionLabel

                                Layout.fillWidth: true
                                Layout.leftMargin: 10
                                Layout.rightMargin: 10
                                Layout.bottomMargin: 10

                                elide: Text.ElideRight
                                maximumLineCount: 3
                                wrapMode: Text.WordWrap
                                font: Runtime.minimumFontMetrics.font
                                text: qmlAction.tooltip !== undefined ? qmlAction.tooltip.trim() : ""
                                visible: text !== ""
                            }
                        }

                        ShortcutField {
                            id: _shortcutField

                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: 10
                            Layout.preferredWidth: _delegateLayout.width * 0.3

                            enabled: shortcutIsEditable
                            opacity: enabled ? 1 : 0.5
                            description: "Shortcut for <b>" + actionManager.title + "</b> » <i>" + qmlAction.text + "</i>"
                            portableShortcut: qmlAction.shortcut !== undefined ? qmlAction.shortcut : ""
                            placeholderText: qmlAction.defaultShortcut !== undefined ? ("Default: " + Gui.nativeShortcut(qmlAction.defaultShortcut)) :
                                                                                       (qmlAction.allowShortcut === true ? "None Set" : "")

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

        filters: ActionsModelFilter.ShortcutsEditorFilters
        customFilterMode: true

        onFilterRequest: (qmlAction, actionManager, result) => {
                             if(_filterText.length === 0) {
                                 result.value = true
                             } else {
                                 const givenText = _filterText.text.toLowerCase()

                                 let text = (actionManager.title + ": " + qmlAction.text)
                                 if(qmlAction.keywords !== undefined) {
                                     if(typeof qmlAction.keywords === "string")
                                     text += ", " + qmlAction.keywords
                                     else if(qmlAction.keywords.length > 0)
                                     text += ", " + qmlAction.keywords.join(", ")
                                 }
                                 if(qmlAction.tooltip !== undefined) {
                                     text += ", " + qmlAction.tooltip
                                 }

                                 text = text.toLowerCase()

                                 result.value = (text.indexOf(givenText) >= 0)
                             }
                         }

        onModelReset: Qt.callLater(_actionsView.resetCurrentItem)
        onRowsRemoved: Qt.callLater(_actionsView.resetCurrentItem)
        onRowsInserted: Qt.callLater(_actionsView.resetCurrentItem)
    }
}
