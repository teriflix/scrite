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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Item {
    id: root

    property alias to: _slider.to
    property alias from: _slider.from
    property alias value: _slider.value
    property alias pressed: _slider.pressed
    property alias stepSize: _slider.stepSize
    property alias zoomLevel: _slider.zoomLevel
    property alias zoomSliderVisible: _slider.visible

    signal sliderMoved()
    signal zoomInRequest()
    signal zoomOutRequest()

    implicitWidth: _layout.width
    implicitHeight: Runtime.idealFontMetrics.height + 8

    RowLayout {
        id: _layout

        anchors.verticalCenter: parent.verticalCenter

        IconButton {
            enabled: _slider.value > _slider.from
            source: "qrc:/icons/navigation/zoom_out.png"
            tooltipText: "Zoom Out"

            onClicked: {
                if(_slider.stepSize > 0)
                    _slider.value = _slider.value-_slider.stepSize
                else
                    zoomOutRequest()
            }
        }

        Slider {
            id: _slider

            property real zoomLevel: value

            to: 2
            from: 0.4
            value: 1
            stepSize: 0.1
            orientation: Qt.Horizontal

            topPadding: 0
            bottomPadding: 0

            onMoved: sliderMoved()
        }

        IconButton {
            source: "qrc:/icons/navigation/zoom_in.png"
            enabled: _slider.value < _slider.to
            tooltipText: "Zoom In"

            onClicked: {
                if(_slider.stepSize > 0)
                    _slider.value = _slider.value+_slider.stepSize
                else
                    zoomInRequest()
            }
        }

        VclLabel {
            Layout.preferredWidth: Runtime.minimumFontMetrics.advanceWidth("999%") + leftPadding + rightPadding

            text: Math.round(_slider.zoomLevel * 100) + "%"
            leftPadding: 5
            rightPadding: 5
            horizontalAlignment: Text.AlignRight

            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
        }

        component IconButton : Image {
            property alias containsMouse: _iconButtonMouseArea.containsMouse

            property string tooltipText

            signal clicked()

            Layout.preferredWidth: Runtime.idealFontMetrics.height
            Layout.preferredHeight: Runtime.idealFontMetrics.height

            scale: _iconButtonMouseArea.containsMouse ? (_iconButtonMouseArea.pressed ? 1 : 1.5) : 1
            mipmap: true

            Behavior on scale { NumberAnimation { duration: Runtime.stdAnimationDuration } }

            MouseArea {
                id: _iconButtonMouseArea

                ToolTip.text: parent.tooltipText
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                ToolTip.visible: containsMouse && !pressed

                anchors.fill: parent

                hoverEnabled: true

                onClicked: parent.clicked()
            }
        }
    }
}
