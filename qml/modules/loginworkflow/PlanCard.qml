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
    id: planCard

    property url icon: "qrc:/images/appicon.png"
    property string name
    property string duration
    property string durationNote
    property string price
    property string priceNote
    property string actionLink

    signal actionLinkClicked()

    spacing: 10

    RowLayout {
        Layout.minimumWidth: parent.width * 0.25
        Layout.maximumWidth: parent.width * 0.25
        Layout.preferredWidth: parent.width * 0.25

        spacing: 0

        Image {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 36
            Layout.preferredWidth: 36

            source: planCard.icon
        }

        VclLabel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            padding: 5
            text: planCard.name
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }
    }

    ColumnLayout {
        Layout.minimumWidth: parent.width * 0.3
        Layout.maximumWidth: parent.width * 0.3
        Layout.preferredWidth: parent.width * 0.3

        VclLabel {
            Layout.fillWidth: true

            text: planCard.duration
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
            maximumLineCount: 2
        }

        VclLabel {
            Layout.fillWidth: true

            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            text: planCard.durationNote
            visible: text !== ""
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }
    }

    ColumnLayout {
        Layout.minimumWidth: parent.width * 0.3
        Layout.maximumWidth: parent.width * 0.3
        Layout.preferredWidth: parent.width * 0.3

        VclLabel {
            Layout.fillWidth: true

            text: planCard.price
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
            maximumLineCount: 2
        }

        VclLabel {
            Layout.fillWidth: true

            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            text: planCard.priceNote
            visible: text !== ""
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }
    }

    Link {
        Layout.fillWidth: true

        opacity: enabled ? 1 : 0.5
        text: planCard.actionLink
        horizontalAlignment: Text.AlignRight
        font.bold: enabled
        defaultColor: enabled ? Runtime.colors.accent.c500.background : Runtime.colors.primary.c300.background
        onClicked: planCard.actionLinkClicked()
    }
}
