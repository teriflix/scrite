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
import "qrc:/qml/controls"

Item {
    id: tabView

    property var tabNames: ["Default"]
    property real cornerItemSpace: cornerLoader.width
    property color tabColor: Runtime.colors.primary.windowColor
    property alias currentTabIndex: tabBar.currentIndex
    property alias currentTabContent: tabContentLoader.sourceComponent
    property alias tabBarVisible: tabBar.visible
    property alias cornerItem: cornerLoader.sourceComponent
    property alias currentTabItem: tabContentLoader.item
    property alias tabContentBorderVisible: tabContentLoaderBorder.visible

    Row {
        id: tabBar
        anchors.left: parent.left
        anchors.leftMargin: 20
        spacing: -height*0.4
        property int currentIndex: 0

        Repeater {
            id: tabRepeater

            model: tabBar.visible ? tabNames : 0

            delegate: TrapeziumTab {
                required property int index
                required property string modelData

                tabFillColor: active ? tabColor : Runtime.colors.tint(tabColor, Runtime.colors.sceneControlTint)
                tabBorderColor: Color.isVeryLight(tabColor) ? Runtime.colors.primary.borderColor : tabColor
                tabBorderWidth: 1
                text: modelData
                tabIndex: index
                tabCount: 2
                textColor: active ? Color.textColorFor(tabColor) : Runtime.colors.primary.regular.text
                currentTabIndex: tabBar.currentIndex
                onRequestActivation: tabBar.currentIndex = index
            }
        }
    }

    Loader {
        id: cornerLoader
        anchors.left: tabBar.right
        anchors.right: parent.right
        height: tabBar.height
        active: tabBar.visible
        visible: active
    }

    Rectangle {
        anchors.top: tabBar.visible ? tabBar.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: tabContentLoaderBorder.visible ? Qt.rgba(1,1,1,0.25) : Qt.rgba(0,0,0,0)
        border.width: 0

        Loader {
            id: tabContentLoader
            anchors.fill: parent
        }

        Rectangle {
            id: tabContentLoaderBorder

            anchors.fill: tabContentLoader

            color: Qt.rgba(0,0,0,0)

            border.width: 1
            border.color: Color.isVeryLight(tabColor) ? Runtime.colors.primary.windowColor : tabColor
        }
    }

    // Refactor aliases in this...
    component TrapeziumTab : Item {
        id: tabBarTab

        width: implicitTabSize
        height: Runtime.idealFontMetrics.font.pointSize + 16

        readonly property real tabTextWidth: tabText.width
        readonly property real implicitTabSize: tabTextWidth*1.1 + 2*_private.tabShapeOffset
        property string text
        property color textColor: Runtime.colors.primary.regular.text
        property color tabFillColor
        property color tabBorderColor
        property real tabBorderWidth

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

            fillColor: tabBarTab.tabFillColor
            outlineColor: tabBarTab.tabBorderColor
            outlineWidth: tabBarTab.tabBorderWidth
            renderingMechanism: PainterPathItem.UseAntialiasedQPainter
            painterPath: tabBarTab.alignment === Qt.AlignRight ? rightPainterPath.createObject(tabShapeItem) : topPainterPath.createObject(tabShapeItem)

            VclLabel {
                id: tabText

                anchors.centerIn: parent
                anchors.horizontalCenterOffset: tabBarTab.alignment === Qt.AlignRight ? -tabText.height*0.1 : 0

                text: tabBarTab.text
                color: tabBarTab.textColor

                font.bold: tabBarTab.active
                font.pointSize: Runtime.idealFontMetrics.font.pointSize

                rotation: tabBarTab.alignment === Qt.AlignRight ? 90 : 0
                Behavior on font.pixelSize {
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: Runtime.stdAnimationDuration }
                }
                Behavior on color {
                    enabled: Runtime.applicationSettings.enableAnimations
                    ColorAnimation { duration: 125 }
                }
            }

            Rectangle {
                readonly property real margin: _private.tabShapeOffset*0.15
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
    }

    Component {
        id: topPainterPath

        PainterPath {
            id: tabPath
            readonly property real radius: _private.tabShapeRadius // Math.min(itemRect.width, itemRect.height)*0.2
            readonly property real offset: _private.tabShapeOffset // itemRect.width*0.1

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
            readonly property real radius: _private.tabShapeRadius // Math.min(itemRect.width, itemRect.height)*0.2
            readonly property real offset: _private.tabShapeOffset // itemRect.height*0.1

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

    QtObject {
        id: _private

        readonly property real tabShapeRadius: 8
        readonly property real tabShapeOffset: 18
    }
}

