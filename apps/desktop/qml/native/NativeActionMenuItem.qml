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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick.Controls
import Qt.labs.platform as Native

import Scrite.App

import "../globals"

Native.MenuItem {
    id: root

    required property var action

    // Native menus have no context. Actions read it from the source that is passed to their
    // onTriggered handler, like they do from items of hosted menus.
    readonly property AbstractActionContext context: null

    property Action _qmlAction: action as Action

    text: _qmlAction ? (action.menuText !== undefined ? action.menuText : _qmlAction.text) : ""
    // Native menus follow the appearance of the OS, not that of Scrite. On macOS, masked icons are
    // template images, which the OS tints for light and dark modes, and for highlighted items.
    icon.mask: Platform.isMacOSDesktop
    icon.source: {
        const iconSource = _qmlAction && _qmlAction.icon.source ? _qmlAction.icon.source.toString() : ""
        if(iconSource === "")
            return ""
        return Platform.isMacOSDesktop ? iconSource : Runtime.themedIcon(iconSource)
    }
    checkable: (_qmlAction ? _qmlAction.checkable : false) || (action.down !== undefined)
    checked: _qmlAction && _qmlAction.checkable ? _qmlAction.checked : (action.down !== undefined ? action.down : false)
    enabled: _qmlAction ? _qmlAction.enabled : false
    visible: text !== ""
    shortcut: _qmlAction ? _qmlAction.shortcut : ""

    onTriggered: {
        if(_qmlAction) {
            // Calling trigger is enough even for checkable actions
            _qmlAction.trigger(root)
        }
    }

    onActionChanged: {
        if(Object.changeProperty(_qmlAction, "#nativelyShown", true)) {
            Runtime.nativelyNotShownActions.scheduleFilter()
        }
    }

    readonly property QtObject _guard: QtObject {
        Component.onDestruction: {
            if(root._qmlAction) {
                if(Object.resetProperty(root._qmlAction, "#nativelyShown")) {
                    Runtime.nativelyNotShownActions.scheduleFilter()
                }
            }
        }
    }

    readonly property Binding _roleBinding: Binding {
        target: root
        property: "role"
        value: root.action.nativeMenuItemRole
        when: root.action.nativeMenuItemRole !== undefined
    }
}
