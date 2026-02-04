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

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/overlays"
import "qrc:/qml/structureview"

ScrollArea {
    id: root

    required property bool canvasPreviewUpdatingThumbnail
    required property bool canvasPreviewInteracting

    property alias newSceneColor: _canvas.newSceneColor

    property bool isZoomFit: false
    property rect viewportRect: Qt.rect( visibleArea.xPosition * contentWidth / _canvas.scale,
                                        visibleArea.yPosition * contentHeight / _canvas.scale,
                                        visibleArea.widthRatio * contentWidth / _canvas.scale,
                                        visibleArea.heightRatio * contentHeight / _canvas.scale )

    readonly property alias canvas: _canvas
    readonly property alias selection: _canvas.selection
    readonly property alias rubberband: _canvas.rubberband
    readonly property alias canvasScale: _canvas.scale
    readonly property alias tabSequence: _canvas.tabSequence
    readonly property alias elementLayer: _canvas.elementLayer
    readonly property alias groupCategory: _canvas.groupCategory
    readonly property alias annotationGrip: _canvas.annotationGrip
    readonly property alias annotationLayer: _canvas.annotationLayer
    readonly property alias editElementItem: _canvas.editElementItem
    readonly property alias itemsBoundingBox: _canvas.itemsBoundingBox
    readonly property alias currentAnnotation: _canvas.currentAnnotation
    readonly property alias currentElementItem: _canvas.currentElementItem
    readonly property alias draggedElementItem: _canvas.draggedElementItem
    readonly property alias canvasGroupBoxCount: _canvas.groupBoxCount
    readonly property alias canvasEpisodeBoxCount: _canvas.episodeBoxCount
    readonly property alias interactiveCreationMode: _canvas.interactiveCreationMode
    readonly property alias availableAnnotationKeys: _canvas.availableAnnotationKeys

    signal editorRequest()
    signal releaseEditorRequest()
    signal selectionModeOffRequest()
    signal denyCanvasPreviewRequest()
    signal allowCanvasPreviewRequest()

    function createNewAnnotation(annotationType) {
        _canvas.newSceneColor = "white"
        _canvas.createItemMode = annotationType
    }

    function createNewScene(sceneColor) {
        _canvas.newSceneColor = sceneColor
        _canvas.createItemMode = "element"
    }

    function switchGroupCategory(groupCategory) {
        Scrite.document.structure.preferredGroupCategory = groupCategory
    }

    function selectAllElements() {
        _canvas.selectAllElements()
    }

    function layoutElementSelection(layout) {
        _canvas.layoutElementSelection(layout)
    }

    function confirmAndDeleteElement(element) {
        if(element) {
            if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI && element.follow)
                element.follow.confirmAndDeleteSelf()
            else
                _private.deleteElement(element)
        }
    }

    function enablePanAndZoomAnimation(delay) {
        _private.enablePanAndZoomAnimation(delay)
    }

    function zoomOneMiddleArea() {
        _private.zoomOneMiddleArea()
    }

    function zoomOneToItem(item) {
        _private.zoomOneToItem(item)
    }

    Component.onCompleted: Runtime.execLater(_private, Runtime.stdAnimationDuration, _private.initialize)

    clip: true

    contentWidth: _canvas.width * _canvas.scale
    contentHeight: _canvas.height * _canvas.scale
    initialContentWidth: _canvas.width
    initialContentHeight: _canvas.height

    interactive: _private.interactive
    minimumScale: _canvas.itemsBoundingBox.itemCount > 0 ? Math.min(0.25, width/_canvas.itemsBoundingBox.width, height/_canvas.itemsBoundingBox.height) : 0.25
    zoomOnScroll: Runtime.workspaceSettings.mouseWheelZoomsInStructureCanvas
    showScrollBars: Scrite.document.structure.elementCount >= 1
    animatePanAndZoom: false

    StructureCanvas {
        id: _canvas

        canvasScrollMoving: root.moving
        canvasScrollHeight: root.height
        canvasScrollFlicking: root.flicking
        canvasScrollInteractive: root.interactive
        canvasScrollViewportRect: root.viewportRect
        canvasScrollSuggestedScale: root.suggestedScale
        canvasPreviewUpdatingThumbnail: root.canvasPreviewUpdatingThumbnail

        onEditElementItemChanged: {
            if(editElementItem && _canvas.scaleIsLessForEdit) {
                Runtime.execLater(root, 500, function() { _private.zoomOneToItem(editElementItem) })
            }
        }

        onZoomOneRequest: () => { _private.zoomOneToCurrentItem() }
        onZoomOneToItemRequest: (item) => { _private.zoomOneToItem(item) }
        onEditorRequest: () => { root.editorRequest() }
        onDeleteElementRequest: (element) => { Qt.callLater(_private.deleteElement, element) }
        onDeleteElementsRequest: (elementList) => { Qt.callLater(_private.deleteElements, elementList) }
        onSelectionModeOffRequest: () => { root.selectionModeOffRequest() }
        onDenyCanvasPreviewRequest: () => { root.denyCanvasPreviewRequest() }
        onAllowCanvasPreviewRequest: () => { root.allowCanvasPreviewRequest() }
        onEnsureItemVisibleRequest: (item) => { root.ensureItemVisible(item) }
        onEnsureAreaVisibleRequest: (area) => { root.ensureAreaVisible(area, suggestedScale, 0) }
    }

    QtObject {
        id: _private

        readonly property Action releaseFocusAction: Action {
            shortcut: Gui.shortcut(Qt.Key_Escape)
            enabled: _canvas.editElementItem !== null

            onTriggered: (source) => {
                             _canvas.editElementItem.focus = false
                         }
        }

        property bool interactive: {
            const canvasInteractionGoingOn =  (_canvas.rubberband.active ||
                                               _canvas.selection.active ||
                                               root.canvasPreviewInteracting /*||
                                               _canvas.annotationGrip*/)
            const canvasItemInteractionGoingOn = (/*_canvas.currentElementItem ||*/
                                                  _canvas.editElementItem ||
                                                  _canvas.draggedElementItem)

            return !canvasInteractionGoingOn && !canvasItemInteractionGoingOn
        }

        function enablePanAndZoomAnimation(delay) {
            if(animatePanAndZoom === true)
                return

            if(delay === undefined || delay === null)
                animatePanAndZoom = true
            else
                Runtime.execLater(root, delay, () => { root.animatePanAndZoom = true })
        }

        function initialize() {
            if(Scrite.document.structure.forceBeatBoardLayout)
                Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)
            _canvas.itemsBoundingBox.markPreviewDirty()
            _canvas.itemsBoundingBox.recomputeBoundingBox()

            _private.zoomOneToCurrentItem()
            animatePanAndZoom = true

            if(FocusInspector.hasFocus(root)) {
                Scrite.window.activeFocusItem.focus = false
            }
        }

        function zoomSanityCheck() {
            if( !GMath.doRectanglesIntersect(_canvas.itemsBoundingBox.boundingBox, root.viewportRect) ) {
                let item = _canvas.currentElementItem
                if(item === null)
                    item = _canvas.elementItemAt(0)
                root.ensureItemVisible(item)
            }
        }

        function zoomOneToCurrentItem() {
            let item = _canvas.currentElementItem
            if(item === null) {
                item = _canvas.elementItemAt(Scrite.document.structure.currentElementIndex)
            }

            if(item) {
                zoomOneToItem(item)
            } else {
                zoomOneMiddleArea()
            }
        }

        function zoomOneMiddleArea() {
            if(_canvas.itemsBoundingBox.itemCount > 0) {
                const bbox = _canvas.itemsBoundingBox.boundingBox
                if(bbox.width < root.width && bbox.height < root.height) {
                    root.ensureAreaVisible(bbox)
                } else {
                    const areaSize = Qt.size(root.width*0.5, root.height*0.5)
                    const bboxCenter = Qt.point(bbox.x + bbox.width/2, bbox.y + bbox.height/2)
                    const middleArea = Qt.rect(bboxCenter.x - areaSize.width/2, bboxCenter.y - areaSize.height/2, areaSize.width, areaSize.height)
                    root.ensureAreaVisible(middleArea)
                }
            } else {
                const middleArea = Qt.rect((_canvas.width-root.width)/2,
                                         (_canvas.height-root.height)/2,
                                         root.width,
                                         root.height)
                root.ensureAreaVisible(middleArea)
            }
        }

        function zoomOneToItem(item) {
            if(item === null)
                return
            let bbox = _canvas.itemsBoundingBox.boundingBox
            let itemRect = Qt.rect(item.x, item.y, item.width, item.height)
            let atBest = Qt.size(root.width, root.height)
            let visibleArea = GMath.querySubRectangle(bbox, itemRect, atBest)
            root.zoomFit(visibleArea)
        }

        function deleteElement(element) {
            if(element === null)
                return

            root.releaseEditorRequest()
            Runtime.undoStack.active = false
            Scrite.document.structure.removeElements([element])
            Runtime.undoStack.active = true
            root.editorRequest()
        }

        function deleteElements(elementList) {
            if(elementList === undefined || elementList.length === undefined || elementList === 0)
                return

            root.releaseEditorRequest()
            Runtime.undoStack.active = false
            Scrite.document.structure.removeElements(elementList)
            Runtime.undoStack.active = true
            root.editorRequest()
        }
    }
}
