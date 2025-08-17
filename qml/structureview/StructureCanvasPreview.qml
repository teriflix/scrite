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

Item {
    id: root

    required property StructureCanvasScrollArea canvasScroll

    readonly property real maxSize: 150

    property alias interacting: _panMouseArea.pressed
    property alias updatingThumbnail: _preview.isUpdatingPreview

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
                var scale = root.canvasScroll.itemsBoundingBox.width / _preview.width
                var x = root.canvasScroll.itemsBoundingBox.x + mouse.x * scale - root.canvasScroll.width/2
                var y = root.canvasScroll.itemsBoundingBox.y + mouse.y * scale - root.canvasScroll.height/2
                var area = Qt.rect(x,y,root.canvasScroll.width,root.canvasScroll.height)

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

            color: Scrite.app.translucent(Runtime.colors.accent.highlight.background, 0.25)
            border.width: 2
            border.color: Runtime.colors.accent.borderColor

            DelayedPropertyBinder {
                id: _geometryBinder

                set: {
                    if(!root.visible)
                        return Qt.rect(0,0,0,0)

                    var visibleRect = root.canvasScroll.viewportRect
                    if( Scrite.app.isRectangleInRectangle(visibleRect, root.canvasScroll.itemsBoundingBox.boundingBox) )
                        return Qt.rect(0,0,0,0)

                    var intersect = Scrite.app.intersectedRectangle(visibleRect, root.canvasScroll.itemsBoundingBox.boundingBox)
                    var scale = _preview.width / Math.max(root.canvasScroll.itemsBoundingBox.width, 500)
                    var ret = Qt.rect( (intersect.x-root.canvasScroll.itemsBoundingBox.left)*scale,
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
                var scale = _preview.width / Math.max(root.canvasScroll.itemsBoundingBox.width, 500)
                var ix = (x/scale)+root.canvasScroll.itemsBoundingBox.left
                var iy = (y/scale)+root.canvasScroll.itemsBoundingBox.top
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

        function evaluatePreviewSize() {
            var w = Math.max(root.canvasScroll.itemsBoundingBox.width, 500)
            var h = Math.max(root.canvasScroll.itemsBoundingBox.height, 500)

            var scale = 1
            if(w < h)
                scale = maxSize / w
            else
                scale = maxSize / h

            w *= scale
            h *= scale

            if(w > root.canvasScroll.width-60)
                scale = (root.canvasScroll.width-60)/w
            else if(h >= root.canvasScroll.height-60)
                scale = (root.canvasScroll.height-60)/h
            else
                scale = 1

            w *= scale
            h *= scale

            return Qt.size(w+10, h+10)
        }
    }
}
