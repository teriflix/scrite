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
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    id: root

    height: layout.height+50

    GridLayout {
        id: layout

        y: 10
        width: parent.width-20
        columns: 2
        rowSpacing: 20
        columnSpacing: 20

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2
            Layout.fillHeight: true

            label: VclLabel { text: "Graphics" }

            ColumnLayout {
                width: parent.width

                spacing: 0

                VclCheckBox {
                    text: "Enable Animations"
                    checked: Runtime.applicationSettings.enableAnimations
                    onToggled: Runtime.applicationSettings.enableAnimations = checked
                }

                VclCheckBox {
                    text: "Use Native Text Rendering"
                    checked: Runtime.applicationSettings.useNativeTextRendering
                    onToggled: {
                        Runtime.applicationSettings.useNativeTextRendering = checked

                        if(Runtime.useNativeTextRendering !== checked) {
                            const msg = checked ? "Native OS text rendering engine will be used when you restart Scrite." : "Qt's text renderning engine will be used when you restart Scrite."
                            MessageBox.information("Requires Restart", msg)
                        }
                    }

                    ToolTip.text: "If texts are not being rendered properly on your display, then switch to native text rendering. Otherwise, keep this setting unchecked."
                    ToolTip.visible: hovered
                    ToolTip.delay: 1000
                }

                VclCheckBox {
                    text: "Use Software Renderer"
                    checked: Runtime.applicationSettings.useSoftwareRenderer
                    onToggled: {
                        Runtime.applicationSettings.useSoftwareRenderer = checked

                        if(Runtime.currentUseSoftwareRenderer !== checked) {
                            const msg = checked ? "Software renderer will be used when you restart Scrite." : "Accelerated graphics renderer will be used when you restart Scrite."
                            MessageBox.information("Requires Restart", msg)
                        }
                    }
                    ToolTip.text: "If you feel that Scrite is not responding fast enough, then you may want to switch to using a Software Renderer to speed things up. Otherwise, keep this option unchecked for best experience."
                    ToolTip.visible: hovered
                    ToolTip.delay: 1000
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2
            Layout.fillHeight: true

            label: VclLabel { text: "Display" }
            clip: true

            GridLayout {
                width: parent.width
                columns: 2

                VclLabel {
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

                VclLabel {
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

                VclLabel {
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
            Layout.fillHeight: true

            label: VclLabel { text: "Window Tabs" }

            ColumnLayout {
                width: parent.width
                spacing: 5

                VclLabel {
                    property string secondSentence: {
                        if(Runtime.canShowNotebookInStructure)
                            return ""
                        return "(Expand the window by " + 10*Math.ceil((Runtime.minWindowWidthForShowingNotebookInStructure - Runtime.width)/10) + " pixels to enable this option)"
                    }

                    Layout.fillWidth: true
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    text: "Move Notebook into the Structure tab to see all three aspects of your screenplay in a single view. " + secondSentence
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
                        if(!checked && Runtime.mainWindowTab === Runtime.MainWindowTab.ScritedTab) {
                            try {
                                Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScreenplayTab)
                            } catch(e) {
                                console.log(e)
                            }
                        }
                    }
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2
            Layout.fillHeight: true
            Layout.rowSpan: 2

            label: VclLabel { text: "Saving Files" }
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
                    Layout.fillWidth: true

                    text: "Enable Restore (" + (Scrite.document.autoSave ? "New Files Only" : "All Files") + ")"                    
                    checked: Scrite.vault.enabled
                    onToggled: Scrite.vault.enabled = checked
                }

                VclCheckBox {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    text: "Ask to Reload if File Changes"
                    checked: Runtime.applicationSettings.reloadPrompt
                    onToggled: Runtime.applicationSettings.reloadPrompt = checked
                }

                VclCheckBox {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    text: "Notify Missing Recent Files"
                    checked: Runtime.applicationSettings.notifyMissingRecentFiles
                    onToggled: Runtime.applicationSettings.notifyMissingRecentFiles = checked
                }
            }
        }

        /*
          // We don't enable this on any platform, so why bother showing it?

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2
            Layout.fillHeight: true

            label: VclLabel { text: "PDF Export" }

            ColumnLayout {
                spacing: 5
                width: parent.width

                VclLabel {
                    Layout.fillWidth: true
                    font.pointSize: Runtime.minimumFontMetrics.font.pointSize
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
        */

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2
            Layout.fillHeight: true

            label: VclLabel { text: Scrite.app.isMacOSPlatform ? "Scroll/Flick Speed (Windows/Linux Only)" : "Scroll/Flick Speed" }
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

                VclLabel {
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
