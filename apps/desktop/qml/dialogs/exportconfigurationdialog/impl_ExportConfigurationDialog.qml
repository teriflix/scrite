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
import ".."
import "../../notifications"

VclDialog {
    id: root

    property AbstractExporter exporter

    width: Math.min(Scrite.window.width*0.9, 700)
    height: Math.min(Scrite.window.height*0.9, 600)

    handleLanguageShortcuts: true
    title: exporter ? ("Export to " + exporter.formatName) : "Export Configuration Dialog"

    content: visible ? (_private.exportEnabled ? exportConfigContent : _exportFeatureDisabledContent) : null
    bottomBar: visible && _private.exportEnabled ? exportButtonFooter : null

    // Show this component if the exporter feature is disabled for the user
    Component {
        id: _exportFeatureDisabledContent

        DisabledFeatureNotice {
            color: Qt.rgba(1,1,1,0.9)
            featureName: exporter ? exporter.format : Scrite.ExportFeature
        }
    }

    // Show this component if the exporter is enabled for the current user
    Component {
        id: _exportConfigContent

        Item {
            implicitHeight: _exportConfigItemLayout.implicitHeight + 2*_exportConfigItemLayout.spacing

            ColumnLayout {
                id: _exportConfigItemLayout

                width: parent.width - 4*spacing
                anchors.top: parent.top
                anchors.topMargin: 2*spacing
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 10

                // Show file selector for non-PDF exporters
                FileSelector {
                    id: _fileSelector
                    Layout.fillWidth: true

                    label: "Select a file to export into"
                    absoluteFilePath: exporter.fileName
                    onAbsoluteFilePathChanged: exporter.fileName = absoluteFilePath
                    nameFilters: exporter.nameFilters
                    tabSequenceManager: _tabSequence
                    visible: !_private.isPdfExport
                    enabled: visible && _private.exportSaveFeature.enabled
                    opacity: enabled ? 1 : 0.5
                }

                VclLabel {
                    Layout.fillWidth: true
                    Layout.rightMargin: 20
                    Layout.bottomMargin: 24

                    visible: !_private.exportSaveFeature.enabled
                    wrapMode: Text.WordWrap

                    text: "<b>NOTE:</b> Your current subscription plan allows you to preview, but not save the PDF. Click <u>here</u> to know about your subscription plan and features."

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: UserAccountDialog.launch("Subscriptions")
                    }
                }

                // Show configuration editors
                Repeater {
                    model: _private.configuration.fields

                    delegate: Loader {
                        id: _fieldLoader

                        required property int index
                        required property var modelData

                        Layout.fillWidth: true
                        source: _delegateChooser.delegateSource(modelData.editor)
                        opacity: enabled ? 1 : 0.5
                        enabled: {
                            if(modelData.feature !== "") {
                                const afc = Qt.createQmlObject("import io.scrite.components 1.0; AppFeature { }", _fieldLoader)
                                afc.featureName = modelData.feature
                                const ret = afc.enabled
                                afc.destroy()
                                return ret
                            }
                            return true
                        }

                        onLoaded: {
                            item.exporter = exporter
                            item._tabSequence = _tabSequence
                            item.fieldInfo = modelData
                        }
                    }
                }

                // Dummy item
                Item {
                    TabSequenceManager {
                        id: _tabSequence
                        wrapAround: true
                    }
                }

                Component.onCompleted: {
                    if(Object.isOfType(exporter, "StructureExporter"))
                        Runtime.activateMainWindowTab(Runtime.MainWindowTab.StructureTab)

                    if(Object.isOfType(exporter, "AbstractTextDocumentExporter")) {
                        exporter.capitalizeSentences = Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences
                        exporter.polishParagraphs = Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
                    }
                }
            }

            Component.onCompleted: {
                if(_private.isPdfExport)
                    Runtime.showHelpTip("watermark")
                Runtime.showHelpTip(Object.typeOf(exporter))
            }
        }
    }

    Component {
        id: _exportButtonFooter

        Item {
            id: _footerItem
            height: _footerLayout.height+20

            RowLayout {
                id: _footerLayout
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 16

                spacing: 20

                VclButton {
                    enabled: exporter.canCopyToClipboard
                    visible: exporter.canCopyToClipboard
                    text: "Copy to Clipboard"
                    onClicked: {
                        _exportJob.copyToClipboard = true
                        _exportJob.start()
                    }
                }

                VclButton {
                    id: _exportButton

                    Component.onCompleted: Qt.callLater(_exportButton.forceActiveFocus)

                    enabled: exporter.fileName !== "" && _private.exportEnabled
                    text: _private.isPdfExport ? "Generate PDF" : "Export"
                    onClicked: {
                        _exportJob.copyToClipboard = false
                        _exportJob.start()
                    }

                    ActionHandler {
                        action: root.acceptAction

                        onTriggered: _exportButton.clicked()
                    }
                }
            }

            SequentialAnimation {
                id: _exportJob

                property bool copyToClipboard: false

                running: false

                // Launch wait dialog ..
                ScriptAction {
                    script: {
                        Object.save(exporter)

                        const message = _private.isPdfExport ? "Generating PDF ..." : ("Exporting to \"" + exporter.fileName + "\" ...")
                        _private.waitDialog = WaitDialog.launch(message, Aggregation.progressReport(exporter))
                    }
                }

                // Wait for it to show up on the UI ...
                PauseAnimation {
                    duration: 200
                }

                // Perform the export job ...
                ScriptAction {
                    script: {
                        const dlFileName = exporter.fileName
                        if(_private.isPdfExport) {
                            exporter.fileName = Runtime.fileNamager.generateUniqueTemporaryFileName("pdf")
                            Runtime.fileNamager.addToAutoDeleteList(exporter.fileName)
                        }

                        const success = exporter.write(_exportJob.copyToClipboard ? AbstractExporter.ClipboardTarget : AbstractExporter.FileTarget)

                        Qt.callLater(_private.waitDialog.close)
                        _private.waitDialog = null

                        if(success) {
                            if(_exportJob.copyToClipboard) {
                                MessageBox.information(exporter.formatName + " - Export", "Successfully copied text to clipboard.", root.close)
                                return
                            }

                            if(_private.isPdfExport) {
                                PdfDialog.launch("Screenplay", exporter.fileName, dlFileName, 2, _private.exportSaveFeature.enabled)
                            } else
                                File.revealOnDesktop(exporter.fileName)
                            Qt.callLater(root.close)
                        } else {
                            const exporterErrors = Aggregation.errorReport(exporter)
                            MessageBox.information(exporter.formatName + " - Export", exporterErrors.errorMessage, root.close)
                        }
                    }
                }
            }
        }
    }

    // Factory function for loading delegates
    QtObject {
        id: _delegateChooser

        readonly property var knownEditors: [
            "IntegerSpinBox",
            "CheckBox",
            "TextBox"
        ]

        function delegateSource(kind) {
            var ret = "./editor_"
            if(knownEditors.indexOf(kind) >= 0)
                ret += kind
            else
                ret += "Unknown"
            ret += ".qml"
            return ret
        }
    }

    // Private section
    QtObject {
        id: _private

        property var configuration: exporter ? exporter.configuration() : {"title": "Unknown", "fields": []}
        property bool isPdfExport: exporter ? exporter.format === "Screenplay/PDF" : false
        property bool exportEnabled: exporter ? exporter.featureEnabled : false

        property AppFeature exportSaveFeature: AppFeature {
            featureName: exporter ? "export/" + exporter.format.toLowerCase() + (_private.isPdfExport ? "/save": "") : "export"
        }

        property VclDialog waitDialog
    }

    onClosed: Runtime.execLater(exporter, 100, () => { if(exporter) exporter.discard() } )

    Connections {
        target: root.exporter
        function onAboutToDelete() {
            root.exporter = null
        }
    }
}
