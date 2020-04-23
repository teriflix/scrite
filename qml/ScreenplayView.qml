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

                ToolButton2 {
                    icon.source: "../icons/content/add_box.png"
                    suggestedWidth: parent.width; suggestedHeight: parent.width
                    ToolTip.text: "Add a act, chapter or interval break."
                    autoRepeat: false
                    enabled: scriteDocument.screenplay.elementCount === 0 ||
                             scriteDocument.screenplay.currentElementIndex >= 0
                    onClicked: breakElementMenu.popup()
                    down: breakElementMenu.visible

                    Menu {
                        id: breakElementMenu

                        Repeater {
                            model: app.enumerationModel(scriteDocument.screenplay, "BreakType")

                            MenuItem {
                                text: modelData.key
                                onClicked: scriteDocument.screenplay.insertBreakElement(modelData.value, scriteDocument.screenplay.currentElementIndex+1)
                            }
                        }
                    }
                }
            }
        }
    }

    FocusIndicator {
        id: focusIndicator
        active: mainUndoStack.active
        anchors.fill: screenplayElementList
        anchors.margins: -3

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
            screenplayElementList.somethingIsBeingDropped = true
            screenplayElementList.footerItem.highlightAsDropArea = true
        }

        onExited: {
            screenplayElementList.somethingIsBeingDropped = false
            screenplayElementList.footerItem.highlightAsDropArea = false
        }

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
        anchors.margins: 3
        clip: true
        property bool somethingIsBeingDropped: false
        visible: count > 0 || somethingIsBeingDropped
        model: scriteDocument.screenplay
        property real minimumDelegateWidth: 100
        property real perElementWidth: 2.5
        property bool moveMode: false
        orientation: Qt.Horizontal
        currentIndex: scriteDocument.screenplay.currentElementIndex
        ScrollBar.horizontal: ScrollBar {
            policy: screenplayElementList.width < screenplayElementList.contentWidth ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.5)
                dark: "black"
            }
        }
        FocusTracker.window: qmlWindow
        FocusTracker.indicator.target: mainUndoStack
        FocusTracker.indicator.property: "timelineEditorActive"

        Transition {
            id: moveAndDisplace
            NumberAnimation { properties: "x,y"; duration: 250 }
        }

        moveDisplaced: moveAndDisplace
        move: moveAndDisplace

        EventFilter.active: app.isWindowsPlatform || app.isLinuxPlatform
        EventFilter.events: [31]
        EventFilter.onFilter: {
            if(event.delta < 0)
                contentX = Math.min(contentX+20, contentWidth-width)
            else
                contentX = Math.max(contentX-20, 0)
            result.acceptEvent = true
            result.filter = true
        }

        footer: Item {
            property bool highlightAsDropArea: false
            width: screenplayElementList.minimumDelegateWidth
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
            property bool isBreakElement: element.elementType === ScreenplayElement.BreakElementType
            property bool active: element.scene ? scriteDocument.screenplay.activeScene === element.scene : false
            property int sceneElementCount: element.scene ? element.scene.elementCount : 1
            property string sceneTitle: element.scene ? element.scene.title : element.sceneID
            property color sceneColor: element.scene ? element.scene.color : "white"
            width: isBreakElement ? 60 :
                                    Math.max(screenplayElementList.minimumDelegateWidth, sceneElementCount*screenplayElementList.perElementWidth*zoomLevel)
            height: screenplayElementList.height-10

            Loader {
                anchors.fill: parent
                anchors.leftMargin: 7.5
                anchors.rightMargin: 2.5
                anchors.topMargin: 2
                active: element !== null // && (isBreakElement || element.scene !== null)
                sourceComponent: Rectangle {
                    radius: isBreakElement ? 0 : 8
                    color: Qt.tint(sceneColor, "#C0FFFFFF")
                    border.color: color === Qt.rgba(1,1,1,1) ? "black" : sceneColor
                    border.width: elementItemDelegate.active ? 4 : 1
                    Behavior on border.width { NumberAnimation { duration: 400 } }

                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: menuButton.bottom
                        anchors.bottom: dragTriggerButton.top
                        anchors.margins: 5

                        Text {
                            lineHeight: 1.25
                            text: sceneTitle
                            elide: Text.ElideRight
                            anchors.centerIn: parent
                            font.bold: isBreakElement
                            transformOrigin: Item.Center
                            verticalAlignment: Text.AlignTop
                            rotation: isBreakElement ? -90 : 0
                            horizontalAlignment: Text.AlignHCenter
                            maximumLineCount: isBreakElement ? 1 : 4
                            font.pixelSize: isBreakElement ? 18 : 15
                            visible: isBreakElement ? true : width >= 80
                            wrapMode: isBreakElement ? Text.NoWrap : Text.WrapAnywhere
                            font.capitalization: isBreakElement ? Font.AllUppercase : Font.MixedCase
                            width: elementItemDelegate.isBreakElement ? parent.height : parent.width
                            height: elementItemDelegate.isBreakElement ? contentHeight : parent.height
                        }
                    }

                    MouseArea {
                        enabled: !isBreakElement
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
                        "scrite/sceneID": element.sceneID
                    }
                    Drag.source: element
                    Drag.onActiveChanged: {
                        if(!isBreakElement)
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
        width: parent.width*0.5
        anchors.centerIn: parent
        sourceComponent: TextArea {
            readOnly: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            renderType: Text.NativeRendering
            font.pixelSize: 30
            enabled: false
            text: "Drag scenes on the structure canvas from their bottom right corner to this timeline view here."
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
        if(element.elementType === ScreenplayElement.SceneElementType)
            scriteDocument.screenplay.currentElementIndex = index
    }

    Component {
        id: screenplayElementComponent

        ScreenplayElement {
            screenplay: scriteDocument.screenplay
        }
    }
}
