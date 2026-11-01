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

  Unlike qtquick-std/ScriteMenuBar.qml, the keyboard can navigate this one: Alt gives focus to the
  first menu, arrow keys move between menus, Down, Return and Enter open the highlighted menu, and
  Esc gives focus back. It is built when SCRITE_MENUBAR_KEY_NAVIGATION is ON.

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

    // Pressing and releasing Alt, with no other key or mouse button in between, gives focus to the
    // first menu, just as it does in a native menu bar.
    readonly property bool usable: Scrite.window.active && Runtime.dialogs.objectCount === 0
    property bool altArmed: false
    property Item itemToRefocus

    // Gives focus to the first item of the menu bar, from where arrow keys move between items.
    function focusFirstItem() {
        if(root.count === 0)
            return

        root.itemToRefocus = Window.window.activeFocusItem
        root.currentIndex = 0
        root.itemAt(0).forceActiveFocus(Qt.TabFocusReason)
        root.itemAt(0).highlighted = true
    }

    // Opens the menu of the item that is highlighted, which is the one that arrow keys moved to. It
    // is not always the one that has active focus, nor the current item of the bar.
    function openFocusedMenu() {
        const index = root.highlightedIndex >= 0 ? root.highlightedIndex : root.currentIndex
        root.currentIndex = index
        root.menuAt(index).open()
    }

    // Index of the highlighted item, or -1.
    readonly property int highlightedIndex: {
        for(let i = 0; i < root.count; i++) {
            if(root.itemAt(i).highlighted)
                return i
        }
        return -1
    }

    // When arrow keys move the highlight while a menu is open, that menu closes and the menu of the
    // highlighted item opens, as it does in Windows apps.
    onHighlightedIndexChanged: {
        if(highlightedIndex < 0)
            return

        for(let i = 0; i < root.count; i++) {
            const menu = root.menuAt(i)
            if(i !== highlightedIndex && menu.opened) {
                menu.close()
                root.menuAt(highlightedIndex).open()
                return
            }
        }
    }

    function restoreFocus() {
        for(let i = 0; i < root.count; i++)
            root.itemAt(i).highlighted = false

        if(root.itemToRefocus)
            root.itemToRefocus.forceActiveFocus()
        root.itemToRefocus = null
    }

    EventFilter.target: Scrite.app
    EventFilter.events: [EventFilter.KeyPress, EventFilter.KeyRelease, EventFilter.ShortcutOverride, EventFilter.MouseButtonPress]
    EventFilter.onFilter: (watched, event, result) => {
                              // Qt Quick Controls moves focus to the menu bar when Alt is pressed and
                              // released, even if a shortcut was used with Alt in between. So Alt never
                              // reaches it. This menu bar decides by itself when to take focus.
                              if(event.key === Qt.Key_Alt && event.type !== EventFilter.MouseButtonPress) {
                                  result.filter = true
                                  result.acceptEvent = true
                              }

                              if(!root.usable) {
                                  root.altArmed = false
                                  return
                              }

                              if(event.type === EventFilter.KeyPress) {
                                  if(event.key === Qt.Key_Alt) {
                                    root.altArmed = !event.isAutoRepeat && !event.controlModifier
                                  } else {
                                    root.altArmed = false
                                  }
                              } else if(event.type === EventFilter.KeyRelease) {
                                  // A shortcut that uses Alt consumes the press of its key, so only the
                                  // release of that key shows that it was used.
                                  if(event.key !== Qt.Key_Alt) {
                                      root.altArmed = false
                                  } else if(root.altArmed) {
                                      root.altArmed = false
                                      root.focusFirstItem()
                                  }
                              } else {
                                  root.altArmed = false
                              }
                          }

    enabled: usable

    Keys.onDownPressed: openFocusedMenu()
    Keys.onReturnPressed: openFocusedMenu()
    Keys.onEnterPressed: openFocusedMenu()
    Keys.onEscapePressed: restoreFocus()

    Instantiator {
        model: root.host

        delegate: EntriesMenu {
            required property int index

            keyboardNavigable: true
        }

        onObjectAdded: (index, object) => root.insertMenu(index, object)
        onObjectRemoved: (index, object) => root.removeMenu(object)
    }
}
