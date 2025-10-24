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

Item {
    id: root

    property real availableHeight: 500

    height: availableHeight

    ListView {
        id: _actionsView

        anchors.fill: parent

        ScrollBar.vertical: VclScrollBar { }

        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        boundsBehavior: Flickable.StopAtBounds
        clip: true
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        keyNavigationEnabled: false
        model: ActionsModelFilter {
            filters: ActionsModelFilter.ShortcutsEditorFilters
        }

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
            required property string groupName
            required property var actionManager
            required property var qmlAction

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

                    shortcut: qmlAction.shortcut
                    placeholderText: qmlAction.defaultShortcut !== undefined ? ("Default: " + Scrite.app.polishShortcutTextForDisplay(qmlAction.defaultShortcut)) : ""

                    onActiveFocusChanged: {
                        if(activeFocus) {
                            _actionsView.currentIndex = index
                        }
                    }
                }

                ToolButton {
                    flat: true

                    visible: qmlAction.defaultShortcut !== undefined
                    enabled: visible && Scrite.app.polishShortcutTextForDisplay(qmlAction.defaultShortcut) !== Scrite.app.polishShortcutTextForDisplay(qmlAction.shortcut)
                    opacity: enabled ? 1 : 0.5
                    icon.source: "qrc:/icons/content/undo.png"
                }
            }
        }

        highlight: Rectangle {
            color: Runtime.colors.primary.highlight.background
        }
    }
}
