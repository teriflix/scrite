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

Item {
    signal requestEditor()
    signal releaseEditor()

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
            spacing: 3
            width: parent.width-4
            anchors.verticalCenter: parent.verticalCenter

            Row {
                id: toolbarButtons
                spacing: parent.spacing
                anchors.verticalCenter: parent.verticalCenter

                ToolButton2 {
                    id: newSceneButton
                    icon.source: "../icons/content/add_box.png"
                    text: "Add Scene"
                    shortcutText: "N"
                    ToolTip.text: "Press " + shortcutText + " to create a new scene under the mouse on the canvas."
                    suggestedWidth: 130
                    suggestedHeight: 50
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

                ToolButton2 {
                    icon.source: "../icons/navigation/zoom_in.png"
                    text: "Zoom In"
                    display: ToolButton.IconOnly
                    suggestedHeight: 45
                    anchors.verticalCenter: parent.verticalCenter
                    autoRepeat: true
                    onClicked: canvasScroll.zoomIn()
                }

                ToolButton2 {
                    icon.source: "../icons/navigation/zoom_out.png"
                    text: "Zoom Out"
                    display: ToolButton.IconOnly
                    suggestedHeight: 45
                    anchors.verticalCenter: parent.verticalCenter
                    autoRepeat: true
                    onClicked: canvasScroll.zoomOut()
                }
            }

            SearchBar {
                id: searchBar
                width: parent.width-toolbarButtons.width-parent.spacing
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    FocusIndicator {
        id: focusIndicator
        active: mainUndoStack.active
        anchors.fill: canvasScroll
        anchors.margins: -3
    }

    Rectangle {
        anchors.fill: canvasScroll
        color: structureCanvasSettings.canvasColor
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
        showScrollBars: scriteDocument.structure.elementCount >= 1

        GridBackground {
            id: canvas
            antialiasing: false
            majorTickLineWidth: 2
            minorTickLineWidth: 1
            width: widthBinder.get
            height: heightBinder.get
            tickColorOpacity: 0.5 * scale
            scale: canvasScroll.suggestedScale
            border.width: 2
            border.color: structureCanvasSettings.gridColor
            gridIsVisible: structureCanvasSettings.showGrid
            majorTickColor: structureCanvasSettings.gridColor
            minorTickColor: structureCanvasSettings.gridColor
            tickDistance: scriteDocument.structure.canvasGridSize

            transformOrigin: Item.TopLeft

            Image {
                anchors.fill: parent
                fillMode: Image.Tile
                source: "../images/notebookpage.jpg"
                opacity: 0.5
            }

            DelayedPropertyBinder {
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
            FocusTracker.indicator.target: mainUndoStack
            FocusTracker.indicator.property: "structureEditorActive"

            property int currentIndex: scriteDocument.structure.currentElementIndex
            property int editIndex: -1    // index of item being edited
            property bool ensureCurrentItemIsVisible: true
            onCurrentIndexChanged: {
                editIndex = -1
                if(ensureCurrentItemIsVisible) {
                    canvasScroll.ensureItemVisible(elementItems.itemAt(currentIndex), scale)
                }
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

                element.scene.undoRedoEnabled = true
            }

            property bool newElementMode: false
            property color newElementColor: "blue"

            Keys.onPressed: {
                if(event.key === Qt.Key_N || event.key === Qt.Key_Plus) {
                    var pos = app.mapGlobalPositionToItem(canvas, app.cursorPosition())
                    createElement(pos.x-130, pos.y-22, newElementColor)
                }
            }

            MouseArea {
                id: canvasMouseArea
                anchors.fill: parent
                hoverEnabled: parent.newElementMode
                cursorShape: parent.newElementMode ? Qt.DragMoveCursor : Qt.ArrowCursor
                preventStealing: true
                property bool selectRectWasJustCreated: false

                onDoubleClicked: {
                    canvas.createElement(mouse.x-130, mouse.y-22, parent.newElementColor)
                }

                onClicked: {
                    parent.forceActiveFocus()
                    if(selectRectWasJustCreated || mouse.modifiers & Qt.ControlModifier)
                        return

                    selectionRect.visible = false

                    if(parent.newElementMode) {
                        canvas.createElement(mouse.x-130, mouse.y-22, parent.newElementColor)
                        parent.newElementMode = false
                    } else {
                        scriteDocument.structure.currentElementIndex = -1
                        requestEditor()
                    }
                }

                onPositionChanged: {
                    if(!selectionRect.visible || selectionRect.enabled) {
                        mouse.accepted = false
                        return;
                    }

                    selectionRect.to = Qt.point(mouse.x, mouse.y)
                    selectionRect.enabled = false
                }

                onPressed: {
                    parent.forceActiveFocus()
                    if(parent.newElementMode || selectionRect.enabled || mouse.modifiers & Qt.ControlModifier)
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

                onReleased: {
                    if(!selectionRect.visible || selectionRect.enabled || mouse.modifiers & Qt.ControlModifier) {
                        mouse.accepted = false
                        return
                    }

                    selectionRect.to = Qt.point(mouse.x, mouse.y)
                    selectionRect.enabled = true
                    if(selectionRect.width < 50 && selectionRect.height < 50) {
                        selectionRect.enabled = false
                        selectionRect.visible = false
                    } else {
                        selectRectWasJustCreated = true
                        app.execLater(250, function() { selectRectWasJustCreated=false })
                    }
                }
            }

            Rectangle {
                id: selectionRect
                visible: false
                enabled: false
                color: app.translucent(app.palette.highlight,0.2)
                border { width: 2; color: app.palette.highlight }
                onVisibleChanged: {
                    if(!visible)
                        enabled = false
                }
                z: 10

                property point from: Qt.point(0,0)
                property point to: Qt.point(0,0)
                property rect area: {
                    if(enabled)
                        return tightRect
                    if(from === to)
                        return Qt.rect(from.x, from.y, 1, 1)
                    return Qt.rect( Math.min(from.x,to.x), Math.min(from.y,to.y), Math.abs(to.x-from.x), Math.abs(to.y-from.y) )
                }
                property rect tightRect: Qt.rect(0,0,0,0)
                property point topLeft

                x: area.x
                y: area.y
                width: area.width
                height: area.height

                MouseArea {
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.XAndYAxis
                    drag.minimumX: 0
                    drag.minimumY: 0
                }

                onXChanged: shiftElements()
                onYChanged: shiftElements()

                function prepare() {
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
                    tightRect = Qt.rect(x,y,width,height)
                    topLeft = Qt.point(tightRect.x, tightRect.y)
                }

                function cleanup() {
                    for(var i=0; i<elements.length; i++) {
                        var item = elements[i]
                        item.element.x = scriteDocument.structure.snapToGrid(item.x)
                        item.element.y = scriteDocument.structure.snapToGrid(item.y)
                    }
                    elements = []
                    from = Qt.point(0,0)
                    to = Qt.point(0,0)
                    topLeft = Qt.point(0,0)
                }

                function computeTightRect() {
                    var bounds = {
                        "p1": { x: -1, y: -1 },
                        "p2": { x: -1, y: -1 },
                        "unite": function(pt) {
                            if(this.p1.x < 0 || this.p1.y < 0) {
                                this.p1.x = pt.x
                                this.p1.y = pt.y
                            } else {
                                this.p1.x = Math.min(this.p1.x, pt.x)
                                this.p1.y = Math.min(this.p1.y, pt.y)
                            }
                            if(this.p2.x < 0 || this.p2.y < 0) {
                                this.p2.x = pt.x
                                this.p2.y = pt.y
                            } else {
                                this.p2.x = Math.max(this.p2.x, pt.x)
                                this.p2.y = Math.max(this.p2.y, pt.y)
                            }

                            this.p1.x = Math.round(this.p1.x)
                            this.p2.x = Math.round(this.p2.x)
                            this.p1.y = Math.round(this.p1.y)
                            this.p2.y = Math.round(this.p2.y)
                        }
                    }

                    for(var i=0; i<elements.length; i++) {
                        var item = elements[i]
                        var p1 = Qt.point(item.x, item.y)
                        var p2 = Qt.point(item.x+item.width, item.y+item.height)
                        bounds.unite(p1)
                        bounds.unite(p2)
                    }

                    pauseShifting = true
                    tightRect = Qt.rect(bounds.p1.x-10, bounds.p1.y-10,
                                        (bounds.p2.x-bounds.p1.x+20),
                                        (bounds.p2.y-bounds.p1.y+20))
                    topLeft = Qt.point(tightRect.x, tightRect.y)
                    app.execLater(100, function() { pauseShifting=false })
                }

                onEnabledChanged: {
                    if(enabled) {
                        prepare()
                        app.execLater(100, computeTightRect)
                    } else
                        cleanup()
                }

                property var elements: []

                property bool pauseShifting: false
                function shiftElements(snapToGrid) {
                    if(!enabled || pauseShifting || elements.length === 0)
                        return

                    var i, item
                    var dx = x - topLeft.x
                    var dy = y - topLeft.y
                    topLeft = Qt.point(x,y)
                    for(i=0; i<elements.length; i++) {
                        item = elements[i].element
                        item.x = item.x + dx
                        item.y = item.y + dy
                    }
                }
            }

            BorderImage {
                property Item currentElementItem: elementItems.itemAt(canvas.currentIndex)
                source: "../icons/content/shadow.png"
                anchors.fill: currentElementItem
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
                anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
                border { left: 21; top: 21; right: 21; bottom: 21 }
                visible: currentElementItem !== null
                opacity: 0.55
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
                    x: positionBinder.get.x
                    y: positionBinder.get.y

                    DelayedPropertyBinder {
                        id: positionBinder
                        initial: Qt.point(element.x, element.y)
                        set: element.position
                        onGetChanged: {
                            elementItem.x = get.x
                            elementItem.y = get.y
                        }
                    }

                    // This happens when element is dragged
                    onXChanged: element.x = x // width/2
                    onYChanged: element.y = y // height/2

                    // This happens when text is edited
                    onWidthChanged: element.width = width
                    onHeightChanged: element.height = height

                    Keys.onPressed: {
                        if(event.key === Qt.Key_F2)
                            canvas.editIndex = index
                    }

                    Rectangle {
                        id: background
                        radius: 3
                        anchors.fill: parent
                        border.width: parent.selected ? 2 : 1
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
                                onClicked: {
                                    releaseEditor()
                                    scriteDocument.screenplay.removeSceneElements(element.scene)
                                    scriteDocument.structure.removeElement(element)
                                }
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
            text: "Create scenes by clicking on the 'Add Scene' button OR double-click while holding the " + app.polishShortcutTextForDisplay("Ctrl") + " key."
        }
    }

    Component {
        id: structureElementComponent

        StructureElement {
            scene: Scene {
                title: "New Scene"
                heading.locationType: "INT"
                heading.location: "SOMEWHERE"
                heading.moment: "DAY"
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
                    outlineWidth: app.devicePixelRatio*2

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
