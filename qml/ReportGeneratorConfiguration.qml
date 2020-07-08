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
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

Item {
    id: configurationBox
    property AbstractReportGenerator generator
    property var formInfo: {"title": "Unknown", "groupedFields": []}

    width: 700
    height: formInfo.fields.length > 0 ? 700 : 275
    readonly property color dialogColor: primaryColors.c300.background

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
            Column {
                id: formTitle
                width: parent.width
                spacing: 10
                y: 20

                Text {
                    font.pointSize: 24
                    font.bold: true
                    text: formInfo.title
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: primaryColors.c300.text
                }

                Text {
                    text: "Configure options for your report using the options in the tabs below."
                    width: parent.width * 0.7
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: primaryColors.c300.text
                }
            }

            Item {
                id: formOptionsArea
                anchors.top: formTitle.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: buttonRow.top
                anchors.topMargin: 15
                anchors.bottomMargin: 10

                Row {
                    id: tabBar
                    anchors.top: parent.top
                    anchors.left: parent.left
                    property int currentIndex: 0

                    Repeater {
                        id: tabRepeater
                        model: formInfo.groupedFields

                        Rectangle {
                            width: tabText.contentWidth + 40
                            height: tabText.contentHeight + 30
                            color: selected ? "white" : Qt.rgba(0,0,0,0)
                            property bool selected: tabBar.currentIndex === index

                            Rectangle {
                                height: 4
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: accentColors.c500.background
                                visible: parent.selected
                            }

                            Text {
                                id: tabText
                                anchors.centerIn: parent
                                font.pixelSize: 16
                                text: modelData.name
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: tabBar.currentIndex = index
                            }
                        }
                    }
                }

                Rectangle {
                    id: contentPanel
                    anchors.top: tabBar.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    clip: true

                    StackLayout {
                        currentIndex: tabBar.currentIndex

                        Item {
                            id: firstTab
                            implicitWidth: contentPanel.width
                            implicitHeight: contentPanel.height

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
                                id: firstTabScrollView
                                anchors.fill: parent
                                anchors.leftMargin: 20

                                Column {
                                    spacing: 10
                                    width: firstTabScrollView.width

                                    Item { width: parent.width; height: 10 }

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
                                        model: formInfo.groupedFields[0].fields

                                        Loader {
                                            width: parent.width
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
                        }

                        Repeater {
                            model: formInfo.groupedFields.length-1

                            Item {
                                id: subsequentTab
                                implicitWidth: contentPanel.width
                                implicitHeight: contentPanel.height

                                ScrollView {
                                    id: subsequenTabScrollView
                                    anchors.fill: parent
                                    anchors.leftMargin: 20

                                    Column {
                                        spacing: 5
                                        width: subsequenTabScrollView.width

                                        Item { width: parent.width; height: 10 }

                                        Repeater {
                                            model: formInfo.groupedFields[index+1].fields

                                            Loader {
                                                width: parent.width
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
                            }
                        }
                    }
                }
            }

            Row {
                id: buttonRow
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 20
                anchors.bottomMargin: 20
                spacing: 20

                Button2 {
                    text: "Cancel"
                    Material.background: primaryColors.c100.background
                    Material.foreground: primaryColors.c100.text
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
                    Material.background: primaryColors.c100.background
                    Material.foreground: primaryColors.c100.text

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
        if(kind === "MultipleLocationSelector")
            return editor_MultipleLocationSelector
        if(kind === "MultipleSceneSelector")
            return editor_MultipleSceneSelector
        if(kind === "CheckBox")
            return editor_CheckBox
        if(kind === "EnumSelector")
            return editor_EnumSelector
        if(kind === "TextBox")
            return editor_TextBox
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
            height: fieldTitleText.height + (characterNameListView.visible ? 275 : 0)

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
        id: editor_MultipleLocationSelector

        Item {
            property var fieldInfo
            property var allLocations: scriteDocument.structure.allLocations()
            property var selectedLocations: []

            height: multipleLocationSelectorLayout.height

            Column {
                id: multipleLocationSelectorLayout
                width: parent.width-20
                spacing: 5

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: fieldInfo.label
                }

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.italic: true
                    text: "(Double click on an item in one list, to add a it to the other.)"
                }

                Row {
                    width: parent.width - 5
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: 350
                    spacing: 10

                    ScrollView {
                        width: (parent.width - addRemoveButtons.width - parent.spacing*2)/2
                        height: parent.height
                        background: Rectangle {
                            color: primaryColors.c100.background
                            border.width: 1
                            border.color: primaryColors.c100.text
                        }
                        ListView {
                            id: allLocationsView
                            model: allLocations
                            clip: true
                            property string currentLocation: currentIndex >= 0 ? allLocations[currentIndex] : ""
                            delegate: Item {
                                width: allLocationsView.width-1
                                height: 25

                                Text {
                                    id: allLocatonText
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                    text: modelData
                                    color: selectedLocations.indexOf(modelData) >= 0 ? "gray" : "black"
                                }

                                ToolTip.visible: allLocatonTextMouseArea.containsMouse && (allLocatonText.contentWidth > allLocatonText.width)
                                ToolTip.text: modelData

                                MouseArea {
                                    id: allLocatonTextMouseArea
                                    hoverEnabled: true
                                    anchors.fill: parent
                                    onClicked: allLocationsView.currentIndex = index
                                    onDoubleClicked: addOneButton.click()
                                }
                            }
                            highlight: Rectangle { color: accentColors.windowColor }
                            highlightFollowsCurrentItem: true
                            highlightResizeDuration: 0
                            highlightMoveDuration: 0
                        }
                    }

                    Column {
                        id: addRemoveButtons
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        ToolButton3 {
                            id: addOneButton
                            iconSource: "../icons/action/add_one.png"
                            enabled: selectedLocations.indexOf(allLocationsView.currentLocation) < 0
                            onClicked: {
                                var locs = selectedLocations
                                locs.push( allLocationsView.currentLocation )
                                selectedLocations = locs
                                generator.setConfigurationValue(fieldInfo.name, locs)
                            }
                        }

                        ToolButton3 {
                            id: removeOneButton
                            iconSource: "../icons/action/remove_one.png"
                            enabled: selectedLocationsView.currentIndex >= 0
                            onClicked: {
                                var locs = selectedLocations
                                locs.splice(selectedLocationsView.currentIndex,1)
                                selectedLocations = locs
                                generator.setConfigurationValue(fieldInfo.name, locs)
                            }
                        }

                        ToolButton3 {
                            id: addAllButton
                            iconSource: "../icons/action/add_all.png"
                            enabled: selectedLocationsView.count !== allLocationsView.count
                        }

                        ToolButton3 {
                            id: removeAllButton
                            iconSource: "../icons/action/remove_all.png"
                            enabled: selectedLocationsView.count > 0
                        }
                    }

                    ScrollView {
                        width: (parent.width - addRemoveButtons.width - parent.spacing*2)/2
                        height: parent.height
                        background: Rectangle {
                            color: primaryColors.c100.background
                            border.width: 1
                            border.color: primaryColors.c100.text
                        }

                        ListView {
                            id: selectedLocationsView
                            anchors.fill: parent
                            anchors.margins: 2
                            clip: true
                            model: selectedLocations
                            property string currentLocation: currentIndex >= 0 ? selectedLocations[currentIndex] : ""
                            delegate: Item {
                                width: selectedLocationsView.width-1
                                height: 25

                                Text {
                                    id: selectedLocationText
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                    text: modelData
                                }

                                ToolTip.visible: selectedLocationTextMouseArea.containsMouse && (selectedLocationText.contentWidth > selectedLocationText.width)
                                ToolTip.text: modelData

                                MouseArea {
                                    id: selectedLocationTextMouseArea
                                    hoverEnabled: true
                                    anchors.fill: parent
                                    onClicked: selectedLocationsView.currentIndex = index
                                    onDoubleClicked: removeOneButton.click()
                                }
                            }
                            highlight: Rectangle { color: accentColors.windowColor }
                            highlightFollowsCurrentItem: true
                            highlightResizeDuration: 0
                            highlightMoveDuration: 0
                        }
                    }
                }
            }
        }
    }

    Component {
        id: editor_MultipleSceneSelector

        Column {
            property var fieldInfo
            spacing: 5

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: fieldInfo.label
            }

            Text {
                font.italic: true
                wrapMode: Text.WordWrap
                text: "(Select one or more locations from the list below)"
            }

            ScrollView {
                width: parent.width - 20
                height: 350
                background: Rectangle {
                    color: primaryColors.c100.background
                    border.width: 1
                    border.color: primaryColors.c100.text
                }
                ListView {
                    id: sceneListView
                    model: scriteDocument.screenplay
                    clip: true
                    delegate: CheckBox2 {
                        width: sceneListView.width-1
                        font.family: scriteDocument.formatting.defaultFont.family
                        text: {
                            var scene = screenplayElement.scene
                            if(scene && scene.heading.enabled)
                                return "[" + screenplayElement.sceneNumber + "] " + (scene && scene.heading.enabled ? scene.heading.text : "")
                            return "NO SCENE HEADING"
                        }
                        enabled: screenplayElement.scene && screenplayElement.scene.heading.enabled
                        onToggled: {
                            var numbers = generator.getConfigurationValue(fieldInfo.name)
                            if(checked)
                                numbers.push(screenplayElement.sceneNumber)
                            else {
                                var idx = numbers.indexOf(screenplayElement.sceneNumber)
                                numbers.splice(idx, 1)
                            }
                            generator.setConfigurationValue(fieldInfo.name, numbers)
                        }
                    }
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
        id: editor_TextBox

        Column {
            property var fieldInfo
            spacing: 5

            Text {
                text: fieldInfo.name
                font.capitalization: Font.Capitalize
            }

            TextField2 {
                width: parent.width - 30
                placeholderText: fieldInfo.label
                onTextChanged: {
                    if(generator)
                        generator.setConfigurationValue(fieldInfo.name, text)
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
