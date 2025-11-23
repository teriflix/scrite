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


import "qrc:/qml/globals"
import "qrc:/qml/controls"

Flickable {
    id: root

    property bool changing: false
    property bool zoomOnScroll: Platform.isWindowsDesktop || Platform.isLinuxDesktop
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
        if (!area || area.width <= 0 || area.height <= 0)
            return;

        // Disable the interactive zoom logic in onZoomScaleChanged
        zoomScaleBehavior.allow = false;

        // Calculate the best scale to fit the area, respecting min/max scale.
        const newScale = Math.min(width / area.width, height / area.height);
        zoomScale = Math.max(pinchHandler.minimumScale, Math.min(newScale, pinchHandler.maximumScale));

        // Update content size based on new scale
        contentWidth = initialContentWidth * zoomScale;
        contentHeight = initialContentHeight * zoomScale;

        // Calculate contentX/Y to center the area in the viewport at the new scale.
        const newContentX = area.x * zoomScale - (width - area.width * zoomScale) / 2;
        const newContentY = area.y * zoomScale - (height - area.height * zoomScale) / 2;

        // Clamp values to be within the flickable's bounds.
        contentX = Math.max(0, Math.min(newContentX, contentWidth - width));
        contentY = Math.max(0, Math.min(newContentY, contentHeight - height));

        // Re-enable the interactive zoom logic
        zoomScaleBehavior.allow = true;
    }

    function ensureItemVisible(item) {
        if (item === null)
            return;

        const leaveMargin = 20

        let currentScale = root.zoomScale;

        // Required viewport size in pixels to contain the item
        const requiredWidth = item.width * currentScale + 2 * leaveMargin;
        const requiredHeight = item.height * currentScale + 2 * leaveMargin;

        // 1. SCALE: Check if the item is larger than the viewport and scale down if needed.
        if (requiredWidth > width || requiredHeight > height) {
            const scaleX = width / (item.width + 2 * leaveMargin);
            const scaleY = height / (item.height + 2 * leaveMargin);
            const newScale = Math.min(scaleX, scaleY);

            // Only zoom out, don't zoom in. Respect minimum scale.
            if (newScale < currentScale) {
                currentScale = Math.max(pinchHandler.minimumScale, newScale);
                zoomScale = currentScale; // This will trigger onZoomScaleChanged
            }
        }

        // 2. PAN: Calculate the minimum pan required to make the item visible.

        // The item's bounding box in the scaled content coordinate system.
        const itemScaledRect = Qt.rect(item.x * currentScale,
                                       item.y * currentScale,
                                       item.width * currentScale,
                                       item.height * currentScale);

        // The viewport's bounding box in the scaled content coordinate system.
        const viewRect = Qt.rect(contentX, contentY, width, height);

        let newContentX = contentX;
        let newContentY = contentY;

        // Horizontal check: Pan only if the item is outside the view.
        if (itemScaledRect.x < viewRect.x + leaveMargin) {
            // Item's left edge is off-screen to the left. Pan right.
            newContentX = itemScaledRect.x - leaveMargin;
        } else if (itemScaledRect.x + itemScaledRect.width > viewRect.x + viewRect.width - leaveMargin) {
            // Item's right edge is off-screen to the right. Pan left.
            newContentX = itemScaledRect.x + itemScaledRect.width - width + leaveMargin;
        }

        // Vertical check: Pan only if the item is outside the view.
        if (itemScaledRect.y < viewRect.y + leaveMargin) {
            // Item's top edge is off-screen to the top. Pan down.
            newContentY = itemScaledRect.y - leaveMargin;
        } else if (itemScaledRect.y + itemScaledRect.height > viewRect.y + viewRect.height - leaveMargin) {
            // Item's bottom edge is off-screen to the bottom. Pan up.
            newContentY = itemScaledRect.y + itemScaledRect.height - height + leaveMargin;
        }

        // Apply the calculated pan, clamping to the valid bounds.
        contentX = Math.max(0, Math.min(newContentX, contentWidth - width));
        contentY = Math.max(0, Math.min(newContentY, contentHeight - height));
    }

    function ensureAreaVisible(area, scaling, leaveMargin) {
        if (!area || area.width <= 0 || area.height <= 0)
            return;

        if (scaling === undefined)
            scaling = root.zoomScale; // Use current zoomScale if not provided

        if (leaveMargin === undefined)
            leaveMargin = 20; // Margin in unscaled pixels

        // The area to make visible, in unscaled content coordinates.
        const targetArea = area;

        // The current viewport, in unscaled content coordinates.
        const viewRect = Qt.rect(contentX / scaling, contentY / scaling, width / scaling, height / scaling);

        // Check if the target area is already fully visible within the viewport (with margin).
        if (targetArea.x >= viewRect.x + leaveMargin &&
            targetArea.y >= viewRect.y + leaveMargin &&
            targetArea.x + targetArea.width <= viewRect.x + viewRect.width - leaveMargin &&
            targetArea.y + targetArea.height <= viewRect.y + viewRect.height - leaveMargin) {
            return; // Already visible
        }

        // Calculate the new contentX and contentY to center the target area.
        let newContentX = (targetArea.x + targetArea.width / 2) * scaling - width / 2;
        let newContentY = (targetArea.y + targetArea.height / 2) * scaling - height / 2;

        // Clamp the new positions to the valid bounds of the Flickable.
        contentX = Math.max(0, Math.min(newContentX, contentWidth - width));
        contentY = Math.max(0, Math.min(newContentY, contentHeight - height));
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
        if (!zoomScaleBehavior.allow) {
            // If allow is false, it means a programmatic change is happening.
            // Just update content size and exit.
            contentWidth = initialContentWidth * zoomScale;
            contentHeight = initialContentHeight * zoomScale;
            return;
        }

        // This logic zooms towards the mouse cursor. It can interfere with programmatic
        // zoom/pan like zoomFit. It's kept for interactive use, but be aware of its effects.
        let cursorPos = MouseCursor.position()
        let fCursorPos = MouseCursor.itemPosition(root, cursorPos)
        let fContainsCursor = fCursorPos.x >= 0 && fCursorPos.y >= 0 && fCursorPos.x <= width && fCursorPos.y <= height
        let visibleArea = Qt.rect(contentX, contentY, width, height)
        let mousePoint = fContainsCursor ?
                MouseCursor.itemPosition(contentItem, MouseCursor.position()) :
                Qt.point(contentX+width/2, contentY+height/2)
        let newWidth = initialContentWidth * zoomScale
        let newHeight = initialContentHeight * zoomScale

        resizeContent(newWidth, newHeight, mousePoint)
        returnToBoundsTimer.start()
    }
}
