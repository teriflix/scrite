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

    x: adjustedX(Runtime.markupToolsSettings.contentX)
    y: adjustedY(Runtime.markupToolsSettings.contentY)
    width: toolbuttonSize.width * ActionHub.markupTools.count
    height: toolbuttonSize.height + titleBarHeight

    title: "Markup Tools"
    visible: Runtime.screenplayEditorSettings.markupToolsDockVisible && Runtime.screenplayEditor

    function init() { }

    Component.onCompleted: {
        Qt.callLater( () => {
                         _saveSettingsTask.enabled = true
                     })
    }

    content: ActionManagerToolBar {
        enabled: !Scrite.document.readOnly && Runtime.allowAppUsage
        actionManager: ActionHub.markupTools
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
            Runtime.screenplayEditorSettings.markupToolsDockVisible = false
        }

        // Private
        function saveCoordinates() {
            Runtime.markupToolsSettings.contentX = Math.round(root.x)
            Runtime.markupToolsSettings.contentY = Math.round(root.y)
        }
    }

    readonly property size toolbuttonSize: Runtime.estimateTypeSize("ToolButton { icon.source: \"qrc:/icons/content/blank.png\"; display: ToolButton.IconOnly }")
}
