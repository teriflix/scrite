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
        anchors.left: notesGrid.left
        anchors.top: notesGrid.top
        anchors.bottom: notesGrid.bottom
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

    GridView {
        id: notesGrid
        width: notesGrid.width
        anchors.left: parent.left
        anchors.right: notebookTabsView.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 5
        clip: true
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            opacity: active ? 1 : 0.2
            Behavior on opacity {
                enabled: screenplayEditorSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }
        }

        property real minimumCellWidth: 450
        property int nrCells: Math.floor(width/minimumCellWidth)

        cellWidth: width/nrCells
        cellHeight: 500

        model: notesPack ? notesPack.noteCount+1 : 0

        delegate: Item {
            width: notesGrid.cellWidth
            height: notesGrid.cellHeight

            Loader {
                anchors.fill: parent
                anchors.rightMargin: ((index+1)%notesGrid.nrCells)===0 ? 20 : 5
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
                        Behavior on color {
                            enabled: screenplayEditorSettings.enableAnimations
                            ColorAnimation { duration: 500 }
                        }
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
                                        font.pixelSize: 20
                                        background: Item { }
                                        leftPadding: 10
                                        rightPadding: 10
                                        // renderType: Text.NativeRendering
                                        selectByMouse: true
                                        selectByKeyboard: true
                                        onTextChanged: {
                                            if(activeFocus)
                                                note.heading = text
                                        }
                                        readOnly: scriteDocument.readOnly
                                        palette: app.palette
                                        Keys.onReturnPressed: editingFinished()
                                        anchors.verticalCenter: parent.verticalCenter
                                        KeyNavigation.tab: contentEdit
                                        Transliterator.textDocument: textDocument
                                        Transliterator.cursorPosition: cursorPosition
                                        Transliterator.hasActiveFocus: activeFocus
                                    }

                                    ToolButton3 {
                                        id: menuButton
                                        iconSource: "../icons/navigation/menu.png"
                                        anchors.verticalCenter: parent.verticalCenter
                                        down: noteMenuLoader.item.visible
                                        enabled: !scriteDocument.readOnly
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
                                                onMenuItemClicked: note.color = color
                                            }
                                        }
                                    }

                                    ToolButton3 {
                                        id: deleteButton
                                        iconSource: "../icons/action/delete.png"
                                        anchors.verticalCenter: parent.verticalCenter
                                        onClicked: notesPack.removeNote(note)
                                        enabled: !scriteDocument.readOnly
                                    }
                                }
                            }

                            TextArea {
                                id: contentEdit
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: note.content
                                textFormat: TextArea.PlainText
                                background: Item { }
                                leftPadding: 10
                                rightPadding: 10
                                // renderType: Text.NativeRendering
                                font.pixelSize: 18
                                onTextChanged: {
                                    if(activeFocus)
                                        note.content = text
                                }
                                readOnly: scriteDocument.readOnly
                                palette: app.palette
                                selectByMouse: true
                                selectByKeyboard: true
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
            visible: !scriteDocument.readOnly
            enabled: !scriteDocument.readOnly

            Rectangle {
                anchors.fill: parent
                anchors.margins: 10
                radius: 5
                color: primaryColors.windowColor
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
            width: 800
            height: 680
            color: primaryColors.c10.background

            Item {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    id: title
                    width: parent.width
                    anchors.top: parent.top
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    text: "Check the characters for which you want to create sections in the notebook"
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
