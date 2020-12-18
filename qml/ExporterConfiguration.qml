/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import Scrite 1.0
import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Controls 2.13

Item {
    id: configurationBox
    property AbstractExporter exporter
    property var formInfo: exporter ? exporter.configurationFormInfo() : {"title": "Unknown", "fields": []}

    width: 700
    height: exporter && (exporter.requiresConfiguration || exporter.canBundleFonts) ?
            Math.min(documentUI.height*0.9, contentLoader.item.idealHeight) : 300

    Component.onCompleted: {
        exporter = scriteDocument.createExporter(modalDialog.arguments)
        if(exporter === null) {
            modalDialog.closeable = true
            var exportKind = modalDialog.arguments.split("/").last()
            notice.text = exportKind + " Export"
        } else if(app.verifyType(exporter, "StructureExporter"))
            mainTabBar.currentIndex = 1 // FIXME: Ugly hack to ensure that structure tab is active for StructureExporter.
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

                property bool scrollBarRequired: formView.height < formView.contentHeight
                ScrollBar.vertical.policy: formView.scrollBarRequired ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

                Column {
                    width: formView.width - (formView.scrollBarRequired ? 17 : 0)
                    spacing: 10

                    Text {
                        font.pointSize: Screen.devicePixelRatio > 1 ? 24 : 20
                        font.bold: true
                        text: exporter.formatName + " - Export"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    FileSelector {
                        id: fileSelector
                        width: parent.width-20
                        label: "Select a file to export into"
                        absoluteFilePath: exporter.fileName
                        onAbsoluteFilePathChanged: exporter.fileName = absoluteFilePath
                        nameFilters: exporter.nameFilters
                        folder: workspaceSettings.lastOpenExportFolderUrl
                        onFolderChanged: workspaceSettings.lastOpenExportFolderUrl = folder
                    }

                    Loader {
                        width: parent.width
                        active: exporter.canBundleFonts
                        sourceComponent: GroupBox {
                            width: parent.width
                            label: Text {
                                text: "Export fonts for the following languages"
                            }
                            height: languageBundleView.height+45

                            Grid {
                                id: languageBundleView
                                width: parent.width-10
                                anchors.top: parent.top
                                spacing: 5
                                columns: 3

                                Repeater {
                                    model: app.enumerationModel(app.transliterationEngine, "Language")
                                    delegate: CheckBox2 {
                                        width: languageBundleView.width/languageBundleView.columns
                                        checkable: true
                                        checked: exporter.isFontForLanguageBundled(modelData.value)
                                        text: modelData.key
                                        onToggled: exporter.bundleFontForLanguage(modelData.value,checked)
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

                    EventFilter.target: app
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
                    enabled: fileSelector.absoluteFilePath !== ""
                    text: "Export"
                    onClicked: {
                        if(exporter.write()) {
                            app.revealFileOnDesktop(exporter.fileName)
                            modalDialog.close()
                        }
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
            }

            SpinBox {
                from: fieldInfo.min
                to: fieldInfo.max
                value: exporter ? exporter.getConfigurationValue(fieldInfo.name) : 0
                onValueModified: {
                    if(exporter)
                        exporter.setConfigurationValue(fieldInfo.name, value)
                }
            }
        }
    }

    Component {
        id: editor_CheckBox

        CheckBox2 {
            property var fieldInfo
            text: fieldInfo.label
            checkable: true
            checked: exporter ? exporter.getConfigurationValue(fieldInfo.name) : false
            onToggled: exporter ? exporter.setConfigurationValue(fieldInfo.name, checked) : false
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
            }

            TextField2 {
                width: parent.width-30
                placeholderText: fieldInfo.label
                onTextChanged: exporter.setConfigurationValue(fieldInfo.name, text)
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
