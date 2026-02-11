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

    spacing: 6

    VclLabel {
        Layout.fillWidth: true

        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2

        text: fieldInfo.label
    }

    VclLabel {
        Layout.fillWidth: true
        Layout.rightMargin: 10

        visible: text !== ""
        wrapMode: Text.WordWrap
        font.italic: true
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize

        text: fieldInfo.note
    }

    SpinBox {
        to: fieldInfo.max
        from: fieldInfo.min

        value: report ? report.getConfigurationValue(fieldInfo.name) : 0

        onValueModified: {
            if(report)
                report.setConfigurationValue(fieldInfo.name, value)
        }
    }
}
