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
import "qrc:/qml/helpers"

Item {
    id: root

    required property Item annotationItem
    required property Annotation annotation
    required property Item annotationContainer
    required property real structureCanvasScale

    signal resetRequest()
    signal requestCanvasFocus()

    EventFilter.target: Scrite.app
    EventFilter.active: enabled && visible && !AnnotationPropertyEditorDock.visible
    EventFilter.events: [EventFilter.KeyPress]
    EventFilter.onFilter: (object, event, result) => { _private.filterEvent(event, result) }

    Component.onCompleted: requestCanvasFocus()

    x: annotationItem.x
    y: annotationItem.y
    width: annotationItem.width
    height: annotationItem.height

    enabled: !Scrite.document.readOnly

    AnnotationToolBar {
        anchors.left: parent.left
        anchors.bottom: parent.top
        anchors.margins: _private.gripSize

        scale: 1.0 / root.structureCanvasScale

        annotation: root.annotation

        onResetRequest: root.resetRequest()
    }

    PainterPathItem {
        id: _focusIndicator

        anchors.fill: parent
        anchors.margins: -_private.gripSize/2

        renderType: PainterPathItem.OutlineOnly
        outlineColor: Runtime.colors.accent.a700.background
        outlineStyle: PainterPathItem.DashDotDotLine
        outlineWidth: _private.onePxSize
        renderingMechanism: PainterPathItem.UseQPainter

        painterPath: PainterPath {
            MoveTo { x: _private.onePxSize; y: _private.onePxSize }
            LineTo { x: _focusIndicator.width-_private.onePxSize; y: _private.onePxSize }
            LineTo { x: _focusIndicator.width-_private.onePxSize; y: _focusIndicator.height-_private.onePxSize }
            LineTo { x: _private.onePxSize; y: _focusIndicator.height-_private.onePxSize }
            CloseSubpath { }
        }
    }

    MouseArea {
        anchors.fill: parent

        enabled: annotation.movable
        cursorShape: Qt.SizeAllCursor
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton|Qt.RightButton
        propagateComposedEvents: true

        drag.axis: Drag.XAndYAxis
        drag.target: annotationItem
        drag.minimumX: 0
        drag.minimumY: 0

        onPressed: requestCanvasFocus()

        onDoubleClicked: {
            if(Runtime.structureCanvasSettings.displayAnnotationProperties === false)
                Runtime.structureCanvasSettings.displayAnnotationProperties = true
        }
    }

    GripHandle {
        id: _rightGrip

        x: parent.width - width/2
        y: (parent.height - height)/2
        dragAxis: Drag.XAxis

        onGripPressed: root.requestCanvasFocus()
        onGripHandleMoved: _private.snapAnnotationGeometryToGrid(Qt.rect(root.x, root.y, x + width/2, root.height))
    }

    GripHandle {
        id: _bottomGrip

        x: (parent.width - width)/2
        y: parent.height - height/2
        dragAxis: Drag.YAxis

        onGripPressed: root.requestCanvasFocus()
        onGripHandleMoved: _private.snapAnnotationGeometryToGrid(Qt.rect(root.x, root.y, root.width, y + height/2))
    }

    GripHandle {
        id: _bottomRightGrip

        x: parent.width - width/2
        y: parent.height - height/2
        dragAxis: Drag.XAndYAxis

        onGripPressed: root.requestCanvasFocus()
        onGripHandleMoved: _private.snapAnnotationGeometryToGrid(Qt.rect(root.x, root.y, x + width/2, y + height/2))
    }

    onXChanged: _private.geoUpdateTimer.start()
    onYChanged: _private.geoUpdateTimer.start()
    onWidthChanged: _private.geoUpdateTimer.start()
    onHeightChanged: _private.geoUpdateTimer.start()

    component GripHandle : Rectangle {
        id: _gripHandle

        property int dragAxis: Drag.XAxis
        property int minimumDragDistance: 20

        signal gripPressed()
        signal gripHandleMoved()

        width: _private.gripSize
        height: _private.gripSize

        color: Runtime.colors.accent.a700.background
        visible: annotation.resizable
        enabled: visible

        onXChanged: {
            if(dragAxis === Drag.XAxis || dragAxis === Drag.XAndYAxis)
                gripHandleWasMoved()
        }
        onYChanged: {
            if(dragAxis === Drag.YAxis || dragAxis === Drag.XAndYAxis)
                gripHandleWasMoved()
        }

        MouseArea {
            id: __gripHandleMouseArea

            anchors.fill: parent

            cursorShape: Qt.SizeHorCursor
            hoverEnabled: true

            drag.axis: _gripHandle.dragAxis
            drag.target: parent
            drag.minimumX: _gripHandle.minimumDragDistance
            drag.minimumY: _gripHandle.minimumDragDistance

            onPressed: _gripHandle.gripPressed()
        }

        function gripHandleWasMoved() {
            Utils.execLater(_gripHandle, _private.geometryUpdateInterval, _gripHandle.gripHandleMoved)
        }
    }

    QtObject {
        id: _private

        readonly property int geometryUpdateInterval: 50
        readonly property real gripSize: 10 * _private.onePxSize
        readonly property real onePxSize: Math.max(1, 1/root.structureCanvasScale)

        readonly property Timer geoUpdateTimer: Timer {
            interval: _private.geometryUpdateInterval

            onTriggered: _private.snapAnnotationGeometryToGrid(Qt.rect(root.x,root.y,root.width,root.height))
        }

        function filterEvent(event, result) {
            let dist = (event.controlModifier ? 5 : 1) * _canvas.tickDistance
            switch(event.key) {
                case Qt.Key_Left: {
                    if(event.shiftModifier) {
                        root.width -= annotation.resizable ? dist : 0
                        root.width = Math.max(root.width, 20)
                    } else {
                        root.x -= annotation.movable ? dist : 0
                    }
                    result.accept = true
                    result.filter = true
                } break
                case Qt.Key_Right: {
                    if(event.shiftModifier) {
                        root.width += annotation.resizable ? dist : 0
                    } else {
                        root.x += annotation.movable ? dist : 0
                    }
                    result.accept = true
                    result.filter = true
                } break
                case Qt.Key_Up: {
                    if(event.shiftModifier) {
                        root.height -= annotation.resizable ? dist : 0
                        root.height = Math.max(root.height, 20)
                    } else {
                        root.y -= annotation.movable ? dist : 0
                    }
                    result.accept = true
                    result.filter = true
                } break
                case Qt.Key_Down: {
                    if(event.shiftModifier) {
                        root.height += annotation.resizable ? dist : 0
                    } else {
                        root.y += annotation.movable ? dist : 0
                    }
                    result.accept = true
                    result.filter = true
                } break
                case Qt.Key_F2: {
                    if(Runtime.structureCanvasSettings.displayAnnotationProperties === false) {
                        Runtime.structureCanvasSettings.displayAnnotationProperties = true
                        result.accept = true
                        result.filter = true
                    }
                } break
                case Qt.Key_Escape: {
                    root.resetRequest()
                }
                break
            }
        }

        function snapAnnotationGeometryToGrid(rect) {
            var gx = Scrite.document.structure.snapToGrid(rect.x)
            var gy = Scrite.document.structure.snapToGrid(rect.y)
            var gw = Scrite.document.structure.snapToGrid(rect.width)
            var gh = Scrite.document.structure.snapToGrid(rect.height)
            annotation.geometry = Qt.rect(gx, gy, gw, gh)
        }
    }
}
