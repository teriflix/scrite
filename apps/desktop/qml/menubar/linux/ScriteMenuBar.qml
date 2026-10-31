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
import QtQuick
import QtQuick.Controls

import Scrite.App

import "../../helpers"

/**
  The menu bar of Linux, which is in the window. It shows the MenuGroups that Scrite (see
  ScriteMenuBarMenus) and extensions give the hostUri Scrite.App.MainWindow.MenuBar.

  TODO: The native menu bar has a hidden Misc menu for the actions that are in no menu, because
  macOS does not deliver the shortcuts of such actions (see native/NativeMenuGroupMenu.qml). This
  bar has none, on the assumption that Qt Quick Controls actions get their shortcuts regardless of
  menus. If that turns out to be wrong on Linux, a hidden menu is needed here too.
  */
MenuBar {
    id: root

    readonly property MenuHost host: MenuHost {
        uri: "Scrite.App.MainWindow.MenuBar"
    }

    Instantiator {
        model: root.host

        delegate: EntriesMenu {
            required property int index
        }

        onObjectAdded: (index, object) => root.insertMenu(index, object)
        onObjectRemoved: (index, object) => root.removeMenu(object)
    }
}
