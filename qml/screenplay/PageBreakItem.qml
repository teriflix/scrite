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
import Qt.labs.qmlmodels 1.0
import QtQuick.Shapes 1.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Rectangle {
    id: root

    readonly property color defaultForeground: Runtime.colors.primary.c600.background
    readonly property color defaultColor: fullSize ? Runtime.colors.primary.c100.background : Runtime.colors.primary.c50.background

    property bool fullSize: true
    property int placement: Qt.TopEdge // or Qt.BottomEdge
    property color foreground: defaultForeground
    property real textFontSize: Math.max(Runtime.sceneEditorFontMetrics.font.pointSize*0.7, 6)

    implicitWidth: 100
    implicitHeight: loader.height * (fullSize ? 1.2 : 1.1)

    color: defaultColor

    Loader {
        id: loader

        y: placement === Qt.TopEdge ? (root.height-height) : 0
        width: parent.width

        sourceComponent: root.fullSize ? fullSizeVariant : miniSizeVariant
    }

    Component {
        id: fullSizeVariant

        RowLayout {
            DashedLine {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
            }

            VclLabel {
                text: "Page Break"
                font.pointSize: root.textFontSize
                padding: root.fullSize ? 5 : 2
                color: root.foreground
            }

            DashedLine {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

    Component {
        id: miniSizeVariant

        DashedLine {
            height: 8
        }
    }

    component DashedLine : Shape {
        id: shape

        implicitHeight: 2

        ShapePath {
            strokeColor: root.foreground
            strokeWidth: 1
            strokeStyle: ShapePath.DashLine
            dashPattern: [3,5]
            startX: 0; startY: shape.height/2
            PathLine { x: shape.width; y: shape.height/2 }
        }
    }
}
