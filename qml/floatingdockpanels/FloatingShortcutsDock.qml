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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

FloatingDock {
    id: root

    x: adjustedX(Runtime.shortcutsDockWidgetSettings.contentX)
    y: adjustedY(Runtime.shortcutsDockWidgetSettings.contentY)
    width: 425
    height: Scrite.window.height * 0.85 - titleBarHeight
    visible: Runtime.shortcutsDockWidgetSettings.visible

    title: "Shortcuts"

    function init() { }
    Component.onCompleted: {
        Qt.callLater( () => {
                         _saveSettingsTask.enabled = true
                     })
    }

    content: ListView {
        id: _actionsView

        ScrollBar.vertical: VclScrollBar { }

        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        boundsBehavior: Flickable.StopAtBounds
        clip: true
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        keyNavigationEnabled: true
        model: ActionsModelFilter {
            filters: ActionsModelFilter.ShortcutsDockFilters
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

            RowLayout {
                id: _delegateLayout

                width: parent.width

                opacity: qmlAction.enabled ? 1 : 0.5

                Image {
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

                VclLabel {
                    Layout.preferredWidth: _delegateLayout.width * 0.35

                    text: Gui.nativeShortcut(qmlAction.shortcut)
                    elide: Text.ElideRight

                    font.family: "Courier Prime"
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                }
            }

            MouseArea {
                ToolTip.text: {
                    const tt = qmlAction.tooltip !== undefined ? qmlAction.tooltip : qmlAction.text
                    const sc = Gui.nativeShortcut(qmlAction.shortcut)
                    return sc === "" ? tt : (tt + " ( " + sc + " )")
                }
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                ToolTip.visible: ToolTip.text !== "" && containsMouse

                anchors.fill: parent

                hoverEnabled: true
            }
        }

        highlight: Rectangle {
            color: Runtime.colors.primary.highlight.background
        }
    }

    // Private Section

    // This block ensures that everytime the floating dock coordinates change,
    // they are stored in persistent settings
    Connections {
        id: _saveSettingsTask

        target: root
        enabled: false

        function onXChanged() {
            Qt.callLater(_saveSettingsTask.saveCoordinates)
        }

        function onYChanged() {
            Qt.callLater(_saveSettingsTask.saveCoordinates)
        }

        function onCloseRequest() {
            Runtime.shortcutsDockWidgetSettings.visible = false
        }

        // Private
        function saveCoordinates() {
            Runtime.shortcutsDockWidgetSettings.contentX = Math.round(root.x)
            Runtime.shortcutsDockWidgetSettings.contentY = Math.round(root.y)
        }
    }

    QtObject {
        id: _private

        readonly property ShortcutsModel shortcutsModel: ShortcutsModel {
            // We need groups to show up in a specific order, hence this property.
            // Shortcuts belonging to other groups should show up at the end.
            groups: [ "Application", "Formatting", "Settings", "Language", "File", "Edit" ]

            onModelReset: _shortcutsView.currentIndex = 0
        }
    }
}
