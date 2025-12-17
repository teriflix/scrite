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
import "qrc:/qml/structureview/structureelements"

Item {
    id: root

    required property bool canvasScrollMoving
    required property bool canvasScrollFlicking
    required property bool canvasHasActiveFocus
    required property bool canvasHasAnnotationGrip
    required property bool canvasPreviewIsUpdating
    required property bool canvasScaleIsLessForEdit
    required property bool canvasInInteractiveCreationMode

    required property rect canvasScrollViewportRect

    required property real canvasScale

    required property Item canvasScrollViewport
    required property TabSequenceManager canvasTabSequence
    required property BoundingBoxEvaluator canvasItemsBoundingBox

    readonly property alias selection: _selection
    readonly property alias rubberband: _rubberband
    readonly property alias groupBoxes: _private.groupBoxes
    readonly property alias episodeBoxes: _private.episodeBoxes
    readonly property alias groupBoxCount: _private.groupBoxCount
    readonly property alias episodeBoxCount: _private.episodeBoxCount
    readonly property alias editElementItem: _private.editElementItem
    readonly property alias elementItemCount: _elementItems.count
    readonly property alias currentElementItem: _private.currentElementItem
    readonly property alias draggedElementItem: _private.draggedElementItem

    property alias rubberbandSelectionMode: _rubberband.selectionMode
    readonly property alias rubberbandActive: _rubberband.active
    readonly property alias rubberbandSelecting: _rubberband.selecting

    property bool groupsBeingMoved: false

    property string groupCategory: Scrite.document.structure.preferredGroupCategory

    signal zoomOneRequest()
    signal editorRequest()
    signal deleteElementsRequest(var elementList)
    signal deleteElementRequest(StructureElement element)
    signal zoomOneToItemRequest(Item item)
    signal selectionModeOffRequest()
    signal canvasActiveFocusRequest()
    signal denyCanvasPreviewRequest()
    signal allowCanvasPreviewRequest()
    signal resetAnnotationGripRequest()
    signal ensureItemVisibleRequest(Item item)
    signal rectangleAnnotationRequest(real x, real y, real w, real h)

    function createElement(x, y, color) {
        return _private.createElement(x, y, color)
    }

    function selectAllElements() {
        _selection.clear()
        _selection.init(_elementItems, root.canvasItemsBoundingBox.boundingBox, true)
    }

    function layoutElementSelection(layout) {
        _selection.layout(layout)
    }

    function elementItemAt(index) {
        return _elementItems.itemAt(index)
    }

    Component.onCompleted: {
        Runtime.execLater(root, 250, _private.reevaluateEpisodeAndGroupBoxes)
    }

    RubberBand {
        id: _rubberband

        anchors.fill: parent

        z: active ? 1000 : -1

        onSelect: {
            _selection.init(_elementItems, rectangle)
            root.selectionModeOffRequest()
        }

        onTryStart: {
            parent.forceActiveFocus()
            active = true // TODO
        }
    }

    ActiveStructureElementBackdrop {
        id: _currentElementItemBackdrop

        currentElementItem: _private.currentElementItem

        enabled: !Scrite.document.readOnly
        visible: currentElementItem !== null && currentElementItem.visible
        opacity: root.canvasHasActiveFocus && !_selection.hasItems ? 1 : 0.25
    }

    Repeater {
        id: _episodeBoxes

        model: root.episodeBoxes

        StructureCanvasEpisodeBox {
            required property int index
            required property var modelData

            episodeBox: modelData
            episodeBoxIndex: index
            episodeBoxCount: root.episodeBoxes.length

            canvasScrollViewport: root.canvasScrollViewport
            canvasItemsBoundingBox: root.canvasItemsBoundingBox
            canvasScrollViewportRect: root.canvasScrollViewportRect

            enabled: !root.canvasInInteractiveCreationMode && !_currentElementItemBackdrop.visible && !root.canvasHasAnnotationGrip
        }
    }

    Repeater {
        id: _groupBoxes

        model: root.groupBoxes

        StructureCanvasGroupBox {
            required property int index
            required property var modelData

            enabled: !root.canvasInInteractiveCreationMode && !root.canvasHasAnnotationGrip

            groupBox: modelData
            elementItems: _elementItems
            groupBoxIndex: index
            groupBoxCount: root.groupBoxes.length

            canvasScrollViewport: root.canvasScrollViewport
            canvasItemsBoundingBox: root.canvasItemsBoundingBox
            canvasScrollViewportRect: root.canvasScrollViewportRect

            onSetSelectionRequest: (items) => { _selection.set(items) }
            onClearSelectionRequest: () => { _selection.clear() }
        }
    }

    Repeater {
        id: _elementConnectorItems

        model: Scrite.document.loading ? 0 : Scrite.document.structureElementConnectors

        delegate: StructureElementConnectorDelegate {
            required property string connectorLabel

            required property StructureElement connectorToElement
            required property StructureElement connectorFromElement

            labelText: connectorLabel
            toElement: connectorToElement
            fromElement: connectorFromElement
            canvasScale: root.canvasScale
            canvasScrollViewport: root.canvasScrollViewport
            canvasItemsBoundingBox: root.canvasItemsBoundingBox
            canvasScrollViewportRect: root.canvasScrollViewportRect

            labelVisible: !root.canvasPreviewIsUpdating
        }
    }

    MouseArea {
        id: _finishEditingMouseArea

        anchors.fill: parent

        enabled: _private.editElementItem
        acceptedButtons: Qt.LeftButton

        onClicked: _private.editElementItem.finishEditingRequest()
    }

    Repeater {
        id: _elementStacks

        model: Scrite.document.loading ? null : Scrite.document.structure.elementStacks

        delegate: StructureElementStackTabBar {
            required property QtObject objectItem

            elementStack: objectItem

            canvasScrollViewport: root.canvasScrollViewport
            canvasItemsBoundingBox: root.canvasItemsBoundingBox
            canvasScrollViewportRect: root.canvasScrollViewportRect
        }
    }

    DropArea {
        id: _catchAllDropArea
        anchors.fill: parent

        keys: [Runtime.timelineViewSettings.dropAreaKey]

        onDropped: (drop) => {
            const otherScene = Object.typeOf(drop.source) === "ScreenplayElement" ? drop.source.scene : drop.source
            if(Scrite.document.screenplay.firstIndexOfScene(otherScene) < 0) {
                MessageBox.information("",
                    "Scenes must be added to the timeline before they can be stacked."
                )
                drop.ignore()
                return
            }

            const otherSceneId = otherScene.id
            const otherElement = Scrite.document.structure.findElementBySceneID(otherSceneId)
            if(otherElement === null) {
                drop.ignore()
                return
            }

            otherElement.unstack()
            drop.acceptProposedAction()
        }
    }

    Repeater {
        id: _elementItems

        model: Scrite.document.loading ? null : Scrite.document.structure.elementsModel

        delegate: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI ? _private.indexCardDelegate : _private.synopsisBoxDelegate
    }

    StructureElementsSelection {
        id: _selection

        anchors.fill: parent

        onZoomOneRequest: root.zoomOneRequest()
        onDeleteElementsRequest: (elementList) => { root.deleteElementsRequest(elementList) }
        onDenyCanvasPreviewRequest: root.denyCanvasPreviewRequest()
        onAllowCanvasPreviewRequest: root.allowCanvasPreviewRequest()
        onEnsureItemVisibleRequest: (item) => { root.ensureItemVisibleRequest(item) }
        onRectangleAnnotationRequest: (x, y, w, h) => { root.rectangleAnnotationRequest(x, y, w, h) }
        onInitiateSelectionInBoundaryRequest: (boundary) => { init(_elementItems, boundary) }
    }

    StructureElementContextMenu {
        id: _elementContextMenu

        onRefitSelectionRequest: { Runtime.execLater(_selection, 250, function() { _selection.refit() }) }
        onEnsureItemVisibleRequest: (item) => { root.ensureItemVisibleRequest(item) }
        onDeleteElementRequest: (element) => {  root.deleteElementRequest(element) }
    }

    onGroupCategoryChanged: {
        // Scrite.document.structure.preferredGroupCategory = groupCategory
        Runtime.execLater(root, 250, _private.reevaluateEpisodeAndGroupBoxes)
    }

    QtObject {
        id: _private

        property var groupBoxes: []
        property var episodeBoxes: []
        property int groupBoxCount: groupBoxes && groupBoxes.length ? groupBoxes.length : 0
        property int episodeBoxCount: episodeBoxes && episodeBoxes.length ? episodeBoxes.length : 0

        property AbstractStructureElementUI editElementItem: null
        property AbstractStructureElementUI currentElementItem: currentElementItemBinder.get
        property AbstractStructureElementUI draggedElementItem: null

        readonly property SequentialAnimation layoutAnimation: StructureElementsLayoutTask {
            onDenyCanvasPreviewRequest: root.denyCanvasPreviewRequest()
            onAllowCanvasPreviewRequest: root.allowCanvasPreviewRequest()
        }

        readonly property Component indexCardDelegate: StructureElementIndexCard {
            id: _indexCard

            required property int index
            required property StructureElement modelData

            element: modelData
            elementIndex: index

            canvasTabSequence: root.canvasTabSequence
            canvasScrollMoving: root.canvasScrollMoving
            canvasScrollFlicking: root.canvasScrollFlicking
            canvasScrollViewport: root.canvasScrollViewport
            canvasItemsBoundingBox: root.canvasItemsBoundingBox
            canvasScaleIsLessForEdit: root.canvasScaleIsLessForEdit
            canvasScrollViewportRect: root.canvasScrollViewportRect

            onIsEditingChanged: {
                if(isEditing)
                    _private.editElementItem = _indexCard
                else if(_private.editElementItem === _indexCard)
                    _private.editElementItem = null
            }

            onIsBeingDraggedChanged: {
                if(isBeingDragged)
                    _private.draggedElementItem = _indexCard
                else if(_private.draggedElementItem === _indexCard)
                    _private.draggedElementItem = null
            }

            onEditorRequest: () => {
                                 root.editorRequest()
                             }

            onRequestContextMenu: (element) => {
                                      _elementContextMenu.element = element
                                      _elementContextMenu.popup()
                                  }

            onResetAnnotationGripRequest: () => {
                                              root.resetAnnotationGripRequest()
                                          }

            onZoomOneToItemRequest: (item) => {
                                        root.zoomOneToItemRequest(item)
                                    }

            onDeleteElementRequest: (element) => {
                                        root.deleteElementRequest(element)
                                    }

            onFinishEditingRequest: () => { } // ???

            onCanvasActiveFocusRequest: () => {
                                            root.canvasActiveFocusRequest()
                                        }
        }

        readonly property Component synopsisBoxDelegate: StructureElementSynopsisBox {
            id: _synopsisBox

            required property int index
            required property StructureElement modelData

            element: modelData
            elementIndex: index

            canvasTabSequence: root.canvasTabSequence
            canvasScrollMoving: root.canvasScrollMoving
            canvasScrollFlicking: root.canvasScrollFlicking
            canvasScrollViewport: root.canvasScrollViewport
            canvasItemsBoundingBox: root.canvasItemsBoundingBox
            canvasScaleIsLessForEdit: root.canvasScaleIsLessForEdit
            canvasScrollViewportRect: root.canvasScrollViewportRect

            onIsEditingChanged: {
                if(isEditing)
                    _private.editElementItem = _synopsisBox
                else if(_private.editElementItem === _synopsisBox)
                    _private.editElementItem = null
            }

            onIsBeingDraggedChanged: {
                if(isBeingDragged)
                    _private.draggedElementItem = _synopsisBox
                else if(_private.draggedElementItem === _synopsisBox)
                    _private.draggedElementItem = null
            }

            onEditorRequest: () => {
                                 root.editorRequest()
                             }

            onRequestContextMenu: (element) => {
                                      _elementContextMenu.element = element
                                      _elementContextMenu.popup()
                                  }

            onResetAnnotationGripRequest: () => {
                                              root.resetAnnotationGripRequest()
                                          }

            onZoomOneToItemRequest: (item) => {
                                        root.zoomOneToItemRequest(item)
                                    }

            onDeleteElementRequest: (element) => {
                                        root.deleteElementRequest(element)
                                    }

            onFinishEditingRequest: () => { } // TODO ???

            onCanvasActiveFocusRequest: () => {
                                            root.canvasActiveFocusRequest()
                                        }
        }

        readonly property DelayedProperty currentElementItemBinder: DelayedProperty {
            initial: null
            set: _elementItems.count > Scrite.document.structure.currentElementIndex ? _elementItems.itemAt(Scrite.document.structure.currentElementIndex) : null
            onGetChanged: { if(get) root.ensureItemVisibleRequest(get) }
        }

        readonly property TrackerPack boxEvaluationTracker: TrackerPack {
            delay: 250

            TrackProperty {
                target: _elementItems
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

            onTracked: _private.reevaluateEpisodeAndGroupBoxes()
        }

        readonly property Component newStructureElementTemplate: Component {
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

        function createElement(x, y, color) {
            if(Scrite.document.readOnly)
                return null

            var props = {
                "x": Math.max(Scrite.document.structure.snapToGrid(x), 130),
                "y": Math.max(Scrite.document.structure.snapToGrid(y), 50)
            }

            let element = newStructureElementTemplate.createObject(Scrite.document.structure, props)
            element.scene.color = color

            Scrite.document.structure.addElement(element)
            Scrite.document.structure.currentElementIndex = Scrite.document.structure.elementCount-1
            root.editorRequest()
            root.canvasActiveFocusRequest()

            element.scene.undoRedoEnabled = true

            return element
        }

        function reevaluateEpisodeAndGroupBoxes() {
            if(root.groupsBeingMoved)
                return

            const egBoxes = Scrite.document.structure.evaluateEpisodeAndGroupBoxes(Scrite.document.screenplay, root.groupCategory)
            groupBoxes = egBoxes.groupBoxes
            episodeBoxes = egBoxes.episodeBoxes
        }
    }
}

