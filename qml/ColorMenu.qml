/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
    property var colors: ["blue", "magenta", "purple", "yellow", "orange", "red", "brown", "gray", "white"]

    signal menuItemClicked(string color)

    Repeater {
        model: colors

        MenuItem {
            height: 33

            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter

                Item { width: 10; height: 10 }

                Rectangle {
                    width: 25
                    height: 25
                    border { width: 1; color: "lightgray" }
                    color: modelData
                    opacity: 0.75
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData
                    font.capitalization: Font.Capitalize
                }
            }

            onClicked: colorMenu.menuItemClicked(modelData)
        }
    }
}
