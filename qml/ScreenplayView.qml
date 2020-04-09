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
    id: screenplayView
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

    Item {
        id: screenplayTools
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 1
        width: 60
        z: 1

        Rectangle {
            width: parent.height
            height: parent.width
            anchors.centerIn: parent
            transformOrigin: Item.Center
            rotation: -90
            gradient: Gradient {
                GradientStop { position: 0; color: "#FF8c9cb1" }
                GradientStop { position: 1; color: "#008c9cb1" }
            }
        }

        ScrollView {
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            width: screenplayToolsLayout.width

            Column {
                id: screenplayToolsLayout
                width: 40

                ToolButton2 {
                    icon.source: "../icons/content/clear_all.png"
                    suggestedWidth: parent.width; suggestedHeight: parent.width
                    ToolTip.text: "Clear the screenplay, while retaining the scenes."
                    onClicked: {
                        askQuestion({
                                        "question": "Are you sure you want to clear the screenplay?",
                                        "okButtonText": "Yes",
                                        "cancelButtonText": "No",
                                        "callback": function(val) {
                                            if(val) {
                                                screenplayElementList.forceActiveFocus()
                                                scriteDocument.screenplay.clearElements()
                                            }
                                        }
                                    }, this)
                    }
                }

                ToolButton2 {
                    icon.source: "../icons/navigation/zoom_in.png"
                    suggestedWidth: parent.width; suggestedHeight: parent.width
                    ToolTip.text: "Increase size of blocks in this view."
                    onClicked: zoomLevel = Math.min(zoomLevel * 1.1, 4.0)
                    autoRepeat: true
                }

                ToolButton2 {
                    icon.source: "../icons/navigation/zoom_out.png"
                    suggestedWidth: parent.width; suggestedHeight: parent.width
                    ToolTip.text: "Decrease size of blocks in this view."
                    onClicked: zoomLevel = Math.max(zoomLevel * 0.9, screenplayElementList.perElementWidth/screenplayElementList.minimumDelegateWidth)
                    autoRepeat: true
                }
            }
        }
    }

    FocusIndicator {
        id: focusIndicator
        active: structureScreenplayUndoStack.active
        anchors.fill: screenplayElementList
        anchors.margins: -10

        MouseArea {
            anchors.fill: parent
            onClicked: screenplayElementList.forceActiveFocus()
        }
    }

    DropArea {
        anchors.fill: parent
        keys: [dropAreaKey]

        onEntered: {
            screenplayElementList.forceActiveFocus()
            screenplayElementList.footerItem.highlightAsDropArea = true
        }
        onExited: screenplayElementList.footerItem.highlightAsDropArea = false

        onDropped: {
            screenplayElementList.footerItem.highlightAsDropArea = false
            dropSceneAt(drop.source, -1)
            drop.acceptProposedAction()
        }
    }

    ListView {
        id: screenplayElementList
        anchors.left: screenplayTools.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 10
        anchors.leftMargin: 3
        clip: true
        visible: count > 0
        model: scriteDocument.screenplay
        property real minimumDelegateWidth: 100
        property real perElementWidth: 2.5
        property bool moveMode: false
        orientation: Qt.Horizontal
        currentIndex: scriteDocument.screenplay.currentElementIndex
        ScrollBar.horizontal: ScrollBar { }
        FocusTracker.window: qmlWindow
        FocusTracker.indicator.target: structureScreenplayUndoStack
        FocusTracker.indicator.property: "screenplayViewHasFocus"

        Transition {
            id: moveAndDisplace
            NumberAnimation { properties: "x,y"; duration: 250 }
        }

        moveDisplaced: moveAndDisplace
        move: moveAndDisplace

        footer: Item {
            property bool highlightAsDropArea: false
            width: screenplayElementList.width-2*screenplayElementList.minimumDelegateWidth
            height: screenplayElementList.height

            Rectangle {
                width: 5
                height: parent.height
                color: parent.highlightAsDropArea ? dropAreaHighlightColor : Qt.rgba(0,0,0,0)
            }
        }

        delegate: Item {
            id: elementItemDelegate
            property ScreenplayElement element: screenplayElement
            property bool active: element ? scriteDocument.screenplay.activeScene === element.scene : false
            width: Math.max(screenplayElementList.minimumDelegateWidth, element.scene.elementCount*screenplayElementList.perElementWidth*zoomLevel)
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
                        verticalAlignment: Text.AlignTop
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
                            elementItemDelegate.forceActiveFocus()
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
                    Drag.onActiveChanged: {
                        scriteDocument.screenplay.currentElementIndex = index
                        screenplayElementList.moveMode = Drag.active
                    }

                    Image {
                        id: dragTriggerButton
                        source: "../icons/action/view_array.png"
                        width: 24; height: 24
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 1
                        anchors.rightMargin: 3
                        opacity: dragMouseArea.containsMouse ? 1 : 0.25
                        scale: dragMouseArea.containsMouse ? 2 : 1
                        Behavior on scale { NumberAnimation { duration: 250 } }

                        MouseArea {
                            id: dragMouseArea
                            hoverEnabled: true
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

                onEntered: {
                    screenplayElementList.forceActiveFocus()
                    dropAreaIndicator.highlightAsDropArea = true
                }
                onExited: dropAreaIndicator.highlightAsDropArea = false

                onDropped: {
                    dropAreaIndicator.highlightAsDropArea = false
                    dropSceneAt(drop.source, index)
                    drop.acceptProposedAction()
                    if(!screenplayElementList.moveMode)
                        screenplayElementList.positionViewAtIndex(index,ListView.Contain)
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

    Loader {
        active: scriteDocument.screenplay.elementCount === 0
        width: parent.width*0.7
        anchors.centerIn: parent
        sourceComponent: TextArea {
            readOnly: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            renderType: Text.NativeRendering
            font.pixelSize: 30
            enabled: false
            text: "Once you create a scene in the canvas, you can drag and drop them here to insert them into the timeline of your screenplay."
        }
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
        scriteDocument.screenplay.insertElementAt(element, index)
        scriteDocument.screenplay.currentElementIndex = index
    }

    Component {
        id: screenplayElementComponent

        ScreenplayElement {
            screenplay: scriteDocument.screenplay
        }
    }
}
