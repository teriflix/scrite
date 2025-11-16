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


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

ColumnLayout {
    property var fieldInfo
    property AbstractReportGenerator report

    property alias characterNames: characterNameListView.selectedCharacters
    onCharacterNamesChanged: {
        if(fieldInfo)
            report.setConfigurationValue(fieldInfo.name, characterNames)
    }

    onFieldInfoChanged: {
        characterNameListView.selectedCharacters = report.getConfigurationValue(fieldInfo.name)
        characterNameListView.visible = characterNameListView.selectedCharacters.length === 0
    }

    spacing: 10

    VclLabel {
        Layout.fillWidth: true

        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2

        text: fieldInfo.label
    }

    Loader {
        id: fieldTitleText

        Layout.fillWidth: true
        Layout.rightMargin: 30

        sourceComponent: Flow {
            spacing: 5
            flow: Flow.LeftToRight

            VclLabel {
                id: charactersPrefix
                text: characterNames.length === 0 ? "No Characters Selected" : "»"
                topPadding: 0
                bottomPadding: 5
            }

            Repeater {
                model: characterNames

                TagText {
                    required property string modelData

                    property var colors: Runtime.colors.accent.c600

                    border.width: 1
                    border.color: colors.text
                    color: colors.background
                    textColor: colors.text
                    text: modelData
                    leftPadding: 10
                    rightPadding: 10
                    topPadding: 2
                    bottomPadding: 2
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    closable: true
                    onCloseRequest: {
                        var list = characterNameListView.selectedCharacters
                        list.splice( list.indexOf(text), 1 )
                        characterNameListView.selectedCharacters = list
                    }
                }
            }

            Image {
                source: "qrc:/icons/content/add_box.png"
                width: charactersPrefix.height
                height: charactersPrefix.height
                opacity: 0.5
                visible: !characterNameListView.visible

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                    onClicked: characterNameListView.visible = true

                    ToolTipPopup {
                        container: parent
                        text: "Add another character."
                        visible: parent.containsMouse
                    }
                }
            }

            Image {
                source: "qrc:/icons/content/clear_all.png"
                width: charactersPrefix.height
                height: charactersPrefix.height
                opacity: 0.5
                visible: characterNameListView.selectedCharacters.length > 0

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                    onClicked: {
                        characterNameListView.selectedCharacters = []
                        characterNameListView.visible = true
                    }

                    ToolTipPopup {
                        container: parent
                        text: "Remove all characters and start fresh."
                        visible: parent.containsMouse
                    }
                }
            }
        }
    }

    CharactersView {
        id: characterNameListView

        Layout.fillWidth: true
        Layout.leftMargin: 5
        Layout.rightMargin: 30
        Layout.preferredHeight: 280
    }
}

