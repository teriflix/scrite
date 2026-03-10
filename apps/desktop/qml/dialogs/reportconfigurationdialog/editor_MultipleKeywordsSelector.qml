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

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap

        text: "No keywords were used in this document."
        visible: _keywordsView.count === 0
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 350

        color: Qt.rgba(0,0,0,0)
        border.width: 1
        border.color: Runtime.colors.primary.borderColor

        Flickable {
            id: _root_2

            ScrollBar.vertical: VclScrollBar { }

            anchors.fill: parent
            anchors.margins: 1

            clip: true
            contentWidth: _keywordsLayout.width
            contentHeight: _keywordsLayout.height

            Flow {
                id: _keywordsLayout

                width: _root_2.width-20

                spacing: 20

                Repeater {
                    id: _keywordsView

                    property var keywords: report.getConfigurationValue(fieldInfo.name)

                    model: Scrite.document.structure.sceneTags

                    delegate: VclCheckBox {
                        required property int index
                        required property string modelData

                        width: Math.min(parent.width * 0.8, implicitWidth)

                        text: modelData
                        checked: _keywordsView.keywords.indexOf(modelData) >= 0

                        onToggled: {
                            let keywords = _keywordsView.keywords
                            if(checked)
                                keywords.push(modelData)
                            else
                                keywords.splice(keywords.indexOf(modelData), 1)

                            if(report.setConfigurationValue(fieldInfo.name, keywords))
                                _keywordsView.keywords = keywords
                        }
                    }
                }
            }
        }
    }
}
