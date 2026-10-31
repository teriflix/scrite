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
import QtQuick.Controls

import Scrite.App

import "../globals"
import "../helpers"

Menu {
    id: root

    property bool autoWidth: true

    property int minInsertIndex: 0
    property int insertIndex: minInsertIndex

    // Setting uri makes this menu a host with that uri: it shows the MenuGroups that Scrite and
    // extensions give that hostUri. Their items go after the items of this menu (or from
    // insertIndex, if that is above zero), and are kept up to date as groups come and go. The
    // actions in them are given context through the source of their onTriggered handler. Several
    // menus can have the same uri.
    property string uri
    property AbstractActionContext context

    // Used by EntriesMenu, to show the contents of a single group, as a submenu of it.
    property AbstractActionGroup group

    function includeMenu(object) {
        let menu = object as Menu
        if(menu) {
            insertMenu(insertIndex, menu)
            ++insertIndex
            return
        }
    }

    function includeMenuItem(object) {
        let menuItem = object as MenuItem
        if(menuItem) {
            insertItem(insertIndex, menuItem)
            ++insertIndex
            return
        }
    }

    function includeAction(object) {
        let action = object as Action
        if(action) {
            insertAction(insertIndex, action)
            ++insertIndex
            return
        }
    }

    function excludeMenu(object) {
        let menu = object as Menu
        if(menu) {
            for(let i=0; i<count; i++) {
                if(menuAt(i) === menu) {
                    takeMenu(i)
                    insertIndex = Math.max(insertIndex-1, minInsertIndex)
                    return
                }
            }
        }
    }

    function excludeMenuItem(object) {
        let menuItem = object as MenuItem
        if(menuItem) {
            for(let i=0; i<count; i++) {
                if(itemAt(i) === menuItem) {
                    takeItem(i)
                    insertIndex = Math.max(insertIndex-1, minInsertIndex)
                    return
                }
            }
        }
    }

    function excludeAction(object) {
        let action = object as Action
        if(action) {
            for(let i=0; i<count; i++) {
                if(actionAt(i) === action) {
                    takeAction(i)
                    insertIndex = Math.max(insertIndex-1, minInsertIndex)
                    return
                }
            }
        }
    }

    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    closePolicy: Popup.CloseOnEscape|Popup.CloseOnPressOutside

    onAboutToShow: Qt.callLater(determineWidth)

    function determineWidth() {
        if(autoWidth)
            Runtime.execLater(root, Runtime.stdAnimationDuration/2, __determineWidth)
    }

    function __determineWidth() {
        if(autoWidth) {
            let maxWidth = 0
            for(let i=0; i<count; i++) {
                let menuItem = itemAt(i)
                maxWidth = Math.max(menuItem.implicitWidth, maxWidth)
            }
            width = maxWidth + leftPadding + rightPadding
        }
    }

    onUriChanged: _private.activate()
    onGroupChanged: _private.activate()
    onContextChanged: _private.scheduleRebuild()

    Component.onCompleted: _private.activate()

    Component.onDestruction: _private.deactivate()

    QtObject {
        id: _private

        // Everything below is made only when this menu is a host.
        property bool active: false
        property bool pendingSeparator: false
        property bool lastWasSeparator: false
        property int cursor: 0
        property var host: null
        property var added: []         // What is in the menu on behalf of the groups.
        property var wrappers: []      // Groups made for ActionManagers, destroyed with the menus.
        property var delegates: []     // Objects made from group delegates, one for each group.
        property var observed: []      // Signals that cause a rebuild, as {object, name}.
        property var entriesMenuComponent: null

        readonly property Component hostComponent: Component {
            MenuHost { }
        }

        readonly property Component wrapperGroupComponent: Component {
            MenuGroup { }
        }

        readonly property Component separatorComponent: Component {
            MenuSeparator { }
        }

        readonly property Component actionItemComponent: Component {
            SctMenuItem {
                id: _menuItem

                required property var qmlAction

                action: qmlAction
                focusPolicy: Qt.NoFocus
                opacity: enabled ? 1 : 0.5

                icon.source: qmlAction.icon.source !== undefined ? Runtime.themedIcon(qmlAction.icon.source) : ""
                icon.color: qmlAction.icon.color

                font.pointSize: Runtime.idealFontMetrics.font.pointSize

                ToolTipPopup {
                    container: _menuItem

                    text: _menuItem.qmlAction.tooltip !== undefined ? _menuItem.qmlAction.tooltip : ""
                    visible: text !== "" && _menuItem.hovered
                }
            }
        }

        function activate() {
            if(active || (root.uri === "" && root.group === null))
                return

            if(root.group === null) {
                host = hostComponent.createObject(root, {"uri": root.uri})
                host.context = Qt.binding( () => { return root.context } )
                host.modelReset.connect(scheduleRebuild)
            }

            active = true
            scheduleRebuild()
        }

        function deactivate() {
            if(!active)
                return

            active = false
            disconnectObserved()
        }

        // A timer goes with this menu, so that a rebuild that is waiting doesn't run after it.
        readonly property Timer rebuildTimer: Timer {
            interval: 0
            onTriggered: _private.rebuild()
        }

        function scheduleRebuild() {
            if(active)
                rebuildTimer.restart()
        }

        function rebuild() {
            if(!active)
                return

            clear()

            // After the items of the menu, unless insertIndex says otherwise.
            cursor = root.insertIndex > 0 ? Math.min(root.insertIndex, root.count) : root.count
            pendingSeparator = false
            lastWasSeparator = cursor > 0 && (root.itemAt(cursor-1) as MenuSeparator) !== null

            const touchedGroups = []
            if(root.group !== null) {
                renderContent(root.group, touchedGroups)
            } else if(host !== null) {
                for(let i=0; i<host.count; i++)
                    renderGroup(host.groupAt(i), touchedGroups)
            }

            finishSeparators()

            discardUntouchedDelegates(touchedGroups)

            root.determineWidth()
        }

        function clear() {
            disconnectObserved()

            const objects = added
            added = []
            objects.forEach( (object) => take(object) )

            const leftovers = wrappers
            wrappers = []
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

        function isDelegateObject(object) {
            return delegates.some( (entry) => entry.object === object || (entry.parts && entry.parts.includes(object)) )
        }

        function take(object) {
            const menu = object as Menu
            if(menu) {
                for(let i=0; i<root.count; i++) {
                    if(root.menuAt(i) === menu) {
                        root.takeMenu(i)
                        break
                    }
                }
                if(!isDelegateObject(object))
                    menu.destroy()
                return
            }

            for(let j=0; j<root.count; j++) {
                if(root.itemAt(j) === object) {
                    root.takeItem(j)
                    break
                }
            }
            if(!isDelegateObject(object))
                object.destroy()
        }

        function insertAtCursor(object, isSeparator) {
            if(object as Menu)
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
                    insertAtCursor(separatorComponent.createObject(root.contentItem), true)
            }
            insertAtCursor(object, false)
        }

        function putSeparator() {
            pendingSeparator = false
            if(cursor > 0 && !lastWasSeparator)
                insertAtCursor(separatorComponent.createObject(root.contentItem), true)
        }

        // The items of the menu itself may follow what was put last.
        function finishSeparators() {
            if(pendingSeparator && cursor < root.count && !lastWasSeparator &&
                    (root.itemAt(cursor) as MenuSeparator) === null)
                insertAtCursor(separatorComponent.createObject(root.contentItem), true)
            pendingSeparator = false
        }

        function putAction(action) {
            // Made under the content item of the menu, which is an Item, until it is put in the menu.
            put(actionItemComponent.createObject(root.contentItem, {"qmlAction": action, "context": root.context}))
        }

        function putSubMenu(group) {
            if(entriesMenuComponent === null)
                entriesMenuComponent = Qt.createComponent(Qt.resolvedUrl("../helpers/EntriesMenu.qml"))
            put(entriesMenuComponent.createObject(root, {"group": group, "context": root.context}))
        }

        // A group is either a submenu, or if it is inline, its items follow a separator. The groups
        // of toolbars are the buttons of one row, and are flat in a menu, one after the other.
        function renderGroup(group, touchedGroups) {
            if((group as ToolBarGroup) !== null) {
                renderContent(group, touchedGroups)
                return
            }

            observe(group, ["inlineChanged"])

            if(group.inline !== true) {
                putSubMenu(group)
                return
            }

            // Told apart from what comes before and after it.
            pendingSeparator = true
            renderContent(group, touchedGroups)
            pendingSeparator = true
        }

        function renderContent(group, touchedGroups) {
            observe(group, ["entriesChanged", "delegateChanged", "mergedChanged", "actionManagerChanged"])

            const manager = group.actionManager
            if(manager) {
                observe(manager, ["visibleActionsChanged"])
                manager.visibleActions.forEach( (action) => putAction(action) )
            }

            group.entries.forEach( (entry) => renderEntry(entry, touchedGroups) )

            // The delegates of toolbar groups make buttons, which menus don't show, and aren't made.
            if(group.delegate && (group as ToolBarGroup) === null)
                renderDelegate(group, touchedGroups)

            group.merged.forEach( (merged) => renderGroup(merged, touchedGroups) )
        }

        function renderEntry(entry, touchedGroups) {
            if(entry === null || entry === undefined) {
                putSeparator()
                return
            }

            const group = entry as MenuGroup
            if(group) {
                renderGroup(group, touchedGroups)
                return
            }

            const manager = entry as ActionManager
            if(manager) {
                const wrapper = wrapperGroupComponent.createObject(root, {"actionManager": manager})
                wrappers.push(wrapper)
                putSubMenu(wrapper)
                return
            }

            const action = entry as Action
            if(action)
                putAction(action)
        }

        // Delegates are made once for each group, with the context of this menu, and are destroyed
        // when the group goes or when this menu does.
        function renderDelegate(group, touchedGroups) {
            touchedGroups.push(group)

            let entry = delegates.find( (e) => e.group === group )
            if(entry && entry.component !== group.delegate) {
                discardDelegate(entry)
                entry = undefined
            }

            if(!entry) {
                const object = group.delegate.createObject(root, {"context": Qt.binding( () => { return root.context } )})
                if(!object)
                    return
                entry = {"group": group, "component": group.delegate, "object": object}
                delegates.push(entry)
            }

            // What the delegate made that is put in the menu as it is, which isn't destroyed with it.
            entry.parts = []

            const delegateObject = entry.object
            const several = delegateObject as MenuItemsDelegate
            if(several) {
                for(let i=0; i<several.objects.length; i++)
                    putDelegated(several.objects[i], entry)
            } else {
                putDelegated(delegateObject, entry)
            }
            // Delegates of toolbar groups make buttons, which menus don't show.
        }

        function isInstantiator(object) {
            return typeof object.objectAt === "function" && object.count !== undefined && object.objectAdded !== undefined
        }

        // Instantiators in a MenuItemsDelegate are followed as they make, and remove, objects.
        function putDelegated(object, entry) {
            if(isInstantiator(object)) {
                observe(object, ["objectAdded", "objectRemoved"])
                for(let i=0; i<object.count; i++)
                    putDelegated(object.objectAt(i), entry)
            } else if(object as Menu) {
                entry.parts.push(object)
                put(object)
            } else if(object as MenuItem) {
                entry.parts.push(object)
                put(object)
            } else if(object as Action) {
                putAction(object)
            }
        }

        function discardDelegate(entry) {
            delegates = delegates.filter( (e) => e !== entry )
            if(entry.parts)
                entry.parts.forEach( (part) => take(part) )
            entry.object.destroy()
        }

        function discardUntouchedDelegates(touchedGroups) {
            delegates.filter( (e) => !touchedGroups.includes(e.group) ).forEach( (e) => discardDelegate(e) )
        }
    }
}
