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
    id: root

    property var fieldInfo
    property AbstractReportGenerator report

    spacing: 5
    enabled: report !== null

    VclLabel {
        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 20

        text: fieldInfo.label
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        font.bold: true
        maximumLineCount: 2
    }

    SplitView {
        id: splitView

        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 20
        Layout.preferredHeight: 100

        readonly property real minLeftColumnWidth: 0.2
        property real leftColumnWidth: report ? report.getConfigurationValue(fieldInfo.name) : minLeftColumnWidth

        orientation: Qt.Horizontal

        Rectangle {
            SplitView.minimumWidth: splitView.availableWidth * splitView.minLeftColumnWidth
            SplitView.maximumWidth: splitView.availableWidth * (1.0 - splitView.minLeftColumnWidth)
            SplitView.preferredWidth: splitView.availableWidth * splitView.leftColumnWidth

            border.width: 1
            border.color: Runtime.colors.primary.borderColor
            color: Runtime.colors.primary.c200.background

            VclText {
                anchors.centerIn: parent

                text: Math.round(splitView.leftColumnWidth*100) + "%"
            }

            onWidthChanged: splitView.leftColumnWidth = width/splitView.availableWidth
        }

        Rectangle {
            border.width: 1
            border.color: Runtime.colors.primary.borderColor
            color: Runtime.colors.primary.c200.background

            VclText {
                anchors.centerIn: parent

                text: Math.round((1-splitView.leftColumnWidth)*100) + "%"
            }
        }

        onLeftColumnWidthChanged: {
            if(resizing && report)
                report.setConfigurationValue(fieldInfo.name, leftColumnWidth)
        }
    }
}
