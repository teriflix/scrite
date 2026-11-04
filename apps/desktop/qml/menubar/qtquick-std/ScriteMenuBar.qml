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
  The menu bar of Windows and Linux, which is in the window and follows the color scheme of the app. It shows the MenuGroups that Scrite (see
  ScriteMenuBarMenus) and extensions give the hostUri Scrite.App.MainWindow.MenuBar.

  It is meant to be used with the mouse. Keyboard navigation of the menus is in
  qtquick-keynavigation/ScriteMenuBar.qml, which is built when SCRITE_MENUBAR_KEY_NAVIGATION is ON.

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

    // The menu bar is not used while a dialog box is open, or when the window is not active.
    readonly property bool usable: Scrite.window.active && Runtime.dialogs.objectCount === 0

    enabled: usable

    // Qt Quick Controls moves focus to the menu bar when Alt is pressed and released, even if a
    // shortcut was used with Alt in between. So Alt never reaches it.
    EventFilter.target: Scrite.app
    EventFilter.events: [EventFilter.KeyPress, EventFilter.KeyRelease, EventFilter.ShortcutOverride]
    EventFilter.onFilter: (watched, event, result) => {
                              if(event.key === Qt.Key_Alt) {
                                  result.filter = true
                                  result.acceptEvent = true
                              }
                          }

    Instantiator {
        model: root.host

        delegate: EntriesMenu {
            required property int index
        }

        onObjectAdded: (index, object) => root.insertMenu(index, object)
        onObjectRemoved: (index, object) => root.removeMenu(object)
    }

    ActionHandler {
        action: ActionHub.applicationOptions.find("toggleMenubarVisibility")

        checked: root.visible
        onToggled: () => {
                       Qt.callLater(toggleVisibility)
                   }

        function toggleVisibility() {
            root.visible = !root.visible
        }
    }
}
