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

Item {
    signal requestEditor()

    ToolBar {
        id: toolbar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 1

        ToolButton {
            icon.source: "../icons/content/add_box.png"
            text: "New Scene"
            display: ToolButton.TextBesideIcon
            down: newSceneColorMenuLoader.active
            onClicked: newSceneColorMenuLoader.active = true

            Loader {
                id: newSceneColorMenuLoader
                width: parent.width; height: 1
                anchors.top: parent.bottom
                sourceComponent: ColorMenu { }
                active: false
                onItemChanged: {
                    if(item)
                        item.open()
                }

                Connections {
                    target: newSceneColorMenuLoader.item
                    onAboutToHide: newSceneColorMenuLoader.active = false
                    onMenuItemClicked: {
                        canvas.newElementColor = color
                        canvas.newElementMode = true
                        newSceneColorMenuLoader.active = false
                    }
                }
            }
        }
    }

    ScrollView {
        id: canvasScroll
        anchors.left: parent.left
        anchors.top: toolbar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 1
        contentWidth: canvas.width * canvas.scale
        contentHeight: canvas.height * canvas.scale
        clip: true

        Item {
            id: canvas
            width: scriteDocument.structure.canvasWidth
            height: scriteDocument.structure.canvasHeight
            scale: scriteDocument.structure.zoomLevel
            transformOrigin: Item.TopLeft
            onScaleChanged: canvasScroll.contentItem.returnToBounds()

            property int currentIndex: scriteDocument.structure.currentElementIndex
            property int editIndex: -1    // index of item being edited
            onCurrentIndexChanged: {
                if(currentIndex !== editIndex)
                    editIndex = -1
                if(currentIndex >= 0) {
                    var element = scriteDocument.structure.elementAt(currentIndex)
                    var rect = Qt.rect(element.x-150,element.y-50,300,100)
                    var flick = canvasScroll.contentItem

                    if(rect.left < flick.contentX)
                        flick.contentX = rect.left
                    else if(rect.right > flick.contentX+canvasScroll.width)
                        flick.contentX = rect.right-canvasScroll.width

                    if(rect.top < flick.contentY)
                        flick.contentY = rect.top
                    else if(rect.bottom > flick.contentY+canvasScroll.height)
                        flick.contentY = rect.bottom-canvasScroll.height
                }
            }
            onEditIndexChanged: {
                if(editIndex >= 0)
                    scriteDocument.structure.currentElementIndex = editIndex
            }

            property color newElementColor: "blue"
            property bool newElementMode: false

            Rectangle {
                anchors.fill: parent
                color: "#F8ECC2"
                opacity: 0.4
            }

            GridBackground {
                anchors.fill: parent
                opacity: 0.5
                majorTickColor: "darkgray"
                minorTickColor: "gray"
                majorTickLineWidth: 5
                minorTickLineWidth: 1
                tickDistance: 10
            }

            MouseArea {
                id: canvasMouseArea
                anchors.fill: parent
                hoverEnabled: parent.newElementMode
                cursorShape: parent.newElementMode ? Qt.DragMoveCursor : Qt.ArrowCursor
                onDoubleClicked: canvas.createElement(mouse.x, mouse.y, parent.newElementColor)
                preventStealing: true
                onClicked: {
                    if(selectionRect.visible) {
                        var dist = Math.max(
                                    Math.max( mouse.x - selectionRect.area.right,
                                             selectionRect.area.left - mouse.x ),
                                    Math.max( mouse.y - selectionRect.area.bottom,
                                             selectionRect.area.top - mouse.y )
                                    )
                        if(dist > 10)
                            selectionRect.visible = false
                    }

                    if(parent.newElementMode) {
                        canvas.createElement(mouse.x, mouse.y, parent.newElementColor)
                        parent.newElementMode = false
                    } else {
                        scriteDocument.structure.currentElementIndex = -1
                        requestEditor()
                    }
                }

                onPressed: {
                    if(parent.newElementMode || selectionRect.enabled)
                        return;

                    scriteDocument.structure.currentElementIndex = -1
                    selectionRect.from = Qt.point(mouse.x, mouse.y)
                    selectionRect.to = Qt.point(mouse.x, mouse.y)
                    selectionRect.enabled = false
                    selectionRect.visible = true
                }

                onPositionChanged: {
                    if(!selectionRect.visible || selectionRect.enabled)
                        return;

                    selectionRect.to = Qt.point(mouse.x, mouse.y)
                    selectionRect.enabled = false
                }

                onReleased: {
                    if(!selectionRect.visible || selectionRect.enabled)
                        return;

                    selectionRect.to = Qt.point(mouse.x, mouse.y)
                    selectionRect.enabled = true
                    if(selectionRect.width < 50 && selectionRect.height < 50) {
                        selectionRect.enabled = false
                        selectionRect.visible = false
                    }
                }
            }

            function createElement(x, y, c) {
                var props = {"x": x, "y": y}
                var element = structureElementComponent.createObject(scriteDocument.structure, props)
                element.scene.color = c
                scriteDocument.structure.addElement(element)
                editIndex = scriteDocument.structure.elementCount-1
                scriteDocument.structure.currentElementIndex = editIndex
                requestEditor()
            }

            Rectangle {
                id: selectionRect
                visible: false
                enabled: false
                color: systemPalette.highlight
                border { width: 2; color: "black" }
                radius: 8                    
                onVisibleChanged: {
                    if(!visible)
                        enabled = false
                }

                property point from: Qt.point(0,0)
                property point to: Qt.point(0,0)
                property rect area: {
                    if(enabled)
                        return Qt.rect(x, y, width, height)
                    if(from === to)
                        return Qt.rect(from.x, from.y, 1, 1)
                    return Qt.rect( Math.min(from.x,to.x), Math.min(from.y,to.y), Math.abs(to.x-from.x), Math.abs(to.y-from.y) )
                }
                property point topLeft

                x: area.x
                y: area.y
                width: area.width
                height: area.height

                MouseArea {
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.XAndYAxis
                }

                onXChanged: shiftElements()
                onYChanged: shiftElements()

                onEnabledChanged: {
                    if(enabled) {
                        var items = []
                        var count = elementItems.count
                        for(var i=0; i<count; i++) {
                            var item = elementItems.itemAt(i)
                            var p1 = Qt.point(item.x, item.y)
                            var p2 = Qt.point(item.x+item.width, item.y+item.height)
                            var areaContainsPoint = function(p) {
                                return area.left <= p.x && p.x <= area.right &&
                                        area.top <= p.y && p.y <= area.bottom;
                            }
                            if(areaContainsPoint(p1) || areaContainsPoint(p2))
                                items.push(item)
                        }
                        elements = items
                        topLeft = Qt.point(area.x, area.y)
                    } else {
                        elements = []
                        from = Qt.point(0,0)
                        to = Qt.point(0,0)
                        topLeft = Qt.point(0,0)
                    }
                }

                property var elements: []

                function shiftElements() {
                    if(!enabled || elements.length === 0)
                        return

                    var dx = x - topLeft.x
                    var dy = y - topLeft.y
                    topLeft = Qt.point(x,y)
                    for(var i=0; i<elements.length; i++) {
                        var item = elements[i]
                        item.x = item.x + dx
                        item.y = item.y + dy
                    }
                }
            }

            Loader {
                anchors.fill: parent
                sourceComponent: elementSequenceVisualizerComponent
                active: elementItems.count === scriteDocument.structure.elementCount
            }

            Repeater {
                id: elementItems
                model: scriteDocument.structure.elementCount

                Item {
                    id: elementItem
                    property StructureElement element: scriteDocument.structure.elementAt(index)
                    property bool selected: canvas.currentIndex === index
                    property bool editing: canvas.editIndex === index
                    width: titleText.width + 10
                    height: titleText.height + 10
                    x: element.x - width/2
                    y: element.y - height/2
                    focus: selected && !editing

                    Keys.onPressed: {
                        if(event.key === Qt.Key_F2)
                            canvas.editIndex = index
                    }

                    onSelectedChanged: {
                        if(selected)
                            scriteDocument.structure.currentElementIndex = index
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 8
                        border.width: parent.selected ? 4 : 1
                        border.color: (element.scene.color === Qt.rgba(1,1,1,1) ? "lightgray" : element.scene.color)
                        color: Qt.tint(element.scene.color, "#C0FFFFFF")
                        Behavior on border.width { NumberAnimation { duration: 400 } }
                    }

                    TextArea {
                        id: titleText
                        width: 250
                        wrapMode: Text.WordWrap
                        text: element.scene.title
                        anchors.centerIn: parent
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        readOnly: !parent.editing
                        onReadOnlyChanged: {
                            if(readOnly === false) {
                                selectAll()
                                forceActiveFocus()
                            }
                        }
                        onTextChanged: element.scene.title = text
                        onEditingFinished: canvas.editIndex = -1
                        Keys.onReturnPressed: editingFinished()
                    }

                    ToolButton {
                        id: elementOptionsButton
                        icon.source: "../icons/navigation/menu.png"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.right
                        anchors.leftMargin: 10
                        visible: parent.selected
                        down: elementOptionsMenuLoader.active
                        onClicked: elementOptionsMenuLoader.active = true
                    }

                    Loader {
                        id: elementOptionsMenuLoader
                        anchors.top: elementOptionsButton.bottom
                        anchors.right: parent.right
                        anchors.topMargin: 10
                        anchors.rightMargin: -elementOptionsButton.width-10
                        width: parent.width; height: 1
                        sourceComponent: Menu {
                            signal colorMenuItemClicked(string color)
                            onAboutToHide: elementOptionsMenuLoader.active = false

                            MenuItem {
                                action: Action {
                                    text: "Scene Heading"
                                    checkable: true
                                    checked: element.scene.heading.enabled
                                }
                                onTriggered: element.scene.heading.enabled = action.checked
                            }

                            ColorMenu {
                                title: "Colors"
                                onMenuItemClicked: colorMenuItemClicked(color)
                            }

                            MenuItem {
                                text: "Delete"
                                onClicked: scriteDocument.structure.removeElement(element)
                            }
                        }
                        active: false
                        onItemChanged: {
                            if(item)
                                item.open()
                        }

                        Connections {
                            target: elementOptionsMenuLoader.item
                            onColorMenuItemClicked: {
                                element.scene.color = color
                                elementOptionsMenuLoader.active = false
                            }
                        }
                    }

                    MouseArea {
                        id: elementItemMouseArea
                        anchors.fill: parent
                        enabled: !titleText.activeFocus && !selectionRect.visible
                        hoverEnabled: true
                        cursorShape: enabled ? (pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor) : Qt.ArrowCursor
                        acceptedButtons: Qt.LeftButton|Qt.RightButton
                        onClicked: {
                            if(mouse.button === Qt.RightButton)
                                elementOptionsMenuLoader.active = true
                            else if(mouse.button === Qt.LeftButton) {
                                scriteDocument.structure.currentElementIndex = index
                                requestEditor()
                            }
                        }
                        onDoubleClicked: {
                            if(mouse.button === Qt.LeftButton)
                                canvas.editIndex = index
                        }

                        drag.target: parent
                        drag.axis: Drag.XAndYAxis
                        drag.onActiveChanged: {
                            scriteDocument.structure.currentElementIndex = index
                            requestEditor()
                        }
                    }

                    onXChanged: updatePosition()
                    onYChanged: updatePosition()
                    function updatePosition() {
                        element.x = (x + width/2)
                        element.y = (y + height/2)
                    }

                    onWidthChanged: element.width = width
                    onHeightChanged: element.height = height

                    // Drag to timeline support
                    Drag.active: dragMouseArea.drag.active
                    Drag.dragType: Drag.Automatic
                    Drag.supportedActions: Qt.LinkAction
                    Drag.hotSpot.x: width/2
                    Drag.hotSpot.y: height/2
                    Drag.mimeData: {
                        "scrite/sceneID": element.scene.id
                    }
                    Drag.source: element.scene

                    Image {
                        visible: !parent.editing
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
                                elementItem.grabToImage(function(result) {
                                    elementItem.Drag.imageSource = result.url
                                })
                            }
                        }
                    }
                }
            }
        }
    }

    Slider {
        id: zoomSlider
        anchors.bottom: canvasScroll.bottom
        anchors.right: canvasScroll.right
        anchors.rightMargin: 20
        width: 150
        from: Math.min(canvasScroll.width/canvas.width, canvasScroll.height/canvas.height)
        to: 2
        value: 1
        onValueChanged: scriteDocument.structure.zoomLevel = value
    }

    TextArea {
        readOnly: true
        width: parent.width*0.7
        anchors.centerIn: parent
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 30
        enabled: false
        visible: scriteDocument.structure.elementCount === 0
        text: "Double click on an empty area in the canvas to create a scene. Hold the command key and drag lines from one scene to another to create a sequence."
    }

    Component {
        id: structureElementComponent

        StructureElement {
            scene: Scene {
                title: "New Scene"
                heading.locationType: SceneHeading.Interior
                heading.location: "Somewhere"
                heading.moment: SceneHeading.Day
            }
        }
    }

    Component {
        id: elementSequenceVisualizerComponent

        Item {
            id: pathItemsContainer

            Repeater {
                model: scriteDocument.structureElementSequence

                StructureElementConnector {
                    lineType: StructureElementConnector.CurvedLine
                    fromElement: scriteDocument.structure.elementAt(modelData.from)
                    toElement: scriteDocument.structure.elementAt(modelData.to)
                    arrowAndLabelSpacing: labelBg.width

                    Rectangle {
                        id: labelBg
                        width: Math.max(label.width,label.height)+20
                        height: width; radius: width/2
                        border.width: 1; border.color: "black"
                        x: parent.suggestedLabelPosition.x - radius
                        y: parent.suggestedLabelPosition.y - radius
                        color: Qt.tint(parent.outlineColor, "#E0FFFFFF")

                        Text {
                            id: label
                            anchors.centerIn: parent
                            font.pixelSize: 12
                            text: (index+1)
                        }
                    }
                }
            }
        }
    }
}
