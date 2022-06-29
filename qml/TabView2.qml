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

Item {
    property alias tabsArray: tabRepeater.model
    property alias content: contentLoader.sourceComponent
    property alias contentItem: contentLoader.item
    property alias currentIndex: tabBar.currentIndex
    property string tabTitleRole: "title"
    property string tabTooltipRole: "tooltip"

    Row {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        property int currentIndex: 0

        Repeater {
            id: tabRepeater
            model: tabBar.tabs

            Rectangle {
                width: tabText.contentWidth + 40
                height: tabText.contentHeight + 30
                color: selected ? "white" : Qt.rgba(0,0,0,0)
                property bool selected: tabBar.currentIndex === index

                Rectangle {
                    height: 4
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    color: accentColors.c500.background
                    visible: parent.selected
                }

                Text {
                    id: tabText
                    anchors.centerIn: parent
                    font.pixelSize: 16
                    text: tabTitleRole === "" ? modelData : modelData[tabTitleRole]
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: tabBar.currentIndex = index
                    hoverEnabled: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: tabTooltipRole === "" ? modelData : modelData[tabTooltipRole]
                }
            }
        }
    }

    Rectangle {
        id: contentPanel
        anchors.top: tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true

        Loader {
            id: contentLoader
            anchors.fill: parent
        }
    }
}
