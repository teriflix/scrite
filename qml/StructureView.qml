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

    Rectangle {
        id: toolbar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 1
        color: "lightgray"
        height: toolbarLayout.height+4

        Row {
            id: toolbarLayout
            spacing: 10
            width: parent.width-4
            anchors.verticalCenter: parent.verticalCenter

            ToolButton {
                id: newSceneButton
                icon.source: "../icons/content/add_box.png"
                text: "New Scene"
                display: ToolButton.TextBesideIcon
                down: newSceneColorMenuLoader.active
                onClicked: newSceneColorMenuLoader.active = true
                anchors.verticalCenter: parent.verticalCenter

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

            SearchBar {
                id: searchBar
                width: parent.width-newSceneButton.width-parent.spacing
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    FocusIndicator {
        id: focusIndicator
        active: structureScreenplayUndoStack.active
        anchors.fill: canvasScroll
        anchors.margins: -3
    }

    Rectangle {
        anchors.fill: canvasScroll
        color: "#F8ECC2"
        opacity: 0.4
    }

    ScrollArea {
        id: canvasScroll
        anchors.left: parent.left
        anchors.top: toolbar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 3
        contentWidth: canvas.width * canvas.scale
        contentHeight: canvas.height * canvas.scale
        initialContentWidth: canvas.width
        initialContentHeight: canvas.height
        clip: true

        GridBackground {
            id: canvas
            width: widthBinder.get
            height: heightBinder.get
            scale: canvasScroll.suggestedScale
            majorTickColor: "darkgray"
            minorTickColor: "gray"
            majorTickLineWidth: 5
            minorTickLineWidth: 1
            tickDistance: scriteDocument.structure.canvasGridSize
            antialiasing: false
            tickColorOpacity: 0.5 * scale

            transformOrigin: Item.TopLeft

            DelayedPropertyBinder{
                id: widthBinder
                initial: 1000
                set: Math.max((Math.ceil(canvas.childrenRect.right / 100) * 100), initial)
                onGetChanged: scriteDocument.structure.canvasWidth = get
            }

            DelayedPropertyBinder {
                id: heightBinder
                initial: 1000
                set: Math.max((Math.ceil(canvas.childrenRect.bottom / 100) * 100), initial)
                onGetChanged: scriteDocument.structure.canvasHeight = get
            }

            FocusTracker.window: qmlWindow
            FocusTracker.indicator.target: structureScreenplayUndoStack
            FocusTracker.indicator.property: "structureViewHasFocus"

            property int currentIndex: scriteDocument.structure.currentElementIndex
            property int editIndex: -1    // index of item being edited
            property bool ensureCurrentItemIsVisible: true
            onCurrentIndexChanged: {
                editIndex = -1
                if(ensureCurrentItemIsVisible)
                    canvasScroll.ensureItemVisible(elementItems.itemAt(currentIndex), scale)
            }
            onEditIndexChanged: {
                if(editIndex >= 0 && currentIndex !== editIndex) {
                    ensureCurrentItemIsVisible = false
                    scriteDocument.structure.currentElementIndex = editIndex
                    ensureCurrentItemIsVisible = true
                }
            }

            property int documentProgressStatus: Aggregation.findProgressReport(scriteDocument).status
            onDocumentProgressStatusChanged: {
                if(documentProgressStatus === ProgressReport.Started) {
                    ensureCurrentItemIsVisible = scriteDocument.structure.elementCount === 0
                    return
                }

                if(documentProgressStatus === ProgressReport.Finished) {
                    if(ensureCurrentItemIsVisible)
                        app.execLater(100, function() { canvasScroll.ensureItemVisible(elementItems.itemAt(currentIndex), canvasScroll.scale) })
                    ensureCurrentItemIsVisible = true
                }
            }

            function createElement(x, y, c) {
                var props = {
                    "x": Math.max(scriteDocument.structure.snapToGrid(x), 130),
                    "y": Math.max(scriteDocument.structure.snapToGrid(y), 50)
                }

                var element = structureElementComponent.createObject(scriteDocument.structure, props)
                element.scene.color = c
                scriteDocument.structure.addElement(element)

                canvas.ensureCurrentItemIsVisible = false
                scriteDocument.structure.currentElementIndex = scriteDocument.structure.elementCount-1
                requestEditor()
                canvas.editIndex = scriteDocument.structure.elementCount-1
                canvas.ensureCurrentItemIsVisible = true
            }

            property color newElementColor: "blue"
            property bool newElementMode: false

            MouseArea {
                id: canvasMouseArea
                anchors.fill: parent
                hoverEnabled: parent.newElementMode
                cursorShape: parent.newElementMode ? Qt.DragMoveCursor : Qt.ArrowCursor
                onDoubleClicked: canvas.createElement(mouse.x-130, mouse.y-22, parent.newElementColor)
                preventStealing: true
                onClicked: {
                    parent.forceActiveFocus()
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
                        canvas.createElement(mouse.x-130, mouse.y-22, parent.newElementColor)
                        parent.newElementMode = false
                    } else {
                        scriteDocument.structure.currentElementIndex = -1
                        requestEditor()
                    }
                }

                onPressed: {
                    parent.forceActiveFocus()
                    if(parent.newElementMode || selectionRect.enabled)
                        return

                    if(mouse.modifiers & Qt.ControlModifier) {
                        scriteDocument.structure.currentElementIndex = -1
                        selectionRect.from = Qt.point(mouse.x, mouse.y)
                        selectionRect.to = Qt.point(mouse.x, mouse.y)
                        selectionRect.enabled = false
                        selectionRect.visible = true
                    } else
                        mouse.accepted = false
                }

                onPositionChanged: {
                    if(!selectionRect.visible || selectionRect.enabled) {
                        mouse.accepted = false
                        return;
                    }

                    selectionRect.to = Qt.point(mouse.x, mouse.y)
                    selectionRect.enabled = false
                }

                onReleased: {
                    if(!selectionRect.visible || selectionRect.enabled) {
                        mouse.accepted = false
                        return;
                    }

                    selectionRect.to = Qt.point(mouse.x, mouse.y)
                    selectionRect.enabled = true
                    if(selectionRect.width < 50 && selectionRect.height < 50) {
                        selectionRect.enabled = false
                        selectionRect.visible = false
                    }
                }
            }

            Rectangle {
                id: selectionRect
                visible: false
                enabled: false
                color: systemPalette.highlight
                border { width: 0; color: "black" }
                radius: 8                    
                onVisibleChanged: {
                    if(!visible)
                        enabled = false
                }
                opacity: 0.5
                z: 10

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
                model: scriteDocument.structure.elements

                Item {
                    id: elementItem
                    property StructureElement element: modelData
                    property bool selected: canvas.currentIndex === index
                    property bool editing: canvas.editIndex === index
                    property real elementX: element.x
                    property real elementY: element.y
                    width: titleText.width + 10
                    height: titleText.height + 10
                    x: elementX // - width/2
                    y: elementY // - height/2

                    // This happens when element is dragged
                    onXChanged: element.x = x // width/2
                    onYChanged: element.y = y // height/2

                    // This happens when unto/redo happens
                    onElementXChanged: x = elementX // - width/2
                    onElementYChanged: y = elementY // - height/2

                    // This happens when text is edited
                    onWidthChanged: element.width = width
                    onHeightChanged: element.height = height

                    Keys.onPressed: {
                        if(event.key === Qt.Key_F2)
                            canvas.editIndex = index
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 8
                        border.width: parent.selected ? 4 : 1
                        border.color: (element.scene.color === Qt.rgba(1,1,1,1) ? "lightgray" : element.scene.color)
                        color: Qt.tint(element.scene.color, "#C0FFFFFF")
                        Behavior on border.width { NumberAnimation { duration: 400 } }
                    }

                    TextViewEdit {
                        id: titleText
                        width: 250
                        wrapMode: Text.WordWrap
                        text: element.scene.title
                        anchors.centerIn: parent
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        readOnly: !parent.editing
                        onTextEdited: element.scene.title = text
                        onEditingFinished: {
                            canvas.editIndex = -1
                            searchBar.searchEngine.clearSearch()
                        }
                        onHighlightRequest: scriteDocument.structure.currentElementIndex = index
                        Keys.onReturnPressed: editingFinished()
                        searchEngine: searchBar.searchEngine
                        searchSequenceNumber: index
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
                        enabled: !titleText.hasFocus && !selectionRect.visible
                        hoverEnabled: true
                        cursorShape: enabled ? (pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor) : Qt.ArrowCursor
                        acceptedButtons: Qt.LeftButton|Qt.RightButton
                        onClicked: {
                            parent.forceActiveFocus()
                            if(mouse.button === Qt.RightButton)
                                elementOptionsMenuLoader.active = true
                            else if(mouse.button === Qt.LeftButton) {
                                canvas.ensureCurrentItemIsVisible = false
                                scriteDocument.structure.currentElementIndex = index
                                canvas.ensureCurrentItemIsVisible = true
                                requestEditor()
                            }
                        }
                        onDoubleClicked: {
                            if(mouse.button === Qt.LeftButton)
                                canvas.editIndex = index
                        }

                        drag.target: parent
                        drag.axis: Drag.XAndYAxis
                        drag.minimumX: 0
                        drag.minimumY: 0
                        drag.onActiveChanged: {
                            parent.forceActiveFocus()
                            canvas.ensureCurrentItemIsVisible = false
                            scriteDocument.structure.currentElementIndex = index
                            canvas.ensureCurrentItemIsVisible = true
                            requestEditor()
                            parent.x = scriteDocument.structure.snapToGrid(parent.x)
                            parent.y = scriteDocument.structure.snapToGrid(parent.y)
                        }
                    }

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

    Loader {
        width: parent.width*0.7
        anchors.centerIn: parent
        active: scriteDocument.structure.elementCount === 0
        sourceComponent: TextArea {
            readOnly: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 30
            enabled: false
            renderType: Text.NativeRendering
            text: "Double click on an empty area in the canvas to create a scene. Hold the command key and drag lines from one scene to another to create a sequence."
        }
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
