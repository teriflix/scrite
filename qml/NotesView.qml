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
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    id: notesView
    property Component listHeader
    property Model notesModel: null
    property string title: "Notes list"

    signal newNoteRequest(color noteColor)
    signal removeNoteRequest(int index)
    onNotesModelChanged: notesList.currentIndex = 0

    ListView {
        id: notesList
        anchors.fill: parent
        model: notesModel
        orientation: Qt.Horizontal
        flickableDirection: Flickable.HorizontalFlick
        property bool scrollBarNeeded: true // contentWidth > width
        property real itemWidth: Math.max( Math.floor(Screen.width/4.5), 350 )
        property real itemHeight: height - (scrollBarNeeded ? 20 : 0)
        highlightMoveDuration: screenplayEditorSettings.enableAnimations ? 250 : 50
        highlightResizeDuration: screenplayEditorSettings.enableAnimations ? 250 : 50
        clip: true
        ScrollBar.horizontal: ScrollBar {
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
        header: Item {
            width: listHeader ? notesList.itemWidth : 0
            height: notesList.itemHeight

            Loader {
                visible: listHeader !== null
                anchors.fill: parent
                anchors.margins: 6
                sourceComponent: listHeader
            }
        }
        delegate: Item {
            property Note note: modelData
            width: notesList.itemWidth
            height: notesList.itemHeight

            Rectangle {
                id: noteItemArea
                anchors.fill: parent
                anchors.margins: 6
                border.width: notesList.currentIndex === index ? 2 : 1
                border.color: note.color
                color: notesList.currentIndex === index ? Qt.tint(note.color, "#c0ffffff") : app.translucent(note.color, 0.25)
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
                        policy: noteItemScroll.contentHeight > noteItemScroll.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
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
                        width: noteItemScroll.width - (noteItemScroll.contentHeight > noteItemScroll.height ? 20 : 0)

                        Rectangle {
                            color: notesList.currentIndex === index ? Qt.tint(note.color, "#c0ffffff") : app.translucent(note.color, 0.25)
                            width: parent.width
                            height: colorNoteButton.height + 4
                            border.width: notesList.currentIndex === index ? 1 : 0
                            border.color: app.textColorFor(note.color)
                            radius: 6

                            Row {
                                width: parent.width - 10
                                spacing: 5
                                anchors.centerIn: parent

                                Text {
                                    id: noteTitleText
                                    font.pointSize: app.idealFontPointSize
                                    font.letterSpacing: 1
                                    text: "Note #" + (index+1)
                                    leftPadding: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width - colorNoteButton.width - removeNoteButton.width - 2*parent.spacing
                                }

                                ToolButton3 {
                                    id: colorNoteButton
                                    anchors.verticalCenter: parent.verticalCenter
                                    iconSource: "../icons/navigation/menu.png"
                                    onClicked: colorMenu.open()
                                    down: colorMenu.visible
                                    enabled: !scriteDocument.readOnly

                                    Item {
                                        anchors.left: parent.left
                                        anchors.top: parent.bottom

                                        ColorMenu {
                                            id: colorMenu
                                            onMenuItemClicked: note.color = color
                                        }
                                    }
                                }

                                ToolButton3 {
                                    id: removeNoteButton
                                    anchors.verticalCenter: parent.verticalCenter
                                    enabled: !scriteDocument.readOnly
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
                        }

                        Item {
                            width: parent.width
                            height: 5
                        }

                        TextField2 {
                            id: headingField
                            width: parent.width
                            height: Math.max(colorNoteButton.height, contentHeight+app.idealFontPointSize+8)
                            anchors.margins: 10
                            label: "Heading"
                            placeholderText: "Heading"
                            font.pointSize: app.idealFontPointSize + 2
                            maximumLength: 256
                            font.bold: true
                            text: note.heading
                            onTextChanged: note.heading = text
                            wrapMode: Text.WordWrap
                            tabItem: contentField
                            enableTransliteration: true
                            readOnly: scriteDocument.readOnly
                            onActiveFocusChanged: notesList.currentIndex = index
                        }

                        TextArea {
                            id: contentField
                            font.pointSize: app.idealFontPointSize
                            selectByMouse: true
                            selectByKeyboard: true
                            text: note.content
                            readOnly: scriteDocument.readOnly
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            Transliterator.textDocument: textDocument
                            Transliterator.cursorPosition: cursorPosition
                            Transliterator.hasActiveFocus: activeFocus
                            onTextChanged: note.content = text
                            placeholderText: "Type note content here"
                            width: parent.width
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
            width: titleLoader.active ? notesList.width : notesList.itemWidth
            height: notesList.itemHeight

            Button2 {
                anchors.centerIn: parent
                text: "Add Note"
                enabled: !scriteDocument.readOnly
                onClicked: newNoteColor.open()
                down: newNoteColor.visible

                Item {
                    anchors.left: parent.left
                    anchors.top: parent.bottom

                    ColorMenu {
                        id: newNoteColor
                        onMenuItemClicked: {
                            newNoteRequest(color)
                            notesList.currentIndex = notesModel.objectCount - 1
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: titleLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.verticalCenter
        active: notesModel && !listHeader ? notesModel.objectCount === 0 : false
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
