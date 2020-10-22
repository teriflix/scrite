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
import QtQuick.Controls 2.13

import Scrite 1.0

Item {
    Image {
        anchors.fill: parent
        source: "../images/notebookpage.jpg"
        fillMode: Image.Stretch
        smooth: true
    }

    NotebookTabModel {
        id: noteSources
        structure: scriteDocument.loading ? null : scriteDocument.structure
        activeScene: {
            if(scriteDocument.screenplay.activeScene)
                return scriteDocument.screenplay.activeScene
            var idx = scriteDocument.structure.currentElementIndex
            if(idx >= 0) {
                var se = scriteDocument.structure.elementAt(idx)
                if(se.scene)
                    return se.scene
            }
            return null
        }

        property int currentIndex: notebookTabsView.currentIndex
        onCurrentIndexChanged: fetchCurrents()
        onRefreshed: fetchCurrents()

        function fetchCurrents() {
            currentSource = sourceAt(currentIndex)
            currentColor = colorAt(currentIndex)
            currentLabel = labelAt(currentIndex)
        }

        property var currentSource
        property color currentColor: "white"
        property string currentLabel: "none"
    }

    function switchToStoryTab() {
        notebookTabsView.currentIndex = 0
    }

    function switchToSceneTab(scene) {
        // if(noteSources.activeScene === scene)
        notebookTabsView.currentIndex = 1
    }

    function switchToCharacterTab(name) {
        var idx = noteSources.indexOfLabel(name);
        if(idx >= 0)
            notebookTabsView.currentIndex = idx
    }

    ListView {
        id: notebookTabsView
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: tabScrollButtons.top
        anchors.rightMargin: 3
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        clip: true
        width: 45
        model: noteSources
        spacing: -width*0.4
        currentIndex: 0
        highlightMoveDuration: 0
        footer: Item {
            width: notebookTabsView.width
            height: width * 1.5

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

    Column {
        id: tabScrollButtons
        width: notebookTabsView.width
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: -10

        ToolButton {
            icon.source: "../icons/navigation/keyboard_arrow_up.png"
            width: parent.width
            enabled: notebookTabsView.currentIndex > 0
            onClicked: notebookTabsView.currentIndex = Math.max(0, notebookTabsView.currentIndex-1)
            ToolTip.text: "Click to switch to the previous tab"
            ToolTip.visible: hovered
            ToolTip.delay: 1000
            hoverEnabled: true
        }

        ToolButton {
            icon.source: "../icons/navigation/keyboard_arrow_down.png"
            width: parent.width
            enabled: notebookTabsView.currentIndex < notebookTabsView.count-1
            onClicked: notebookTabsView.currentIndex = Math.min(notebookTabsView.count-1, notebookTabsView.currentIndex+1)
            ToolTip.text: "Click to switch to the next tab"
            ToolTip.visible: hovered
            ToolTip.delay: 1000
            hoverEnabled: true
        }
    }

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

    property color currentTabNoteColor: !scriteDocument.loading && notebookTabsView.currentIndex >= 0 ? noteSources.currentColor : "black"
    property var currentTabNotesSource: !scriteDocument.loading && notebookTabsView.currentIndex >= 0 ? noteSources.currentSource : scriteDocument.structure

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
            showCharacterInfoInNotesTab: workspaceSettings.showNotebookInStructure
            onCharacterDoubleClicked: switchToCharacterTab(characterName)
        }
    }

    Component {
        id: notesViewComponent

        Item {
            TabView3 {
                id: notesTabView
                anchors.fill: parent
                anchors.margins: 2
                tabNames: ["Relationships", "(" + currentTabNotesSource.noteCount + ") Notes"]
                currentTabIndex: notebookSettings.activeTab
                onCurrentTabIndexChanged: notebookSettings.activeTab = currentTabIndex
                tabColor: currentTabNoteColor
                currentTabContent: Item {
                    CharacterRelationshipsGraphView {
                        anchors.fill: parent
                        anchors.margins: 2
                        visible: notesTabView.currentTabIndex === 0
                        z: visible ? 1 : 0
                        scene: app.verifyType(currentTabNotesSource, "Scene") ? currentTabNotesSource : null
                        onCharacterDoubleClicked: switchToCharacterTab(characterName)
                    }

                    NotesView {
                        anchors.fill: parent
                        anchors.margins: 2
                        visible: notesTabView.currentTabIndex === 1
                        z: visible ? 1 : 0
                        notesModel: scriteDocument.loading ? null : (currentTabNotesSource ? currentTabNotesSource.notesModel : null)
                        onNewNoteRequest: {
                            var note = noteComponent.createObject(currentTabNotesSource)
                            note.color = noteColor
                            currentTabNotesSource.addNote(note)
                        }
                        onRemoveNoteRequest: currentTabNotesSource.removeNote(currentTabNotesSource.noteAt(index))
                        title: {
                            if(notebookTabsView.currentTabIndex > 0)
                                return "You can capture your thoughts, ideas and research related to '<b>" + currentTabNotesSource.name + "</b>' here.";
                            return "You can capture your thoughts, ideas and research about your screenplay here.";
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
