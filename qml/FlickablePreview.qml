/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12
import Scrite 1.0

Item {
    id: flickablePreview
    property Flickable flickable
    property Item content: flickable.contentItem
    property bool updatingThumbnail: false
    property bool interacting: viewportIndicatorMouseArea.drag.active

    property real maximumWidth: 300
    property real maximumHeight: 300

    signal viewportRectRequest(var rect)

    width: maximumWidth
    height: maximumHeight

    BorderImage {
        source: "../icons/content/shadow.png"
        anchors.fill: previewArea
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
        border { left: 21; top: 21; right: 21; bottom: 21 }
        opacity: 0.55 * previewArea.opacity
    }

    Rectangle {
        id: previewArea
        anchors.fill: thumbImage
        color: primaryColors.c50.background
        border { width: 1; color: primaryColors.borderColor }
        opacity: previewMouseArea.containsMouse || viewportIndicatorMouseArea.containsMouse ? 1.0 : 0.5

        MouseArea {
            id: previewMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.MidButton
            hoverEnabled: true
            cursorShape: viewportIndicatorMouseArea.drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        }
    }

    Image {
        id: thumbImage
        anchors.fill: parent
        clip: true
        smooth: true
        mipmap: true

        Rectangle {
            id: viewportIndicator
            color: primaryColors.highlight.background
            opacity: 0.5
            width: flickablePreview.flickable.visibleArea.widthRatio * parent.width
            height: flickablePreview.flickable.visibleArea.heightRatio * parent.height

            TrackObject {
                objectName: "CanvasPreviewTracker"
                enabled: !viewportIndicatorMouseArea.drag.active && flickablePreview.visible

                TrackProperty { target: flickablePreview.flickable.visibleArea; property: "xPosition" }
                TrackProperty { target: flickablePreview.flickable.visibleArea; property: "yPosition" }

                onTracked: {
                    viewportIndicator.x = flickablePreview.flickable.visibleArea.xPosition * thumbImage.width
                    viewportIndicator.y = flickablePreview.flickable.visibleArea.yPosition * thumbImage.height
                }
            }

            TrackObject {
                objectName: "PreviewCanvasTracker"
                enabled: viewportIndicatorMouseArea.drag.active && flickablePreview.visible

                TrackProperty { target: viewportIndicator; property: "x" }
                TrackProperty { target: viewportIndicator; property: "y" }

                onTracked: {
                    var rect = Qt.rect( (viewportIndicator.x/thumbImage.width)*(content.width-1),
                                        (viewportIndicator.y/thumbImage.height)*(content.height-1),
                                        (viewportIndicator.width/thumbImage.width)*(content.width-1),
                                        (viewportIndicator.height/thumbImage.height)*(content.height-1) );
                    var vrect = Qt.rect(0, 0, content.width, content.height)
                    var vrr = app.intersectedRectangle(vrect,rect)
                    flickablePreview.viewportRectRequest(vrr)
                }
            }

            MouseArea {
                id: viewportIndicatorMouseArea
                anchors.fill: parent
                hoverEnabled: drag.active
                cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                drag.target: parent
                drag.minimumX: 0
                drag.maximumX: thumbImage.width - parent.width
                drag.minimumY: 0
                drag.maximumY: thumbImage.height - parent.height
            }
        }
    }

    function updateThumbnail() {
        if(flickable === null || content === null || thumbImage.width <= 0 || thumbImage.height <= 0)
            return

        var thumbScale = maximumHeight / content.height
        if(content.width > content.height)
            thumbScale = maximumWidth / content.width

        var size = Qt.size(content.width*thumbScale, content.height*thumbScale)
        width = size.width
        height = size.height

        var dpr = app.devicePixelRatio

        updatingThumbnail = true
        content.grabToImage(function(result) {
            thumbImage.source = result.url;
            flickablePreview.updatingThumbnail = false
        }, Qt.size(width*dpr,height*dpr));
    }
}
