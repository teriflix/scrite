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
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../globals"
import "../controls"

/**
  A toolbar that shows the ToolBarGroups that Scrite and extensions declare for its uri. Each group
  is shown as a row of buttons (expanded), as one menu button (menu), or not at all (hidden). When
  availableWidth is set, the groups with the lowest priority are collapsed first, so that all
  of them fit in it. Without availableWidth, every group is expanded.

  Several toolbars can have the same uri, and each shows all the groups of it. The context is what
  actions read from the source of their onTriggered handler, and what the delegates of groups are
  given.
  */
Item {
    id: root

    property string uri
    property AbstractActionContext context
    property real availableWidth: 0
    property real roomyWidth: 0

    // True when nothing is collapsed, and there is at least roomyWidth to spare.
    readonly property bool roomy: _host.roomy

    implicitWidth: _row.implicitWidth
    implicitHeight: _row.implicitHeight

    ToolBarHost {
        id: _host

        uri: root.uri
        context: root.context
        availableWidth: root.availableWidth
        roomyWidth: root.roomyWidth
    }

    RowLayout {
        id: _row

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        spacing: 0

        Repeater {
            model: _host

            delegate: Item {
                id: _group

                required property int index
                required property ToolBarGroup group
                required property int level

                // What the group has in its expanded form: the actions of its action manager, its
                // entries, what its delegate makes, and the same from the groups merged into it.
                readonly property var entries: {
                    const list = []
                    _private.collectEntries(_group.group, list)
                    return list
                }

                // A backdrop sets its group apart, so the group before it needs no separator.
                readonly property ToolBarGroup nextGroup: _host.groupAt(_group.index + 1) as ToolBarGroup
                readonly property bool showSeparator: _group.group.separator && !(_group.nextGroup && _group.nextGroup.backdrop)

                readonly property real separatorWidth: _group.showSeparator ? 1 : 0

                function reportWidths() {
                    _host.setGroupWidths(_group.index,
                                         _expandedRow.implicitWidth + separatorWidth,
                                         _menuButton.implicitWidth + separatorWidth)
                }

                Layout.fillHeight: true

                implicitWidth: _content.implicitWidth
                implicitHeight: _content.implicitHeight
                visible: _group.level !== ToolBarHost.Hidden

                // The backdrop of groups that have one, which includes what is merged into them.
                Loader {
                    anchors.fill: parent
                    anchors.topMargin: 3
                    anchors.bottomMargin: 3

                    sourceComponent: _group.group.backdrop
                }

                RowLayout {
                    id: _content

                    anchors.fill: parent

                    spacing: 0

                    // The expanded form is always there, only hidden when it isn't shown, so that
                    // its width is known.
                    RowLayout {
                        id: _expandedRow

                        Layout.fillHeight: true

                        spacing: 0
                        visible: _group.level === ToolBarHost.Expanded

                        Repeater {
                            model: _group.entries

                            delegate: Loader {
                                id: _entryLoader

                                required property var modelData

                                // An action that isn't visible takes no room either, which the button
                                // that it makes cannot tell the layout, because a loader is as wide as
                                // what it loads even when that is hidden.
                                readonly property Action entryAction: modelData as Action

                                Layout.fillHeight: true

                                visible: entryAction && entryAction.visible !== undefined ? entryAction.visible : true

                                sourceComponent: {
                                    if(_entryLoader.modelData === null || _entryLoader.modelData === undefined)
                                        return null
                                    if(_entryLoader.modelData.delegateGroup !== undefined)
                                        return _delegateItemComponent
                                    if(_entryLoader.modelData as MenuGroup)
                                        return _groupMenuButtonComponent
                                    if(_entryLoader.modelData as ActionManager)
                                        return _managerMenuButtonComponent
                                    if(_entryLoader.modelData as Action)
                                        return _actionButtonComponent
                                    return null
                                }

                                Component {
                                    id: _actionButtonComponent

                                    ActionToolButton {
                                        action: _entryLoader.modelData as Action
                                        context: root.context
                                    }
                                }

                                Component {
                                    id: _managerMenuButtonComponent

                                    ActionManagerToolButton {
                                        actionManager: _entryLoader.modelData as ActionManager
                                    }
                                }

                                Component {
                                    id: _groupMenuButtonComponent

                                    GroupMenuButton {
                                        group: _entryLoader.modelData as MenuGroup
                                        context: root.context
                                    }
                                }

                                Component {
                                    id: _delegateItemComponent

                                    Item {
                                        id: _delegateItem

                                        property Item delegateItem: null

                                        implicitWidth: delegateItem ? delegateItem.implicitWidth : 0
                                        implicitHeight: delegateItem ? delegateItem.implicitHeight : 0

                                        Component.onCompleted: {
                                            const component = _entryLoader.modelData.delegateGroup.delegate
                                            if(component) {
                                                delegateItem = component.createObject(_delegateItem, {"context": Qt.binding( () => { return root.context } )}) as Item
                                                if(delegateItem)
                                                    delegateItem.anchors.verticalCenter = _delegateItem.verticalCenter
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        onImplicitWidthChanged: _group.reportWidths()
                    }

                    GroupMenuButton {
                        id: _menuButton

                        Layout.fillHeight: true

                        group: _group.group
                        context: root.context
                        visible: _group.level === ToolBarHost.Menu

                        onImplicitWidthChanged: _group.reportWidths()
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 1

                        color: Runtime.colors.primary.borderColor
                        visible: _group.showSeparator
                    }
                }

                onIndexChanged: reportWidths()
                Component.onCompleted: reportWidths()
            }
        }
    }

    component GroupMenuButton: ToolButton {
        id: _button

        required property AbstractActionGroup group
        property AbstractActionContext context: null

        Material.background: "transparent"

        display: icon.source.toString() !== "" ? ToolButton.IconOnly : ToolButton.TextOnly
        down: _menu.visible
        focusPolicy: Qt.NoFocus
        text: group.title

        icon.color: "transparent"
        icon.source: group.iconSource !== "" ? Runtime.themedIcon(group.iconSource) : ""

        onClicked: _menu.open()

        Item {
            anchors.bottom: parent.bottom

            width: _menu.width
            height: 1

            EntriesMenu {
                id: _menu

                group: _button.group
                context: _button.context
            }
        }

        ToolTipPopup {
            container: _button

            text: _button.display === ToolButton.IconOnly ? _button.group.title : ""
            visible: text !== "" && _button.hovered
        }
    }

    QtObject {
        id: _private

        function collectEntries(group, list) {
            const manager = group.actionManager
            if(manager)
                manager.visibleActions.forEach( (action) => list.push(action) )

            group.entries.forEach( (entry) => list.push(entry) )

            if(group.delegate)
                list.push({"delegateGroup": group})

            group.merged.forEach( (merged) => collectEntries(merged, list) )
        }
    }
}
