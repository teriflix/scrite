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

    Material.background: Runtime.colors.primary.c10.background

    flat: true
    down: action.down !== undefined ? action.down === true : (pressed || checked)
    display: action.icon.source == "" && action.icon.name == "" ? ToolButton.TextOnly : ToolButton.IconOnly
    focusPolicy: Qt.NoFocus
    visible: action.visible !== undefined ? action.visible : true
    opacity: enabled ? 1 : 0.5

    font.pointSize: Runtime.idealFontMetrics.font.pointSize
    icon.source: Runtime.themedIcon(action.icon.source)
    icon.color: down ? Runtime.colors.buttonDownIconColor : action.icon.color

    ToolTipPopup {
        container: root

        text: {
            const sc = Gui.portableShortcut(root.action.shortcut)
            const tt = root.action.tooltip !== undefined ? root.action.tooltip : root.action.text
            return sc === "" ? tt : (tt + " (" + sc + ")")
        }
        visible: text !== "" && root.hovered
    }
}
