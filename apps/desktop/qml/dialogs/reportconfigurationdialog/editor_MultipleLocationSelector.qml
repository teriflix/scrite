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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../controls"
import "../../helpers"

ColumnLayout {
    id: root

    property scriteObjectConfigField fieldInfo
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

        text: root.fieldInfo.label
    }

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap
        font.italic: true
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize

        text: root.fieldInfo.note
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.rightMargin: 20
        Layout.preferredHeight: 250

        color: Runtime.colors.primary.c50.background
        border.width: 1
        border.color: Runtime.colors.primary.c50.text

        ListView {
            id: _locationListView

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
            ScrollBar.vertical: VclScrollBar { }

            anchors.fill: parent
            anchors.margins: 1

            model: root.allLocations
            clip: true

            delegate: VclCheckBox {
                required property int index
                required property var modelData

                text: modelData
                width: _locationListView.width-1
                checked: root.selectedLocations.indexOf(modelData) >= 0

                font.family: Scrite.document.formatting.defaultFont.family

                onToggled: {
                    let locs = root.selectedLocations
                    if(checked)
                        locs.push(modelData)
                    else
                        locs.splice(locs.indexOf(modelData), 1)
                    root.selectedLocations = locs
                    root.report.setConfigurationValue(root.fieldInfo.name, locs)
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        spacing: 20

        VclButton {
            text: "Select All"
            enabled: root.selectedLocations.length < root.allLocations.length
            onClicked: {
                root.selectedLocations = root.allLocations
                root.report.setConfigurationValue(root.fieldInfo.name, root.selectedLocations)
            }
        }

        VclButton {
            text: "Unselect All"
            enabled: root.selectedLocations.length > 0
            onClicked: {
                root.selectedLocations = []
                root.report.setConfigurationValue(root.fieldInfo.name, root.selectedLocations)
            }
        }
    }
}
