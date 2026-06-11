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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../controls"

Rectangle {
    id: root

    property alias cornerContent: _cornerContentLoader.sourceComponent
    property alias cornerContentItem: _cornerContentLoader.item
    property alias currentIndex: _pageList.currentIndex
    property alias pageContent: _pageContentLoader.sourceComponent
    property alias pageContentItem: _pageContentLoader.item
    property alias pageListVisible: _pageList.visible
    property alias pagesArray: _pageRepeater.model

    property int pageContentSpacing: 20

    property real availablePageContentHeight: _pageContentArea.height
    property real availablePageContentWidth: _pageContentLoader.width
    property real maxPageListWidth: 220
    property real pageListWidth: Math.max(width * 0.2, maxPageListWidth)

    property bool highlightPageWithCounter: pageCounterRole !== ""
    property string pageTitleRole
    property string pageCounterRole // Can be used to show count of unread messages for example

    color: Runtime.colors.primary.c50.background

    function indexOfPage(pageTitle) {
        for(var i=0; i<_pageRepeater.count; i++)
            if(_pageRepeater.itemAt(i).pageTitle === pageTitle)
                return i
        return -1
    }

    RowLayout {
        anchors.fill: parent

        spacing: root.pageContentSpacing

        Rectangle {
            id: _pageList

            Layout.fillHeight: true
            Layout.minimumWidth: root.pageListWidth
            Layout.maximumWidth: root.pageListWidth
            Layout.preferredWidth: root.pageListWidth

            property int currentIndex: -1

            color: Runtime.colors.accent.c600.background

            Action {
                shortcut: ActionHub.applicationOptions.find("tabDown").shortcut

                onTriggered: () => {
                                 _pageList.currentIndex = (_pageList.currentIndex+1)%_pageRepeater.count
                             }
            }

            Action {
                shortcut: ActionHub.applicationOptions.find("tabUp").shortcut

                onTriggered: () => {
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
                        id: _pageRepeaterDelegate

                        required property int index
                        required property var modelData

                        property int pageCounter: root.pageCounterRole === "" ? -1 : (_pageRepeaterDelegate.modelData[root.pageCounterRole] !== undefined ?
                                                                                          parseInt(_pageRepeaterDelegate.modelData[root.pageCounterRole]) : -1)
                        property string pageTitle: root.pageTitleRole === "" ? _pageRepeaterDelegate.modelData : _pageRepeaterDelegate.modelData[root.pageTitleRole]

                        Layout.fillWidth: true
                        Layout.preferredHeight: _pageLabel.height*1.25

                        Rectangle {
                            anchors.fill: parent

                            color: Runtime.colors.primary.c100.background
                            opacity: 0.8
                            visible: _pageList.currentIndex === _pageRepeaterDelegate.index

                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                        }

                        RowLayout {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: 5
                            anchors.rightMargin: 24
                            anchors.verticalCenter: parent.verticalCenter

                            Item {
                                Layout.fillWidth: true

                                Rectangle {
                                    id: _counterCircle

                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.right: parent.right

                                    width: Math.max(_pageCounter.width,_pageCounter.height)
                                    height: width
                                    radius: width/2

                                    color: _pageList.currentIndex === _pageRepeaterDelegate.index ? Runtime.colors.primary.c50.text : Runtime.colors.accent.c600.text
                                    visible: _pageRepeaterDelegate.pageCounter > 0

                                    VclLabel {
                                        id: _pageCounter

                                        anchors.centerIn: parent

                                        text: _pageRepeaterDelegate.pageCounter
                                        color: Color.textColorFor(parent.color)
                                        padding: 4
                                    }

                                    SequentialAnimation {
                                        id: _counterHighlightAnimation

                                        loops: 2

                                        NumberAnimation { target: _counterCircle; property: "scale"; from: 1.0; to: 1.5; duration: 400; easing.type: Easing.OutQuad }
                                        NumberAnimation { target: _counterCircle; property: "scale"; from: 1.5; to: 1.0; duration: 600; easing.type: Easing.InQuad }
                                    }

                                    Component.onCompleted: {
                                        if(_pageRepeaterDelegate.pageCounter > 0 && root.highlightPageWithCounter)
                                            _counterHighlightAnimation.start()
                                    }

                                    Connections {
                                        target: Qt.application
                                        function onStateChanged() {
                                            if(Qt.application.state === Qt.ApplicationActive &&
                                                    _pageRepeaterDelegate.pageCounter > 0 && root.highlightPageWithCounter)
                                                _counterHighlightAnimation.restart()
                                        }
                                    }
                                }
                            }

                            VclLabel {
                                id: _pageLabel

                                text: _pageRepeaterDelegate.pageTitle
                                color: _pageList.currentIndex === _pageRepeaterDelegate.index ? Runtime.colors.primary.c50.text : Runtime.colors.accent.c600.text
                                elide: Text.ElideMiddle
                                horizontalAlignment: Text.AlignRight
                                topPadding: 6
                                bottomPadding: 6

                                font.bold: _pageList.currentIndex === _pageRepeaterDelegate.index
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }
                        }

                        Image {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter

                            width: 24
                            height: 24

                            source: Runtime.themedIcon("qrc:/icons/navigation/arrow_right.png")
                            visible: _pageList.currentIndex === _pageRepeaterDelegate.index
                        }

                        MouseArea {
                            anchors.fill: parent

                            onClicked: _pageList.currentIndex = _pageRepeaterDelegate.index
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
            Layout.leftMargin: root.pageListVisible ? 0 : 20

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
