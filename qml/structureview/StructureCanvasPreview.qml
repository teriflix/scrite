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

Item {
    id: root

    required property StructureCanvasScrollArea canvasScroll

    readonly property alias interacting: _panMouseArea.pressed
    readonly property alias updatingThumbnail: _preview.isUpdatingPreview
    readonly property alias isContentOverflowing: _private.isContentOverflowing

    property bool allowed: true

    property size previewSize: _private.evaluatePreviewSize()

    width: previewSize.width
    height: previewSize.height

    BoxShadow {
        anchors.fill: parent
        opacity: 0.55 * _preview.opacity
    }

    BoundingBoxPreview {
        id: _preview

        anchors.fill: parent
        anchors.margins: 5

        evaluator: root.canvasScroll.itemsBoundingBox
        backgroundColor: Runtime.colors.primary.c100.background
        backgroundOpacity: 0.9

        MouseArea {
            id: _jumpToMouseArea

            anchors.fill: parent

            enabled: root.canvasScroll.itemsBoundingBox.itemCount > 0

            onClicked: {
                let scale = root.canvasScroll.itemsBoundingBox.width / _preview.width
                let x = root.canvasScroll.itemsBoundingBox.x + mouse.x * scale - root.canvasScroll.width/2
                let y = root.canvasScroll.itemsBoundingBox.y + mouse.y * scale - root.canvasScroll.height/2
                let area = Qt.rect(x,y,root.canvasScroll.width,root.canvasScroll.height)

                root.canvasScroll.zoomOne()
                root.canvasScroll.ensureVisible(area)
            }
        }

        Rectangle {
            id: _viewportIndicator

            x: _geometryBinder.get.x
            y: _geometryBinder.get.y
            width: _geometryBinder.get.width
            height: _geometryBinder.get.height

            color: Color.translucent(Runtime.colors.accent.highlight.background, 0.25)
            border.width: 2
            border.color: Runtime.colors.accent.borderColor

            DelayedPropertyBinder {
                id: _geometryBinder

                set: {
                    let visibleRect = root.canvasScroll.viewportRect
                    if( GMath.isRectangleInRectangle(visibleRect, root.canvasScroll.itemsBoundingBox.boundingBox) )
                        return Qt.rect(0,0,0,0)

                    let intersect = GMath.intersectedRectangle(visibleRect, root.canvasScroll.itemsBoundingBox.boundingBox)
                    let scale = _preview.width / Math.max(root.canvasScroll.itemsBoundingBox.width, 500)
                    let ret = Qt.rect( (intersect.x-root.canvasScroll.itemsBoundingBox.left)*scale,
                                       (intersect.y-root.canvasScroll.itemsBoundingBox.top)*scale,
                                       (intersect.width*scale),
                                       (intersect.height*scale) )
                    return ret
                }
                delay: 10
                initial: Qt.rect(0,0,0,0)
            }

            MouseArea {
                id: _panMouseArea

                anchors.fill: parent

                drag.axis: Drag.XAndYAxis
                drag.target: parent
                drag.minimumX: 0
                drag.minimumY: 0
                drag.maximumX: _preview.width - parent.width
                drag.maximumY: _preview.height - parent.height
                drag.onActiveChanged: root.canvasScroll.animatePanAndZoom = !drag.active

                enabled: parent.width > 0 && parent.height > 0
                cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                hoverEnabled: drag.active
            }

            onXChanged: {
                if(_panMouseArea.drag.active)
                    panViewport()
            }
            onYChanged: {
                if(_panMouseArea.drag.active)
                    panViewport()
            }

            function panViewport() {
                let scale = _preview.width / Math.max(root.canvasScroll.itemsBoundingBox.width, 500)
                let ix = (x/scale)+root.canvasScroll.itemsBoundingBox.left
                let iy = (y/scale)+root.canvasScroll.itemsBoundingBox.top
                root.canvasScroll.contentX = ix * root.canvasScroll.canvas.scale
                root.canvasScroll.contentY = iy * root.canvasScroll.canvas.scale
            }
        }
    }

    onVisibleChanged: {
        if(visible)
            root.canvasScroll.itemsBoundingBox.markPreviewDirty()
    }

    QtObject {
        id: _private

        property real previewSize: Runtime.structureCanvasSettings.previewSize

        property bool isContentOverflowing: {
            const of = 0.075 // Runtime.structureCanvasSettings.overflowFactor
            const xf = _preview.width * of
            const yf = _preview.height * of
            return _viewportIndicator.x >= xf || _viewportIndicator.y >= yf || _viewportIndicator.width < (_preview.width - 2*xf) || _viewportIndicator.height < (_preview.height - 2*yf)
        }

        function evaluatePreviewSize() {
            const maxSize = Math.min( Math.min(root.canvasScroll.width-60, root.canvasScroll.height-60), previewSize)
            const w = root.canvasScroll.itemsBoundingBox.width
            const h = root.canvasScroll.itemsBoundingBox.height
            const scale = Math.min(maxSize/w, maxSize/h)
            return Qt.size(w*scale+10, h*scale+10)
        }

        onPreviewSizeChanged: root.canvasScroll.itemsBoundingBox.markPreviewDirty()
    }
}
