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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"

FloatingDock {
    id: root

    x: adjustedX(Runtime.shortcutsDockWidgetSettings.contentX)
    y: adjustedY(Runtime.shortcutsDockWidgetSettings.contentY)
    width: 375
    height: Scrite.window.height * 0.85 - titleBarHeight
    visible: Runtime.shortcutsDockWidgetSettings.visible

    title: "Shortcuts"

    function init() { }
    Component.onCompleted: {
        Qt.callLater( () => {
                         saveSettingsTask.enabled = true
                     })
    }

    content: ListView {
        id: shortcutsView
        model: Scrite.shortcuts
        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: VclScrollBar { }
        section.property: "itemGroup"
        section.criteria: ViewSection.FullString
        section.delegate: VclLabel {
            required property string section

            width: shortcutsView.width-15

            background: Rectangle {
                color: Runtime.colors.accent.c100.background
            }

            color: Runtime.colors.accent.c100.text

            font.bold: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter

            padding: 8

            text: section
        }
        delegate: Item {
            required property string itemTitle
            required property string itemShortcut
            required property bool itemVisible
            required property bool itemEnabled

            width: shortcutsView.width-17
            height: itemVisible ? 40 : 0
            opacity: itemEnabled ? 1 : 0.5
            visible: itemVisible

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.right: parent.right

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: itemTitle
                    width: parent.width * 0.65
                    elide: Text.ElideRight
                }

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Courier Prime"
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                    text: Scrite.app.polishShortcutTextForDisplay(itemShortcut)
                    width: parent.width * 0.35
                }
            }
        }
    }

    // Private Section

    // This block ensures that everytime the floating dock coordinates change,
    // they are stored in persistent settings
    Connections {
        id: saveSettingsTask

        target: root
        enabled: false

        function onXChanged() {
            Qt.callLater(saveSettingsTask.saveCoordinates)
        }

        function onYChanged() {
            Qt.callLater(saveSettingsTask.saveCoordinates)
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
}
