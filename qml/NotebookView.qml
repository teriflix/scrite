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

        property int currentIndex: notebookTabsPanel.currentIndex
        onCurrentIndexChanged: fetchCurrents()
        onRefreshed: fetchCurrents()

        function fetchCurrents() {
            currentTabSource = tabSourceAt(currentIndex)
            currentTabColor = tabColorAt(currentIndex)
            currentTabLabel = tabLabelAt(currentIndex)
            currentTabGroup = tabGroupAt(currentIndex)
        }

        property var currentTabSource
        property color currentTabColor: "white"
        property string currentTabLabel: "none"
        property string currentTabGroup: "none"
    }

    function switchToStoryTab() {
        notebookTabsPanel.currentIndex = 0
    }

    function switchToSceneTab() {
        // if(noteSources.activeScene === scene)
        notebookTabsPanel.currentIndex = 1
    }

    function switchToCharacterTab(name) {
        var idx = noteSources.indexOfLabel(name);
        if(idx >= 0)
            notebookTabsPanel.currentIndex = idx
    }

    SidePanel {
        id: notebookTabsPanel
        height: parent.height
        buttonY: 5
        label: ""
        z: 1
        property int currentIndex: 0
        maxPanelWidth: Math.max(parent.width*0.2, 300)
        expanded: true

        content: ListView {
            id: notebookTabsListView
            clip: true
            model: noteSources
            ScrollBar.vertical: ScrollBar {
                policy: notebookTabsListView.contentHeight > notebookTabsListView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
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
            footer: Item {
                width: notebookTabsListView.width-1
                height: 50

                ToolButton3 {
                    enabled: !scriteDocument.readOnly
                    iconSource: "../icons/content/person_add.png"
                    anchors.centerIn: parent
                    ToolTip.text: "Create notebook page for a character."
                    onClicked: {
                        modalDialog.popupSource = this
                        modalDialog.sourceComponent = newCharactersDialogUi
                        modalDialog.active = true
                    }
                }
            }
            delegate: Rectangle {
                color: Qt.tint(tabColor, (notebookTabsListView.currentIndex === index ? "#9CFFFFFF" : "#E7FFFFFF"))
                width: notebookTabsListView.width-1
                height: tabLabelText.height

                Text {
                    id: tabLabelText
                    width: parent.width
                    padding: 10
                    text: tabLabel
                    elide: Text.ElideRight
                    color: app.isLightColor(parent.color) ? "black" : "white"
                    font.pointSize: app.idealFontPointSize
                    font.bold: notebookTabsListView.currentIndex === index

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: scriteDocument.readOnly ? Qt.LeftButton : (Qt.LeftButton | Qt.RightButton)
                        onClicked: {
                            if(mouse.button === Qt.LeftButton)
                                notebookTabsListView.currentIndex = index
                            else if(mouse.button === Qt.RightButton) {
                                if(tabGroup === "Character") {
                                    characterItemMenu.character = tabSource
                                    characterItemMenu.popup()
                                }
                            }
                        }
                    }
                }
            }
            section.property: "tabGroup"
            section.criteria: ViewSection.FullString
            section.delegate: Rectangle {
                color: primaryColors.c400.background
                width: notebookTabsListView.width-1
                height: groupText.height

                Text {
                    id: groupText
                    text: section
                    color: primaryColors.c400.text
                    elide: Text.ElideMiddle
                    padding: 10
                    font.bold: true
                    font.capitalization: Font.AllUppercase
                    font.pointSize: app.idealFontPointSize-2
                    anchors.verticalCenter: parent.verticalCenter
                }

                ToolButton3 {
                    visible: !scriteDocument.readOnly && section === "Character"
                    iconSource: "../icons/content/person_add.png"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: groupText.right
                    anchors.leftMargin: 10
                    height: groupText.height
                    ToolTip.text: "Create notebook page for a character."
                    onClicked: {
                        modalDialog.popupSource = this
                        modalDialog.sourceComponent = newCharactersDialogUi
                        modalDialog.active = true
                    }
                }
            }
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            currentIndex: notebookTabsPanel.currentIndex
            onCurrentIndexChanged: notebookTabsPanel.currentIndex = currentIndex
        }
    }

    Menu2 {
        id: characterItemMenu
        property Character character

        MenuItem2 {
            text: "Delete"
            enabled: !scriteDocument.readOnly
            onClicked: {
                notebookTabsPanel.currentIndex = 0
                scriteDocument.structure.removeCharacter(characterItemMenu.character)
                characterItemMenu.close()
            }
        }
    }

    property color currentTabNoteColor: !scriteDocument.loading && notebookTabsPanel.currentIndex >= 0 ? noteSources.currentTabColor : "black"
    property var currentTabNotesSource: !scriteDocument.loading && notebookTabsPanel.currentIndex >= 0 ? noteSources.currentTabSource : scriteDocument.structure

    Rectangle {
        anchors.left: notebookTabsPanel.width == notebookTabsPanel.minPanelWidth ? parent.left : notebookTabsPanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: app.translucent(border.color, 0.04)
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
                            if(notebookTabsPanel.currentTabIndex > 0)
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
