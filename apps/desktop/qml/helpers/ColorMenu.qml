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

import QtQuick
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

VclMenu {
    id: root

    readonly property real minCellSize: 50
    property color selectedColor: "white"

    signal menuItemClicked(string color)

    width: minCellSize * 5 + 10
    height: minCellSize * (4 + Math.ceil((Runtime.workspaceSettings.customColors.length+1)/4)) +  10

    VclMenuItem {
        width: root.width
        height: _colorGrid.height

        background: Item { }
        contentItem: Grid {
            id: _colorGrid

            property int currentIndex: -1
            property real cellSize: width / columns

            width: root.width

            columns: Math.floor(width / root.minCellSize)

            Repeater {
                model: SceneColors.palette.concat(Runtime.workspaceSettings.customColors)
                delegate: _colorItemDelegate
            }

            VclToolButton {
                icon.source: "qrc:/icons/content/add_circle_outline.png"
                suggestedWidth: _colorGrid.cellSize
                suggestedHeight: _colorGrid.cellSize
                toolTipText: "Pick a custom color"

                onClicked: {
                    let color = Color.pick("white")
                    let colors = Runtime.workspaceSettings.customColors
                    colors.unshift(color)
                    if(colors.length > 10)
                        colors.pop()
                    Runtime.workspaceSettings.customColors = colors

                    root.menuItemClicked(color)
                    root.close()
                }
            }
        }
    }

    Component {
        id: _colorItemDelegate

        Rectangle {
            id: _colorDelegateItem

            required property int index
            required property color modelData

            Component.onCompleted: {
                if(modelData == root.selectedColor)
                    _colorGrid.currentIndex = index
            }

            width: _colorGrid.cellSize
            height: _colorGrid.cellSize
            color: (_colorGrid.currentIndex === index) ? Color.translucent(Runtime.palette.highlight, 0.25) : Qt.rgba(0,0,0,0)

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                border {
                    width: (_colorGrid.currentIndex === _colorDelegateItem.index) ? 3 : 1;
                    color: Qt.darker(_colorDelegateItem.modelData)
                }
                color: _colorDelegateItem.modelData
            }

            MouseArea {
                id: _mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    root.menuItemClicked(_colorDelegateItem.modelData)
                    root.close()
                }
                onEntered: _colorGrid.currentIndex = _colorDelegateItem.index
            }
        }
    }
}
