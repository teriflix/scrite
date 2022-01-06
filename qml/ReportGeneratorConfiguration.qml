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

import io.scrite.components 1.0

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

Item {
    id: configurationBox
    property AbstractReportGenerator generator
    property var formInfo: {"title": "Unknown", "description": "", "groupedFields": []}

    width: 750
    height: formInfo.fields.length > 0 || (generator && !generator.featureEnabled) ? 720 : 275
    readonly property color dialogColor: primaryColors.c300.background
    readonly property bool isPdfExport: generator ? generator.format === AbstractReportGenerator.AdobePDF : false

    Component.onCompleted: {
        modalDialog.closeOnEscape = false

        var reportName = typeof modalDialog.arguments === "string" ? modalDialog.arguments : modalDialog.arguments.reportName
        generator = Scrite.document.createReportGenerator(reportName)
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
                enabled: generator.featureEnabled
                opacity: enabled ? 1 : 0.5

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

            DisabledFeatureNotice {
                anchors.fill: formOptionsArea
                visible: !generator.featureEnabled
                color: Qt.rgba(1,1,1,0.9)
                featureName: ""
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

                    EventFilter.target: Scrite.app
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
                    enabled: fileSelector.absoluteFilePath !== "" && generator.featureEnabled
                    text: "Generate"
                    Material.background: primaryColors.c100.background
                    Material.foreground: primaryColors.c100.text
                    onClicked: busyOverlay.visible = true
                }
            }

            BusyOverlay {
                id: busyOverlay
                anchors.fill: parent
                busyMessage: "Generating \"" + (isPdfExport ? generator.title : generator.fileName) + "\" ..."

                FileManager {
                    id: fileManager
                }

                onVisibleChanged: {
                    if(visible) {
                        Scrite.app.execLater(busyOverlay, 100, function() {
                            const dlFileName = generator.fileName
                            if(generator.format === AbstractReportGenerator.AdobePDF)
                                generator.fileName = fileManager.generateUniqueTemporaryFileName("pdf")

                            if(generator.generate()) {
                                if(generator.format === AbstractReportGenerator.AdobePDF) {
                                    const ppr = generator.singlePageReport ? 1 : 2
                                    pdfViewer.show(generator.title, generator.fileName, dlFileName, ppr)
                                } else
                                    Scrite.app.revealFileOnDesktop(generator.fileName)
                                modalDialog.close()
                            } else
                                busyOverlay.visible = false
                        })
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
        if(kind === "MultipleEpisodeSelector")
            return editor_MultipleEpisodeSelector
        if(kind === "MultipleTagGroupSelector")
            return editor_MultipleTagGroupSelector;
        if(kind === "CheckBox")
            return editor_CheckBox
        if(kind === "EnumSelector")
            return editor_EnumSelector
        if(kind === "TextBox")
            return editor_TextBox
        if(kind === "IntegerSpinBox")
            return editor_IntegerSpinBox
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
            height: fieldTitleText.height + fieldLabelText.height + (characterNameListView.visible ? 325 : 10)

            onFieldInfoChanged: {
                characterNameListView.selectedCharacters = generator.getConfigurationValue(fieldInfo.name)
                characterNameListView.visible = characterNameListView.selectedCharacters.length === 0
            }

            Text {
                id: fieldLabelText
                text: fieldInfo.label
                width: parent.width
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
                font.pointSize: Scrite.app.idealFontPointSize
            }

            Loader {
                id: fieldTitleText
                width: parent.width-30
                anchors.top: fieldLabelText.bottom
                anchors.topMargin: 10
                sourceComponent: Flow {
                    spacing: 5
                    flow: Flow.LeftToRight

                    Text {
                        id: charactersPrefix
                        text: characterNames.length === 0 ? "No Characters Selected" : "»"
                        topPadding: 0
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
                        width: charactersPrefix.height
                        height: charactersPrefix.height
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
                        width: charactersPrefix.height
                        height: charactersPrefix.height
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
            }
        }
    }

    Component {
        id: editor_MultipleLocationSelector

        Item {
            property var fieldInfo
            property var allLocations: Scrite.document.structure.allLocations()
            property var selectedLocations: []

            height: multipleLocationSelectorLayout.height

            Column {
                id: multipleLocationSelectorLayout
                width: parent.width-20
                spacing: 5

                Text {
                    text: fieldInfo.label
                    width: parent.width
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    font.pointSize: Scrite.app.idealFontPointSize
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
                    height: 300
                    background: Rectangle {
                        color: primaryColors.c50.background
                        border.width: 1
                        border.color: primaryColors.c50.text
                    }
                    ListView {
                        id: locationListView
                        model: allLocations
                        clip: true
                        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                        delegate: CheckBox2 {
                            width: locationListView.width-1
                            font.family: Scrite.document.formatting.defaultFont.family
                            text: modelData
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

        /**
          Once upon a time sceneNumber was the same as element index.
          It is no longer the case. Scene number can event be user specified.
          So rather than using ScreenplayElement.sceneNumber we now use ScreenplayElement.elementIndex
          to let the report generator know which scenes the user has selected.
          */

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
                    width: (parent.width - parent.spacing*2)*0.25
                    label: ""
                    placeholderText: "INT, EXT ..."
                    property var items: parent.split(text)
                    font.capitalization: Font.AllUppercase
                }

                TextField2 {
                    id: locFilter
                    width: parent.width - locTypeFilter.width - locTypeFilter.width
                    label: ""
                    placeholderText: Scrite.document.structure.allLocations()[0] + " ..."
                    property var items: parent.split(text)
                    font.capitalization: Font.AllUppercase
                }

                TextField2 {
                    id: momentFilter
                    width: locTypeFilter.width
                    label: ""
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
                    model: Scrite.document.screenplay
                    clip: true
                    property var selectedSceneNumbers: []
                    property var selectedEpisodeNumbers: generator.episodeNumbers
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

                    function filter(scene) {
                        if(selectedEpisodeNumbers && selectedEpisodeNumbers.length > 0) {
                            if(scene && selectedEpisodeNumbers.indexOf(scene.episodeIndex+1) < 0)
                                return false
                        }

                        if(scene) {
                            if(!scene.heading.enabled)
                                return false

                            var ret = true
                            if(ret && locTypeFilter.items.length > 0)
                                ret &= locTypeFilter.items.contains(scene.heading.locationType)
                            if(ret && momentFilter.items.length > 0)
                                ret &= momentFilter.items.contains(scene.heading.moment)
                            if(ret && locFilter.items.length > 0)
                                ret &= locFilter.items.contains(scene.heading.location)

                            return ret
                        }

                        return locTypeFilter.items.length === 0 && momentFilter.items.length === 0 && locFilter.items.length === 0
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
                            font.pointSize: Scrite.app.idealFontPointSize
                            font.family: Scrite.document.formatting.defaultFont.family
                            visible: sceneListView.filter(screenplayElement.scene) && screenplayElement.scene && screenplayElement.scene.heading.enabled
                            text: {
                                var scene = screenplayElement.scene
                                if(scene && scene.heading.enabled)
                                    return "[" + screenplayElement.resolvedSceneNumber + "] " + (scene && scene.heading.enabled ? scene.heading.text : "")
                                return "NO SCENE HEADING"
                            }
                            checked: sceneListView.selectedSceneNumbers.indexOf(screenplayElement.elementIndex) >= 0
                            enabled: screenplayElement.scene && screenplayElement.scene.heading.enabled
                            onToggled: sceneListView.select(screenplayElement.elementIndex, checked)
                        }
                    }
                }
            }

            Row {
                spacing: 10

                Button2 {
                    text: "Select All"
                    onClicked: {
                        var count = Scrite.document.screenplay.elementCount
                        var numbers = generator.getConfigurationValue(fieldInfo.name)
                        for(var i=0; i<count; i++) {
                            var element = Scrite.document.screenplay.elementAt(i)
                            if( sceneListView.filter(element.scene) ) {
                                if(numbers.indexOf(element.elementIndex) < 0)
                                    numbers.push(element.elementIndex)
                            }
                        }
                        sceneListView.selectedSceneNumbers = numbers
                        generator.setConfigurationValue(fieldInfo.name, numbers)
                    }
                }

                Button2 {
                    text: "Unselect All"
                    onClicked: {
                        var count = Scrite.document.screenplay.elementCount
                        var numbers = generator.getConfigurationValue(fieldInfo.name)
                        for(var i=0; i<count; i++) {
                            var element = Scrite.document.screenplay.elementAt(i)
                            if( sceneListView.filter(element.scene) ) {
                                var idx = numbers.indexOf(element.elementIndex)
                                if(idx >= 0)
                                    numbers.splice(idx, 1)
                            }
                        }
                        sceneListView.selectedSceneNumbers = numbers
                        generator.setConfigurationValue(fieldInfo.name, numbers)
                    }
                }

                Text {
                    font.pointSize: Scrite.app.idealFontPointSize
                    text: sceneListView.selectedSceneNumbers.length === 0 ? "All Scenes Are Selected" : ("" + sceneListView.selectedSceneNumbers.length + " Scene(s) Are Selected")
                    anchors.verticalCenter: parent.verticalCenter
                    padding: 5
                }
            }
        }
    }

    Component {
        id: editor_MultipleEpisodeSelector

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
                text: fieldInfo.note
                font.pixelSize: 10
                font.italic: true
            }

            Item {
                width: parent.width
                height: parent.spacing
            }

            ScrollView {
                width: parent.width-20
                height: 320
                background: Rectangle {
                    color: primaryColors.c50.background
                    border.width: 1
                    border.color: primaryColors.c50.text
                }
                ListView {
                    id: episodeListView
                    model: Scrite.document.screenplay.episodeCount + 1
                    clip: true
                    property var episodeNumbers: generator.getConfigurationValue(fieldInfo.name)
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

                    function select(episodeNumber, flag) {
                        var numbers = generator.getConfigurationValue(fieldInfo.name)
                        var idx = numbers.indexOf(episodeNumber)
                        if(flag) {
                            if(idx < 0)
                                numbers.push(episodeNumber)
                            else
                                return
                        } else {
                            if(idx >= 0)
                                numbers.splice(idx, 1)
                            else
                                return
                        }
                        episodeNumbers = numbers
                        generator.setConfigurationValue(fieldInfo.name, numbers)
                    }

                    delegate: Item {
                        width: episodeListView.width-1
                        height: index > 0 ? 40 : (Scrite.document.screenplay.episodeCount === 0 ? 40 : 0)

                        Text {
                            text: "No espisodes in this screenplay"
                            visible: Scrite.document.screenplay.episodeCount === 0
                            anchors.centerIn: parent
                        }

                        CheckBox2 {
                            text: "EPISODE " + index
                            anchors.verticalCenter: parent.verticalCenter
                            visible: index > 0
                            font.pointSize: Scrite.app.idealFontPointSize
                            font.family: Scrite.document.formatting.defaultFont.family
                            checked: episodeListView.episodeNumbers.indexOf(index) >= 0
                            onToggled: episodeListView.select(index, checked)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: editor_MultipleTagGroupSelector

        Column {
            spacing: 5
            property var fieldInfo

            Text {
                id: labelText
                width: parent.width-20
                wrapMode: Text.WordWrap
                text: fieldInfo.label
            }

            Text {
                id: noteText
                width: parent.width-20
                wrapMode: Text.WordWrap
                text: fieldInfo.note
                font.pixelSize: 10
                font.italic: true
            }

            ScrollView {
                height: 350
                width: parent.width-20
                background: Rectangle {
                    color: primaryColors.c50.background
                    border.width: 1
                    border.color: primaryColors.c50.text
                }
                ListView {
                    id: groupsView
                    clip: true
                    model: GenericArrayModel {
                        array: Scrite.document.structure.groupsModel
                        objectMembers: ["category", "label", "name"]
                    }
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    section.property: "category"
                    section.criteria: ViewSection.FullString
                    section.delegate: Item {
                        width: groupsView.width
                        height: 40
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 3
                            color: primaryColors.windowColor
                            Text {
                                text: section
                                topPadding: 5
                                bottomPadding: 5
                                anchors.centerIn: parent
                                color: primaryColors.button.text
                                font.pointSize: Scrite.app.idealFontPointSize
                            }
                        }
                    }
                    property var checkedTags: generator.getConfigurationValue(fieldInfo.name)
                    delegate: CheckBox2 {
                        text: label
                        checked: groupsView.checkedTags.indexOf(name) >= 0
                        onToggled: {
                            var tags = groupsView.checkedTags
                            if(checked)
                                tags.push(name)
                            else
                                tags.splice(tags.indexOf(name), 1)
                            groupsView.checkedTags = tags
                            generator.setConfigurationValue(fieldInfo.name, tags)
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
            font.pointSize: Scrite.app.idealFontPointSize
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
                width: parent.width
                wrapMode: Text.WordWrap
                font.capitalization: Font.Capitalize
                font.pointSize: Scrite.app.idealFontPointSize-2
            }

            Text {
                text: fieldInfo.note
                width: parent.width
                wrapMode: Text.WordWrap
                font.pointSize: Scrite.app.idealFontPointSize-2
                font.italic: true
                visible: text !== ""
            }

            TextField2 {
                width: parent.width - 30
                label: ""
                text: generator.getConfigurationValue(fieldInfo.name)
                placeholderText: fieldInfo.label
                onTextChanged: {
                    if(generator)
                        generator.setConfigurationValue(fieldInfo.name, text)
                }
            }
        }
    }

    Component {
        id: editor_IntegerSpinBox

        Column {
            property var fieldInfo
            spacing: 6

            Text {
                text: fieldInfo.label
                width: parent.width
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
                font.pointSize: Scrite.app.idealFontPointSize
            }

            Text {
                text: fieldInfo.note
                width: parent.width-10
                wrapMode: Text.WordWrap
                font.pointSize: Scrite.app.idealFontPointSize-4
                font.italic: true
                visible: text !== ""
            }

            SpinBox {
                from: fieldInfo.min
                to: fieldInfo.max
                value: generator ? generator.getConfigurationValue(fieldInfo.name) : 0
                onValueModified: {
                    if(generator)
                        generator.setConfigurationValue(fieldInfo.name, value)
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
