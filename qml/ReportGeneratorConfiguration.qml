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
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

Item {
    id: configurationBox
    property AbstractReportGenerator generator
    property var formInfo: {"title": "Unknown", "description": "", "groupedFields": []}

    width: 750
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
                    font.pointSize: Screen.devicePixelRatio > 1 ? 24 : 20
                    font.bold: true
                    text: formInfo.title
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: primaryColors.c300.text
                }

                Text {
                    text: formInfo.description
                    font.pointSize:  Screen.devicePixelRatio > 1 ? 14 : 10
                    width: parent.width * 0.9
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

                Rectangle {
                    id: pageList
                    width: 175
                    height: parent.height
                    color: primaryColors.c700.background
                    property int currentIndex: 0
                    visible: pageRepeater.count > 1

                    Column {
                        width: parent.width

                        Repeater {
                            id: pageRepeater
                            model: formInfo.groupedFields

                            Rectangle {
                                width: parent.width
                                height: 60
                                color: pageList.currentIndex === index ? contentPanel.color : primaryColors.c10.background

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    font.pixelSize: 18
                                    font.bold: pageList.currentIndex === index
                                    width: parent.width-34
                                    horizontalAlignment: Text.AlignRight
                                    text: modelData.name
                                    elide: Text.ElideRight
                                    color: pageList.currentIndex === index ? "black" : primaryColors.c700.text
                                }

                                Image {
                                    width: 24; height: 24
                                    source: "../icons/navigation/arrow_right.png"
                                    visible: pageList.currentIndex === index
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.right: parent.right
                                    anchors.rightMargin: 10
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: pageList.currentIndex = index
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: contentPanel
                    anchors.top: parent.top
                    anchors.left: pageList.visible ? pageList.right : parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    clip: true

                    StackLayout {
                        currentIndex: pageList.currentIndex

                        Item {
                            id: firstTab
                            implicitWidth: contentPanel.width
                            implicitHeight: contentPanel.height

                            ScrollView {
                                id: firstTabScrollView
                                anchors.fill: parent
                                anchors.leftMargin: 20

                                Column {
                                    spacing: 10
                                    width: firstTabScrollView.width

                                    Item { width: parent.width; height: 10 }

                                    FileSelector {
                                        id: fileSelector
                                        width: parent.width-20
                                        label: "Select a file to export into"
                                        absoluteFilePath: generator.fileName
                                        allowedExtensions: [
                                            {
                                                "label": "Adobe PDF Format",
                                                "suffix": "pdf",
                                                "value": AbstractReportGenerator.AdobePDF,
                                                "enabled": generator.supportsFormat(AbstractReportGenerator.AdobePDF)
                                            },
                                            {
                                                "label": "Open Document Format",
                                                "suffix": "odt",
                                                "value": AbstractReportGenerator.OpenDocumentFormat,
                                                "enabled": generator.supportsFormat(AbstractReportGenerator.OpenDocumentFormat)
                                            }
                                        ]
                                        nameFilters: {
                                            if(generator.format === AbstractReportGenerator.AdobePDF)
                                                return "Adobe PDF (*.pdf)"
                                            return "Open Document Format (*.odt)"
                                        }
                                        onSelectedExtensionChanged: generator.format = selectedExtension.value
                                        onAbsoluteFilePathChanged: generator.fileName = absoluteFilePath
                                        folder: workspaceSettings.lastOpenReportsFolderUrl
                                        onFolderChanged: workspaceSettings.lastOpenReportsFolderUrl = folder
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
                    enabled: fileSelector.absoluteFilePath !== ""
                    text: "Generate"
                    Material.background: primaryColors.c100.background
                    Material.foreground: primaryColors.c100.text

                    onClicked: {
                        if(generator.generate()) {
                            app.revealFileOnDesktop(generator.fileName)
                            modalDialog.close()
                        }
                    }
                }
            }
        }
    }

    property ErrorReport generatorErrors: Aggregation.findErrorReport(generator)

    Notification.title: formInfo.title
    Notification.text: generatorErrors.errorMessage
    Notification.active: generatorErrors.hasError
    Notification.autoClose: false

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
                    text: fieldInfo.note
                    font.pixelSize: 10
                    font.italic: true
                }

                ScrollView {
                    width: parent.width - 20
                    height: 400
                    background: Rectangle {
                        color: primaryColors.c50.background
                        border.width: 1
                        border.color: primaryColors.c50.text
                    }
                    ListView {
                        id: locationListView
                        model: allLocations
                        clip: true
                        delegate: CheckBox2 {
                            width: locationListView.width-1
                            font.family: scriteDocument.formatting.defaultFont.family
                            text: modelData
                            enabled: screenplayElement.scene && screenplayElement.scene.heading.enabled
                            onToggled: {
                                var locs = selectedLocations
                                if(checked)
                                    locs.push(modelData)
                                else
                                    locs.splice(locs.indexOf(modelData), 1)
                                selectedLocations = locs
                                generator.setConfigurationValue(fieldInfo.name, locs)
                            }
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
                width: parent.width
                wrapMode: Text.WordWrap
                text:fieldInfo.note ? (fieldInfo.note + ". Filter by: ") : "Filter by: "
                font.pixelSize: 10
                font.italic: true
            }

            Row {
                spacing: 10
                width: parent.width - 20

                function split(string) {
                    string = string.trim()
                    if(string === "")
                        return []
                    var items = string.split(",")
                    for(var i=0; i<items.length; i++)
                        items[i] = items[i].trim().toUpperCase()
                    items.contains = function(string) {
                        for(var i=0; i<this.length; i++) {
                            if(string.indexOf(this[i]) >= 0)
                                return true
                        }
                        return false
                    }

                    return items
                }

                TextField2 {
                    id: locTypeFilter
                    width: (parent.width - parent.spacing*2)/3
                    placeholderText: "INT, EXT ..."
                    property var items: parent.split(text)
                    font.capitalization: Font.AllUppercase
                }

                TextField2 {
                    id: locFilter
                    width: (parent.width - parent.spacing*2)/3
                    placeholderText: scriteDocument.structure.allLocations()[0] + " ..."
                    property var items: parent.split(text)
                    font.capitalization: Font.AllUppercase
                }

                TextField2 {
                    id: momentFilter
                    width: (parent.width - parent.spacing*2)/3
                    placeholderText: "DAY, NIGHT ..."
                    property var items: parent.split(text)
                    font.capitalization: Font.AllUppercase
                }
            }

            ScrollView {
                width: parent.width - 20
                height: 320
                background: Rectangle {
                    color: primaryColors.c50.background
                    border.width: 1
                    border.color: primaryColors.c50.text
                }
                ListView {
                    id: sceneListView
                    model: scriteDocument.screenplay
                    clip: true
                    property var selectedSceneNumbers: []

                    function filter(scene) {
                        if(scene && scene.heading.enabled) {
                            var ret = true
                            if(ret && locTypeFilter.items.length > 0)
                                ret &= locTypeFilter.items.contains(scene.heading.locationType)
                            if(ret && momentFilter.items.length > 0)
                                ret &= momentFilter.items.contains(scene.heading.moment)
                            if(ret && locFilter.items.length > 0)
                                ret &= locFilter.items.contains(scene.heading.location)
                            return ret
                        }
                        return true
                    }

                    function select(sceneNumber, flag) {
                        var numbers = generator.getConfigurationValue(fieldInfo.name)
                        var idx = numbers.indexOf(sceneNumber)
                        if(flag) {
                            if(idx < 0)
                                numbers.push(sceneNumber)
                            else
                                return
                        } else {
                            if(idx >= 0)
                                numbers.splice(idx, 1)
                            else
                                return
                        }
                        selectedSceneNumbers = numbers
                        generator.setConfigurationValue(fieldInfo.name, numbers)
                    }

                    delegate: Item {
                        width: sceneListView.width-1
                        height: sceneCheckBox.visible ? sceneCheckBox.height : 0

                        CheckBox2 {
                            id: sceneCheckBox
                            width: parent.width-1
                            font.pointSize: app.idealFontPointSize
                            font.family: scriteDocument.formatting.defaultFont.family
                            visible: sceneListView.filter(screenplayElement.scene)
                            text: {
                                var scene = screenplayElement.scene
                                if(scene && scene.heading.enabled)
                                    return "[" + screenplayElement.resolvedSceneNumber + "] " + (scene && scene.heading.enabled ? scene.heading.text : "")
                                return "NO SCENE HEADING"
                            }
                            checked: sceneListView.selectedSceneNumbers.indexOf(screenplayElement.sceneNumber) >= 0
                            enabled: screenplayElement.scene && screenplayElement.scene.heading.enabled
                            onToggled: sceneListView.select(screenplayElement.sceneNumber, checked)
                        }
                    }
                }
            }

            Row {
                spacing: 10

                Button2 {
                    text: "Select All"
                    onClicked: {
                        var count = scriteDocument.screenplay.elementCount
                        var numbers = generator.getConfigurationValue(fieldInfo.name)
                        for(var i=0; i<count; i++) {
                            var element = scriteDocument.screenplay.elementAt(i)
                            if( sceneListView.filter(element.scene) ) {
                                if(numbers.indexOf(element.sceneNumber) < 0)
                                    numbers.push(element.sceneNumber)
                            }
                        }
                        sceneListView.selectedSceneNumbers = numbers
                        generator.setConfigurationValue(fieldInfo.name, numbers)
                    }
                }

                Button2 {
                    text: "Unselect All"
                    onClicked: {
                        var count = scriteDocument.screenplay.elementCount
                        var numbers = generator.getConfigurationValue(fieldInfo.name)
                        for(var i=0; i<count; i++) {
                            var element = scriteDocument.screenplay.elementAt(i)
                            if( sceneListView.filter(element.scene) ) {
                                var idx = numbers.indexOf(element.sceneNumber)
                                if(idx >= 0)
                                    numbers.splice(idx, 1)
                            }
                        }
                        sceneListView.selectedSceneNumbers = numbers
                        generator.setConfigurationValue(fieldInfo.name, numbers)
                    }
                }

                Text {
                    font.pointSize: app.idealFontPointSize
                    text: sceneListView.selectedSceneNumbers.length === 0 ? "All Scenes Are Selected" : ("" + sceneListView.selectedSceneNumbers.length + " Scene(s) Are Selected")
                    anchors.verticalCenter: parent.verticalCenter
                    padding: 5
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
