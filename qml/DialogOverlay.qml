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

import QtQuick 2.5
import QtGraphicalEffects 1.0

Rectangle {
    id: dialogOverlay
    anchors.fill: parent
    color: Qt.rgba(1,1,1,t*0.4)
    property alias sourceComponent: contentsLoader.sourceComponent
    property alias active: contentsLoader.active
    property alias dialogItem: contentsLoader.item
    property Item popupSource
    property rect popupSourceArea: defaultPopupSourceArea
    property rect defaultPopupSourceArea: Qt.rect( (parent.x+(parent.width-10)/2), (parent.y+(parent.height-10)/2), 10, 10 )
    property bool animationComplete: false
    property alias closeable: closeButton.visible
    property alias backgroundColor: contentsAreaBackground.color
    onBackgroundColorChanged: blur.color = Qt.tint(backgroundColor, Qt.rgba(1,1,1,0.75))
    signal closeRequest()

    function close() {
        if(!active)
            return

        if(contentsLoader.width === 0 || contentsLoader.height === 0)
            closeRequest()
        else {
            contentsLoader.grabToImage( function(result) {
                    popupSourceImage.source = result.url
                    closeRequest()
                },
                Qt.size( contentsLoader.width, contentsLoader.height )
            )
        }
    }

    onActiveChanged: {
        if(active) {
            if(popupSource) {
                var mappedRect = mapFromItem(popupSource, 0, 0, popupSource.width, popupSource.height)
                popupSourceArea = Qt.rect(mappedRect.x, mappedRect.y, mappedRect.width, mappedRect.height)
            } else
                popupSourceArea = defaultPopupSourceArea
        }
    }

    property real t: active ? 1 : 0
    visible: t > 0
    onVisibleChanged: blur.visible = visible

    Behavior on t {
        SequentialAnimation {
            ScriptAction {
                script: animationComplete = false
            }
            NumberAnimation {
                duration: 500
                easing.type: Easing.InOutBack
            }
            ScriptAction {
                script: animationComplete = true
            }
        }
    }

    onTChanged: blur.radius = blur.maxRadius * t

    MouseArea {
        anchors.fill: parent
        onWheel: wheel.accepted = true
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

    BorderImage {
        source: "../icons/content/shadow.png"
        anchors.fill: contentsAreaBackground
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
        border { left: 21; top: 21; right: 21; bottom: 21 }
        smooth: true
        visible: true
        opacity: 0.75
    }

    Rectangle {
        id: contentsAreaBackground
        anchors.fill: contentsArea
        anchors.margins: -4
        radius: 6
        Behavior on color { ColorAnimation { duration: 500 } }
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
            onActiveChanged: {
                if(!active)
                    backgroundColor = Qt.rgba(1,1,1,0)
            }
        }
    }

    Image {
        id: closeButton
        anchors.top: contentsArea.top
        anchors.right: contentsArea.right
        anchors.margins: -width/2
        source: "../icons/action/dialog_close_button.png"
        width: 32; height: 32
        smooth: true
        fillMode: Image.PreserveAspectFit
        MouseArea {
            anchors.fill: parent
            onClicked: close()
        }
    }
}

