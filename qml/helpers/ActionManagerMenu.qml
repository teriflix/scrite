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

Menu {
    id: root

    required property ActionManager actionManager

    Material.accent: Runtime.colors.accent.key
    Material.primary: Runtime.colors.primary.key
    Material.theme: Runtime.colors.theme

    title: actionManager ? actionManager.name : ""
    closePolicy: Popup.CloseOnEscape|Popup.CloseOnPressOutside
    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    // Repeater doesn't seem to work in cases where the order of the actions
    // change for whatever reason. They also don't work if actions are added
    // or removed dynamically. So, its important that we recreate the entire
    // menu from ground up.
    //
    // This also means that you should never alter this menu by yourself in any
    // case.

    Repeater {
        id: _menuItems

        Component.onCompleted: reload()

        function reload() {
            model = 0
            model = root.actionManager

            let width = 200
            for(let i=0; i<count; i++) {
                const menuItem = itemAt(i)
                width = Math.max(menuItem.contentItem.implicitWidth, width)
            }
            width += root.leftPadding + root.rightPadding + 30

            root.width = width
        }

        delegate: MenuItem {
            required property var qmlAction

            Material.accent: Runtime.colors.accent.key
            Material.primary: Runtime.colors.primary.key
            Material.theme: Runtime.colors.theme

            ToolTip.text: Scrite.app.polishShortcutTextForDisplay(qmlAction.shortcut)

            action: qmlAction
            font.bold: qmlAction.checkable && qmlAction.checked
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }
    }

    onActionManagerChanged: {
        _menuItems.reload()
    }
}
