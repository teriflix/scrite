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
import QtQuick
import QtQuick.Controls as Controls
import Qt.labs.platform as Native

import Scrite.App

import "../../globals"
import "../../native"

/**
  A native menu that shows everything that a group has: the natively visible actions of its action
  manager, its entries, what its delegate makes, and the groups merged into it. The counterpart of
  EntriesMenu for native menus. Submenus are made from this same file, when they are needed.

  The menu of Tools on macOS has a hidden "Misc" menu, to which every action that is not shown in
  any other menu goes, because macOS requires that every action that can have a shortcut is in a
  menu.
  */
Native.Menu {
    id: root

    required property AbstractActionGroup group

    title: group ? group.title : ""

    onGroupChanged: _private.scheduleRebuild()

    Component.onCompleted: _private.scheduleRebuild()

    Component.onDestruction: _private.disconnectObserved()

    QtObject {
        id: _private

        // macOS does not deliver the shortcut of an action that isn't in a native menu, so every
        // action that isn't in one goes into the hidden Misc menu of Tools.
        //
        // TODO: This is for macOS only for now. If shortcuts of actions that are not in the menu bar
        // turn out not to be delivered on Windows (native menu bar, same as here) or on Linux (the
        // in-window menu bar of menubar/linux, which has no such menu), the same is needed there:
        // drop the Platform check here, and for Linux add a hidden menu to EntriesMenu of Tools.
        readonly property bool isToolsMenu: Platform.isMacOSDesktop && root.group !== null &&
                                            root.group.uri === "Scrite.MenuBar.Tools"

        property bool pendingSeparator: false
        property bool lastWasSeparator: false
        property int cursor: 0
        property var added: []         // Items and menus that are in the menu on behalf of the group.
        property var made: []          // Everything made along the way, destroyed with the items.
        property var observed: []      // Signals that cause a rebuild, as {object, name}.
        property var menuComponent: null
        property var catchAllMenu: null

        // Catch all menu for every other action thats not already covered. Made only for the menu
        // that needs it, and not for every menu that is made from this file.
        readonly property Component catchAllComponent: Component {
            Native.Menu {
                id: _catchAllMenu

                title: "Misc"
                visible: false

                Instantiator {
                    model: Runtime.nativelyNotShownActions

                    delegate: NativeActionMenuItem {
                        required property int index
                        required property var qmlAction
                        required property var actionManager
                        required property bool shortcutIsEditable
                        required property string groupName

                        property bool nativelyShownAlready: Object.queryProperty(action, "#nativelyShown")

                        action: qmlAction
                        text: actionManager.title + ": " + action.text
                        visible: true
                    }

                    onObjectAdded: (index, object) => _catchAllMenu.addItem(object)
                    onObjectRemoved: (index, object) => _catchAllMenu.removeItem(object)
                }
            }
        }

        readonly property Component wrapperGroupComponent: Component {
            MenuGroup { }
        }

        readonly property Component separatorComponent: Component {
            Native.MenuSeparator { }
        }

        readonly property Component actionItemComponent: Component {
            NativeActionMenuItem { }
        }

        // A timer goes with this menu, so that a rebuild that is waiting doesn't run after it.
        readonly property Timer rebuildTimer: Timer {
            interval: 0
            onTriggered: _private.rebuild()
        }

        function scheduleRebuild() {
            rebuildTimer.restart()
        }

        function rebuild() {
            clearContributed()

            if(root.group === null)
                return

            // Made once, before the content, so that the content is put ahead of it.
            if(isToolsMenu && catchAllMenu === null) {
                catchAllMenu = catchAllComponent.createObject(root)
                root.addMenu(catchAllMenu)
            }

            cursor = 0
            pendingSeparator = false
            lastWasSeparator = false
            renderContent(root.group)
            pendingSeparator = false
        }

        function clearContributed() {
            disconnectObserved()

            const objects = added
            added = []
            objects.forEach( (object) => {
                                if(object as Native.Menu)
                                    root.removeMenu(object)
                                else
                                    root.removeItem(object)
                                object.destroy()
                            } )

            const leftovers = made
            made = []
            leftovers.forEach( (object) => object.destroy() )
        }

        function disconnectObserved() {
            observed.forEach( (entry) => {
                                 try {
                                     entry.object[entry.name].disconnect(scheduleRebuild)
                                 } catch(e) { }
                             } )
            observed = []
        }

        function observe(object, names) {
            names.forEach( (name) => {
                              if(object[name] !== undefined) {
                                  object[name].connect(scheduleRebuild)
                                  observed.push({"object": object, "name": name})
                              }
                          } )
        }

        function insertAtCursor(object, isSeparator) {
            if(object as Native.Menu)
                root.insertMenu(cursor, object)
            else
                root.insertItem(cursor, object)
            ++cursor
            added.push(object)
            lastWasSeparator = isSeparator
        }

        // A separator that groups ask for is put only when an item follows it, and when an item
        // precedes it that isn't a separator already. So menus never start or end with one, and
        // never have two in a row.
        function put(object) {
            if(pendingSeparator) {
                pendingSeparator = false
                if(cursor > 0 && !lastWasSeparator)
                    insertAtCursor(separatorComponent.createObject(root), true)
            }
            insertAtCursor(object, false)
        }

        function putSeparator() {
            pendingSeparator = false
            if(cursor > 0 && !lastWasSeparator)
                insertAtCursor(separatorComponent.createObject(root), true)
        }

        function putAction(action) {
            put(actionItemComponent.createObject(root, {"action": action}))
        }

        function putSubMenu(group) {
            if(menuComponent === null)
                menuComponent = Qt.createComponent(Qt.resolvedUrl("NativeMenuGroupMenu.qml"))
            put(menuComponent.createObject(root, {"group": group}))
        }

        function renderGroup(group) {
            observe(group, ["inlineChanged"])

            if(group.inline !== true) {
                putSubMenu(group)
                return
            }

            // Told apart from what comes before and after it.
            pendingSeparator = true
            renderContent(group)
            pendingSeparator = true
        }

        function renderContent(group) {
            observe(group, ["entriesChanged", "delegateChanged", "mergedChanged", "actionManagerChanged"])

            const manager = group.actionManager
            if(manager) {
                observe(manager, ["nativelyVisibleActionsChanged"])
                manager.nativelyVisibleActions.forEach( (action) => putAction(action) )
            }

            group.entries.forEach( (entry) => renderEntry(entry) )

            if(group.delegate)
                renderDelegate(group)

            group.merged.forEach( (merged) => renderGroup(merged) )
        }

        function renderEntry(entry) {
            if(entry === null || entry === undefined) {
                putSeparator()
                return
            }

            const group = entry as MenuGroup
            if(group) {
                renderGroup(group)
                return
            }

            const manager = entry as ActionManager
            if(manager) {
                const wrapper = wrapperGroupComponent.createObject(root, {"actionManager": manager})
                made.push(wrapper)
                putSubMenu(wrapper)
                return
            }

            const action = entry as Controls.Action
            if(action)
                putAction(action)
        }

        // The menu bar has no context, so what delegates make is made without one. Only actions
        // are shown in native menus.
        function renderDelegate(group) {
            const object = group.delegate.createObject(root, {"context": null})
            if(!object)
                return

            made.push(object)

            const several = object as MenuItemsDelegate
            if(several) {
                for(let i=0; i<several.objects.length; i++)
                    putDelegated(several.objects[i])
            } else {
                putDelegated(object)
            }
        }

        function isInstantiator(object) {
            return typeof object.objectAt === "function" && object.count !== undefined && object.objectAdded !== undefined
        }

        // Instantiators in a MenuItemsDelegate are followed as they make, and remove, objects.
        function putDelegated(object) {
            if(isInstantiator(object)) {
                observe(object, ["objectAdded", "objectRemoved"])
                for(let i=0; i<object.count; i++)
                    putDelegated(object.objectAt(i))
            } else if(object as Controls.Action) {
                putAction(object)
            }
        }
    }
}
