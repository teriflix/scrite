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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Rectangle {
    id: pageView

    property alias pageListVisible: pageList.visible
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
    property real availablePageContentWidth: pageContentLoader.width
    property real availablePageContentHeight: pageContentArea.height

    color: Runtime.colors.primary.c50.background

    Rectangle {
        id: pageList

        property int currentIndex: -1

        width: pageListWidth
        height: parent.height
        anchors.left: parent.left

        color: Runtime.colors.accent.c600.background

        Column {
            id: pageRepeaterLayout

            width: parent.width
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 20

            clip: true

            Repeater {
                id: pageRepeater

                Item {
                    width: parent.width
                    height: 60

                    Rectangle {
                        width: parent.width
                        height: parent.height - 15

                        color: Runtime.colors.primary.c100.background
                        opacity: 0.8
                        visible: pageList.currentIndex === index

                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                    }

                    VclLabel {
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        anchors.right: parent.right
                        anchors.rightMargin: 24
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        font.bold: pageList.currentIndex === index
                        text: pageTitleRole === "" ? modelData : modelData[pageTitleRole]
                        color: pageList.currentIndex === index ? Runtime.colors.primary.c50.text : Runtime.colors.accent.c600.text
                        elide: Text.ElideMiddle
                        horizontalAlignment: Text.AlignRight
                    }

                    Image {
                        width: 24; height: 24
                        source: "qrc:/icons/navigation/arrow_right.png"
                        visible: pageList.currentIndex === index
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
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
        anchors.left: pageList.visible ? pageList.right : parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: pageContentSpacing
        property bool showScrollBars: contentHeight > height
        contentWidth: pageContentLoader.width
        contentHeight: pageContentLoader.height
        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
        clip: showScrollBars

        ScrollBar.vertical: VclScrollBar { flickable: pageContentArea }

        Loader {
            id: pageContentLoader
            width: pageContentArea.showScrollBars ? pageContentArea.width-17 : pageContentArea.width
        }
    }
}
