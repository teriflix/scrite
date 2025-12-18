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
import "qrc:/qml/structureview/"
import "qrc:/qml/structureview/annotations"
import "qrc:/qml/structureview/structureelements"

GridBackground {
    id: root

    property color newSceneColor

    required property bool canvasScrollMoving
    required property bool canvasScrollFlicking
    required property bool canvasScrollInteractive
    required property bool canvasPreviewUpdatingThumbnail

    required property real canvasScrollHeight
    required property real canvasScrollSuggestedScale

    required property rect canvasScrollViewportRect

    readonly property alias groupCategory: _elementLayer.groupCategory
    property alias createItemMode: _createItemHandler.mode
    property alias rubberbandSelectionMode: _elementLayer.rubberbandSelectionMode

    property bool scaleIsLessForEdit: (350*root.scale < root.canvasScrollHeight*0.25)

    readonly property alias hasFocus: _private.hasFocus
    readonly property alias selection: _elementLayer.selection
    readonly property alias rubberband: _elementLayer.rubberband
    readonly property alias tabSequence: _private.tabSequence
    readonly property alias elementLayer: _elementLayer
    readonly property alias groupBoxCount: _elementLayer.groupBoxCount
    readonly property alias episodeBoxCount: _elementLayer.episodeBoxCount
    readonly property alias annotationGrip: _annotationLayer.grip
    readonly property alias annotationLayer: _annotationLayer
    readonly property alias editElementItem: _elementLayer.editElementItem
    readonly property alias itemsBoundingBox: _private.itemsBoundingBox
    readonly property alias currentElementItem: _elementLayer.currentElementItem
    readonly property alias draggedElementItem: _elementLayer.draggedElementItem
    readonly property alias interactiveCreationMode: _createItemHandler.active
    readonly property alias availableAnnotationKeys: _annotationLayer.availableAnnotationKeys

    readonly property Annotation currentAnnotation: _annotationLayer.grip ? _annotationLayer.grip.annotation : null

    signal editorRequest()
    signal zoomOneRequest()
    signal zoomOneToItemRequest(Item item)
    signal deleteElementRequest(StructureElement element)
    signal deleteElementsRequest(var elementList)
    signal selectionModeOffRequest()
    signal denyCanvasPreviewRequest()
    signal allowCanvasPreviewRequest()
    signal ensureItemVisibleRequest(Item item)
    signal ensureAreaVisibleRequest(rect area)

    function selectAllElements() {
        _elementLayer.selectAllElements()
    }

    function layoutElementSelection(layout) {
        _elementLayer.layoutElementSelection(layout)
    }

    function elementItemAt(index) {
        return _elementLayer.elementItemAt(index)
    }

    EventFilter.events: [EventFilter.DragEnter, EventFilter.DragMove, EventFilter.Drop]
    EventFilter.onFilter: (object, event, result) => {
                              _private.filterEvent(event, result)
                          }

    Component.onCompleted: AnnotationPropertyEditorDock.canvasItemsBoundingBox = _private.itemsBoundingBox

    FocusTracker.window: Scrite.window
    FocusTracker.objectName: "StructureCanvas"
    FocusTracker.evaluationMethod: FocusTracker.StandardFocusEvaluation
    FocusTracker.indicator.target: _private
    FocusTracker.indicator.property: "hasFocus"

    width: _private.widthBinder.get
    height: _private.heightBinder.get

    border.width: 2
    border.color: Runtime.structureCanvasSettings.gridColor

    tickDistance: Scrite.document.structure.canvasGridSize
    gridIsVisible: Runtime.structureCanvasSettings.showGrid && root.canvasScrollInteractive
    majorTickColor: Runtime.structureCanvasSettings.gridColor
    minorTickColor: Runtime.structureCanvasSettings.gridColor
    backgroundColor: root.canvasScrollInteractive ? Runtime.colors.primary.c10.background : Color.translucent(Runtime.colors.primary.c300.background, 0.75)

    scale: root.canvasScrollSuggestedScale
    antialiasing: false
    tickColorOpacity: 0.25 * scale
    majorTickLineWidth: 2*Scrite.app.devicePixelRatio
    minorTickLineWidth: 1*Scrite.app.devicePixelRatio

    transformOrigin: Item.TopLeft

    onScaleIsLessForEditChanged: {
        if(scaleIsLessForEdit)
            tabSequence.releaseFocus()
    }

    Behavior on backgroundColor {
        enabled: Runtime.applicationSettings.enableAnimations
        ColorAnimation { duration: 250 }
    }

    AnnotationLayer {
        id: _annotationLayer

        canvasScale: root.scale
        canvasContextMenu: _canvasContextMenu
        canvasScrollMoving: root.canvasScrollMoving
        canvasScrollFlicking: root.canvasScrollFlicking
        canvasItemsBoundingBox: _private.itemsBoundingBox

        canvasScrollViewportRect: root.canvasScrollViewportRect

        anchors.fill: parent
        enabled: !_createItemHandler.active && opacity === 1
        opacity: _elementLayer.rubberband.selectionMode || _elementLayer.rubberband.selecting ? 0.1 : 1

        onCanvasActiveFocusRequest: root.forceActiveFocus()
        onEnsureAreaVisibleRequest: (area) => { root.ensureAreaVisibleRequest(area) }
    }

    StructureElementLayer {
        id: _elementLayer

        anchors.fill: parent

        canvasScrollMoving: root.canvasScrollMoving
        canvasScrollFlicking: root.canvasScrollFlicking

        canvasScale: root.scale
        canvasTabSequence: _private.tabSequence
        canvasScrollViewport: root
        canvasHasActiveFocus: root.activeFocus
        canvasItemsBoundingBox: _private.itemsBoundingBox
        canvasPreviewIsUpdating: root.canvasPreviewUpdatingThumbnail
        canvasHasAnnotationGrip: _annotationLayer.grip !== null
        canvasScaleIsLessForEdit: root.scaleIsLessForEdit
        canvasScrollViewportRect: root.canvasScrollViewportRect
        canvasInInteractiveCreationMode: _createItemHandler.active

        enabled: !_createItemHandler.enabled

        onDeleteElementRequest: (element) => {
                                    root.deleteElementRequest(element)
                                }

        onDeleteElementsRequest: (elementList) => {
                                     root.deleteElementsRequest(elementList)
                                 }

        onCanvasActiveFocusRequest: () => {
                                        root.forceActiveFocus()
                                    }

        onEditElementItemChanged: () => {
            if(editElementItem)
                _annotationLayer.resetGrip()
        }

        onZoomOneRequest: () => {
                            root.zoomOneRequest()
                          }

        onZoomOneToItemRequest: (item) => {
                                    root.zoomOneToItemRequest(item)
                                }

        onEditorRequest: () => {
                            root.editorRequest()
                         }

        onSelectionModeOffRequest: () => {
                                       root.selectionModeOffRequest()
                                   }

        onDenyCanvasPreviewRequest: () => {
                                        root.denyCanvasPreviewRequest()
                                    }

        onAllowCanvasPreviewRequest: () => {
                                        root.allowCanvasPreviewRequest()
                                     }

        onResetAnnotationGripRequest: () => {
                                          _annotationLayer.resetGrip()
                                      }

        onEnsureItemVisibleRequest: (item) => {
                                        root.ensureItemVisibleRequest(item)
                                    }

        onRectangleAnnotationRequest: (x, y, w, h) => {
                                        _annotationLayer.dropRectangleAnnotation(x, y, w, h)
                                      }
    }

    MouseArea {
        anchors.fill: parent

        enabled: _elementLayer.editElementItem === null && !_elementLayer.selection.active
        acceptedButtons: Qt.RightButton

        onPressed: _canvasContextMenu.popup()
    }

    Loader {
        id: _createItemHandler

        property string mode

        function done() { mode = "" }

        anchors.fill: parent

        active: mode !== ""
        visible: active
        enabled: active
        sourceComponent: {
            if(mode === "")
                return null

            return mode === "element" ? _private.elementCreationMouseArea : _private.annotationCreationMouseArea
        }

        onActiveChanged: {
            if(active)
                _annotationLayer.resetGrip()
        }
    }

    StructureCanvasContextMenu {
        id: _canvasContextMenu

        coloredSceneColor: root.newSceneColor

        onVisibleChanged: () => {
                              if(visible)
                                _annotationLayer.resetGrip()
                          }

        onCreateElementRequest: (x, y, sceneColor) => {
                                    _private.createElement(x, y, sceneColor)
                                }
        onCreateAnnotationRequest: (x, y, type) => {
                                       _private.createAnnotation(x, y, type)
                                   }
    }

    ActionHandler {
        action: ActionHub.screenplayOperations.find("newScene")

        enabled: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI && _private.hasFocus

        onTriggered: (source) => {
                         const cardIndex = Scrite.document.structure.currentElementIndex
                         const card = _elementLayer.elementItemAt(cardIndex)
                         if(card)
                            card.forceActiveFocus()
                     }
    }

    QtObject {
        id: _private

        property bool hasFocus: false

        readonly property TabSequenceManager tabSequence: TabSequenceManager {
            wrapAround: true
        }

        readonly property BoundingBoxEvaluator itemsBoundingBox : BoundingBoxEvaluator {
            margin: 50
            initialRect: GMath.uniteRectangles(Scrite.document.structure.annotationsBoundingBox,
                                               root.canvasScrollViewportRect)
        }

        readonly property DelayedProperty widthBinder: DelayedProperty {
            set: Math.max( Math.ceil(_private.itemsBoundingBox.right / 100) * 100, 120000 )
            initial: 1000

            onGetChanged: Scrite.document.structure.canvasWidth = get
        }

        readonly property DelayedProperty heightBinder : DelayedProperty {
            set: Math.max( Math.ceil(_private.itemsBoundingBox.bottom / 100) * 100, 120000 )
            initial: 1000

            onGetChanged: Scrite.document.structure.canvasHeight = get
        }

        readonly property Component annotationCreationMouseArea: CreateAnnotationMouseArea {
            scale: root.scale
            annotationType: _createItemHandler.mode

            onDone: _createItemHandler.done()

            onCreateAnnotationRequest: (x, y, type) => {
                                           _annotationLayer.createAnnotation(type, x, y)
                                       }
        }

        readonly property Component elementCreationMouseArea: CreateStructureElementMouseArea {
            scale: root.scale
            sceneColor: root.newSceneColor

            onDone: _createItemHandler.done()

            onCreateElementRequest: (x, y, sceneColor) => {
                                        _elementLayer.createElement(x, y, sceneColor)
                                    }
        }

        function createItem(what, where) {
            if(Scrite.document.readOnly)
                return null

            if(what === undefined || what === "" | what === "element")
                return createElement(where.x, where.y, root.newSceneColor)
            else
                return createAnnotation(where.x, where.y, what)
        }

        function createAnnotation(x, y, type) {
            return _annotationLayer.createAnnotation(type, x, y)
        }

        function createElement(x, y, color) {
            return _elementLayer.createElement(x, y, color)
        }

        function filterEvent(event, result) {
            result.acceptEvent = false

            switch(event.type) {
                case EventFilter.DragEnter:
                case EventFilter.DragLeave:
                case EventFilter.Drop:
                break
                default:
                return
            }

            let sceneId = event.mimeData[Runtime.timelineViewSettings.dropAreaKey]
            let element = Scrite.document.structure.findElementBySceneID(sceneId)
            if(element === null)
                return

            if(element.stackId === "")
                return

            result.acceptEvent = true
            result.filter = true

            if(event.type === EventFilter.Drop) {
                element.stackId = ""
                Runtime.execLater(element, 250, function() {
                    if(!Scrite.document.structure.forceBeatBoardLayout) {
                        element.x = event.pos.x
                        element.y = event.pos.y
                    }
                    Scrite.document.structure.currentElementIndex = Scrite.document.structure.indexOfElement(element)
                    Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.firstIndexOfScene(element.scene)
                })
            }
        }
    }
}
