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

    property var allLocations: Scrite.document.structure.allLocations()
    property var selectedLocations: []

    spacing: 5

    VclLabel {
        Layout.fillWidth: true

        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
        maximumLineCount: 2

        text: fieldInfo.label
    }

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap
        font.italic: true
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize

        text: fieldInfo.note
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.rightMargin: 20
        Layout.preferredHeight: 250

        color: Runtime.colors.primary.c50.background
        border.width: 1
        border.color: Runtime.colors.primary.c50.text

        ListView {
            id: locationListView

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
            ScrollBar.vertical: VclScrollBar { }

            anchors.fill: parent
            anchors.margins: 1

            model: allLocations
            clip: true

            delegate: VclCheckBox {
                required property int index
                required property var modelData

                text: modelData
                width: locationListView.width-1
                checked: selectedLocations.indexOf(modelData) >= 0

                font.family: Scrite.document.formatting.defaultFont.family

                onToggled: {
                    var locs = selectedLocations
                    if(checked)
                        locs.push(modelData)
                    else
                        locs.splice(locs.indexOf(modelData), 1)
                    selectedLocations = locs
                    report.setConfigurationValue(fieldInfo.name, locs)
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        spacing: 20

        VclButton {
            text: "Select All"
            enabled: selectedLocations.length < allLocations.length
            onClicked: {
                selectedLocations = allLocations
                report.setConfigurationValue(fieldInfo.name, selectedLocations)
            }
        }

        VclButton {
            text: "Unselect All"
            enabled: selectedLocations.length > 0
            onClicked: {
                selectedLocations = []
                report.setConfigurationValue(fieldInfo.name, selectedLocations)
            }
        }
    }
}
