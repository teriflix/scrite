/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"

AbstractNotebookPage {
    id: root

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.

    GridView {
        id: _gridView

        property int __columnCount: Math.floor(width/__idealCellWidth)
        property real __idealCellWidth: Math.min(250,width)

        ScrollBar.vertical: _scrollBar

        anchors.fill: parent
        anchors.rightMargin: contentHeight > height ? 17 : 12

        model: notebookModel.bookmarkedNotes

        cellWidth: width/__columnCount
        cellHeight: 150
        highlightMoveDuration: 0

        highlight: Item {
            BoxShadow {
                anchors.fill: highlightedItem
                opacity: 0.5
            }
            Item {
                id: highlightedItem
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.topMargin: 12
            }
        }

        delegate: Item {
            id: _delegate

            required property int index
            required property string noteTitle
            required property string noteSummary
            required property QtObject noteObject

            width: _gridView.cellWidth
            height: _gridView.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 12
                anchors.leftMargin: 12

                border.width: 1
                border.color: _gridView.currentIndex === _delegate.index ? "darkgray" : Runtime.colors.primary.borderColor

                Column {
                    anchors.fill: parent
                    anchors.margins: 10

                    spacing: 10

                    Row {
                        width: parent.width
                        spacing: 5

                        Image {
                            anchors.verticalCenter: parent.verticalCenter

                            width: 32
                            height: 32

                            mipmap: true

                            source: {
                                if(Object.typeOf(_delegate.noteObject) === "Notes") {
                                    switch(_delegate.noteObject.ownerType) {
                                    case Notes.SceneOwner:
                                        return "qrc:/icons/content/scene.png"
                                    case Notes.CharacterOwner:
                                        return "qrc:/icons/content/person_outline.png"
                                    case Notes.BreakOwner:
                                        return "qrc:/icons/content/story.png"
                                    default:
                                        break
                                    }
                                } else if(Object.typeOf(_delegate.noteObject) === "Character")
                                    return "qrc:/icons/content/person_outline.png"
                                else if(Object.typeOf(_delegate.noteObject) === "Note") {
                                    switch(_delegate.noteObject.type) {
                                    case Note.TextNoteType:
                                        return "qrc:/icons/content/note.png"
                                    case Note.FormNoteType:
                                        return "qrc:/icons/content/form.png"
                                    default:
                                        break
                                    }
                                }
                                return "qrc:/icons/content/bookmark.png"
                            }
                        }

                        VclLabel {
                            id: _headingText

                            anchors.verticalCenter: parent.verticalCenter

                            elide: Text.ElideRight
                            maximumLineCount: 1
                            text: _delegate.noteTitle
                            width: parent.width-32-parent.spacing

                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        }
                    }

                    VclLabel {
                        width: parent.width
                        height: parent.height - _headingText.height - parent.spacing

                        color: _headingText.color
                        elide: Text.ElideRight
                        opacity: 0.75
                        text: _delegate.noteSummary
                        wrapMode: Text.WordWrap

                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }
                }
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    _gridView.currentIndex = _delegate.index
                }

                onDoubleClicked: {
                    _gridView.currentIndex = _delegate.index
                    root.switchRequest(_delegate.noteObject)
                }
            }
        }
    }

    VclScrollBar {
        id: _scrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        flickable: _gridView
        orientation: Qt.Vertical
    }
}
