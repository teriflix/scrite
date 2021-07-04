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

import Scrite 1.0

Item {
    id: tabView
    property var tabNames: ["Default"]
    property var tabColor: primaryColors.windowColor
    property alias currentTabIndex: tabBar.currentIndex
    property alias currentTabContent: tabContentLoader.sourceComponent
    property alias tabBarVisible: tabBar.visible
    property alias cornerItem: cornerLoader.sourceComponent

    Row {
        id: tabBar
        anchors.left: parent.left
        anchors.leftMargin: 20
        spacing: -height*0.4
        property int currentIndex: 0

        Repeater {
            id: tabRepeater
            model: tabNames

            TabBarTab {
                tabFillColor: active ? tabColor : Qt.tint(tabColor, "#C0FFFFFF")
                tabBorderColor: tabColor
                tabBorderWidth: 1
                text: modelData
                tabIndex: index
                tabCount: 2
                textColor: active ? app.textColorFor(tabColor) : "black"
                font.pointSize: app.idealFontPointSize
                font.bold: active
                currentTabIndex: tabBar.currentIndex
                onRequestActivation: tabBar.currentIndex = index
            }
        }
    }

    ItemPositionMapper {
        id: lastTabItemMapper
        from: tabRepeater.count > 0 ? tabRepeater.itemAt( tabRepeater.count-1 ) : null
        to: tabView
    }

    Loader {
        id: cornerLoader
        x: lastTabItemMapper.from ? lastTabItemMapper.mappedPosition.x + lastTabItemMapper.from.width : 0
        y: 0
        width: tabBar.width - x
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
            border.color: tabBar.visible ? tabColor : primaryColors.borderColor
            color: Qt.rgba(0,0,0,0)
        }
    }
}

