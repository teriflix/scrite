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
        }

        if(generator !== null)
            formInfo = generator.configurationFormInfo()

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

            Rectangle {
                anchors.fill: formView
                anchors.margins: -3
                visible: formView.contentHeight > formView.height
                color: primaryColors.c10.background
                border.width: 1
                border.color: primaryColors.borderColor
            }

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
                                enabled: generator.supportsFormat(AbstractReportGenerator.AdobePDF)
                            }

                            RadioButton2 {
                                text: "Open Document Format"
                                checked: generator.format === AbstractReportGenerator.OpenDocumentFormat
                                onClicked: generator.format = AbstractReportGenerator.OpenDocumentFormat
                                enabled: generator.supportsFormat(AbstractReportGenerator.OpenDocumentFormat)
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

                    Item {
                        width: parent.width
                        height: 40
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
        if(kind === "EnumSelector")
            return editor_EnumSelector
        return editor_Unknown
    }

    Component {
        id: editor_MultipleCharacterNameSelector

        Item {
            property var fieldInfo
            property alias characterNames: characterNameListView.selectedCharacters
            onCharacterNamesChanged: {
                if(fieldInfo)
                    generator.setConfigurationValue(fieldInfo.name, characterNames)
            }
            height: fieldTitleText.height + (characterNameListView.visible ? 300 : 0)

            onFieldInfoChanged: {
                characterNameListView.selectedCharacters = generator.getConfigurationValue(fieldInfo.name)
                characterNameListView.visible = characterNameListView.selectedCharacters.length === 0
            }

            Loader {
                id: fieldTitleText
                width: parent.width-30
                sourceComponent: Flow {
                    spacing: 5
                    flow: Flow.LeftToRight

                    Text {
                        id: sceneCharactersListHeading
                        text: fieldInfo.label + ": "
                        font.bold: true
                        font.pointSize: 12
                        topPadding: 2
                        bottomPadding: 2
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
                            topPadding: 2
                            bottomPadding: 2
                            font.pointSize: 12
                            closable: true
                            onCloseRequest: {
                                var list = characterNameListView.selectedCharacters
                                list.splice( list.indexOf(text), 1 )
                                characterNameListView.selectedCharacters = list
                            }
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
                            ToolTip.visible: containsMouse
                            ToolTip.text: "Add another character."
                            ToolTip.delay: 1000
                        }
                    }

                    Image {
                        source: "../icons/content/clear_all.png"
                        width: sceneCharactersListHeading.height
                        height: sceneCharactersListHeading.height
                        opacity: 0.5
                        visible: characterNameListView.selectedCharacters.length > 0

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                            onClicked: {
                                characterNameListView.selectedCharacters = []
                                characterNameListView.visible = true
                            }
                            ToolTip.visible: containsMouse
                            ToolTip.text: "Remove all characters and start fresh."
                            ToolTip.delay: 1000
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
            property var fieldInfo
            text: fieldInfo.label
            checkable: true
            checked: generator ? generator.getConfigurationValue(fieldInfo.name) : false
            onToggled: generator ? generator.setConfigurationValue(fieldInfo.name, checked) : false
        }
    }

    Component {
        id: editor_EnumSelector

        Column {
            property var fieldInfo
            spacing: 5

            Text {
                text: fieldInfo.label + ": "
                width: parent.width - 30
            }

            ComboBox2 {
                model: fieldInfo.choices
                textRole: "key"
                width: parent.width - 30
                onCurrentIndexChanged: {
                    if(generator)
                        generator.setConfigurationValue(fieldInfo.name, fieldInfo.choices[currentIndex].value)
                }
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
