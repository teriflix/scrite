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
    property real suggestedScale: zoomScale
    property rect visibleContentRect: Qt.rect(contentX, contentY, width/zoomScale, height/zoomScale)
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

        const s = Math.min(width/area.width, height/area.height)
        const center = Qt.point(area.x + area.width/2, area.y + area.height/2)
        const w = width/s
        const h = height/s

        const area2 = Qt.rect(Math.max(center.x-w/2,0), Math.max(center.y-h/2,0), w, h)

        zoomScale = s
        ensureVisible(area2, s)
    }

    function ensureItemVisible(item, scaling, leaveMargin) {
        if(item === null)
            return

        const area = Qt.rect(item.x, item.y, item.width, item.height)
        ensureVisible(area,
                      scaling === undefined ? root.zoomScale : scaling,
                      leaveMargin === undefined ? 20 : leaveMargin)
    }

    function ensureVisible(area, scaling, leaveMargin) {
        if(scaling === undefined)
            scaling = 1
        if(leaveMargin === undefined)
            leaveMargin = 20 * scaling
        else
            leaveMargin *= scaling

        area = Qt.rect( area.x*scaling, area.y*scaling, area.width*scaling, area.height*scaling )

        // Check if the area can be contained within the viewport
        if(area.width > visibleContentRect.width || area.height > visibleContentRect.height) {
            // We are here if area cannot fit into the space of the flickable.
            // In this case, we just try and fit the center point of area into the
            // view.
            const w = width*0.2
            const h = height*0.2
            area = Qt.rect( (area.left+area.right)/2-w/2,
                            (area.top+area.bottom)/2-h/2,
                            w, h )
        }

        // Check if item is already visible
        if(area.left >= visibleContentRect.left && area.top >= visibleContentRect.top &&
           area.right <= visibleContentRect.right && area.bottom <= visibleContentRect.bottom)
            return; // already visible

        let cx = undefined
        let cy = undefined
        if(area.left >= visibleContentRect.right || area.right >= visibleContentRect.right)
            cx = (area.right + leaveMargin) - width
        else if(area.right <= visibleArea.left || area.left <= visibleContentRect.left)
            cx = area.left - leaveMargin

        if(area.top >= visibleContentRect.bottom || area.bottom >= visibleContentRect.bottom)
            cy = (area.bottom + leaveMargin) - height
        else if(area.bottom <= visibleContentRect.top || area.top <= visibleContentRect.top)
            cy = area.top - leaveMargin

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
        let cursorPos = Scrite.app.cursorPosition()
        let fCursorPos = Scrite.app.mapGlobalPositionToItem(root, cursorPos)
        let fContainsCursor = fCursorPos.x >= 0 && fCursorPos.y >= 0 && fCursorPos.x <= width && fCursorPos.y <= height
        let visibleArea = Qt.rect(contentX, contentY, width, height)
        let mousePoint = fContainsCursor ?
                Scrite.app.mapGlobalPositionToItem(contentItem, Scrite.app.cursorPosition()) :
                Qt.point(contentX+width/2, contentY+height/2)
        let newWidth = initialContentWidth * zoomScale
        let newHeight = initialContentHeight * zoomScale

        resizeContent(newWidth, newHeight, mousePoint)
        returnToBoundsTimer.start()
    }
}
