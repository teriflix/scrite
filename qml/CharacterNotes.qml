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
    property bool showCharacterInfoInNotesTab: false

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
        visible: !showCharacterInfoInNotesTab
    }

    Loader {
        id: contextPanel
        width: Math.min( ui.height*0.425, 400 )
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 20
        active: !showCharacterInfoInNotesTab
        sourceComponent: characterInfoComponent
        visible: !showCharacterInfoInNotesTab
    }

    Loader {
        id: detailsPanel
        anchors.left: showCharacterInfoInNotesTab ? parent.left : contextPanelArea.right
        anchors.top: contextPanelArea.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
        anchors.leftMargin: 10
        sourceComponent: TabView3 {
            id: detailsTab
            tabNames: ["Relationships", "(" + character.noteCount + ") Notes"]
            tabColor: colorHint
            currentTabIndex: notebookSettings.activeTab
            onCurrentTabIndexChanged: notebookSettings.activeTab = currentTabIndex
            currentTabContent: Item {
                CharacterRelationshipsGraphView {
                    anchors.fill: parent
                    anchors.margins: 2
                    visible: detailsTab.currentTabIndex === 0
                    z: visible ? 1 : 0
                    character: characterNotes.character
                    editRelationshipsEnabled: !scriteDocument.readOnly
                    onCharacterDoubleClicked: characterNotes.characterDoubleClicked(characterName)
                    onAddNewRelationshipRequest: {
                        modalDialog.closeable = false
                        modalDialog.popupSource = sourceItem
                        modalDialog.sourceComponent = addRelationshipDialogComponent
                        modalDialog.active = true
                    }
                    onRemoveRelationshipWithRequest: {
                        var relationship = character.findRelationship(otherCharacter)
                        character.removeRelationship(relationship)
                    }
                }

                NotesView {
                    anchors.fill: parent
                    anchors.margins: 2
                    visible: detailsTab.currentTabIndex === 1
                    z: visible ? 1 : 0
                    notesModel: character.notesModel
                    onNewNoteRequest: {
                        var note = noteComponent.createObject(character)
                        note.color = noteColor
                        character.addNote(note)
                    }
                    listHeader: showCharacterInfoInNotesTab ? characterInfoComponent : null
                    onRemoveNoteRequest: character.removeNote(character.noteAt(index))
                    title: "You can capture your thoughts, ideas and research related to '<b>" + character.name + "</b>' here."
                }
            }
        }
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
                        topPadding: 8
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
        id: addRelationshipDialogComponent

        Rectangle {
            width: Math.max(800, ui.width*0.5)
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
}
