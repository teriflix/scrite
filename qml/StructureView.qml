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
        color: primaryColors.c100.background
        height: toolbarLayout.height+4
        border.width: 1
        border.color: primaryColors.borderColor
        radius: 6

        Flow {
            id: toolbarLayout
            spacing: 3
            width: parent.width-4
            anchors.verticalCenter: parent.verticalCenter
            layoutDirection: Flow.LeftToRight
            property real rowHeight: newSceneButton.height

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
                height: parent.rowHeight
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                iconSource: "../icons/hardware/mouse.png"
                autoRepeat: false
                ToolTip.text: "Mouse wheel currently " + (checked ? "zooms" : "scrolls") + ". Click this button to make it " + (checked ? "scroll" : "zoom") + "."
                checkable: true
                checked: workspaceSettings.mouseWheelZoomsInStructureCanvas
                onCheckedChanged: workspaceSettings.mouseWheelZoomsInStructureCanvas = checked
            }

            ToolButton3 {
                id: cmdZoomIn
                onClicked: { canvasScroll.zoomIn(); canvasScroll.updateScriteDocumentUserData() }
                iconSource: "../icons/navigation/zoom_in.png"
                autoRepeat: true
                ToolTip.text: "Zoom In"
            }

            ToolButton3 {
                id: cmdZoomOut
                onClicked: { canvasScroll.zoomOut(); canvasScroll.updateScriteDocumentUserData() }
                iconSource: "../icons/navigation/zoom_out.png"
                autoRepeat: true
                ToolTip.text: "Zoom Out"
            }

            ToolButton3 {
                id: cmdZoomOne
                onClicked: click()
                iconSource: "../icons/navigation/zoom_one.png"
                autoRepeat: true
                ToolTip.text: "Zoom One"
                function click() {
                    var item = currentElementItemBinder.get
                    if(item === null) {
                        if(elementItems.count > 0)
                            item = elementItems.itemAt(0)
                        if(item === null) {
                            canvasScroll.zoomOneMiddleArea()
                            canvasScroll.updateScriteDocumentUserData()
                            return
                        }
                    }
                    canvasScroll.zoomOneToItem(item)
                    canvasScroll.updateScriteDocumentUserData()
                }
            }

            ToolButton3 {
                id: cmdZoomFit
                onClicked: {
                    canvasItemsBoundingBox.recomputeBoundingBox()
                    canvasScroll.zoomFit(canvasItemsBoundingBox.boundingBox);
                    canvasScroll.isZoomFit = true
                    canvasScroll.updateScriteDocumentUserData()
                }
                iconSource: "../icons/navigation/zoom_fit.png"
                autoRepeat: true
                ToolTip.text: "Zoom Fit"
            }

            Rectangle {
                width: 1
                height: parent.rowHeight
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                id: selectionModeButton
                enabled: !scriteDocument.readOnly && (selection.hasItems ? selection.canLayout : scriteDocument.structure.elementCount >= 2)
                iconSource: "../icons/action/selection_drag.png"
                ToolTip.text: "Selection mode"
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

            ToolButton3 {
                id: beatBoardLayoutToolButton
                enabled: !scriteDocument.readOnly && scriteDocument.screenplay.elementCount > 0
                iconSource: "../icons/action/layout_beat_sheet.png"
                ToolTip.text: "Beat Board Layout"
                checkable: true
                checked: false
                onToggled: {
                    scriteDocument.structure.forceBeatBoardLayout = checked
                    if(checked) {
                        var rect = scriteDocument.structure.placeElementsInBeatBoardLayout(scriteDocument.screenplay)
                        cmdZoomOne.click()
                    }
                }

                Component.onCompleted: checked = scriteDocument.structure.forceBeatBoardLayout

                Connections {
                    target: scriteDocument.structure
                    onForceBeatBoardLayoutChanged: beatBoardLayoutToolButton.checked = scriteDocument.structure.forceBeatBoardLayout
                }
            }

            ToolButton3 {
                iconSource: "../icons/action/layout_grouping.png"
                ToolTip.text: "Grouping Options"
                onClicked: layoutGroupingMenu.popup()
                down: layoutGroupingMenu.visible

                Menu2 {
                    id: layoutGroupingMenu
                    width: 350

                    MenuItem2 {
                        text: "Acts"
                        font.bold: canvas.groupCategory === ""
                        onTriggered: canvas.groupCategory = ""
                    }

                    MenuSeparator { }

                    Repeater {
                        model: scriteDocument.structure.groupCategories

                        MenuItem2 {
                            text: app.camelCased(modelData)
                            font.bold: canvas.groupCategory === modelData
                            onTriggered: canvas.groupCategory = modelData
                        }
                    }
                }
            }

            ToolButton3 {
                id: tagMenuOption
                iconSource: "../icons/action/tag.png"
                enabled: (selection.hasItems || currentElementItemBinder.get !== null) && scriteDocument.structure.canvasUIMode === Structure.IndexCardUI
                ToolTip.text: {
                    if(selection.hasItems)
                        return "Tag the " + selection.items.length + " selected index card(s)"
                    else if(currentElementItemBinder.get !== null)
                        return "Tag the selected index card."
                    return ""
                }
                onClicked: {
                    tagMenuLoader.popup()
                }
                down: tagMenuLoader.active

                MenuLoader {
                    id: tagMenuLoader
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    menu: StructureGroupsMenu {
                        innerTitle: tagMenuOption.ToolTip.text
                        sceneGroup: SceneGroup {
                            structure: scriteDocument.structure
                        }

                        onToggled: {
                            if(selection.hasItems)
                                app.execLater(selection, 250, function() { selection.refit() })
                        }

                        onAboutToShow: {
                            sceneGroup.clearScenes()
                            if(selection.hasItems) {
                                var items = selection.items
                                items.forEach( function(item) {
                                    sceneGroup.addScene(item.element.scene)
                                })
                            } else {
                                sceneGroup.addScene(currentElementItemBinder.get.element.scene)
                            }
                        }
                        onClosed: sceneGroup.clearScenes()
                    }
                }
            }

            Rectangle {
                width: 1
                height: parent.rowHeight
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                enabled: !selection.hasItems && (annotationGripLoader.active || currentElementItemBinder.get !== null)
                iconSource: "../icons/content/content_copy.png"
                ToolTip.text: "Copy the selected scene or annotation."
                onClicked: {
                    if(annotationGripLoader.active) {
                        scriteDocument.structure.copy(annotationGripLoader.annotation)
                        statusText.show("Annotation Copied")
                    } else {
                        var spe = scriteDocument.structure.elementAt(scriteDocument.structure.currentElementIndex)
                        if(spe !== null) {
                            scriteDocument.structure.copy(spe)
                            statusText.show("Scene Copied")
                        }
                    }
                }
                shortcut: "Ctrl+C"
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Copy Annotation"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: app.polishShortcutTextForDisplay("Ctrl+C")
            }

            ToolButton3 {
                enabled: !scriteDocument.readOnly && scriteDocument.structure.canPaste
                iconSource: "../icons/content/content_paste.png"
                ToolTip.text: "Paste from clipboard"
                onClicked: {
                    var gpos = app.globalMousePosition()
                    var pos = canvasScroll.mapFromGlobal(gpos.x, gpos.y)
                    if(pos.x < 0 || pos.y < 0 || pos.x >= canvasScroll.width || pos.y >= canvasScroll.height)
                        scriteDocument.structure.paste()
                    else {
                        pos = canvas.mapFromGlobal(gpos.x, gpos.y)
                        scriteDocument.structure.paste(Qt.point(pos.x,pos.y))
                    }
                }
                shortcut: "Ctrl+V"
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Paste"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: app.polishShortcutTextForDisplay(shortcut)
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
        zoomOnScroll: workspaceSettings.mouseWheelZoomsInStructureCanvas
        interactive: !(rubberBand.active || selection.active || canvasPreview.interacting || annotationGripLoader.active) && mouseOverItem === null && editItem === null && maybeDragItem === null
        property Item mouseOverItem
        property Item editItem
        property Item maybeDragItem
        property bool isZoomFit: false

        property rect viewportRect: Qt.rect( visibleArea.xPosition * contentWidth / canvas.scale,
                                           visibleArea.yPosition * contentHeight / canvas.scale,
                                           visibleArea.widthRatio * contentWidth / canvas.scale,
                                           visibleArea.heightRatio * contentHeight / canvas.scale )

        Connections {
            target: scriteDocument
            onJustLoaded: canvasScroll.updateFromScriteDocumentUserDataLater()
        }

        Component.onCompleted: canvasScroll.updateFromScriteDocumentUserDataLater()
        Component.onDestruction: canvasScroll.updateScriteDocumentUserData()
        onZoomScaleChangedInteractively: Qt.callLater(updateScriteDocumentUserData)
        onContentXChanged: Qt.callLater(updateScriteDocumentUserData)
        onContentYChanged: Qt.callLater(updateScriteDocumentUserData)
        onZoomScaleChanged: isZoomFit = false
        onAnimatePanAndZoomChanged: Qt.callLater(updateScriteDocumentUserData)
        animatePanAndZoom: false
        property bool updateScriteDocumentUserDataEnabled: false

        function updateScriteDocumentUserData() {
            if(!updateScriteDocumentUserDataEnabled || scriteDocument.readOnly || animatingPanOrZoom)
                return

            var userData = scriteDocument.userData
            userData["StructureView.canvasScroll"] = {
                "version": 0,
                "contentX": canvasScroll.contentX,
                "contentY": canvasScroll.contentY,
                "zoomScale": canvasScroll.zoomScale,
                "isZoomFit": canvasScroll.isZoomFit
            }
            scriteDocument.userData = userData
        }

        function updateFromScriteDocumentUserData() {
            if(elementItems.count < scriteDocument.structure.elementCount) {
                updateFromScriteDocumentUserDataLater()
                return
            }

            var userData = scriteDocument.userData
            var csData = userData["StructureView.canvasScroll"];
            if(csData && csData.version === 0) {
                canvasScroll.zoomScale = csData.zoomScale
                canvasScroll.contentX = csData.contentX
                canvasScroll.contentY = csData.contentY
                canvasScroll.isZoomFit = csData.isZoomFit === true
                if(canvasScroll.isZoomFit) {
                    app.execLater(canvasScroll, 500, function() {
                        var area = canvasItemsBoundingBox.boundingBox
                        canvasScroll.zoomFit(area)
                    })
                }
            } else {
                if(scriteDocument.structure.elementCount > 0) {
                    var item = currentElementItemBinder.get
                    if(item === null)
                        item = elementItems.itemAt(0)
                    canvasScroll.ensureItemVisible(item, canvas.scale)
                } else
                    canvasScroll.zoomOneMiddleArea()
            }

            if(scriteDocument.structure.forceBeatBoardLayout)
                scriteDocument.structure.placeElementsInBeatBoardLayout(scriteDocument.screenplay)

            updateScriteDocumentUserDataEnabled = true
            animatePanAndZoom = true
        }

        function updateFromScriteDocumentUserDataLater() {
            app.execLater(canvasScroll, 500, updateFromScriteDocumentUserData)
        }

        onUpdateScriteDocumentUserDataEnabledChanged: {
            if(updateScriteDocumentUserDataEnabled)
                app.execLater(canvasScroll, 500, zoomSanityCheck)
        }

        function zoomSanityCheck() {
            if( !app.doRectanglesIntersect(canvasItemsBoundingBox.boundingBox, canvasScroll.viewportRect) ) {
                var item = currentElementItemBinder.get
                if(item === null)
                    item = elementItems.itemAt(0)
                canvasScroll.ensureItemVisible(item, canvas.scale)
            }
        }

        function zoomOneMiddleArea() {
            var middleArea = Qt.rect((canvas.width-canvasScroll.width)/2,
                                     (canvas.height-canvasScroll.height)/2,
                                     canvasScroll.width,
                                     canvasScroll.height)
            canvasScroll.ensureVisible(middleArea)
        }

        function zoomOneToItem(item) {
            if(item === null)
                return
            var bbox = canvasItemsBoundingBox.boundingBox
            var itemRect = Qt.rect(item.x, item.y, item.width, item.height)
            var atBest = Qt.size(canvasScroll.width, canvasScroll.height)
            var visibleArea = app.querySubRectangle(bbox, itemRect, atBest)
            canvasScroll.zoomFit(visibleArea)
        }

        function deleteCurrentElement(element) {
            if(element === null)
                element = scriteDocument.structure.elementAt(criteDocument.structure.currentElementIndex)
            if(element === null)
                return

            askQuestion({
                "question": "Are you sure you want to delete this scene?",
                "okButtonText": "Yes",
                "cancelButtonText": "No",
                "callback": function(val) {
                    if(val) {
                        var nextScene = null
                        var nextElement = null
                        if(element.scene.addedToScreenplay) {
                            nextElement = scriteDocument.screenplay.elementAt(element.scene.screenplayElementIndexList[0]+1)
                            if(nextElement === null)
                                nextElement = scriteDocument.screenplay.elementAt(scriteDocument.screenplay.lastSceneIndex())
                            if(nextElement !== null)
                                nextScene = nextElement.scene
                        } else {
                            var idx = scriteDocument.structure.indexOfElement(element)
                            var i = 0;
                            for(i=idx+1; i<scriteDocument.structure.elementCount; i++) {
                                nextElement = scriteDocument.structure.elementAt(i)
                                if(nextElement.scene.addedToScreenplay)
                                    continue;
                                nextScene = nextElement.scene
                                break
                            }

                            if(nextScene === null) {
                                for(i=0; i<idx; i++) {
                                    nextElement = scriteDocument.structure.elementAt(i)
                                    if(nextElement.scene.addedToScreenplay)
                                        continue;
                                    nextScene = nextElement.scene
                                    break
                                }
                            }
                        }

                        releaseEditor()
                        scriteDocument.screenplay.removeSceneElements(element.scene)
                        scriteDocument.structure.removeElement(element)

                        Qt.callLater(function(scene) {
                            if(scriteDocument.screenplay.elementCount === 0)
                                return
                            if(scene === null)
                                scene = scriteDocument.screenplay.elementAt(scriteDocument.screenplay.lastSceneIndex())
                            var idx = scriteDocument.structure.indexOfScene(scene)
                            if(idx >= 0) {
                                scriteDocument.structure.currentElementIndex = idx
                                scriteDocument.screenplay.currentElementIndex = scriteDocument.screenplay.firstIndexOfScene(scene)
                            }
                        }, nextScene)
                    }
                }
            }, element.follow)
        }

        GridBackground {
            id: canvas
            antialiasing: false
            majorTickLineWidth: 2*app.devicePixelRatio
            minorTickLineWidth: 1*app.devicePixelRatio
            width: widthBinder.get
            height: heightBinder.get
            tickColorOpacity: 0.25 * scale
            scale: canvasScroll.suggestedScale
            border.width: 2
            border.color: structureCanvasSettings.gridColor
            gridIsVisible: structureCanvasSettings.showGrid && canvasScroll.interactive
            majorTickColor: structureCanvasSettings.gridColor
            minorTickColor: structureCanvasSettings.gridColor
            tickDistance: scriteDocument.structure.canvasGridSize
            transformOrigin: Item.TopLeft
            backgroundColor: canvasScroll.interactive ? primaryColors.c10.background : app.translucent(primaryColors.c300.background, 0.75)
            Behavior on backgroundColor {
                enabled: screenplayEditorSettings.enableAnimations
                ColorAnimation { duration: 250 }
            }

            property bool scaleLessThanHalf: scale < 0.5
            onScaleLessThanHalfChanged: {
                if(scaleLessThanHalf)
                    canvasTabSequence.releaseFocus()
            }

            TabSequenceManager {
                id: canvasTabSequence
                wrapAround: true
                releaseFocusEnabled: true
                onFocusWasReleased: canvas.forceActiveFocus()
            }

            function createItem(what, where) {
                if(scriteDocument.readOnly)
                    return

                if(what === undefined || what === "" | what === "element")
                    createElement(where.x, where.y, newSceneButton.activeColor)
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
                requestEditorLater()
                element.scene.undoRedoEnabled = true
            }

            readonly property var annotationsList: [
                { "title": "Text", "what": "text" },
                { "title": "Oval", "what": "oval" },
                { "title": "Image", "what": "image" },
                { "title": "Rectangle", "what": "rectangle" },
                { "title": "Website Link", "what": "url" },
                { "title": "Vertical Line", "what": "vline" },
                { "title": "Horizontal Line", "what": "hline" }
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
                case "oval":
                    structureView.createNewOvalAnnotation(x,y)
                    break
                }
            }

            BoundingBoxEvaluator {
                id: canvasItemsBoundingBox
                initialRect: scriteDocument.structure.annotationsBoundingBox
            }

            DelayedPropertyBinder {
                id: widthBinder
                initial: 1000
                set: Math.max( Math.ceil(canvasItemsBoundingBox.right / 100) * 100, 120000 )
                onGetChanged: scriteDocument.structure.canvasWidth = get
            }

            DelayedPropertyBinder {
                id: heightBinder
                initial: 1000
                set: Math.max( Math.ceil(canvasItemsBoundingBox.bottom / 100) * 100, 120000 )
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
                hoverEnabled: true
                z: 10000

                property string what
                function handle(_what) {
                    what = _what
                    enabled = true
                }

                Image {
                    width: 30/canvas.scale
                    height: width
                    sourceSize.width: width
                    sourceSize.height: height
                    source: "../icons/navigation/center_focus.svg"
                    property real halfSize: width/2
                    x: parent.mouseX - halfSize
                    y: parent.mouseY - halfSize
                    visible: parent.enabled
                }

                EventFilter.target: app
                EventFilter.events: [EventFilter.KeyPress]
                EventFilter.active: createItemMouseHandler.enabled
                EventFilter.onFilter: {
                    if(event.key === Qt.Key_Escape)
                        createItemMouseHandler.enabled = false
                    result.accept = false
                    result.filter = false
                }

                onClicked: {
                    var _what = what
                    what = ""
                    enabled = false
                    if(!scriteDocument.readOnly) {
                        var where = Qt.point(mouse.x, mouse.y)
                        if(_what === "element")
                            where = Qt.point(mouse.x-130, mouse.y-22)
                        canvas.createItem(_what, where)
                    }
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

                StructureCanvasViewportFilterModel {
                    id: annotationsFilterModel
                    enabled: scriteDocument.loading ? false : scriteDocument.structure.annotationCount > 100
                    structure: scriteDocument.structure
                    type: StructureCanvasViewportFilterModel.AnnotationType
                    viewportRect: canvasScroll.viewportRect
                    computeStrategy: StructureCanvasViewportFilterModel.PreComputeStrategy
                    filterStrategy: StructureCanvasViewportFilterModel.IntersectsStrategy
                }

                Repeater {
                    id: annotationItems
                    model: annotationsFilterModel
                    delegate: Loader {
                        property Annotation annotation: modelData
                        property int annotationIndex: index
                        active: !annotationsFilterModel.enabled
                        property bool canvasIsChanging: active ? false :canvasScroll.changing
                        onCanvasIsChangingChanged: {
                            if(!canvasIsChanging)
                                active = true
                        }
                        asynchronous: true
                        sourceComponent: {
                            switch(annotation.type) {
                            case "rectangle": return rectangleAnnotationComponent
                            case "text": return textAnnotationComponent
                            case "url": return urlAnnotationComponent
                            case "image": return imageAnnotationComponent
                            case "line": return lineAnnotationComponent
                            case "oval": return ovalAnnotationComponent
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

                    Connections {
                        target: structureCanvasSettings
                        onDisplayAnnotationPropertiesChanged: {
                            if(structureCanvasSettings.displayAnnotationProperties)
                                floatingDockWidget.display("Annotation Properties", annotationPropertyEditorComponent)
                            else
                                floatingDockWidget.close()
                        }
                    }

                    onAnnotationChanged: {
                        if(annotation === null)
                            floatingDockWidget.hide()
                        else {
                            if(floatingDockWidget.contentX < 0) {
                                var maxContentX = (documentUI.width - floatingDockWidget.contentWidth - 20)
                                floatingDockWidget.contentX = Math.min(documentUI.mapFromItem(structureView, 0, 0).x + structureView.width + 40, maxContentX)
                                floatingDockWidget.contentY = (documentUI.height - floatingDockWidget.contentHeight)/2
                            }

                            if(structureCanvasSettings.displayAnnotationProperties)
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
                id: currentElementItemShadow
                anchors.fill: currentElementItem
                visible: currentElementItem !== null && !annotationGripLoader.active && currentElementItem.visible
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

                EventFilter.target: app
                EventFilter.active: !scriteDocument.readOnly && visible && opacity === 1 && !modalDialog.active && !createItemMouseHandler.enabled
                EventFilter.events: [6]
                EventFilter.onFilter: {
                    var dist = (event.controlModifier ? 5 : 1) * canvas.tickDistance
                    var element = scriteDocument.structure.elementAt(scriteDocument.structure.currentElementIndex)
                    if(element === null)
                        return

                    switch(event.key) {
                    case Qt.Key_Left:
                        element.x -= dist
                        result.accept = true
                        result.filter = true
                        scriteDocument.structure.forceBeatBoardLayout = false
                        break
                    case Qt.Key_Right:
                        element.x += dist
                        result.accept = true
                        result.filter = true
                        scriteDocument.structure.forceBeatBoardLayout = false
                        break
                    case Qt.Key_Up:
                        element.y -= dist
                        result.accept = true
                        result.filter = true
                        scriteDocument.structure.forceBeatBoardLayout = false
                        break
                    case Qt.Key_Down:
                        element.y += dist
                        result.accept = true
                        result.filter = true
                        scriteDocument.structure.forceBeatBoardLayout = false
                        break
                    case Qt.Key_Delete:
                    case Qt.Key_Backspace:
                        result.accept = true
                        result.filter = true
                        canvasScroll.deleteCurrentElement(element)
                        break
                    }
                }
            }

            property string groupCategory: scriteDocument.structure.preferredGroupCategory
            property var groupBoxes: []
            property var episodeBoxes: []
            property bool groupsBeingMoved: false
            Component.onCompleted: app.execLater(canvas, 250, reevaluateEpisodeAndGroupBoxes)

            function reevaluateEpisodeAndGroupBoxes() {
                if(groupsBeingMoved)
                    return
                var egBoxes = scriteDocument.structure.evaluateEpisodeAndGroupBoxes(scriteDocument.screenplay, canvas.groupCategory)
                canvas.groupBoxes = egBoxes.groupBoxes
                canvas.episodeBoxes = egBoxes.episodeBoxes
            }

            onGroupCategoryChanged: {
                scriteDocument.structure.preferredGroupCategory = groupCategory
                app.execLater(canvas, 250, reevaluateEpisodeAndGroupBoxes)
            }

            TrackerPack {
                delay: 250

                TrackProperty {
                    target: elementItems
                    property: "count"
                }

                TrackSignal {
                    target: scriteDocument.screenplay
                    signal: "elementsChanged()"
                }

                TrackSignal {
                    target: scriteDocument.screenplay
                    signal: "breakTitleChanged()"
                }

                TrackSignal {
                    target: scriteDocument
                    signal: "loadingChanged()"
                }

                TrackSignal {
                    target: scriteDocument.structure
                    signal: "structureChanged()"
                }

                TrackSignal {
                    target: scriteDocument.screenplay
                    signal: "elementSceneGroupsChanged(ScreenplayElement*)"
                }

                TrackSignal {
                    target: scriteDocument.screenplay
                    signal: "episodeCountChanged()"
                }

                onTracked: canvas.reevaluateEpisodeAndGroupBoxes()
            }

            Repeater {
                model: canvas.episodeBoxes

                Rectangle {
                    id: canvasEpisodeBox

                    property real topMarginForStacks: scriteDocument.structure.elementStacks.objectCount > 0 ? 15 : 0

                    x: modelData.geometry.x - 40
                    y: modelData.geometry.y - 120 - topMarginForStacks
                    width: modelData.geometry.width + 80
                    height: modelData.geometry.height + 120 + topMarginForStacks + 40
                    color: app.translucent(accentColors.windowColor, 0.1)
                    border.width: 2
                    border.color: accentColors.c600.background
                    enabled: !createItemMouseHandler.enabled && !currentElementItemShadow.visible && !annotationGripLoader.active

                    BoundingBoxItem.evaluator: canvasItemsBoundingBox
                    BoundingBoxItem.stackOrder: 1.0 + (index/canvas.episodeBoxes.length)
                    BoundingBoxItem.livePreview: false
                    BoundingBoxItem.previewFillColor: Qt.rgba(0,0,0,0)
                    BoundingBoxItem.previewBorderColor: Qt.rgba(0,0,0,0)
                    BoundingBoxItem.viewportItem: canvas
                    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
                    BoundingBoxItem.viewportRect: canvasScroll.viewportRect

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: episodeNameText.bottom
                        anchors.bottomMargin: -8
                        color: accentColors.c600.background
                    }

                    Text {
                        id: episodeNameText
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.margins: 8
                        font.family: "Arial"
                        font.pointSize: app.idealFontPointSize + 8
                        font.bold: true
                        color: accentColors.c600.text
                        text: "<b>" + modelData.name + "</b><font size=\"-2\">: " + modelData.sceneCount + (modelData.sceneCount === 1 ? " scene": " scenes") + "</font>"
                    }
                }
            }

            Repeater {
                model: canvas.groupBoxes

                Rectangle {
                    id: canvasGroupBoxItem
                    property real topMarginForStacks: scriteDocument.structure.elementStacks.objectCount > 0 ? 15 : 0
                    x: modelData.geometry.x - 20
                    y: modelData.geometry.y - 20 - topMarginForStacks
                    width: modelData.geometry.width + 40
                    height: modelData.geometry.height + 40 + topMarginForStacks
                    radius: 0
                    color: app.translucent(accentColors.windowColor, 0.1)
                    border.width: 1
                    border.color: accentColors.borderColor
                    enabled: !createItemMouseHandler.enabled && !annotationGripLoader.active

                    BoundingBoxItem.evaluator: canvasItemsBoundingBox
                    BoundingBoxItem.stackOrder: 2.0 + (index/canvas.groupBoxes.length)
                    BoundingBoxItem.livePreview: false
                    BoundingBoxItem.previewFillColor: Qt.rgba(0,0,0,0)
                    BoundingBoxItem.previewBorderColor: Qt.rgba(0,0,0,0)
                    BoundingBoxItem.viewportItem: canvas
                    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
                    BoundingBoxItem.viewportRect: canvasScroll.viewportRect

                    onXChanged: if(canvasBeatMouseArea.drag.active || canvasBeatLabelMouseArea.drag.active) Qt.callLater(moveBeat)
                    onYChanged: if(canvasBeatMouseArea.drag.active || canvasBeatLabelMouseArea.drag.active) Qt.callLater(moveBeat)

                    function moveBeat() {
                        var dx = x - refX
                        var dy = y - refY
                        var nrElements = modelData.sceneCount
                        for(var i=0; i<nrElements; i++) {
                            var item = elementItems.itemAt(modelData.sceneIndexes[i])
                            item.x = item.x + dx
                            item.y = item.y + dy
                        }
                        refX = x
                        refY = y
                    }

                    function selectBeatItems() {
                        var items = []
                        var nrElements = modelData.sceneCount
                        for(var i=0; i<nrElements; i++) {
                            var item = elementItems.itemAt(modelData.sceneIndexes[i])
                            items.push(item)
                        }

                        selection.set(items)
                    }

                    property real refX: x
                    property real refY: y

                    MouseArea {
                        id: canvasBeatMouseArea
                        anchors.fill: parent
                        drag.target: controlPressed ? null : canvasGroupBoxItem
                        drag.axis: Drag.XAndYAxis
                        cursorShape: Qt.SizeAllCursor
                        property bool controlPressed: false
                        onPressed: {
                            controlPressed = mouse.modifiers & Qt.ControlModifier
                            if(controlPressed) {
                                mouse.accepted = false
                                return
                            }
                        }

                        drag.onActiveChanged: {
                            selection.clear()
                            canvasGroupBoxItem.refX = canvasGroupBoxItem.x
                            canvasGroupBoxItem.refY = canvasGroupBoxItem.y
                            canvas.groupsBeingMoved = drag.active
                        }
                        onDoubleClicked: canvasGroupBoxItem.selectBeatItems()
                    }

                    Rectangle {
                        anchors.fill: beatLabel
                        anchors.margins: -parent.radius
                        border.width: parent.border.width
                        border.color: parent.border.color
                        color: app.translucent(accentColors.windowColor, 0.4)

                        MouseArea {
                            id: canvasBeatLabelMouseArea
                            anchors.fill: parent
                            drag.target: canvasGroupBoxItem
                            drag.axis: Drag.XAndYAxis
                            cursorShape: Qt.SizeAllCursor
                            drag.onActiveChanged: {
                                selection.clear()
                                canvasGroupBoxItem.refX = canvasGroupBoxItem.x
                                canvasGroupBoxItem.refY = canvasGroupBoxItem.y
                                canvas.groupsBeingMoved = drag.active
                            }
                            onDoubleClicked: canvasGroupBoxItem.selectBeatItems()
                        }
                    }

                    Text {
                        id: beatLabel
                        text: "<b>" + modelData.name + "</b><font size=\"-2\">: " + modelData.sceneCount + (modelData.sceneCount === 1 ? " scene": " scenes") + "</font>"
                        font.family: "Arial"
                        font.pointSize: app.idealFontPointSize + 3
                        anchors.bottom: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: parent.radius*2
                        anchors.bottomMargin: parent.radius-parent.border.width
                        padding: 10
                        color: "black"
                    }
                }
            }

            Repeater {
                id: elementConnectorItems
                model: scriteDocument.loading ? 0 : scriteDocument.structureElementConnectors
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

            EventFilter.events: [EventFilter.DragEnter, EventFilter.DragMove, EventFilter.Drop]
            EventFilter.onFilter: {
                result.acceptEvent = false

                switch(event.type) {
                case EventFilter.DragEnter:
                case EventFilter.DragLeave:
                case EventFilter.Drop:
                    break
                default:
                    return
                }

                var sceneId = event.mimeData["scrite/sceneID"]
                var element = scriteDocument.structure.findElementBySceneID(sceneId)
                if(element === null)
                    return

                if(element.stackId === "")
                    return

                result.acceptEvent = true
                result.filter = true

                if(event.type === EventFilter.Drop) {
                    element.stackId = ""
                    app.execLater(element, 250, function() {
                        if(!scriteDocument.structure.forceBeatBoardLayout) {
                            element.x = event.pos.x
                            element.y = event.pos.y
                        }
                        scriteDocument.structure.currentElementIndex = scriteDocument.structure.indexOfElement(element)
                        scriteDocument.screenplay.currentElementIndex = scriteDocument.screenplay.firstIndexOfScene(element.scene)
                    })
                }
            }

            Repeater {
                id: stackBinders
                model: scriteDocument.loading ? null : scriteDocument.structure.elementStacks
                delegate: Item {
                    id: stackBinderItem
                    x: objectItem.geometry.x
                    y: objectItem.geometry.y
                    width: objectItem.geometry.width
                    height: objectItem.geometry.height

                    BoundingBoxItem.evaluator: canvasItemsBoundingBox
                    BoundingBoxItem.livePreview: false
                    BoundingBoxItem.previewFillColor: Qt.rgba(0,0,0,0)
                    BoundingBoxItem.previewBorderColor: Qt.rgba(0,0,0,0)
                    BoundingBoxItem.viewportItem: canvas
                    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
                    BoundingBoxItem.viewportRect: canvasScroll.viewportRect

                    Flickable {
                        id: tabBarItemFlick
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5
                        anchors.bottom: parent.top
                        anchors.bottomMargin: -tabBarItem.activeTabBorderWidth-0.5
                        contentWidth: tabBarItem.width
                        contentHeight: tabBarItem.height
                        height: contentHeight
                        interactive: contentWidth > width
                        clip: interactive

                        SimpleTabBarItem {
                            id: tabBarItem
                            tabCount: objectItem.objectCount
                            activeTabBorderWidth: (objectItem.hasCurrentElement ? 2 : 1)
                            tabLabelStyle: SimpleTabBarItem.Alphabets
                            activeTabIndex: objectItem.topmostElementIndex
                            activeTabColor: Qt.tint(objectItem.topmostElement.scene.color, (objectItem.hasCurrentElement ? "#C0FFFFFF" : "#F0FFFFFF"))
                            activeTabBorderColor: app.isLightColor(objectItem.topmostElement.scene.color) ? "black" : objectItem.topmostElement.scene.color
                            activeTabFont.pointSize: app.idealFontPointSize
                            activeTabFont.bold: true
                            activeTabTextColor: app.textColorFor(activeTabColor)
                            inactiveTabTextColor: app.translucent(app.textColorFor(inactiveTabColor), 0.75)
                            inactiveTabFont.pointSize: app.idealFontPointSize-4
                            minimumTabWidth: stackBinderItem.width*0.1
                            onTabClicked: objectItem.bringElementToTop(index)
                            onActiveTabIndexChanged: Qt.callLater(ensureActiveTabIsVisible)
                            onTabPathsUpdated: Qt.callLater(ensureActiveTabIsVisible)

                            Connections {
                                target: objectItem
                                onDataChanged: tabBarItem.updateTabAttributes()
                                onStackInitialized: tabBarItem.updateTabAttributes()
                            }

                            onAttributeRequest: {
                                if(index === activeTabIndex)
                                    return
                                var element = objectItem.objectAt(index)
                                switch(attr) {
                                case SimpleTabBarItem.TabColor:
                                    requestedAttributeValue = Qt.tint(element.scene.color, "#D0FFFFFF")
                                    break
                                case SimpleTabBarItem.TabBorderColor:
                                    requestedAttributeValue = app.isLightColor(element.scene.color) ? "gray" : element.scene.color
                                    break
                                default:
                                    break
                                }
                            }

                            function ensureActiveTabIsVisible() {
                                if(activeTabIndex < 0) {
                                    tabBarItemFlick.contentX = 0
                                    return
                                }
                                var r = tabRect(activeTabIndex)
                                if(tabBarItemFlick.contentX > r.x)
                                    tabBarItemFlick.contentX = r.x
                                else if(tabBarItemFlick.contentX + tabBarItemFlick.width < r.x + r.width)
                                    tabBarItemFlick.contentX = r.x + r.width - tabBarItemFlick.width
                            }
                        }
                    }
                }
            }

            Repeater {
                id: elementItems
                model: scriteDocument.loading ? null : scriteDocument.structure.elementsModel
                delegate: scriteDocument.structure.canvasUIMode === Structure.IndexCardUI ? structureElementIndexCardUIDelegate : structureElementSynopsisEditorUIDelegate
            }

            Selection {
                id: selection
                z: 3
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

                    MenuItem2 {
                        text: "Stack"
                        enabled: {
                            if(scriteDocument.structure.canvasUIMode !== Structure.IndexCardUI)
                                return false

                            var items = selection.items
                            var actIndex = -1
                            for(var i=0; i<items.length; i++) {
                                var item = items[i]
                                if(item.element.stackId !== "")
                                    return false

                                if(i === 0)
                                    actIndex = item.element.scene.actIndex
                                else if(actIndex !== item.element.scene.actIndex)
                                    return false
                            }

                            if(actIndex < 0)
                                return false

                            return true
                        }
                        onTriggered: {
                            var items = selection.items
                            var id = app.createUniqueId()
                            items.forEach( function(item) {
                                item.element.stackId = id
                            })
                            selection.clear()
                        }
                    }

                    MenuItem2 {
                        text: "Add To Timeline"
                        onClicked: {
                            var items = selection.items
                            items.forEach( function(item) {
                                scriteDocument.screenplay.addScene(item.element.scene)
                            })
                        }
                    }

                    StructureGroupsMenu {
                        sceneGroup: SceneGroup {
                            structure: scriteDocument.structure
                        }

                        onToggled: app.execLater(selection, 250, function() { selection.refit() })

                        onAboutToShow: {
                            sceneGroup.clearScenes()
                            var items = selection.items
                            items.forEach( function(item) {
                                sceneGroup.addScene(item.element.scene)
                            })
                        }
                        onClosed: sceneGroup.clearScenes()
                    }
                }

                function layout(type) {
                    if(scriteDocument.readOnly)
                        return

                    if(!hasItems) {
                        var rect = scriteDocument.structure.layoutElements(type)
                        canvasScroll.zoomFit(rect)
                        scriteDocument.structure.forceBeatBoardLayout = false
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
                            scriteDocument.structure.forceBeatBoardLayout = false
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
                        Qt.callLater( function() { canvasMenu.close() } )
                        newSceneButton.activeColor = color
                        if(canvasMenu.isContextMenu)
                            canvas.createItem("element", Qt.point(canvasMenu.x-130,canvasMenu.y-22), newSceneButton.activeColor)
                        else
                            createItemMouseHandler.handle("element")
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
                                Qt.callLater( function() { canvasMenu.close() } )
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
                    text: "Add To Timeline"
                    property Scene lastScene: scriteDocument.screenplay.elementCount > 0 && scriteDocument.screenplay.elementAt(scriteDocument.screenplay.elementCount-1).scene
                    enabled: elementContextMenu.element !== null && elementContextMenu.element.scene !== lastScene
                    onClicked: {
                        var lastScreenplayScene = null
                        if(scriteDocument.screenplay.elementCount > 0)
                            lastScreenplayScene = scriteDocument.screenplay.elementAt(scriteDocument.screenplay.elementCount-1).scene
                        if(lastScreenplayScene === null || elementContextMenu.element.scene !== lastScreenplayScene)
                            scriteDocument.screenplay.addScene(elementContextMenu.element.scene)
                        elementContextMenu.element = null
                    }
                }

                StructureGroupsMenu {
                    sceneGroup: SceneGroup {
                        structure: scriteDocument.structure
                    }
                    onToggled: app.execLater(selection, 250, function() { selection.refit() })
                    onAboutToShow: {
                        sceneGroup.clearScenes()
                        sceneGroup.addScene(elementContextMenu.element.scene)
                    }
                    onClosed: sceneGroup.clearScenes()
                }

                MenuSeparator { }

                MenuItem2 {
                    text: "Delete"
                    enabled: elementContextMenu.element !== null
                    onClicked: {
                        canvasScroll.deleteCurrentElement(elementContextMenu.element, this)
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
        property alias interacting: panMouseArea.pressed

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

        onVisibleChanged: {
            if(visible)
                canvasItemsBoundingBox.markPreviewDirty()
        }

        BoundingBoxPreview {
            id: previewArea
            anchors.fill: parent
            anchors.margins: 5
            evaluator: canvasItemsBoundingBox
            backgroundColor: primaryColors.c100.background
            backgroundOpacity: 0.9

            Rectangle {
                id: viewportIndicator
                color: primaryColors.highlight.background
                opacity: 0.5

                DelayedPropertyBinder {
                    id: geometryBinder
                    initial: Qt.rect(0,0,0,0)
                    set: {
                        if(!canvasPreview.visible)
                            return Qt.rect(0,0,0,0)

                        var visibleRect = canvasScroll.viewportRect
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
                    delay: 10
                }

                x: geometryBinder.get.x
                y: geometryBinder.get.y
                width: geometryBinder.get.width
                height: geometryBinder.get.height

                onXChanged: {
                    if(panMouseArea.drag.active)
                        panViewport()
                }
                onYChanged: {
                    if(panMouseArea.drag.active)
                        panViewport()
                }

                function panViewport() {
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
                    drag.onActiveChanged: canvasScroll.animatePanAndZoom = !drag.active
                }
            }
        }
    }

    Loader {
        width: parent.width*0.7
        anchors.centerIn: parent
        active: scriteDocument.structure.elementCount === 0 && scriteDocument.structure.annotationCount === 0
        sourceComponent: Text {
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 30
            enabled: false
            color: primaryColors.c600.background
            // renderType: Text.NativeRendering
            text: "Create scenes by clicking on the 'Add Scene' button OR right click to see options."
        }
    }

    Component {
        id: newStructureElementComponent

        StructureElement {
            objectName: "newElement"
            scene: Scene {
                title: scriteDocument.structure.canvasUIMode === Structure.IndexCardUI ? "" : "New Scene"
                heading.locationType: "INT"
                heading.location: "SOMEWHERE"
                heading.moment: "DAY"
            }
            Component.onCompleted: {
                scene.undoRedoEnabled = true
                scriteDocument.structure.forceBeatBoardLayout = false
            }
        }
    }

    // This is the old style structure element delegate, where we were only showing the synopsis.
    Component {
        id: structureElementSynopsisEditorUIDelegate

        Item {
            id: elementItem
            property StructureElement element: modelData
            Component.onCompleted: element.follow = elementItem
            enabled: selection.active === false

            BoundingBoxItem.evaluator: canvasItemsBoundingBox
            BoundingBoxItem.stackOrder: 3.0 + (index/scriteDocument.structure.elementCount)
            BoundingBoxItem.livePreview: false
            BoundingBoxItem.previewFillColor: selected ? Qt.darker(element.scene.color) : element.scene.color
            BoundingBoxItem.previewBorderColor: app.isLightColor(element.scene.color) ? "black" : background.color
            BoundingBoxItem.previewBorderWidth: selected ? 3 : 1.5
            BoundingBoxItem.viewportItem: canvas
            BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
            BoundingBoxItem.viewportRect: canvasScroll.viewportRect

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
                    if(!scriteDocument.readOnly) {
                        titleText.editMode = true
                        if(canvasScroll.mouseOverItem === elementItem)
                            canvasScroll.mouseOverItem = null
                    }
                }
                onClicked: {
                    annotationGripLoader.reset()
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    requestEditorLater()
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
                        scriteDocument.structure.forceBeatBoardLayout = false
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
                    elementContextMenu.popup()
                }
            }

            // Drag to timeline support
            Drag.active: dragMouseArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.LinkAction
            Drag.hotSpot.x: elementItem.width/2 // dragHandle.x + dragHandle.width/2
            Drag.hotSpot.y: elementItem.height/2 // dragHandle.y + dragHandle.height/2
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
                    hoverEnabled: !canvasScroll.flicking && !canvasScroll.moving && elementItem.selected
                    anchors.fill: parent
                    drag.target: parent
                    cursorShape: Qt.SizeAllCursor
                    onContainsMouseChanged: {
                        if(containsMouse)
                            canvasScroll.maybeDragItem = elementItem
                        else if(canvasScroll.maybeDragItem === elementItem)
                            canvasScroll.maybeDragItem = null
                    }
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

    // This is the new style structure element delegate, where we are showing index cards like UI
    // on the structure canvas.
    Component {
        id: structureElementIndexCardUIDelegate

        Item {
            id: elementItem
            property StructureElement element: modelData
            property int elementIndex: index
            property bool selected: scriteDocument.structure.currentElementIndex === index
            z: selected ? 1 : 0

            function select() {
                scriteDocument.structure.currentElementIndex = index
            }

            function activate() {
                canvasTabSequence.releaseFocus()
                annotationGripLoader.reset()
                scriteDocument.structure.currentElementIndex = index
                requestEditorLater()
            }

            function finishEditing() {
                if(canvasScroll.editItem === elementItem)
                    canvasScroll.editItem = null
                canvasTabSequence.releaseFocus()
            }

            function zoomOneForFocus() {
                if(canvas.scale < 0.65)
                    canvasScroll.zoomOneToItem(elementItem)
            }

            property bool visibleInViewport: true
            property StructureElementStack elementStack
            property bool stackedOnTop: (elementStack === null || elementStack.topmostElement === element)
            visible: visibleInViewport && stackedOnTop
            onVisibleChanged: {
                if(!visible) {
                    if(headingField.activeFocus || synopsisField.activeFocus)
                        canvasTabSequence.releaseFocus()
                }
            }

            function determineElementStack() {
                if(element.stackId === "")
                    elementStack = null
                else if(elementStack === null || elementStack.stackId !== element.stackId)
                    elementStack = scriteDocument.structure.elementStacks.findStackById(element.stackId)
            }

            TrackerPack {
                delay: 250
                TrackSignal { target: element; signal: "stackIdChanged()" }
                TrackSignal { target: scriteDocument.structure.elementStacks; signal: "objectCountChanged()" }
                TrackSignal { target: elementStack; signal: "objectCountChanged()" }
                TrackSignal { target: elementStack; signal: "stackLeaderChanged()" }
                TrackSignal { target: elementStack; signal: "topmostElementChanged()" }
                onTracked: elementItem.determineElementStack()
            }

            Component.onCompleted: {
                element.follow = elementItem
                elementItem.determineElementStack()
            }

            BoundingBoxItem.evaluator: canvasItemsBoundingBox
            BoundingBoxItem.stackOrder: 3.0 + (index/scriteDocument.structure.elementCount)
            BoundingBoxItem.livePreview: false
            BoundingBoxItem.previewFillColor: app.translucent(element.scene.color, selected ? 0.75 : 0.1)
            BoundingBoxItem.previewBorderColor: app.isLightColor(element.scene.color) ? "black" : element.scene.color
            BoundingBoxItem.previewBorderWidth: selected ? 3 : 1.5
            BoundingBoxItem.viewportItem: canvas
            BoundingBoxItem.visibilityMode: stackedOnTop ? BoundingBoxItem.VisibleUponViewportIntersection : BoundingBoxItem.IgnoreVisibility
            BoundingBoxItem.viewportRect: canvasScroll.viewportRect
            BoundingBoxItem.visibilityProperty: "visibleInViewport"

            onSelectedChanged: {
                if(selected && (mainUndoStack.structureEditorActive || scriteDocument.structure.elementCount === 1))
                    synopsisField.forceActiveFocus()
                else
                    canvasTabSequence.releaseFocus()
            }

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

            width: 350
            height: indexCardLayout.height + 20

            Rectangle {
                id: background
                anchors.fill: parent
                color: Qt.tint(element.scene.color, selected ? "#C0FFFFFF" : "#F0FFFFFF")
                border.width: elementItem.selected ? 2 : 1
                border.color: app.isLightColor(element.scene.color) ? "black" : element.scene.color

                // Move index-card around
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onPressed: {
                        elementItem.select()
                        canvas.forceActiveFocus()
                    }

                    drag.target: scriteDocument.readOnly ? null : elementItem
                    drag.axis: Drag.XAndYAxis
                    drag.minimumX: 0
                    drag.minimumY: 0
                    drag.onActiveChanged: {
                        canvas.forceActiveFocus()
                        scriteDocument.structure.currentElementIndex = index
                        if(drag.active === false) {
                            elementItem.x = scriteDocument.structure.snapToGrid(elementItem.x)
                            elementItem.y = scriteDocument.structure.snapToGrid(elementItem.y)
                            scriteDocument.structure.forceBeatBoardLayout = false
                        }
                    }
                }

                // Context menu support for index card
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: {
                        canvasTabSequence.releaseFocus()
                        canvas.forceActiveFocus()
                        elementItem.select()
                        elementContextMenu.element = elementItem.element
                        elementContextMenu.popup()
                    }
                }
            }

            property bool focus2: headingField.activeFocus | synopsisField.activeFocus
            onFocus2Changed: {
                if(focus2)
                    canvasScroll.editItem = elementItem
                else if(canvasScroll.editItem === elementItem)
                    canvasScroll.editItem = null
            }

            Column {
                id: indexCardLayout
                width: parent.width - 20
                anchors.centerIn: parent
                spacing: 10

                Rectangle {
                    width: parent.width
                    height: 10
                    color: selected ? element.scene.color : Qt.tint(element.scene.color, "#90FFFFFF")
                    border.color: app.isLightColor(element.scene.color) ? "gray" : element.scene.color
                    border.width: 1
                }

                TextField2 {
                    id: headingField
                    width: parent.width
                    text: element.title
                    enabled: true
                    label: "Scene Heading"
                    labelAlwaysVisible: true
                    placeholderText: "Index Card Title"
                    maximumLength: 140
                    font.family: scriteDocument.formatting.defaultFont.family
                    font.bold: true
                    font.capitalization: Font.AllUppercase
                    font.pointSize: app.idealFontPointSize
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    readOnly: scriteDocument.readOnly || canvas.scale < 0.5
                    onEditingComplete: { element.title = text; TabSequenceItem.focusNext() }
                    onActiveFocusChanged: {
                        if(activeFocus) {
                            elementItem.select()
                            if(!readOnly)
                                elementItem.zoomOneForFocus()
                        }
                    }
                    Keys.onEscapePressed: canvasTabSequence.releaseFocus()
                    enableTransliteration: true
                    property var currentLanguage: app.transliterationEngine.language
                    onCurrentLanguageChanged: {
                        if(currentLanguage !== TransliterationEngine.English)
                            font.capitalization = Font.MixedCase
                        else
                            font.capitalization = Font.AllUppercase
                    }
                    TabSequenceItem.enabled: elementItem.stackedOnTop
                    TabSequenceItem.manager: canvasTabSequence
                    TabSequenceItem.sequence: {
                        var indexes = element.scene.screenplayElementIndexList
                        if(indexes.length === 0)
                            return elementIndex * 2 + 0
                        return (indexes[0] + scriteDocument.structure.elementCount) * 2 + 0
                    }
                    TabSequenceItem.onAboutToReceiveFocus: scriteDocument.structure.currentElementIndex = elementIndex
                }

                Column {
                    spacing: 0
                    width: parent.width

                    Text {
                        id: labelText
                        text: "Synopsis"
                        font.pointSize: app.idealFontPointSize/2
                    }

                    Flickable {
                        id: synopsisFieldFlick
                        clip: true
                        width: parent.width
                        height: 200
                        contentWidth: synopsisField.width
                        contentHeight: synopsisField.height
                        interactive: synopsisField.activeFocus && scrollBarVisible
                        property bool scrollBarVisible: synopsisField.height > synopsisFieldFlick.height
                        ScrollBar.vertical: ScrollBar {
                            policy: synopsisFieldFlick.scrollBarVisible ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                        }
                        flickableDirection: Flickable.VerticalFlick
                        TextArea {
                            id: synopsisField
                            width: synopsisFieldFlick.scrollBarVisible ? synopsisFieldFlick.width-20 : synopsisFieldFlick.width
                            height: Math.max(synopsisFieldFlick.height-1, synopsisField.contentHeight+50)
                            background: Item { }
                            selectByMouse: true
                            selectByKeyboard: true
                            Transliterator.textDocument: textDocument
                            Transliterator.cursorPosition: cursorPosition
                            Transliterator.hasActiveFocus: activeFocus
                            placeholderText: "Describe what happens in this scene."
                            font.pointSize: app.idealFontPointSize
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            readOnly: scriteDocument.readOnly || canvas.scale < 0.5
                            text: element.scene.title
                            onTextChanged: element.scene.title = text
                            onActiveFocusChanged: {
                                if(activeFocus) {
                                    elementItem.select()
                                    if(!readOnly)
                                        elementItem.zoomOneForFocus()
                                }
                            }
                            Keys.onEscapePressed: canvasTabSequence.releaseFocus()
                            SpecialSymbolsSupport {
                                anchors.top: parent.bottom
                                anchors.left: parent.left
                                textEditor: synopsisField
                                textEditorHasCursorInterface: true
                                enabled: !scriteDocument.readOnly
                            }
                            onCursorRectangleChanged: {
                                var y1 = cursorRectangle.y
                                var y2 = cursorRectangle.y + cursorRectangle.height
                                if(y1 < synopsisFieldFlick.contentY)
                                    synopsisFieldFlick.contentY = Math.max(y1-10, 0)
                                else if(y2 > synopsisFieldFlick.contentY + synopsisFieldFlick.height)
                                    synopsisFieldFlick.contentY = y2+10 - synopsisFieldFlick.height
                            }
                            TabSequenceItem.enabled: elementItem.stackedOnTop
                            TabSequenceItem.manager: canvasTabSequence
                            TabSequenceItem.sequence: {
                                var indexes = element.scene.screenplayElementIndexList
                                if(indexes.length === 0)
                                    return elementIndex * 2 + 1
                                return (indexes[0] + scriteDocument.structure.elementCount) * 2 + 1
                            }
                            TabSequenceItem.onAboutToReceiveFocus: scriteDocument.structure.currentElementIndex = elementIndex
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: synopsisField.hovered ? 2 : 1
                        color: synopsisField.hovered ? "black" : primaryColors.borderColor
                    }
                }

                Text {
                    font.pointSize: app.idealAppFontSize - 2
                    width: element.scene.hasCharacters ? characterList.width : parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: width < contentWidth ? Text.AlignHCenter : Text.AlignLeft
                    text: scriteDocument.structure.presentableGroupNames(element.scene.groups)
                    visible: element.scene.groups.length > 0
                }

                Item {
                    width: parent.width
                    height: Math.max(characterList.height, dragHandle.height)

                    SceneTypeImage {
                        id: sceneTypeImage
                        width: 24; height: 24
                        opacity: 0.5
                        showTooltip: false
                        sceneType: element.scene.type
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        visible: sceneType !== Scene.Standard
                    }

                    Text {
                        id: characterList
                        font.pointSize: app.idealAppFontSize - 2
                        anchors.left: sceneTypeImage.right
                        anchors.right: dragHandle.left
                        anchors.margins: 5
                        anchors.verticalCenter: parent.verticalCenter
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        horizontalAlignment: width < contentWidth ? Text.AlignHCenter : Text.AlignLeft
                        opacity: element.scene.hasCharacters ? 1 : 0
                        text: {
                            if(element.scene.hasCharacters)
                                return "<b>Characters</b>: " + element.scene.characterNames.join(", ")
                            return ""
                        }
                    }

                    Image {
                        id: dragHandle
                        source: "../icons/action/view_array.png"
                        width: 24; height: 24
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        scale: dragHandleMouseArea.containsMouse ? 2 : 1
                        opacity: dragHandleMouseArea.containsMouse ? 1 : 0.1
                        Behavior on scale {
                            enabled: screenplayEditorSettings.enableAnimations
                            NumberAnimation { duration: 250 }
                        }

                        MouseArea {
                            id: dragHandleMouseArea
                            anchors.fill: parent
                            hoverEnabled: !canvasScroll.flicking && !canvasScroll.moving && elementItem.selected
                            drag.target: parent
                            cursorShape: Qt.SizeAllCursor
                            drag.onActiveChanged: {
                                if(drag.active)
                                    canvas.forceActiveFocus()
                            }
                            onContainsMouseChanged: {
                                if(containsMouse)
                                    canvasScroll.maybeDragItem = elementItem
                                else if(canvasScroll.maybeDragItem === elementItem)
                                    canvasScroll.maybeDragItem = null
                            }
                            onPressed: {
                                canvas.forceActiveFocus()
                                elementItem.grabToImage(function(result) {
                                    elementItem.Drag.imageSource = result.url
                                })
                            }
                        }
                    }
                }

                // Drag to timeline support
                Drag.active: dragHandleMouseArea.drag.active
                Drag.dragType: Drag.Automatic
                Drag.supportedActions: Qt.LinkAction
                Drag.mimeData: { "scrite/sceneID": element.scene.id }
                Drag.source: element.scene
            }

            // Accept drops for stacking items on top of each other.
            Rectangle {
                anchors.fill: parent
                anchors.margins: -10
                border.width: 2
                border.color: app.translucent("black", alpha)
                color: app.translucent("#cfd8dc", alpha)
                radius: 6
                property real alpha: 0
                enabled: !dragHandleMouseArea.drag.active && element.scene.addedToScreenplay

                DropArea {
                    anchors.fill: parent
                    keys: ["scrite/sceneID"]
                    onEntered: parent.alpha = 0.5
                    onExited: parent.alpha = 0
                    onDropped: {
                        parent.alpha = 0

                        var otherScene = app.typeName(drop.source) === "ScreenplayElement" ? drop.source.scene : drop.source
                        if(scriteDocument.screenplay.firstIndexOfScene(otherScene) < 0) {
                            showInformation({
                                "message": "Scenes must be added to the timeline before they can be stacked."
                            })
                            drop.ignore()
                            return
                        }

                        var otherSceneId = otherScene.id
                        if(otherSceneId === element.scene.id) {
                            drop.ignore()
                            return
                        }

                        var otherElement = scriteDocument.structure.findElementBySceneID(otherSceneId)
                        if(otherElement === null) {
                            drop.ignore()
                            return
                        }

                        if(element.scene.actIndex < 0 || otherElement.scene.actIndex < 0) {
                            showInformation({
                                "message": "Scenes must be added to the timeline before they can be stacked."
                            })
                            drop.ignore()
                            return
                        }

                        if(element.scene.actIndex !== otherElement.scene.actIndex) {
                            showInformation({
                                "message": "Scenes must belong to the same act for them to be stacked."
                            })
                            drop.ignore()
                            return
                        }

                        var otherElementIndex = scriteDocument.structure.indexOfElement(otherElement)
                        Qt.callLater( function() { scriteDocument.structure.currentElementIndex = otherElementIndex } )

                        var myStackId = element.stackId
                        var otherStackId = otherElement.stackId
                        drop.acceptProposedAction()

                        if(myStackId === "") {
                            var uid = app.createUniqueId()
                            element.stackId = uid
                            otherElement.stackId = uid
                        } else {
                            otherElement.stackId = myStackId
                        }

                        Qt.callLater( function() { element.stackLeader = true } )
                    }
                }
            }
        }
    }

    Component {
        id: elementConnectorComponent

        StructureElementConnector {
            lineType: StructureElementConnector.CurvedLine
            fromElement: connectorFromElement
            toElement: connectorToElement
            arrowAndLabelSpacing: labelBg.width
            outlineWidth: app.devicePixelRatio*canvas.scale*structureCanvasSettings.connectorLineWidth
            visible: {
                if(canBeVisible)
                    return intersects(canvasScroll.viewportRect)
                return false
            }

            Rectangle {
                id: labelBg
                width: Math.max(labelItem.width,labelItem.height)+20
                height: width; radius: width/2
                border.width: 1; border.color: primaryColors.borderColor
                x: parent.suggestedLabelPosition.x - radius
                y: parent.suggestedLabelPosition.y - radius
                color: Qt.tint(parent.outlineColor, "#E0FFFFFF")
                visible: !canvasPreview.updatingThumbnail

                Text {
                    id: labelItem
                    anchors.centerIn: parent
                    font.pixelSize: 12
                    text: connectorLabel
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

            Component.onCompleted: canvas.forceActiveFocus()

            property real gripSize: 10 * onePxSize
            property real onePxSize: Math.max(1, 1/canvas.scale)

            BoxShadow {
                anchors.fill: annotationToolBar
                opacity: 0.5
            }

            Rectangle {
                id: annotationToolBar
                anchors.left: parent.left
                anchors.bottom: parent.top
                anchors.bottomMargin: parent.gripSize
                color: primaryColors.c100.background
                height: annotationToolBarLayout.height+5
                width: annotationToolBarLayout.width+5
                border.width: 1
                border.color: primaryColors.borderColor

                Row {
                    id: annotationToolBarLayout
                    anchors.centerIn: parent

                    ToolButton3 {
                        iconSource: "../icons/action/edit.png"
                        ToolTip.text: "Edit properties of this annotation"
                        down: floatingDockWidget.visible
                        onClicked: structureCanvasSettings.displayAnnotationProperties = !structureCanvasSettings.displayAnnotationProperties
                        Connections {
                            target: floatingDockWidget
                            onCloseRequest: structureCanvasSettings.displayAnnotationProperties = false
                        }
                    }

                    ToolButton3 {
                        iconSource: "../icons/action/keyboard_arrow_up.png"
                        ToolTip.text: "Bring this annotation to front"
                        enabled: scriteDocument.structure.canBringToFront(annotationGripLoader.annotation)
                        onClicked: {
                            var a = annotationGripLoader.annotation
                            annotationGripLoader.reset()
                            scriteDocument.structure.bringToFront(a)
                        }
                    }

                    ToolButton3 {
                        iconSource: "../icons/action/keyboard_arrow_down.png"
                        ToolTip.text: "Send this annotation to back"
                        enabled: scriteDocument.structure.canSendToBack(annotationGripLoader.annotation)
                        onClicked: {
                            var a = annotationGripLoader.annotation
                            annotationGripLoader.reset()
                            scriteDocument.structure.sendToBack(a)
                        }
                    }

                    ToolButton3 {
                        iconSource: "../icons/action/delete.png"
                        ToolTip.text: "Delete this annotation"
                        onClicked: {
                            var a = annotationGripLoader.annotation
                            annotationGripLoader.reset()
                            scriteDocument.structure.removeAnnotation(a)
                        }
                    }
                }
            }

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

            EventFilter.target: app
            EventFilter.active: !scriteDocument.readOnly && !floatingDockWidget.contentHasFocus && !modalDialog.active && !createItemMouseHandler.enabled
            EventFilter.events: [6]
            EventFilter.onFilter: {
                var dist = (event.controlModifier ? 5 : 1) * canvas.tickDistance
                switch(event.key) {
                case Qt.Key_Left:
                    if(event.shiftModifier) {
                        annotationGripItem.width -= annotation.resizable ? dist : 0
                        annotationGripItem.width = Math.max(annotationGripItem.width, 20)
                    } else
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
                    if(event.shiftModifier) {
                        annotationGripItem.height -= annotation.resizable ? dist : 0
                        annotationGripItem.height = Math.max(annotationGripItem.height, 20)
                    }
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
                case Qt.Key_F2:
                    if(structureCanvasSettings.displayAnnotationProperties === false) {
                        structureCanvasSettings.displayAnnotationProperties = true
                        result.accept = true
                        result.filter = true
                    }
                    break
                case Qt.Key_Escape:
                    annotationGripLoader.reset()
                    break
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

            function snapAnnotationGeometryToGrid(rect) {
                var gx = scriteDocument.structure.snapToGrid(rect.x)
                var gy = scriteDocument.structure.snapToGrid(rect.y)
                var gw = scriteDocument.structure.snapToGrid(rect.width)
                var gh = scriteDocument.structure.snapToGrid(rect.height)
                annotation.geometry = Qt.rect(gx, gy, gw, gh)
            }

            Timer {
                id: annotGeoUpdateTimer
                interval: geometryUpdateInterval
                onTriggered: {
                    snapAnnotationGeometryToGrid(Qt.rect(annotationGripItem.x,annotationGripItem.y,annotationGripItem.width,annotationGripItem.height))
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
                onPressed: canvas.forceActiveFocus()
                onDoubleClicked: {
                    if(structureCanvasSettings.displayAnnotationProperties === false)
                        structureCanvasSettings.displayAnnotationProperties = true
                }
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
                        snapAnnotationGeometryToGrid(Qt.rect(annotationGripItem.x, annotationGripItem.y, rightGrip.x + rightGrip.width/2, annotationGripItem.height))
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeHorCursor
                    drag.target: parent
                    drag.axis: Drag.XAxis
                    drag.minimumX: 20
                    onPressed: canvas.forceActiveFocus()
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
                        snapAnnotationGeometryToGrid(Qt.rect(annotationGripItem.x, annotationGripItem.y, annotationGripItem.width, bottomGrip.y + bottomGrip.height/2))
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeVerCursor
                    drag.target: parent
                    drag.axis: Drag.YAxis
                    drag.minimumY: 20
                    onPressed: canvas.forceActiveFocus()
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
                        snapAnnotationGeometryToGrid(Qt.rect(annotationGripItem.x, annotationGripItem.y, bottomRightGrip.x + bottomRightGrip.width/2, bottomRightGrip.y + bottomRightGrip.height/2))
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeFDiagCursor
                    drag.target: parent
                    drag.axis: Drag.XAndYAxis
                    drag.minimumX: 20
                    drag.minimumY: 20
                    onPressed: canvas.forceActiveFocus()
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

        }
    }

    function createNewOvalAnnotation(x, y) {
        if(scriteDocument.readOnly)
            return

        var w = 80
        var h = 80
        var rect =Qt.rect(x - w/2, y-h/2, w, h)
        var annot = annotationObject.createObject(canvas)
        annot.type = "oval"
        annot.geometry = rect
        scriteDocument.structure.addAnnotation(annot)
    }

    Component {
        id: ovalAnnotationComponent

        AnnotationItem {
            color: Qt.rgba(0,0,0,0)
            border.width: 0
            border.color: Qt.rgba(0,0,0,0)

            PainterPathItem {
                id: ovalPathItem
                anchors.fill: parent
                anchors.margins: annotation.attributes.borderWidth
                renderType: annotation.attributes.fillBackground ? PainterPathItem.OutlineAndFill : PainterPathItem.OutlineOnly
                renderingMechanism: PainterPathItem.UseOpenGL
                fillColor: annotation.attributes.color
                outlineColor: annotation.attributes.borderColor
                outlineWidth: annotation.attributes.borderWidth
                painterPath: PainterPath {
                    MoveTo {
                        x: ovalPathItem.width
                        y: ovalPathItem.height/2
                    }
                    ArcTo {
                        rectangle: Qt.rect(0, 0, ovalPathItem.width, ovalPathItem.height)
                        startAngle: 0
                        sweepLength: 360
                    }
                }
            }
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
            id: textAnnotationItem

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
                    if(isUrlValid) {
                        var annotAttrs = annotation.attributes
                        annotation.removeImage(annotAttrs.imageName)
                        annotAttrs.imageName = ""
                        annotation.attributes = annotAttrs
                    }
                }
                onStatusChanged: {
                    if(status === UrlAttributes.Ready && isUrlValid) {
                        var annotAttrs = annotation.attributes
                        var urlAttrs = attributes
                        annotAttrs.title = urlAttrs.title
                        annotAttrs.description = urlAttrs.description
                        annotAttrs.imageName = ""
                        annotAttrs.imageUrl = urlAttrs.image
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
                        maximumLineCount: 3
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
                height: sourceSize.height / sourceSize.width * width
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.Stretch
                smooth: canvasScroll.moving || canvasScroll.flicking ? false : true
                mipmap: smooth
                source: annotation.imageUrl(annotation.attributes.image)
                asynchronous: true
                onStatusChanged: {
                    if(status === Image.Ready)
                        parent.BoundingBoxItem.markPreviewDirty()
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

    function requestEditorLater() {
        app.execLater(structureView, 100, function() { requestEditor() })
    }
}
