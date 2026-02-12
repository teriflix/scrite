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
    width: _private.toolbuttonSize.width * ActionHub.markupTools.count
    height: _private.toolbuttonSize.height + titleBarHeight
    visible: _private.dockVisibility

    title: "Markup Tools"

    function init() { }

    Component.onCompleted: {
        Qt.callLater( () => { _private.enableSaveCoordinates = true })
    }

    content: ActionManagerToolBar {
        enabled: !Scrite.document.readOnly && Runtime.allowAppUsage
        actionManager: ActionHub.markupTools
    }

    onXChanged: {
        Qt.callLater(_private.saveCoordinates)
    }

    onYChanged: {
        Qt.callLater(_private.saveCoordinates)
    }

    onCloseRequest: {
        Runtime.screenplayEditorSettings.markupToolsDockVisible = false
    }

    // Private Section
    QtObject {
        id: _private

        readonly property size toolbuttonSize: Runtime.estimateTypeSize("ToolButton { icon.source: \"qrc:/icons/content/blank.png\"; display: ToolButton.IconOnly }")

        property bool enableSaveCoordinates: false
        property bool dockVisibility: Runtime.screenplayEditorSettings.markupToolsDockVisible && Runtime.screenplayEditor

        function saveCoordinates() {
            if(enableSaveCoordinates) {
                Runtime.markupToolsSettings.contentX = Math.round(root.x)
                Runtime.markupToolsSettings.contentY = Math.round(root.y)
            }
        }

        onDockVisibilityChanged: {
            if(dockVisibility)
                root.open()
            else
                root.close()
        }
    }
}
