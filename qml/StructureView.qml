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
                down: newSceneColorMenuLoader.active
                enabled: !scriteDocument.readOnly
                onClicked: newSceneColorMenuLoader.active = true
                iconSource: "../icons/content/add_box.png"
                ToolTip.text: "Add Scene"
                property color activeColor: "white"

                Loader {
                    id: newSceneColorMenuLoader
                    width: parent.width; height: 1
                    anchors.top: parent.bottom
                    sourceComponent: ColorMenu {
                        selectedColor: newSceneButton.activeColor
                    }
                    active: false
                    onItemChanged: {
                        if(item)
                            item.open()
                    }

                    Connections {
                        target: newSceneColorMenuLoader.item
                        onAboutToHide: newSceneColorMenuLoader.active = false
                        onMenuItemClicked: {
                            newSceneButton.activeColor = color
                            createElementMouseHandler.enabled = true
                            newSceneColorMenuLoader.active = false
                        }
                    }
                }
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
                onClicked: selection.init(elementItems, elementsBoundingBox.boundingBox)
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

            function createElement(x, y, c) {
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

            TightBoundingBoxEvaluator {
                id: elementsBoundingBox
            }

            DelayedPropertyBinder {
                id: widthBinder
                initial: 1000
                set: Math.max( Math.ceil(elementsBoundingBox.right / 100) * 100, 60000 )
                onGetChanged: scriteDocument.structure.canvasWidth = get
            }

            DelayedPropertyBinder {
                id: heightBinder
                initial: 1000
                set: Math.max( Math.ceil(elementsBoundingBox.bottom / 100) * 100, 60000 )
                onGetChanged: scriteDocument.structure.canvasHeight = get
            }

            FocusTracker.window: qmlWindow
            FocusTracker.indicator.target: mainUndoStack
            FocusTracker.indicator.property: "structureEditorActive"

            MouseArea {
                id: createElementMouseHandler
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                enabled: false
                cursorShape: Qt.CrossCursor
                onClicked: {
                    if(!scriteDocument.readOnly)
                        canvas.createElement(mouse.x-130, mouse.y-22, newSceneButton.activeColor)
                    enabled = false
                }
            }

            Item {
                id: annotationsLayer
                anchors.fill: parent
                enabled: !createElementMouseHandler.enabled

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
                        sourceComponent: {
                            switch(annotation.type) {
                            case "rectangle": return rectangleAnnotationComponent
                            }
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
                        target: createElementMouseHandler
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
                        target: canvasContextMenu
                        onVisibleChanged: {
                            if(canvasContextMenu.visible)
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
                enabled: !createElementMouseHandler.enabled
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
                onPressed: canvasContextMenu.popup()
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
                        }                    }
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
                id: canvasContextMenu

                MenuItem2 {
                    text: "New Scene"
                    enabled: !scriteDocument.readOnly
                    onClicked: {
                        canvas.createElement(canvasContextMenu.x-130, canvasContextMenu.y-22, newSceneButton.activeColor)
                        canvasContextMenu.close()
                    }
                }

                ColorMenu {
                    title: "Colored Scene"
                    selectedColor: newSceneButton.activeColor
                    enabled: !scriteDocument.readOnly
                    onMenuItemClicked: {
                        newSceneButton.activeColor = color
                        canvas.createElement(canvasContextMenu.x-130, canvasContextMenu.y-22, newSceneButton.activeColor)
                        canvasContextMenu.close()
                    }
                }

                MenuSeparator { }

                Menu2 {
                    title: "Annotation"

                    MenuItem2 {
                        text: "Rectangle"
                        onClicked: structureView.createNewRectangleAnnotation(canvasContextMenu.x,canvasContextMenu.y)
                    }

                    MenuItem2 {
                        text: "Text"
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

    FlickablePreview {
        id: canvasPreview
        anchors.right: canvasScroll.right
        anchors.bottom: canvasScroll.bottom
        anchors.margins: 30
        flickable: canvasScroll
        content: canvas
        maximumWidth: 150
        maximumHeight: 150
        onViewportRectRequest: canvasScroll.ensureVisible(rect, canvas.scale, 0)
        visible: structureCanvasSettings.showPreview

        TrackerPack {
            delay: 100
            enabled: !scriteDocument.loading && canvasPreview.visible

            TrackProperty { target: scriteDocument; property: "modified" }
            TrackProperty { target: canvas; property: "width" }
            TrackProperty { target: canvas; property: "height" }
            TrackProperty { target: canvasScroll; property: "width" }
            TrackProperty { target: canvasScroll; property: "height" }
            TrackProperty { target: selection; property: active }
            TrackSignal { target: scriteDocument.structure; signal: "structureChanged()" }

            onTracked: {
                var sh = 150
                var mw = sh
                var mh = sh
                if(canvas.width !== canvas.height) {
                    var maxSize = Qt.size(canvasScroll.width-canvasPreview.anchors.rightMargin-12,canvasScroll.height-canvasPreview.anchors.bottomMargin-12)
                    if(maxSize.width < 0 || maxSize.height < 0) {
                        canvasPreview.maximumWidth = sh
                        canvasPreview.maximumHeight = sh
                        return // dont generate any preview yet.
                    }
                    var ar = Math.max(canvas.width,canvas.height)/Math.min(canvas.width,canvas.height)
                    if(canvas.width > canvas.height)
                        mw = ar * sh
                    else
                        mh = ar * sh
                    var size = app.scaledSize( Qt.size(mw,mh), maxSize )
                    mw = size.width
                    mh = size.height

                    if(mh > sh && mw > sh) {
                        if(mh > sh) {
                            mw *= sh/mh;
                            mh = sh
                        } else if(mw > sh) {
                            mh *= sh/mw;
                            mw = sh
                        }
                    }
                }

                canvasPreview.maximumWidth = mw
                canvasPreview.maximumHeight = mh
                canvasPreview.updateThumbnail()
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

            TightBoundingBoxItem.evaluator: elementsBoundingBox

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
                border.color: (element.scene.color === Qt.rgba(1,1,1,1) ? "lightgray" : element.scene.color)
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

        Annotation { }
    }

    Component {
        id: annotationGripComponent

        Item {
            id: annotationGripItem
            x: annotationItem.x
            y: annotationItem.y
            width: annotationItem.width
            height: annotationItem.height
            readonly property int geometryUpdateInterval: 50

            PainterPathItem {
                id: focusIndicator
                anchors.fill: parent
                anchors.margins: -5
                renderType: PainterPathItem.OutlineOnly
                renderingMechanism: PainterPathItem.UseQPainter
                outlineWidth: 1
                outlineColor: accentColors.a700.background
                outlineStyle: PainterPathItem.DashDotDotLine
                painterPath: PainterPath {
                    MoveTo { x: 0; y: 0 }
                    LineTo { x: focusIndicator.width-1; y: 0 }
                    LineTo { x: focusIndicator.width-1; y: focusIndicator.height-1 }
                    LineTo { x: 0; y: focusIndicator.height-1 }
                    CloseSubpath { }
                }
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
                cursorShape: Qt.SizeAllCursor
                drag.target: annotationItem
                drag.minimumX: 0
                drag.minimumY: 0
                drag.maximumX: canvas.width - parent.width
                drag.maximumY: canvas.height - parent.height
                drag.axis: Drag.XAndYAxis
            }

            Rectangle {
                id: rightGrip
                width: 10
                height: 10
                color: accentColors.a700.background
                x: parent.width - width/2
                y: (parent.height - height)/2

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
                width: 10
                height: 10
                color: accentColors.a700.background
                x: (parent.width - width)/2
                y: parent.height - height/2

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
                width: 10
                height: 10
                color: accentColors.a700.background
                x: parent.width - width/2
                y: parent.height - height/2

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

    function createNewRectangleAnnotation(x, y) {
        var annot = annotationObject.createObject(canvas)
        annot.type = "rectangle"
        annot.geometry = Qt.rect(x, y, 200, 200)
        scriteDocument.structure.addAnnotation(annot)
    }

    Component {
        id: rectangleAnnotationComponent

        Rectangle {
            x: annotation.geometry.x
            y: annotation.geometry.y
            width: annotation.geometry.width
            height: annotation.geometry.height
            color: annotation.attributes.color
            border {
                width: annotation.attributes.borderWidth
                color: annotation.attributes.borderColor
            }
            opacity: annotation.attributes.opacity / 100

            MouseArea {
                anchors.fill: parent
                enabled: annotationGripLoader.annotation !== annotation
                onClicked: {
                    annotationGripLoader.annotationItem = parent
                    annotationGripLoader.annotation = annotation
                }
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
