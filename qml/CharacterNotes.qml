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
                        backTabItem: weightField
                        tabItem: aliasesField
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
                        backTabItem: designationField
                        tabItem: typeField
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
                            backTabItem: aliasesField
                            tabItem: genderField
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
                            backTabItem: typeField
                            tabItem: ageField
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
                            backTabItem: genderField
                            tabItem: bodyTypeField
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
                            backTabItem: ageField
                            tabItem: heightField
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
                            backTabItem: bodyTypeField
                            tabItem: weightField
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
                            backTabItem: heightField
                            tabItem: designationField
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
        id: characterBoxComponent

        Rectangle {
            property Character character

            width: 150
            height: 120
            color: "white"
            border.width: 1
            border.color: "black"

            Row {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                clip: true

                Image {
                    width: parent.height
                    height: parent.height
                    source: {
                        if(character.photos.length > 0)
                            return "file:///" + character.photos[0]
                        return "../icons/content/person_outline.png"
                    }
                    fillMode: Image.PreserveAspectFit
                    mipmap: true; smooth: true
                }

                Column {
                    width: parent.width - parent.height - parent.spacing
                    anchors.top: parent.top
                    spacing: 4

                    Text {
                        text: character.name
                        font.bold: true
                        font.pointSize: app.idealFontPointSize
                        font.capitalization: Font.AllUppercase
                        width: parent.width
                        elide: Text.ElideRight
                    }

                    Text {
                        text: character.designation
                        font.pointSize: app.idealFontPointSize-2
                        visible: text !== ""
                        width: parent.width
                        elide: Text.ElideRight
                    }

                    Flow {
                        width: parent.width
                        spacing: 8

                        Text {
                            text: character.gender
                            font.pointSize: app.idealFontPointSize-3
                            visible: character.gender !== ""
                        }

                        Text {
                            text: "Age: " + character.age
                            font.pointSize: app.idealFontPointSize-3
                            visible: character.age !== ""
                        }

                        Text {
                            text: "Height: " + character.height
                            font.pointSize: app.idealFontPointSize-3
                            visible: character.height !== ""
                        }

                        Text {
                            text: "Weight: " + character.weight
                            font.pointSize: app.idealFontPointSize-3
                            visible: character.weight !== ""
                        }
                    }

                    Text {
                        text: character.aliases.join(", ")
                        font.pointSize: app.idealFontPointSize-3
                        visible: text !== ""
                        width: parent.width
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
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
                var lastNote = currentSource.notesModel.objectAt(currentSource.notesModel.objectCount-1)
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
                    delegate: Column {
                        width: relationshipView.width - 20
                        spacing: 2

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
                                width: parent.width - relationshipIndexLabel.width - parent.spacing
                                label: "Relationship:"
                                labelAlwaysVisible: true
                                placeholderText: "friend, spouse, etc.. <max 50 characters>"
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                text: modelData.name
                                onTextEdited: modelData.name = text
                            }
                        }

                        Loader {
                            width: parent.width
                            sourceComponent: characterBoxComponent
                            onItemChanged: {
                                if(item)
                                    item.character = modelData.withCharacter
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
            width: Math.max(Math.min(800, height*1.2), addRelationshipDialogButtons.width+80)
            height: Math.min(unrelatedCharacterNames.length, 10) * 50 + title.height + addRelationshipDialogButtons.height + 60
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
                    text: "Check the characters with whom you want to establish relationship for <strong>" + character.name + "</strong>."
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    anchors.fill: charactersFlowScroll
                    anchors.margins: -1
                    border.width: 1
                    border.color: primaryColors.borderColor
                }

                SearchBar {
                    id: searchBar
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: title.bottom
                    anchors.topMargin: 10
                    searchEngine.objectName: "Characters Search Engine"
                }

                ScrollArea {
                    id: charactersFlowScroll
                    anchors.top: searchBar.bottom
                    anchors.bottom: addRelationshipDialogButtons.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10
                    contentWidth: charactersFlow.width
                    contentHeight: charactersFlow.height
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                    ScrollBar.horizontal.policy: charactersFlow.width > width ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                    ScrollBar.horizontal.opacity: active ? 1 : 0.2

                    Flow {
                        id: charactersFlow
                        flow: Flow.TopToBottom
                        height: charactersFlowScroll.height - 4
                        property real columnWidth: 100

                        Repeater {
                            id: characterCheckBoxes
                            model: unrelatedCharacterNames

                            CheckBox2 {
                                id: characterCheckBox
                                text: modelData
                                width: charactersFlow.columnWidth
                                Component.onCompleted: charactersFlow.columnWidth = Math.max(charactersFlow.columnWidth, implicitWidth)
                                checked: false

                                property bool highlight: false
                                background: Rectangle {
                                    color: characterCheckBox.Material.background
                                }
                                Material.background: highlight ? accentColors.c300.background : primaryColors.c10.background
                                Material.foreground: highlight ? accentColors.c300.text : primaryColors.c10.text

                                SearchAgent.engine: searchBar.searchEngine
                                SearchAgent.onSearchRequest: {
                                    SearchAgent.searchResultCount = SearchAgent.indexesOf(string, characterCheckBox.text).length > 0 ? 1 : 0
                                }
                                SearchAgent.onCurrentSearchResultIndexChanged: {
                                    characterCheckBox.highlight = SearchAgent.currentSearchResultIndex >= 0
                                    charactersFlowScroll.ensureItemVisible(characterCheckBox,1,10)
                                }
                                SearchAgent.onClearSearchRequest: characterCheckBox.font.bold = false
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
                            for(var i=0; i<characterCheckBoxes.count; i++) {
                                var checkBox = characterCheckBoxes.itemAt(i)
                                if(checkBox.checked) {
                                    var ch = scriteDocument.structure.addCharacter(checkBox.text)
                                    if(ch)
                                        character.addRelationship("Related To", ch)
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
