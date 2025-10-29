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
import "qrc:/qml/dialogs"
import "qrc:/qml/notifications"

VclDialog {
    id: root

    property AbstractReportGenerator report
    property string initialPage

    width: Math.min(Scrite.window.width*0.9, 800)
    height: Math.min(Scrite.window.height*0.9, 650)
    title: report ? report.title : "Report Configuration Dialog"

    content: report && visible ? (_private.reportEnabled ? reportConfigContent : reportFeatureDisabledContent) : null
    bottomBar: report && visible && _private.reportEnabled ? generateButtonBar : null

    // Show this component if the report feature is disabled for the current user
    Component {
        id: reportFeatureDisabledContent

        DisabledFeatureNotice {
            color: Qt.rgba(1,1,1,0.9)
            featureName: report.title
        }
    }

    // Show this component if the report is enabled for the current user
    Component {
        id: reportConfigContent

        ColumnLayout {
            spacing: 0

            VclLabel {
                Layout.fillWidth: true

                background: Rectangle {
                    color: Runtime.colors.accent.c600.background
                }

                color: Runtime.colors.accent.c600.text
                visible: text !== ""
                leftPadding: 16
                bottomPadding: 16
                wrapMode: Text.WordWrap

                text: report.description
            }

            PageView {
                id: reportConfigPageView
                Layout.fillWidth: true
                Layout.fillHeight: true
                pagesArray: _private.formInfo.groupedFields
                pageTitleRole: "name"
                pageListWidth: Math.max(width * 0.15, 150)
                currentIndex: {
                     if(root.initialPage !== "") {
                         const pa = pagesArray
                         for(var i=0; i<pa.length; i++) {
                             if(pa[i][pageTitleRole] === root.initialPage)
                                return i
                         }
                     }
                     return 0
                }
                pageListVisible: pagesArray && pagesArray.length > 1
                pageContent: ColumnLayout {
                    width: reportConfigPageView.availablePageContentWidth

                    Loader {
                        id: pageContentLoader

                        Layout.fillWidth: true
                        Layout.margins: 10
                        Layout.leftMargin: 0

                        sourceComponent: reportConfigPageView.currentIndex === 0 ? reportConfigPage0 : reportConfigPageN
                        onLoaded: item.fieldGroupIndex = reportConfigPageView.currentIndex
                    }

                    Connections {
                        target: reportConfigPageView

                        function onCurrentIndexChanged() {
                            pageContentLoader.active = false
                            Qt.callLater( () => { pageContentLoader.active = true } )
                        }
                    }
                }

                Component.onCompleted: {
                    if(Object.isOfType(report, "AbstractScreenplaySubsetReport")) {
                        report.capitalizeSentences = Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences
                        report.polishParagraphs = Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
                    }

                    if(_private.isPdfExport)
                        Runtime.showHelpTip("watermark")
                    Runtime.showHelpTip("reports")
                    Runtime.showHelpTip(Object.typeOf(report))
                }
            }
        }
    }

    // The first page in the PageView of reportConfigContent should show
    // FileSelector and other options from the report generator
    Component {
        id: reportConfigPage0

        ColumnLayout {
            property int fieldGroupIndex: -1

            spacing: 5

            FileSelector {
                id: fileSelector
                Layout.fillWidth: true

                label: "Select a file to export into"
                absoluteFilePath: report.fileName
                enabled: _private.reportSaveFeature.enabled
                allowedExtensions: [
                    {
                        "label": "Adobe PDF Format",
                        "suffix": "pdf",
                        "value": AbstractReportGenerator.AdobePDF,
                        "enabled": report.supportsFormat(AbstractReportGenerator.AdobePDF)
                    },
                    {
                        "label": "Open Document Format",
                        "suffix": "odt",
                        "value": AbstractReportGenerator.OpenDocumentFormat,
                        "enabled": report.supportsFormat(AbstractReportGenerator.OpenDocumentFormat) && _private.reportSaveFeature.enabled
                    }
                ]
                nameFilters: {
                    if(report.format === AbstractReportGenerator.AdobePDF)
                        return "Adobe PDF (*.pdf)"
                    return "Open Document Format (*.odt)"
                }

                onSelectedExtensionChanged: report.format = selectedExtension.value
                onAbsoluteFilePathChanged: report.fileName = absoluteFilePath

                Component.onCompleted: {
                    const aes = allowedExtensions
                    const idx = report.format === AbstractReportGenerator.AdobePDF ? 0 : 1
                    selectedExtension = aes[idx]
                }
            }

            VclLabel {
                Layout.fillWidth: true
                Layout.rightMargin: 20
                Layout.bottomMargin: 24

                visible: !_private.reportSaveFeature.enabled
                wrapMode: Text.WordWrap

                text: "<b>NOTE:</b> Your current subscription plan allows you to preview, but not save the PDF or generate ODT. Click <u>here</u> to know about your subscription plan and features."

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: UserAccountDialog.launch("Subscriptions")
                }
            }

            Repeater {
                model: _private.formInfo.groupedFields[0].fields
                delegate: fieldEditorLoader
            }
        }
    }

    // Every page there after in PageView of reportConfigContent should
    // show only options from the report generator
    Component {
        id: reportConfigPageN

        ColumnLayout {
            property int fieldGroupIndex: -1

            spacing: 5

            Repeater {
                model: fieldGroupIndex > 0 ? _private.formInfo.groupedFields[fieldGroupIndex].fields : []
                delegate: fieldEditorLoader
            }
        }
    }

    // Editor fields are instances of this component.
    Component {
        id: fieldEditorLoader

        Loader {
            id: fieldLoader
            required property var modelData

            Layout.fillWidth: true

            source: delegateChooser.delegateSource(modelData.editor)
            onLoaded: {
                item.report = report
                item.fieldInfo = modelData
                if(item.getReady)
                    item.getReady()
            }

            opacity: enabled ? 1 : 0.5
            enabled: {
                if(modelData.feature !== "") {
                    const afc = Qt.createQmlObject("import io.scrite.components 1.0; AppFeature { }", fieldLoader)
                    afc.featureName = modelData.feature
                    const ret = afc.enabled
                    afc.destroy()
                    return ret
                }
                return true
            }
        }
    }

    Component {
        id: generateButtonBar

        Item {
            id: footerItem
            height: footerLayout.height+20

            RowLayout {
                id: footerLayout
                width: parent.width-32
                anchors.centerIn: parent

                VclButton {
                    Layout.alignment: Qt.AlignRight
                    enabled: report.fileName !== "" && _private.reportEnabled
                    text: "Generate"
                    onClicked: generateReportJob.start()
                }
            }

            SequentialAnimation {
                id: generateReportJob

                running: false

                // Launch wait dialog ..
                ScriptAction {
                    script: {
                        Object.save(report)
                        _private.waitDialog = WaitDialog.launch("Generating " + report.title + " ...", Aggregation.progressReport(report))
                    }
                }

                // Wait for it to show up on the UI ...
                PauseAnimation {
                    duration: 200
                }

                // Perform the export job ...
                ScriptAction {
                    script: {
                        const dlFileName = report.fileName
                        if(_private.isPdfExport) {
                            report.fileName = Runtime.fileNamager.generateUniqueTemporaryFileName("pdf")
                            Runtime.fileNamager.addToAutoDeleteList(report.fileName)
                        }

                        const success = report.generate()

                        Qt.callLater(_private.waitDialog.close)
                        _private.waitDialog = null

                        if(success) {
                            if(_private.isPdfExport) {
                                PdfDialog.launch(report.title, report.fileName, dlFileName, report.singlePageReport ? 1 : 2, _private.reportSaveFeature.enabled)
                            } else
                                File.revealOnDesktop(report.fileName)
                            Qt.callLater(root.close)
                        } else {
                            const reportErrors = Aggregation.errorReport(report)
                            MessageBox.information(report.title, reportErrors.errorMessage, () => {
                                                       Qt.callLater(root.close)
                                                   } )
                        }
                    }
                }
            }
        }
    }

    // Factory function for loading delegates
    QtObject {
        id: delegateChooser

        readonly property var knownEditors: [
            "MultipleCharacterNameSelector",
            "MultipleLocationSelector",
            "MultipleSceneSelector",
            "MultipleEpisodeSelector",
            "MultipleTagGroupSelector",
            "CheckBox",
            "EnumSelector",
            "TextBox",
            "IntegerSpinBox",
            "TwoColumnLayoutSelector",
            "TwoColumnWidthDistributionEditor"
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

        property var formInfo: report ? report.configurationFormInfo() : {"title": "Unknown", "description": "", "groupedFields": []}
        property bool isPdfExport: report ? report.format === AbstractReportGenerator.AdobePDF : false
        property bool reportEnabled: report ? report.featureEnabled : false

        property AppFeature reportSaveFeature: AppFeature {
            featureName: report ? "report/" + report.title.toLowerCase() + "/save" : "report"
        }

        property VclDialog waitDialog
    }

    onClosed: Runtime.execLater(report, 100, report.discard)

    Connections {
        target: root.report

        function onAboutToDelete() {
            root.report = null
        }
    }
}
