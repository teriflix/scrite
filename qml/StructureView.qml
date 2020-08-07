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
    id: structureView
    signal requestEditor()
    signal releaseEditor()

    Rectangle {
        id: toolbar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 1
        color: primaryColors.c100.background
        height: toolbarLayout.height+4

        Row {
            id: toolbarLayout
            spacing: 3
            width: parent.width-4
            anchors.verticalCenter: parent.verticalCenter

            ToolButton3 {
                id: newSceneButton
                down: canvasMenu.visible
                enabled: !scriteDocument.readOnly
                onClicked: {
                    canvasMenu.isContextMenu = false
                    canvasMenu.popup()
                }
                iconSource: "../icons/content/add_box.png"
                ToolTip.text: "Add Scene"
                property color activeColor: "white"
            }

            ToolButton3 {
                down: canvasPreview.visible
                checked: canvasPreview.visible
                checkable: true
                onToggled: structureCanvasSettings.showPreview = checked
                iconSource: "../icons/action/preview.png"
                ToolTip.text: "Preview"
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                onClicked: canvasScroll.zoomIn()
                iconSource: "../icons/navigation/zoom_in.png"
                autoRepeat: true
                ToolTip.text: "Zoom In"
            }

            ToolButton3 {
                onClicked: canvasScroll.zoomOut()
                iconSource: "../icons/navigation/zoom_out.png"
                autoRepeat: true
                ToolTip.text: "Zoom Out"
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                id: selectionModeButton
                enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                iconSource: "../icons/action/selection_drag.png"
                ToolTip.text: "Selecttion mode"
                checkable: true
                onClicked: selection.layout(Structure.HorizontalLayout)
            }

            ToolButton3 {
                enabled: !scriteDocument.readOnly && scriteDocument.structure.elementCount >= 2
                iconSource: "../icons/content/select_all.png"
                ToolTip.text: "Select All"
                onClicked: selection.init(elementItems, canvasItemsBoundingBox.boundingBox)
            }

            ToolButton3 {
                enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                iconSource: "../icons/action/layout_horizontally.png"
                ToolTip.text: "Layout Horizontally"
                onClicked: selection.layout(Structure.HorizontalLayout)
            }

            ToolButton3 {
                enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                iconSource: "../icons/action/layout_vertically.png"
                ToolTip.text: "Layout Vertically"
                onClicked: selection.layout(Structure.VerticalLayout)
            }

            ToolButton3 {
                enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                iconSource: "../icons/action/layout_flow_horizontally.png"
                ToolTip.text: "Flow Horizontally"
                onClicked: selection.layout(Structure.FlowHorizontalLayout)
            }

            ToolButton3 {
                enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                iconSource: "../icons/action/layout_flow_vertically.png"
                ToolTip.text: "Flow Vertically"
                onClicked: selection.layout(Structure.FlowVerticalLayout)
            }
        }
    }

    Rectangle {
        anchors.fill: canvasScroll
        color: structureCanvasSettings.canvasColor
    }

    ScrollArea {
        id: canvasScroll
        anchors.left: parent.left
        anchors.top: toolbar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentWidth: canvas.width * canvas.scale
        contentHeight: canvas.height * canvas.scale
        initialContentWidth: canvas.width
        initialContentHeight: canvas.height
        clip: true
        showScrollBars: scriteDocument.structure.elementCount >= 1
        interactive: !(rubberBand.active || selection.active || canvasPreview.interacting) && mouseOverItem === null && editItem === null
        property Item mouseOverItem
        property Item editItem

        GridBackground {
            id: canvas
            antialiasing: false
            majorTickLineWidth: 2
            minorTickLineWidth: 1
            width: widthBinder.get
            height: heightBinder.get
            tickColorOpacity: 0.25 * scale
            scale: canvasScroll.suggestedScale
            border.width: 2
            border.color: structureCanvasSettings.gridColor
            gridIsVisible: canvasPreview.updatingThumbnail ? false : structureCanvasSettings.showGrid
            majorTickColor: structureCanvasSettings.gridColor
            minorTickColor: structureCanvasSettings.gridColor
            tickDistance: scriteDocument.structure.canvasGridSize
            transformOrigin: Item.TopLeft

            Shortcut {
                enabled: !scriteDocument.readOnly
                sequence: "Ctrl+V"
                onActivated: {
                    var gpos = app.globalMousePosition()
                    var pos = canvasScroll.mapFromGlobal(gpos.x, gpos.y)
                    if(pos.x < 0 || pos.y < 0 || pos.x >= canvasScroll.width || pos.y >= canvasScroll.height)
                        scriteDocument.structure.paste()
                    else {
                        pos = canvas.mapFromGlobal(gpos.x, gpos.y)
                        scriteDocument.structure.paste(Qt.point(pos.x,pos.y))
                    }
                }
                context: Qt.ApplicationShortcut
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Paste"
                ShortcutsModelItem.shortcut: app.polishShortcutTextForDisplay("Ctrl+V")
            }

            function createItem(what, where) {
                if(scriteDocument.readOnly)
                    return

                if(what === undefined || what === "" | what === "element")
                    createElement(where.x-130, where.y-22, newSceneButton.activeColor)
                else
                    createAnnotation(what, where.x, where.y)
            }

            function createElement(x, y, c) {
                if(scriteDocument.readOnly)
                    return

                var props = {
                    "x": Math.max(scriteDocument.structure.snapToGrid(x), 130),
                    "y": Math.max(scriteDocument.structure.snapToGrid(y), 50)
                }

                var element = newStructureElementComponent.createObject(scriteDocument.structure, props)
                element.scene.color = c
                scriteDocument.structure.addElement(element)
                scriteDocument.structure.currentElementIndex = scriteDocument.structure.elementCount-1
                requestEditor()
                element.scene.undoRedoEnabled = true
            }

            readonly property var annotationsList: [
                { "title": "Horizontal Line", "what": "hline" },
                { "title": "Vertical Line", "what": "vline" },
                { "title": "Rectangle", "what": "rectangle" },
                { "title": "Text", "what": "text" },
                { "title": "Website Link", "what": "url" },
                { "title": "Image", "what": "image" }
            ]

            function createAnnotation(type, x, y) {
                if(scriteDocument.readOnly)
                    return

                switch(type) {
                case "hline":
                    structureView.createNewLineAnnotation(x,y)
                    break
                case "vline":
                    structureView.createNewLineAnnotation(x,y, "Vertical")
                    break
                case "rectangle":
                    structureView.createNewRectangleAnnotation(x,y)
                    break
                case "text":
                    structureView.createNewTextAnnotation(x,y)
                    break
                case "url":
                    structureView.createNewUrlAnnotation(x,y)
                    break
                case "image":
                    structureView.createNewImageAnnotation(x,y)
                    break
                }
            }

            TightBoundingBoxEvaluator {
                id: canvasItemsBoundingBox
            }

            DelayedPropertyBinder {
                id: widthBinder
                initial: 1000
                set: Math.max( Math.ceil(canvasItemsBoundingBox.right / 100) * 100, 60000 )
                onGetChanged: scriteDocument.structure.canvasWidth = get
            }

            DelayedPropertyBinder {
                id: heightBinder
                initial: 1000
                set: Math.max( Math.ceil(canvasItemsBoundingBox.bottom / 100) * 100, 60000 )
                onGetChanged: scriteDocument.structure.canvasHeight = get
            }

            FocusTracker.window: qmlWindow
            FocusTracker.indicator.target: mainUndoStack
            FocusTracker.indicator.property: "structureEditorActive"

            MouseArea {
                id: createItemMouseHandler
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                enabled: false
                cursorShape: Qt.CrossCursor

                property string what
                function handle(_what) {
                    what = _what
                    enabled = true
                }

                onClicked: {
                    if(!scriteDocument.readOnly)
                        canvas.createItem(what, Qt.point(mouse.x, mouse.y))
                    what = ""
                    enabled = false
                }
            }

            Item {
                id: annotationsLayer
                anchors.fill: parent
                enabled: !createItemMouseHandler.enabled && opacity === 1
                opacity: rubberBand.selectionMode || rubberBand.selecting ? 0.1 : 1

                Behavior on opacity {
                    enabled: screenplayEditorSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: annotationGripLoader.active
                    onClicked: annotationGripLoader.reset()
                }

                Repeater {
                    id: annotationItems
                    model: scriteDocument.loading ? 0 : scriteDocument.structure.annotations
                    delegate: Loader {
                        property Annotation annotation: modelData
                        property int annotationIndex: index
                        sourceComponent: {
                            switch(annotation.type) {
                            case "rectangle": return rectangleAnnotationComponent
                            case "text": return textAnnotationComponent
                            case "url": return urlAnnotationComponent
                            case "image": return imageAnnotationComponent
                            case "line": return lineAnnotationComponent
                            }
                            return null
                        }
                    }
                }

                Loader {
                    id: annotationGripLoader
                    property Item annotationItem
                    property Annotation annotation
                    sourceComponent: annotationGripComponent
                    active: annotation !== null
                    onActiveChanged: {
                        if(!active)
                            floatingDockWidget.hide()
                    }

                    Component.onDestruction: reset()

                    function reset() {
                        floatingDockWidget.hide()
                        annotation = null
                        annotationItem = null
                    }

                    onAnnotationChanged: {
                        if(annotation === null)
                            floatingDockWidget.hide()
                        else {
                            if(floatingDockWidget.contentX < 0) {
                                floatingDockWidget.contentX = documentUI.mapFromItem(structureView, 0, 0).x + structureView.width + 40
                                floatingDockWidget.contentY = (documentUI.height - floatingDockWidget.contentHeight)/2
                            }

                            floatingDockWidget.display("Annotation Properties", annotationPropertyEditorComponent)
                        }
                    }

                    Connections {
                        target: createItemMouseHandler
                        onEnabledChanged: {
                            if(canvasScroll.enabled)
                                annotationGripLoader.reset()
                        }
                    }

                    Connections {
                        target: canvasScroll
                        onEditItemChanged: {
                            if(canvasScroll.editItem !== null)
                                annotationGripLoader.reset()
                        }
                    }

                    Connections {
                        target: canvasMenu
                        onVisibleChanged: {
                            if(canvasMenu.visible)
                                annotationGripLoader.reset()
                        }
                    }

                    Connections {
                        target: scriteDocument.structure
                        onCurrentElementIndexChanged: {
                            if(scriteDocument.structure.currentElementIndex >= 0)
                                annotationGripLoader.reset()
                        }
                        onAnnotationCountChanged: annotationGripLoader.reset()
                    }
                }
            }

            RubberBand {
                id: rubberBand
                enabled: !createItemMouseHandler.enabled
                anchors.fill: parent
                z: active ? 1000 : -1
                selectionMode: selectionModeButton.checked
                onTryStart: {
                    parent.forceActiveFocus()
                    active = true // TODO
                }
                onSelect: {
                    selection.init(elementItems, rectangle)
                    selectionModeButton.checked = false
                }
            }

            BoxShadow {
                anchors.fill: currentElementItem
                visible: currentElementItem !== null && !annotationGripLoader.active
                property Item currentElementItem: currentElementItemBinder.get
                onCurrentElementItemChanged: canvasScroll.ensureItemVisible(currentElementItem, canvas.scale)
                opacity: canvas.activeFocus && !selection.hasItems ? 1 : 0.25

                Behavior on opacity {
                    enabled: screenplayEditorSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }

                DelayedPropertyBinder {
                    id: currentElementItemBinder
                    initial: null
                    set: elementItems.count > scriteDocument.structure.currentElementIndex ? elementItems.itemAt(scriteDocument.structure.currentElementIndex) : null
                }
            }

            Repeater {
                id: elementConnectorItems
                model: scriteDocument.loading ? 0 : scriteDocument.structureElementSequence
                delegate: elementConnectorComponent
            }

            MouseArea {
                anchors.fill: parent
                enabled: canvasScroll.editItem !== null
                acceptedButtons: Qt.LeftButton
                onClicked: canvasScroll.editItem.finishEditing()
            }

            MouseArea {
                anchors.fill: parent
                enabled: canvasScroll.editItem === null && !selection.active
                acceptedButtons: Qt.RightButton
                onPressed: {
                    canvasMenu.isContextMenu = true
                    canvasMenu.popup()
                }
            }

            Repeater {
                id: elementItems
                model: scriteDocument.loading ? 0 : scriteDocument.structure.elements
                delegate: structureElementDelegate
            }

            Selection {
                id: selection
                anchors.fill: parent
                interactive: !scriteDocument.readOnly
                onMoveItem: {
                    item.x = item.x + dx
                    item.y = item.y + dy
                }
                onPlaceItem: {
                    item.x = scriteDocument.structure.snapToGrid(item.x)
                    item.y = scriteDocument.structure.snapToGrid(item.y)
                }

                contextMenu: Menu2 {
                    id: selectionContextMenu
                    width: 250

                    ColorMenu {
                        title: "Scenes Color"
                        onMenuItemClicked: {
                            var items = selection.items
                            items.forEach( function(item) {
                                item.element.scene.color = color
                            })
                            selectionContextMenu.close()
                        }
                    }

                    Menu2 {
                        title: "Mark Scenes As"

                        Repeater {
                            model: app.enumerationModelForType("Scene", "Type")

                            MenuItem2 {
                                text: modelData.key
                                onTriggered: {
                                    var items = selection.items
                                    items.forEach( function(item) {
                                        item.element.scene.type = modelData.value
                                    })
                                    selectionContextMenu.close()
                                }
                            }
                        }
                    }

                    Menu2 {
                        title: "Layout"

                        MenuItem2 {
                            enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                            icon.source: "../icons/action/layout_horizontally.png"
                            text: "Layout Horizontally"
                            onClicked: selection.layout(Structure.HorizontalLayout)
                        }

                        MenuItem2 {
                            enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                            icon.source: "../icons/action/layout_vertically.png"
                            text: "Layout Vertically"
                            onClicked: selection.layout(Structure.VerticalLayout)
                        }

                        MenuItem2 {
                            enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                            icon.source: "../icons/action/layout_flow_horizontally.png"
                            text: "Flow Horizontally"
                            onClicked: selection.layout(Structure.FlowHorizontalLayout)
                        }

                        MenuItem2 {
                            enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                            icon.source: "../icons/action/layout_flow_vertically.png"
                            text: "Flow Vertically"
                            onClicked: selection.layout(Structure.FlowVerticalLayout)
                        }
                    }

                    MenuItem2 {
                        text: "Annotate With Rectangle"
                        onClicked: {
                            createNewRectangleAnnotation(selection.rect.x-10, selection.rect.y-10, selection.rect.width+20, selection.rect.height+20)
                            selection.clear()
                        }
                    }
                }

                function layout(type) {
                    if(scriteDocument.readOnly)
                        return

                    if(!hasItems) {
                        scriteDocument.structure.layoutElements(type)
                        return
                    }

                    if(!canLayout)
                        return

                    layoutAnimation.layoutType = type
                    layoutAnimation.start()
                }

                SequentialAnimation {
                    id: layoutAnimation

                    property var layoutType: -1
                    property var layoutItems: []
                    property var layoutItemBounds
                    running: false

                    ScriptAction {
                        script: {
                            layoutAnimation.layoutItems = selection.items
                            selection.clear()
                        }
                    }

                    PauseAnimation {
                        duration: 50
                    }

                    ScriptAction {
                        script: {
                            var oldItems = layoutAnimation.layoutItems
                            layoutAnimation.layoutItems = []
                            oldItems.forEach( function(item) {
                                item.element.selected = true
                            })
                            layoutAnimation.layoutItemBounds = scriteDocument.structure.layoutElements(layoutAnimation.layoutType)
                            layoutAnimation.layoutType = -1
                            oldItems.forEach( function(item) {
                                item.element.selected = false
                            })
                        }
                    }

                    PauseAnimation {
                        duration: 50
                    }

                    ScriptAction {
                        script: {
                            var rect = {
                                "top": layoutAnimation.layoutItemBounds.top,
                                "left": layoutAnimation.layoutItemBounds.left,
                                "right": layoutAnimation.layoutItemBounds.left + layoutAnimation.layoutItemBounds.width-1,
                                "bottom": layoutAnimation.layoutItemBounds.top + layoutAnimation.layoutItemBounds.height-1
                            };
                            layoutAnimation.layoutItemBounds = undefined
                            selection.init(elementItems, rect)
                        }
                    }
                }
            }

            Menu2 {
                id: canvasMenu

                property bool isContextMenu: false

                MenuItem2 {
                    text: "New Scene"
                    enabled: !scriteDocument.readOnly
                    onClicked: {
                        if(canvasMenu.isContextMenu)
                            canvas.createItem("element", Qt.point(canvasMenu.x-130,canvasMenu.y-22), newSceneButton.activeColor)
                        else
                            createItemMouseHandler.handle("element")
                    }
                }

                ColorMenu {
                    title: "Colored Scene"
                    selectedColor: newSceneButton.activeColor
                    enabled: !scriteDocument.readOnly
                    onMenuItemClicked: {
                        newSceneButton.activeColor = color
                        if(canvasMenu.isContextMenu)
                            canvas.createItem("element", Qt.point(canvasMenu.x-130,canvasMenu.y-22), newSceneButton.activeColor)
                        else
                            createItemMouseHandler.handle("element")
                        canvasMenu.close()
                    }
                }

                MenuSeparator { }

                Menu2 {
                    title: "Annotation"

                    Repeater {
                        model: canvas.annotationsList

                        MenuItem2 {
                            property var annotationInfo: canvas.annotationsList[index]
                            text: annotationInfo.title
                            enabled: !scriteDocument.readOnly && annotationInfo.what !== ""
                            onClicked: {
                                if(canvasMenu.isContextMenu)
                                    canvas.createItem(annotationInfo.what, Qt.point(canvasMenu.x, canvasMenu.y))
                                else
                                    createItemMouseHandler.handle(annotationInfo.what)
                            }
                        }
                    }
                }
            }

            Menu2 {
                id: elementContextMenu
                property StructureElement element
                onElementChanged: {
                    if(element)
                        popup()
                    else
                        close()
                }

                MenuItem2 {
                    action: Action {
                        text: "Scene Heading"
                        checkable: true
                        checked: elementContextMenu.element ? elementContextMenu.element.scene.heading.enabled : false
                    }
                    enabled: elementContextMenu.element !== null
                    onTriggered: {
                        elementContextMenu.element.scene.heading.enabled = action.checked
                        elementContextMenu.element = null
                    }
                }

                ColorMenu {
                    title: "Color"
                    enabled: elementContextMenu.element !== null
                    onMenuItemClicked: {
                        elementContextMenu.element.scene.color = color
                        elementContextMenu.element = null
                    }
                }

                Menu2 {
                    title: "Mark Scene As"

                    Repeater {
                        model: elementContextMenu.element ? app.enumerationModelForType("Scene", "Type") : 0

                        MenuItem2 {
                            text: modelData.key
                            font.bold: elementContextMenu.element.scene.type === modelData.value
                            onTriggered: {
                                elementContextMenu.element.scene.type = modelData.value
                                elementContextMenu.element = null
                            }
                        }
                    }
                }

                MenuItem2 {
                    text: "Duplicate"
                    enabled: elementContextMenu.element !== null
                    onClicked: {
                        releaseEditor()
                        elementContextMenu.element.duplicate()
                        elementContextMenu.element = null
                    }
                }

                MenuSeparator { }

                MenuItem2 {
                    text: "Delete"
                    enabled: elementContextMenu.element !== null
                    onClicked: {
                        releaseEditor()
                        scriteDocument.screenplay.removeSceneElements(elementContextMenu.element.scene)
                        scriteDocument.structure.removeElement(elementContextMenu.element)
                        elementContextMenu.element = null
                    }
                }
            }
        }
    }

    Item {
        id: canvasPreview
        visible: structureCanvasSettings.showPreview
        anchors.right: canvasScroll.right
        anchors.bottom: canvasScroll.bottom
        anchors.margins: 30

        readonly property real maxSize: 150
        property size previewSize: {
            var w = Math.max(canvasItemsBoundingBox.width, 500)
            var h = Math.max(canvasItemsBoundingBox.height, 500)

            var scale = 1
            if(w < h)
                scale = maxSize / w
            else
                scale = maxSize / h

            w *= scale
            h *= scale

            if(w > parent.width-60)
                scale = (parent.width-60)/w
            else if(h >= parent.height-60)
                scale = (parent.height-60)/h
            else
                scale = 1

            w *= scale
            h *= scale

            return Qt.size(w+10, h+10)
        }
        width: previewSize.width
        height: previewSize.height

        BoxShadow {
            anchors.fill: parent
            opacity: 0.55 * previewArea.opacity
        }

        TightBoundingBoxPreview {
            id: previewArea
            anchors.fill: parent
            anchors.margins: 5
            evaluator: canvasItemsBoundingBox
            backgroundColor: primaryColors.c50.background
            backgroundOpacity: 0.25

            Rectangle {
                id: viewportIndicator
                color: primaryColors.highlight.background
                opacity: 0.5

                property rect geometry: {
                    if(!canvasPreview.visible)
                        return Qt.rect(0,0,0,0)

                    var visibleRect = Qt.rect( canvasScroll.visibleArea.xPosition * canvasScroll.contentWidth / canvas.scale,
                                               canvasScroll.visibleArea.yPosition * canvasScroll.contentHeight / canvas.scale,
                                               canvasScroll.visibleArea.widthRatio * canvasScroll.contentWidth / canvas.scale,
                                               canvasScroll.visibleArea.heightRatio * canvasScroll.contentHeight / canvas.scale )
                    if( app.isRectangleInRectangle(visibleRect,canvasItemsBoundingBox.boundingBox) )
                        return Qt.rect(0,0,0,0)

                    var intersect = app.intersectedRectangle(visibleRect, canvasItemsBoundingBox.boundingBox)
                    var scale = previewArea.width / Math.max(canvasItemsBoundingBox.width, 500)
                    var ret = Qt.rect( (intersect.x-canvasItemsBoundingBox.left)*scale,
                                       (intersect.y-canvasItemsBoundingBox.top)*scale,
                                       (intersect.width*scale),
                                       (intersect.height*scale) )
                    return ret
                }
                x: geometry.x
                y: geometry.y
                width: geometry.width
                height: geometry.height
            }

            Item {
                x: viewportIndicator.x
                y: viewportIndicator.y
                width: viewportIndicator.width
                height: viewportIndicator.height

                onXChanged: panViewport()
                onYChanged: panViewport()

                function panViewport() {
                    if(!panMouseArea.drag.active)
                        return

                    var scale = previewArea.width / Math.max(canvasItemsBoundingBox.width, 500)
                    var ix = (x/scale)+canvasItemsBoundingBox.left
                    var iy = (y/scale)+canvasItemsBoundingBox.top
                    canvasScroll.contentX = ix * canvas.scale
                    canvasScroll.contentY = iy * canvas.scale
                }

                MouseArea {
                    id: panMouseArea
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.XAndYAxis
                    drag.minimumX: 0
                    drag.minimumY: 0
                    drag.maximumX: previewArea.width - parent.width
                    drag.maximumY: previewArea.height - parent.height
                    enabled: parent.width > 0 && parent.height > 0
                    hoverEnabled: drag.active
                    cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                }
            }
        }
    }

    Loader {
        width: parent.width*0.7
        anchors.centerIn: parent
        active: scriteDocument.structure.elementCount === 0 && scriteDocument.structure.annotationCount === 0
        sourceComponent: TextArea {
            readOnly: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 30
            enabled: false
            // renderType: Text.NativeRendering
            text: "Create scenes by clicking on the 'Add Scene' button OR right click to see options."
        }
    }

    Component {
        id: newStructureElementComponent

        StructureElement {
            objectName: "newElement"
            scene: Scene {
                title: "New Scene"
                heading.locationType: "INT"
                heading.location: "SOMEWHERE"
                heading.moment: "DAY"
            }
        }
    }

    Component {
        id: structureElementDelegate

        Item {
            id: elementItem
            property StructureElement element: modelData
            Component.onCompleted: element.follow = elementItem
            enabled: selection.active === false

            TightBoundingBoxItem.evaluator: canvasItemsBoundingBox
            TightBoundingBoxItem.stackOrder: 2.0 + (index/scriteDocument.structure.elementCount)
            TightBoundingBoxItem.livePreview: false
            TightBoundingBoxItem.previewFillColor: background.color
            TightBoundingBoxItem.previewBorderColor: selected ? "black" : background.border.color

            readonly property bool selected: scriteDocument.structure.currentElementIndex === index
            readonly property bool editing: titleText.readOnly === false
            onEditingChanged: {
                if(editing)
                    canvasScroll.editItem = elementItem
                else if(canvasScroll.editItem === elementItem)
                    canvasScroll.editItem = null
            }

            function finishEditing() {
                titleText.editMode = false
                element.objectName = "oldElement"
            }

            x: positionBinder.get.x
            y: positionBinder.get.y
            width: titleText.width + 10
            height: titleText.height + 10

            DelayedPropertyBinder {
                id: positionBinder
                initial: Qt.point(element.x, element.y)
                set: element.position
                onGetChanged: {
                    elementItem.x = get.x
                    elementItem.y = get.y
                }
            }

            Rectangle {
                id: background
                anchors.fill: parent
                border.width: elementItem.selected ? 2 : 1
                border.color: (element.scene.color === Qt.rgba(1,1,1,1) ? "gray" : element.scene.color)
                color: Qt.tint(element.scene.color, "#C0FFFFFF")
                Behavior on border.width {
                    enabled: screenplayEditorSettings.enableAnimations
                    NumberAnimation { duration: 400 }
                }
            }

            TextViewEdit {
                id: titleText
                width: 250
                wrapMode: Text.WordWrap
                text: element.scene.title
                anchors.centerIn: parent
                font.pointSize: 13
                horizontalAlignment: Text.AlignHCenter
                onTextEdited: element.scene.title = text
                onEditingFinished: {
                    editMode = false
                    element.objectName = "oldElement"
                }
                onHighlightRequest: scriteDocument.structure.currentElementIndex = index
                Keys.onReturnPressed: editingFinished()
                property bool editMode: element.objectName === "newElement"
                readOnly: !(editMode && index === scriteDocument.structure.currentElementIndex)
                leftPadding: 17
                rightPadding: 17
                topPadding: 5
                bottomPadding: 5
            }

            QtObject {
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Copy Scene"
                ShortcutsModelItem.visible: elementItem.EventFilter.active
                ShortcutsModelItem.shortcut: app.polishShortcutTextForDisplay("Ctrl+C")
            }

            EventFilter.target: app
            EventFilter.active: !scriteDocument.readOnly && canvas.activeFocus && selected && !selection.hasItems
            EventFilter.events: [6]
            EventFilter.onFilter: {
                var dist = (event.controlModifier ? 5 : 1) * canvas.tickDistance
                switch(event.key) {
                case Qt.Key_Left:
                    element.x -= dist
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_Right:
                    element.x += dist
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_Up:
                    element.y -= dist
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_Down:
                    element.y += dist
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_C:
                    if(event.controlModifier) {
                        scriteDocument.structure.copy(element)
                        statusText.show("Element Copied")
                        result.accept = true
                        result.filter = true
                    }
                    break
                }
            }

            MouseArea {
                anchors.fill: titleText
                enabled: titleText.readOnly === true
                onPressedChanged: {
                    if(pressed) {
                        canvasScroll.mouseOverItem = elementItem
                        scriteDocument.structure.currentElementIndex = index
                    } else if(canvasScroll.mouseOverItem === elementItem)
                        canvasScroll.mouseOverItem = null
                }
                acceptedButtons: Qt.LeftButton
                onDoubleClicked: {
                    annotationGripLoader.reset()
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    if(!scriteDocument.readOnly)
                        titleText.editMode = true
                }
                onClicked: {
                    annotationGripLoader.reset()
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    requestEditor()
                }

                drag.target: scriteDocument.readOnly ? null : elementItem
                drag.axis: Drag.XAndYAxis
                drag.minimumX: 0
                drag.minimumY: 0
                drag.onActiveChanged: {
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    if(drag.active === false) {
                        elementItem.x = scriteDocument.structure.snapToGrid(parent.x)
                        elementItem.y = scriteDocument.structure.snapToGrid(parent.y)
                    }
                }
            }

            Keys.onPressed: {
                if(event.key === Qt.Key_F2) {
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    if(!scriteDocument.readOnly)
                        titleText.editMode = true
                    event.accepted = true
                } else
                    event.accepted = false
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    elementContextMenu.element = elementItem.element
                }
            }

            // Drag to timeline support
            Drag.active: dragMouseArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.LinkAction
            Drag.hotSpot.x: dragHandle.x + dragHandle.width/2
            Drag.hotSpot.y: dragHandle.y + dragHandle.height/2
            Drag.mimeData: {
                "scrite/sceneID": element.scene.id
            }
            Drag.source: element.scene

            SceneTypeImage {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 3
                width: 24; height: 24
                opacity: 0.5
                showTooltip: false
                sceneType: elementItem.element.scene.type
            }

            Image {
                id: dragHandle
                visible: !parent.editing && !scriteDocument.readOnly
                enabled: canvasScroll.editItem === null && !scriteDocument.readOnly
                source: "../icons/action/view_array.png"
                width: 24; height: 24
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 1
                anchors.rightMargin: 3
                opacity: dragMouseArea.containsMouse ? 1 : 0.1
                scale: dragMouseArea.containsMouse ? 2 : 1
                Behavior on scale {
                    enabled: screenplayEditorSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }

                MouseArea {
                    id: dragMouseArea
                    hoverEnabled: true
                    anchors.fill: parent
                    drag.target: parent
                    cursorShape: Qt.SizeAllCursor
                    onPressed: {
                        canvas.forceActiveFocus()
                        elementItem.grabToImage(function(result) {
                            elementItem.Drag.imageSource = result.url
                        })
                    }
                }
            }
        }
    }

    Component {
        id: elementConnectorComponent

        StructureElementConnector {
            lineType: StructureElementConnector.CurvedLine
            fromElement: scriteDocument.structure.elementAt(modelData.from)
            toElement: scriteDocument.structure.elementAt(modelData.to)
            arrowAndLabelSpacing: labelBg.width
            outlineWidth: canvasPreview.updatingThumbnail ? 0.1 : app.devicePixelRatio*canvas.scale

            Rectangle {
                id: labelBg
                width: Math.max(label.width,label.height)+20
                height: width; radius: width/2
                border.width: 1; border.color: primaryColors.borderColor
                x: parent.suggestedLabelPosition.x - radius
                y: parent.suggestedLabelPosition.y - radius
                color: Qt.tint(parent.outlineColor, "#E0FFFFFF")
                visible: !canvasPreview.updatingThumbnail

                Text {
                    id: label
                    anchors.centerIn: parent
                    font.pixelSize: 12
                    text: (index+1)
                }
            }
        }
    }

    // Template annotation component
    Component {
        id: annotationObject

        Annotation {
            objectName: "ica" // interactively created annotation
        }
    }

    Component {
        id: annotationGripComponent

        Item {
            id: annotationGripItem
            x: annotationItem.x
            y: annotationItem.y
            width: annotationItem.width
            height: annotationItem.height
            enabled: !scriteDocument.readOnly
            readonly property int geometryUpdateInterval: 50

            property real gripSize: 10 * onePxSize
            property real onePxSize: Math.max(1, 1/canvas.scale)

            PainterPathItem {
                id: focusIndicator
                anchors.fill: parent
                anchors.margins: -gripSize/2
                renderType: PainterPathItem.OutlineOnly
                renderingMechanism: PainterPathItem.UseQPainter
                outlineWidth: onePxSize
                outlineColor: accentColors.a700.background
                outlineStyle: PainterPathItem.DashDotDotLine
                painterPath: PainterPath {
                    MoveTo { x: onePxSize; y: onePxSize }
                    LineTo { x: focusIndicator.width-onePxSize; y: onePxSize }
                    LineTo { x: focusIndicator.width-onePxSize; y: focusIndicator.height-onePxSize }
                    LineTo { x: onePxSize; y: focusIndicator.height-onePxSize }
                    CloseSubpath { }
                }
            }

            QtObject {
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Copy Annotation"
                ShortcutsModelItem.visible: annotationGripItem.EventFilter.active
                ShortcutsModelItem.shortcut: app.polishShortcutTextForDisplay("Ctrl+C")
            }

            EventFilter.target: app
            EventFilter.active: !scriteDocument.readOnly && !floatingDockWidget.contentHasFocus
            EventFilter.events: [6]
            EventFilter.onFilter: {
                var dist = (event.controlModifier ? 5 : 1) * canvas.tickDistance
                switch(event.key) {
                case Qt.Key_Left:
                    if(event.shiftModifier)
                        annotationGripItem.width -= annotation.resizable ? dist : 0
                    else
                        annotationGripItem.x -= annotation.movable ? dist : 0
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_Right:
                    if(event.shiftModifier)
                        annotationGripItem.width += annotation.resizable ? dist : 0
                    else
                        annotationGripItem.x += annotation.movable ? dist : 0
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_Up:
                    if(event.shiftModifier)
                        annotationGripItem.height -= annotation.resizable ? dist : 0
                    else
                        annotationGripItem.y -= annotation.movable ? dist : 0
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_Down:
                    if(event.shiftModifier)
                        annotationGripItem.height += annotation.resizable ? dist : 0
                    else
                        annotationGripItem.y += annotation.movable ? dist : 0
                    result.accept = true
                    result.filter = true
                    break
                case Qt.Key_C:
                    if(event.controlModifier) {
                        scriteDocument.structure.copy(annotation)
                        statusText.show("Annotation Copied")
                        result.accept = true
                        result.filter = true
                    }
                }
            }

            function deleteAnnotation() {
                var a = annotationGripLoader.annotation
                annotationGripLoader.reset()
                scriteDocument.structure.removeAnnotation(a)
            }

            onXChanged: annotGeoUpdateTimer.start()
            onYChanged: annotGeoUpdateTimer.start()
            onWidthChanged: annotGeoUpdateTimer.start()
            onHeightChanged: annotGeoUpdateTimer.start()

            Timer {
                id: annotGeoUpdateTimer
                interval: geometryUpdateInterval
                onTriggered: {
                    annotation.geometry = Qt.rect(annotationGripItem.x, annotationGripItem.y, annotationGripItem.width, annotationGripItem.height)
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton|Qt.RightButton
                cursorShape: Qt.SizeAllCursor
                drag.target: annotationItem
                drag.minimumX: 0
                drag.minimumY: 0
                drag.axis: Drag.XAndYAxis
                enabled: annotation.movable
                propagateComposedEvents: true
            }

            Rectangle {
                id: rightGrip
                width: gripSize
                height: gripSize
                color: accentColors.a700.background
                x: parent.width - width/2
                y: (parent.height - height)/2
                visible: annotation.resizable
                enabled: visible

                onXChanged: widthUpdateTimer.start()

                Timer {
                    id: widthUpdateTimer
                    interval: geometryUpdateInterval
                    onTriggered: {
                        annotation.geometry = Qt.rect(annotationGripItem.x, annotationGripItem.y, rightGrip.x + rightGrip.width/2, annotationGripItem.height)
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeHorCursor
                    drag.target: parent
                    drag.axis: Drag.XAxis
                }
            }

            Rectangle {
                id: bottomGrip
                width: gripSize
                height: gripSize
                color: accentColors.a700.background
                x: (parent.width - width)/2
                y: parent.height - height/2
                visible: annotation.resizable
                enabled: visible

                onYChanged: heightUpdateTimer.start()

                Timer {
                    id: heightUpdateTimer
                    interval: geometryUpdateInterval
                    onTriggered: {
                        annotation.geometry = Qt.rect(annotationGripItem.x, annotationGripItem.y, annotationGripItem.width, bottomGrip.y + bottomGrip.height/2)
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeVerCursor
                    drag.target: parent
                    drag.axis: Drag.YAxis
                }
            }

            Rectangle {
                id: bottomRightGrip
                width: gripSize
                height: gripSize
                color: accentColors.a700.background
                x: parent.width - width/2
                y: parent.height - height/2
                visible: annotation.resizable
                enabled: visible

                onXChanged: sizeUpdateTimer.start()
                onYChanged: sizeUpdateTimer.start()

                Timer {
                    id: sizeUpdateTimer
                    interval: geometryUpdateInterval
                    onTriggered: {
                        annotation.geometry = Qt.rect(annotationGripItem.x, annotationGripItem.y, bottomRightGrip.x + bottomRightGrip.width/2, bottomRightGrip.y + bottomRightGrip.height/2)
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeFDiagCursor
                    drag.target: parent
                    drag.axis: Drag.XAndYAxis
                }
            }
        }
    }

    Component {
        id: annotationPropertyEditorComponent

        AnnotationPropertyEditor {
            annotation: annotationGripLoader.annotation
        }
    }

    function createNewRectangleAnnotation(x, y, w, h) {
        if(scriteDocument.readOnly)
            return

        var doNotAlignRect = w && h

        w = w ? w : 200
        h = h ? h : 200
        var rect = doNotAlignRect ? Qt.rect(x, y, w, h) : Qt.rect(x - w/2, y-h/2, w, h)
        var annot = annotationObject.createObject(canvas)
        annot.type = "rectangle"
        annot.geometry = rect
        scriteDocument.structure.addAnnotation(annot)
    }

    Component {
        id: rectangleAnnotationComponent

        AnnotationItem {
            TightBoundingBoxItem.previewFillColor: color
            TightBoundingBoxItem.previewBorderColor: border.color
            TightBoundingBoxItem.livePreview: false
        }
    }

    function createNewTextAnnotation(x, y) {
        if(scriteDocument.readOnly)
            return

        var w = 200
        var h = 40

        var annot = annotationObject.createObject(canvas)
        annot.type = "text"
        annot.geometry = Qt.rect(x-w/2, y-h/2, w, h)
        scriteDocument.structure.addAnnotation(annot)
    }

    Component {
        id: textAnnotationComponent

        AnnotationItem {
            Text {
                anchors.centerIn: parent
                horizontalAlignment: {
                    switch(annotation.attributes.hAlign) {
                    case "left": return Text.AlignLeft
                    case "right": return Text.AlignRight
                    }
                    return Text.AlignHCenter
                }
                verticalAlignment: {
                    switch(annotation.attributes.vAlign) {
                    case "top": return Text.AlignTop
                    case "bottom": return Text.AlignBottom
                    }
                    return Text.AlignVCenter
                }
                text: annotation.attributes.text
                color: annotation.attributes.textColor
                font.family: annotation.attributes.fontFamily
                font.pointSize: annotation.attributes.fontSize
                font.bold: annotation.attributes.fontStyle.indexOf('bold') >= 0
                font.italic: annotation.attributes.fontStyle.indexOf('italic') >= 0
                font.underline: annotation.attributes.fontStyle.indexOf('underline') >= 0
                width: parent.width - 15
                height: parent.height - 15
                clip: true
                wrapMode: Text.WordWrap
            }
        }
    }

    function createNewUrlAnnotation(x, y) {
        if(scriteDocument.readOnly)
            return

        var w = 300
        var h = 350 // app.isMacOSPlatform ? 60 : 350

        var annot = annotationObject.createObject(canvas)
        annot.type = "url"
        annot.geometry = Qt.rect(x-w/2, y-20, w, h)
        scriteDocument.structure.addAnnotation(annot)
    }

    Component {
        id: urlAnnotationComponent

        AnnotationItem {
            id: urlAnnotItem
            color: primaryColors.c50.background
            border {
                width: 1
                color: primaryColors.borderColor
            }
            opacity: 1
            property bool annotationHasLocalImage: annotation.attributes.imageName !== undefined && annotation.attributes.imageName !== ""
            Component.onCompleted: annotation.resizable = false

            UrlAttributes {
                id: urlAttribs
                url: annotation.attributes.url2 !== annotation.attributes.url ? annotation.attributes.url : ""
                onUrlChanged: {
                    if(url !== "") {
                        var annotAttrs = annotation.attributes
                        annotation.removeImage(annotAttrs.imageName)
                        annotAttrs.imageName = ""
                        annotation.attributes = annotAttrs
                    }
                }
                onStatusChanged: {
                    if(status === UrlAttributes.Ready && url !== "") {
                        var annotAttrs = annotation.attributes
                        var urlAttrs = attributes
                        if(annotAttrs.title === "")
                            annotAttrs.title = urlAttrs.title
                        if(annotAttrs.description === "")
                            annotAttrs.description = urlAttrs.description
                        if(annotAttrs.imageUrl === "") {
                            annotAttrs.imageName = ""
                            annotAttrs.imageUrl = urlAttrs.image
                        }
                        annotAttrs.url2 = annotAttrs.url
                        annotation.attributes = annotAttrs
                    }
                }
            }

            Loader {
                anchors.fill: parent
                anchors.margins: 8
                active: annotation.attributes.url !== ""
                clip: true
                sourceComponent: Column {
                    spacing: 8

                    Rectangle {
                        width: parent.width
                        height: (width/16)*9
                        color: annotationHasLocalImage ? Qt.rgba(0,0,0,0) : primaryColors.c500.background

                        Image {
                            id: imageItem
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                            source: {
                                if(annotationHasLocalImage)
                                    annotation.imageUrl(annotation.attributes.imageName)
                                // Lets avoid using HTTPS for as long as possible
                                // Want to avoid having to bundle OpenSSL with Scrite.
                                return app.toHttpUrl(annotation.attributes.imageUrl)
                            }
                            onStatusChanged: {
                                if(status === Image.Ready) {
                                    if(!annotationHasLocalImage) {
                                        imageItem.grabToImage(function(result) {
                                            var attrs = annotation.attributes
                                            attrs.imageName = annotation.addImage(result.image)
                                            annotation.attributes = attrs
                                        })
                                    }
                                }
                            }
                        }
                    }

                    Text {
                        font.bold: true
                        font.pointSize: app.idealFontPointSize + 2
                        text: annotation.attributes.title
                        width: parent.width
                        maximumLineCount: 2
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                    }

                    Text {
                        font.pointSize: app.idealFontPointSize
                        text: annotation.attributes.description
                        width: parent.width
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        maximumLineCount: 4
                    }

                    Text {
                        font.pointSize: app.idealFontPointSize - 2
                        color: urlAttribs.status === UrlAttributes.Error ? "red" : "blue"
                        text: annotation.attributes.url
                        width: parent.width
                        elide: Text.ElideRight
                        font.underline: urlMouseArea.containsMouse

                        MouseArea {
                            id: urlMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            enabled: urlAttribs.status !== UrlAttributes.Error
                            onClicked: Qt.openUrlExternally(annotation.attributes.url)
                        }
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: urlAttribs.status === UrlAttributes.Loading
            }

            Text {
                anchors.fill: parent
                anchors.margins: 10
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: app.idealFontPointSize
                text: app.isMacOSPlatform && annotationGripLoader.annotationItem !== urlAnnotItem ? "Set a URL to get a clickable link here." : "Set a URL to preview it here."
                visible: annotation.attributes.url === ""
            }
        }
    }

    function createNewImageAnnotation(x, y) {
        if(scriteDocument.readOnly)
            return

        var w = 300
        var h = 160

        var annot = annotationObject.createObject(canvas)
        annot.type = "image"
        annot.geometry = Qt.rect(x-w/2, y-h/2, w, h)
        scriteDocument.structure.addAnnotation(annot)
    }

    Component {
        id: imageAnnotationComponent

        AnnotationItem {
            id: imageAnnotItem
            clip: true
            color: image.isSet ? (annotation.attributes.fillBackground ? annotation.attributes.backgroundColor : Qt.rgba(0,0,0,0)) : primaryColors.c100.background

            Image {
                id: image
                property bool isSet: annotation.attributes.image !== "" && status === Image.Ready
                width: parent.width - 10
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
                smooth: canvasScroll.moving || canvasScroll.flicking ? false : true
                mipmap: smooth
                source: annotation.imageUrl(annotation.attributes.image)
                asynchronous: true
                onStatusChanged: {
                    if(status === Image.Ready)
                        parent.TightBoundingBoxItem.markPreviewDirty()
                }
            }

            Text {
                width: image.width
                height: Math.max(parent.height - image.height - 10, 0)
                visible: height > 0
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                font.pointSize: app.idealFontPointSize
                text: image.isSet ? annotation.attributes.caption : (annotationGripLoader.annotationItem === imageAnnotItem ? "Set an image" : "Click to set an image")
                color: annotation.attributes.captionColor
                anchors.top: image.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 5
                horizontalAlignment: {
                    switch(annotation.attributes.captionAlignment) {
                    case "left": return Text.AlignLeft
                    case "right": return Text.AlignRight
                    }
                    return Text.AlignHCenter
                }
                verticalAlignment: image.isSet ? Text.AlignTop : Text.AlignVCenter
            }
        }
    }

    function createNewLineAnnotation(x, y, orientation) {
        if(scriteDocument.readOnly)
            return

        var w = 300
        var h = 20

        var annot = annotationObject.createObject(canvas)
        annot.type = "line"
        annot.geometry = Qt.rect(x, y, 300, 20)
        var attrs = annot.attributes
        if(orientation && orientation === "Vertical") {
            attrs.orientation = orientation
            annot.attributes = attrs
            w = 20
            h = 300
        }
        annot.geometry = Qt.rect(x-w/2, y-h/2, w, h)
        scriteDocument.structure.addAnnotation(annot)
    }

    Component {
        id: lineAnnotationComponent

        AnnotationItem {
            color: Qt.rgba(0,0,0,0)
            border.width: 0

            Rectangle {
                anchors.centerIn: parent
                width: annotation.attributes.orientation === "Horizontal" ? parent.width : annotation.attributes.lineWidth
                height: annotation.attributes.orientation === "Vertical" ? parent.height : annotation.attributes.lineWidth
                color: annotation.attributes.lineColor
                opacity: annotation.attributes.opacity
            }
        }
    }

    Component {
        id: unknownAnnotationComponent

        Text {
            text: "Unknown annotation: <strong>" + annotation.type + "</strong>"
            x: annotation.geometry.x
            y: annotation.geometry.y
        }
    }
}
