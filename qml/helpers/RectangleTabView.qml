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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    property alias tabsArray: tabRepeater.model
    property alias content: contentLoader.sourceComponent
    property alias contentItem: contentLoader.item
    property alias currentIndex: tabBar.currentIndex
    property string tabTitleRole: "title"
    property string tabTooltipRole: "tooltip"

    Row {
        id: tabBar

        property int currentIndex: 0

        anchors.top: parent.top
        anchors.left: parent.left

        Repeater {
            id: tabRepeater

            model: 0

            delegate: Rectangle {
                required property int index
                required property var modelData

                property bool selected: tabBar.currentIndex === index

                width: tabText.contentWidth + 40
                height: tabText.contentHeight + 30
                color: selected ? "white" : Qt.rgba(0,0,0,0)

                Rectangle {
                    height: 4
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    color: Runtime.colors.accent.c500.background
                    visible: parent.selected
                }

                VclLabel {
                    id: tabText
                    anchors.centerIn: parent
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    text: tabTitleRole === "" ? modelData : modelData[tabTitleRole]
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: tabBar.currentIndex = index
                    hoverEnabled: true

                    ToolTipPopup {
                        text: tabTooltipRole === "" ? modelData : modelData[tabTooltipRole]
                        visible: container.containsMouse
                    }
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
