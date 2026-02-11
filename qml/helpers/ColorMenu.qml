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
                model: SceneColors.palette.concat(Runtime.workspaceSettings.customColors)
                delegate: colorItemDelegate
            }

            VclToolButton {
                icon.source: "qrc:/icons/content/add_circle_outline.png"
                suggestedWidth: colorGrid.cellSize
                suggestedHeight: colorGrid.cellSize
                toolTipText: "Pick a custom color"

                onClicked: {
                    let color = Color.pick("white")
                    let colors = Runtime.workspaceSettings.customColors
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
            required property int index
            required property color modelData

            Component.onCompleted: {
                if(modelData == selectedColor)
                    colorGrid.currentIndex = index
            }

            width: parent.cellSize
            height: parent.cellSize
            color: (colorGrid.currentIndex === index) ? Color.translucent(Scrite.app.palette.highlight, 0.25) : Qt.rgba(0,0,0,0)

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
