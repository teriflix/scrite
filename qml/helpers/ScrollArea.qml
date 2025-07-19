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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"

Flickable {
    id: root

    property bool changing: false
    property bool zoomOnScroll: Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform
    property bool showScrollBars: true
    property bool animatePanAndZoom: true
    property bool animatingPanOrZoom: contentXAnimation.running || contentYAnimation.running || zoomScaleAnimation.running

    property real zoomScale: 1
    property rect visibleRect: Qt.rect(contentX, contentY, width, height)
    property real suggestedScale: zoomScale
    property real initialContentWidth: 100
    property real initialContentHeight: 100

    property alias handlePinchZoom: pinchHandler.enabled
    property alias minimumScale: pinchHandler.minimumScale
    property alias maximumScale: pinchHandler.maximumScale

    signal zoomScaleChangedInteractively()

    function zoomIn() {
        const zf = 1+Runtime.scrollAreaSettings.zoomFactor
        zoomScale = Math.min(zoomScale*zf, pinchHandler.maximumScale)
    }

    function zoomOut() {
        const zf = 1-Runtime.scrollAreaSettings.zoomFactor
        zoomScale = Math.max(zoomScale*zf, pinchHandler.minimumScale)
    }

    function zoomOne() {
        zoomScale = 1
    }

    function zoomTo(val) {
        zoomScale = val
    }

    function zoomFit(area) {
        if(!area)
            return

        if(area.width <= 0 || area.height <= 0)
            return

        var s = Math.min(width/area.width, height/area.height)
        var center = Qt.point(area.x + area.width/2, area.y + area.height/2)
        var w = width/s
        var h = height/s
        area = Qt.rect(Math.max(center.x-w/2,0), Math.max(center.y-h/2,0), w, h)

        zoomScale = s
        ensureVisible(area, s)
    }

    function ensureItemVisibleLater(item, scaling, leaveMargin, delay) {
        ensureItemVisibleTimer.item = item
        ensureItemVisibleTimer.scaling = scaling ? scaling : 1
        ensureItemVisibleTimer.leaveMargin = leaveMargin ? leaveMargin : 0
        ensureItemVisibleTimer.interval = delay ? delay : 500
        ensureItemVisibleTimer.start()
    }

    function ensureItemVisible(item, scaling, leaveMargin) {
        if(item === null)
            return
        var area = Qt.rect(item.x, item.y, item.width, item.height)
        ensureVisible(area, scaling, leaveMargin)
    }

    function ensureVisible(area, scaling, leaveMargin) {
        if(leaveMargin === undefined)
            leaveMargin = 20
        if(scaling === undefined)
            scaling = 1

        area = Qt.rect( area.x*scaling, area.y*scaling,
                        area.width*scaling, area.height*scaling )

        if(area.right > contentWidth || area.bottom > contentHeight || width < 0 || height < 0) {
            _private.ensureVisibleParams = {
                "area": area, "scaling": scaling, "leaveMargin": leaveMargin
            }
            Utils.execLater(root, 500, function() {
                var params = _private.ensureVisibleParams
                _private.ensureVisibleParams = undefined
                ensureVisible(params.area, params.scaling, params.leaveMargin)
            })
            return
        }

        // Check if the areaangle can be contained within the viewport
        if(area.width > visibleRect.width || area.height > visibleRect.height) {
            // We are here if area cannot fit into the space of the flickable.
            // In this case, we just try and fit the center point of area into the
            // view.
            var w = width*0.2
            var h = height*0.2
            area = Qt.rect( (area.left+area.right)/2-h/2,
                            (area.top+area.bottom)/2-w/2,
                            w, h )
        }

        // Check if item is already visible
        if(area.left >= visibleRect.left && area.top >= visibleRect.top &&
           area.right <= visibleRect.right && area.bottom <= visibleRect.bottom)
            return; // already visible

        var cx = undefined
        var cy = undefined
        if(area.left >= visibleRect.right || area.right >= visibleRect.right)
            cx = (area.right + leaveMargin) - width
        else if(area.right <= visibleArea.left || area.left <= visibleRect.left)
            cx = area.left - leaveMargin

        if(area.top >= visibleRect.bottom || area.bottom >= visibleRect.bottom)
            cy = (area.bottom + leaveMargin) - height
        else if(area.bottom <= visibleRect.top || area.top <= visibleRect.top)
            cy = area.top - leaveMargin

        if(cx !== undefined)
            contentX = Math.max(Math.min(cx, contentWidth-width-1),0)
        if(cy !== undefined)
            contentY = Math.max(Math.min(cy, contentHeight-height-1),0)
    }

    function ensureVisibleFast(area) {
        // Check if item is already visible
        if(area.left >= visibleRect.left && area.top >= visibleRect.top &&
           area.right <= visibleRect.right && area.bottom <= visibleRect.bottom)
            return; // already visible

        var cx = undefined
        var cy = undefined
        if(area.left >= visibleRect.right || area.right >= visibleRect.right)
            cx = area.right - width
        else if(area.right <= visibleArea.left || area.left <= visibleRect.left)
            cx = area.left

        if(area.top >= visibleRect.bottom || area.bottom >= visibleRect.bottom)
            cy = area.bottom - height
        else if(area.bottom <= visibleRect.top || area.top <= visibleRect.top)
            cy = area.top

        if(cx !== undefined)
            contentX = Math.max(Math.min(cx, contentWidth-width-1),0)
        if(cy !== undefined)
            contentY = Math.max(Math.min(cy, contentHeight-height-1),0)
    }

    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

    ScrollBar.horizontal: VclScrollBar { flickable: root }
    ScrollBar.vertical: VclScrollBar { flickable: root }

    EventFilter.active: zoomOnScroll
    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: {
        if(event.delta < 0)
            zoomOut()
        else
            zoomIn()
        zoomScaleChangedInteractively()
        result.acceptEvent = true
        result.filter = true
    }

    clip: true
    boundsBehavior: Flickable.StopAtBounds

    Behavior on contentX {
        enabled: Runtime.applicationSettings.enableAnimations && animatePanAndZoom
        NumberAnimation { id: contentXAnimation; duration: 250 }
    }

    Behavior on contentY {
        enabled: Runtime.applicationSettings.enableAnimations && animatePanAndZoom
        NumberAnimation { id: contentYAnimation; duration: 250 }
    }

    Behavior on zoomScale {
        id: zoomScaleBehavior
        property bool allow: true
        enabled: Runtime.applicationSettings.enableAnimations && animatePanAndZoom && allow
        NumberAnimation { id: zoomScaleAnimation; duration: 250 }
    }

    Timer {
        id: ensureItemVisibleTimer

        property Item item
        property real scaling
        property real leaveMargin

        repeat: false
        interval: 250
        running: false

        onTriggered: {
            if(root.moving || root.flicking) {
                Qt.callLater(start)
                return
            }

            ensureItemVisible(item, scaling, leaveMargin)
            item = null
            scaling = 1
            leaveMargin = 0
            delay = 250
        }
    }

    Timer {
        id: returnToBoundsTimer
        objectName: "ScrollArea.returnToBoundsTimer"

        repeat: false
        running: false
        interval: 500

        onTriggered: parent.returnToBounds()
    }

    TrackerPack {
        delay: 250

        TrackProperty {
            target: root
            property: "contentX"
            onTracked: root.changing = true
        }

        TrackProperty {
            target: root
            property: "contentY"
            onTracked: root.changing = true
        }

        TrackProperty {
            target: root
            property: "moving"
            onTracked: root.changing = true
        }

        TrackProperty {
            target: root
            property: "flicking"
            onTracked: root.changing = true
        }

        onTracked: root.changing = false
    }

    PinchHandler {
        id: pinchHandler

        target: null

        minimumScale: Math.min(parent.scale, 0.25)
        maximumScale: Math.max(4, parent.scale)
        minimumRotation: 0
        maximumRotation: 0
        minimumPointCount: 2

        onScaleChanged: {
            zoomScaleBehavior.allow = false
            zoomScale = activeScale
            zoomScaleChangedInteractively()
            zoomScaleBehavior.allow = true
        }

        onTargetChanged: target = null
    }

    onZoomScaleChanged: {
        var cursorPos = Scrite.app.cursorPosition()
        var fCursorPos = Scrite.app.mapGlobalPositionToItem(root, cursorPos)
        var fContainsCursor = fCursorPos.x >= 0 && fCursorPos.y >= 0 && fCursorPos.x <= width && fCursorPos.y <= height
        var visibleArea = Qt.rect(contentX, contentY, width, height)
        var mousePoint = fContainsCursor ?
                    Scrite.app.mapGlobalPositionToItem(contentItem, Scrite.app.cursorPosition()) :
                    Qt.point(contentX+width/2, contentY+height/2)
        var newWidth = initialContentWidth * zoomScale
        var newHeight = initialContentHeight * zoomScale
        resizeContent(newWidth, newHeight, mousePoint)
        returnToBoundsTimer.start()
    }

    QtObject {
        id: _private

        property var ensureVisibleParams
    }
}
