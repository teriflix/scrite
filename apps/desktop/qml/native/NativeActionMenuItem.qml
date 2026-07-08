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

import QtQuick.Controls
import Qt.labs.platform as Native

import "../globals"

Native.MenuItem {
    id: root

    required property var action

    property Action qmlAction: action as Action

    text: qmlAction ? qmlAction.text : ""
    checkable: (qmlAction ? qmlAction.checkable : false) || (action.down !== undefined)
    checked: qmlAction && qmlAction.checkable ? qmlAction.checked : (action.down !== undefined ? action.down : false)
    enabled: qmlAction ? qmlAction.enabled : false
    visible: {
        const hasText = action.text !== ""
        if(action.nativeVisible !== undefined)
            return action.nativeVisible && hasText
        return action.visible !== undefined ? action.visible && hasText : hasText
    }
    shortcut: qmlAction ? qmlAction.shortcut : ""
    role: action.nativeMenuItemType !== undefined ? action.nativeMenuItemType : Native.MenuItem.NoRole
    onRoleChanged: {
        if(role !== Native.MenuItem.NoRole)
            Gui.log(text + " has role as " + role)
    }

    onTriggered: {
        if(qmlAction) {
            // Calling trigger is enough even for checkable actions
            qmlAction.trigger(root)
        }
    }
}
