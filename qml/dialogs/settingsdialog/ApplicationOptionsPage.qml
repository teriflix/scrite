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
    height: layout.height+50

    GridLayout {
        id: layout

        y: 10
        width: parent.width-50
        columns: 2
        rowSpacing: 20
        columnSpacing: 20

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclText { text: "Graphics" }

            ColumnLayout {
                width: parent.width

                VclCheckBox {
                    text: "Enable Animations"
                    checked: Runtime.applicationSettings.enableAnimations
                    onToggled: Runtime.applicationSettings.enableAnimations = checked
                }

                VclCheckBox {
                    text: "Use Software Renderer"
                    checked: Runtime.applicationSettings.useSoftwareRenderer
                    onToggled: {
                        Runtime.applicationSettings.useSoftwareRenderer = checked
                        Notification.active = true
                    }
                    Notification.title: "Requires Restart"
                    Notification.text: checked ? "Software renderer will be used when you restart Scrite." : "Accelerated graphics renderer will be used when you restart Scrite."
                    Notification.autoClose: false
                    ToolTip.text: "If you feel that Scrite is not responding fast enough, then you may want to switch to using a Software Renderer to speed things up. Otherwise, keep this option unchecked for best experience."
                    ToolTip.visible: hovered
                    ToolTip.delay: 1000
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 10

                    VclText {
                        id: themeLabel
                        text: "Theme: "
                        leftPadding: 10
                    }

                    VclComboBox {
                        Layout.fillWidth: true

                        model: Scrite.app.availableThemes
                        readonly property int materialStyleIndex: Scrite.app.availableThemes.indexOf("Material");
                        currentIndex: {
                            const idx = Scrite.app.availableThemes.indexOf(Runtime.applicationSettings.theme)
                            if(idx < 0)
                                return materialStyleIndex
                            return idx
                        }
                        onCurrentTextChanged: {
                            const oldTheme = Runtime.applicationSettings.theme
                            Runtime.applicationSettings.theme = currentText
                            Notification.active = oldTheme !== currentText
                        }
                        Notification.title: "Requires Restart"
                        Notification.text: "\"" + currentText + "\" theme will be used when you restart Scrite."
                        Notification.autoClose: false

                        ToolTip.text: "Scrite's UI is designed for use with Material theme and with software rendering disabled. If the UI is not rendering properly on your computer, then switching to a different theme may help."
                        ToolTip.visible: hovered
                        ToolTip.delay: 1000
                    }
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclText { text: "Display" }
            clip: true

            GridLayout {
                width: parent.width
                columns: 2

                VclText {
                    Layout.alignment: Qt.AlignVCenter
                    text: "DPI:"
                    padding: 5
                }

                VclTextField {
                    Layout.fillWidth: true
                    placeholderText: "leave empty for default (" + Math.round(Scrite.document.displayFormat.pageLayout.defaultResolution) + "), or enter a custom value."
                    text: Scrite.document.displayFormat.pageLayout.customResolution > 0 ? Scrite.document.displayFormat.pageLayout.customResolution : ""
                    onEditingComplete: {
                        var value = parseFloat(text)
                        if(isNaN(value))
                            Scrite.document.displayFormat.pageLayout.customResolution = 0
                        else
                            Scrite.document.displayFormat.pageLayout.customResolution = value
                    }
                }

                VclText {
                    Layout.alignment: Qt.AlignVCenter
                    text: "Scale:"
                    padding: 5
                }

                VclTextField {
                    Layout.fillWidth: true
                    enabled: Scrite.app.isWindowsPlatform
                    placeholderText: "Default: 1.0. Requires restart if changed."
                    text: Scrite.app.isWindowsPlatform ? Scrite.app.getWindowsEnvironmentVariable("SCRITE_UI_SCALE_FACTOR", "1.0") : "1.0"
                    onEditingComplete: {
                        var value = parseFloat(text)
                        if(isNaN(value))
                            value = 1.0

                        value = Math.min(Math.max(0.1,value),10)
                        value = Math.round(value*100)/100

                        Scrite.app.removeWindowsEnvironmentVariable("SCRITE_DPI_MODE")
                        Scrite.app.changeWindowsEnvironmentVariable("SCRITE_UI_SCALE_FACTOR", ""+value)
                    }
                }

                VclText {
                    Layout.alignment: Qt.AlignVCenter
                    text: "Font Size:"
                    padding: 5
                }

                VclTextField {
                    Layout.fillWidth: true
                    placeholderText: "Ideal font point-size to use for all text in the UI."
                    text: Scrite.app.customFontPointSize === 0 ? Runtime.idealFontMetrics.font.pointSize : Scrite.app.customFontPointSize
                    validator: IntValidator {
                        bottom: 0; top: 100
                    }
                    Component.onDestruction: applyCustomFontSize()

                    function applyCustomFontSize() {
                        if(length > 0)
                            Scrite.app.customFontPointSize = parseInt(text)
                        else
                            Scrite.app.customFontPointSize = 0
                    }
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclText { text: "Window Tabs" }

            ColumnLayout {
                width: parent.width
                spacing: 5

                VclText {
                    Layout.fillWidth: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                    text: "Move Notebook into the Structure tab to see all three aspects of your screenplay in a single view. (Note: This works when Scrite window size is atleast 1600 px wide.)"
                    wrapMode: Text.WordWrap
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Move Notebook into the Structure tab"
                    checked: Runtime.showNotebookInStructure
                    enabled: Runtime.canShowNotebookInStructure
                    onToggled: {
                        Runtime.workspaceSettings.showNotebookInStructure = checked
                        if(checked) {
                            Runtime.workspaceSettings.animateStructureIcon = true
                            Runtime.workspaceSettings.animateNotebookIcon = true
                        }
                    }
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Show Scrited Tab"
                    checked: Runtime.workspaceSettings.showScritedTab
                    onToggled: {
                        Runtime.workspaceSettings.showScritedTab = checked
                        if(!checked && Runtime.mainWindowTab === Runtime.e_ScritedTab) {
                            try {
                                Runtime.activateMainWindowTab(Runtime.e_ScreenplayTab)
                            } catch(e) {
                                Scrite.app.log(e)
                            }
                        }
                    }
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclText { text: "Saving Files" }
            clip: true

            GridLayout {
                width: parent.width
                columns: 2

                VclCheckBox {
                    text: "Auto Save"
                    checked: Scrite.document.autoSave
                    onToggled: Scrite.document.autoSave = checked
                }

                VclTextField {
                    label: enabled ? "Interval in seconds:" : ""
                    enabled: Scrite.document.autoSave
                    text: enabled ? Scrite.document.autoSaveDurationInSeconds : "No Auto Save"
                    validator: IntValidator {
                        bottom: 1; top: 3600
                    }
                    onTextEdited: Scrite.document.autoSaveDurationInSeconds = parseInt(text)
                }

                VclCheckBox {
                    text: "Limit Backups"
                    checked: Scrite.document.maxBackupCount > 0
                    onToggled: Scrite.document.maxBackupCount = checked ? 20 : 0
                }

                VclTextField {
                    label: enabled ? "Number of backups to retain:" : ""
                    enabled: Scrite.document.maxBackupCount > 0
                    text: enabled ? Scrite.document.maxBackupCount : "Unlimited Backups"
                    validator: IntValidator {
                        bottom: 1; top: 3600
                    }
                    onTextEdited: Scrite.document.maxBackupCount = parseInt(text)
                }

                VclCheckBox {
                    Layout.columnSpan: 2

                    text: "Enable Restore (" + (Scrite.document.autoSave ? "New Files Only" : "All Files") + ")"
                    width: parent.width
                    checked: Scrite.vault.enabled
                    onToggled: Scrite.vault.enabled = checked
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclText { text: "PDF Export" }

            ColumnLayout {
                spacing: 5
                width: parent.width

                VclText {
                    Layout.fillWidth: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                    text: "If you are facing issues with PDF export, then choose Printer Driver in the combo-box below. Otherwise we strongly advise you to use PDF Driver."
                    wrapMode: Text.WordWrap
                }

                VclComboBox {
                    Layout.fillWidth: true

                    enabled: false // Qt 5.15.7's PdfWriter is broken!
                    model: [
                        { "key": "PDF Driver", "value": true },
                        { "key": "Printer Driver", "value": false }
                    ]
                    textRole: "key"
                    // currentIndex: Runtime.pdfExportSettings.usePdfDriver ? 0 : 1
                    currentIndex: 1
                    onCurrentIndexChanged: Runtime.pdfExportSettings.usePdfDriver = (currentIndex === 0)
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclText { text: Scrite.app.isMacOSPlatform ? "Scroll/Flick Speed (Windows/Linux Only)" : "Scroll/Flick Speed" }
            enabled: !Scrite.app.isMacOSPlatform
            opacity: enabled ? 1 : 0.5

            RowLayout {
                width: parent.width

                Slider {
                    id: flickSpeedSlider
                    Layout.fillWidth: true

                    from: 0.1
                    to: 3
                    value: Runtime.workspaceSettings.flickScrollSpeedFactor
                    stepSize: 0.1
                    snapMode: Slider.SnapAlways
                    onMoved: Runtime.workspaceSettings.flickScrollSpeedFactor = value
                    ToolTip.text: "Configure the scroll sensitivity of your mouse and trackpad."
                }

                VclText {
                    text: Math.round( flickSpeedSlider.value*100 ) + "%"
                }

                FlatToolButton {
                    iconSource: "qrc:/icons/action/reset.png"
                    onClicked: Runtime.workspaceSettings.flickScrollSpeedFactor = 1
                    ToolTip.text: "Reset flick/scroll speed to 100%"
                }
            }
        }
    }
}
