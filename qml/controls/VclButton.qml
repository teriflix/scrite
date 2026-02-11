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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"

Button {
    id: root

    property bool toolTipVisible: hovered
    property string toolTipText

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    Component.onCompleted: {
        if(!Scrite.app.usingMaterialTheme) {
            background = backgroundComponent.createObject(root)
            font.pointSize = Runtime.idealFontMetrics.font.pointSize
        }
    }

    font.pointSize: Runtime.idealFontMetrics.font.pointSize

    implicitWidth: Math.max(_private.textRect.width + 40, 120)
    implicitHeight: Math.max(_private.textRect.height + 20, 50)

    Component {
        id: backgroundComponent

        Rectangle {
            implicitWidth: 120
            implicitHeight: 30
            color: root.down ? border.color : Runtime.colors.primary.button.background
            border.width: 1
            border.color: Qt.darker(Runtime.colors.primary.button.background,1.25)
        }
    }

    ToolTipPopup {
        container: root
        text: root.toolTipText
        visible: text !== "" && root.toolTipVisible
    }

    QtObject {
        id: _private

        property rect textRect: GMath.boundingRect(root.text, root.font)
    }
}
