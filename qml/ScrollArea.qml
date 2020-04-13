/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.12

Flickable {
    id: flickable
    readonly property rect visibleRect: Qt.rect(contentX, contentY, width, height)
    property real initialContentWidth: 100
    property real initialContentHeight: 100
    property alias suggestedScale: pinchHandler.activeScale
    property alias handlePinchZoom: pinchHandler.enabled
    boundsBehavior: Flickable.StopAtBounds
    clip: true

    Behavior on contentX { NumberAnimation { duration: 250 } }
    Behavior on contentY { NumberAnimation { duration: 250 } }

    ScrollBar.horizontal: ScrollBar {
        policy: ScrollBar.AlwaysOn
        minimumSize: 0.1
    }
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AlwaysOn
        minimumSize: 0.1
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

    PinchHandler {
        id: pinchHandler
        target: null
        onTargetChanged: target = null

        minimumScale: 0.25
        maximumScale: 4
        minimumRotation: 0
        maximumRotation: 0
        enabled: flickable !== null
        minimumPointCount: 2

        onScaleChanged: {
            if(flickable === null)
                return

            var visibleArea = Qt.rect(flickable.contentX, flickable.contentY, flickable.width, flickable.height)
            var mousePoint = app.mapGlobalPositionToItem(flickable.contentItem, app.cursorPosition())
            var newWidth = flickable.initialContentWidth * activeScale
            var newHeight = flickable.initialContentHeight * activeScale
            flickable.resizeContent(newWidth, newHeight, mousePoint)
            flickable.returnToBounds()
        }
    }
}
