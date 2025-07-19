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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

StructureElementConnector {
    id: root

    required property rect canvasScrollViewportRect

    required property real canvasScale

    required property string labelText

    property alias labelVisible: _labelBg.visible

    visible: {
        if(canBeVisible)
            return intersects(root.canvasScrollViewportRect)
        return false
    }
    lineType: StructureElementConnector.CurvedLine
    outlineWidth: Scrite.app.devicePixelRatio * root.canvasScale * Runtime.structureCanvasSettings.lineWidthOfConnectors
    arrowAndLabelSpacing: _labelBg.width

    Rectangle {
        id: _labelBg

        x: root.suggestedLabelPosition.x - radius
        y: root.suggestedLabelPosition.y - radius

        width: Math.max(_labelItem.width,_labelItem.height)+20
        height: width
        radius: width/2

        color: Qt.tint(root.outlineColor, "#E0FFFFFF")
        border.width: 1
        border.color: Runtime.colors.primary.borderColor

        VclText {
            id: _labelItem

            anchors.centerIn: parent

            text: root.labelText
            font.pixelSize: 12
        }
    }
}
