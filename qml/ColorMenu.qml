/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13

Menu {
    id: colorMenu
    width: minCellSize * 5 + 10
    height: minCellSize * 4 + 10

    signal menuItemClicked(string color)
    readonly property real minCellSize: 50

    MenuItem {
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
                model: app.standardColors

                Rectangle {
                    width: parent.cellSize
                    height: parent.cellSize
                    color: (colorGrid.currentIndex === index) ? app.translucent(app.palette.highlight, 0.25) : Qt.rgba(0,0,0,0)

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
    }
}
