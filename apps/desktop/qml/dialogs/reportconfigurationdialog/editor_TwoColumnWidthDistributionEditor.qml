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
        id: _splitView

        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 20
        Layout.preferredHeight: 100

        readonly property real minLeftColumnWidth: 0.2
        property real leftColumnWidth: report ? report.getConfigurationValue(fieldInfo.name) : minLeftColumnWidth

        orientation: Qt.Horizontal

        Rectangle {
            SplitView.minimumWidth: _splitView.availableWidth * _splitView.minLeftColumnWidth
            SplitView.maximumWidth: _splitView.availableWidth * (1.0 - _splitView.minLeftColumnWidth)
            SplitView.preferredWidth: _splitView.availableWidth * _splitView.leftColumnWidth

            border.width: 1
            border.color: Runtime.colors.primary.borderColor
            color: Runtime.colors.primary.c200.background

            VclTextField {
                id: _leftColumnWidthEditor

                anchors.centerIn: parent

                width: Runtime.idealFontMetrics.averageCharacterWidth * 4

                text: Math.round(_splitView.leftColumnWidth*100) + "%"
                validator: IntValidator {
                    bottom: 20
                    top: 80
                }

                onTextEdited: {
                    const v = parseInt(text)/100
                    if(v >= 0.2 && v <= 0.8) {
                        _splitView.leftColumnWidth = v

                    }
                }
            }

            onWidthChanged: _splitView.leftColumnWidth = width/_splitView.availableWidth
        }

        Rectangle {
            border.width: 1
            border.color: Runtime.colors.primary.borderColor
            color: Runtime.colors.primary.c200.background

            VclText {
                anchors.centerIn: parent

                text: Math.round((1-_splitView.leftColumnWidth)*100) + "%"
            }
        }

        onLeftColumnWidthChanged: {
            if( (resizing || _leftColumnWidthEditor.activeFocus) && report)
                report.setConfigurationValue(fieldInfo.name, leftColumnWidth)
        }
    }
}
