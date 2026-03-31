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


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"

ToolButton {
    id: root

    required property ActionManager actionManager

    Material.background: Runtime.colors.primary.c10.background

    display: actionManager.iconSource !== undefined ? ToolButton.IconOnly : ToolButton.TextOnly
    down: _menu.visible
    focusPolicy: Qt.NoFocus
    text: actionManager.title

    icon.color: "transparent"
    icon.source: actionManager.iconSource !== undefined ? Runtime.themedIcon(actionManager.iconSource) : ""

    onClicked: _menu.open()

    Item {
        anchors.bottom: parent.bottom

        width: _menu.width
        height: 1

        ActionManagerMenu {
            id: _menu

            actionManager: root.actionManager
        }
    }

    ToolTipPopup {
        container: root

        text: root.display === ToolButton.IconOnly ? root.actionManager.title : ""
        visible: root.display === ToolButton.IconOnly ? text !== "" && root.hovered : false
    }
}
