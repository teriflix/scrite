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
import "qrc:/qml/dialogs"

Item {
    id: root

    height: layout.height + 2*layout.margin

    GridLayout {
        id: layout

        readonly property real margin: 10

        width: parent.width-2*margin
        y: margin*2

        columns: 2
        rowSpacing: margin*2
        columnSpacing: margin*2

        RowLayout {
            Layout.preferredWidth: (parent.width-(parent.columns-1)*parent.columnSpacing)/parent.columns
            spacing: 10

            VclLabel {
                id: themeLabel
                text: "Theme: "
            }

            VclComboBox {
                id: themesComboBox

                readonly property int materialStyleIndex: Scrite.app.availableThemes.indexOf("Material");

                Layout.fillWidth: true

                enabled: Runtime.currentUseSoftwareRenderer === false
                model: Scrite.app.availableThemes

                currentIndex: {
                    const idx = Scrite.app.availableThemes.indexOf(Runtime.applicationSettings.theme)
                    if(idx < 0)
                        return materialStyleIndex
                    return idx
                }

                onCurrentTextChanged: {
                    Runtime.applicationSettings.theme = currentText
                    if(Runtime.currentTheme !== currentText)
                        MessageBox.information("Requires Restart", "Scrite will use <b>" + currentText + "</b> theme upon restart.")
                }

                ToolTipPopup {
                    container: parent
                    text: "Scrite's UI is designed for use with Material theme and with software rendering disabled. If the UI is not rendering properly on your computer, then switching to a different theme may help."
                    visible: parent.hovered
                }
            }
        }

        Item {
            Layout.preferredWidth: (parent.width-(parent.columns-1)*parent.columnSpacing)/parent.columns
            Layout.fillHeight: true
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (parent.width-(parent.columns-1)*parent.columnSpacing)/parent.columns

            label: VclLabel {
                text: "Colors"
            }

            ColumnLayout {
                width: parent.width
                spacing: 10

                GridLayout {
                    Layout.alignment: Qt.AlignHCenter

                    columns: 4
                    rowSpacing: 6
                    columnSpacing: 6

                    Repeater {
                        model: _private.availableColorOptions

                        delegate: Rectangle {
                            required property int modelData

                            implicitWidth: _private.colorSelectorSize
                            implicitHeight: _private.colorSelectorSize

                            color: Material.color(modelData)

                            VclLabel {
                                anchors.centerIn: parent
                                text: "✓"
                                color: Color.textColorFor(parent.color)
                                visible: Runtime.colors.accent.key === modelData
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    Runtime.colors.accent.key = modelData
                                    Runtime.applicationSettings.accentColor = modelData
                                }
                            }
                        }
                    }
                }

                ToolButton {
                    Layout.alignment: Qt.AlignHCenter

                    text: "Reset"
                    enabled: Runtime.colors.accent.key !== Runtime.colors.defaultAccentColor
                    icon.source: "qrc:/icons/action/reset.png"

                    onClicked: {
                        Runtime.colors.accent.key = Runtime.colors.defaultAccentColor
                        Runtime.applicationSettings.accentColor = Runtime.colors.defaultAccentColor
                    }
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (parent.width-(parent.columns-1)*parent.columnSpacing)/parent.columns

            enabled: false

            label: VclLabel {
                text: "UI Mode"
            }

            ColumnLayout {
                width: parent.width

                VclLabel {
                    Layout.fillWidth: true

                    text: "This feature is in the works, the options below will get enabled whenever its fully implemented."
                    wrapMode: Text.WordWrap
                }

                VclRadioButton {
                    text: "Light"
                }

                VclRadioButton {
                    text: "Dark"
                }

                VclRadioButton {
                    text: "System"
                }
            }
        }
    }

    QtObject {
        id: _private

        readonly property var availableColorOptions: [
                // Material.Red,
                Material.Pink,
                Material.Purple,
                Material.DeepPurple,

                Material.Indigo,
                Material.Blue,
                Material.LightBlue,
                Material.Cyan,

                Material.Teal,
                Material.Green,
                Material.LightGreen,
                // Material.Lime,

                // Material.Yellow,
                // Material.Amber,
                // Material.Orange,
                // Material.DeepOrange,

                Material.Brown,
                Material.Grey,
                Material.BlueGrey
            ]

        readonly property real colorSelectorSize: 45
    }
}
