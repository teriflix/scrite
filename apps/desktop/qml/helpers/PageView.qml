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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

Rectangle {
    id: root

    property alias pageListVisible: _pageList.visible
    property alias pagesArray: _pageRepeater.model
    property alias currentIndex: _pageList.currentIndex
    property string pageTitleRole
    property alias cornerContent: _cornerContentLoader.sourceComponent
    property alias cornerContentItem: _cornerContentLoader.item
    property alias pageContent: _pageContentLoader.sourceComponent
    property alias pageContentItem: _pageContentLoader.item
    property int pageContentSpacing: 20
    property real maxPageListWidth: 220
    property real pageListWidth: Math.max(width * 0.2, maxPageListWidth)
    property real availablePageContentWidth: _pageContentLoader.width
    property real availablePageContentHeight: _pageContentArea.height

    color: Runtime.colors.primary.c50.background

    RowLayout {
        anchors.fill: parent

        spacing: root.pageContentSpacing

        Rectangle {
            id: _pageList

            Layout.fillHeight: true
            Layout.minimumWidth: pageListWidth
            Layout.maximumWidth: pageListWidth
            Layout.preferredWidth: pageListWidth

            property int currentIndex: -1

            color: Runtime.colors.accent.c600.background

            Action {
                shortcut: ActionHub.applicationOptions.find("tabDown").shortcut

                onTriggered: (source) => {
                                 _pageList.currentIndex = (_pageList.currentIndex+1)%_pageRepeater.count
                             }
            }

            Action {
                shortcut: ActionHub.applicationOptions.find("tabUp").shortcut

                onTriggered: (source) => {
                                 const prevIndex = _pageList.currentIndex-1
                                 _pageList.currentIndex = prevIndex < 0 ? (_pageRepeater.count-1) : prevIndex
                             }
            }

            ColumnLayout {
                id: _pageRepeaterLayout

                anchors.fill: parent
                anchors.topMargin: 20

                Repeater {
                    id: _pageRepeater

                    delegate: Item {
                        required property int index
                        required property var modelData

                        Layout.fillWidth: true
                        Layout.preferredHeight: _pageLabel.height*1.25

                        Rectangle {
                            anchors.fill: parent

                            color: Runtime.colors.primary.c100.background
                            opacity: 0.8
                            visible: _pageList.currentIndex === index

                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                        }

                        VclLabel {
                            id: _pageLabel

                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: 5
                            anchors.rightMargin: 24
                            anchors.verticalCenter: parent.verticalCenter

                            text: pageTitleRole === "" ? modelData : modelData[pageTitleRole]
                            color: _pageList.currentIndex === index ? Runtime.colors.primary.c50.text : Runtime.colors.accent.c600.text
                            elide: Text.ElideMiddle
                            horizontalAlignment: Text.AlignRight
                            topPadding: 6
                            bottomPadding: 6

                            font.bold: _pageList.currentIndex === index
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        }

                        Image {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter

                            width: 24
                            height: 24

                            source: "qrc:/icons/navigation/arrow_right.png"
                            visible: _pageList.currentIndex === index
                        }

                        MouseArea {
                            anchors.fill: parent

                            onClicked: _pageList.currentIndex = index
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                Loader {
                    id: _cornerContentLoader

                    Layout.fillWidth: true
                }
            }
        }

        Flickable {
            id: _pageContentArea

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: pageListVisible ? 0 : 20

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            clip: ScrollBar.vertical.needed
            contentWidth: _pageContentLoader.active && ScrollBar.vertical.needed ? (width-20) : width
            contentHeight: _pageContentLoader.height

            ScrollBar.vertical: VclScrollBar { flickable: _pageContentArea }

            Loader {
                id: _pageContentLoader

                width: _pageContentArea.contentWidth

                onItemChanged: Object.resetProperty(_pageContentLoader, "height")
            }
        }
    }
}
