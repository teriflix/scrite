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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

RowLayout {
    id: root

    property var fieldInfo
    property AbstractReportGenerator report

    Layout.topMargin: 20
    Layout.fillWidth: true

    spacing: 10

    Repeater {
        model: [
            { title: "Video/Audio", icon: "twocolumn-av-layout.png", value: TwoColumnReport.VideoAudioLayout },
            { title: "All Left", icon: "twocolumn-left-layout.png", value: TwoColumnReport.EverythingLeft },
            { title: "All Right", icon: "twocolumn-right-layout.png", value: TwoColumnReport.EverythingRight },
        ]

        ColumnLayout {
            required property var modelData

            Layout.fillWidth: true

            spacing: 10

            Item {
                Layout.preferredWidth: _private.optionWidth
                Layout.preferredHeight: width * 1.41354292623942 // A4 page size ratio

                Image {
                    anchors.fill: parent
                    anchors.margins: 2
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                    source: "qrc:/icons/reports/" + modelData.icon
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor

                    onClicked: report.layout = modelData.value
                }
            }

            VclRadioButton {
                Layout.alignment: Qt.AlignHCenter

                text: modelData.title
                checked: report.layout === modelData.value
                onToggled: report.layout = modelData.value
            }
        }
    }

    QtObject {
        id: _private

        readonly property real optionWidth: root.width * 0.33 - 20
    }
}

