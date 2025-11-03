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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"

Item {
    id: root

    // This should be set to styleData context variable in NotesTreeView.
    //
    // In that case, why not just use styleData?
    // I want to avoid using leap-of-faith variable names as much as possible.
    // Within the context of this item, styleData would be leap-of-faith.
    //
    // Alright, fair enough. But why not just make styleData as required property?
    // That would have been perfect, except QML Engine (atleast in 5.15.x) complains
    // if we name a required property the same as context variable offered by the
    // engine while instantiating TreeView delegates.
    required property var itemData

    required property real treeViewWidth

    signal makeCurrentRequest()
    signal noteMenuRequest(Note note)
    signal characterMenuRequest(Character character)

    Rectangle {
        id: _container

        x: -parent.x
        width: root.treeViewWidth
        height: _layout.height

        color: {
            if(itemData.selected)
                return Runtime.colors.primary.highlight.background

            let baseColor = undefined

            if(itemData.value.notebookItemType === NotebookModel.NotesType) {
                switch(itemData.value.notebookItemObject.ownerType) {
                case Notes.SceneOwner:
                case Notes.CharacterOwner:
                    baseColor = itemData.value.notebookItemObject.color
                    break
                default:
                    break
                }
            } else if(itemData.value.notebookItemType === NotebookModel.NoteType)
                baseColor = itemData.value.notebookItemObject.color

            if(baseColor)
                return Qt.tint(baseColor, Runtime.colors.sceneHeadingTint)

            return Runtime.colors.primary.c10.background
        }
    }

    Row {
        id: _layout

        width: root.treeViewWidth - parent.x
        height: Runtime.idealFontMetrics.height + 20

        spacing: 5

        Image {
            id: _icon

            anchors.verticalCenter: parent.verticalCenter

            width: parent.height * 0.6
            height: width

            mipmap: true
            visible: source != ""
            opacity: {
                switch(itemData.value.notebookItemType) {
                case NotebookModel.EpisodeBreakType:
                case NotebookModel.ActBreakType:
                    return itemData.value.notebookItemObject ? 1 : 0.5
                }
                return 1
            }

            source: {
                switch(itemData.value.notebookItemType) {
                case NotebookModel.EpisodeBreakType:
                    return "qrc:/icons/content/episode.png"
                case NotebookModel.ActBreakType:
                    return "qrc:/icons/content/act.png"
                case NotebookModel.NotesType:
                    switch(itemData.value.notebookItemObject.ownerType) {
                    case Notes.SceneOwner:
                        return "qrc:/icons/content/scene.png"
                    case Notes.CharacterOwner:
                        return "qrc:/icons/content/person_outline.png"
                    case Notes.BreakOwner:
                        return "qrc:/icons/content/story.png"
                    default:
                        break
                    }
                    break;
                case NotebookModel.NoteType:
                    switch(itemData.value.notebookItemObject.type) {
                    case Note.TextNoteType:
                        return "qrc:/icons/content/note.png"
                    case Note.FormNoteType:
                        return "qrc:/icons/content/form.png"
                    case Note.CheckListNoteType:
                        return "qrc:/icons/content/checklist.png"
                    default:
                        break
                    }
                    break;
                }

                return ""
            }
        }

        VclLabel {
            id: _text

            anchors.verticalCenter: parent.verticalCenter

            width: _layout.width-(_icon.visible ? (_icon.width+_layout.spacing) : 0)

            color: Color.textColorFor(_container.color)
            elide: Text.ElideRight
            padding: 5
            text: itemData.value.notebookItemTitle ? itemData.value.notebookItemTitle : ""

            font.family: Runtime.idealFontMetrics.font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            font.capitalization: Runtime.idealFontMetrics.font.capitalization
            font.bold: itemData.value.notebookItemType === NotebookModel.CategoryType ||
                       (itemData.value.notebookItemType === NotebookModel.NotesType &&
                        itemData.value.notebookItemObject.ownerType === Notes.StructureOwner)
        }
    }

    MouseArea {
        ToolTip.text: _text.text
        ToolTip.visible: containsMouse && _text.text !== "" && _text.truncated
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

        anchors.fill: parent

        acceptedButtons: Qt.RightButton
        hoverEnabled: _text.text !== "" && _text.truncated

        onClicked: {
            root.makeCurrentRequest()

            if(itemData.value.notebookItemType === NotebookModel.NoteType) {
                root.noteMenuRequest(itemData.value.notebookItemObject)
            } else if(itemData.value.notebookItemType === NotebookModel.NotesType &&
                      itemData.value.notebookItemObject.ownerType === Notes.CharacterOwner) {
                root.characterMenuRequest(itemData.value.notebookItemObject.character)
            }
        }
    }
}
