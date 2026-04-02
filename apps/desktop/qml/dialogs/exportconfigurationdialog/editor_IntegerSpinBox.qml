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
    property AbstractExporter exporter
    property TabSequenceManager tabSequence

    spacing: 10

    VclLabel {
        Layout.fillWidth: true

        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
        maximumLineCount: 2

        text: root.fieldInfo.label
    }

    VclSpinBox {
        TabSequenceItem.manager: root.tabSequence

        from: root.fieldInfo.min
        to: root.fieldInfo.max

        value: root.exporter ? root.exporter.getConfigurationValue(root.fieldInfo.name) : 0
        
        onValueModified: {
            if(root.exporter)
                root.exporter.setConfigurationValue(root.fieldInfo.name, value)
        }
    }
}
