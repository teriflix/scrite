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

import io.scrite.components 1.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

Item {
    id: tabBarTab
    width: implicitTabSize
    height: Scrite.app.idealFontPointSize + 16

    readonly property real tabTextWidth: tabText.width
    readonly property real implicitTabSize: tabTextWidth*1.1 + 2*tabShapeOffset
    property alias text: tabText.text
    property alias textColor: tabText.color
    property alias font: tabText.font
    property alias tabFillColor: tabShapeItem.fillColor
    property alias tabBorderColor: tabShapeItem.outlineColor
    property alias tabBorderWidth: tabShapeItem.outlineWidth
    property alias containsMouse: tabMouseArea.containsMouse
    property alias acceptedMouseButtons: tabMouseArea.acceptedButtons

    property bool hoverEnabled: false
    property int tabCount: 1
    property int tabIndex: 0
    property int currentTabIndex: -1
    property bool active: currentTabIndex === tabIndex
    z: active ? tabCount+1 : (currentTabIndex < tabIndex ? tabCount-tabIndex-1 : tabIndex)

    property int alignment: Qt.AlignTop

    signal requestActivation()
    signal requestContextMenu()

    PainterPathItem {
        id: tabShapeItem
        anchors.fill: parent
        anchors.topMargin: active ? 0 : parent.height*0.1
        fillColor: active ? primaryColors.windowColor : primaryColors.c200.background
        outlineColor: primaryColors.borderColor
        outlineWidth: 2
        renderingMechanism: PainterPathItem.UseAntialiasedQPainter
        painterPath: tabBarTab.alignment === Qt.AlignRight ? rightPainterPath.createObject(tabShapeItem) : topPainterPath.createObject(tabShapeItem)

        Text {
            id: tabText
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: tabBarTab.alignment === Qt.AlignRight ? -tabText.height*0.1 : 0
            font.pointSize: Scrite.app.idealFontPointSize
            font.bold: tabBarTab.active
            rotation: tabBarTab.alignment === Qt.AlignRight ? 90 : 0
            Behavior on font.pixelSize {
                enabled: applicationSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }
            Behavior on color {
                enabled: applicationSettings.enableAnimations
                ColorAnimation { duration: 125 }
            }
        }

        Rectangle {
            readonly property real margin: tabShapeOffset*0.15
            readonly property real size: parent.outlineWidth
            x: margin
            y: tabBarTab.alignment === Qt.AlignTop ? parent.height - height : margin
            width: tabBarTab.alignment === Qt.AlignTop ? parent.width-2*margin : size
            height: tabBarTab.alignment === Qt.AlignTop ? size : parent.height-2*margin
            color: parent.fillColor
            visible: size > 0 && tabBarTab.active
        }
    }

    MouseArea {
        id: tabMouseArea
        hoverEnabled: parent.hoverEnabled
        anchors.fill: parent
        onClicked: {
            if(mouse.button === Qt.RightButton)
                tabBarTab.requestContextMenu()
            else
                tabBarTab.requestActivation()
        }
    }

    readonly property real tabShapeRadius: 8
    readonly property real tabShapeOffset: 18

    Component {
        id: topPainterPath

        PainterPath {
            id: tabPath
            readonly property real radius: tabShapeRadius // Math.min(itemRect.width, itemRect.height)*0.2
            readonly property real offset: tabShapeOffset // itemRect.width*0.1

            property point c1: Qt.point(itemRect.left+offset, itemRect.top+1)
            property point c2: Qt.point(itemRect.right-1-offset, itemRect.top+1)

            property point p1: Qt.point(itemRect.left, itemRect.bottom)
            property point p2: pointInLine(c1, p1, radius, true)
            property point p3: pointInLine(c1, c2, radius, true)
            property point p4: pointInLine(c2, c1, radius, true)
            property point p5: pointInLine(c2, p6, radius, true)
            property point p6: Qt.point(itemRect.right-1, itemRect.bottom)

            MoveTo { x: tabPath.p1.x; y: tabPath.p1.y }
            LineTo { x: tabPath.p2.x; y: tabPath.p2.y }
            QuadTo { controlPoint: tabPath.c1; endPoint: tabPath.p3 }
            LineTo { x: tabPath.p4.x; y: tabPath.p4.y }
            QuadTo { controlPoint: tabPath.c2; endPoint: tabPath.p5 }
            LineTo { x: tabPath.p6.x; y: tabPath.p6.y }
            CloseSubpath { }
        }
    }

    Component {
        id: rightPainterPath

        PainterPath {
            id: tabPath
            readonly property real radius: tabShapeRadius // Math.min(itemRect.width, itemRect.height)*0.2
            readonly property real offset: tabShapeOffset // itemRect.height*0.1

            property point c1: Qt.point(itemRect.right-1, itemRect.top+offset)
            property point c2: Qt.point(itemRect.right-1, itemRect.bottom-1-offset)

            property point p1: Qt.point(itemRect.left, itemRect.top)
            property point p2: pointInLine(c1, p1, radius, true)
            property point p3: pointInLine(c1, c2, radius, true)
            property point p4: pointInLine(c2, c1, radius, true)
            property point p5: pointInLine(c2, p6, radius, true)
            property point p6: Qt.point(itemRect.left, itemRect.bottom)

            MoveTo { x: tabPath.p1.x; y: tabPath.p1.y }
            LineTo { x: tabPath.p2.x; y: tabPath.p2.y }
            QuadTo { controlPoint: tabPath.c1; endPoint: tabPath.p3 }
            LineTo { x: tabPath.p4.x; y: tabPath.p4.y }
            QuadTo { controlPoint: tabPath.c2; endPoint: tabPath.p5 }
            LineTo { x: tabPath.p6.x; y: tabPath.p6.y }
            CloseSubpath { }
        }
    }
}
