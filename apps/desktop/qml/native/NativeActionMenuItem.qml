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

import io.scrite.components

import "../globals"

Native.MenuItem {
    id: root

    required property var action

    property Action _qmlAction: action as Action

    text: _qmlAction ? _qmlAction.text : ""
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
