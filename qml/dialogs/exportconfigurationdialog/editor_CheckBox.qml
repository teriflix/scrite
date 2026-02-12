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
    property AbstractExporter exporter
    property TabSequenceManager tabSequence

    VclCheckBox {
        id: checkBox

        Layout.fillWidth: true

        TabSequenceItem.manager: tabSequence

        font.pointSize: Runtime.idealFontMetrics.font.pointSize

        text: fieldInfo.label
        checkable: true
        checked: exporter ? exporter.getConfigurationValue(fieldInfo.name) : false

        onToggled: exporter ? exporter.setConfigurationValue(fieldInfo.name, checked) : false
    }

    VclLabel {
        Layout.fillWidth: true

        color: Runtime.colors.primary.c600.background
        visible: text !== ""
        wrapMode: Text.WordWrap
        leftPadding: 2*checkBox.leftPadding + checkBox.implicitIndicatorWidth
        font.pointSize: Runtime.minimumFontMetrics.font.pointSize

        text: fieldInfo.note
    }
}
