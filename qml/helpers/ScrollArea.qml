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
    readonly property alias suggestedScale: root.zoomScale
    property rect visibleContentRect: Qt.rect(contentX, contentY, width/zoomScale, height/zoomScale)
    property real initialContentWidth: 100
    property real initialContentHeight: 100

    property alias handlePinchZoom: pinchHandler.enabled
    property alias minimumScale: pinchHandler.minimumScale
    property alias maximumScale: pinchHandler.maximumScale

    signal zoomScaleChangedInteractively()

    function zoomIn() {
        const zf = 1+Runtime.scrollAreaSettings.zoomFactor
        zoomTo(zoomScale*zf)
    }

    function zoomOut() {
        const zf = 1-Runtime.scrollAreaSettings.zoomFactor
        zoomTo(zoomScale*zf);
    }

    function zoomOne() {
        zoomScale = 1
    }

    function zoomTo(val) {
        zoomScale = Runtime.bounded(minimumScale, val, maximumScale)
    }

    function zoomFit(area) {
        if (!area || area.width <= 0 || area.height <= 0)
            return;

        zoomScaleBehavior.allow = false;

        const newScale = Math.min(width / area.width, height / area.height);
        zoomTo(newScale)

        contentWidth = initialContentWidth * zoomScale;
        contentHeight = initialContentHeight * zoomScale;

        const newContentX = area.x * zoomScale - (width - area.width * zoomScale) / 2;
        const newContentY = area.y * zoomScale - (height - area.height * zoomScale) / 2;

        contentX = Math.max(0, Math.min(newContentX, contentWidth - width));
        contentY = Math.max(0, Math.min(newContentY, contentHeight - height));

        zoomScaleBehavior.allow = true;
    }

    function ensureItemVisible(item) {
        if (item === null)
            return;

        const leaveMargin = 20

        let currentScale = root.zoomScale;

        const requiredWidth = item.width * currentScale + 2 * leaveMargin;
        const requiredHeight = item.height * currentScale + 2 * leaveMargin;

        if (requiredWidth > width || requiredHeight > height) {
            const scaleX = width / (item.width + 2 * leaveMargin);
            const scaleY = height / (item.height + 2 * leaveMargin);
            const newScale = Math.min(scaleX, scaleY);

            if (newScale !== currentScale)
                zoomTo(Math.max(pinchHandler.minimumScale, newScale))
        }

        const itemScaledRect = Qt.rect(item.x * currentScale,
                                       item.y * currentScale,
                                       item.width * currentScale,
                                       item.height * currentScale);

        const viewRect = Qt.rect(contentX, contentY, width, height);

        let newContentX = contentX;
        let newContentY = contentY;

        if (itemScaledRect.x < viewRect.x + leaveMargin) {
            newContentX = itemScaledRect.x - leaveMargin;
        } else if (itemScaledRect.x + itemScaledRect.width > viewRect.x + viewRect.width - leaveMargin) {
            newContentX = itemScaledRect.x + itemScaledRect.width - width + leaveMargin;
        }

        if (itemScaledRect.y < viewRect.y + leaveMargin) {
            newContentY = itemScaledRect.y - leaveMargin;
        } else if (itemScaledRect.y + itemScaledRect.height > viewRect.y + viewRect.height - leaveMargin) {
            newContentY = itemScaledRect.y + itemScaledRect.height - height + leaveMargin;
        }

        contentX = Math.max(0, Math.min(newContentX, contentWidth - width));
        contentY = Math.max(0, Math.min(newContentY, contentHeight - height));
    }

    function ensureAreaVisible(area, scaling, leaveMargin) {
        if (!area || area.width <= 0 || area.height <= 0)
            return;

        if (scaling === undefined)
            scaling = root.zoomScale;

        if (leaveMargin === undefined)
            leaveMargin = 20;

        const targetArea = area;

        const viewRect = Qt.rect(contentX / scaling, contentY / scaling, width / scaling, height / scaling);

        if (targetArea.x >= viewRect.x + leaveMargin &&
            targetArea.y >= viewRect.y + leaveMargin &&
            targetArea.x + targetArea.width <= viewRect.x + viewRect.width - leaveMargin &&
            targetArea.y + targetArea.height <= viewRect.y + viewRect.height - leaveMargin) {
            return;
        }

        let newContentX = (targetArea.x + targetArea.width / 2) * scaling - width / 2;
        let newContentY = (targetArea.y + targetArea.height / 2) * scaling - height / 2;

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

        repeat: false
        running: false
        interval: Runtime.stdAnimationDuration

        onTriggered: parent.returnToBounds()
    }

    Timer {
        id: changingTimer

        property var dependencies: [root.contentX, root.contentY, root.moving, root.flicking]

        repeat: false
        running: false
        interval: Runtime.stdAnimationDuration

        onTriggered: root.changing = false

        onDependenciesChanged: {
            root.changing = true
            start()
        }
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
            zoomScaleBehavior.allow = false;

            const newScale = Math.max(minimumScale, Math.min(pinchHandler.activeScale, maximumScale));
            if (zoomScale !== newScale) {
                const pinchCenter = pinchHandler.centroid.position;
                resizeContent(initialContentWidth * newScale, initialContentHeight * newScale, pinchCenter);
                zoomScale = newScale;
                zoomScaleChangedInteractively();
            }

            zoomScaleBehavior.allow = true;
        }

        onTargetChanged: target = null
    }

    onZoomScaleChanged: {
        if (!zoomScaleBehavior.allow || pinchHandler.active)
            return;

        // This logic zooms towards the mouse cursor. It can interfere with programmatic
        // zoom/pan like zoomFit. It's kept for interactive use, but be aware of its effects.
        const cursorPos = MouseCursor.position()
        const fCursorPos = MouseCursor.itemPosition(root, cursorPos)
        const fContainsCursor = fCursorPos.x >= 0 && fCursorPos.y >= 0 && fCursorPos.x <= width && fCursorPos.y <= height
        const mousePoint = fContainsCursor ?
                MouseCursor.itemPosition(contentItem, MouseCursor.position()) :
                Qt.point(contentX+width/2, contentY+height/2)
        const newWidth = initialContentWidth * zoomScale
        const newHeight = initialContentHeight * zoomScale

        resizeContent(newWidth, newHeight, mousePoint)
        returnToBoundsTimer.start()
    }
}
