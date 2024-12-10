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
import "qrc:/qml/helpers"

RowLayout {
    id: root

    property url icon: "qrc:/images/prodicon.png"
    property string name
    property string duration
    property string durationNote
    property string price
    property string priceNote
    property string actionLink
    property bool actionLinkEnabled: true

    signal actionLinkClicked()

    spacing: 10

    RowLayout {
        Layout.minimumWidth: parent.width * 0.25
        Layout.maximumWidth: parent.width * 0.25
        Layout.preferredWidth: parent.width * 0.25
        z: 1

        spacing: 5

        Image {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 32
            Layout.preferredWidth: 32

            source: root.icon
            mipmap: true
            smooth: true
            fillMode: Image.PreserveAspectFit
        }

        LabelWithTooltip {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            text: root.name
            padding: 5
        }
    }

    ColumnLayout {
        Layout.minimumWidth: parent.width * 0.3
        Layout.maximumWidth: parent.width * 0.3
        Layout.preferredWidth: parent.width * 0.3
        z: 1

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

    ColumnLayout {
        Layout.minimumWidth: parent.width * 0.3
        Layout.maximumWidth: parent.width * 0.3
        Layout.preferredWidth: parent.width * 0.3
        z: 1

        LabelWithTooltip {
            Layout.fillWidth: true

            text: root.price
        }

        LabelWithTooltip {
            Layout.fillWidth: true

            text: root.priceNote
            visible: text !== ""
            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
        }
    }

    Link {
        Layout.fillWidth: true

        text: root.actionLink
        enabled: root.actionLinkEnabled
        font.bold: true
        horizontalAlignment: Text.AlignRight

        onClicked: root.actionLinkClicked()
    }

    component LabelWithTooltip : VclLabel {
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            ToolTip.text: parent.text
            ToolTip.visible: parent.truncated && containsMouse
        }
    }
}
