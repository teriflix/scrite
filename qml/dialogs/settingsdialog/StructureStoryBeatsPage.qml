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
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    id: root

    readonly property int e_CurrentDocumentTarget: 0
    readonly property int e_DefaultGlobalTarget: 1
    property int target: e_DefaultGlobalTarget

    ColumnLayout {
        id: _layout

        anchors.fill: parent
        anchors.margins: 15
        anchors.leftMargin: 0

        spacing: 20

        VclLabel {
            Layout.fillWidth: true

            font.bold: true
            wrapMode: Text.WordWrap

            text: target === e_CurrentDocumentTarget ? "Story beats used in the currently open document" : "Default story beats for use in all new documents created in the future"
        }

        VclLabel {
            Layout.fillWidth: true

            wrapMode: Text.WordWrap

            text: "Customize categories & groups you use for tagging index cards on the structure canvas."
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c50.background

            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            FlickableTextArea {
                id: _storyBeatsEditor

                anchors.fill: parent
                anchors.margins: 1

                clip: true
                color: Runtime.colors.primary.c50.text
                text: target === e_CurrentDocumentTarget ? Scrite.document.structure.groupsData : File.read(Scrite.document.structure.defaultGroupsDataFile)

                font.family: "Courier Prime"
                font.pointSize: Runtime.idealFontMetrics.font.pointSize

                onTextEdited: _applyButton.enabled = true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            VclButton {
                text: "Help"
                onClicked: Qt.openUrlExternally("https://www.scrite.io/story-beats/")
            }

            Item {
                Layout.fillWidth: true
            }

            VclButton {
                id: _applyButton

                text: "Apply"
                enabled: false
                onClicked: {
                    if(target === e_CurrentDocumentTarget)
                        Scrite.document.structure.groupsData = _storyBeatsEditor.text
                    else {
                        File.write(Scrite.document.structure.defaultGroupsDataFile, _storyBeatsEditor.text)

                        MessageBox.question("Story Beats",
                                            "Do you want to use these story beats in the current document as well?",
                                            ["Yes", "No"],
                                            (answer) => {
                                                if(answer === "Yes") {
                                                    Scrite.document.structure.groupsData = _storyBeatsEditor.text
                                                }
                                            })
                    }
                    enabled = false
                }
            }
        }
    }
}
