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

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

ColumnLayout {
    property var fieldInfo
    property AbstractReportGenerator report

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
        Layout.preferredHeight: 350

        background: Rectangle {
            color: Runtime.colors.primary.c50.background
            border.width: 1
            border.color: Runtime.colors.primary.c50.text
        }

        ListView {
            id: groupsView
            clip: true
            model: GenericArrayModel {
                array: Scrite.document.structure.groupsModel
                objectMembers: ["category", "label", "name"]
            }
            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
            section.property: "category"
            section.criteria: ViewSection.FullString
            section.delegate: Item {
                width: groupsView.width
                height: 40
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 3
                    color: Runtime.colors.primary.windowColor
                    VclLabel {
                        text: section
                        topPadding: 5
                        bottomPadding: 5
                        anchors.centerIn: parent
                        color: Runtime.colors.primary.button.text
                    }
                }
            }
            property var checkedTags: report.getConfigurationValue(fieldInfo.name)
            delegate: VclCheckBox {
                text: label
                checked: groupsView.checkedTags.indexOf(name) >= 0
                onToggled: {
                    var tags = groupsView.checkedTags
                    if(checked)
                        tags.push(name)
                    else
                        tags.splice(tags.indexOf(name), 1)
                    groupsView.checkedTags = tags
                    report.setConfigurationValue(fieldInfo.name, tags)
                }
            }
        }
    }
}
