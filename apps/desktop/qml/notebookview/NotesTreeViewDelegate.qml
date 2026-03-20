/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQml
import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../helpers"
import "../controls"

TreeViewDelegate {
    id: root

    required property var itemData
    required property real treeViewWidth
    required property var modelIndex

    implicitHeight: Runtime.idealFontMetrics.height + 20

    signal clicked(var index)
    signal doubleClicked(var index)
    signal noteMenuRequest(Note note)
    signal makeCurrentRequest()
    signal characterMenuRequest(Character character)

    background: Rectangle {
        id: _container
        width: root.treeViewWidth
        height: root.implicitHeight
        color: {
            if(root.current)
                return Runtime.colors.primary.highlight.background

            let baseColor = undefined

            if(root.itemData.value.notebookItemType === NotebookModel.NotesType) {
                switch(root.itemData.value.notebookItemObject.ownerType) {
                case Notes.SceneOwner:
                case Notes.CharacterOwner:
                    baseColor = root.itemData.value.notebookItemObject.color
                    break
                default:
                    break
                }
            } else if(root.itemData.value.notebookItemType === NotebookModel.NoteType)
                baseColor = root.itemData.value.notebookItemObject.color

            if(baseColor)
                return Runtime.colors.tint(baseColor, Runtime.colors.sceneHeadingTint)

            return Runtime.colors.primary.c10.background
        }
    }

    contentItem: Row {
        id: _layout

        width: Math.max(0, root.treeViewWidth - root.leftPadding - root.rightPadding)
        height: root.implicitHeight

        spacing: 5

        Image {
            id: _icon

            anchors.verticalCenter: parent.verticalCenter

            width: parent.height * 0.6
            height: width

            mipmap: true
            visible: source != ""
            opacity: {
                switch(root.itemData.value.notebookItemType) {
                case NotebookModel.EpisodeBreakType:
                case NotebookModel.ActBreakType:
                    return root.itemData.value.notebookItemObject ? 1 : 0.5
                }
                return 1
            }

            source: {
                switch(root.itemData.value.notebookItemType) {
                case NotebookModel.EpisodeBreakType:
                    return "qrc:/icons/content/episode.png"
                case NotebookModel.ActBreakType:
                    return "qrc:/icons/content/act.png"
                case NotebookModel.NotesType:
                    switch(root.itemData.value.notebookItemObject.ownerType) {
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
                    switch(root.itemData.value.notebookItemObject.type) {
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

            width: _layout.width - (_icon.visible ? (_icon.width + _layout.spacing) : 0)

            color: Color.textColorFor(_container.color)
            elide: Text.ElideRight
            padding: 5
            text: root.itemData.value.notebookItemTitle ? root.itemData.value.notebookItemTitle : ""

            font.family: Runtime.idealFontMetrics.font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            font.capitalization: Runtime.idealFontMetrics.font.capitalization
            font.bold: root.itemData.value.notebookItemType === NotebookModel.CategoryType ||
                       (root.itemData.value.notebookItemType === NotebookModel.NotesType &&
                        root.itemData.value.notebookItemObject.ownerType === Notes.StructureOwner)
        }
    }

    MouseArea {
        id: _mouseArea

        ToolTipPopup {
            text: _text.text
            visible: _mouseArea.containsMouse && _text.text !== "" && _text.truncated
        }

        anchors.fill: root

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: _text.text !== "" && _text.truncated

        onClicked: (mouse) => {
            root.makeCurrentRequest()

            if(mouse.button === Qt.RightButton) {
                if(root.itemData.value.notebookItemType === NotebookModel.NoteType) {
                    root.noteMenuRequest(root.itemData.value.notebookItemObject)
                } else if(root.itemData.value.notebookItemType === NotebookModel.NotesType &&
                          root.itemData.value.notebookItemObject.ownerType === Notes.CharacterOwner) {
                    root.characterMenuRequest(root.itemData.value.notebookItemObject.character)
                }
            } else if(mouse.button === Qt.LeftButton) {
                root.clicked(root.modelIndex)
            }
        }

        onDoubleClicked: (mouse) => {
            if(mouse.button === Qt.LeftButton)
                root.doubleClicked(root.modelIndex)
        }
    }
}
