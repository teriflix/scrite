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
        onActiveSceneChanged: evaluateNoteSources()
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

    property var notesPack: notebookTabsView.currentIndex >= 0 ? noteSources[notebookTabsView.currentIndex].source : scriteDocument.structure

    Rectangle {
        anchors.left: notesView.left
        anchors.top: notesView.top
        anchors.bottom: notesView.bottom
        anchors.right: notebookTabsView.left
        anchors.leftMargin: -2
        anchors.topMargin: -2
        anchors.bottomMargin: -2
        anchors.rightMargin: -1
        color: app.translucent(border.color, 0.04)
        radius: 4
        border.width: 2
        border.color: notebookTabsView.currentIndex >= 0 ? noteSources[notebookTabsView.currentIndex].color : "black"
    }

    NotesView {
        id: notesView
        anchors.left: parent.left
        anchors.right: notebookTabsView.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 5
        notesModel: scriteDocument.loading ? null : (notesPack ? notesPack.notesModel : null)
        onNewNoteRequest: notesPack.addNote(noteComponent.createObject(notesPack))
        onRemoveNoteRequest: notesPack.removeNote(notesPack.noteAt(index))
        title: {
            if(notebookTabsView.currentIndex > 0)
                return "You can capture your thoughts, ideas and research related to '<b>" + notesPack.name + "</b>' here.";
            return "You can capture your thoughts, ideas and research about your screenplay here.";
        }
    }

    Component {
        id: noteComponent

        Note {
            heading: "Note Heading"
            color: {
                var lastNote = notesPack.notesModel.objectAt(notesPack.notesModel.objectCount-1)
                if(lastNote)
                    return lastNote.color
                return "white"
            }
        }
    }
}
