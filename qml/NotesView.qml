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
    id: notesView
    property Model notesModel: null
    property string title: "Notes list"

    signal newNoteRequest()
    signal removeNoteRequest(int index)
    onNotesModelChanged: notesList.currentIndex = 0

    ListView {
        id: notesList
        anchors.fill: parent
        model: notesModel
        orientation: Qt.Horizontal
        flickableDirection: Flickable.HorizontalFlick
        property real itemSize: Math.max((notesView.width-2*spacing)/3, 300)
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        clip: true
        ScrollBar.horizontal: ScrollBar {
            policy: notesList.contentWidth > notesList.width ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
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
        delegate: Item {
            property Note note: modelData
            width: notesList.itemSize
            height: notesList.contentWidth > notesList.width ? notesList.height - 20 : notesList.height

            Rectangle {
                id: noteItemArea
                anchors.fill: parent
                anchors.margins: 10
                border.width: notesList.currentIndex === index ? 2 : 1
                border.color: note.color
                color: app.translucent(note.color, 0.25)
                radius: 6

                Flickable {
                    id: noteItemScroll
                    anchors.fill: parent
                    anchors.margins: 20
                    flickableDirection: Flickable.VerticalFlick
                    contentHeight: noteItemContent.height
                    contentWidth: noteItemContent.width
                    clip: true

                    ScrollBar.vertical: ScrollBar {
                        policy: noteItemScroll.contentHeight > notesList.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
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
                        id: noteItemContent
                        spacing: 10
                        width: noteItemScroll.width - (noteItemScroll.contentHeight > notesList.height ? 20 : 0)

                        Item {
                            width: parent.width
                            height: Math.max(removeNoteButton.height, headingField.height) + 20

                            TextField2 {
                                id: headingField
                                anchors.left: parent.left
                                anchors.right: colorNoteButton.left
                                anchors.verticalCenter: parent.verticalCenter
                                height: Math.max(colorNoteButton.height, contentHeight)
                                anchors.margins: 10
                                label: "Heading"
                                placeholderText: "Heading"
                                font.pointSize: app.idealFontPointSize + 2
                                font.bold: true
                                text: note.heading
                                onTextChanged: note.heading = text
                                wrapMode: Text.WordWrap
                                width: parent.width
                                tabItem: contentField
                                enableTransliteration: true
                                readOnly: scriteDocument.readOnly
                                onActiveFocusChanged: notesList.currentIndex = index
                            }

                            ToolButton3 {
                                id: colorNoteButton
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: removeNoteButton.left
                                anchors.margins: 10
                                iconSource: "../icons/navigation/menu.png"
                                onClicked: colorMenu.open()

                                ColorMenu {
                                    id: colorMenu
                                    onMenuItemClicked: note.color = color
                                }
                            }

                            ToolButton3 {
                                id: removeNoteButton
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                iconSource: "../icons/action/delete.png"
                                onClicked: {
                                    if(index+1 < notesModel.objectCount)
                                        notesList.currentIndex = index+1
                                    else
                                        notesList.currentIndex = 0
                                    removeNoteRequest(index)
                                }
                            }
                        }

                        TextArea {
                            id: contentField
                            font.pointSize: app.idealFontPointSize
                            selectByMouse: true
                            selectByKeyboard: true
                            text: note.content
                            leftPadding: 10
                            rightPadding: 10
                            topPadding: 10
                            bottomPadding: 10
                            readOnly: scriteDocument.readOnly
                            wrapMode: Text.WordWrap
                            Transliterator.textDocument: textDocument
                            Transliterator.cursorPosition: cursorPosition
                            Transliterator.hasActiveFocus: activeFocus
                            onTextChanged: note.content = text
                            placeholderText: "Type note content here"
                            width: parent.width - 20
                            height: Math.max(contentHeight+50, parent.width)
                            onActiveFocusChanged: notesList.currentIndex = index
                            onCursorRectangleChanged: {
                                if(cursorRectangle.y < 20) {
                                    noteItemScroll.contentY = 0
                                    return
                                }
                                var pt = noteItemContent.mapFromItem(contentField, cursorRectangle.x, cursorRectangle.y)
                                if(pt.y < noteItemScroll.contentY)
                                    noteItemScroll.contentY = Math.max(pt.y-20, 0)
                                else if(pt.y + cursorRectangle.height > noteItemScroll.contentY + noteItemScroll.height)
                                    noteItemScroll.contentY = Math.max( Math.max(noteItemContent.height,pt.y+cursorRectangle.height+20) -noteItemScroll.height, 0)
                            }
                            background: Item { }
                            SpecialSymbolsSupport {
                                anchors.top: parent.bottom
                                anchors.left: parent.left
                                textEditor: contentField
                                textEditorHasCursorInterface: true
                                enabled: !scriteDocument.readOnly
                            }
                        }
                    }
                }
            }
        }
        footer: Item {
            width: notesList.itemSize
            height: notesList.height

            Button2 {
                anchors.centerIn: parent
                text: "Add Note"
                enabled: !scriteDocument.readOnly
                onClicked: {
                    newNoteRequest()
                    notesList.currentIndex = notesModel.objectCount - 1
                }
            }
        }
    }

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.verticalCenter
        active: notesModel ? notesModel.objectCount === 0 : false
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
                text: title
            }
        }
    }
}
