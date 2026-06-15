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

import "../"
import "../../globals"
import "../../controls"
import "../../helpers"

Item {
    id: root

    height: _layout.height + 2*_layout.margin

    GridLayout {
        id: _layout

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
                id: _themeLabel
                text: "Theme: "
            }

            VclComboBox {
                id: _themesComboBox

                Layout.fillWidth: true

                enabled: Runtime.currentUseSoftwareRenderer === false
                model: Scrite.app.availableThemes

                currentIndex: {
                    const idx = Scrite.app.availableThemes.indexOf(Runtime.applicationSettings.uiTheme)
                    return idx < 0 ? 0 : idx
                }

                onCurrentTextChanged: {
                    Runtime.applicationSettings.uiTheme = currentText
                    if(Runtime.currentTheme !== currentText)
                        MessageBox.information("Requires Restart", "Scrite will use <b>" + currentText + "</b> theme upon restart.")
                }

                ToolTipPopup {
                    container: _themesComboBox
                    text: "Scrite's UI is designed for use with Material theme and with software rendering disabled. If the UI is not rendering properly on your computer, then switching to a different theme may help."
                    visible: _themesComboBox.hovered
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
                            id: _availableColorOptionDelegate

                            required property int modelData

                            implicitWidth: _private.colorSelectorSize
                            implicitHeight: _private.colorSelectorSize

                            color: Material.color(modelData)

                            VclLabel {
                                anchors.centerIn: parent
                                text: "✓"
                                color: Color.textColorFor(parent.color)
                                visible: Runtime.colors.accent.key === _availableColorOptionDelegate.modelData
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    Runtime.colors.accent.key = _availableColorOptionDelegate.modelData
                                    Runtime.applicationSettings.accentColor = _availableColorOptionDelegate.modelData
                                }
                            }
                        }
                    }
                }

                ToolButton {
                    Layout.alignment: Qt.AlignHCenter

                    text: "Reset"
                    enabled: Runtime.colors.accent.key !== Runtime.colors.defaultAccentColor
                    icon.source: Runtime.themedIcon("qrc:/icons/action/reset.png")

                    onClicked: {
                        Runtime.colors.accent.key = Runtime.colors.defaultAccentColor
                        Runtime.applicationSettings.accentColor = Runtime.colors.defaultAccentColor
                    }
                }
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (parent.width-(parent.columns-1)*parent.columnSpacing)/parent.columns

            GroupBox {
                Layout.fillWidth: true

                label: VclLabel {
                    text: "UI / Color Mode"
                }

                ColumnLayout {
                    width: parent.width

                    VclRadioButton {
                        text: "Light"
                        checked: Runtime.applicationSettings.colorMode === "Light"
                        onClicked: Runtime.applicationSettings.colorMode = "Light"
                    }

                    VclRadioButton {
                        text: "Dark"
                        checked: Runtime.applicationSettings.colorMode === "Dark"
                        onClicked: Runtime.applicationSettings.colorMode = "Dark"
                    }

                    VclRadioButton {
                        text: "System"
                        checked: Runtime.applicationSettings.colorMode === "System"
                        onClicked: Runtime.applicationSettings.colorMode = "System"
                    }
                }
            }

            GroupBox {
                Layout.fillWidth: true

                label: VclCheckBox {
                    text: "Use Custom PDF Page Colors"
                    checked: Runtime.applicationSettings.useCustomPdfPageColor
                    onToggled: Runtime.applicationSettings.useCustomPdfPageColor = !Runtime.applicationSettings.useCustomPdfPageColor
                }

                ColumnLayout {
                    width: parent.width

                    enabled: Runtime.applicationSettings.useCustomPdfPageColor

                    RowLayout {
                        Rectangle {
                            implicitWidth: _private.colorSelectorSize/2
                            implicitHeight: _private.colorSelectorSize/2

                            opacity: enabled ? 1 : 0.5
                            color: Runtime.applicationSettings.lightModePdfPageColor
                            border.width: 1
                            border.color: "black"

                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    Runtime.applicationSettings.lightModePdfPageColor = Color.pick(parent.color)
                                }
                            }
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            text: "Light Mode Page"
                        }

                        ToolButton {
                            Layout.alignment: Qt.AlignHCenter

                            text: "Reset"
                            enabled: Color.name(Runtime.applicationSettings.lightModePdfPageColor) !== Color.name("white")
                            icon.source: Runtime.themedIcon("qrc:/icons/action/reset.png")

                            onClicked: {
                                Runtime.applicationSettings.lightModePdfPageColor = "white"
                            }
                        }
                    }

                    RowLayout {
                        Rectangle {
                            implicitWidth: _private.colorSelectorSize/2
                            implicitHeight: _private.colorSelectorSize/2

                            opacity: enabled ? 1 : 0.5
                            color: Runtime.applicationSettings.darkModePdfPageColor
                            border.width: 1
                            border.color: "black"

                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    Runtime.applicationSettings.darkModePdfPageColor = Color.pick(parent.color)
                                }
                            }
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            text: "Dark Mode Page"
                        }

                        ToolButton {
                            Layout.alignment: Qt.AlignHCenter

                            text: "Reset"
                            enabled: Color.name(Runtime.applicationSettings.darkModePdfPageColor) !== Color.name("lightgray")
                            icon.source: Runtime.themedIcon("qrc:/icons/action/reset.png")

                            onClicked: {
                                Runtime.applicationSettings.darkModePdfPageColor = "lightgray"
                            }
                        }
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap
                        text: "While the generated PDF files will always have a white background, this setting allows you to customise the colors to use while displaying PDFs within the app."
                    }
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
