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
    readonly property alias canvasGroupBoxCount: _canvas.groupBoxCount
    readonly property alias itemsBoundingBox: _canvas.itemsBoundingBox
    readonly property alias currentAnnotation: _canvas.currentAnnotation
    readonly property alias canvasEpisodeBoxCount: _canvas.episodeBoxCount
    readonly property alias currentElementItem: _canvas.currentElementItem
    readonly property alias draggedElementItem: _canvas.draggedElementItem
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
        _canvas.groupCategory = groupCategory
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

    Component.onCompleted: _private.updateFromScriteDocumentUserDataLater()
    Component.onDestruction: _private.updateScriteDocumentUserData()

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

        onZoomOneRequest: () => { _private.zoomOne() }
        onZoomOneToItemRequest: (item) => { _private.zoomOneToItem(item) }
        onEditorRequest: () => { root.editorRequest() }
        onDeleteElementRequest: (element) => { _private.deleteElement(element) }
        onSelectionModeOffRequest: () => { root.selectionModeOffRequest() }
        onDenyCanvasPreviewRequest: () => { root.denyCanvasPreviewRequest() }
        onAllowCanvasPreviewRequest: () => { root.allowCanvasPreviewRequest() }
        onEnsureItemVisibleRequest: (item) => { root.ensureItemVisible(item) }
        onEnsureAreaVisibleRequest: (area) => { root.ensureVisible(area, suggestedScale, 0) }
    }

    Connections {
        target: Scrite.document

        function onJustLoaded() { _private.updateFromScriteDocumentUserDataLater() }
    }

    onContentXChanged: Qt.callLater(_private.updateScriteDocumentUserData)

    onContentYChanged: Qt.callLater(_private.updateScriteDocumentUserData)

    onZoomScaleChanged: isZoomFit = false

    onAnimatePanAndZoomChanged: Qt.callLater(_private.updateScriteDocumentUserData)

    onZoomScaleChangedInteractively: Qt.callLater(_private.updateScriteDocumentUserData)

    QtObject {
        id: _private

        property bool updateScriteDocumentUserDataEnabled: false

        property bool interactive: {
            const canvasInteractionGoingOn =  (_canvas.rubberband.active ||
                                               _canvas.selection.active ||
                                               root.canvasPreviewInteracting ||
                                               _canvas.annotationGrip)
            const canvasItemInteractionGoingOn = (/*_canvas.currentElementItem ||*/
                                                  _canvas.editElementItem ||
                                                  _canvas.draggedElementItem)

            return !canvasInteractionGoingOn && !canvasItemInteractionGoingOn
        }

        function enablePanAndZoomAnimation(delay) {
            if(animatePanAndZoom === true)
                return

            if(delay === undefined || delay === null)
                animatePanAndZoom = true;
            else
                Runtime.execLater(root, delay, () => { root.animatePanAndZoom = true })
        }

        function updateScriteDocumentUserData() {
            if(!updateScriteDocumentUserDataEnabled || Scrite.document.readOnly || animatingPanOrZoom)
                return

            let userData = Scrite.document.userData
            userData["StructureView.canvasScroll"] = {
                "version": 0,
                "contentX": root.contentX,
                "contentY": root.contentY,
                "zoomScale": root.zoomScale,
                "isZoomFit": root.isZoomFit
            }

            Scrite.document.userData = userData
        }

        function updateFromScriteDocumentUserData() {
            let userData = Scrite.document.userData
            let csData = userData["StructureView.canvasScroll"];
            if(csData && csData.version === 0) {
                root.zoomScale = csData.zoomScale
                root.contentX = csData.contentX
                root.contentY = csData.contentY
                root.isZoomFit = csData.isZoomFit === true
                if(root.isZoomFit) {
                    Runtime.execLater(root, 500, function() {
                        var area = _canvas.itemsBoundingBox.boundingBox
                        root.zoomFit(area)
                        root.enablePanAndZoomAnimation(2000)
                    })
                }
            } else {
                if(Scrite.document.structure.elementCount > 0) {
                    let item = _canvas.currentElementItem
                    if(item === null)
                        item = _canvas.elementLayer.elementItemAt(0)
                    if(Runtime.firstSwitchToStructureTab)
                        root.zoomOneToItem(item)
                    else
                        root.ensureItemVisible(item, _canvas.scale)
                } else
                    root.zoomOneMiddleArea()
                root.enablePanAndZoomAnimation(2000)
            }

            if(Scrite.document.structure.forceBeatBoardLayout)
                Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)

            updateScriteDocumentUserDataEnabled = true
            Runtime.firstSwitchToStructureTab = false
        }

        function updateFromScriteDocumentUserDataLater() {
            Runtime.execLater(root, 500, updateFromScriteDocumentUserData)
        }

        function zoomSanityCheck() {
            if( !GMath.doRectanglesIntersect(_canvas.itemsBoundingBox.boundingBox, root.viewportRect) ) {
                let item = _currentElementItemBinder.get
                if(item === null)
                    item = _elementItems.itemAt(0)
                root.ensureItemVisible(item, _canvas.scale)
            }
        }

        function zoomOne() {
            const item = _canvas.currentElementItem
            if(item === null) {
                item = _canvas.elementItemAt(0)
            }

            if(item) {
                zoomOneToItem(item)
            } else {
                zoomOneMiddleArea()
            }

            updateScriteDocumentUserData()
        }

        function zoomOneMiddleArea() {

            if(_canvas.itemsBoundingBox.itemCount > 0) {
                const bbox = _canvas.itemsBoundingBox.boundingBox
                if(bbox.width < root.width && bbox.height < root.height) {
                    root.ensureVisible(bbox)
                } else {
                    const areaSize = Qt.size(root.width*0.5, root.height*0.5)
                    const bboxCenter = Qt.point(bbox.x + bbox.width/2, bbox.y + bbox.height/2)
                    const middleArea = Qt.rect(bboxCenter.x - areaSize.width/2, bboxCenter.y - areaSize.height/2, areaSize.width, areaSize.height)
                    root.ensureVisible(middleArea)
                }
            } else {
                const middleArea = Qt.rect((_canvas.width-root.width)/2,
                                         (_canvas.height-root.height)/2,
                                         root.width,
                                         root.height)
                root.ensureVisible(middleArea)
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

            let nextScene = null
            let nextElement = null
            if(element.scene.addedToScreenplay) {
                nextElement = Scrite.document.screenplay.elementAt(element.scene.screenplayElementIndexList[0]+1)
                if(nextElement === null)
                    nextElement = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.lastSceneIndex())
                if(nextElement !== null)
                    nextScene = nextElement.scene
            } else {
                let idx = Scrite.document.structure.indexOfElement(element)
                let i = 0;
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

            root.releaseEditorRequest()

            Scrite.document.screenplay.removeSceneElements(element.scene)
            Scrite.document.structure.removeElement(element)

            Qt.callLater(function(scene) {
                if(Scrite.document.screenplay.elementCount === 0)
                    return
                if(scene === null)
                    scene = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.lastSceneIndex())
                let idx = Scrite.document.structure.indexOfScene(scene)
                if(idx >= 0) {
                    Scrite.document.structure.currentElementIndex = idx
                    Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.firstIndexOfScene(scene)
                }
            }, nextScene)
        }

        onUpdateScriteDocumentUserDataEnabledChanged: {
            if(updateScriteDocumentUserDataEnabled)
                Runtime.execLater(root, 500, zoomSanityCheck)
        }
    }
}
