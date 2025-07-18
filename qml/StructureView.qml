/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/overlays"
import "qrc:/qml/structure"

Item {
    id: structureView
    signal requestEditor()
    signal releaseEditor()

    readonly property real toolbarSize: toolbar.width
    readonly property size maxDragImageSize: Qt.size(36, 36)

    Rectangle {
        id: toolbar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: statusBar.top
        color: Runtime.colors.primary.c100.background
        width: toolbarLayout.width+4

        Flow {
            id: toolbarLayout
            spacing: 1
            height: parent.height-5
            anchors.horizontalCenter: parent.horizontalCenter
            flow: Flow.TopToBottom
            layoutDirection: Qt.RightToLeft
            property real columnWidth: structureTabButton.width

            FlatToolButton {
                id: structureTabButton
                visible: Runtime.showNotebookInStructure
                iconSource: "qrc:/icons/navigation/structure_tab.png"
                down: true
                ToolTip.text: "Structure\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
            }

            FlatToolButton {
                id: notebookTabButton
                visible: Runtime.showNotebookInStructure
                iconSource: "qrc:/icons/navigation/notebook_tab.png"
                ToolTip.text: "Notebook Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                onClicked: Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Notebook")
            }

            Rectangle {
                width: toolbarLayout.columnWidth
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            FlatToolButton {
                id: newSceneButton
                down: newSceneMenu.visible
                enabled: !Scrite.document.readOnly
                onClicked: newSceneMenu.open()
                iconSource: "qrc:/icons/action/add_scene.png"
                ToolTip.text: "Add Scene"
                hasMenu: true
                property color activeColor: "white"

                Item {
                    anchors.top: parent.top
                    anchors.right: parent.right

                    VclMenu {
                        id: newSceneMenu

                        VclMenuItem {
                            text: "New Scene"
                            enabled: !Scrite.document.readOnly
                            onClicked: {
                                Qt.callLater( function() { newSceneMenu.close() } )
                                createItemMouseHandler.handle("element")
                            }
                        }

                        ColorMenu {
                            title: "Colored Scene"
                            selectedColor: newSceneButton.activeColor
                            enabled: !Scrite.document.readOnly
                            onMenuItemClicked: {
                                Qt.callLater( function() { newSceneMenu.close() } )
                                newSceneButton.activeColor = color
                                createItemMouseHandler.handle("element")
                            }
                        }
                    }
                }
            }

            FlatToolButton {
                id: newAnnotationButton
                down: newAnnotationMenu.visible
                enabled: !Scrite.document.readOnly
                onClicked: newAnnotationMenu.open()
                iconSource: "qrc:/icons/action/add_annotation.png"
                ToolTip.text: "Add Annotation"
                hasMenu: true

                Item {
                    id: newAnnotationMenuArea
                    anchors.top: parent.top
                    anchors.right: parent.right

                    VclMenu {
                        id: newAnnotationMenu

                        Repeater {
                            model: canvas.annotationsList

                            VclMenuItem {
                                property var annotationInfo: canvas.annotationsList[index]
                                text: annotationInfo.title
                                enabled: !Scrite.document.readOnly && annotationInfo.what !== ""
                                onClicked: {
                                    Qt.callLater( function() { newAnnotationMenu.close() } )
                                    createItemMouseHandler.handle(annotationInfo.what)
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: toolbarLayout.columnWidth
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            FlatToolButton {
                id: selectionModeButton
                enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2)
                iconSource: "qrc:/icons/action/selection_drag.png"
                ToolTip.text: "Selection mode"
                checkable: true
                onClicked: selection.layout(Structure.HorizontalLayout)
            }

            FlatToolButton {
                enabled: !Scrite.document.readOnly && Scrite.document.structure.elementCount >= 2
                iconSource: "qrc:/icons/content/select_all.png"
                ToolTip.text: "Select All"
                onClicked: selection.init(elementItems, canvasItemsBoundingBox.boundingBox, true)
            }

            FlatToolButton {
                enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
                iconSource: "qrc:/icons/action/layout_options.png"
                ToolTip.text: "Layout Options"
                down: layoutOptionsMenu.visible
                onClicked: layoutOptionsMenu.visible = true
                hasMenu: true

                Item {
                    anchors.top: parent.top
                    anchors.right: parent.right

                    VclMenu {
                        id: layoutOptionsMenu
                        width: 250

                        VclMenuItem {
                            icon.source: "qrc:/icons/action/layout_horizontally.png"
                            text: "Layout Horizontally"
                            onClicked: selection.layout(Structure.HorizontalLayout)
                        }

                        VclMenuItem {
                            icon.source: "qrc:/icons/action/layout_vertically.png"
                            text: "Layout Vertically"
                            onClicked: selection.layout(Structure.VerticalLayout)
                        }

                        VclMenuItem {
                            icon.source: "qrc:/icons/action/layout_flow_horizontally.png"
                            text: "Flow Horizontally"
                            onClicked: selection.layout(Structure.FlowHorizontalLayout)
                        }

                        VclMenuItem {
                            icon.source: "qrc:/icons/action/layout_flow_vertically.png"
                            text: "Flow Vertically"
                            onClicked: selection.layout(Structure.FlowVerticalLayout)
                        }
                    }
                }
            }

            FlatToolButton {
                id: beatBoardLayoutToolButton
                enabled: !Scrite.document.readOnly
                iconSource: "qrc:/icons/action/layout_beat_sheet.png"
                ToolTip.text: "Beat Board Layout"
                checkable: true
                checked: false
                onToggled: {
                    canvasPreview.allowed = false
                    Scrite.document.structure.forceBeatBoardLayout = checked
                    if(checked && Scrite.document.structure.elementCount > 0) {
                        Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)
                    }
                    Utils.execLater(canvasPreview, 1000, function() {
                        cmdZoomOne.click()
                        canvasPreview.allowed = true
                    })
                }

                Component.onCompleted: checked = Scrite.document.structure.forceBeatBoardLayout

                Connections {
                    target: Scrite.document.structure

                    function onForceBeatBoardLayoutChanged() {
                        beatBoardLayoutToolButton.checked = Scrite.document.structure.forceBeatBoardLayout
                    }

                    function onIndexCardFieldsChanged() {
                        Qt.callLater( function() {
                            Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)
                        })
                    }
                }

                Connections {
                    target: Scrite.document.screenplay
                    enabled: Scrite.document.structure.forceBeatBoardLayout
                    function onElementRemoved(element, index) {
                        Qt.callLater( function() {
                            Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)
                        })
                    }
                }
            }

            FlatToolButton {
                iconSource: "qrc:/icons/action/layout_grouping.png"
                ToolTip.text: "Grouping Options"
                onClicked: layoutGroupingMenu.popup()
                down: layoutGroupingMenu.visible

                VclMenu {
                    id: layoutGroupingMenu
                    width: 350

                    VclMenuItem {
                        text: "Acts"
                        font.bold: canvas.groupCategory === ""
                        onTriggered: canvas.groupCategory = ""
                    }

                    Repeater {
                        model: Scrite.document.structure.groupCategories

                        VclMenuItem {
                            text: Scrite.app.camelCased(modelData)
                            font.bold: canvas.groupCategory === modelData
                            onTriggered: canvas.groupCategory = modelData
                        }
                    }

                    MenuSeparator { }

                    VclMenuItem {
                        text: "Customise"
                        onTriggered: StructureStoryBeatsDialog.launch()
                    }
                }
            }

            FlatToolButton {
                id: tagMenuOption
                iconSource: "qrc:/icons/action/tag.png"
                enabled: (selection.hasItems || currentElementItemBinder.get !== null) && Scrite.document.structure.canvasUIMode === Structure.IndexCardUI
                ToolTip.text: {
                    if(selection.hasItems)
                        return "Tag the " + selection.items.length + " selected index card(s)"
                    else if(currentElementItemBinder.get !== null)
                        return "Tag the selected index card."
                    return ""
                }
                onClicked: tagMenuLoader.popup()
                down: tagMenuLoader.active

                MenuLoader {
                    id: tagMenuLoader
                    anchors.top: parent.top
                    anchors.right: parent.right

                    menu: StructureGroupsMenu {
                        innerTitle: tagMenuOption.ToolTip.text
                        sceneGroup: SceneGroup {
                            structure: Scrite.document.structure
                        }

                        onToggled: {
                            if(selection.hasItems)
                                Utils.execLater(selection, 250, function() { selection.refit() })
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

            FlatToolButton {
                id: changeColorOption
                enabled: (selection.hasItems || currentElementItemBinder.get !== null)
                property Scene scene: currentElementItemBinder.get ? currentElementItemBinder.get.element.scene :
                                      (selection.hasItems ? selection.items[0].element.scene : null)
                iconSource: scene ? "image://color/" + scene.color + "/1" : "image://color/gray/1"
                down: colorMenuLoader.active
                onClicked: colorMenuLoader.popup()
                ToolTip.text: "Change current scene(s) color."

                MenuLoader {
                    id: colorMenuLoader
                    anchors.top: parent.top
                    anchors.right: parent.right

                   menu: ColorMenu {
                        title: "Scenes Color"
                        onMenuItemClicked: {
                            if(selection.hasItems) {
                                var items = selection.items
                                items.forEach( function(item) {
                                    item.element.scene.color = color
                                })
                            } else {
                                currentElementItemBinder.get.element.scene.color = color
                            }
                            colorMenuLoader.active = false
                        }
                    }
                }
            }

            FlatToolButton {
                id: changeSceneTypeOption
                enabled: changeColorOption.enabled
                readonly property var sceneTypeModel: Scrite.app.enumerationModelForType("Scene", "Type")
                property Scene scene: currentElementItemBinder.get ? currentElementItemBinder.get.element.scene :
                                      (selection.hasItems ? selection.items[0].element.scene : null)
                property int sceneType: (scene && scene.type !== Scene.Standard) ? scene.type : Scene.Standard
                iconSource: {
                    if(sceneType === Scene.Standard)
                        return "qrc:/icons/content/standard_scene.png"
                    return sceneTypeModel[sceneType].icon
                }
                onClicked: sceneTypeMenuLoader.popup()
                down: sceneTypeMenuLoader.active
                ToolTip.text: enabled ? "Change scene type from '" + (sceneTypeModel[sceneType].key) + "' to something else." : ""

                MenuLoader {
                    id: sceneTypeMenuLoader
                    anchors.top: parent.top
                    anchors.right: parent.right
                    menu: MarkSceneAsMenu {
                        enableValidation: false
                        onTriggered: {
                            if(selection.hasItems) {
                                var items = selection.items
                                items.forEach( function(item) {
                                    item.element.scene.type = type
                                })
                            } else {
                                currentElementItemBinder.get.element.scene.type = type
                            }
                            sceneTypeMenuLoader.active = false
                        }
                    }
                }
            }

            FlatToolButton {
                id: deleteSceneOption
                enabled: !selection.hasItems && currentElementItemBinder.get
                iconSource: "qrc:/icons/action/delete.png"
                ToolTip.text: enabled ? "Delete selected scene." : ""
                onClicked: {
                    var element = currentElementItemBinder.get.element
                    if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI && element.follow)
                        element.follow.confirmAndDeleteSelf()
                    else
                        canvasScroll.deleteElement(element)
                }
            }

            Rectangle {
                width: toolbarLayout.columnWidth
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            FlatToolButton {
                enabled: !selection.hasItems && (annotationGripLoader.active || currentElementItemBinder.get !== null)
                iconSource: "qrc:/icons/content/content_copy.png"
                ToolTip.text: "Copy the selected scene or annotation."
                onClicked: {
                    if(annotationGripLoader.active) {
                        Scrite.document.structure.copy(annotationGripLoader.annotation)
                        AnimatedTextOverlay.show("Annotation Copied")
                    } else {
                        var spe = Scrite.document.structure.elementAt(Scrite.document.structure.currentElementIndex)
                        if(spe !== null) {
                            Scrite.document.structure.copy(spe)
                            AnimatedTextOverlay.show("Scene Copied")
                        }
                    }
                }
                shortcut: "Ctrl+C"
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Copy Annotation"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: Scrite.app.polishShortcutTextForDisplay("Ctrl+C")
            }

            FlatToolButton {
                enabled: !Scrite.document.readOnly && Scrite.document.structure.canPaste
                iconSource: "qrc:/icons/content/content_paste.png"
                ToolTip.text: "Paste from clipboard"
                onClicked: {
                    var gpos = Scrite.app.globalMousePosition()
                    var pos = canvasScroll.mapFromGlobal(gpos.x, gpos.y)
                    if(pos.x < 0 || pos.y < 0 || pos.x >= canvasScroll.width || pos.y >= canvasScroll.height)
                        Scrite.document.structure.paste()
                    else {
                        pos = canvas.mapFromGlobal(gpos.x, gpos.y)
                        Scrite.document.structure.paste(Qt.point(pos.x,pos.y))
                    }
                }
                shortcut: "Ctrl+V"
                ShortcutsModelItem.group: "Edit"
                ShortcutsModelItem.title: "Paste"
                ShortcutsModelItem.enabled: enabled
                ShortcutsModelItem.shortcut: Scrite.app.polishShortcutTextForDisplay(shortcut)
            }

            FlatToolButton {
                id: pdfExportButton
                iconSource: "qrc:/icons/file/generate_pdf.png"
                ToolTip.text: "Export the contents of the structure canvas to PDF."
                onClicked: ExportConfigurationDialog.launch(Scrite.document.structure.createExporterObject())
            }
        }

        Rectangle {
            width: 1
            height: parent.height
            anchors.right: parent.right
            color: Runtime.colors.primary.borderColor
        }
    }

    Rectangle {
        anchors.fill: canvasScroll
        color: Runtime.structureCanvasSettings.canvasColor
    }

    ScrollArea {
        id: canvasScroll
        anchors.left: toolbar.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: statusBar.top
        contentWidth: canvas.width * canvas.scale
        contentHeight: canvas.height * canvas.scale
        initialContentWidth: canvas.width
        initialContentHeight: canvas.height
        clip: true
        showScrollBars: Scrite.document.structure.elementCount >= 1
        zoomOnScroll: Runtime.workspaceSettings.mouseWheelZoomsInStructureCanvas
        interactive: !(rubberBand.active || selection.active || canvasPreview.interacting || annotationGripLoader.active) && mouseOverItem === null && editItem === null && maybeDragItem === null
        minimumScale: canvasItemsBoundingBox.itemCount > 0 ? Math.min(0.25, width/canvasItemsBoundingBox.width, height/canvasItemsBoundingBox.height) : 0.25
        property Item mouseOverItem
        property Item editItem
        property Item maybeDragItem
        property bool isZoomFit: false

        onEditItemChanged: {
            if(editItem) {
                Utils.execLater(canvasScroll, 500, function() {
                    if(!canvasScroll.editItem && canvas.scaleIsLessForEdit)
                        canvasScroll.zoomOneToItem(canvasScroll.editItem)
                })
            }
        }

        property rect viewportRect: Qt.rect( visibleArea.xPosition * contentWidth / canvas.scale,
                                           visibleArea.yPosition * contentHeight / canvas.scale,
                                           visibleArea.widthRatio * contentWidth / canvas.scale,
                                           visibleArea.heightRatio * contentHeight / canvas.scale )

        Connections {
            target: Scrite.document
            function onJustLoaded() { canvasScroll.updateFromScriteDocumentUserDataLater() }
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

        function enablePanAndZoomAnimation(delay) {
            if(animatePanAndZoom === true)
                return
            if(delay === undefined || delay === null)
                animatePanAndZoom = true;
            else
                Utils.execLater(canvasScroll, delay, () => {
                                        canvasScroll.animatePanAndZoom = true
                                     })
        }

        function updateScriteDocumentUserData() {
            if(!updateScriteDocumentUserDataEnabled || Scrite.document.readOnly || animatingPanOrZoom)
                return

            var userData = Scrite.document.userData
            userData["StructureView.canvasScroll"] = {
                "version": 0,
                "contentX": canvasScroll.contentX,
                "contentY": canvasScroll.contentY,
                "zoomScale": canvasScroll.zoomScale,
                "isZoomFit": canvasScroll.isZoomFit
            }
            Scrite.document.userData = userData
        }

        function updateFromScriteDocumentUserData() {
            if(elementItems.count < Scrite.document.structure.elementCount) {
                updateFromScriteDocumentUserDataLater()
                return
            }

            var userData = Scrite.document.userData
            var csData = userData["StructureView.canvasScroll"];
            if(csData && csData.version === 0) {
                canvasScroll.zoomScale = csData.zoomScale
                canvasScroll.contentX = csData.contentX
                canvasScroll.contentY = csData.contentY
                canvasScroll.isZoomFit = csData.isZoomFit === true
                if(canvasScroll.isZoomFit) {
                    Utils.execLater(canvasScroll, 500, function() {
                        var area = canvasItemsBoundingBox.boundingBox
                        canvasScroll.zoomFit(area)
                        canvasScroll.enablePanAndZoomAnimation(2000)
                    })
                }
            } else {
                if(Scrite.document.structure.elementCount > 0) {
                    var item = currentElementItemBinder.get
                    if(item === null)
                        item = elementItems.itemAt(0)
                    if(Runtime.firstSwitchToStructureTab)
                        canvasScroll.zoomOneToItem(item)
                    else
                        canvasScroll.ensureItemVisible(item, canvas.scale)
                } else
                    canvasScroll.zoomOneMiddleArea()
                canvasScroll.enablePanAndZoomAnimation(2000)
            }

            if(Scrite.document.structure.forceBeatBoardLayout)
                Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)

            updateScriteDocumentUserDataEnabled = true
            Runtime.screenplayEditorSettings.firstSwitchToStructureTab = false
            Runtime.firstSwitchToStructureTab = false
        }

        function updateFromScriteDocumentUserDataLater() {
            Utils.execLater(canvasScroll, 500, updateFromScriteDocumentUserData)
        }

        onUpdateScriteDocumentUserDataEnabledChanged: {
            if(updateScriteDocumentUserDataEnabled)
                Utils.execLater(canvasScroll, 500, zoomSanityCheck)
        }

        function zoomSanityCheck() {
            if( !Scrite.app.doRectanglesIntersect(canvasItemsBoundingBox.boundingBox, canvasScroll.viewportRect) ) {
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
            var visibleArea = Scrite.app.querySubRectangle(bbox, itemRect, atBest)
            canvasScroll.zoomFit(visibleArea)
        }

        function deleteElement(element) {
            if(element === null)
                return

            var nextScene = null
            var nextElement = null
            if(element.scene.addedToScreenplay) {
                nextElement = Scrite.document.screenplay.elementAt(element.scene.screenplayElementIndexList[0]+1)
                if(nextElement === null)
                    nextElement = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.lastSceneIndex())
                if(nextElement !== null)
                    nextScene = nextElement.scene
            } else {
                var idx = Scrite.document.structure.indexOfElement(element)
                var i = 0;
                for(i=idx+1; i<Scrite.document.structure.elementCount; i++) {
                    nextElement = Scrite.document.structure.elementAt(i)
                    if(nextElement.scene.addedToScreenplay)
                        continue;
                    nextScene = nextElement.scene
                    break
                }

                if(nextScene === null) {
                    for(i=0; i<idx; i++) {
                        nextElement = Scrite.document.structure.elementAt(i)
                        if(nextElement.scene.addedToScreenplay)
                            continue;
                        nextScene = nextElement.scene
                        break
                    }
                }
            }

            releaseEditor()
            Scrite.document.screenplay.removeSceneElements(element.scene)
            Scrite.document.structure.removeElement(element)

            Qt.callLater(function(scene) {
                if(Scrite.document.screenplay.elementCount === 0)
                    return
                if(scene === null)
                    scene = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.lastSceneIndex())
                var idx = Scrite.document.structure.indexOfScene(scene)
                if(idx >= 0) {
                    Scrite.document.structure.currentElementIndex = idx
                    Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.firstIndexOfScene(scene)
                }
            }, nextScene)
        }

        GridBackground {
            id: canvas
            antialiasing: false
            majorTickLineWidth: 2*Scrite.app.devicePixelRatio
            minorTickLineWidth: 1*Scrite.app.devicePixelRatio
            width: widthBinder.get
            height: heightBinder.get
            tickColorOpacity: 0.25 * scale
            scale: canvasScroll.suggestedScale
            border.width: 2
            border.color: Runtime.structureCanvasSettings.gridColor
            gridIsVisible: Runtime.structureCanvasSettings.showGrid && canvasScroll.interactive
            majorTickColor: Runtime.structureCanvasSettings.gridColor
            minorTickColor: Runtime.structureCanvasSettings.gridColor
            tickDistance: Scrite.document.structure.canvasGridSize
            transformOrigin: Item.TopLeft
            backgroundColor: canvasScroll.interactive ? Runtime.colors.primary.c10.background : Scrite.app.translucent(Runtime.colors.primary.c300.background, 0.75)
            Behavior on backgroundColor {
                enabled: Runtime.applicationSettings.enableAnimations
                ColorAnimation { duration: 250 }
            }

            property bool scaleIsLessForEdit: (350*canvas.scale < canvasScroll.height*0.25)
            onScaleIsLessForEditChanged: {
                if(scaleIsLessForEdit)
                    canvasTabSequence.releaseFocus()
            }

            TabSequenceManager {
                id: canvasTabSequence
                wrapAround: true
                releaseFocusEnabled: true
                onFocusWasReleased: canvas.forceActiveFocus()
            }

            function createItem(what, where) {
                if(Scrite.document.readOnly)
                    return

                if(what === undefined || what === "" | what === "element")
                    createElement(where.x, where.y, newSceneButton.activeColor)
                else
                    createAnnotation(what, where.x, where.y)
            }

            function createElement(x, y, c) {
                if(Scrite.document.readOnly)
                    return

                var props = {
                    "x": Math.max(Scrite.document.structure.snapToGrid(x), 130),
                    "y": Math.max(Scrite.document.structure.snapToGrid(y), 50)
                }

                var element = newStructureElementComponent.createObject(Scrite.document.structure, props)
                element.scene.color = c
                Scrite.document.structure.addElement(element)
                Scrite.document.structure.currentElementIndex = Scrite.document.structure.elementCount-1
                requestEditorLater()
                canvas.forceActiveFocus()
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
                if(Scrite.document.readOnly)
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
                initialRect: Scrite.document.structure.annotationsBoundingBox
                margin: 50
            }

            DelayedPropertyBinder {
                id: widthBinder
                initial: 1000
                set: Math.max( Math.ceil(canvasItemsBoundingBox.right / 100) * 100, 120000 )
                onGetChanged: Scrite.document.structure.canvasWidth = get
            }

            DelayedPropertyBinder {
                id: heightBinder
                initial: 1000
                set: Math.max( Math.ceil(canvasItemsBoundingBox.bottom / 100) * 100, 120000 )
                onGetChanged: Scrite.document.structure.canvasHeight = get
            }

            FocusTracker.window: Scrite.window
            FocusTracker.indicator.target: Runtime.undoStack
            FocusTracker.indicator.property: "structureEditorActive"

            MouseArea {
                id: createItemMouseHandler
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                enabled: false
                hoverEnabled: true
                property bool enabled2: enabled
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
                    source: parent.what === "element" ? "qrc:/icons/action/add_scene.png" : "qrc:/icons/action/add_annotation.png"
                    property real halfSize: width/2
                    x: parent.mouseX - halfSize
                    y: parent.mouseY - halfSize
                    visible: parent.enabled
                }

                EventFilter.target: Scrite.app
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
                    if(!Scrite.document.readOnly) {
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
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: annotationGripLoader.active
                    onClicked: annotationGripLoader.reset()
                }

                StructureCanvasViewportFilterModel {
                    id: annotationsFilterModel
                    enabled: Scrite.document.loading ? false : Scrite.document.structure.annotationCount > 100
                    structure: Scrite.document.structure
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
                    Component.onDestruction: reset()

                    function reset() {
                        annotation = null
                        annotationItem = null
                    }

                    Connections {
                        target: createItemMouseHandler
                        function onEnabled2Changed() {
                            if(canvasScroll.enabled)
                                annotationGripLoader.reset()
                        }
                    }

                    Connections {
                        target: canvasScroll
                        function onEditItemChanged() {
                            if(canvasScroll.editItem !== null)
                                annotationGripLoader.reset()
                        }
                    }

                    Connections {
                        target: canvasContextMenu
                        function onVisibleChanged() {
                            if(canvasContextMenu.visible)
                                annotationGripLoader.reset()
                        }
                    }

                    Connections {
                        target: Scrite.document.structure
                        function onCurrentElementIndexChanged() {
                            if(Scrite.document.structure.currentElementIndex >= 0)
                                annotationGripLoader.reset()
                        }
                        function onAnnotationCountChanged() {
                            annotationGripLoader.reset()
                        }
                    }

                    Connections {
                        target: Scrite.document.screenplay
                        function onCurrentElementIndexChanged(val) {
                            var element = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.currentElementIndex)
                            var info = Scrite.document.structure.queryBreakElements(element)
                            if(info.indexes && info.indexes.length > 0) {
                                var fi = info.indexes[0]
                                var fe = Scrite.document.structure.elementAt(fi)
                                if(fe === null)
                                    return
                                var febox = fe.geometry
                                var topPadding = element.breakType === Screenplay.Episode ? 150 : 90
                                febox = Qt.rect(febox.x-50, febox.y-topPadding, febox.width, febox.height)
                                canvasScroll.ensureVisible(febox, canvas.scale, 0)
                            }
                        }
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
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }

                DelayedPropertyBinder {
                    id: currentElementItemBinder
                    initial: null
                    set: elementItems.count > Scrite.document.structure.currentElementIndex ? elementItems.itemAt(Scrite.document.structure.currentElementIndex) : null
                }

                EventFilter.target: Scrite.app
                EventFilter.active: !Scrite.document.readOnly && visible && opacity === 1 && !createItemMouseHandler.enabled
                EventFilter.events: [EventFilter.KeyPress]
                EventFilter.onFilter: {
                    var dist = (event.controlModifier ? 5 : 1) * canvas.tickDistance
                    var element = Scrite.document.structure.elementAt(Scrite.document.structure.currentElementIndex)
                    if(element === null)
                        return

                    var fbbl = Scrite.document.structure.forceBeatBoardLayout

                    if(!fbbl)
                        element.undoRedoEnabled = true

                    switch(event.key) {
                    case Qt.Key_Left:
                        if(fbbl) return
                        element.x -= dist
                        result.accept = true
                        result.filter = true
                        break
                    case Qt.Key_Right:
                        if(fbbl) return
                        element.x += dist
                        result.accept = true
                        result.filter = true
                        break
                    case Qt.Key_Up:
                        if(fbbl) return
                        element.y -= dist
                        result.accept = true
                        result.filter = true
                        break
                    case Qt.Key_Down:
                        if(fbbl) return
                        element.y += dist
                        result.accept = true
                        result.filter = true
                        break
                    case Qt.Key_Delete:
                    case Qt.Key_Backspace:
                        result.accept = true
                        result.filter = true
                        if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI && element.follow)
                            element.follow.confirmAndDeleteSelf()
                        else
                            canvasScroll.deleteElement(element)
                        break
                    }

                    element.undoRedoEnabled = false
                }
            }

            property string groupCategory: Scrite.document.structure.preferredGroupCategory
            property var groupBoxes: []
            property var episodeBoxes: []
            property bool groupsBeingMoved: false
            Component.onCompleted: Utils.execLater(canvas, 250, reevaluateEpisodeAndGroupBoxes)

            function reevaluateEpisodeAndGroupBoxes() {
                if(groupsBeingMoved)
                    return
                var egBoxes = Scrite.document.structure.evaluateEpisodeAndGroupBoxes(Scrite.document.screenplay, canvas.groupCategory)
                canvas.groupBoxes = egBoxes.groupBoxes
                canvas.episodeBoxes = egBoxes.episodeBoxes
            }

            onGroupCategoryChanged: {
                Scrite.document.structure.preferredGroupCategory = groupCategory
                Utils.execLater(canvas, 250, reevaluateEpisodeAndGroupBoxes)
            }

            TrackerPack {
                delay: 250

                TrackProperty {
                    target: elementItems
                    property: "count"
                }

                TrackSignal {
                    target: Scrite.document.screenplay
                    signal: "elementsChanged()"
                }

                TrackSignal {
                    target: Scrite.document.screenplay
                    signal: "breakTitleChanged()"
                }

                TrackSignal {
                    target: Scrite.document
                    signal: "loadingChanged()"
                }

                TrackSignal {
                    target: Scrite.document.structure
                    signal: "structureChanged()"
                }

                TrackSignal {
                    target: Scrite.document.screenplay
                    signal: "elementSceneGroupsChanged(ScreenplayElement*)"
                }

                TrackSignal {
                    target: Scrite.document.screenplay
                    signal: "episodeCountChanged()"
                }

                onTracked: canvas.reevaluateEpisodeAndGroupBoxes()
            }

            Repeater {
                model: canvas.episodeBoxes

                Rectangle {
                    id: canvasEpisodeBox

                    property real topMarginForStacks: Scrite.document.structure.elementStacks.objectCount > 0 ? 15 : 0

                    x: modelData.geometry.x - 40
                    y: modelData.geometry.y - 120 - topMarginForStacks
                    width: modelData.geometry.width + 80
                    height: modelData.geometry.height + 120 + topMarginForStacks + 40
                    color: Scrite.app.translucent(Runtime.colors.accent.c100.background, Scrite.document.structure.forceBeatBoardLayout ? 0.3 : 0.1)
                    border.width: 2
                    border.color: Runtime.colors.accent.c600.background
                    enabled: !createItemMouseHandler.enabled && !currentElementItemShadow.visible && !annotationGripLoader.active

                    BoundingBoxItem.evaluator: canvasItemsBoundingBox
                    BoundingBoxItem.stackOrder: 1.0 + (index/canvas.episodeBoxes.length)
                    BoundingBoxItem.livePreview: false
                    BoundingBoxItem.previewFillColor: Qt.rgba(0,0,0,0.05)
                    BoundingBoxItem.previewBorderColor: Qt.rgba(0,0,0,0.5)
                    BoundingBoxItem.viewportItem: canvas
                    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
                    BoundingBoxItem.viewportRect: canvasScroll.viewportRect

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: episodeNameText.bottom
                        anchors.bottomMargin: -8
                        color: Runtime.colors.accent.c200.background
                    }

                    VclLabel {
                        id: episodeNameText
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.margins: 8
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 8
                        font.bold: true
                        color: Runtime.colors.accent.c200.text
                        text: "<b>" + modelData.name + "</b><font size=\"-2\">: " + modelData.sceneCount + (modelData.sceneCount === 1 ? " Scene": " Scenes") + "</font>"
                    }
                }
            }

            Repeater {
                model: canvas.groupBoxes

                Rectangle {
                    id: canvasGroupBoxItem
                    property real topMarginForStacks: Scrite.document.structure.elementStacks.objectCount > 0 ? 15 : 0
                    x: modelData.geometry.x - 20
                    y: modelData.geometry.y - 20 - topMarginForStacks
                    width: modelData.geometry.width + 40
                    height: modelData.geometry.height + 40 + topMarginForStacks
                    radius: 0
                    color: Scrite.app.translucent(Runtime.colors.accent.c100.background, Scrite.document.structure.forceBeatBoardLayout ? 0.3 : 0.1)
                    border.width: 1
                    border.color: Runtime.colors.accent.borderColor
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
                        var idxList = modelData.sceneIndexes
                        var movedIdxList = []
                        for(var i=0; i<nrElements; i++) {
                            var idx = idxList[i]
                            if(movedIdxList.indexOf(idx) < 0) {
                                var item = elementItems.itemAt(idxList[i])
                                item.x = item.x + dx
                                item.y = item.y + dy
                                movedIdxList.push(idx)
                            }
                        }
                        refX = x
                        refY = y
                    }

                    function selectBeatItems() {
                        var items = []
                        var nrElements = modelData.sceneCount
                        var idxList = modelData.sceneIndexes
                        var selIdxList = []
                        for(var i=0; i<nrElements; i++) {
                            var idx = idxList[i]
                            if(selIdxList.indexOf(idx) < 0) {
                                var item = elementItems.itemAt(idxList[i])
                                items.push(item)
                                selIdxList.push(idx)
                            }
                        }

                        selection.set(items)
                    }

                    property real refX: x
                    property real refY: y

                    MouseArea {
                        id: canvasBeatMouseArea
                        anchors.fill: parent
                        drag.target: controlPressed || Scrite.document.structure.forceBeatBoardLayout ? null : canvasGroupBoxItem
                        drag.axis: controlPressed || Scrite.document.structure.forceBeatBoardLayout ? Drag.None : Drag.XAndYAxis
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
                        color: Scrite.app.translucent(Runtime.colors.accent.c200.background, 0.4)

                        MouseArea {
                            id: canvasBeatLabelMouseArea
                            anchors.fill: parent
                            drag.target: controlPressed || Scrite.document.structure.forceBeatBoardLayout ? null : canvasGroupBoxItem
                            drag.axis: controlPressed || Scrite.document.structure.forceBeatBoardLayout ? Drag.None : Drag.XAndYAxis
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
                    }

                    VclLabel {
                        id: beatLabel
                        text: "<b>" + modelData.name + "</b><font size=\"-2\">: " + modelData.sceneCount + (modelData.sceneCount === 1 ? " Scene": " Scenes") + "</font>"
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 3
                        anchors.bottom: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: parent.radius*2
                        anchors.bottomMargin: parent.radius-parent.border.width
                        padding: 10
                        color: Runtime.colors.accent.c200.text
                    }
                }
            }

            Repeater {
                id: elementConnectorItems
                model: Scrite.document.loading ? 0 : Scrite.document.structureElementConnectors
                delegate: elementConnectorComponent
            }

            MouseArea {
                anchors.fill: parent
                enabled: canvasScroll.editItem
                acceptedButtons: Qt.LeftButton
                onClicked: canvasScroll.editItem.finishEditing()
            }

            MouseArea {
                anchors.fill: parent
                enabled: canvasScroll.editItem === null && !selection.active
                acceptedButtons: Qt.RightButton
                onPressed: canvasContextMenu.popup()
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

                var sceneId = event.mimeData[Runtime.timelineViewSettings.dropAreaKey]
                var element = Scrite.document.structure.findElementBySceneID(sceneId)
                if(element === null)
                    return

                if(element.stackId === "")
                    return

                result.acceptEvent = true
                result.filter = true

                if(event.type === EventFilter.Drop) {
                    element.stackId = ""
                    Utils.execLater(element, 250, function() {
                        if(!Scrite.document.structure.forceBeatBoardLayout) {
                            element.x = event.pos.x
                            element.y = event.pos.y
                        }
                        Scrite.document.structure.currentElementIndex = Scrite.document.structure.indexOfElement(element)
                        Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.firstIndexOfScene(element.scene)
                    })
                }
            }

            Repeater {
                id: stackBinders
                model: Scrite.document.loading ? null : Scrite.document.structure.elementStacks
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
                        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                        SimpleTabBarItem {
                            id: tabBarItem
                            tabCount: objectItem.objectCount
                            activeTabBorderWidth: (objectItem.hasCurrentElement ? 2 : 1)
                            tabLabelStyle: SimpleTabBarItem.Alphabets
                            activeTabIndex: objectItem.topmostElementIndex
                            activeTabColor: Qt.tint(objectItem.topmostElement.scene.color, (objectItem.hasCurrentElement ? "#C0FFFFFF" : "#F0FFFFFF"))
                            activeTabBorderColor: Scrite.app.isLightColor(objectItem.topmostElement.scene.color) ? "black" : objectItem.topmostElement.scene.color
                            activeTabFont.pointSize: Runtime.idealFontMetrics.font.pointSize
                            activeTabFont.bold: true
                            activeTabTextColor: Scrite.app.textColorFor(activeTabColor)
                            inactiveTabTextColor: Scrite.app.translucent(Scrite.app.textColorFor(inactiveTabColor), 0.75)
                            inactiveTabFont.pointSize: Runtime.idealFontMetrics.font.pointSize-4
                            minimumTabWidth: stackBinderItem.width*0.1
                            onTabClicked: objectItem.bringElementToTop(index)
                            onActiveTabIndexChanged: Qt.callLater(ensureActiveTabIsVisible)
                            onTabPathsUpdated: Qt.callLater(ensureActiveTabIsVisible)

                            Connections {
                                target: objectItem
                                ignoreUnknownSignals: true
                                function onDataChanged2() { tabBarItem.updateTabAttributes() }
                                function onStackInitialized() { tabBarItem.updateTabAttributes() } // ???
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
                                    requestedAttributeValue = Scrite.app.isLightColor(element.scene.color) ? "gray" : element.scene.color
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

            DropArea {
                id: catchAllDropArea
                anchors.fill: parent
                keys: [Runtime.timelineViewSettings.dropAreaKey]
                onDropped: (drop) => {
                    var otherScene = Scrite.app.typeName(drop.source) === "ScreenplayElement" ? drop.source.scene : drop.source
                    if(Scrite.document.screenplay.firstIndexOfScene(otherScene) < 0) {
                        MessageBox.information("",
                            "Scenes must be added to the timeline before they can be stacked."
                        )
                        drop.ignore()
                        return
                    }

                    var otherSceneId = otherScene.id
                    var otherElement = Scrite.document.structure.findElementBySceneID(otherSceneId)
                    if(otherElement === null) {
                        drop.ignore()
                        return
                    }

                    otherElement.unstack()
                    drop.acceptProposedAction()
                }
            }

            Repeater {
                id: elementItems
                model: Scrite.document.loading ? null : Scrite.document.structure.elementsModel
                delegate: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI ? structureElementIndexCardUIDelegate : structureElementSynopsisEditorUIDelegate
            }

            Selection {
                id: selection
                z: 3
                anchors.fill: parent
                interactive: !Scrite.document.readOnly && !Scrite.document.structure.forceBeatBoardLayout
                onMoveItem: {
                    item.x = item.x + dx
                    item.y = item.y + dy
                }
                onPlaceItem: {
                    item.x = Scrite.document.structure.snapToGrid(item.x)
                    item.y = Scrite.document.structure.snapToGrid(item.y)
                }

                contextMenu: VclMenu {
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

                    MarkSceneAsMenu {
                        title: "Mark Scenes As"
                        enableValidation: false
                        onTriggered: {
                            var items = selection.items
                            items.forEach( function(item) {
                                item.element.scene.type = type
                            })
                            selectionContextMenu.close()
                        }
                    }

                    VclMenu {
                        title: "Layout"

                        VclMenuItem {
                            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
                            icon.source: "qrc:/icons/action/layout_horizontally.png"
                            text: "Layout Horizontally"
                            onClicked: selection.layout(Structure.HorizontalLayout)
                        }

                        VclMenuItem {
                            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
                            icon.source: "qrc:/icons/action/layout_vertically.png"
                            text: "Layout Vertically"
                            onClicked: selection.layout(Structure.VerticalLayout)
                        }

                        VclMenuItem {
                            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
                            icon.source: "qrc:/icons/action/layout_flow_horizontally.png"
                            text: "Flow Horizontally"
                            onClicked: selection.layout(Structure.FlowHorizontalLayout)
                        }

                        VclMenuItem {
                            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
                            icon.source: "qrc:/icons/action/layout_flow_vertically.png"
                            text: "Flow Vertically"
                            onClicked: selection.layout(Structure.FlowVerticalLayout)
                        }
                    }

                    VclMenuItem {
                        text: "Annotate With Rectangle"
                        onClicked: {
                            createNewRectangleAnnotation(selection.rect.x-10, selection.rect.y-10, selection.rect.width+20, selection.rect.height+20)
                            selection.clear()
                        }
                    }

                    VclMenuItem {
                        text: "Stack"
                        enabled: {
                            if(Scrite.document.structure.canvasUIMode !== Structure.IndexCardUI)
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
                            var id = Scrite.app.createUniqueId()
                            items.forEach( function(item) {
                                item.element.stackId = id
                            })
                            selection.clear()
                        }
                    }

                    VclMenuItem {
                        text: "Add To Timeline"
                        onClicked: {
                            var items = selection.items
                            items.forEach( function(item) {
                                Scrite.document.screenplay.addScene(item.element.scene)
                            })
                        }
                    }

                    VclMenuItem {
                        text: "Remove From Timeline"
                        enabled: {
                            if(!selection.hasItems)
                                return false
                            var items = selection.items
                            for(var i=0; i<items.length; i++) {
                                if(items[i].element.scene.addedToScreenplay)
                                    continue
                                return false
                            }
                            return true
                        }
                        onClicked: {
                            var items = selection.items
                            var firstItem = items[0]
                            items.forEach( function(item) {
                                Scrite.document.screenplay.removeSceneElements(item.element.scene)
                            })
                            selection.clear()
                            canvasScroll.ensureItemVisibleLater(firstItem, canvas.scale)
                        }
                    }

                    StructureGroupsMenu {
                        sceneGroup: SceneGroup {
                            structure: Scrite.document.structure
                        }

                        onToggled: Utils.execLater(selection, 250, function() { selection.refit() })

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
                    if(Scrite.document.readOnly || Scrite.document.structure.forceBeatBoardLayout)
                        return

                    if(!hasItems) {
                        canvasPreview.allowed = false
                        var rect = Scrite.document.structure.layoutElements(type)
                        Utils.execLater(selection, 1000, function() {
                            cmdZoomOne.click()
                            canvasPreview.allowed = true
                        })
                        return
                    }

                    if(!canLayout)
                        return

                    layoutAnimation.layoutType = type
                    layoutAnimation.start()
                }

                SequentialAnimation {
                    id: layoutAnimation

                    property int layoutType: -1
                    property var layoutItems: []
                    property var layoutItemBounds
                    running: false

                    ScriptAction {
                        script: {
                            canvasPreview.allowed = false
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
                            layoutAnimation.layoutItemBounds = Scrite.document.structure.layoutElements(layoutAnimation.layoutType)
                            layoutAnimation.layoutType = -1
                            oldItems.forEach( function(item) {
                                item.element.selected = false
                            })
                            Scrite.document.structure.forceBeatBoardLayout = false
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
                            canvasPreview.allowed = true
                        }
                    }
                }
            }

            VclMenu {
                id: canvasContextMenu

                property bool isContextMenu: false

                VclMenuItem {
                    text: "New Scene"
                    enabled: !Scrite.document.readOnly
                    onClicked: {
                        Qt.callLater( function() { canvasContextMenu.close() } )
                        canvas.createItem("element", Qt.point(canvasContextMenu.x-130,canvasContextMenu.y-22), newSceneButton.activeColor)
                    }
                }

                ColorMenu {
                    title: "Colored Scene"
                    selectedColor: newSceneButton.activeColor
                    enabled: !Scrite.document.readOnly
                    onMenuItemClicked: {
                        Qt.callLater( function() { canvasContextMenu.close() } )
                        newSceneButton.activeColor = color
                        canvas.createItem("element", Qt.point(canvasContextMenu.x-130,canvasContextMenu.y-22), newSceneButton.activeColor)
                    }
                }

                MenuSeparator { }

                VclMenu {
                    title: "Annotation"

                    Repeater {
                        model: canvas.annotationsList

                        VclMenuItem {
                            property var annotationInfo: canvas.annotationsList[index]
                            text: annotationInfo.title
                            enabled: !Scrite.document.readOnly && annotationInfo.what !== ""
                            onClicked: {
                                Qt.callLater( function() { canvasContextMenu.close() } )
                                canvas.createItem(annotationInfo.what, Qt.point(canvasContextMenu.x, canvasContextMenu.y))
                            }
                        }
                    }
                }
            }

            VclMenu {
                id: elementContextMenu
                width: 250
                property StructureElement element
                onElementChanged: {
                    if(element)
                        popup()
                    else
                        close()
                }

                VclMenuItem {
                    action: Action {
                        text: "Scene Heading"
                        checkable: true
                        checked: elementContextMenu.element ? elementContextMenu.element.scene.heading.enabled : false
                    }
                    enabled: elementContextMenu.element
                    onTriggered: {
                        elementContextMenu.element.scene.heading.enabled = action.checked
                        elementContextMenu.element = null
                    }
                }

                ColorMenu {
                    title: "Color"
                    enabled: elementContextMenu.element
                    onMenuItemClicked: {
                        elementContextMenu.element.scene.color = color
                        elementContextMenu.element = null
                    }
                }

                MarkSceneAsMenu {
                    title: "Mark Scene As"
                    scene: elementContextMenu.element ? elementContextMenu.element.scene : null
                    onTriggered: elementContextMenu.element = null
                }

                VclMenuItem {
                    text: "Add To Timeline"
                    property Scene lastScene: Scrite.document.screenplay.elementCount > 0 && Scrite.document.screenplay.elementAt(Scrite.document.screenplay.elementCount-1).scene
                    enabled: elementContextMenu.element && elementContextMenu.element.scene !== lastScene
                    onClicked: {
                        var lastScreenplayScene = null
                        if(Scrite.document.screenplay.elementCount > 0)
                            lastScreenplayScene = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.elementCount-1).scene
                        if(lastScreenplayScene === null || elementContextMenu.element.scene !== lastScreenplayScene)
                            Scrite.document.screenplay.addScene(elementContextMenu.element.scene)
                        elementContextMenu.element = null
                    }
                }

                VclMenuItem {
                    text: "Remove From Timeline"
                    enabled: elementContextMenu.element && elementContextMenu.element.scene.addedToScreenplay
                    onClicked: {
                        Scrite.document.screenplay.removeSceneElements(elementContextMenu.element.scene)
                        canvasScroll.ensureItemVisibleLater(elementContextMenu.element.follow, canvas.scale)
                        elementContextMenu.element = null
                    }
                }

                StructureGroupsMenu {
                    sceneGroup: SceneGroup {
                        structure: Scrite.document.structure
                    }
                    onToggled: Utils.execLater(selection, 250, function() { selection.refit() })
                    onAboutToShow: {
                        sceneGroup.clearScenes()
                        sceneGroup.addScene(elementContextMenu.element.scene)
                    }
                    onClosed: sceneGroup.clearScenes()
                }

                VclMenuItem {
                    text: "Index Card Fields"
                    enabled: elementContextMenu.element
                    onClicked: StructureIndexCardFieldsDialog.launch()
                }

                MenuSeparator { }

                VclMenuItem {
                    text: "Delete"
                    enabled: elementContextMenu.element
                    onClicked: {
                        var element = elementContextMenu.element
                        elementContextMenu.element = null
                        if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI && element.follow)
                            element.follow.confirmAndDeleteSelf()
                        else
                            canvasScroll.deleteElement(element)
                    }
                }
            }
        }
    }

    Item {
        id: canvasPreview
        visible: allowed && Runtime.structureCanvasSettings.showPreview && parent.width > 400
        anchors.right: canvasScroll.right
        anchors.bottom: canvasScroll.bottom
        anchors.margins: 30
        property alias interacting: panMouseArea.pressed
        property bool allowed: true

        readonly property real maxSize: 150
        property size previewSize: evaluatePreviewSize()
        width: previewSize.width
        height: previewSize.height

        function evaluatePreviewSize() {
            var w = Math.max(canvasItemsBoundingBox.width, 500)
            var h = Math.max(canvasItemsBoundingBox.height, 500)

            var scale = 1
            if(w < h)
                scale = maxSize / w
            else
                scale = maxSize / h

            w *= scale
            h *= scale

            if(w > canvasScroll.width-60)
                scale = (canvasScroll.width-60)/w
            else if(h >= canvasScroll.height-60)
                scale = (canvasScroll.height-60)/h
            else
                scale = 1

            w *= scale
            h *= scale

            return Qt.size(w+10, h+10)
        }

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
            backgroundColor: Runtime.colors.primary.c100.background
            backgroundOpacity: 0.9

            MouseArea {
                id: jumpToMouseArea
                anchors.fill: parent
                enabled: canvasItemsBoundingBox.itemCount > 0
                onClicked: {
                    var scale = canvasItemsBoundingBox.width / previewArea.width
                    var x = canvasItemsBoundingBox.x + mouse.x * scale - canvasScroll.width/2
                    var y = canvasItemsBoundingBox.y + mouse.y * scale - canvasScroll.height/2
                    var area = Qt.rect(x,y,canvasScroll.width,canvasScroll.height)
                    canvasScroll.zoomOne()
                    canvasScroll.ensureVisible(area)
                }
            }

            Rectangle {
                id: viewportIndicator
                color: Scrite.app.translucent(Runtime.colors.accent.highlight.background, 0.25)
                border.width: 2
                border.color: Runtime.colors.accent.borderColor

                DelayedPropertyBinder {
                    id: geometryBinder
                    initial: Qt.rect(0,0,0,0)
                    set: {
                        if(!canvasPreview.visible)
                            return Qt.rect(0,0,0,0)

                        var visibleRect = canvasScroll.viewportRect
                        if( Scrite.app.isRectangleInRectangle(visibleRect,canvasItemsBoundingBox.boundingBox) )
                            return Qt.rect(0,0,0,0)

                        var intersect = Scrite.app.intersectedRectangle(visibleRect, canvasItemsBoundingBox.boundingBox)
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

    BasicAttachmentsDropArea {
        id: annotationAttachmentDropArea
        z: -10
        allowedType: Attachments.PhotosOnly
        anchors.fill: canvasScroll
        onDropped: {
            var pos = canvas.mapFromItem(canvasScroll, mouse.x, mouse.y)
            createNewImageAnnotation(pos.x, pos.y, attachment.filePath)
        }
    }

    Rectangle {
        id: annotationAttachmentNotice
        anchors.fill: parent
        visible: annotationAttachmentDropArea.active
        color: Scrite.app.translucent(Runtime.colors.primary.c500.background, 0.5)

        Rectangle {
            anchors.fill: attachmentNotice
            anchors.margins: -30
            radius: 4
            color: Runtime.colors.primary.c700.background
        }

        VclLabel {
            id: attachmentNotice
            anchors.centerIn: parent
            width: parent.width * 0.5 /* noticeWidthFactor */
            wrapMode: Text.WordWrap
            color: Runtime.colors.primary.c700.text
            text: parent.visible ? "<b>" + annotationAttachmentDropArea.attachment.title + "</b><br/><br/>" + "Drop here as an annotation." : ""
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }
    }

    Rectangle {
        id: statusBar
        height: 30
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Runtime.colors.primary.windowColor
        border.width: 1
        border.color: Runtime.colors.primary.borderColor
        clip: true

        VclText {
            anchors.left: parent.left
            anchors.right: statusBarControls.left
            anchors.margins: 10
            elide: Text.ElideRight
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: statusBar.height * 0.5
            text: {
                if(!canvasScroll.interactive)
                    return "Canvas Locked While Index Card Has Focus. Hit ESC To Release Focus."
                var ret = Scrite.document.structure.elementCount + " Scenes";
                if(canvas.episodeBoxes.length > 0)
                    ret += ", " + canvas.episodeBoxes.length + " Episodes";
                if(Scrite.document.structure.forceBeatBoardLayout)
                    ret += ", Scenes Not Movable"
                ret += "."
                return ret;
            }

            MouseArea {
                anchors.fill: parent
                enabled: parent.truncated
                hoverEnabled: true
                ToolTip.text: parent.text
                ToolTip.visible: containsMouse
            }
        }

        Row {
            id: statusBarControls
            height: parent.height-6
            spacing: 10
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right

            FlatToolButton {
                iconSource: "qrc:/icons/content/view_options.png"
                autoRepeat: false
                ToolTip.text: "Toggle between index-card view options."
                checkable: false
                suggestedWidth: parent.height
                suggestedHeight: parent.height
                onClicked: structureViewOptionsMenu.show()
                down: structureViewOptionsMenu.active

                MenuLoader {
                    id: structureViewOptionsMenu
                    anchors.left: parent.left
                    anchors.bottom: parent.top
                    anchors.bottomMargin: item ? item.height : 0
                    menu: VclMenu {

                        VclMenuItem {
                            text: "Index Cards"
                            property bool _checked: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI &&
                                                    Scrite.document.structure.indexCardContent === Structure.Synopsis
                            icon.source: _checked ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                            onClicked: {
                                if(!_checked) {
                                    if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI)
                                        Scrite.document.structure.indexCardContent = Structure.Synopsis
                                    else
                                        mainUiContentLoader.reset( () => {
                                            Scrite.document.structure.canvasUIMode = Structure.IndexCardUI
                                            Scrite.document.structure.indexCardContent = Structure.Synopsis
                                        } )
                                }
                            }
                        }

                        VclMenuItem {
                            text: "Photo Cards"
                            property bool _checked: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI &&
                                                    Scrite.document.structure.indexCardContent === Structure.FeaturedPhoto
                            icon.source: _checked ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                            onClicked: {
                                if(!_checked)
                                    if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI)
                                        Scrite.document.structure.indexCardContent = Structure.FeaturedPhoto
                                    else
                                        mainUiContentLoader.reset( () => {
                                            Scrite.document.structure.canvasUIMode = Structure.IndexCardUI
                                            Scrite.document.structure.indexCardContent = Structure.FeaturedPhoto
                                        } )
                            }
                        }

                        VclMenuItem {
                            text: "Synopsis Cards"
                            property bool _checked: Scrite.document.structure.canvasUIMode === Structure.SynopsisEditorUI
                            icon.source: _checked ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"
                            onClicked: {
                                if(!_checked) {
                                    if(Scrite.document.structure.elementCount > 0)
                                        MessageBox.information("Cannot Switch to Synopsis Cards",
                                                               "Switching to Synopsis Cards is only possible when the structure canvas is empty.")
                                    else
                                        mainUiContentLoader.reset( () => {
                                            Scrite.document.structure.canvasUIMode = Structure.SynopsisEditorUI
                                        } )
                                }
                            }
                        }
                    }
                }
            }

            FlatToolButton {
                iconSource: "qrc:/icons/hardware/mouse.png"
                autoRepeat: false
                ToolTip.text: "Mouse wheel currently " + (checked ? "zooms" : "scrolls") + ". Click this button to make it " + (checked ? "scroll" : "zoom") + "."
                checkable: true
                checked: Runtime.workspaceSettings.mouseWheelZoomsInStructureCanvas
                onCheckedChanged: Runtime.workspaceSettings.mouseWheelZoomsInStructureCanvas = checked
                suggestedWidth: parent.height
                suggestedHeight: parent.height
            }

            FlatToolButton {
                down: canvasPreview.visible
                checked: canvasPreview.visible
                checkable: true
                onToggled: Runtime.structureCanvasSettings.showPreview = checked
                iconSource: "qrc:/icons/action/thumbnail.png"
                ToolTip.text: "Preview"
                suggestedWidth: parent.height
                suggestedHeight: parent.height
            }

            Rectangle {
                height: parent.height
                width: 1
                color: Runtime.colors.primary.borderColor
            }

            FlatToolButton {
                id: cmdZoomOne
                onClicked: click()
                iconSource: "qrc:/icons/navigation/zoom_one.png"
                autoRepeat: true
                ToolTip.text: "Zoom One"
                suggestedWidth: parent.height
                suggestedHeight: parent.height
                enabled: canvasItemsBoundingBox.itemCount > 0
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

            FlatToolButton {
                id: cmdZoomFit
                suggestedWidth: parent.height
                suggestedHeight: parent.height
                enabled: canvasItemsBoundingBox.itemCount > 0
                onClicked: {
                    canvasItemsBoundingBox.recomputeBoundingBox()
                    canvasScroll.zoomFit(canvasItemsBoundingBox.boundingBox);
                    canvasScroll.isZoomFit = true
                    canvasScroll.updateScriteDocumentUserData()
                }
                iconSource: "qrc:/icons/navigation/zoom_fit.png"
                autoRepeat: true
                ToolTip.text: "Zoom Fit"
            }

            ZoomSlider {
                id: zoomSlider
                from: canvasScroll.minimumScale
                to: canvasScroll.maximumScale
                stepSize: 0.0
                anchors.verticalCenter: parent.verticalCenter
                value: canvas.scale
                onSliderMoved: Qt.callLater(applyZoom)
                onZoomInRequest: canvasScroll.zoomIn()
                onZoomOutRequest: canvasScroll.zoomOut()
                height: parent.height
                function applyZoom() {
                    canvasScroll.animatePanAndZoom = false
                    canvasScroll.zoomTo(zoomLevel)
                    canvasScroll.animatePanAndZoom = true
                }
            }
        }
    }

    Component {
        id: newStructureElementComponent

        StructureElement {
            objectName: "newElement"
            scene: Scene {
                title: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI ? "" : "New Scene"
                heading.locationType: "INT"
                heading.location: "SOMEWHERE"
                heading.moment: "DAY"
            }
            Component.onCompleted: {
                scene.undoRedoEnabled = true
                Scrite.document.structure.forceBeatBoardLayout = false
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
            BoundingBoxItem.stackOrder: 3.0 + (index/Scrite.document.structure.elementCount)
            BoundingBoxItem.livePreview: false
            BoundingBoxItem.previewFillColor: selected ? Qt.darker(element.scene.color) : element.scene.color
            BoundingBoxItem.previewBorderColor: Scrite.app.isLightColor(element.scene.color) ? "black" : background.color
            BoundingBoxItem.previewBorderWidth: selected ? 3 : 1.5
            BoundingBoxItem.viewportItem: canvas
            BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
            BoundingBoxItem.viewportRect: canvasScroll.viewportRect

            readonly property bool selected: Scrite.document.structure.currentElementIndex === index
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
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: 400 }
                }
            }

            TextViewEdit {
                id: titleText
                width: 250
                wrapMode: Text.WordWrap
                text: element.scene.synopsis
                anchors.centerIn: parent
                font.pointSize: 13
                horizontalAlignment: Text.AlignLeft
                onTextEdited: element.scene.synopsis = text
                onEditingFinished: {
                    editMode = false
                    element.objectName = "oldElement"
                }
                onHighlightRequest: Scrite.document.structure.currentElementIndex = index
                Keys.onReturnPressed: editingFinished()
                property bool editMode: element.objectName === "newElement"
                readOnly: !(editMode && index === Scrite.document.structure.currentElementIndex)
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
                        Scrite.document.structure.currentElementIndex = index
                    } else if(canvasScroll.mouseOverItem === elementItem)
                        canvasScroll.mouseOverItem = null
                }
                acceptedButtons: Qt.LeftButton
                onDoubleClicked: {
                    annotationGripLoader.reset()
                    canvas.forceActiveFocus()
                    Scrite.document.structure.currentElementIndex = index
                    if(!Scrite.document.readOnly) {
                        titleText.editMode = true
                        if(canvasScroll.mouseOverItem === elementItem)
                            canvasScroll.mouseOverItem = null
                    }
                }
                onClicked: {
                    annotationGripLoader.reset()
                    canvas.forceActiveFocus()
                    Scrite.document.structure.currentElementIndex = index
                    requestEditorLater()
                }

                drag.target: Scrite.document.readOnly || Scrite.document.structure.forceBeatBoardLayout ? null : elementItem
                drag.axis: Drag.XAndYAxis
                drag.minimumX: 0
                drag.minimumY: 0
                drag.onActiveChanged: {
                    canvas.forceActiveFocus()
                    Scrite.document.structure.currentElementIndex = index
                    if(drag.active === false) {
                        elementItem.x = Scrite.document.structure.snapToGrid(parent.x)
                        elementItem.y = Scrite.document.structure.snapToGrid(parent.y)
                    } else
                        elementItem.element.syncWithFollow = true
                }
            }

            Keys.onPressed: {
                if(event.key === Qt.Key_F2) {
                    canvas.forceActiveFocus()
                    Scrite.document.structure.currentElementIndex = index
                    if(!Scrite.document.readOnly)
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
                    Scrite.document.structure.currentElementIndex = index
                    elementContextMenu.element = elementItem.element
                    elementContextMenu.popup()
                }
            }

            property size dragImageSize: {
                var s = elementItem.width > elementItem.height ? maxDragImageSize.width / elementItem.width : maxDragImageSize.height / elementItem.height
                return Qt.size( elementItem.width*s, elementItem.height*s )
            }

            // Drag to timeline support
            Drag.active: dragMouseArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.LinkAction
            Drag.hotSpot.x: dragImageSize.width/2 // dragHandle.x + dragHandle.width/2
            Drag.hotSpot.y: dragImageSize.height/2 // dragHandle.y + dragHandle.height/2
            Drag.mimeData: {
                let md = {}
                md[Runtime.timelineViewSettings.dropAreaKey] = element.scene.id
                return md
            }
            Drag.source: element.scene

            SceneTypeImage {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 3
                width: 18; height: 18
                opacity: 0.5
                showTooltip: false
                sceneType: elementItem.element.scene.type
                lightBackground: Scrite.app.isLightColor(background.color)
            }

            Image {
                id: dragHandle
                visible: !parent.editing && !Scrite.document.readOnly
                enabled: !canvasScroll.editItem && !Scrite.document.readOnly
                source: elementItem.element.scene.addedToScreenplay || elementItem.Drag.active ? "qrc:/icons/action/view_array.png" : "qrc:/icons/content/add_circle_outline.png"
                width: 18; height: 18
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 3
                anchors.bottomMargin: 1
                opacity: elementItem.selected ? 1 : 0.1
                scale: dragMouseArea.pressed ? 2 : 1
                Behavior on scale {
                    enabled: Runtime.applicationSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }

                MouseArea {
                    id: dragMouseArea
                    anchors.fill: parent
                    drag.target: parent
                    drag.onActiveChanged: {
                        if(drag.active)
                            canvas.forceActiveFocus()
                        else if(canvasScroll.maybeDragItem === elementItem)
                            canvasScroll.maybeDragItem = null
                    }
                    onPressed: {
                        canvas.forceActiveFocus()
                        canvasScroll.maybeDragItem = elementItem
                        elementItem.grabToImage(function(result) {
                            elementItem.Drag.imageSource = result.url
                        }, elementItem.dragImageSize)
                    }
                    onReleased: {
                        if(canvasScroll.maybeDragItem === elementItem)
                            canvasScroll.maybeDragItem = null
                    }
                    onClicked: {
                        if(!elementItem.element.scene.addedToScreenplay)
                            Scrite.document.screenplay.addScene(elementItem.element.scene)
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
            property bool selected: Scrite.document.structure.currentElementIndex === index
            z: selected ? 1 : 0

            function select() {
                Scrite.document.structure.currentElementIndex = index
            }

            function activate() {
                canvasTabSequence.releaseFocus()
                annotationGripLoader.reset()
                Scrite.document.structure.currentElementIndex = index
                requestEditorLater()
            }

            function finishEditing() {
                if(canvasScroll.editItem === elementItem)
                    canvasScroll.editItem = null
                canvasTabSequence.releaseFocus()
            }

            function zoomOneForFocus() {
                if(canvas.scaleIsLessForEdit)
                    canvasScroll.zoomOneToItem(elementItem)
            }

            property bool visibleInViewport: true
            property StructureElementStack elementStack
            property bool stackedOnTop: (elementStack === null || elementStack.topmostElement === element)
            visible: visibleInViewport && stackedOnTop
            onVisibleChanged: {
                if(!visible) {
                    if(Scrite.app.hasActiveFocus(Scrite.window,indexCardLayout))
                        canvasTabSequence.releaseFocus()
                }
            }

            function determineElementStack() {
                if(element.stackId === "")
                    elementStack = null
                else if(elementStack === null || elementStack.stackId !== element.stackId)
                    elementStack = Scrite.document.structure.elementStacks.findStackById(element.stackId)
            }

            TrackerPack {
                delay: 250
                TrackSignal { target: element; signal: "stackIdChanged()" }
                TrackSignal { target: Scrite.document.structure.elementStacks; signal: "objectCountChanged()" }
                TrackSignal { target: elementStack; signal: "objectCountChanged()" }
                TrackSignal { target: elementStack; signal: "stackLeaderChanged()" }
                TrackSignal { target: elementStack; signal: "topmostElementChanged()" }
                onTracked: elementItem.determineElementStack()
            }

            Component.onCompleted: {
                elementItem.determineElementStack()
                element.follow = elementItem
            }

            BoundingBoxItem.evaluator: canvasItemsBoundingBox
            BoundingBoxItem.stackOrder: 3.0 + (index/Scrite.document.structure.elementCount)
            BoundingBoxItem.livePreview: false
            BoundingBoxItem.previewFillColor: Scrite.app.translucent(element.scene.color, selected ? 0.75 : 0.1)
            BoundingBoxItem.previewBorderColor: Scrite.app.isLightColor(element.scene.color) ? "black" : element.scene.color
            BoundingBoxItem.previewBorderWidth: selected ? 3 : 1.5
            BoundingBoxItem.viewportItem: canvas
            BoundingBoxItem.visibilityMode: stackedOnTop ? BoundingBoxItem.VisibleUponViewportIntersection : BoundingBoxItem.IgnoreVisibility
            BoundingBoxItem.viewportRect: canvasScroll.viewportRect
            BoundingBoxItem.visibilityProperty: "visibleInViewport"

            onSelectedChanged: {
                if(selected && (Runtime.undoStack.structureEditorActive || Scrite.document.structure.elementCount === 1))
                    synopsisFieldLoader.forceActiveFocus()
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
            height: 300 + Scrite.document.structure.indexCardFields.length * 50

            property int nrFocusFieldCount: {
                const nrHeadingFields = 1
                const nrSynopsisFields = Scrite.document.structure.indexCardContent === Structure.Synopsis ? 1 : 0
                const nrIndexCardFields = Scrite.document.structure.indexCardContent === Structure.Synopsis ? Scrite.document.structure.indexCardFields.length : 0
                return nrHeadingFields + nrSynopsisFields + nrIndexCardFields
            }

            Rectangle {
                id: background
                anchors.fill: parent
                color: Qt.tint(element.scene.color, selected ? "#C0FFFFFF" : "#F0FFFFFF")
                border.width: elementItem.selected ? 2 : 1

                property color borderColor: Scrite.app.isLightColor(element.scene.color) ? Qt.rgba(0.75,0.75,0.75,1.0) : element.scene.color
                border.color: elementItem.selected ? borderColor : Qt.lighter(borderColor)

                // Move index-card around
                MouseArea {
                    id: moveMouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onPressed: {
                        elementItem.element.undoRedoEnabled = true
                        elementItem.select()
                        canvas.forceActiveFocus()
                    }
                    onReleased: {
                        elementItem.element.undoRedoEnabled = false
                    }

                    drag.target: Scrite.document.readOnly || Scrite.document.structure.forceBeatBoardLayout ? null : elementItem
                    drag.axis: Drag.XAndYAxis
                    drag.minimumX: 0
                    drag.minimumY: 0
                    drag.onActiveChanged: {
                        canvas.forceActiveFocus()
                        Scrite.document.structure.currentElementIndex = index
                        if(drag.active === false) {
                            elementItem.x = Scrite.document.structure.snapToGrid(elementItem.x)
                            elementItem.y = Scrite.document.structure.snapToGrid(elementItem.y)
                        } else
                            elementItem.element.syncWithFollow = true
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

            property bool focus2: headingFieldLoader.hasFocus | synopsisFieldLoader.hasFocus
            onFocus2Changed: {
                if(focus2)
                    canvasScroll.editItem = elementItem
                else if(canvasScroll.editItem === elementItem)
                    canvasScroll.editItem = null
            }

            ColumnLayout {
                id: indexCardLayout

                readonly property real margin: 7

                anchors.fill: parent
                anchors.margins: margin

                spacing: 10

                LodLoader {
                    id: headingFieldLoader

                    property bool hasFocus: false

                    Layout.fillWidth: true

                    TabSequenceItem.enabled: elementItem.stackedOnTop
                    TabSequenceItem.manager: canvasTabSequence
                    TabSequenceItem.sequence: {
                        var indexes = element.scene.screenplayElementIndexList
                        if(indexes.length === 0)
                            return elementIndex * elementItem.nrFocusFieldCount + 0
                        return (indexes[0] + Scrite.document.structure.elementCount) * elementItem.nrFocusFieldCount + 0
                    }
                    TabSequenceItem.onAboutToReceiveFocus: {
                        Scrite.document.structure.currentElementIndex = elementIndex
                        Qt.callLater(maybeAssumeFocus)
                    }

                    lod: elementItem.selected && !canvas.scaleIsLessForEdit ? eHIGH : eLOW

                    lowDetailComponent: TextEdit {
                        id: basicHeadingField

                        Transliterator.defaultFont: font
                        Transliterator.textDocument: textDocument
                        Transliterator.applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts

                        Component.onCompleted: headingFieldLoader.hasFocus = false

                        text: element.hasTitle ? element.title : "Index Card Title"
                        color: element.hasTitle ? "black" : "gray"
                        enabled: false
                        readOnly: true

                        topPadding: 4
                        leftPadding: 4
                        rightPadding: 4
                        bottomPadding: 4

                        selectByMouse: false
                        selectByKeyboard: false

                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        // font.capitalization: element.hasNativeTitle ? Font.MixedCase : Font.AllUppercase

                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }

                    highDetailComponent: VclTextField {
                        id: headingField

                        property var currentLanguage: Scrite.app.transliterationEngine.language

                        Component.onCompleted: headingFieldLoader.hasFocus = activeFocus
                        Keys.onEscapePressed: canvasTabSequence.releaseFocus()

                        width: parent.width

                        text: element.title
                        label: ""
                        enabled: !readOnly
                        readOnly: Scrite.document.readOnly
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        maximumLength: 140
                        placeholderText: "Scene Heading / Name"
                        labelAlwaysVisible: true
                        enableTransliteration: true

                        topPadding: 4
                        leftPadding: 4
                        rightPadding: 4
                        bottomPadding: 4

                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize

                        onEditingComplete: { element.title = text; TabSequenceItem.focusNext() }

                        onActiveFocusChanged: {
                            if(activeFocus)
                                elementItem.select()
                            headingFieldLoader.hasFocus = activeFocus
                        }
                    }

                    onItemChanged: Qt.callLater(maybeAssumeFocus)
                    onFocusChanged: Qt.callLater(maybeAssumeFocus)

                    function maybeAssumeFocus() {
                        if(focus && lod === eHIGH && item) {
                            item.selectAll()
                            item.forceActiveFocus()
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    currentIndex: Scrite.document.structure.indexCardContent === Structure.Synopsis ? 0 : 1

                    ColumnLayout {
                        spacing: 10
                        visible: parent.currentIndex === 0

                        LodLoader {
                            id: synopsisFieldLoader

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            TabSequenceItem.enabled: elementItem.stackedOnTop
                            TabSequenceItem.manager: canvasTabSequence
                            TabSequenceItem.sequence: {
                                var indexes = element.scene.screenplayElementIndexList
                                if(indexes.length === 0)
                                    return elementIndex * elementItem.nrFocusFieldCount + 1
                                return (indexes[0] + Scrite.document.structure.elementCount) * elementItem.nrFocusFieldCount + 1
                            }
                            TabSequenceItem.onAboutToReceiveFocus: {
                                Scrite.document.structure.currentElementIndex = elementIndex
                                Qt.callLater(maybeAssumeFocus)
                            }

                            property bool hasFocus: false

                            lod: elementItem.selected && !canvas.scaleIsLessForEdit ? eHIGH : eLOW
                            sanctioned: parent.visible
                            resetWidthBeforeLodChange: false
                            resetHeightBeforeLodChange: false

                            lowDetailComponent: Rectangle {
                                clip: true
                                height: synopsisFieldLoader.height
                                border.width: synopsisTextDisplay.truncated ? 1 : 0
                                border.color: Runtime.colors.primary.borderColor
                                color: synopsisTextDisplay.truncated ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0)

                                TextEdit {
                                    id: synopsisTextDisplay

                                    anchors.fill: parent

                                    Transliterator.defaultFont: font
                                    Transliterator.textDocument: textDocument
                                    Transliterator.applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                                    Transliterator.spellCheckEnabled: Runtime.screenplayEditorSettings.enableSpellCheck

                                    topPadding: 4
                                    leftPadding: 4
                                    rightPadding: 4
                                    bottomPadding: 4

                                    text: element.scene.hasSynopsis ? element.scene.synopsis : "Describe what happens in this scene."
                                    color: element.scene.hasTitle ? "black" : "gray"
                                    enabled: false
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    readOnly: true
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                                    selectByMouse: false
                                    selectByKeyboard: false

                                    TextAreaSpellingSuggestionsMenu { }
                                }

                                Component.onCompleted: synopsisFieldLoader.hasFocus = false
                            }

                            highDetailComponent: Item {
                                width: synopsisFieldLoader.width
                                height: synopsisFieldLoader.height

                                function assumeFocus() {
                                    synopsisField.forceActiveFocus()
                                    synopsisField.cursorPosition = synopsisField.length
                                }

                                Flickable {
                                    id: synopsisFieldFlick

                                    property bool scrollBarVisible: contentHeight > height

                                    ScrollBar.vertical: VclScrollBar { flickable: synopsisFieldFlick }

                                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                                    clip: true
                                    width: parent.width
                                    height: parent.height-5
                                    interactive: synopsisField.activeFocus && scrollBarVisible
                                    contentWidth: synopsisField.contentWidth
                                    contentHeight: Math.max(height, synopsisField.contentHeight + 100)
                                    flickableDirection: Flickable.VerticalFlick

                                    TextArea {
                                        id: synopsisField

                                        Transliterator.defaultFont: font
                                        Transliterator.textDocument: textDocument
                                        Transliterator.cursorPosition: cursorPosition
                                        Transliterator.hasActiveFocus: activeFocus
                                        Transliterator.applyLanguageFonts: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                                        Transliterator.spellCheckEnabled: Runtime.screenplayEditorSettings.enableSpellCheck

                                        Keys.onEscapePressed: canvasTabSequence.releaseFocus()
                                        Component.onCompleted: synopsisFieldLoader.hasFocus = activeFocus

                                        width: synopsisFieldFlick.scrollBarVisible ? synopsisFieldFlick.width-20 : synopsisFieldFlick.width

                                        background: Item { }

                                        topPadding: 4
                                        leftPadding: 4
                                        rightPadding: 4
                                        bottomPadding: 4

                                        selectByMouse: true
                                        selectByKeyboard: true

                                        text: element.scene.synopsis
                                        readOnly: Scrite.document.readOnly
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                        placeholderText: "Describe what happens in this scene."

                                        onTextChanged: element.scene.synopsis = text

                                        onActiveFocusChanged: {
                                            if(activeFocus) {
                                                elementItem.select()
                                                cursorFocusAnimation.active = true
                                            } else
                                                elementItem.element.scene.trimSynopsis()
                                            synopsisFieldLoader.hasFocus = activeFocus
                                        }

                                        onCursorRectangleChanged: {
                                            var y1 = cursorRectangle.y
                                            var y2 = cursorRectangle.y + cursorRectangle.height
                                            if(y1 < synopsisFieldFlick.contentY)
                                                synopsisFieldFlick.contentY = Math.max(y1-10, 0)
                                            else if(y2 > synopsisFieldFlick.contentY + synopsisFieldFlick.height)
                                                synopsisFieldFlick.contentY = y2+10 - synopsisFieldFlick.height
                                        }

                                        SpecialSymbolsSupport {
                                            anchors.top: parent.bottom
                                            anchors.left: parent.left
                                            textEditor: synopsisField
                                            textEditorHasCursorInterface: true
                                            enabled: !Scrite.document.readOnly
                                        }

                                        TextAreaSpellingSuggestionsMenu { }

                                        Loader {
                                            id: cursorFocusAnimation
                                            x: synopsisField.cursorRectangle.x
                                            y: synopsisField.cursorRectangle.y
                                            width: synopsisField.cursorRectangle.width
                                            height: synopsisField.cursorRectangle.height
                                            active: false
                                            sourceComponent: Item {
                                                Rectangle {
                                                    id: cursorFocusRect
                                                    width: t*parent.width
                                                    height: Math.max(t*parent.height*0.5, parent.height)
                                                    anchors.centerIn: parent
                                                    opacity: 1.0 - (t/10)*0.8
                                                    visible: false
                                                    color: "black"
                                                    property real t: 10
                                                    property bool scaledDown: t <= 1
                                                    onScaledDownChanged: {
                                                        if(scaledDown)
                                                            cursorFocusAnimation.active = false
                                                    }
                                                    Behavior on t {
                                                        NumberAnimation { duration: 500 }
                                                    }
                                                }

                                                Component.onCompleted: Utils.execLater(cursorFocusRect, 250, function() {
                                                    cursorFocusRect.visible = true
                                                    cursorFocusRect.t = 1
                                                })
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    anchors.bottom: parent.bottom
                                    width: parent.width
                                    height: synopsisField.hovered || synopsisField.activeFocus ? 2 : 1
                                    color: Runtime.colors.primary.c500.background
                                }
                            }

                            onFocusChanged: Qt.callLater(maybeAssumeFocus)
                            onItemChanged: Qt.callLater(maybeAssumeFocus)

                            function maybeAssumeFocus() {
                                if(focus && lod === eHIGH && item)
                                    item.assumeFocus()
                            }
                        }

                        IndexCardFields {
                            Layout.fillWidth: true

                            lod: synopsisFieldLoader.lod
                            visible: hasFields
                            sanctioned: parent.visible

                            structureElement: elementItem.element

                            tabSequenceEnabled: elementItem.stackedOnTop
                            tabSequenceManager: canvasTabSequence
                            startTabSequence: {
                                var indexes = element.scene.screenplayElementIndexList
                                if(indexes.length === 0)
                                    return elementIndex * elementItem.nrFocusFieldCount + 2
                                return (indexes[0] + Scrite.document.structure.elementCount) * elementItem.nrFocusFieldCount + 2
                            }

                            onFieldAboutToReceiveFocus: Scrite.document.structure.currentElementIndex = elementIndex
                        }
                    }

                    LodLoader {
                        id: featuredImageFieldLoader

                        lod: synopsisFieldLoader.lod
                        visible: sanctioned
                        sanctioned: parent.currentIndex === 1
                        resetWidthBeforeLodChange: false
                        resetHeightBeforeLodChange: false

                        lowDetailComponent: Image {
                            id: lowLodfeaturedImageField

                            property Attachments sceneAttachments: element.scene.attachments
                            property Attachment featuredAttachment: sceneAttachments.featuredAttachment
                            property Attachment featuredImage: featuredAttachment && featuredAttachment.type === Attachment.Photo ? featuredAttachment : null
                            property string fillModeAttrib: "indexCardFillMode"
                            property int defaultFillMode: Image.PreserveAspectCrop

                            fillMode: {
                                if(!featuredImage)
                                    return defaultFillMode
                                const ud = featuredImage.userData
                                if(ud[fillModeAttrib])
                                    return ud[fillModeAttrib] === "fit" ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                                return defaultFillMode
                            }
                            source: featuredImage ? featuredImage.fileSource : ""
                            mipmap: !(canvasScroll.moving || canvasScroll.flicking)

                            Loader {
                                anchors.fill: parent
                                active: !parent.featuredAttachment
                                sourceComponent: AttachmentsDropArea {
                                    allowedType: Attachments.PhotosOnly
                                    target: lowLodfeaturedImageField.sceneAttachments
                                    onDropped: {
                                        attachment.featured = true
                                        allowDrop()
                                    }
                                    attachmentNoticeSuffix: "Drop this photo to tag it as featured image for this scene."

                                    VclLabel {
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        anchors.centerIn: parent
                                        wrapMode: Text.WordWrap
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                        text: "Drag & Drop a Photo"
                                        visible: !parent.active
                                    }
                                }
                            }
                        }

                        highDetailComponent: SceneFeaturedImage {
                            scene: element.scene
                            fillModeAttrib: "indexCardFillMode"
                            defaultFillMode: Image.PreserveAspectCrop
                            mipmap: !(canvasScroll.moving || canvasScroll.flicking)
                        }
                    }
                }

                Item {
                    id: footerRow

                    property bool lightBackground: Scrite.app.isLightColor(footerBg.color)

                    Layout.fillWidth: true
                    Layout.preferredHeight: footerRowLayout.height

                    Rectangle {
                        id: footerBg

                        anchors.fill: parent
                        anchors.margins: -5

                        property color baseColor: background.border.color
                        color: Qt.tint(baseColor, elementItem.selected ? "#70FFFFFF" : "#A0FFFFFF")
                    }

                    RowLayout {
                        id: footerRowLayout

                        width: parent.width
                        spacing: 5

                        SceneTypeImage {
                            id: sceneTypeImage

                            Layout.alignment: Qt.AlignBottom
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24

                            opacity: 0.5
                            visible: sceneType !== Scene.Standard
                            sceneType: elementItem.element.scene.type
                            showTooltip: false

                            lightBackground: footerRow.lightBackground
                        }

                        ColumnLayout {
                            Layout.fillWidth: true

                            spacing: parent.spacing

                            VclLabel {
                                id: groupsLabel

                                Layout.fillWidth: true

                                text: Scrite.document.structure.presentableGroupNames(element.scene.groups)
                                color: footerRow.lightBackground ? "black" : "white"
                                visible: element.scene.groups.length > 0 || !element.scene.hasCharacters
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                font.pointSize: Scrite.app.idealAppFontSize - 2
                            }

                            VclLabel {
                                id: characterList

                                Layout.fillWidth: true

                                text: {
                                    if(element.scene.hasCharacters)
                                        return "<b>Characters</b>: " + element.scene.characterNames.join(", ")
                                    return ""
                                }
                                color: footerRow.lightBackground ? "black" : "white"
                                visible: element.scene.hasCharacters
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                font.pointSize: Scrite.app.idealAppFontSize - 2
                            }
                        }

                        Item {
                            id: dragHandle

                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            Layout.alignment: Qt.AlignBottom

                            Image {
                                id: dragHandleImage

                                anchors.fill: parent

                                source: elementItem.element.scene.addedToScreenplay || elementItem.Drag.active ?
                                            (footerRow.lightBackground ? "qrc:/icons/action/view_array.png" : "qrc:/icons/action/view_array_inverted.png") :
                                            (footerRow.lightBackground ? "qrc:/icons/content/add_circle_outline.png" : "qrc:/icons/content/add_circle_outline_inverted.png")

                                scale: dragHandleMouseArea.pressed ? 2 : 1
                                opacity: elementItem.selected ? 1 : 0.1

                                Behavior on scale {
                                    enabled: Runtime.applicationSettings.enableAnimations
                                    NumberAnimation { duration: 250 }
                                }
                            }

                            MouseArea {
                                id: dragHandleMouseArea
                                anchors.fill: parent
                                hoverEnabled: !canvasScroll.flicking && !canvasScroll.moving && elementItem.selected
                                drag.target: dragHandleImage
                                drag.onActiveChanged: {
                                    if(drag.active)
                                        canvas.forceActiveFocus()
                                    else if(canvasScroll.maybeDragItem === elementItem)
                                        canvasScroll.maybeDragItem = null
                                }
                                onPressed: {
                                    canvas.forceActiveFocus()
                                    canvasScroll.maybeDragItem = elementItem
                                    elementItem.grabToImage(function(result) {
                                        elementItem.Drag.imageSource = result.url
                                    }, elementItem.dragImageSize)
                                }
                                onReleased: {
                                    if(canvasScroll.maybeDragItem === elementItem)
                                        canvasScroll.maybeDragItem = null
                                }
                                onClicked: {
                                    if(!elementItem.element.scene.addedToScreenplay)
                                        Scrite.document.screenplay.addScene(elementItem.element.scene)
                                }
                            }
                        }
                    }
                }
            }

            // Drag to timeline support
            property size dragImageSize: {
                var s = elementItem.width > elementItem.height ? maxDragImageSize.width / elementItem.width : maxDragImageSize.height / elementItem.height
                return Qt.size( elementItem.width*s, elementItem.height*s )
            }

            Drag.active: dragHandleMouseArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.LinkAction
            Drag.hotSpot.x: dragImageSize.width/2 // dragHandle.x + dragHandle.width/2
            Drag.hotSpot.y: dragImageSize.height/2 // dragHandle.y + dragHandle.height/2
            Drag.mimeData: {
                let md = {}
                md[Runtime.timelineViewSettings.dropAreaKey] = element.scene.id
                return md
            }
            Drag.source: element.scene

            // Accept drops for stacking items on top of each other.
            Rectangle {
                anchors.fill: parent
                anchors.margins: -10
                border.width: 2
                border.color: Scrite.app.translucent("black", alpha)
                color: Scrite.app.translucent("#cfd8dc", alpha)
                radius: 6
                property real alpha: dropAreaForStacking.containsDrag ? 0.5 : 0
                enabled: !dragHandleMouseArea.drag.active && element.scene.addedToScreenplay

                DropArea {
                    id: dropAreaForStacking
                    anchors.fill: parent
                    keys: [Runtime.timelineViewSettings.dropAreaKey]
                    onDropped: (drop) => {
                        var otherScene = Scrite.app.typeName(drop.source) === "ScreenplayElement" ? drop.source.scene : drop.source
                        if(Scrite.document.screenplay.firstIndexOfScene(otherScene) < 0) {
                            MessageBox.information("",
                                "Scenes must be added to the timeline before they can be stacked."
                            )
                            drop.ignore()
                            return
                        }

                        var otherSceneId = otherScene.id
                        if(otherSceneId === element.scene.id) {
                            drop.ignore()
                            return
                        }

                        var otherElement = Scrite.document.structure.findElementBySceneID(otherSceneId)
                        if(otherElement === null) {
                            drop.ignore()
                            return
                        }

                        if(element.scene.actIndex < 0 || otherElement.scene.actIndex < 0) {
                            MessageBox.information("",
                                "Scenes must be added to the timeline before they can be stacked."
                            )
                            drop.ignore()
                            return
                        }

                        if(element.scene.actIndex !== otherElement.scene.actIndex) {
                            MessageBox.information("",
                                "Scenes must belong to the same act for them to be stacked."
                            )
                            drop.ignore()
                            return
                        }

                        var otherElementIndex = Scrite.document.structure.indexOfElement(otherElement)
                        Qt.callLater( function() { Scrite.document.structure.currentElementIndex = otherElementIndex } )

                        var myStackId = element.stackId
                        var otherStackId = otherElement.stackId
                        drop.acceptProposedAction()

                        if(myStackId === "") {
                            var uid = Scrite.app.createUniqueId()
                            element.stackId = uid
                            otherElement.stackId = uid
                        } else {
                            otherElement.stackId = myStackId
                        }

                        Qt.callLater( function() { element.stackLeader = true } )
                    }
                }
            }

            function confirmAndDeleteSelf() {
                deleteConfirmationBox.active = true
            }

            Loader {
                id: deleteConfirmationBox
                active: false
                anchors.fill: parent
                sourceComponent: Rectangle {
                    id: deleteConfirmationItem
                    property bool allowDeactivate: false

                    Component.onCompleted: {
                        elementItem.zoomOneForFocus()
                        forceActiveFocus()
                        Utils.execLater(deleteConfirmationItem, 500, function() {
                            deleteConfirmationItem.allowDeactivate = true
                        })
                    }

                    color: Scrite.app.translucent(Runtime.colors.primary.c600.background,0.85)

                    property bool visibleInViewport: elementItem.visibleInViewport
                    onVisibleInViewportChanged: {
                        if(!visibleInViewport && allowDeactivate)
                            deleteConfirmationBox.active = false
                    }

                    onActiveFocusChanged: {
                        if(!activeFocus && allowDeactivate)
                            deleteConfirmationBox.active = false
                    }

                    MouseArea {
                        anchors.fill: parent
                    }

                    Column {
                        width: parent.width-20
                        anchors.centerIn: parent
                        spacing: 40

                        VclLabel {
                            text: "Are you sure you want to delete this index card?"
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            color: Runtime.colors.primary.c600.text
                        }

                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 20

                            VclButton {
                                text: "Yes"
                                focusPolicy: Qt.NoFocus
                                onClicked: canvasScroll.deleteElement(elementItem.element)
                            }

                            VclButton {
                                text: "No"
                                focusPolicy: Qt.NoFocus
                                onClicked: deleteConfirmationBox.active = false
                            }
                        }
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
            outlineWidth: Scrite.app.devicePixelRatio * canvas.scale * Runtime.structureCanvasSettings.lineWidthOfConnectors
            visible: {
                if(canBeVisible)
                    return intersects(canvasScroll.viewportRect)
                return false
            }

            Rectangle {
                id: labelBg
                width: Math.max(labelItem.width,labelItem.height)+20
                height: width; radius: width/2
                border.width: 1; border.color: Runtime.colors.primary.borderColor
                x: parent.suggestedLabelPosition.x - radius
                y: parent.suggestedLabelPosition.y - radius
                color: Qt.tint(parent.outlineColor, "#E0FFFFFF")
                visible: !canvasPreview.updatingThumbnail

                VclText {
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
            enabled: !Scrite.document.readOnly
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
                color: Runtime.colors.primary.c100.background
                height: annotationToolBarLayout.height+5
                width: annotationToolBarLayout.width+5
                border.width: 1
                border.color: Runtime.colors.primary.borderColor

                Row {
                    id: annotationToolBarLayout
                    anchors.centerIn: parent

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/edit.png"
                        ToolTip.text: "Edit properties of this annotation"
                        down: AnnotationPropertyEditorDock.visible
                        onClicked: Runtime.structureCanvasSettings.displayAnnotationProperties = !Runtime.structureCanvasSettings.displayAnnotationProperties
                    }

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/keyboard_arrow_up.png"
                        ToolTip.text: "Bring this annotation to front"
                        enabled: Scrite.document.structure.canBringToFront(annotationGripLoader.annotation)
                        onClicked: {
                            var a = annotationGripLoader.annotation
                            annotationGripLoader.reset()
                            Scrite.document.structure.bringToFront(a)
                        }
                    }

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/keyboard_arrow_down.png"
                        ToolTip.text: "Send this annotation to back"
                        enabled: Scrite.document.structure.canSendToBack(annotationGripLoader.annotation)
                        onClicked: {
                            var a = annotationGripLoader.annotation
                            annotationGripLoader.reset()
                            Scrite.document.structure.sendToBack(a)
                        }
                    }

                    FlatToolButton {
                        iconSource: "qrc:/icons/action/delete.png"
                        ToolTip.text: "Delete this annotation"
                        onClicked: {
                            var a = annotationGripLoader.annotation
                            annotationGripLoader.reset()
                            Scrite.document.structure.removeAnnotation(a)
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
                outlineColor: Runtime.colors.accent.a700.background
                outlineStyle: PainterPathItem.DashDotDotLine
                painterPath: PainterPath {
                    MoveTo { x: onePxSize; y: onePxSize }
                    LineTo { x: focusIndicator.width-onePxSize; y: onePxSize }
                    LineTo { x: focusIndicator.width-onePxSize; y: focusIndicator.height-onePxSize }
                    LineTo { x: onePxSize; y: focusIndicator.height-onePxSize }
                    CloseSubpath { }
                }
            }

            EventFilter.target: Scrite.app
            EventFilter.active: !Scrite.document.readOnly && !createItemMouseHandler.enabled && !AnnotationPropertyEditorDock.visible
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
                    if(Runtime.structureCanvasSettings.displayAnnotationProperties === false) {
                        Runtime.structureCanvasSettings.displayAnnotationProperties = true
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
                Scrite.document.structure.removeAnnotation(a)
            }

            onXChanged: annotGeoUpdateTimer.start()
            onYChanged: annotGeoUpdateTimer.start()
            onWidthChanged: annotGeoUpdateTimer.start()
            onHeightChanged: annotGeoUpdateTimer.start()

            function snapAnnotationGeometryToGrid(rect) {
                var gx = Scrite.document.structure.snapToGrid(rect.x)
                var gy = Scrite.document.structure.snapToGrid(rect.y)
                var gw = Scrite.document.structure.snapToGrid(rect.width)
                var gh = Scrite.document.structure.snapToGrid(rect.height)
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
                    if(Runtime.structureCanvasSettings.displayAnnotationProperties === false)
                        Runtime.structureCanvasSettings.displayAnnotationProperties = true
                }
            }

            Rectangle {
                id: rightGrip
                width: gripSize
                height: gripSize
                color: Runtime.colors.accent.a700.background
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
                color: Runtime.colors.accent.a700.background
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
                color: Runtime.colors.accent.a700.background
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

    function createNewRectangleAnnotation(x, y, w, h) {
        if(Scrite.document.readOnly)
            return

        var doNotAlignRect = w && h

        w = w ? w : 200
        h = h ? h : 200
        var rect = doNotAlignRect ? Qt.rect(x, y, w, h) : Qt.rect(x - w/2, y-h/2, w, h)
        var annot = annotationObject.createObject(canvas)
        annot.type = "rectangle"
        annot.geometry = rect
        Scrite.document.structure.addAnnotation(annot)
    }

    Component {
        id: rectangleAnnotationComponent

        AnnotationItem {

        }
    }

    function createNewOvalAnnotation(x, y) {
        if(Scrite.document.readOnly)
            return

        var w = 80
        var h = 80
        var rect =Qt.rect(x - w/2, y-h/2, w, h)
        var annot = annotationObject.createObject(canvas)
        annot.type = "oval"
        annot.geometry = rect
        Scrite.document.structure.addAnnotation(annot)
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
        if(Scrite.document.readOnly)
            return

        var w = 200
        var h = 40

        var annot = annotationObject.createObject(canvas)
        annot.type = "text"
        annot.geometry = Qt.rect(x-w/2, y-h/2, w, h)
        Scrite.document.structure.addAnnotation(annot)
    }

    Component {
        id: textAnnotationComponent

        AnnotationItem {
            id: textAnnotationItem

            VclLabel {
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
        if(Scrite.document.readOnly)
            return

        var w = 300
        var h = 350 // Scrite.app.isMacOSPlatform ? 60 : 350

        var annot = annotationObject.createObject(canvas)
        annot.type = "url"
        annot.geometry = Qt.rect(x-w/2, y-20, w, h)
        Scrite.document.structure.addAnnotation(annot)
    }

    Component {
        id: urlAnnotationComponent

        AnnotationItem {
            id: urlAnnotItem
            color: Runtime.colors.primary.c50.background
            border {
                width: 1
                color: Runtime.colors.primary.borderColor
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
                        color: annotationHasLocalImage ? Qt.rgba(0,0,0,0) : Runtime.colors.primary.c500.background

                        Image {
                            id: imageItem
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                            source: {
                                if(annotationHasLocalImage)
                                    annotation.imageUrl(annotation.attributes.imageName)
                                // Lets avoid using HTTPS for as long as possible
                                // Want to avoid having to bundle OpenSSL with Scrite.
                                return Scrite.app.toHttpUrl(annotation.attributes.imageUrl)
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

                    VclLabel {
                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                        text: annotation.attributes.title
                        width: parent.width
                        maximumLineCount: 2
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                    }

                    VclLabel {
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        text: annotation.attributes.description
                        width: parent.width
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        maximumLineCount: 3
                    }

                    VclLabel {
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
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

            BusyIcon {
                anchors.centerIn: parent
                running: urlAttribs.status === UrlAttributes.Loading
            }

            VclLabel {
                anchors.fill: parent
                anchors.margins: 10
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: Scrite.app.isMacOSPlatform && annotationGripLoader.annotationItem !== urlAnnotItem ? "Set a URL to get a clickable link here." : "Set a URL to preview it here."
                visible: annotation.attributes.url === ""
            }
        }
    }

    function createNewImageAnnotation(x, y, filePath) {
        if(Scrite.document.readOnly)
            return

        var w = 300
        var h = 160

        var annot = annotationObject.createObject(canvas)
        annot.type = "image"
        annot.geometry = Qt.rect(x-w/2, y-h/2, w, h)

        if(filePath && typeof filePath === "string") {
            var attrs = annot.attributes
            attrs["image"] = annot.addImage(filePath)
            annot.attributes = attrs
        }

        Scrite.document.structure.addAnnotation(annot)
    }

    Component {
        id: imageAnnotationComponent

        AnnotationItem {
            id: imageAnnotItem
            clip: true
            color: image.isSet ? (annotation.attributes.fillBackground ? annotation.attributes.backgroundColor : Qt.rgba(0,0,0,0)) : Runtime.colors.primary.c100.background

            BoundingBoxItem.livePreview: false
            BoundingBoxItem.previewImageSource: image.source

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

            VclLabel {
                width: image.width
                height: Math.max(parent.height - image.height - 10, 0)
                visible: height > 0
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
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
        if(Scrite.document.readOnly)
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
        Scrite.document.structure.addAnnotation(annot)
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

        VclLabel {
            text: "Unknown annotation: <strong>" + annotation.type + "</strong>"
            x: annotation.geometry.x
            y: annotation.geometry.y
        }
    }

    function requestEditorLater() {
        Utils.execLater(structureView, 100, function() { requestEditor() })
    }

    Loader {
        id: notebookIconAnimator
        active: Runtime.workspaceSettings.animateNotebookIcon && Runtime.showNotebookInStructure
        anchors.fill: parent
        sourceComponent: UiElementHighlight {
            uiElement: notebookTabButton
            onDone: Runtime.workspaceSettings.animateNotebookIcon = false
            description: notebookTabButton.ToolTip.text
            property bool scaleDone: false
            onScaleAnimationDone: scaleDone = true
            Component.onDestruction: {
                if(scaleDone)
                    Runtime.workspaceSettings.animateNotebookIcon = false
            }
        }
    }

    Component.onCompleted: {
        AnnotationPropertyEditorDock.annotation = Qt.binding( () => { return annotationGripLoader.annotation } )
        AnnotationPropertyEditorDock.itemsBoundingBox = canvasItemsBoundingBox
        Scrite.user.logActivity1("structure")
    }
}
