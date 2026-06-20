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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../../globals"
import "../../controls"
import "../../helpers"

Item {
    id: root

    property url icon: exclusive ? "qrc:/images/exclprodicon.png" : "qrc:/images/prodicon.png"

    property bool   exclusive: false
    property bool   isBestValue: false
    property bool   actionLinkVisible: true
    property bool   actionLinkEnabled: true
    property bool   useFixedFontForPrice: true

    property string name
    property string duration
    property string durationNote
    property string price
    property string actualPrice: ""
    property string savingsLabel: ""
    property string priceNote
    property string actionLink

    signal actionLinkClicked()

    implicitHeight: _row.implicitHeight
    implicitWidth: _row.implicitWidth

    RowLayout {
        id: _row
        anchors.fill: parent
        spacing: 10

        // Column 1: icon + name + best value badge
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 25
            z: 1
            spacing: 5

            Image {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 32
                Layout.preferredWidth: 32

                source: Runtime.themedIcon(root.icon)
                mipmap: true
                smooth: true
                fillMode: Image.PreserveAspectFit
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                LabelWithTooltip {
                    Layout.fillWidth: true
                    text: root.name
                    padding: root.isBestValue ? 0 : 5
                }

                Rectangle {
                    visible: root.isBestValue
                    color: Runtime.colors.tx(Runtime.colors.accent.c600.background)
                    radius: 4
                    implicitWidth: _bvText.implicitWidth + 12
                    implicitHeight: _bvText.implicitHeight + 4

                    VclLabel {
                        id: _bvText
                        anchors.centerIn: parent
                        text: "Best Value"
                        color: Runtime.colors.accent.c600.text
                        font.bold: true
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }
                }
            }
        }

        // Column 2: duration + note
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 25
            z: 1
            spacing: 2

            LabelWithTooltip {
                Layout.fillWidth: true
                text: root.duration
            }

            LabelWithTooltip {
                Layout.fillWidth: true
                text: root.durationNote
                visible: text !== ""
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            }
        }

        // Column 3: actual price (struck through) + discount % + subtitle
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 20
            z: 1
            spacing: 2

            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                visible: root.actualPrice !== "" || root.savingsLabel !== ""

                VclLabel {
                    text: root.actualPrice
                    font.family: root.useFixedFontForPrice ? Runtime.shortcutFontMetrics.font.family : Runtime.idealFontMetrics.font.family
                    font.strikeout: true
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    opacity: 0.5
                    visible: root.actualPrice !== ""
                }

                VclLabel {
                    text: root.savingsLabel
                    font.bold: true
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    visible: root.savingsLabel !== ""
                }

                Item { Layout.fillWidth: true }
            }

            LabelWithTooltip {
                Layout.fillWidth: true
                text: root.priceNote
                visible: text !== ""
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            }
        }

        // Column 4: price
        VclLabel {
            Layout.fillWidth: true
            Layout.preferredWidth: 15
            text: root.price
            font.family: root.useFixedFontForPrice ? Runtime.shortcutFontMetrics.font.family : Runtime.idealFontMetrics.font.family
            font.bold: root.savingsLabel !== ""
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + (root.useFixedFontForPrice ? 2 : -2)
            horizontalAlignment: Text.AlignRight
            rightPadding: 12
        }

        // Column 5: action link
        Link {
            Layout.fillWidth: true
            Layout.preferredWidth: 15

            text: root.actionLink
            enabled: root.actionLinkEnabled
            opacity: root.actionLinkVisible ? 1 : 0
            font.bold: true
            horizontalAlignment: Text.AlignRight

            onClicked: root.actionLinkClicked()
        }
    }

    component LabelWithTooltip : VclLabel {
        id: _labelWithTooltip

        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2

        MouseArea {
            id: _labelWithTooltipMouseArea

            anchors.fill: parent

            hoverEnabled: true

            ToolTipPopup {
                visible: _labelWithTooltipMouseArea.containsMouse
                text: _labelWithTooltip.text
            }
        }
    }
}
