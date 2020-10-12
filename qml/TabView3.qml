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

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

Item {
    property var tabNames: ["Default"]
    property var tabColor: primaryColors.windowColor
    property alias currentTabIndex: tabBar.currentIndex
    property alias currentTabContent: tabContentLoader.sourceComponent
    property alias tabBarVisible: tabBar.visible

    Row {
        id: tabBar
        anchors.left: parent.left
        anchors.leftMargin: 20
        spacing: -height*0.4
        property int currentIndex: 0

        Repeater {
            model: tabNames

            TabBarTab {
                tabFillColor: active ? tabColor : Qt.tint(tabColor, "#C0FFFFFF")
                tabBorderColor: tabColor
                tabBorderWidth: 1
                text: modelData
                tabIndex: index
                tabCount: 2
                textColor: active ? app.textColorFor(tabColor) : "black"
                font.pixelSize: active ? 20 : 16
                font.bold: active
                currentTabIndex: tabBar.currentIndex
                onRequestActivation: tabBar.currentIndex = index
            }
        }
    }

    Rectangle {
        anchors.top: tabBar.visible ? tabBar.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Qt.rgba(1,1,1,0.25)
        border.width: tabBar.visible ? 1 : 0
        border.color: tabColor
        radius: 6

        Loader {
            id: tabContentLoader
            anchors.fill: parent
            anchors.margins: 2
        }
    }
}

