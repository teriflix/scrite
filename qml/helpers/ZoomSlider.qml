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

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Row {
    id: zoomSliderBox

    property alias to: zoomSlider.to
    property alias from: zoomSlider.from
    property alias value: zoomSlider.value
    property alias pressed: zoomSlider.pressed
    property alias stepSize: zoomSlider.stepSize
    property alias zoomLevel: zoomSlider.zoomLevel
    property alias zoomSliderVisible: zoomSlider.visible

    signal sliderMoved()
    signal zoomInRequest()
    signal zoomOutRequest()

    FlatToolButton {
        id: decrZoom

        ToolTip.text: "Zoom Out"

        anchors.verticalCenter: parent.verticalCenter

        suggestedWidth: parent.height
        suggestedHeight: parent.height

        enabled: zoomSlider.value > zoomSlider.from
        autoRepeat: true
        iconSource: "qrc:/icons/navigation/zoom_out.png"

        onClicked: {
            if(zoomSlider.stepSize > 0)
                zoomSlider.value = zoomSlider.value-zoomSlider.stepSize
            else
                zoomOutRequest()
        }
    }

    Slider {
        id: zoomSlider

        property real zoomLevel: value

        anchors.verticalCenter: parent.verticalCenter

        to: 2
        from: 0.4
        value: 1
        stepSize: 0.1
        orientation: Qt.Horizontal

        onMoved: sliderMoved()
    }

    FlatToolButton {
        id: incrZoom

        ToolTip.text: "Zoom In"

        anchors.verticalCenter: parent.verticalCenter

        suggestedWidth: parent.height
        suggestedHeight: parent.height

        enabled: zoomSlider.value < zoomSlider.to
        autoRepeat: true
        iconSource: "qrc:/icons/navigation/zoom_in.png"

        onClicked: {
            if(zoomSlider.stepSize > 0)
                zoomSlider.value = zoomSlider.value+zoomSlider.stepSize
            else
                zoomInRequest()
        }
    }

    Item { width: parent.height/3; height: parent.height }

    VclLabel {
        id: percentText

        anchors.verticalCenter: parent.verticalCenter

        width: Runtime.minimumFontMetrics.advanceWidth("999%")

        text: Math.round(zoomSlider.zoomLevel * 100) + "%"
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
        horizontalAlignment: Text.AlignRight
    }

    Item { width: parent.height/3; height: parent.height }
}
