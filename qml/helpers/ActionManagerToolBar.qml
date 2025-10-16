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
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"

ToolBar {
    id: root

    required property ActionManager actionManager

    property int display: AbstractButton.IconOnly
    property bool flat: true

    implicitWidth: _layout.width
    implicitHeight: _layout.height

    RowLayout {
        id: _layout

        Repeater {
            model: root.actionManager

            ToolButton {
                required property int index
                required property var qmlAction

                Material.accent: Runtime.colors.accent.key
                Material.primary: Runtime.colors.primary.key
                Material.theme: Runtime.colors.theme

                ToolTip.text: qmlAction.tooltip ? qmlAction.tooltip : ""

                background: Rectangle {
                    color: Runtime.colors.primary.c10.background
                    border.width: root.flat ? 0 : 1
                    border.color: Runtime.colors.primary.borderColor
                }

                flat: root.flat
                action: qmlAction
                display: root.display

                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }
        }
    }
}
