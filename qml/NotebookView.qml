/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
    property var tabColors: ["#6600cc", "#ff3300", "#0000cc", "#993300", "#006600", "#660066", "#003300", "#6600ff", "#999966"]

    Image {
        anchors.fill: parent
        source: "../images/notebookpage.jpg"
        fillMode: Image.Stretch
        smooth: true
        opacity: 0.5
    }

    ListView {
        id: notebookTabsView
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 3
        width: 50
        model: noteSources
        spacing: 3
        currentIndex: 0
        footer: Item {
            width: notebookTabsView.width
            height: width

            RoundButton {
                anchors.centerIn: parent
                hoverEnabled: true
                icon.source: "../icons/navigation/refresh.png"
                onClicked: {
                    modalDialog.popupSource = this
                    modalDialog.sourceComponent = newCharactersDialogUi
                    modalDialog.active = true
                }
                ToolTip.text: "Click this button to detect characters in your screenplay and create sections for a subset of them in this notebook."
                ToolTip.visible: hovered
            }
        }

        delegate: Rectangle {
            width: notebookTabsView.width
            height: textItem.width + 20
            color: selected ? modelData.color : Qt.tint(modelData.color, "#C0FFFFFF")
            border { width: 1; color: "lightgray" }
            radius: 8

            property bool selected: notebookTabsView.currentIndex === index

            Text {
                id: textItem
                rotation: 90
                text: modelData.label.length > 20 ? (modelData.label.substr(0,17)+"...") : modelData.label
                anchors.centerIn: parent
                font.pixelSize: parent.selected ? 20 : 18
                font.bold: parent.selected
                color: parent.selected ? "white" : "black"
                Behavior on font.pixelSize { NumberAnimation { duration: 250 } }
                Behavior on color { ColorAnimation { duration: 125 } }
            }

            MouseArea {
                property bool allowRemove: app.typeName(modelData.source) === "Character"
                anchors.fill: parent
                acceptedButtons: allowRemove ? Qt.LeftButton : (Qt.LeftButton|Qt.RightButton)
                hoverEnabled: modelData.label.length > 20
                ToolTip.visible: hoverEnabled && containsMouse
                ToolTip.text: modelData.label
                onClicked: {
                    notebookTabsView.currentIndex = index
                    if(allowRemove && mouse.button === Qt.RightButton) {
                        characterItemMenu.character = modelData.source
                        characterItemMenu.popup(this)
                    }
                }
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
            sources.push({"source":character, "label":character.name, "color": tabColors[i%tabColors.length]})
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

    Menu {
        id: characterItemMenu
        property Character character

        MenuItem {
            text: "Remove Section"
            onClicked: {
                scriteDocument.structure.removeCharacter(characterItemMenu.character)
                characterItemMenu.close()
            }
        }
    }

    property var notesPack: notebookTabsView.currentIndex >= 0 ? noteSources[notebookTabsView.currentIndex].source : scriteDocument.structure

    GridView {
        id: notesGrid
        width: notesGrid.width
        anchors.left: parent.left
        anchors.right: notebookTabsView.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 5
        clip: true
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AlwaysOn }

        property real minimumCellWidth: 340
        property int nrCells: Math.floor(width/minimumCellWidth)

        cellWidth: width/nrCells
        cellHeight: 400

        model: notesPack ? notesPack.noteCount+1 : 0

        delegate: Item {
            width: notesGrid.cellWidth
            height: notesGrid.cellHeight

            Loader {
                anchors.fill: parent
                anchors.rightMargin: (index%notesGrid.nrCells) ? 20 : 5
                property int noteIndex: index < notesPack.noteCount ? index : -1
                sourceComponent: noteIndex >= 0 ? noteDelegate : newNoteDelegate
                active: true
            }
        }
    }

    Loader {
        anchors.left: notesGrid.left
        anchors.right: notesGrid.right
        anchors.bottom: notesGrid.bottom
        anchors.top: notesGrid.verticalCenter
        active: notesPack ? notesPack.noteCount === 0 : false
        sourceComponent: Item {
            Text {
                anchors.fill: parent
                anchors.margins: 30
                font.pixelSize: 30
                font.letterSpacing: 1
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                lineHeight: 1.2
                text: {
                    if(notebookTabsView.currentIndex > 0)
                        return "You can capture your thoughts, ideas and research related to '<b>" + notesPack.name + "</b>' here.";
                    return "You can capture your thoughts, ideas and research about your screenplay here.";
                }
            }
        }
    }

    Component {
        id: noteDelegate

        Item {
            id: noteItem
            property Note note: notesPack.noteAt(noteIndex)

            Loader {
                anchors.fill: parent
                anchors.margins: 10
                active: parent.note !== null
                sourceComponent: Item {
                    Rectangle {
                        anchors.fill: parent
                        color: Qt.tint(note.color, "#C0FFFFFF")
                        border.width: 2
                        border.color: (note.color === Qt.rgba(1,1,1,1)) ? "black" : note.color
                        radius: 5
                        Behavior on color {  ColorAnimation { duration: 500 } }
                    }

                    ScrollView {
                        id: noteScrollView
                        anchors.fill: parent
                        anchors.margins: 5
                        clip: true

                        Column {
                            width: noteScrollView.width
                            spacing: 10

                            Rectangle {
                                id: noteTitleBar
                                width: parent.width
                                height: noteTitleBarLayout.height+8
                                color: notesGrid.currentIndex === noteIndex ? Qt.rgba(0,0,0,0.25) : Qt.rgba(0,0,0,0)
                                radius: 5

                                Row {
                                    id: noteTitleBarLayout
                                    spacing: 5
                                    width: parent.width-4
                                    anchors.centerIn: parent

                                    TextArea {
                                        id: headingEdit
                                        width: parent.width-menuButton.width-deleteButton.width-2*parent.spacing
                                        wrapMode: Text.WordWrap
                                        text: note.heading
                                        font.bold: true
                                        font.pixelSize: 18
                                        renderType: Text.NativeRendering
                                        onTextChanged: {
                                            if(activeFocus)
                                                note.heading = text
                                        }
                                        palette: app.palette
                                        Keys.onReturnPressed: editingFinished()
                                        anchors.verticalCenter: parent.verticalCenter
                                        KeyNavigation.tab: contentEdit
                                        Transliterator.textDocument: textDocument
                                        Transliterator.cursorPosition: cursorPosition
                                        Transliterator.hasActiveFocus: activeFocus
                                    }

                                    ToolButton {
                                        id: menuButton
                                        icon.source: "../icons/navigation/menu.png"
                                        anchors.verticalCenter: parent.verticalCenter
                                        down: noteMenuLoader.item.visible
                                        onClicked: {
                                            if(noteMenuLoader.item.visible)
                                                noteMenuLoader.item.close()
                                            else
                                                noteMenuLoader.item.open()
                                        }
                                        flat: true

                                        Loader {
                                            id: noteMenuLoader
                                            width: parent.width; height: 1
                                            anchors.top: parent.bottom
                                            sourceComponent: ColorMenu { }
                                            active: true

                                            Connections {
                                                target: noteMenuLoader.item
                                                onMenuItemClicked: note.color = color
                                            }
                                        }
                                    }

                                    ToolButton {
                                        id: deleteButton
                                        icon.source: "../icons/action/delete.png"
                                        anchors.verticalCenter: parent.verticalCenter
                                        onClicked: notesPack.removeNote(note)
                                        flat: true
                                    }
                                }
                            }

                            TextArea {
                                id: contentEdit
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: note.content
                                textFormat: TextArea.PlainText
                                renderType: Text.NativeRendering
                                font.pixelSize: 16
                                onTextChanged: {
                                    if(activeFocus)
                                        note.content = text
                                }
                                palette: app.palette
                                placeholderText: "type the contents of your note here.."
                                KeyNavigation.tab: headingEdit
                                Transliterator.textDocument: textDocument
                                Transliterator.cursorPosition: cursorPosition
                                Transliterator.hasActiveFocus: activeFocus
                            }
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                enabled: notesGrid.currentIndex !== noteIndex
                onClicked: notesGrid.currentIndex = noteIndex
            }
        }
    }

    Component {
        id: newNoteDelegate

        Item {
            Rectangle {
                anchors.fill: parent
                anchors.margins: 10
                radius: 5
                color: "lightgray"
                opacity: 0.25
            }

            RoundButton {
                width: 80; height: 80
                anchors.centerIn: parent
                icon.width: 48
                icon.height: 48
                icon.source: "../icons/action/note_add.png"
                down: noteMenuLoader.item.visible
                onClicked: {
                    if(noteMenuLoader.item.visible)
                        noteMenuLoader.item.close()
                    else
                        noteMenuLoader.item.open()
                }

                Loader {
                    id: noteMenuLoader
                    width: parent.width; height: 1
                    anchors.top: parent.bottom
                    sourceComponent: ColorMenu { }
                    active: true

                    Connections {
                        target: noteMenuLoader.item
                        onMenuItemClicked: {
                            var props = {"color": color}
                            var note = noteComponent.createObject(scriteDocument.structure, props)
                            notesPack.addNote(note)
                            notesGrid.currentIndex = notesPack.noteCount-1
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
        }
    }

    Component {
        id: newCharactersDialogUi

        Rectangle {
            width: 400
            height: 600
            color: "white"

            Item {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    id: title
                    width: parent.width
                    anchors.top: parent.top
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    text: "Create sections in your notebook for characters in your screenplay"
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    anchors.fill: charactersListView
                    anchors.margins: -2
                    border { width: 1; color: "black" }
                }

                ListView {
                    id: charactersListView
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: title.bottom
                    anchors.bottom: createSectionsButton.top
                    anchors.topMargin: 20
                    anchors.bottomMargin: 10
                    clip: true
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AlwaysOn }

                    property var detectedCharacters: scriteDocument.structure.detectCharacters()
                    property var newCharacters: []

                    model: detectedCharacters
                    spacing: 10
                    delegate: Row {
                        width: charactersListView.width
                        spacing: 10

                        CheckBox {
                            checkable: true
                            checked: modelData.added
                            anchors.verticalCenter: parent.verticalCenter
                            enabled: modelData.added === false
                            onToggled: {
                                var chs = charactersListView.newCharacters
                                if(checked)
                                    chs.push(modelData.name)
                                else
                                    chs.splice( chs.indexOf(modelData.name), 1 )
                                charactersListView.newCharacters = chs
                            }
                        }

                        Text {
                            font.pixelSize: 15
                            text: modelData.name
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                Button {
                    id: createSectionsButton
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    text: "Create Sections"
                    onClicked: {
                        scriteDocument.structure.addCharacters(charactersListView.newCharacters)
                        modalDialog.closeRequest()
                    }
                }
            }
        }
    }
}
