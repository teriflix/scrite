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
    property var fieldInfo
    property AbstractExporter exporter
    property TabSequenceManager tabSequence

    spacing: 5

    VclLabel {
        Layout.fillWidth: true

        wrapMode: Text.WordWrap
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
        font.capitalization: Font.Capitalize

        text: fieldInfo.name
    }

    VclTextField {
        Layout.fillWidth: true
        Layout.rightMargin: 30

        TabSequenceItem.manager: tabSequence

        label: ""
        placeholderText: fieldInfo.label
        onTextChanged: exporter.setConfigurationValue(fieldInfo.name, text)
    }
}
