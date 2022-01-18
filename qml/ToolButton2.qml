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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

ToolButton {
    id: toolButton

    property real suggestedWidth: {
        if(display === AbstractButton.IconOnly || text.length === 0)
            return suggestedHeight
        return 120
    }
    property real suggestedHeight: 55
    property string shortcut
    property string shortcutText: shortcut
    implicitWidth: suggestedWidth
    implicitHeight: suggestedHeight

    Material.background: accentColors.key
    Material.foreground: primaryColors.key

    font.pixelSize: 16
    hoverEnabled: true
    display: AbstractButton.TextBesideIcon
    opacity: enabled ? 1 : 0.5
    flat: true
    contentItem: Rectangle {
        color: primaryColors.c10.background
        border.width: toolButton.flat ? 0 : 1
        border.color: primaryColors.borderColor

        Row {
            anchors.centerIn: parent
            spacing: 10

            Image {
                source: toolButton.icon.source
                width: toolButton.icon.width
                height: toolButton.icon.height
                anchors.verticalCenter: parent.verticalCenter
                visible: status === Image.Ready
                smooth: true
                mipmap: true
            }

            Column {
                spacing: parent.spacing/2
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: toolButton.action.text
                    font.pixelSize: toolButton.font.pixelSize
                    font.bold: toolButton.down
                    Behavior on color {
                        enabled: applicationSettings.enableAnimations
                        ColorAnimation { duration: 250 }
                    }
                    visible: toolButton.display !== AbstractButton.IconOnly
                }

                Text {
                    font.pixelSize: 9
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "[" + toolButton.shortcutText + "]"
                    visible: toolButton.shortcutText !== ""
                }
            }
        }
    }
    action: Action {
        text: toolButton.text
        shortcut: toolButton.shortcut
    }

    ToolTip.text: shortcutText === "" ? text : (text + "\t(" + Scrite.app.polishShortcutTextForDisplay(shortcutText) + ")")
    ToolTip.visible: ToolTip.text === "" ? false : hovered
}
