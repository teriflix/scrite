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
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../controls"
import "../../helpers"

ColumnLayout {
    id: root

    property scriteObjectConfigField fieldInfo
    property AbstractReportGenerator report

    spacing: 5

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
        font.capitalization: Font.Capitalize

        text: root.fieldInfo.name
    }

    VclLabel {
        Layout.fillWidth: true

        visible: text !== ""
        wrapMode: Text.WordWrap
        font.italic: true
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize

        text: root.fieldInfo.note
    }

    VclTextField {
        Layout.fillWidth: true

        label: ""
        placeholderText: root.fieldInfo.label

        text: root.report.getConfigurationValue(root.fieldInfo.name)

        onTextChanged: {
            if(root.report)
                root.report.setConfigurationValue(root.fieldInfo.name, text)
        }
    }
}
