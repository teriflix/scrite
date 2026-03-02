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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import io.scrite.components

import "qrc:/qml/globals"

ToolBar {
    id: root

    required property ActionManager actionManager

    property int display: ToolButton.IconOnly
    property bool flat: true

    Material.accent: Runtime.colors.accent.key
    Material.background: Runtime.colors.primary.c10.background
    Material.elevation: 0
    Material.primary: Runtime.colors.primary.key
    Material.theme: Runtime.colors.theme

    implicitWidth: _layout.width
    implicitHeight: _layout.height

    focusPolicy: Qt.NoFocus

    RowLayout {
        id: _layout

        spacing: 0

        Repeater {
            model: root.actionManager.visibleActions

            delegate: ActionToolButton {
                required property int index
                required property var modelData

                property var qmlAction: modelData

                flat: root.flat
                action: qmlAction
            }
        }
    }
}
