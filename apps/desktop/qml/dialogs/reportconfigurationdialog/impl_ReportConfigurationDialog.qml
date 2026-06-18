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
import ".."
import "../../notifications"

VclDialog {
    id: root

    property AbstractReportGenerator report
    property string initialPage

    width: Math.min(Scrite.window.width*0.9, 800)
    height: Math.min(Scrite.window.height*0.9, 650)

    handleLanguageShortcuts: true
    title: report ? report.title : "Report Configuration Dialog"

    content: report && visible ? (_private.reportEnabled ? _reportConfigContent : _reportFeatureDisabledContent) : null
    bottomBar: report && visible && _private.reportEnabled ? _generateButtonBar : null

    // Show this component if the report feature is disabled for the current user
    Component {
        id: _reportFeatureDisabledContent

        DisabledFeatureNotice {
            featureName: root.report.title
        }
    }

    // Show this component if the report is enabled for the current user
    Component {
        id: _reportConfigContent

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

                text: root.report.description
            }

            PageView {
                id: _reportConfigPageView

                Layout.fillWidth: true
                Layout.fillHeight: true

                pagesArray: _private.configuration.groups
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
                    width: _reportConfigPageView.availablePageContentWidth

                    Loader {
                        id: _pageContentLoader

                        Layout.fillWidth: true
                        Layout.margins: 10
                        Layout.leftMargin: 0

                        sourceComponent: _reportConfigPageView.currentIndex === 0 ? _reportConfigPage0 : _reportConfigPageN
                        onLoaded: item.fieldGroupIndex = _reportConfigPageView.currentIndex
                    }

                    Connections {
                        target: _reportConfigPageView

                        function onCurrentIndexChanged() {
                            _pageContentLoader.active = false
                            Qt.callLater( () => { _pageContentLoader.active = true } )
                        }
                    }
                }

                Component.onCompleted: {
                    if(Object.isOfType(root.report, "AbstractScreenplaySubsetReport")) {
                        root.report.capitalizeSentences = Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences
                        root.report.polishParagraphs = Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
                    }

                    if(_private.isPdfExport)
                        Runtime.showHelpTip("watermark")
                    Runtime.showHelpTip("reports")
                    Runtime.showHelpTip(Object.typeOf(root.report))
                }
            }
        }
    }

    // The first page in the PageView of reportConfigContent should show
    // FileSelector and other options from the report generator
    Component {
        id: _reportConfigPage0

        ColumnLayout {
            property int fieldGroupIndex: -1

            spacing: 5

            FileSelector {
                id: _fileSelector
                Layout.fillWidth: true

                label: "Select a file to export into"
                absoluteFilePath: root.report.fileName
                enabled: _private.reportSaveFeature.enabled
                allowedExtensions: [
                    {
                        "label": root.report.formatDescription(AbstractReportGenerator.PdfFormat),
                        "suffix": root.report.formatFileExtension(AbstractReportGenerator.PdfFormat),
                        "value": AbstractReportGenerator.PdfFormat,
                        "enabled": root.report.supportsFormat(AbstractReportGenerator.PdfFormat)
                    },
                    {
                        "label": root.report.formatDescription(AbstractReportGenerator.OpenDocumentFormat),
                        "suffix": root.report.formatFileExtension(AbstractReportGenerator.OpenDocumentFormat),
                        "value": AbstractReportGenerator.OpenDocumentFormat,
                        "enabled": root.report.supportsFormat(AbstractReportGenerator.OpenDocumentFormat) && _private.reportSaveFeature.enabled
                    }
                ]
                nameFilters: {
                    const ae = allowedExtensions
                    if(root.report.format === AbstractReportGenerator.PdfFormat)
                        return ae[0].label + " (*." + ae[0].suffix + ")"
                    return ae[1].label + " (*." + ae[1].suffix + ")"
                }

                onSelectedExtensionChanged: root.report.format = _fileSelector.selectedExtension.value
                onAbsoluteFilePathChanged: root.report.fileName = _fileSelector.absoluteFilePath

                Component.onCompleted: {
                    const aes = _fileSelector.allowedExtensions
                    const idx = root.report.format === AbstractReportGenerator.PdfFormat ? 0 : 1
                    _fileSelector.selectedExtension = aes[idx]
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
                model: _private.configuration.groups[0].fields
                delegate: _fieldEditorLoader
            }
        }
    }

    // Every page there after in PageView of reportConfigContent should
    // show only options from the report generator
    Component {
        id: _reportConfigPageN

        ColumnLayout {
            id: _reportConfigPageNLayout
            property int fieldGroupIndex: -1

            spacing: 5

            Repeater {
                model: _reportConfigPageNLayout.fieldGroupIndex > 0 && _private.configuration.groups[_reportConfigPageNLayout.fieldGroupIndex] ? _private.configuration.groups[_reportConfigPageNLayout.fieldGroupIndex].fields : []
                delegate: _fieldEditorLoader
            }
        }
    }

    // Editor fields are instances of this component.
    Component {
        id: _fieldEditorLoader

        Loader {
            id: _fieldLoader

            required property int index
            required property var modelData

            Layout.fillWidth: true

            source: _delegateChooser.delegateSource(_fieldLoader.modelData.editor)
            onLoaded: {
                item.report = root.report
                item.fieldInfo = _fieldLoader.modelData
                if(item.getReady)
                    item.getReady()
            }

            opacity: enabled ? 1 : 0.5
            enabled: {
                if(_fieldLoader.modelData.feature !== "") {
                    let afcObject = Qt.createQmlObject("import io.scrite.components; AppFeature { }", _fieldLoader)
                    let afc = afcObject as AppFeature
                    afc.featureName = _fieldLoader.modelData.feature
                    const ret = afc.enabled
                    afc.destroy()
                    return ret
                }
                return true
            }
        }
    }

    Component {
        id: _generateButtonBar

        Item {
            id: _footerItem

            height: _footerLayout.height+20

            RowLayout {
                id: _footerLayout

                width: parent.width-32
                anchors.centerIn: parent

                VclButton {
                    Layout.alignment: Qt.AlignRight
                    enabled: root.report.fileName !== "" && _private.reportEnabled
                    text: "Generate"
                    onClicked: _generateReportJob.start()

                    ActionHandler {
                        action: root.acceptAction

                        onTriggered: _generateReportJob.start()
                    }
                }
            }

            SequentialAnimation {
                id: _generateReportJob

                running: false

                // Launch wait dialog ..
                ScriptAction {
                    script: {
                        Object.save(root.report)
                        _private.waitDialog = WaitDialog.launch("Generating " + root.report.title + " ...", Aggregation.progressReport(root.report))
                    }
                }

                // Wait for it to show up on the UI ...
                PauseAnimation {
                    duration: 200
                }

                // Perform the export job ...
                ScriptAction {
                    script: {
                        if(_private.shouldPersonalizeFileName())
                            root.report.personalizeFileName()

                        const dlFileName = root.report.fileName
                        if(_private.isPdfExport) {
                            root.report.fileName = Runtime.fileNamager.generateUniqueTemporaryFileName("pdf")
                            Runtime.fileNamager.addToAutoDeleteList(root.report.fileName)
                        }

                        const success = root.report.generate()

                        Qt.callLater(_private.waitDialog.close)
                        _private.waitDialog = null

                        if(success) {
                            if(_private.isPdfExport) {
                                PdfDialog.launch(root.report.title, root.report.fileName, dlFileName, root.report.singlePageReport ? 1 : 2, _private.reportSaveFeature.enabled)
                            } else
                                File.revealOnDesktop(root.report.fileName)
                            Qt.callLater(root.close)
                        } else {
                            const reportErrors = Aggregation.errorReport(root.report)
                            MessageBox.information(root.report.title, reportErrors.errorMessage, root.close)
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
            "CheckBox",
            "EnumSelector",
            "IntegerSpinBox",
            "MultipleCharacterNameSelector",
            "MultipleEpisodeSelector",
            "MultipleKeywordsSelector",
            "MultipleLocationSelector",
            "MultipleSceneSelector",
            "MultipleTagGroupSelector",
            "TextBox",
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

        property string initialFileName: ""
        property var configuration: root.report ? root.report.configuration() : {"title": "Unknown", "description": "", "groups": []}
        property bool isPdfExport: root.report ? root.report.format === AbstractReportGenerator.PdfFormat : false
        property bool reportEnabled: root.report ? root.report.featureEnabled : false

        property AppFeature reportSaveFeature: AppFeature {
            featureName: root.report ? "report/" + root.report.title.toLowerCase() + "/save" : "report"
        }

        property VclDialog waitDialog

        function shouldPersonalizeFileName() {
            if(!root.report || root.report.fileName === "")
                return false
            const currentBase = _private.fileBaseName(root.report.fileName)
            const initialBase = _private.fileBaseName(_private.initialFileName)
            return currentBase === initialBase
        }

        function fileBaseName(path) {
            const slashIdx = Math.max(path.lastIndexOf('/'), path.lastIndexOf('\\'))
            const name = slashIdx >= 0 ? path.substring(slashIdx + 1) : path
            const dotIdx = name.lastIndexOf('.')
            return dotIdx >= 0 ? name.substring(0, dotIdx) : name
        }
    }

    Component.onCompleted: {
        _private.initialFileName = root.report ? root.report.fileName : ""
    }

    onClosed: Runtime.execLater(root, 100, () => { if(root.report) root.report.discard() })

    Connections {
        target: root.report

        function onAboutToDelete() {
            root.report = null
        }
    }
}
