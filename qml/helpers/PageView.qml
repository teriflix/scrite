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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Rectangle {
    id: root

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

    RowLayout {
        anchors.fill: parent

        spacing: root.pageContentSpacing

        Rectangle {
            id: pageList

            Layout.fillHeight: true
            Layout.minimumWidth: pageListWidth
            Layout.maximumWidth: pageListWidth
            Layout.preferredWidth: pageListWidth

            property int currentIndex: -1

            color: Runtime.colors.accent.c600.background

            ColumnLayout {
                id: pageRepeaterLayout

                anchors.fill: parent
                anchors.topMargin: 20

                Repeater {
                    id: pageRepeater

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60

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

                Item {
                    Layout.fillHeight: true
                }

                Loader {
                    id: cornerContentLoader

                    Layout.fillWidth: true
                }
            }
        }

        Flickable {
            id: pageContentArea

            Layout.fillWidth: true
            Layout.fillHeight: true

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            clip: ScrollBar.vertical.needed
            contentWidth: pageContentLoader.active && ScrollBar.vertical.needed ? (width-20) : width
            contentHeight: pageContentLoader.height

            ScrollBar.vertical: VclScrollBar { flickable: pageContentArea }

            Loader {
                id: pageContentLoader
                width: pageContentArea.contentWidth
            }
        }
    }
}
