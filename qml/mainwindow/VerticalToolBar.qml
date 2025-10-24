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

import "qrc:/qml/"
import "qrc:/qml/globals"
import "qrc:/qml/helpers"

ToolBar {
    id: root

    required property ActionManager actions

    Material.accent: Runtime.colors.accent.key
    Material.background: Runtime.colors.primary.c10.background
    Material.elevation: 0
    Material.primary: Runtime.colors.primary.key
    Material.theme: Runtime.colors.theme

    GridLayout {
        id: _layout

        readonly property size buttonSize: Runtime.estimateTypeSize("ToolButton { icon.source: \"qrc:/icons/content/blank.png\"; display: ToolButton.IconOnly; }")
        property int buttonCount: (actions ? actions.count : 0) + 1

        anchors.fill: parent

        flow: Flow.TopToBottom
        rows: Math.floor(_col1.height/buttonSize.height)
        columns: Math.ceil( (buttonCount * buttonSize.height)/_col1.height )
        rowSpacing: 0
        columnSpacing: 0

        Repeater {
            model: root.actions

            ActionToolButton {
                required property var qmlAction

                action: qmlAction
            }
        }

        Item {
            Layout.row: _layout.rows-1
            Layout.column: _layout.columns-1
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: _layout.buttonSize.width
            Layout.minimumHeight: _layout.buttonSize.height
        }
    }
}
