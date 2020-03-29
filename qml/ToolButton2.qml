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

ToolButton {
    id: toolButton

    property real suggestedWidth: 120
    property real suggestedHeight: 50
    property string shortcut
    property string shortcutText: shortcut

    font.pixelSize: 16
    hoverEnabled: true
    display: AbstractButton.TextBesideIcon
    opacity: enabled ? 1 : 0.5
    background: Rectangle {
        implicitWidth: toolButton.suggestedWidth
        implicitHeight: toolButton.suggestedHeight
        color: toolButton.down ? "gray" : "lightgray"
        border { width: 1; color: "gray" }
        radius: 3
        Behavior on color { ColorAnimation { duration: 250 } }
    }
    contentItem: Item {
        Row {
            anchors.centerIn: parent
            spacing: 10

            Image {
                source: toolButton.icon.source
                width: toolButton.icon.width
                height: toolButton.icon.height
                anchors.verticalCenter: parent.verticalCenter
                visible: status === Image.Ready
            }

            Text {
                text: toolButton.action.text
                color: toolButton.down ? "white" : "black"
                font.pixelSize: toolButton.font.pixelSize
                font.bold: toolButton.down
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: 250 } }
            }
        }

        Text {
            font.pixelSize: 9
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            color: toolButton.down ? "white" : "black"
            text: "[" + toolButton.shortcutText + "]"
            visible: toolButton.shortcut !== ""
        }
    }
    action: Action {
        text: toolButton.text
        shortcut: toolButton.shortcut
    }
}
