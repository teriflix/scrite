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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"

ToolButton {
    id: root

    Material.accent: Runtime.colors.accent.key
    Material.background: Runtime.colors.primary.c10.background
    Material.primary: Runtime.colors.primary.key
    Material.theme: Runtime.colors.theme

    ToolTip.text: {
        const tt = action.tooltip !== undefined ? action.tooltip : action.text
        const sc = Scrite.app.polishShortcutTextForDisplay(action.shortcut)
        return sc === "" ? tt : (tt + " ( " + sc + " )")
    }
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.visible: ToolTip.text !== "" && hovered

    down: action.down !== undefined ? action.down === true : pressed
    display: action.icon.source == "" && action.icon.name == "" ? ToolButton.TextOnly : ToolButton.IconOnly
    focusPolicy: Qt.NoFocus
    visible: action.visible !== undefined ? action.visible : true

    font.pointSize: Runtime.idealFontMetrics.font.pointSize
}
