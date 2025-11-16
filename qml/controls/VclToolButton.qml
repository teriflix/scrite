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
import QtQuick.Controls.Material 2.15

import "qrc:/qml/globals"
import "qrc:/qml/helpers"

ToolButton {
    id: root

    property real suggestedWidth: {
        if(display === AbstractButton.IconOnly || text.length === 0)
            return suggestedHeight
        return 120
    }
    property real suggestedHeight: 55

    property string shortcut
    property string shortcutText: shortcut

    property bool toolTipVisible: hovered
    property string toolTipText: shortcutText === "" ? text : (text + "\t(" + Gui.nativeShortcut(shortcutText) + ")")

    Material.theme: Runtime.colors.theme
    Material.accent: Runtime.colors.accent.key
    Material.primary: Runtime.colors.primary.key

    implicitWidth: suggestedWidth
    implicitHeight: suggestedHeight

    flat: true
    opacity: enabled ? 1 : 0.5
    display: AbstractButton.TextBesideIcon

    hoverEnabled: true
    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    contentItem: Rectangle {
        color: Runtime.colors.primary.c10.background
        border.width: root.flat ? 0 : 1
        border.color: Runtime.colors.primary.borderColor

        Row {
            anchors.centerIn: parent
            spacing: 10

            Image {
                anchors.verticalCenter: parent.verticalCenter

                width: root.icon.width
                height: root.icon.height

                source: root.icon.source
                smooth: true
                mipmap: true
                visible: status === Image.Ready
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter

                spacing: parent.spacing/2

                VclText {
                    text: root.action.text
                    visible: root.display !== AbstractButton.IconOnly

                    font.bold: root.down
                    font.pixelSize: root.font.pixelSize

                    Behavior on color {
                        enabled: Runtime.applicationSettings.enableAnimations
                        ColorAnimation { duration: 250 }
                    }
                }

                VclText {
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: "[" + root.shortcutText + "]"
                    visible: root.shortcutText !== ""
                    font.pixelSize: 9
                }
            }
        }
    }

    action: Action {
        text: root.text
        shortcut: root.shortcut
    }

    ToolTipPopup {
        container: root
        text: root.toolTipText
        visible: text !== "" && root.toolTipVisible
    }
}
