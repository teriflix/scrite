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

import Scrite.App

import "../../globals"
import "../../controls"
import "../../helpers"

ColumnLayout {
    id: root
    
    property scriteObjectConfigField fieldInfo
    property AbstractReportGenerator report

    spacing: 5

    SctLabel {
        Layout.fillWidth: true

        text: root.fieldInfo.label + ": "
    }

    SctComboBox {
        Layout.fillWidth: true
        Layout.rightMargin: 30

        model: root.fieldInfo.choices
        textRole: "key"

        onCurrentIndexChanged: {
            if(root.report)
                root.report.setConfigurationValue(root.fieldInfo.name, root.fieldInfo.choices[currentIndex].value)
        }
    }
}
