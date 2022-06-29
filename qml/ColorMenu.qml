/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Menu2 {
    id: colorMenu
    width: minCellSize * 5 + 10
    height: minCellSize * (4 + Math.ceil((workspaceSettings.customColors.length+1)/4)) +  10

    signal menuItemClicked(string color)
    readonly property real minCellSize: 50
    property color selectedColor: "white"

    MenuItem2 {
        width: colorMenu.width
        height: colorGrid.height

        background: Item { }
        contentItem: Grid {
            id: colorGrid
            width: colorMenu.width
            property int currentIndex: -1

            property real cellSize: width / columns
            columns: Math.floor(width / minCellSize)

            Repeater {
                model: Scrite.app.standardColors.concat(workspaceSettings.customColors)
                delegate: colorItemDelegate
            }

            ToolButton2 {
                icon.source: "../icons/content/add_circle_outline.png"
                suggestedWidth: colorGrid.cellSize
                suggestedHeight: colorGrid.cellSize
                ToolTip.text: "Pick a custom color"
                onClicked: {
                    var color = Scrite.app.pickColor("white")
                    var colors = workspaceSettings.customColors
                    colors.unshift(color)
                    if(colors.length > 10)
                        colors.pop()
                    workspaceSettings.customColors = colors

                    colorMenu.menuItemClicked(modelData)
                    colorMenu.close()
                }
            }
        }
    }

    Component {
        id: colorItemDelegate

        Rectangle {
            width: parent.cellSize
            height: parent.cellSize
            color: (colorGrid.currentIndex === index) ? Scrite.app.translucent(Scrite.app.palette.highlight, 0.25) : Qt.rgba(0,0,0,0)
            Component.onCompleted: {
                if(modelData == selectedColor)
                    colorGrid.currentIndex = index
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                border {
                    width: (colorGrid.currentIndex === index) ? 3 : 1;
                    color: Qt.darker(modelData)
                }
                color: modelData
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    colorMenu.menuItemClicked(modelData)
                    colorMenu.close()
                }
                onEntered: colorGrid.currentIndex = index
            }
        }
    }
}
