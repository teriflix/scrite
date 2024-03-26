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
import "qrc:/qml/dialogs"

Item {
    id: root
    height: layout.height + 2*layout.margin

    ColumnLayout {
        id: layout

        readonly property real margin: 10

        width: parent.width-margin
        y: margin

        spacing: 20

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                text: "Colors"
            }

            ColumnLayout {
                width: parent.width

                spacing: 20

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: parent.spacing * 2

                    ColumnLayout {
                        spacing: 10

                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter

                            VclText {
                                text: "Primary"
                            }

                            FlatToolButton {
                                ToolTip.text: "Reset default accent color"

                                iconSource: "qrc:/icons/action/reset.png"
                                enabled: Runtime.colors.primary.key !== Runtime.colors.defaultPrimaryColor

                                onClicked: {
                                    Runtime.colors.primary.key = Runtime.colors.defaultPrimaryColor
                                    Runtime.applicationSettings.primaryColor = Runtime.colors.defaultPrimaryColor
                                }
                            }
                        }

                        GridLayout {
                            columns: 4
                            rowSpacing: 6
                            columnSpacing: 6

                            Repeater {
                                model: _private.availableColorOptions

                                Rectangle {
                                    required property int modelData

                                    implicitWidth: _private.colorSelectorSize
                                    implicitHeight: _private.colorSelectorSize

                                    color: Material.color(modelData)

                                    VclText {
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: Scrite.app.textColorFor(parent.color)
                                        visible: Runtime.colors.primary.key === modelData
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            Runtime.colors.primary.key = modelData
                                            Runtime.applicationSettings.primaryColor = modelData
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true

                        implicitWidth: 1

                        color: Runtime.colors.primary.borderColor
                    }

                    ColumnLayout {
                        spacing: 10

                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter

                            VclText {
                                text: "Accent"
                            }

                            FlatToolButton {
                                ToolTip.text: "Reset default accent color"

                                iconSource: "qrc:/icons/action/reset.png"
                                enabled: Runtime.colors.accent.key !== Runtime.colors.defaultAccentColor

                                onClicked: {
                                    Runtime.colors.accent.key = Runtime.colors.defaultAccentColor
                                    Runtime.applicationSettings.accentColor = Runtime.colors.defaultAccentColor
                                }
                            }
                        }

                        GridLayout {
                            columns: 4
                            rowSpacing: 6
                            columnSpacing: 6

                            Repeater {
                                model: _private.availableColorOptions

                                Rectangle {
                                    required property int modelData

                                    implicitWidth: _private.colorSelectorSize
                                    implicitHeight: _private.colorSelectorSize

                                    color: Material.color(modelData)

                                    VclText {
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: Scrite.app.textColorFor(parent.color)
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
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                text: "UI Mode"
            }

            ColumnLayout {
                width: parent.width

                VclText {
                    Layout.fillWidth: true

                    text: "This feature is in the works, the options below will get enabled whenever its fully implemented."
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    enabled: false
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 10

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
    }

    QtObject {
        id: _private

        readonly property var availableColorOptions: [
                Material.Red,
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
                Material.Lime,
                Material.Yellow,
                Material.Amber	,
                Material.Orange,
                Material.DeepOrange,
                Material.Brown,
                Material.Grey,
                Material.BlueGrey
            ]

        readonly property real colorSelectorSize: 45
    }
}
