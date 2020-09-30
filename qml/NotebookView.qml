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
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    Image {
        anchors.fill: parent
        source: "../images/notebookpage.jpg"
        fillMode: Image.Stretch
        smooth: true
    }

    ListView {
        id: notebookTabsView
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.rightMargin: 3
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        clip: true
        width: 45
        model: noteSources
        spacing: -width*0.4
        currentIndex: 0
        footer: Item {
            width: notebookTabsView.width
            height: width

            RoundButton {
                anchors.centerIn: parent
                hoverEnabled: true
                icon.source: "../icons/content/person_add.png"
                enabled: !scriteDocument.readOnly
                onClicked: {
                    modalDialog.popupSource = this
                    modalDialog.sourceComponent = newCharactersDialogUi
                    modalDialog.active = true
                }
                ToolTip.text: "Click this button to detect characters in your screenplay and create sections for a subset of them in this notebook."
                ToolTip.visible: hovered
            }
        }

        delegate: TabBarTab {
            width: active ? 40 : 35
            height: implicitTabSize
            alignment: Qt.AlignRight

            tabIndex: index
            tabCount: notebookTabsView.count
            currentTabIndex: notebookTabsView.currentIndex

            tabFillColor: active ? modelData.color : Qt.tint(modelData.color, "#C0FFFFFF")
            tabBorderColor: modelData.color

            text: modelData.label.length > 20 ? (modelData.label.substr(0,17)+"...") : modelData.label
            font.pixelSize: active ? 20 : 16
            font.bold: active
            textColor: active ? app.textColorFor(modelData.color) : "black"

            property bool allowRemove: app.typeName(modelData.source) === "Character"
            acceptedMouseButtons: allowRemove ? (Qt.LeftButton|Qt.RightButton) : Qt.LeftButton

            hoverEnabled: true
            ToolTip.visible: hoverEnabled && containsMouse
            ToolTip.text: modelData.label
            ToolTip.delay: 3000

            onRequestActivation: notebookTabsView.currentIndex = index
            onRequestContextMenu: {
                notebookTabsView.currentIndex = index
                characterItemMenu.character = modelData.source
                characterItemMenu.popup(this)
            }
        }
    }

    property var noteSources: []
    function evaluateNoteSources() {
        var currentIndex = notebookTabsView.currentIndex
        notebookTabsView.currentIndex = -1

        var sources = []
        sources.push( {"source": scriteDocument.structure, "label": "Story", "color": "purple" })

        var activeScene = scriteDocument.screenplay.activeScene
        if(activeScene)
            sources.push({"source": activeScene, "label": activeScene.title, "color": activeScene.color})

        var nrCharacters = scriteDocument.structure.characterCount
        for(var i=0; i<nrCharacters; i++) {
            var character = scriteDocument.structure.characterAt(i)
            sources.push({"source": character, "label":character.name, "color": app.pickStandardColor(i)})
        }

        noteSources = sources
        notebookTabsView.currentIndex = currentIndex
    }

    Connections {
        target: scriteDocument.structure
        onCharacterCountChanged: evaluateNoteSources()
    }
    Connections {
        target: scriteDocument.screenplay
        onActiveSceneChanged: {
            evaluateNoteSources()
            notebookTabsView.currentIndex = 1
        }
    }
    Component.onCompleted: evaluateNoteSources()

    Menu2 {
        id: characterItemMenu
        property Character character

        MenuItem2 {
            text: "Remove Section"
            onClicked: {
                notebookTabsView.currentIndex = 0
                scriteDocument.structure.removeCharacter(characterItemMenu.character)
                characterItemMenu.close()
            }
        }
    }

    property color currentTabNoteColor: !scriteDocument.loading && notebookTabsView.currentIndex >= 0 ? noteSources[notebookTabsView.currentIndex].color : "black"
    property var currentTabNotesSource: !scriteDocument.loading && notebookTabsView.currentIndex >= 0 ? noteSources[notebookTabsView.currentIndex].source : scriteDocument.structure

    Rectangle {
        anchors.left: parent.left
        anchors.right: notebookTabsView.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 3
        anchors.topMargin: 3
        anchors.bottomMargin: 3
        anchors.rightMargin: -1
        color: app.translucent(border.color, 0.04)
        radius: 4
        border.width: 2
        border.color: currentTabNoteColor

        Loader {
            anchors.fill: parent
            anchors.margins: 2
            active: !scriteDocument.loading
            sourceComponent: {
                if( app.verifyType(currentTabNotesSource, "Character") )
                    return characterNotesComponent
                return notesViewComponent
            }
        }
    }

    Component {
        id: characterNotesComponent

        CharacterNotes {
            character: currentTabNotesSource
            colorHint: currentTabNoteColor
        }
    }

    Component {
        id: notesViewComponent

        Item {
            Item {
                anchors.fill: parent
                anchors.margins: 2

                Row {
                    id: notesViewTabBar
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    spacing: -height*0.4
                    property int currentIndex: 0

                    Repeater {
                        model: ["Notes", "Relationships"]

                        TabBarTab {
                            tabFillColor: active ? currentTabNoteColor : Qt.tint(currentTabNoteColor, "#C0FFFFFF")
                            tabBorderColor: currentTabNoteColor
                            tabBorderWidth: 1
                            text: modelData
                            tabIndex: index
                            tabCount: 2
                            textColor: active ? app.textColorFor(currentTabNoteColor) : "black"
                            font.pixelSize: active ? 20 : 16
                            font.bold: active
                            currentTabIndex: notesViewTabBar.currentIndex
                            onRequestActivation: notesViewTabBar.currentIndex = index
                        }
                    }
                }

                Rectangle {
                    anchors.top: notesViewTabBar.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    color: Qt.rgba(1,1,1,0.25)
                    border.width: 1
                    border.color: currentTabNoteColor
                    radius: 6

                    NotesView {
                        anchors.fill: parent
                        anchors.margins: 2
                        visible: notesViewTabBar.currentIndex === 0
                        z: visible ? 1 : 0
                        notesModel: scriteDocument.loading ? null : (currentTabNotesSource ? currentTabNotesSource.notesModel : null)
                        onNewNoteRequest: {
                            var note = noteComponent.createObject(currentTabNotesSource)
                            note.color = noteColor
                            currentTabNotesSource.addNote(note)
                        }
                        onRemoveNoteRequest: currentTabNotesSource.removeNote(currentTabNotesSource.noteAt(index))
                        title: {
                            if(notebookTabsView.currentIndex > 0)
                                return "You can capture your thoughts, ideas and research related to '<b>" + currentTabNotesSource.name + "</b>' here.";
                            return "You can capture your thoughts, ideas and research about your screenplay here.";
                        }
                    }

                    CharacterRelationshipsGraphView {
                        anchors.fill: parent
                        anchors.margins: 2
                        visible: notesViewTabBar.currentIndex === 1
                        z: visible ? 1 : 0
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
        id: newCharactersDialogUi

        Rectangle {
            width: 800
            height: 680
            color: primaryColors.c10.background

            Item {
                anchors.fill: parent
                anchors.margins: 20

                Text {
                    id: title
                    width: parent.width
                    anchors.top: parent.top
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    text: "Check the characters for which you want to create sections in the notebook."
                    wrapMode: Text.WordWrap
                }

                CharactersView {
                    id: charactersListView
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: title.bottom
                    anchors.bottom: createSectionsButton.top
                    anchors.topMargin: 20
                    anchors.bottomMargin: 10
                    charactersModel.array: scriteDocument.structure.detectCharacters()
                    charactersModel.objectMembers: ["name", "added"]
                    sortFilterRole: charactersModel.objectMemberRole("name")
                }

                Button2 {
                    id: createSectionsButton
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    text: "Create Sections"
                    onClicked: {
                        scriteDocument.structure.addCharacters(charactersListView.selectedCharacters)
                        modalDialog.closeRequest()
                    }
                }
            }
        }
    }
}
