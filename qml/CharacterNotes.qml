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
import QtQuick.Window 2.13
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

import Scrite 1.0

Item {
    id: characterNotes
    property Character character
    property color colorHint: primaryColors.borderColor

    signal characterDoubleClicked(string characterName)

    Rectangle {
        id: contextPanelArea
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: contextPanel.right
        anchors.margins: 1
        anchors.rightMargin: -10
        color: "#80ffffff"
    }

    Loader {
        id: contextPanel
        width: Math.min( ui.height*0.425, 400 )
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 20
        sourceComponent: characterInfoComponent
    }

    Loader {
        id: detailsPanel
        anchors.left: contextPanelArea.right
        anchors.top: contextPanelArea.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
        anchors.leftMargin: 10
        sourceComponent: notesComponent
    }

    Component {
        id: characterInfoComponent

        Item {
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
                anchors.fill: parent
                contentWidth: characterQuickInfoViewContent.width
                contentHeight: characterQuickInfoViewContent.height
                clip: true
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
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

                Column {
                    id: characterQuickInfoViewContent
                    width: characterQuickInfoView.width - 20
                    spacing: 10

                    Text {
                        font.bold: true
                        font.pointSize: app.idealFontPointSize
                        text: character.name
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Rectangle {
                        width: parent.width
                        height: parent.width
                        color: photoSlides.currentIndex === photoSlides.count-1 ? Qt.rgba(0,0,0,0.25) : Qt.rgba(0,0,0,0.75)
                        border.width: 1
                        border.color: primaryColors.borderColor

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
                        onTextEdited: character.aliases = text.split(",")
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
    }

    Component {
        id: notesComponent

        NotesView {
            notesModel: character.notesModel
            title: "You can capture your thoughts, ideas and research related to '<b>" + character.name + "</b>' here."
            listHeader: relationshipsHeaderComponent
            onNewNoteRequest: {
                var note = noteComponent.createObject(character)
                note.color = noteColor
                character.addNote(note)
            }
            onRemoveNoteRequest: character.removeNote(character.noteAt(index))
        }
    }

    Component {
        id: noteComponent

        Note {
            heading: "Note Heading"
            Component.onCompleted: {
                var lastNote = currentTabNotesSource.notesModel.objectAt(currentTabNotesSource.notesModel.objectCount-1)
                if(lastNote)
                    color = lastNote.color
                else
                    color = "white"
            }
        }
    }


    Component {
        id: relationshipsHeaderComponent

        Rectangle {
            color: "#80ffffff"
            border.width: 1
            border.color: "black"
            radius: 6

            Column {
                id: relationshipContent
                spacing: 10
                anchors.fill: parent
                anchors.margins: 20
                width: relashionshipScroll.width - (relashionshipScroll.contentHeight > relashionshipScroll.height ? 20 : 0)

                Rectangle {
                    id: titleBar
                    color: "#c0ffffff"
                    width: parent.width
                    height: 46
                    border.width: 1
                    border.color: "black"
                    radius: 6

                    Text {
                        id: noteTitleText
                        width: parent.width
                        leftPadding: 10
                        text: character.relationshipCount > 0 ? (character.relationshipCount + " Relationship(s)") : "Relationships"
                        font.pointSize: app.idealFontPointSize
                        font.letterSpacing: 1
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    ToolButton3 {
                        id: addRelationshipButton
                        iconSource: "../icons/content/add_circle_outline.png"
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        ToolTip.text: "Add Relationship"
                        ToolTip.visible: hovered
                        onClicked: {
                            modalDialog.closeable = false
                            modalDialog.popupSource = addRelationshipButton
                            modalDialog.sourceComponent = addRelationshipDialogComponent
                            modalDialog.active = true
                        }
                    }
                }

                TabSequenceManager {
                    id: relationshipTabSequence
                    wrapAround: true
                }

                ListView {
                    id: relationshipView
                    width: parent.width
                    height: parent.height - titleBar.height - parent.spacing
                    clip: true
                    model: character.relationshipsModel
                    spacing: 10
                    ScrollBar.vertical: ScrollBar {
                        policy: relationshipView.contentHeight > relationshipView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
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
                    delegate: Rectangle {
                        width: relationshipView.width - (relationshipView.contentHeight > relationshipView.height ? 20 : 0)
                        height: delegateLayout.height + 10
                        color: "white"
                        border.color: primaryColors.borderColor
                        border.width: 1
                        radius: 6

                        Column {
                            id: delegateLayout
                            spacing: 2
                            width: parent.width - 10
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right

                            Item {
                                width: parent.width
                                height: 10
                            }

                            Row {
                                width: parent.width
                                spacing: 10

                                Text {
                                    id: relationshipIndexLabel
                                    font.pixelSize: relationshipField.height * 0.7
                                    text: (index+1) + ". "
                                    anchors.top: parent.top
                                }

                                TextField2 {
                                    id: relationshipField
                                    width: parent.width - relationshipIndexLabel.width - removeRelationshipButton.width - 2*parent.spacing
                                    label: "Relationship:"
                                    labelAlwaysVisible: true
                                    placeholderText: "friend, spouse, etc.. <max 50 characters>"
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: modelData.name
                                    onTextEdited: modelData.name = text
                                    TabSequenceItem.manager: relationshipTabSequence
                                    TabSequenceItem.sequence: index
                                }

                                ToolButton3 {
                                    id: removeRelationshipButton
                                    iconSource: "../icons/action/delete.png"
                                    anchors.top: parent.top
                                    onClicked: character.removeRelationship(modelData)
                                }
                            }

                            CharacterBox {
                                character: modelData.withCharacter
                                width: parent.width - 30
                                anchors.right: parent.right
                                color: Qt.rgba(0,0,0,0)
                                border.width: 0
                                onDoubleClicked: characterDoubleClicked(modelData.withCharacter.name)
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: addRelationshipDialogComponent

        Rectangle {
            width: 800
            height: Math.min(charactersList.height, 600) + title.height + searchBar.height + addRelationshipDialogButtons.height + 80
            color: primaryColors.c10.background

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
                    text: "Name and check application relationships for <strong>" + character.name + "</strong>."
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
                                property string otherCharacterName: modelData
                                property bool checked: checkBox.enabled && checkBox.checked
                                property string relationship: relationshipName.text
                                width: charactersList.width
                                height: characterRow.height

                                Row {
                                    id: characterRow
                                    width: parent.width - 20
                                    anchors.right: parent.right
                                    spacing: 10

                                    CheckBox2 {
                                        id: checkBox
                                        width: charactersList.columnWidth
                                        checked: false
                                        Material.background: backgroundColor
                                        Material.foreground: foregroundColor
                                        enabled: relationshipName.length > 0
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        font.pointSize: app.idealFontPointSize
                                        text: "is"
                                        color: foregroundColor
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    TextField2 {
                                        id: relationshipName
                                        width: 280
                                        label: ""
                                        placeholderText: "relationship name"
                                        font.pointSize: app.idealFontPointSize
                                        color: foregroundColor
                                        Material.background: backgroundColor
                                        Material.foreground: foregroundColor
                                        onLengthChanged: checkBox.checked = length > 0
                                        anchors.verticalCenter: parent.verticalCenter
                                        TabSequenceItem.manager: characterListTabManager
                                        TabSequenceItem.sequence: index
                                    }

                                    Text {
                                        width: 400
                                        color: foregroundColor
                                        font.pointSize: app.idealFontPointSize
                                        text: "of <strong>" + otherCharacterName + "</strong>"
                                        anchors.verticalCenter: parent.verticalCenter
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
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
                                    var ch = scriteDocument.structure.addCharacter(item.otherCharacterName)
                                    if(ch)
                                        character.addRelationship(item.relationship, ch)
                                }
                            }
                            modalDialog.close()
                        }
                    }
                }
            }
        }
    }
}
