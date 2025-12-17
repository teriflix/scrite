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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

VclMenu {
    id: root

    required property ActionManager actionManager

    Material.accent: Runtime.colors.accent.key
    Material.primary: Runtime.colors.primary.key
    Material.theme: Runtime.colors.theme

    title: actionManager ? actionManager.title : ""
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

        model: root.actionManager ? root.actionManager.visibleActions : 0

        delegate: VclMenuItem {
            id: _menuItem

            required property var modelData

            property var qmlAction: modelData

            Material.accent: Runtime.colors.accent.key
            Material.primary: Runtime.colors.primary.key
            Material.theme: Runtime.colors.theme

            action: qmlAction
            focusPolicy: Qt.NoFocus
            opacity: enabled ? 1 : 0.5

            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            icon.color: action.icon.color

            // We need a better way to show shortcuts. This is not going to work!

            ToolTipPopup {
                container: _menuItem

                text: qmlAction.tooltip !== undefined ? qmlAction.tooltip : ""
                visible: text !== "" && _menuItem.hovered
            }
        }

        onCountChanged: Qt.callLater(root.determineWidth)
    }
}
