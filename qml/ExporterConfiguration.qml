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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import io.scrite.components 1.0
import "../js/utils.js" as Utils

Item {
    id: configurationBox
    property AbstractExporter exporter
    property var formInfo: exporter ? exporter.configurationFormInfo() : {"title": "Unknown", "fields": []}
    readonly property bool isPdfExport: exporter ? exporter.format === "Screenplay/Adobe PDF" : false

    width: 700
    height: {
        if(exporter && exporter.featureEnabled)
            return (exporter.requiresConfiguration || exporter.canBundleFonts) ? Math.min(documentUI.height*0.9, contentLoader.item.idealHeight) : 300
        return 500
    }

    Component.onCompleted: {
        modalDialog.closeOnEscape = false

        exporter = typeof modalDialog.arguments === "string" ? Scrite.document.createExporter(modalDialog.arguments) : modalDialog.arguments
        if(exporter === null) {
            modalDialog.closeable = true
            var exportKind = modalDialog.arguments.split("/").last()
            notice.text = exportKind + " Export"
        } else {
            if(Scrite.app.verifyType(exporter, "StructureExporter"))
                mainTabBar.currentIndex = 1 // FIXME: Ugly hack to ensure that structure tab is active for StructureExporter.

            if(Scrite.app.verifyType(exporter, "AbstractTextDocumentExporter")) {
                exporter.capitalizeSentences = screenplayEditorSettings.enableAutoCapitalizeSentences
                exporter.polishParagraphs = screenplayEditorSettings.enableAutoPolishParagraphs
            }
        }

        modalDialog.arguments = undefined
    }

    Text {
        id: notice
        anchors.centerIn: parent
        visible: exporter === null
        font.pixelSize: 20
        width: parent.width*0.85
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        horizontalAlignment: Text.AlignHCenter
    }

    TabSequenceManager {
        id: tabSequence
        wrapAround: true
    }

    Loader {
        id: contentLoader
        anchors.fill: parent
        active: exporter
        sourceComponent: Item {
            property real idealHeight: formView.contentHeight + buttonRow.height + 100

            ScrollView {
                id: formView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: buttonRow.top
                anchors.margins: 20
                anchors.bottomMargin: 10
                clip: true
                enabled: exporter.featureEnabled
                opacity: enabled ? 1 : 0.5

                property bool scrollBarRequired: formView.height < formView.contentHeight
                ScrollBar.vertical.policy: formView.scrollBarRequired ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

                Column {
                    width: formView.width - (formView.scrollBarRequired ? 17 : 0)
                    spacing: 10

                    Text {
                        font.pointSize: Screen.devicePixelRatio > 1 ? 24 : 20
                        font.bold: true
                        text: isPdfExport ? "Generate PDF" : (exporter.formatName + " - Export")
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    FileSelector {
                        id: fileSelector
                        width: parent.width-20
                        label: "Select a file to export into"
                        absoluteFilePath: exporter.fileName
                        onAbsoluteFilePathChanged: exporter.fileName = absoluteFilePath
                        nameFilters: exporter.nameFilters
                        tabSequenceManager: tabSequence
                        visible: !isPdfExport
                        enabled: visible
                    }

                    Loader {
                        width: parent.width
                        active: exporter.canBundleFonts
                        sourceComponent: GroupBox {
                            width: parent.width
                            label: Text {
                                text: "Export fonts for the following languages"
                                font.pointSize: Scrite.app.idealFontPointSize
                            }
                            height: languageBundleView.height+45

                            Grid {
                                id: languageBundleView
                                width: parent.width-10
                                anchors.top: parent.top
                                spacing: 5
                                columns: 3

                                Repeater {
                                    model: Scrite.app.enumerationModel(Scrite.app.transliterationEngine, "Language")
                                    delegate: CheckBox2 {
                                        width: languageBundleView.width/languageBundleView.columns
                                        checkable: true
                                        checked: exporter.isFontForLanguageBundled(modelData.value)
                                        text: modelData.key
                                        onToggled: exporter.bundleFontForLanguage(modelData.value,checked)
                                        TabSequenceItem.manager: tabSequence
                                        font.pointSize: Scrite.app.idealFontPointSize
                                    }
                                }
                            }
                        }
                    }

                    Repeater {
                        model: formInfo.fields

                        Loader {
                            width: formView.width
                            active: true
                            sourceComponent: loadFieldEditor(modelData.editor)
                            onItemChanged: {
                                if(item)
                                    item.fieldInfo = modelData
                            }
                        }
                    }
                }
            }

            DisabledFeatureNotice {
                anchors.fill: formView
                visible: isPdfExport ? !exporter.featureEnabled : !exportSaveFeature.enabled
                color: Qt.rgba(1,1,1,0.9)
                featureName: exporter.formatName + " - Export"
            }

            Row {
                id: buttonRow
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 20
                spacing: 20

                Button2 {
                    text: "Cancel"
                    onClicked: {
                        exporter.discard()
                        modalDialog.close()
                    }

                    EventFilter.target: Scrite.app
                    EventFilter.events: [6]
                    EventFilter.onFilter: {
                        if(event.key === Qt.Key_Escape) {
                            result.acceptEvent = true
                            result.filter = true
                            exporter.discard()
                            modalDialog.close()
                        }
                    }
                }

                Button2 {
                    enabled: fileSelector.absoluteFilePath !== "" && exporter.featureEnabled
                    text: isPdfExport ? "Generate" : "Export"
                    onClicked: busyOverlay.visible = true
                }
            }

            BusyOverlay {
                id: busyOverlay
                anchors.fill: parent
                busyMessage: isPdfExport ? "Generating PDF ..." : "Exporting to \"" + exporter.fileName + "\" ..."

                FileManager {
                    id: fileManager
                }

                AppFeature {
                    id: exportSaveFeature
                    featureName: exporter ? "export/" + exporter.formatName.toLowerCase() + "/save" : "export"
                }

                onVisibleChanged: {
                    if(visible) {
                        Utils.execLater(busyOverlay, 100, function() {
                            const dlFileName = exporter.fileName
                            if(isPdfExport)
                                exporter.fileName = fileManager.generateUniqueTemporaryFileName("pdf")

                            if(exporter.write()) {
                                if(isPdfExport)
                                    pdfViewer.show("Screenplay", exporter.fileName, dlFileName, 2, exportSaveFeature.enabled)
                                else
                                    Scrite.app.revealFileOnDesktop(exporter.fileName)
                                modalDialog.close()
                            } else
                                busyOverlay.visible = false
                        })
                    }
                }
            }
        }
    }

    property ErrorReport exporterErrors: Aggregation.findErrorReport(exporter)
    Notification.title: exporter.formatName + " - Export"
    Notification.text: exporterErrors.errorMessage
    Notification.active: exporterErrors.hasError
    Notification.autoClose: false
    Notification.onDismissed: modalDialog.close()

    function loadFieldEditor(kind) {
        if(kind === "IntegerSpinBox")
            return editor_IntegerSpinBox
        if(kind === "CheckBox")
            return editor_CheckBox
        if(kind === "TextBox")
            return editor_TextBox
        return editor_Unknown
    }

    Component {
        id: editor_IntegerSpinBox

        Column {
            property var fieldInfo
            spacing: 10

            Text {
                text: fieldInfo.label
                width: parent.width
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
                font.pointSize: Scrite.app.idealFontPointSize
            }

            SpinBox {
                from: fieldInfo.min
                to: fieldInfo.max
                value: exporter ? exporter.getConfigurationValue(fieldInfo.name) : 0
                onValueModified: {
                    if(exporter)
                        exporter.setConfigurationValue(fieldInfo.name, value)
                }
                TabSequenceItem.manager: tabSequence
            }
        }
    }

    Component {
        id: editor_CheckBox

        Column {
            property var fieldInfo

            CheckBox2 {
                id: checkBox
                width: parent.width
                text: fieldInfo.label
                checkable: true
                checked: exporter ? exporter.getConfigurationValue(fieldInfo.name) : false
                onToggled: exporter ? exporter.setConfigurationValue(fieldInfo.name, checked) : false
                font.pointSize: Scrite.app.idealFontPointSize
                TabSequenceItem.manager: tabSequence
            }

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                leftPadding: 2*checkBox.leftPadding + checkBox.implicitIndicatorWidth
                text: fieldInfo.note
                font.pointSize: Scrite.app.idealFontPointSize-2
                color: primaryColors.c600.background
                visible: fieldInfo.note !== ""
            }
        }
    }

    Component {
        id: editor_TextBox

        Column {
            property var fieldInfo
            spacing: 5

            Text {
                text: fieldInfo.name
                font.capitalization: Font.Capitalize
                font.pointSize: Scrite.app.idealFontPointSize
            }

            TextField2 {
                width: parent.width-30
                label: ""
                placeholderText: fieldInfo.label
                onTextChanged: exporter.setConfigurationValue(fieldInfo.name, text)
                TabSequenceItem.manager: tabSequence
            }
        }
    }

    Component {
        id: editor_Unknown

        Text {
            property var fieldInfo
            textFormat: Text.RichText
            text: "Do not know how to configure <strong>" + fieldInfo.name + "</strong>"
        }
    }
}
