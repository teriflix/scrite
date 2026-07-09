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

import QtQml
import Qt.labs.platform as Native

import io.scrite.components

import "../globals"

Native.Menu {
    id: root

    required property ActionManager actionManager

    title: actionManager ? actionManager.title : ""
    // type: Native.Menu.DefaultMenu

    // Instantiator is used instead of Repeater because Native.MenuItem objects
    // are not QQuickItems and cannot be used with standard Repeater patterns.
    // This allows us to dynamically create native menu items from the action
    // manager's action list.

    Instantiator {
        id: _menuItems

        model: root.actionManager ? root.actionManager.nativelyVisibleActions : 0

        delegate: NativeActionMenuItem {
            required property int index
            required property var modelData

            action: modelData
        }

        onObjectAdded: (index, object) => root.addItem(object)
        onObjectRemoved: (index, object) => root.removeItem(object)
    }
}
