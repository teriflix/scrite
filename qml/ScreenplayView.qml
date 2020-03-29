/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
import Scrite 1.0

Rectangle {
    signal requestEditor()

    color: Qt.tint("#8c9cb1", "#40FFFFFF")
    clip: true

    property real zoomLevel: 1
    property color dropAreaHighlightColor: "gray"
    property string dropAreaKey: "scrite/sceneID"

    Connections {
        target: scriteDocument.screenplay
        onCurrentElementIndexChanged: {
            screenplayElementList.positionViewAtIndex(scriteDocument.screenplay.currentElementIndex, ListView.Contain)
        }
    }

    ListView {
        id: screenplayElementList
        anchors.fill: parent
        anchors.margins: 10
        model: scriteDocument.screenplay
        property real idealDelegateWidth: 100
        orientation: Qt.Horizontal
        currentIndex: scriteDocument.screenplay.currentElementIndex
        footer: Item {
            property bool highlightAsDropArea: false
            property real normalWidth: screenplayElementList.idealDelegateWidth * zoomLevel
            property real availableWidth: screenplayElementList.width - screenplayElementList.count*normalWidth
            width: Math.max(normalWidth, availableWidth)
            height: screenplayElementList.height

            Rectangle {
                width: 5
                height: parent.height
                color: parent.highlightAsDropArea ? dropAreaHighlightColor : Qt.rgba(0,0,0,0)
            }

            DropArea {
                anchors.fill: parent
                keys: [dropAreaKey]

                onEntered: parent.highlightAsDropArea = true
                onExited: parent.highlightAsDropArea = false

                onDropped: {
                    parent.highlightAsDropArea = false
                    dropSceneAt(drop.source, -1)
                    drop.acceptProposedAction()
                }
            }
        }

        delegate: Item {
            id: elementItemDelegate
            property ScreenplayElement element: screenplayElement
            property bool active: element ? scriteDocument.screenplay.activeScene === element.scene : false
            width: screenplayElementList.idealDelegateWidth * zoomLevel
            height: screenplayElementList.height

            Loader {
                anchors.fill: parent
                anchors.leftMargin: 7.5
                anchors.rightMargin: 2.5
                active: element !== null && element.scene !== null
                sourceComponent: Rectangle {
                    radius: 8
                    color: Qt.tint(element.scene.color, "#C0FFFFFF")
                    border.color: color === Qt.rgba(1,1,1,1) ? "black" : element.scene.color
                    border.width: elementItemDelegate.active ? 4 : 1
                    Behavior on border.width { NumberAnimation { duration: 400 } }

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: menuButton.bottom
                        anchors.bottom: dragTriggerButton.top
                        anchors.margins: 5
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WrapAnywhere
                        elide: Text.ElideRight
                        font.pixelSize: 15
                        lineHeight: 1.25
                        text: element.scene.title
                        visible: width >= 80
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            scriteDocument.screenplay.currentElementIndex = index
                            requestEditor()
                        }
                    }

                    RoundButton {
                        id: menuButton
                        icon.source: "../icons/navigation/menu.png"
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 5
                        onClicked: {
                            elementItemMenu.element = element
                            elementItemMenu.popup(this)
                        }
                    }

                    // Drag to timeline support
                    Drag.active: dragMouseArea.drag.active
                    Drag.dragType: Drag.Automatic
                    Drag.supportedActions: Qt.MoveAction
                    Drag.hotSpot.x: width/2
                    Drag.hotSpot.y: height/2
                    Drag.mimeData: {
                        "scrite/sceneID": element.scene.id
                    }
                    Drag.source: element

                    Image {
                        id: dragTriggerButton
                        source: "../icons/action/view_array.png"
                        width: 24; height: 24
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 1
                        anchors.rightMargin: 3

                        MouseArea {
                            id: dragMouseArea
                            anchors.fill: parent
                            drag.target: parent
                            cursorShape: Qt.SizeAllCursor
                            onPressed: {
                                elementItemDelegate.grabToImage(function(result) {
                                    elementItemDelegate.Drag.imageSource = result.url
                                })
                            }
                        }
                    }
                }
            }

            DropArea {
                anchors.fill: parent
                keys: [dropAreaKey]

                onEntered: dropAreaIndicator.highlightAsDropArea = true
                onExited: dropAreaIndicator.highlightAsDropArea = false

                onDropped: {
                    dropAreaIndicator.highlightAsDropArea = false
                    dropSceneAt(drop.source, index)
                    drop.acceptProposedAction()
                }
            }

            Rectangle {
                id: dropAreaIndicator
                width: 5
                height: parent.height
                anchors.left: parent.left

                property bool highlightAsDropArea: false
                color: highlightAsDropArea ? dropAreaHighlightColor : Qt.rgba(0,0,0,0)
            }
        }
    }

    TextArea {
        readOnly: true
        width: parent.width*0.7
        anchors.centerIn: parent
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 30
        enabled: false
        visible: scriteDocument.screenplay.elementCount === 0
        text: "Once you create a scene in the canvas, you can drag and drop them here to insert them into the timeline of your screenplay."
    }

    Menu {
        id: elementItemMenu
        property ScreenplayElement element

        MenuItem {
            text: "Remove"
            onClicked: {
                scriteDocument.screenplay.removeElement(elementItemMenu.element)
                elementItemMenu.close()
            }
        }
    }

    function dropSceneAt(source, index) {
        if(source === null)
            return

        var sourceType = app.typeName(source)

        if(sourceType === "ScreenplayElement") {
            scriteDocument.screenplay.moveElement(source, index)
            return
        }

        var sceneID = source.id
        if(sceneID.length === 0)
            return

        var scene = scriteDocument.structure.findElementBySceneID(sceneID)
        if(scene === null)
            return

        var element = screenplayElementComponent.createObject()
        element.sceneID = sceneID
        scriteDocument.screenplay.insertAt(element, index)
    }

    Component {
        id: screenplayElementComponent

        ScreenplayElement {
            screenplay: scriteDocument.screenplay
        }
    }
}
