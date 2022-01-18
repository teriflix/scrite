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

import QtQuick 2.15
import io.scrite.components 1.0

Item {
    id: dockWidget
    property alias title: titleText.text
    property alias active: contentLoader.active
    property alias content: contentLoader.sourceComponent
    property real contentX
    property real contentY
    property real contentWidth
    property real contentHeight
    property bool contentHasFocus: contentLoader.FocusTracker.hasFocus
    property Item sourceItem

    signal moved()

    function show() {
        if(visible)
            return
        if(sourceItem && applicationSettings.enableAnimations)
            showAnimation.start()
        else {
            container.t = 1
            visible = true
        }
    }

    function hide() {
        if(!visible)
            return
        if(sourceItem && applicationSettings.enableAnimations)
            hideAnimation.start()
        else {
            container.t = 1
            visible = false
        }
    }

    function toggle() {
        if(visible)
            hide()
        else
            show()
    }

    TrackerPack {
        TrackProperty { target: dockWidget; property: "width" }
        TrackProperty { target: dockWidget; property: "height" }
        TrackProperty { target: dockWidget; property: "visible" }

        delay: 50

        onTracked: container.returnToBounds()
    }

    function close() { closeRequest() }
    signal closeRequest()

    BoxShadow {
        anchors.fill: container
    }

    Rectangle {
        id: container
        color: primaryColors.c100.background
        function evaluateSourceGeometry() {
            if(sourceItem) {
                var pt = dockWidget.mapFromItem(sourceItem, 0, 0)
                sourceGeometry = Qt.rect(pt.x, pt.y, sourceItem.width, sourceItem.height)
            } else
                sourceGeometry = Qt.rect(0, 0, 100, 100)
        }

        function returnToBounds() {
            var newX = dockWidget.contentX
            var newY = dockWidget.contentY
            if(dockWidget.contentX + dockWidget.contentWidth > dockWidget.width)
                newX = dockWidget.width - dockWidget.contentWidth
            if(dockWidget.contentY + dockWidget.contentHeight > dockWidget.height)
                newY = dockWidget.height - dockWidget.contentHeight
            dockWidget.contentX = newX
            dockWidget.contentY = newY
        }

        property rect sourceGeometry: Qt.rect(0, 0, 100, 100)
        property rect targetGeometry: Qt.rect(contentX, contentY, contentWidth, contentHeight)
        property real t: 1
        property real distance: Scrite.app.distanceBetweenPoints( Qt.point(sourceGeometry.x,sourceGeometry.y), Qt.point(targetGeometry.x,targetGeometry.y) )

        x: (sourceGeometry.x + (targetGeometry.x - sourceGeometry.x)*t)
        y: (sourceGeometry.y + (targetGeometry.y - sourceGeometry.y)*t)
        width: (sourceGeometry.width + (targetGeometry.width - sourceGeometry.width)*t)
        height: (sourceGeometry.height + (targetGeometry.height - sourceGeometry.height)*t)

        Rectangle {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            color: primaryColors.c300.background

            Text {
                id: titleText
                anchors.centerIn: parent
                font.pointSize: Scrite.app.idealFontPointSize
                text: primaryColors.c300.text
            }

            MouseArea {
                id: titleBarMouseArea
                anchors.fill: parent
                drag.target: container
                drag.axis: Drag.XAndYAxis
                drag.onActiveChanged: {
                    if(!drag.active) {
                        var newX = container.x
                        var newY = container.y
                        contentX = newX
                        contentY = newY
                        container.returnToBounds()
                        moved()
                    }
                }
            }

            ToolButton3 {
                height: parent.height - 6
                width: height
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 4
                iconSource: "../icons/navigation/close.png"
                onClicked: closeRequest()
            }
        }

        Loader {
            id: contentLoader
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            active: dockWidget.visible
            clip: true
            transformOrigin: Item.TopLeft
            scale: parent.t
            FocusTracker.window: Scrite.window
        }
    }

    property int animationDuration: Math.max(100, container.distance*0.15)

    SequentialAnimation {
        id: hideAnimation

        ScriptAction {
            script: container.evaluateSourceGeometry()
        }

        NumberAnimation {
            target: container
            property: "t"
            duration: animationDuration
            from: 1; to: 0
        }

        ScriptAction {
            script: dockWidget.visible = false
        }
    }

    SequentialAnimation {
        id: showAnimation

        ScriptAction {
            script: {
                container.evaluateSourceGeometry()
                dockWidget.visible = true
            }
        }

        NumberAnimation {
            target: container
            property: "t"
            duration: animationDuration
            from: 0; to: 1
        }
    }
}
