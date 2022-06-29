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

import io.scrite.components 1.0

Rectangle {
    id: pageView
    property alias pagesArray: pageRepeater.model
    property alias currentIndex: pageList.currentIndex
    property string pageTitleRole
    property alias cornerContent: cornerContentLoader.sourceComponent
    property alias cornerContentItem: cornerContentLoader.item
    property alias pageContent: pageContentLoader.sourceComponent
    property alias pageContentItem: pageContentLoader.item
    property int pageContentSpacing: 20
    property real maxPageListWidth: 220
    property real pageListWidth: Math.max(width * 0.2, maxPageListWidth)

    Rectangle {
        id: pageList
        width: pageListWidth
        color: primaryColors.c700.background
        height: parent.height
        anchors.left: parent.left
        property int currentIndex: -1

        Column {
            id: pageRepeaterLayout
            width: parent.width
            anchors.top: parent.top
            anchors.right: parent.right

            Repeater {
                id: pageRepeater

                Rectangle {
                    width: parent.width
                    height: 60
                    color: pageList.currentIndex === index ? pageView.color : primaryColors.c10.background

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 40
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 18
                        font.bold: pageList.currentIndex === index
                        text: pageTitleRole === "" ? modelData : modelData[pageTitleRole]
                        color: pageList.currentIndex === index ? "black" : primaryColors.c700.text
                    }

                    Image {
                        width: 24; height: 24
                        source: "../icons/navigation/arrow_right.png"
                        visible: pageList.currentIndex === index
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: pageList.currentIndex = index
                    }
                }
            }
        }

        Loader {
            id: cornerContentLoader
            anchors.top: pageRepeaterLayout.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
        }
    }

    Flickable {
        id: pageContentArea
        anchors.left: pageList.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: pageContentSpacing
        property bool showScrollBars: contentHeight > height
        contentWidth: pageContentLoader.width
        contentHeight: pageContentLoader.height
        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

        ScrollBar.vertical: ScrollBar2 { flickable: pageContentArea }
        Loader {
            id: pageContentLoader
            width: pageContentArea.showScrollBars ? pageContentArea.width-17 : pageContentArea.width
        }
    }
}
