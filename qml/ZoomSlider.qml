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
import Scrite 1.0

Row {
    id: zoomSliderBox

    property alias from: zoomSlider.from
    property alias to: zoomSlider.to
    property alias value: zoomSlider.value
    property alias stepSize: zoomSlider.stepSize
    property alias zoomLevel: zoomSlider.zoomLevel

    Rectangle {
        id: decrZoom
        width: 20; height: 20
        anchors.verticalCenter: parent.verticalCenter
        color: decrZoomMouseArea.containsMouse ? primaryColors.button.background : primaryColors.c10.background
        enabled: zoomSlider.value > zoomSlider.from

        Text {
            text: "-"
            anchors.centerIn: parent
            color: primaryColors.button.text
        }

        MouseArea {
            id: decrZoomMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: zoomSlider.value = zoomSlider.value-zoomSlider.stepSize
        }
    }

    Slider {
        id: zoomSlider
        orientation: Qt.Horizontal
        from: 0.4; to: 2; value: 1
        stepSize: 0.1
        property real zoomLevel: value
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        id: incrZoom
        width: 20; height: 20
        anchors.verticalCenter: parent.verticalCenter
        color: incrZoomMouseArea.containsMouse ? primaryColors.button.background : primaryColors.c10.background
        enabled: zoomSlider.value < zoomSlider.to

        Text {
            text: "+"
            anchors.centerIn: parent
            color: primaryColors.button.text
        }

        MouseArea {
            id: incrZoomMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: zoomSlider.value = zoomSlider.value+zoomSlider.stepSize
        }
    }

    Item { width: parent.height/3; height: parent.height }

    Text {
        anchors.verticalCenter: parent.verticalCenter
        text: Math.round(zoomSlider.zoomLevel * 100) + "%"
    }

    Item { width: parent.height/3; height: parent.height }
}
