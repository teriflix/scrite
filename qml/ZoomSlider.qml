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

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Row {
    id: zoomSliderBox

    property alias to: zoomSlider.to
    property alias from: zoomSlider.from
    property alias value: zoomSlider.value
    property alias pressed: zoomSlider.pressed
    property alias stepSize: zoomSlider.stepSize
    property alias zoomLevel: zoomSlider.zoomLevel

    signal zoomOutRequest()
    signal zoomInRequest()
    signal sliderMoved()

    ToolButton3 {
        id: decrZoom
        suggestedWidth: parent.height
        suggestedHeight: parent.height
        iconSource: "../icons/navigation/zoom_out.png"
        autoRepeat: true
        ToolTip.text: "Zoom Out"
        anchors.verticalCenter: parent.verticalCenter
        enabled: zoomSlider.value > zoomSlider.from
        onClicked: {
            if(zoomSlider.stepSize > 0)
                zoomSlider.value = zoomSlider.value-zoomSlider.stepSize
            else
                zoomOutRequest()
        }
    }

    Slider {
        id: zoomSlider
        orientation: Qt.Horizontal
        from: 0.4; to: 2; value: 1
        stepSize: 0.1
        property real zoomLevel: value
        anchors.verticalCenter: parent.verticalCenter
        onMoved: sliderMoved()
    }

    ToolButton3 {
        id: incrZoom
        suggestedWidth: parent.height
        suggestedHeight: parent.height
        iconSource: "../icons/navigation/zoom_in.png"
        autoRepeat: true
        ToolTip.text: "Zoom In"
        anchors.verticalCenter: parent.verticalCenter
        enabled: zoomSlider.value < zoomSlider.to
        onClicked: {
            if(zoomSlider.stepSize > 0)
                zoomSlider.value = zoomSlider.value+zoomSlider.stepSize
            else
                zoomInRequest()
        }
    }

    Item { width: parent.height/3; height: parent.height }

    Text {
        id: percentText
        anchors.verticalCenter: parent.verticalCenter
        text: Math.round(zoomSlider.zoomLevel * 100) + "%"
        horizontalAlignment: Text.AlignRight
        width: fontMetrics.advanceWidth("999%")

        FontMetrics {
            id: fontMetrics
            font: percentText.font
        }
    }

    Item { width: parent.height/3; height: parent.height }
}
