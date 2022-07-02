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

import io.scrite.components 1.0

Item {
    id: tabView
    property var tabNames: ["Default"]
    property color tabColor: primaryColors.windowColor
    property alias currentTabIndex: tabBar.currentIndex
    property alias currentTabContent: tabContentLoader.sourceComponent
    property alias tabBarVisible: tabBar.visible
    property alias cornerItem: cornerLoader.sourceComponent
    property real cornerItemSpace: cornerLoader.width
    property alias currentTabItem: tabContentLoader.item

    Row {
        id: tabBar
        anchors.left: parent.left
        anchors.leftMargin: 20
        spacing: -height*0.4
        property int currentIndex: 0

        Repeater {
            id: tabRepeater
            model: tabBar.visible ? tabNames : 0

            TabBarTab {
                tabFillColor: active ? tabColor : Qt.tint(tabColor, "#C0FFFFFF")
                tabBorderColor: Scrite.app.isVeryLightColor(tabColor) ? "gray" : tabColor
                tabBorderWidth: 1
                text: modelData
                tabIndex: index
                tabCount: 2
                textColor: active ? Scrite.app.textColorFor(tabColor) : "black"
                font.pointSize: Scrite.app.idealFontPointSize
                font.bold: active
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
        color: Qt.rgba(1,1,1,0.25)
        border.width: 0

        Loader {
            id: tabContentLoader
            anchors.fill: parent
        }

        Rectangle {
            anchors.fill: tabContentLoader
            border.width: 1
            border.color: Scrite.app.isVeryLightColor(tabColor) ? primaryColors.windowColor : tabColor
            color: Qt.rgba(0,0,0,0)
        }
    }
}

