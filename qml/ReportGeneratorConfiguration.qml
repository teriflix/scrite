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
    property var formInfo: {"title": "Unknown", "fields": []}

    width: 700
    height: formInfo.fields.length > 0 ? 700 : 275

    Component.onCompleted: {
        var reportName = typeof modalDialog.arguments === "string" ? modalDialog.arguments : modalDialog.arguments.reportName
        generator = scriteDocument.createReportGenerator(reportName)
        if(generator === null) {
            modalDialog.closeable = true
            notice.text = "Report generator for '" + JSON.stringify(modalDialog.arguments) + "' could not be created."
        } else if(typeof modalDialog.arguments !== "string") {
            var config = modalDialog.arguments.configuration
            for(var member in config)
                generator.setConfigurationValue(member, config[member])
            formInfo = generator.configurationFormInfo()
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

                            RadioButton2 {
                                text: "Adobe PDF Format"
                                checked: generator.format === AbstractReportGenerator.AdobePDF
                                onClicked: generator.format = AbstractReportGenerator.AdobePDF
                            }

                            RadioButton2 {
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

                Button2 {
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

                Button2 {
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
            property alias characterNames: characterNameListView.selectedCharacters
            onCharacterNamesChanged: generator.setConfigurationValue(fieldName, characterNames)
            height: characterNameListView.visible ? 300 : fieldTitleText.height

            onFieldNameChanged: {
                characterNameListView.selectedCharacters = generator.getConfigurationValue(fieldName)
                characterNameListView.visible = characterNameListView.selectedCharacters.length === 0
                console.log(fieldName + ", " + generator.getConfigurationValue(fieldName))
            }

            Loader {
                id: fieldTitleText
                width: parent.width
                sourceComponent: Flow {
                    spacing: 5
                    flow: Flow.LeftToRight

                    Text {
                        id: sceneCharactersListHeading
                        text: fieldTitle + ": "
                        font.bold: true
                        font.pointSize: characterNameListView.visible ? 12 : 15
                        topPadding: 5
                        bottomPadding: 5
                    }

                    Repeater {
                        model: characterNames

                        TagText {
                            property var colors: accentColors.c600
                            border.width: 1
                            border.color: colors.text
                            color: colors.background
                            textColor: colors.text
                            text: modelData
                            leftPadding: 10
                            rightPadding: 10
                            topPadding: 5
                            bottomPadding: 5
                            font.pointSize: characterNameListView.visible ? 12 : 15
                        }
                    }

                    Image {
                        source: "../icons/content/add_box.png"
                        width: sceneCharactersListHeading.height
                        height: sceneCharactersListHeading.height
                        opacity: 0.5
                        visible: !characterNameListView.visible

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                            onClicked: characterNameListView.visible = true
                        }
                    }
                }
            }

            CharactersView {
                id: characterNameListView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: fieldTitleText.bottom
                anchors.topMargin: 10
                anchors.bottom: parent.bottom
                anchors.rightMargin: 30
                anchors.leftMargin: 5
                charactersModel.array: charactersModel.stringListArray(scriteDocument.structure.characterNames)
            }
        }
    }

    Component {
        id: editor_CheckBox

        CheckBox2 {
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
