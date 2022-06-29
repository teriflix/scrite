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

import io.scrite.components 1.0

Rectangle {
    id: dialogOverlay
    color: Scrite.app.translucent(accentColors.c50.background, t*0.4)
    property alias sourceComponent: contentsLoader.sourceComponent
    property alias active: contentsLoader.active
    property alias dialogItem: contentsLoader.item
    property Item popupSource
    property rect popupSourceArea: defaultPopupSourceArea
    property rect defaultPopupSourceArea: Qt.rect( (parent.x+(parent.width-10)/2), (parent.y+(parent.height-10)/2), 10, 10 )
    property bool animationComplete: false
    property alias closeable: closeButton.visible
    property bool closeOnEscape: closeable || closeUponClickOutsideContentArea
    property bool closeUponClickOutsideContentArea: false
    property bool animationsEnabled: true
    readonly property int animationDuration: 400
    signal closeRequest()
    signal aboutToClose()

    function close() {
        if(!active)
            return

        aboutToClose()

        if(contentsLoader.width === 0 || contentsLoader.height === 0)
            closeRequest()
        else
            contentsLoader.grabToImage( function(result) {
                    popupSourceImage.source = result.url
                    closeRequest()
                },
                Qt.size( contentsLoader.width, contentsLoader.height )
            )
    }

    onActiveChanged: {
        if(active) {
            if(popupSource) {
                var pos = dialogOverlay.mapFromItem(popupSource, 0, 0)
                popupSourceArea = Qt.rect(pos.x, pos.y, popupSource.width, popupSource.height)
            } else
                popupSourceArea = defaultPopupSourceArea
        }
    }

    property real t: active ? 1 : 0
    visible: t > 0
    onVisibleChanged: {
        if(visible)
            dialogUnderlay.show()
        else {
            dialogUnderlay.hide()
            popupSourceImage.source = ""
        }
    }

    Behavior on t {
        enabled: animationsEnabled
        NumberAnimation {
            id: tAnimation
            duration: animationDuration
            easing.type: Easing.OutBack
        }
    }

    EventFilter.target: Scrite.app
    EventFilter.events: [6] // KeyPress
    EventFilter.active: closeOnEscape && visible
    EventFilter.onFilter: (event) => {
        if(event.key === Qt.Key_Escape) {
            result.acceptEvent = true
            result.filter = true
            close()
        }
    }

    onTChanged: dialogUnderlay.radius = dialogUnderlay.maxRadius * t

    MouseArea {
        anchors.fill: parent
        onWheel: wheel.accepted = true
        onClicked: {
            if(parent.closeUponClickOutsideContentArea)
                close()
        }
    }

    QtObject {
        id: fromArea
        property real x: popupSourceArea.x
        property real y: popupSourceArea.y
        property real width: popupSourceArea.width
        property real height: popupSourceArea.height
    }

    QtObject {
        id: toArea
        property real x: (parent.width - contentsLoader.width)/2
        property real y: (parent.height - contentsLoader.height)/2
        property real width: contentsLoader.width
        property real height: contentsLoader.height
    }

    BoxShadow {
        anchors.fill: contentsAreaBackground
    }

    Rectangle {
        id: contentsAreaBackground
        anchors.fill: contentsArea
        color: contentsLoader.item && contentsLoader.item.dialogColor ? contentsLoader.item.dialogColor : "white"
    }

    Item {
        id: contentsArea
        x: (fromArea.x + (toArea.x - fromArea.x)*parent.t)
        y: (fromArea.y + (toArea.y - fromArea.y)*parent.t)
        width: (fromArea.width + (toArea.width - fromArea.width)*parent.t)
        height: (fromArea.height + (toArea.height - fromArea.height)*parent.t)

        Image {
            id: popupSourceImage
            anchors.fill: parent
            fillMode: Image.Stretch
            visible: contentsLoader.active === false
        }

        Loader {
            id: contentsLoader
            active: sourceComponent !== undefined
            transformOrigin: Item.TopLeft
            transform: Scale {
                xScale: contentsArea.width / contentsLoader.implicitWidth
                yScale: contentsArea.height / contentsLoader.implicitHeight
            }
            clip: true
            onItemChanged: {
                if(item)
                    item.focus = true
            }
        }
    }

    Image {
        id: closeButton
        anchors.top: contentsArea.top
        anchors.right: contentsArea.right
        anchors.margins: -width/2
        source: "../icons/action/dialog_close_button.png"
        smooth: true
        fillMode: Image.PreserveAspectFit
        opacity: tAnimation.running ? 0 : (closeButtonMouseArea.containsMouse ? 1 : 0.8)

        MouseArea {
            id: closeButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: close()
        }
    }
}

