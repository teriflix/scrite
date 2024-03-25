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

Item {
    id: root

    readonly property int e_CurrentDocumentTarget: 0
    readonly property int e_DefaultGlobalTarget: 1
    property int target: e_DefaultGlobalTarget

    ColumnLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: 15
        anchors.leftMargin: 0

        spacing: 10

        VclText {
            Layout.fillWidth: true

            font.bold: true
            wrapMode: Text.WordWrap

            text: target === e_CurrentDocumentTarget ? "Current Document Story Beats" : "Default Global Story Beats"
        }

        VclText {
            Layout.fillWidth: true

            wrapMode: Text.WordWrap

            text: "Customize categories & groups you use for tagging index cards on the structure canvas."
        }

        FlickableTextArea {
            id: storyBeatsEditor
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true

            font.family: "Courier Prime"
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            color: Runtime.colors.primary.c50.text

            text: target === e_CurrentDocumentTarget ? Scrite.document.structure.groupsData : Scrite.app.fileContents(Scrite.document.structure.defaultGroupsDataFile)
            background: Rectangle {
                color: Runtime.colors.primary.c50.background
                border.width: 1
                border.color: Runtime.colors.primary.borderColor
            }

            onTextChanged: cmdApplyButton.enabled = true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            VclButton {
                text: "Help"
                onClicked: Qt.openUrlExternally("https://www.youtube.com/watch?v=Ql_BjMVpjNc")
            }

            Item {
                Layout.fillWidth: true
            }

            VclButton {
                id: cmdApplyButton

                text: "Apply"
                enabled: false
                onClicked: {
                    if(target === e_CurrentDocumentTarget)
                        Scrite.document.structure.groupsData = storyBeatsEditor.text
                    else
                        Scrite.app.writeToFile(Scrite.document.structure.defaultGroupsDataFile, storyBeatsEditor.text)
                    enabled = false
                }
            }
        }
    }
}
