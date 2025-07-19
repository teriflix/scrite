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
import "qrc:/qml/structureview/annotations"

Item {
    id: root

    required property VclMenu canvasContextMenu

    readonly property var annotationsList: AnnotationFactory.keys

    required property real canvasScale
    required property rect canvasScrollViewportRect
    required property BoundingBoxEvaluator canvasItemsBoundingBox

    signal canvasActiveFocusRequest()
    signal ensureAreaVisibleRequest(rect area)

    property AnnotationGrip grip: _gripLoader.item

    function createAnnotation(type, x, y) {
        return _private.createAnnotation(type, x, y)
    }

    function resetGrip() {
        _gripLoader.reset()
    }

    Behavior on opacity {
        enabled: Runtime.applicationSettings.enableAnimations

        NumberAnimation { duration: 250 }
    }

    MouseArea {
        anchors.fill: parent

        enabled: _gripLoader.active

        onClicked: _gripLoader.reset()
    }

    StructureCanvasViewportFilterModel {
        id: _annotationsFilterModel

        type: StructureCanvasViewportFilterModel.AnnotationType
        enabled: Scrite.document.loading ? false : Scrite.document.structure.annotationCount > 100
        structure: Scrite.document.structure
        viewportRect: root.canvasScrollViewportRect
        filterStrategy: StructureCanvasViewportFilterModel.IntersectsStrategy
        computeStrategy: StructureCanvasViewportFilterModel.PreComputeStrategy
    }

    Repeater {
        id: _annotationItems

        model: _annotationsFilterModel

        delegate: Item {
            id: _annotationDelegateContainer

            property int annotationIndex: index
            property Annotation annotation: modelData
            property bool active: !_annotationsFilterModel.enabled
            property AbstractAnnotationDelegate delegate: null

            Component.onCompleted: getReady()

            visible: active

            onActiveChanged: getReady()

            function getReady() {
                if(active) {
                    delegate = _private.createAnnotationDelegate(annotation, annotationIndex, _annotationDelegateContainer)
                    width = delegate.width
                    height = delegate.height
                } else {
                    delegate.destroy()
                    delegate = null
                    width = 1
                    height = 1
                }
            }
        }
    }

    Loader {
        id: _gripLoader

        property Item annotationItem
        property Annotation annotation

        Component.onDestruction: reset()

        active: annotation !== null && annotationItem !== null
        sourceComponent: AnnotationGrip {
            annotation: _gripLoader.annotation
            annotationItem: _gripLoader.annotationItem
            structureCanvasScale: root.canvasScale

            onResetRequest: _gripLoader.reset()
            onRequestCanvasFocus: root.canvasActiveFocusRequest()
        }

        function reset() {
            annotation = null
            annotationItem = null
        }

        Connections {
            target: Scrite.document.structure

            function onCurrentElementIndexChanged() {
                if(Scrite.document.structure.currentElementIndex >= 0)
                    _gripLoader.reset()
            }

            function onAnnotationCountChanged() {
                _gripLoader.reset()
            }
        }

        Connections {
            target: Scrite.document.screenplay

            function onCurrentElementIndexChanged(val) {
                let element = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.currentElementIndex)
                let info = Scrite.document.structure.queryBreakElements(element)
                if(info.indexes && info.indexes.length > 0) {
                    let fi = info.indexes[0]
                    let fe = Scrite.document.structure.elementAt(fi)
                    if(fe === null)
                        return

                    let area = fe.geometry
                    let topPadding = element.breakType === Screenplay.Episode ? 150 : 90

                    area = Qt.rect(area.x-50, area.y-topPadding, area.width, area.height)
                    root.ensureAreaVisibleRequest(area)
                }
            }
        }

        onAnnotationChanged: {
            AnnotationPropertyEditorDock.annotation = annotation
            if(annotation === null)
                annotationItem = null
        }
    }

    QtObject {
        id: _private

        function createAnnotation(type, x, y) {
            if(Scrite.document.readOnly)
                return null

           return AnnotationFactory.create(type, x, y, root)
        }

        function getAnnotationDelegate(type) {
            return AnnotationFactory.delegateFor(type)
        }

        function createAnnotationDelegate(annotation, annotationIndex, parent) {
            let delgate = AnnotationFactory.createDelegate(annotation, annotationIndex, parent)
            if(delegate) {
                delegate.BoundingBoxItem.evaluator = root.canvasItemsBoundingBox
                delegate.BoundingBoxItem.livePreview = true
                delegate.gripRequest.connect(_private.gripDelegate)
            }
        }

        function gripDelegate(delegate, annotation) {
            _gripLoader.reset()
            _gripLoader.annotationItem = delegate
            _gripLoader.annotation = annotation
        }
    }
}

