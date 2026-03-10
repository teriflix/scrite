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
            id: _groupsView

            property var checkedTags: report.getConfigurationValue(fieldInfo.name)

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            clip: true

            model: GenericArrayModel {
                array: Scrite.document.structure.groupsModel
                objectMembers: ["category", "label", "name"]
            }

            section.property: "category"
            section.criteria: ViewSection.FullString
            section.delegate: Item {
                id: _sectionDelegate
                required property string section

                width: _groupsView.width
                height: 40

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 3
                    color: Runtime.colors.primary.windowColor
                    VclLabel {
                        text: _sectionDelegate.section
                        topPadding: 5
                        bottomPadding: 5
                        anchors.centerIn: parent
                        color: Runtime.colors.primary.button.text
                    }
                }
            }

            delegate: VclCheckBox {
                id: _tagGroupDelegate
                required property int index
                required property string name
                required property string label

                text: _tagGroupDelegate.label
                checked: _groupsView.checkedTags.indexOf(_tagGroupDelegate.name) >= 0

                onToggled: {
                    var tags = _groupsView.checkedTags
                    if(checked)
                        tags.push(_tagGroupDelegate.name)
                    else
                        tags.splice(tags.indexOf(_tagGroupDelegate.name), 1)
                    _groupsView.checkedTags = tags
                    report.setConfigurationValue(fieldInfo.name, tags)
                }
            }
        }
    }
}
