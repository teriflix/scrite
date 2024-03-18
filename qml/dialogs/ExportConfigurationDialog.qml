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

VclDialog {
    id: root

    property AbstractExporter exporter

    width: Math.min(Scrite.window.width*0.9, 700)
    height: Math.min(Scrite.window.height*0.9, 600)
    title: exporter ? ("Export to " + exporter.formatName) : "Export Configuration Dialog"

    content: visible ? (_private.exportEnabled ? exportConfigContent : exportFeatureDisabledContent) : null
    bottomBar: visible && _private.exportEnabled ? exportButtonFooter : null

    // Show this component if the exporter feature is disabled for the user
    Component {
        id: exportFeatureDisabledContent

        DisabledFeatureNotice {
            color: Qt.rgba(1,1,1,0.9)
            featureName: exporter.format
        }
    }

    // Show this component if the exporter is enabled for the current user
    Component {
        id: exportConfigContent

        Item {
            implicitHeight: exportConfigItemLayout.implicitHeight + 2*exportConfigItemLayout.spacing

            ColumnLayout {
                id: exportConfigItemLayout

                width: parent.width - 4*spacing
                anchors.top: parent.top
                anchors.topMargin: 2*spacing
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 10

                // Show file selector for non-PDF exporters
                FileSelector {
                    id: fileSelector
                    Layout.fillWidth: true

                    label: "Select a file to export into"
                    absoluteFilePath: exporter.fileName
                    onAbsoluteFilePathChanged: exporter.fileName = absoluteFilePath
                    nameFilters: exporter.nameFilters
                    tabSequenceManager: tabSequence
                    visible: !_private.isPdfExport
                    enabled: visible
                }

                // Show a group box for selecting fonts to export
                // TODO: Ideally, all fonts used in the current Scrite document must
                // be automatically checked.
                GridLayout {
                    Layout.fillWidth: true

                    columns: 3
                    rowSpacing: 5
                    columnSpacing: 5
                    visible: exporter.canBundleFonts

                    Repeater {
                        model: GenericArrayModel {
                            array: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")
                            objectMembers: ["key", "value"]
                        }

                        VclCheckBox {
                            required property string key
                            required property int value

                            Layout.fillWidth: true

                            checkable: true
                            checked: exporter.isFontForLanguageBundled(value)
                            text: key
                            onToggled: exporter.bundleFontForLanguage(value,checked)
                            TabSequenceItem.manager: tabSequence
                        }
                    }
                }

                // Show configuration editors
                Repeater {
                    model: _private.formInfo.fields

                    Loader {
                        required property int index
                        required property var modelData

                        Layout.fillWidth: true
                        source: delegateChooser.delegateSource(modelData.editor)
                        onLoaded: {
                            item.exporter = exporter
                            item.tabSequence = tabSequence
                            item.fieldInfo = modelData
                        }
                    }
                }

                // Dummy item
                Item {
                    property ErrorReport exporterErrors: Aggregation.findErrorReport(exporter)
                    Notification.title: exporter.formatName + " - Export"
                    Notification.text: exporterErrors.errorMessage
                    Notification.active: exporterErrors.hasError
                    Notification.autoClose: false
                    Notification.onDismissed: root.close()

                    TabSequenceManager {
                        id: tabSequence
                        wrapAround: true
                    }
                }

                Component.onCompleted: {
                    if(Scrite.app.verifyType(exporter, "StructureExporter"))
                        Runtime.activateMainWindowTab(Runtime.e_StructureTab)

                    if(Scrite.app.verifyType(exporter, "AbstractTextDocumentExporter")) {
                        exporter.capitalizeSentences = Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences
                        exporter.polishParagraphs = Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
                    }
                }
            }
        }
    }

    Component {
        id: exportButtonFooter

        Item {
            id: footerItem
            height: footerLayout.height+20

            RowLayout {
                id: footerLayout
                width: parent.width-32
                anchors.centerIn: parent

                VclButton {
                    Layout.alignment: Qt.AlignRight
                    enabled: exporter.fileName !== "" && _private.exportEnabled
                    text: _private.isPdfExport ? "Generate PDF" : "Export"
                    onClicked: {
                        progressDialog.open()
                        Utils.execLater(progressDialog, 100, footerItem.doExport)
                    }
                }
            }

            VclDialog {
                id: progressDialog

                title: "Please wait ..."
                closePolicy: Popup.NoAutoClose
                titleBarButtons: null
                width: root.width * 0.8
                height: 150

                contentItem: VclText {
                    text: _private.isPdfExport ? "Generating PDF ..." : ("Exporting to \"" + exporter.fileName + "\" ...")
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    padding: 20
                }
            }

            function doExport() {
                const dlFileName = exporter.fileName
                if(_private.isPdfExport) {
                    exporter.fileName = Runtime.fileNamager.generateUniqueTemporaryFileName("pdf")
                    Runtime.fileNamager.addToAutoDeleteList(exporter.fileName)
                }

                const success = exporter.write()

                if(success) {
                    if(_private.isPdfExport) {
                        const params = {
                            "title": "Screenplay",
                            "filePath": exporter.fileName,
                            "dlFilePath": dlFileName,
                            "pagesPerRow": 2,
                            "allowSave": _private.exportSaveFeature.enabled
                        }
                        Announcement.shout(Runtime.announcementIds.showPdfRequest, params)
                    } else
                        Scrite.app.revealFileOnDesktop(exporter.fileName)
                    Qt.callLater(root.close)
                }

                Qt.callLater(progressDialog.close)
            }
        }
    }

    // Factory function for loading delegates
    QtObject {
        id: delegateChooser

        readonly property var knownEditors: [
            "IntegerSpinBox",
            "CheckBox",
            "TextBox"
        ]

        function delegateSource(kind) {
            var ret = "./exportconfigurationdialog/editor_"
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

        property var formInfo: exporter ? exporter.configurationFormInfo() : {"title": "Unknown", "fields": []}
        property bool isPdfExport: exporter ? exporter.format === "Screenplay/Adobe PDF" : false
        property bool exportEnabled: isPdfExport ? exporter.featureEnabled : _private.exportSaveFeature.enabled

        property AppFeature exportSaveFeature: AppFeature {
            featureName: exporter ? "export/" + exporter.formatName.toLowerCase() + "/save" : "export"
        }
    }

    onClosed: Utils.execLater(root, 100, exporter.discard)

    Connections {
        target: root.exporter
        function onAboutToDelete() {
            root.exporter = null
        }
    }
}
