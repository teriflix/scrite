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

ColumnLayout {
    property var fieldInfo
    property AbstractReportGenerator report

    spacing: 5

    VclText {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap

        text: fieldInfo.label
    }

    VclText {
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
        }

        ListView {
            id: episodeListView
            model: Scrite.document.screenplay.episodeCount + 1
            clip: true
            property var episodeNumbers: report.getConfigurationValue(fieldInfo.name)
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

            delegate: Item {
                width: episodeListView.width-1
                height: index > 0 ? 40 : (Scrite.document.screenplay.episodeCount === 0 ? 40 : 0)

                VclText {
                    text: "No espisodes in this screenplay"
                    visible: Scrite.document.screenplay.episodeCount === 0
                    anchors.centerIn: parent
                }

                VclCheckBox {
                    text: "EPISODE " + index
                    anchors.verticalCenter: parent.verticalCenter
                    visible: index > 0
                    font.family: Scrite.document.formatting.defaultFont.family
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    checked: episodeListView.episodeNumbers.indexOf(index) >= 0
                    onToggled: episodeListView.select(index, checked)
                }
            }
        }
    }
}
