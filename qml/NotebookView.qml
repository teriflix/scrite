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

import QtQuick 2.13
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12
import QtQuick.Controls 1.4 as OldControls

import Scrite 1.0

Rectangle {

    function switchToStoryTab() {
        notebookTree.setCurrentIndex( notebookTree.model.findModelIndexFor(structure.notes) )
    }

    function switchToSceneTab() {
        var currentScene = scriteDocument.screenplay.activeScene
        if(currentScene)
            notebookTree.setCurrentIndex( notebookTree.model.findModelIndexFor(currentScene.notes) )
    }

    function switchToCharacterTab(name) {
        var character = scriteDocument.structure.findCharacter(name)
        if(character)
            notebookTree.setCurrentIndex( notebookTree.model.findModelIndexFor(character.notes) )
    }

    NotebookModel {
        id: notebookModel
        document: scriteDocument.loading ? null : scriteDocument
    }

    NotebookFilterModel {
        id: notebookFilterModel
        sourceNotebookModel: notebookModel
    }

    FontMetrics {
        id: fontMetrics
        font.pointSize: Math.ceil(app.idealFontPointSize*0.75)
    }

    SplitView {
        orientation: Qt.Horizontal
        anchors.fill: parent
        Material.background: Qt.darker(primaryColors.button.background, 1.1)

        OldControls.TreeView {
            id: notebookTree
            SplitView.preferredWidth: 350
            SplitView.minimumWidth: 150
            clip: true
            headerVisible: false
            model: notebookFilterModel
            alternatingRowColors: false
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            rowDelegate: Rectangle {
                width: notebookTree.width
                height: fontMetrics.height + 20
                color: styleData.selected ? primaryColors.highlight.background : primaryColors.c10.background
            }
            property var currentData: notebookFilterModel.modelIndexData(currentIndex)
            itemDelegate: Item {
                width: notebookTree.width

                Rectangle {
                    width: notebookTree.width - parent.x
                    height: fontMetrics.height + 20
                    color: {
                        if(styleData.selected)
                            return primaryColors.highlight.background

                        var baseColor = undefined

                        if(styleData.value.notebookItemType === NotebookModel.NotesType) {
                            switch(styleData.value.notebookItemObject.ownerType) {
                            case Notes.SceneOwner:
                            case Notes.CharacterOwner:
                                baseColor = styleData.value.notebookItemObject.color
                                break
                            default:
                                break
                            }
                        } else if(styleData.value.notebookItemType === NotebookModel.NoteType)
                            baseColor = styleData.value.notebookItemObject.color

                        if(baseColor)
                            return Qt.tint(baseColor, "#E7FFFFFF")

                        return primaryColors.c10.background
                    }

                    Row {
                        width: parent.width
                        height: parent.height
                        spacing: 5

                        Item {
                            width: 1
                            height: parent.height
                            visible: itemDelegateIcon.visible
                        }

                        Image {
                            id: itemDelegateIcon
                            width: parent.height * 0.6
                            height: width
                            mipmap: true
                            anchors.verticalCenter: parent.verticalCenter
                            visible: source != ""
                            source: {
                                if(styleData.value.notebookItemType === NotebookModel.NotesType) {
                                    switch(styleData.value.notebookItemObject.ownerType) {
                                    case Notes.SceneOwner:
                                        return "../icons/content/scene.png"
                                    case Notes.CharacterOwner:
                                        return "../icons/content/person_outline.png"
                                    case Notes.StructureOwner:
                                        return "../icons/content/story.png"
                                    default:
                                        break
                                    }
                                } else if(styleData.value.notebookItemType === NotebookModel.NoteType) {
                                    switch(styleData.value.notebookItemObject.type) {
                                    case Note.TextNoteType:
                                        return "../icons/content/note.png"
                                    case Note.FormNoteType:
                                        return "../icons/content/form.png"
                                    default:
                                        break
                                    }
                                }

                                return ""
                            }
                        }

                        Text {
                            id: itemDelegateText
                            padding: 5
                            font.family: fontMetrics.font.family
                            font.pointSize: fontMetrics.font.pointSize
                            font.capitalization: fontMetrics.font.capitalization
                            font.bold: styleData.value.notebookItemType === NotebookModel.CategoryType
                            text: styleData.value.notebookItemTitle ? styleData.value.notebookItemTitle : ""
                            color: app.isLightColor(parent.parent.color) ? "black" : "white"
                            elide: Text.ElideRight
                            width: parent.width-(itemDelegateIcon.visible ? (itemDelegateIcon.width+parent.spacing) : 0)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                }
            }
            OldControls.TableViewColumn {
                title: "Name"
                role: "notebookItemData"
                width: 300
                movable: false
                resizable: false
            }
            onDoubleClicked: {
                if(isExpanded(index))
                    collapse(index)
                else
                    expand(index)
            }

            function setCurrentIndex(modelIndex) {
                var pmi = modelIndex.parent
                while(pmi.valid) {
                    notebookTree.expand(pmi)
                    pmi = pmi.parent
                }

                var row = 0
                while(1) {
                    var idx = notebookTree.__model.mapRowToModelIndex(row)
                    if(!idx.valid)
                        break
                    if(idx === modelIndex) {
                        notebookTree.__listView.currentIndex = row
                        notebookTree.__listView.positionViewAtIndex(row, ListView.Contain)
                        break
                    }
                    ++row
                }
            }
        }

        Loader {
            id: notebookContentLoader
            active: notebookContentActiveProperty.value

            property int currentNotebookItemId: notebookTree.currentData ? notebookTree.currentData.notebookItemId : -1

            ResetOnChange {
                id: notebookContentActiveProperty
                trackChangesOn: notebookContentLoader.currentNotebookItemId
                from: false
                to: true
            }

            sourceComponent: {
                if(!notebookTree.currentData)
                    return unknownComponent

                if(notebookTree.currentData.notebookItemType === NotebookModel.CategoryType) {
                    switch(notebookTree.currentData.notebookItemCategory) {
                    case NotebookModel.ScenesCategory:
                        return scenesComponent
                    case NotebookModel.CharactersCategory:
                        return charactersComponent
                    }
                } else if(notebookTree.currentData.notebookItemType === NotebookModel.NotesType) {
                    switch(notebookTree.currentData.notebookItemObject.ownerType) {
                    case Notes.CharacterOwner:
                        return characterNotesComponent
                    default:
                        break
                    }

                    return notesComponent
                } else if(notebookTree.currentData.notebookItemType === NotebookModel.NoteType) {
                    switch(notebookTree.currentData.notebookItemObject.type) {
                    case Note.TextNoteType:
                        return textNoteComponent
                    case Note.FormNoteType:
                        return formNoteComponent
                    }
                }

                return unknownComponent
            }
            onLoaded: item.componentData = notebookTree.currentData

            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(0,0,0,0.05)
                visible: notebookContentActiveProperty.value === false

                BusyIndicator {
                    running: notebookContentActiveProperty.value === false
                    anchors.centerIn: parent
                }
            }
        }
    }

    Component {
        id: unknownComponent

        Item {
            property var componentData

            Text {
                anchors.centerIn: parent
                text: "Unknown"
                font.pointSize: app.idealFontPointSize
            }
        }
    }

    Component {
        id: notesComponent

        Item {
            id: notesSummary
            property var componentData
            property Notes notes: componentData.notebookItemObject
            readonly property real minimumNoteSize: 175
            property real noteSize: notesFlick.width / Math.floor(notesFlick.width/minimumNoteSize)

            Flickable {
                id: notesFlick
                anchors.fill: parent
                anchors.margins: 20

                Flow {
                    width: notesFlick.width

                    Repeater {
                        model: notes

                        Item {
                            width: noteSize; height: noteSize

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 5
                                color: Qt.tint(objectItem.color, "#E7FFFFFF")
                                border.width: 1
                                border.color: app.isLightColor(color) ? "black" : objectItem.color

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: 5
                                    spacing: 5

                                    Text {
                                        id: headingText
                                        font.pointSize: app.idealFontPointSize
                                        font.bold: true
                                        maximumLineCount: 1
                                        width: parent.width
                                        elide: Text.ElideRight
                                        text: objectItem.title
                                        color: primaryColors.c900.background
                                    }

                                    Text {
                                        width: parent.width
                                        height: parent.height - headingText.height - parent.spacing
                                        wrapMode: Text.WordWrap
                                        elide: Text.ElideRight
                                        font.pointSize: app.idealFontPointSize-2
                                        text: objectItem.summary
                                        color: primaryColors.c600.background
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        width: noteSize; height: noteSize
                        visible: !scriteDocument.readOnly

                        Image {
                            anchors.centerIn: parent
                            width: Math.min(parent.width*0.4, 48)
                            height: width
                            source: "../icons/action/note_add.png"
                            smooth: true; mipmap: true
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            anchors.fill: parent
                            // onClicked: TODO
                        }
                    }
                }

                ScrollBar.vertical: notesFlickScrollBar
            }

            ScrollBar {
                id: notesFlickScrollBar
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                policy: notesFlick.height < notesFlick.contentHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                minimumSize: 0.1
                palette {
                    mid: Qt.rgba(0,0,0,0.25)
                    dark: Qt.rgba(0,0,0,0.75)
                }
                opacity: active ? 1 : 0.2
                Behavior on opacity {
                    enabled: screenplayEditorSettings.enableAnimations
                    NumberAnimation { duration: 250 }
                }
            }
        }
    }

    Component {
        id: textNoteComponent

        Item {
            property var componentData
            property Note note: componentData.notebookItemObject

            TextField2 {
                id: noteHeadingField
                font.pointSize: app.idealFontPointSize + 5
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 10
                text: note.title
                readOnly: scriteDocument.readOnly
                onTextChanged: note.title = text
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLength: 256
                placeholderText: "Note Heading"
                label: ""
            }

            Flickable {
                id: noteContentFieldArea
                anchors.top: noteHeadingField.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: attachmentsArea.top
                anchors.margins: 10
                property bool scrollBarVisible: noteContentField.height > height
                ScrollBar.vertical: ScrollBar {
                    policy: noteContentFieldArea.scrollBarVisible ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                }
                clip: true
                contentWidth: noteContentField.width
                contentHeight: noteContentField.height

                TextArea {
                    id: noteContentField
                    width: noteContentFieldArea.width - (noteContentFieldArea.scrollBarVisible ? 20 : 0)
                    height: contentHeight+50
                    font.pointSize: app.idealFontPointSize
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: note.content
                    onTextChanged: note.content = text
                    selectByMouse: true
                    selectByKeyboard: true
                    Transliterator.textDocument: textDocument
                    Transliterator.cursorPosition: cursorPosition
                    Transliterator.hasActiveFocus: activeFocus
                    readOnly: scriteDocument.readOnly
                    background: Item { }
                    placeholderText: "Note Content"
                    SpecialSymbolsSupport {
                        anchors.top: parent.bottom
                        anchors.left: parent.left
                        textEditor: noteContentField
                        textEditorHasCursorInterface: true
                        enabled: !scriteDocument.readOnly
                    }
                }
            }

            AttachmentsView {
                id: attachmentsArea
                attachments: note.attachments
                orientation: ListView.Horizontal
                height: 80
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10
            }

            AttachmentsDropArea {
                id: attachmentsDropArea
                anchors.fill: parent
                target: note.attachments
                enabled: !scriteDocument.readOnly

                Rectangle {
                    anchors.fill: parent
                    visible: attachmentsDropArea.active
                    color: Qt.tint(primaryColors.c500.background, "#E7FFFFFF")

                    Text {
                        anchors.centerIn: parent
                        width: parent.width * 0.5
                        wrapMode: Text.WordWrap
                        text: "<b>" + attachmentsDropArea.attachment.originalFileName + "</b><br/><br/>Add this file as attachment by dropping it here."
                        horizontalAlignment: Text.AlignHCenter
                        color: primaryColors.c10.text
                        font.pointSize: app.idealFontPointSize
                    }
                }
            }
        }
    }

    Component {
        id: formNoteComponent

        Item {
            property var componentData

            Text {
                anchors.centerIn: parent
                text: "Scenes"
                font.pointSize: app.idealFontPointSize
            }
        }
    }

    Component {
        id: scenesComponent

        Item {
            property var componentData

            Text {
                anchors.centerIn: parent
                text: "Scenes"
                font.pointSize: app.idealFontPointSize
            }
        }
    }

    Component {
        id: charactersComponent

        Item {
            property var componentData

            Text {
                anchors.centerIn: parent
                text: "Characters"
                font.pointSize: app.idealFontPointSize
            }
        }
    }

    property int characterNotesTabIndex: 0
    Component {
        id: characterNotesComponent

        Rectangle {
            id: characterNotes
            property var componentData
            property Character character: componentData.notebookItemObject.character

            signal characterDoubleClicked(string characterName)

            Row {
                id: characterTabBar
                spacing: 10
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 8
                property int tabIndex: characterNotesTabIndex
                onTabIndexChanged: characterNotesTabIndex = tabIndex

                Text {
                    font.pointSize: idealAppFontMetrics.font.pointSize
                    font.family: idealAppFontMetrics.font.family
                    font.capitalization: Font.AllUppercase
                    font.bold: true
                    text: character.name + ": "
                    rightPadding: 10
                }

                Repeater {
                    model: ["Information", "Relationships", "Notes"]

                    Text {
                        font: idealAppFontMetrics.font
                        color: characterTabBar.tabIndex === index ? accentColors.c900.background : primaryColors.c900.background
                        text: modelData

                        Rectangle {
                            height: 1
                            color: accentColors.c900.background
                            width: parent.width
                            anchors.top: parent.bottom
                            visible: characterTabBar.tabIndex === index
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: characterTabBar.tabIndex = index
                        }
                    }
                }
            }

            SwipeView {
                id: characterTabContentArea
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: characterTabBar.bottom
                anchors.bottom: parent.bottom
                anchors.topMargin: 10
                interactive: false
                currentIndex: characterTabBar.tabIndex
                orientation: Qt.Horizontal
                clip: true

                Item {
                    width: characterTabContentArea.width
                    height: characterTabContentArea.height

                    EventFilter.events: [31]
                    EventFilter.onFilter: {
                        EventFilter.forwardEventTo(characterQuickInfoView)
                        result.filter = true
                        result.accepted = true
                    }

                    Item {
                        width: Math.min(400, parent.width)
                        height: parent.height
                        anchors.horizontalCenter: parent.horizontalCenter

                        Connections {
                            target: characterNotes
                            onCharacterChanged: app.execLater(this, 100, function() { photoSlides.currentIndex = 0 } )
                        }
                        Component.onCompleted: app.execLater(this, 100, function() { photoSlides.currentIndex = 0 } )

                        FileDialog {
                            id: fileDialog
                            nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                            selectFolder: false
                            selectMultiple: false
                            sidebarVisible: true
                            selectExisting: true
                            folder: workspaceSettings.lastOpenPhotosFolderUrl
                            onFolderChanged: workspaceSettings.lastOpenPhotosFolderUrl = folder

                            onAccepted: {
                                if(fileUrl != "") {
                                    character.addPhoto(app.urlToLocalFile(fileUrl))
                                    photoSlides.currentIndex = character.photos.length - 1
                                }
                            }
                        }

                        TabSequenceManager {
                            id: characterInfoTabSequence
                            wrapAround: true
                        }

                        Flickable {
                            id: characterQuickInfoView
                            width: parent.width
                            height: Math.min(parent.height, contentHeight)
                            anchors.centerIn: parent
                            contentWidth: characterQuickInfoViewContent.width
                            contentHeight: characterQuickInfoViewContent.height
                            clip: true
                            ScrollBar.vertical: characterQuickInfoViewScrollBar

                            Column {
                                id: characterQuickInfoViewContent
                                width: characterQuickInfoView.width
                                spacing: 10

                                Rectangle {
                                    width: Math.min(parent.width, 300)
                                    height: width
                                    color: photoSlides.currentIndex === photoSlides.count-1 ? Qt.rgba(0,0,0,0.25) : Qt.rgba(0,0,0,0.75)
                                    border.width: 1
                                    border.color: primaryColors.borderColor
                                    anchors.horizontalCenter: parent.horizontalCenter

                                    SwipeView {
                                        id: photoSlides
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        clip: true
                                        currentIndex: 0

                                        Repeater {
                                            model: character.photos

                                            Image {
                                                width: photoSlides.width
                                                height: photoSlides.height
                                                fillMode: Image.PreserveAspectFit
                                                source: "file:///" + modelData
                                            }
                                        }

                                        Item {
                                            width: photoSlides.width
                                            height: photoSlides.height

                                            Button2 {
                                                anchors.centerIn: parent
                                                text: "Add Photo"
                                                onClicked: fileDialog.open()
                                                enabled: !scriteDocument.readOnly && photoSlides.count <= 6
                                            }
                                        }
                                    }

                                    ToolButton3 {
                                        anchors.verticalCenter: photoSlides.verticalCenter
                                        anchors.left: parent.left
                                        iconSource: "../icons/navigation/arrow_left_inverted.png"
                                        enabled: photoSlides.currentIndex > 0
                                        onClicked: photoSlides.currentIndex = Math.max(photoSlides.currentIndex-1, 0)
                                    }

                                    ToolButton3 {
                                        anchors.verticalCenter: photoSlides.verticalCenter
                                        anchors.right: parent.right
                                        iconSource: "../icons/navigation/arrow_right_inverted.png"
                                        enabled: photoSlides.currentIndex < photoSlides.count-1
                                        onClicked: photoSlides.currentIndex = Math.min(photoSlides.currentIndex+1, photoSlides.count-1)
                                    }

                                    ToolButton3 {
                                        anchors.top: parent.top
                                        anchors.right: parent.right
                                        iconSource: "../icons/action/delete_inverted.png"
                                        visible: photoSlides.currentIndex < photoSlides.count-1
                                        onClicked: {
                                            var ci = photoSlides.currentIndex
                                            character.removePhoto(photoSlides.currentIndex)
                                            Qt.callLater( function() { photoSlides.currentIndex = Math.min(ci,photoSlides.count-1) } )
                                        }
                                    }
                                }

                                PageIndicator {
                                    count: photoSlides.count
                                    currentIndex: photoSlides.currentIndex
                                    onCurrentIndexChanged: photoSlides.currentIndex = currentIndex
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    interactive: true
                                }

                                TextField2 {
                                    id: designationField
                                    label: "Role / Designation:"
                                    width: parent.width
                                    labelAlwaysVisible: true
                                    placeholderText: "Hero/Heroine/Villian/Other <max 50 letters>"
                                    maximumLength: 50
                                    text: character.designation
                                    TabSequenceItem.sequence: 0
                                    TabSequenceItem.manager: characterInfoTabSequence
                                    onTextEdited: character.designation = text
                                    enableTransliteration: true
                                    readOnly: scriteDocument.readOnly
                                }

                                TextField2 {
                                    id: aliasesField
                                    label: "Aliases:"
                                    width: parent.width
                                    labelAlwaysVisible: true
                                    placeholderText: "<max 100 letters>"
                                    maximumLength: 50
                                    text: character.aliases.join(", ")
                                    TabSequenceItem.sequence: 1
                                    TabSequenceItem.manager: characterInfoTabSequence
                                    onEditingComplete: character.aliases = text.split(",")
                                    enableTransliteration: true
                                    readOnly: scriteDocument.readOnly
                                }

                                Row {
                                    spacing: 10
                                    width: parent.width

                                    TextField2 {
                                        id: typeField
                                        label: "Type:"
                                        width: (parent.width - parent.spacing)/2
                                        labelAlwaysVisible: true
                                        placeholderText: "Human/Animal/Robot <max 25 letters>"
                                        maximumLength: 25
                                        text: character.type
                                        TabSequenceItem.sequence: 2
                                        TabSequenceItem.manager: characterInfoTabSequence
                                        onTextEdited: character.type = text
                                        enableTransliteration: true
                                        readOnly: scriteDocument.readOnly
                                    }

                                    TextField2 {
                                        id: genderField
                                        label: "Gender:"
                                        width: (parent.width - parent.spacing)/2
                                        labelAlwaysVisible: true
                                        placeholderText: "<max 20 letters>"
                                        maximumLength: 20
                                        text: character.gender
                                        TabSequenceItem.sequence: 3
                                        TabSequenceItem.manager: characterInfoTabSequence
                                        onTextEdited: character.gender = text
                                        enableTransliteration: true
                                        readOnly: scriteDocument.readOnly
                                    }
                                }

                                Row {
                                    spacing: 10
                                    width: parent.width

                                    TextField2 {
                                        id: ageField
                                        label: "Age:"
                                        width: (parent.width - parent.spacing)/2
                                        labelAlwaysVisible: true
                                        placeholderText: "<max 20 letters>"
                                        maximumLength: 20
                                        text: character.age
                                        TabSequenceItem.sequence: 4
                                        TabSequenceItem.manager: characterInfoTabSequence
                                        onTextEdited: character.age = text
                                        enableTransliteration: true
                                        readOnly: scriteDocument.readOnly
                                    }

                                    TextField2 {
                                        id: bodyTypeField
                                        label: "Body Type:"
                                        width: (parent.width - parent.spacing)/2
                                        labelAlwaysVisible: true
                                        placeholderText: "<max 20 letters>"
                                        maximumLength: 20
                                        text: character.bodyType
                                        TabSequenceItem.sequence: 5
                                        TabSequenceItem.manager: characterInfoTabSequence
                                        onTextEdited: character.bodyType = text
                                        enableTransliteration: true
                                        readOnly: scriteDocument.readOnly
                                    }
                                }

                                Row {
                                    spacing: 10
                                    width: parent.width

                                    TextField2 {
                                        id: heightField
                                        label: "Height:"
                                        width: (parent.width - parent.spacing)/2
                                        labelAlwaysVisible: true
                                        placeholderText: "<max 20 letters>"
                                        maximumLength: 20
                                        text: character.height
                                        TabSequenceItem.sequence: 6
                                        TabSequenceItem.manager: characterInfoTabSequence
                                        onTextEdited: character.height = text
                                        enableTransliteration: true
                                        readOnly: scriteDocument.readOnly
                                    }

                                    TextField2 {
                                        id: weightField
                                        label: "Weight:"
                                        width: (parent.width - parent.spacing)/2
                                        labelAlwaysVisible: true
                                        placeholderText: "<max 20 letters>"
                                        maximumLength: 20
                                        text: character.weight
                                        TabSequenceItem.sequence: 7
                                        TabSequenceItem.manager: characterInfoTabSequence
                                        onTextEdited: character.weight = text
                                        enableTransliteration: true
                                        readOnly: scriteDocument.readOnly
                                    }
                                }
                            }
                        }
                    }

                    ScrollBar {
                        id: characterQuickInfoViewScrollBar
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        policy: characterQuickInfoView.height < characterQuickInfoView.contentHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                        minimumSize: 0.1
                        palette {
                            mid: Qt.rgba(0,0,0,0.25)
                            dark: Qt.rgba(0,0,0,0.75)
                        }
                        opacity: active ? 1 : 0.2
                        Behavior on opacity {
                            enabled: screenplayEditorSettings.enableAnimations
                            NumberAnimation { duration: 250 }
                        }
                    }
                }

                CharacterRelationshipsGraphView {
                    width: characterTabContentArea.width
                    height: characterTabContentArea.height
                    character: characterNotes.character
                    editRelationshipsEnabled: !scriteDocument.readOnly
                    onCharacterDoubleClicked: characterNotes.characterDoubleClicked(characterName)
                    onAddNewRelationshipRequest: {
                        modalDialog.closeable = false
                        modalDialog.popupSource = sourceItem
                        modalDialog.arguments = character
                        modalDialog.sourceComponent = addRelationshipDialogComponent
                        modalDialog.active = true
                    }
                    onRemoveRelationshipWithRequest: {
                        var relationship = character.findRelationship(otherCharacter)
                        character.removeRelationship(relationship)
                    }
                }

                Loader {
                    width: characterTabContentArea.width
                    height: characterTabContentArea.height
                    sourceComponent: notesComponent
                    active: characterNotes.character
                    onLoaded: item.componentData = characterNotes.componentData
                }
            }
        }
    }

    Component {
        id: addRelationshipDialogComponent

        Rectangle {
            width: Math.max(800, ui.width*0.5)
            height: Math.min(ui.height*0.85, Math.min(charactersList.height, 500) + title.height + searchBar.height + addRelationshipDialogButtons.height + 80)
            color: primaryColors.c10.background

            Component.onCompleted: {
                character = modalDialog.arguments
                modalDialog.arguments = undefined
            }

            property Character character
            readonly property var unrelatedCharacterNames: character.unrelatedCharacterNames()

            Item {
                anchors.fill: parent
                anchors.margins: 20

                Text {
                    id: title
                    width: parent.width
                    anchors.top: parent.top
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    text: "Name applicable relationships between <strong>" + character.name + "</strong> and others in the screenplay."
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    anchors.fill: charactersListScroll
                    anchors.margins: -1
                    border.width: 1
                    border.color: primaryColors.borderColor
                    visible: charactersListScroll.height >= 600
                }

                SearchBar {
                    id: searchBar
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: title.bottom
                    anchors.topMargin: 10
                    searchEngine.objectName: "Characters Search Engine"
                    visible: charactersList.height > charactersListScroll.height
                }

                TabSequenceManager {
                    id: characterListTabManager
                }

                ScrollArea {
                    id: charactersListScroll
                    anchors.top: searchBar.bottom
                    anchors.bottom: addRelationshipDialogButtons.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10
                    contentWidth: charactersList.width
                    contentHeight: charactersList.height
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ScrollBar.vertical.policy: charactersList.height > height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                    ScrollBar.vertical.opacity: active ? 1 : 0.2

                    Column {
                        id: charactersList
                        width: charactersListScroll.width-20

                        Repeater {
                            id: otherCharacterItems
                            model: unrelatedCharacterNames

                            Rectangle {
                                id: characterRowItem
                                property string thisCharacterName: app.camelCased(character.name)
                                property string otherCharacterName: modelData
                                property bool checked: relationshipName.length > 0
                                property string relationship: relationshipName.text
                                width: charactersList.width
                                height: characterRow.height*1.15

                                Row {
                                    id: characterRow
                                    width: parent.width - 20
                                    anchors.right: parent.right
                                    spacing: 10

                                    Image {
                                        width: 24; height: 24
                                        source: "../icons/navigation/check.png"
                                        anchors.verticalCenter: parent.verticalCenter
                                        opacity: relationshipName.length > 0 ? 1 : 0.05
                                    }

                                    Text {
                                        id: characterRowLabel1
                                        font.pointSize: app.idealFontPointSize
                                        text: thisCharacterName + ": "
                                        color: foregroundColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        width: Math.min(implicitWidth, (parent.width-100)/3)
                                        horizontalAlignment: Text.AlignRight
                                    }

                                    TextField2 {
                                        id: relationshipName
                                        enableTransliteration: true
                                        width: parent.width - 32 - characterRowLabel1.width - characterRowLabel2.width - 3*parent.spacing
                                        label: ""
                                        color: foregroundColor
                                        font.pointSize: app.idealFontPointSize
                                        placeholderText: "husband of, wife of, friends with, reports to ..."
                                        Material.background: backgroundColor
                                        Material.foreground: foregroundColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        TabSequenceItem.manager: characterListTabManager
                                        TabSequenceItem.sequence: index
                                        maximumLength: 50
                                    }

                                    Text {
                                        id: characterRowLabel2
                                        font.pointSize: app.idealFontPointSize
                                        text: app.camelCased(otherCharacterName) + "."
                                        color: foregroundColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        width: Math.min(implicitWidth, (parent.width-100)/3)
                                        rightPadding: 10
                                    }
                                }

                                color: backgroundColor
                                property bool highlight: false
                                property color backgroundColor: highlight ? accentColors.c100.background : primaryColors.c10.background
                                property color foregroundColor: highlight ? accentColors.c100.text : primaryColors.c10.text

                                SearchAgent.engine: searchBar.searchEngine
                                SearchAgent.onSearchRequest: {
                                    SearchAgent.searchResultCount = SearchAgent.indexesOf(string, otherCharacterName).length > 0 ? 1 : 0
                                }
                                SearchAgent.onCurrentSearchResultIndexChanged: {
                                    highlight = SearchAgent.currentSearchResultIndex >= 0
                                    charactersListScroll.ensureItemVisible(characterRowItem,1,10)
                                }
                            }
                        }
                    }
                }

                Row {
                    id: addRelationshipDialogButtons
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    spacing: 10

                    Button2 {
                        text: "Cancel"
                        onClicked: modalDialog.close()
                    }

                    Button2 {
                        text: "Create Relationships"
                        onClicked: {
                            for(var i=0; i<otherCharacterItems.count; i++) {
                                var item = otherCharacterItems.itemAt(i)
                                if(item.checked) {
                                    var otherCharacter = scriteDocument.structure.addCharacter(item.otherCharacterName)
                                    if(otherCharacter) {
                                        character.addRelationship(item.relationship, otherCharacter)
                                        character.characterRelationshipGraph = {}
                                    }
                                }
                            }
                            modalDialog.close()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: attachmentsComponent

        Item {
            id: attachmentsArea
            property var componentData
            property Attachments attachments: componentData.notebookItemObject.attachments

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            height: 60
        }
    }
}
