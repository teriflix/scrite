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
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.13

Item {
    id: configurationBox
    property AbstractReportGenerator generator
    property var formInfo: generator ? generator.configurationFormInfo() : {"title": "Unknown", "fields": []}

    width: 700
    height: formInfo.fields.length > 0 ? 700 : 275

    Component.onCompleted: {
        generator = scriteDocument.createReportGenerator(modalDialog.arguments)
        if(generator === null) {
            modalDialog.closeable = true
            notice.text = "Report generator for '" + modalDialog.arguments + "' could not be created."
        }
        modalDialog.arguments = undefined
    }

    Text {
        id: notice
        anchors.centerIn: parent
        visible: generator === null
        font.pixelSize: 20
        width: parent.width*0.85
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        horizontalAlignment: Text.AlignHCenter
    }

    Loader {
        anchors.fill: parent
        active: generator
        sourceComponent: Item {
            FileDialog {
                id: filePathDialog
                folder: {
                    if(scriteDocument.fileName !== "") {
                        var fileInfo = app.fileInfo(scriteDocument.fileName)
                        if(fileInfo.exists)
                            return "file:///" + fileInfo.absolutePath
                    }
                    return "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
                }
                selectFolder: false
                selectMultiple: false
                selectExisting: false
                nameFilters: {
                    if(generator.format === AbstractReportGenerator.AdobePDF)
                        return "Adobe PDF (*.pdf)"
                    return "Open Document Format (*.odt)"
                }
                onAccepted: generator.fileName = app.urlToLocalFile(fileUrl)
            }

            ScrollView {
                id: formView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: buttonRow.top
                anchors.margins: 20
                anchors.bottomMargin: 10

                property bool scrollBarRequired: formView.height < formView.contentHeight
                ScrollBar.vertical.policy: formView.scrollBarRequired ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                ScrollBar.vertical.opacity: ScrollBar.vertical.active ? 1 : 0.2

                Column {
                    width: formView.width - (formView.scrollBarRequired ? 17 : 0)
                    spacing: 10

                    Text {
                        font.pointSize: 24
                        font.bold: true
                        text: formInfo.title
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Column {
                        width: parent.width
                        spacing: parent.spacing/2

                        Text {
                            width: parent.width
                            text: "Select a file to export into"
                        }

                        Row {
                            width: parent.width
                            spacing: parent.spacing

                            TextField {
                                id: filePathField
                                readOnly: true
                                width: parent.width - filePathDialogButton.width - parent.spacing
                                text: generator.fileName
                            }

                            ToolButton2 {
                                id: filePathDialogButton
                                text: "..."
                                suggestedWidth: 35
                                suggestedHeight: 35
                                onClicked: filePathDialog.open()
                                hoverEnabled: false
                            }
                        }

                        Row {
                            spacing: 20

                            RadioButton {
                                text: "Adobe PDF Format"
                                checked: generator.format === AbstractReportGenerator.AdobePDF
                                onClicked: generator.format = AbstractReportGenerator.AdobePDF
                            }

                            RadioButton {
                                text: "Open Document Format"
                                checked: generator.format === AbstractReportGenerator.OpenDocumentFormat
                                onClicked: generator.format = AbstractReportGenerator.OpenDocumentFormat
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
                                if(item) {
                                    item.fieldName = modelData.name
                                    item.fieldTitle = modelData.label
                                }
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

                Button {
                    text: "Cancel"
                    onClicked: {
                        generator.discard()
                        modalDialog.close()
                    }

                    EventFilter.target: app
                    EventFilter.events: [6]
                    EventFilter.onFilter: {
                        if(event.key === Qt.Key_Escape) {
                            result.acceptEvent = true
                            result.filter = true
                            generator.discard()
                            modalDialog.close()
                        }
                    }
                }

                Button {
                    enabled: filePathField.text !== ""
                    text: "Generate"
                    onClicked: {
                        if(generator.generate())
                            app.revealFileOnDesktop(generator.fileName)
                        modalDialog.close()
                    }
                }
            }
        }
    }

    function loadFieldEditor(kind) {
        if(kind === "MultipleCharacterNameSelector")
            return editor_MultipleCharacterNameSelector
        if(kind === "CheckBox")
            return editor_CheckBox
        return editor_Unknown
    }

    Component {
        id: editor_MultipleCharacterNameSelector

        Item {
            property string fieldName
            property string fieldTitle
            property var characterNames: []
            height: 300
            onCharacterNamesChanged: generator.setConfigurationValue(fieldName, characterNames)

            Text {
                id: fieldTitleText
                width: parent.width
                text: fieldTitle + ": " + (characterNames.length > 0 ? characterNames.join(', ') : '')
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Rectangle {
                anchors.fill: characterNameListView
                anchors.margins: -1
                border { width: 1; color: "black" }
            }

            ListView {
                id: characterNameListView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: fieldTitleText.bottom
                anchors.topMargin: 10
                anchors.bottom: parent.bottom
                anchors.rightMargin: 30
                anchors.leftMargin: 5
                clip: true
                model: scriteDocument.structure.characterNames
                spacing: 5
                currentIndex: -1
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    opacity: active ? 1 : 0.2
                    Behavior on opacity { NumberAnimation { duration: 250 } }
                }
                delegate: Item {
                    width: characterNameListView.width
                    height: 35

                    CheckBox {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 5
                        font.pixelSize: 15
                        text: modelData
                        onToggled: {
                            var names = characterNames
                            if(checked)
                                names.push(modelData)
                            else
                                names.splice(names.indexOf(modelData),1)
                            characterNames = names
                        }
                    }
                }
            }
        }
    }

    Component {
        id: editor_CheckBox

        CheckBox {
            property string fieldName
            property string fieldTitle
            text: fieldTitle
            checkable: true
            checked: generator ? generator.getConfigurationValue(fieldName) : false
            onToggled: generator ? generator.setConfigurationValue(fieldName, checked) : false
        }
    }

    Component {
        id: editor_Unknown

        Text {
            property string fieldName
            property string fieldTitle
            textFormat: Text.RichText
            text: "Do not know how to configure <strong>" + fieldName + "</strong>"
        }
    }
}
