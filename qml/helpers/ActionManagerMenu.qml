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

Menu {
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

        model: root.actionManager ? _private.visibleActions : 0

        delegate: MenuItem {
            id: _menuItem

            required property var qmlAction

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

                text: {
                    const sc = Gui.nativeShortcut(qmlAction.shortcut)
                    if(sc === "")
                        return qmlAction.tooltip !== undefined ? qmlAction.tooltip : ""

                    const tt = qmlAction.tooltip !== undefined ? qmlAction.tooltip : qmlAction.text
                    return tt + " (" + sc + " )"
                }
                visible: text !== "" && _menuItem.hovered
            }
        }
    }

    onAboutToShow: _private.adjustMenuWidth()

    QtObject {
        id: _private

        readonly property ActionsModelFilter visibleActions: ActionsModelFilter {
            filters: root.actionManager ? ActionsModelFilter.VisibleActions : ActionsModelFilter.NoActions
            sourceModel: ActionsModel {
                actionManagers: [root.actionManager]
            }
        }

        function adjustMenuWidth() {
            let width = 200

            for(let i=0; i<_menuItems.count; i++) {
                const menuItem = _menuItems.itemAt(i)
                const itemWidth = menuItem.contentItem.implicitWidth + menuItem.leftPadding + menuItem.rightPadding
                width = Math.max(width, itemWidth)
            }

            root.width = width + 20
        }
    }
}
