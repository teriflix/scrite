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

pragma ComponentBehavior: Bound

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

    property alias characterNames: _characterNameListView.selectedCharacters
    onCharacterNamesChanged: {
        if(fieldInfo)
            report.setConfigurationValue(fieldInfo.name, characterNames)
    }

    onFieldInfoChanged: {
        _characterNameListView.selectedCharacters = report.getConfigurationValue(fieldInfo.name)
        _characterNameListView.visible = _characterNameListView.selectedCharacters.length === 0
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
        id: _fieldTitleText

        Layout.fillWidth: true
        Layout.rightMargin: 30

        sourceComponent: Flow {
            spacing: 5
            flow: Flow.LeftToRight

            VclLabel {
                id: _charactersPrefix
                text: characterNames.length === 0 ? "No Characters Selected" : "»"
                topPadding: 0
                bottomPadding: 5
            }

            Repeater {
                model: characterNames

                delegate: TagText {
                    id: _characterTag
                    required property int index
                    required property string modelData

                    property var colors: Runtime.colors.accent.c600

                    border.width: 1
                    border.color: colors.text
                    color: colors.background
                    textColor: colors.text
                    text: _characterTag.modelData
                    leftPadding: 10
                    rightPadding: 10
                    topPadding: 2
                    bottomPadding: 2
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    closable: true
                    onCloseRequest: {
                        var list = _characterNameListView.selectedCharacters
                        list.splice( list.indexOf(text), 1 )
                        _characterNameListView.selectedCharacters = list
                    }
                }
            }

            Image {
                source: "qrc:/icons/content/add_box.png"
                width: _charactersPrefix.height
                height: _charactersPrefix.height
                opacity: 0.5
                visible: !_characterNameListView.visible

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                    onClicked: _characterNameListView.visible = true

                    ToolTipPopup {
                        container: parent
                        text: "Add another character."
                        visible: parent.containsMouse
                    }
                }
            }

            Image {
                source: "qrc:/icons/content/clear_all.png"
                width: _charactersPrefix.height
                height: _charactersPrefix.height
                opacity: 0.5
                visible: _characterNameListView.selectedCharacters.length > 0

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                    onClicked: {
                        _characterNameListView.selectedCharacters = []
                        _characterNameListView.visible = true
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
        id: _characterNameListView

        Layout.fillWidth: true
        Layout.leftMargin: 5
        Layout.rightMargin: 30
        Layout.preferredHeight: 280
    }
}

