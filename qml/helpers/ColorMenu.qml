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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

VclMenu {
    id: root

    readonly property real minCellSize: 50
    property color selectedColor: "white"

    signal menuItemClicked(string color)

    width: minCellSize * 5 + 10
    height: minCellSize * (4 + Math.ceil((Runtime.workspaceSettings.customColors.length+1)/4)) +  10

    VclMenuItem {
        width: root.width
        height: colorGrid.height

        background: Item { }
        contentItem: Grid {
            id: colorGrid
            width: root.width
            property int currentIndex: -1

            property real cellSize: width / columns
            columns: Math.floor(width / minCellSize)

            Repeater {
                model: Scrite.app.standardColors.concat(Runtime.workspaceSettings.customColors)
                delegate: colorItemDelegate
            }

            VclToolButton {
                icon.source: "qrc:/icons/content/add_circle_outline.png"
                suggestedWidth: colorGrid.cellSize
                suggestedHeight: colorGrid.cellSize
                ToolTip.text: "Pick a custom color"
                onClicked: {
                    var color = Scrite.app.pickColor("white")
                    var colors = Runtime.workspaceSettings.customColors
                    colors.unshift(color)
                    if(colors.length > 10)
                        colors.pop()
                    Runtime.workspaceSettings.customColors = colors

                    root.menuItemClicked(modelData)
                    root.close()
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
                    root.menuItemClicked(modelData)
                    root.close()
                }
                onEntered: colorGrid.currentIndex = index
            }
        }
    }
}
