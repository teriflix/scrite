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

    content: ColumnLayout {
        VclTextField {
            id: _shortcutsFilter

            Layout.fillWidth: true
            Layout.topMargin: 8
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            Keys.onUpPressed: _shortcutsView.currentIndex = Math.max(0,_shortcutsView.currentIndex-1)
            Keys.onDownPressed: _shortcutsView.currentIndex = Math.min(_shortcutsView.currentIndex+1,_shortcutsView.count-1)
            Keys.onEscapePressed: { clear(); _private.shortcutsModel.titleFilter = "" }
            Keys.onReturnPressed: _private.shortcutsModel.activateShortcutAt(_shortcutsView.currentIndex)

            placeholderText: "Search/Filter"

            onTextEdited: _private.shortcutsModel.titleFilter = text
        }

        ListView {
            id: _shortcutsView

            Layout.fillWidth: true
            Layout.fillHeight: true

            Keys.onEscapePressed: { _shortcutsFilter.clear(); _private.shortcutsModel.titleFilter = "" }
            Keys.onReturnPressed: _private.shortcutsModel.activateShortcutAt(currentIndex)

            ScrollBar.vertical: VclScrollBar { }

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            clip: true
            model: _private.shortcutsModel
            boundsBehavior: Flickable.StopAtBounds
            keyNavigationEnabled: true
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            highlightFollowsCurrentItem: true

            section.property: "itemGroup"
            section.criteria: ViewSection.FullString
            section.labelPositioning: ViewSection.InlineLabels
            section.delegate: VclLabel {
                required property string section

                width: _shortcutsView.width

                text: section
                color: Runtime.colors.accent.c100.text
                padding: 8

                font.bold: true

                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft

                background: Rectangle {
                    color: Runtime.colors.accent.c100.background
                }
            }

            delegate: Item {
                required property int index
                required property bool itemVisible
                required property bool itemEnabled
                required property bool itemCanBeActivated
                required property string itemTitle
                required property string itemGroup
                required property string itemShortcut

                Keys.onReturnPressed: _private.shortcutsModel.activateShortcutAt(index)

                width: _shortcutsView.width-(_shortcutsView.contentHeight > _shortcutsView.height ? 17 : 0)
                height: _delegateLayout.height+16
                opacity: itemEnabled ? 1 : 0.5

                Row {
                    id: _delegateLayout

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter

                    Link {
                        anchors.verticalCenter: parent.verticalCenter

                        text: itemTitle
                        width: parent.width * 0.65
                        elide: Text.ElideRight
                        enabled: itemCanBeActivated
                        hoverColor: Runtime.colors.accent.c100.text
                        defaultColor: Runtime.colors.primary.c100.text
                        font.underline: containsMouse

                        onClicked: {
                            _shortcutsView.forceActiveFocus()
                            _shortcutsView.currentIndex = index
                            _private.shortcutsModel.activateShortcutAt(index)
                        }
                    }

                    VclLabel {
                        anchors.verticalCenter: parent.verticalCenter

                        text: Scrite.app.polishShortcutTextForDisplay(itemShortcut)
                        width: parent.width * 0.35

                        font.family: "Courier Prime"
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }
                }
            }

            highlight: Rectangle {
                color: Runtime.colors.primary.highlight.background
            }
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
