/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

ColumnLayout {
    property var fieldInfo
    property AbstractReportGenerator report

    function getReady() {
        episodeListView.model = Scrite.document.screenplay.episodeInfoList
    }

    spacing: 5

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap

        text: fieldInfo.label
    }

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap
        font.italic: true
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize

        text: fieldInfo.note
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: 320
        Layout.rightMargin: 20
        Layout.topMargin: parent.spacing

        background: Rectangle {
            color: Runtime.colors.primary.c50.background
            border.width: 1
            border.color: Runtime.colors.primary.c50.text

            VclText {
                anchors.centerIn: parent

                width: parent.width * 0.8
                text: "No espisodes in this screenplay"
                visible: Scrite.document.screenplay.episodeCount === 0
                horizontalAlignment: Text.AlignHCenter
            }
        }

        ListView {
            id: episodeListView

            property var episodeNumbers: fieldInfo ? report.getConfigurationValue(fieldInfo.name) : []

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            function select(episodeNumber, flag) {
                var numbers = report.getConfigurationValue(fieldInfo.name)
                var idx = numbers.indexOf(episodeNumber)
                if(flag) {
                    if(idx < 0)
                        numbers.push(episodeNumber)
                    else
                        return
                } else {
                    if(idx >= 0)
                        numbers.splice(idx, 1)
                    else
                        return
                }
                episodeNumbers = numbers
                report.setConfigurationValue(fieldInfo.name, numbers)
            }

            clip: true

            delegate: VclCheckBox {
                required property int index
                required property var modelData // This is a gadget of type ScreenplayBreakInfo

                property int episodeNumber: modelData.number
                property string episodeName: modelData.title + ": " + modelData.subtitle

                width: episodeListView.width-1

                text: episodeName
                checked: episodeListView.episodeNumbers.indexOf(episodeNumber) >= 0

                onToggled: episodeListView.select(episodeNumber, checked)
            }
        }
    }
}
