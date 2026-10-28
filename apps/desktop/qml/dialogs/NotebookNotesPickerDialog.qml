/****************************************************************************
**
** Copyright (C) 2026 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma ComponentBehavior: Bound
pragma Singleton

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../globals"
import "../controls"
import "../helpers"

/**
Shows the Notebook as it is in the Notebook, with every branch open, for the user to tick notes in.
Only notes can be ticked: the individual ones, and the notes of a scene or a character as a whole.
Whoever launches it is told the keys of what was ticked, in the order in which they were ticked, as
in NotebookFlatLens.keyOf().
*/

DialogLauncher {
    id: root

    function launch(notebook, callback) {
        const dlg = doLaunch({ "notebook": notebook })
        if(dlg)
            dlg.itemsPicked.connect(callback)
        return dlg
    }

    name: "NotebookNotesPickerDialog"
    singleInstanceOnly: true

    dialogComponent: SctDialog {
        id: _dialog

        required property NotebookModel notebook

        signal itemsPicked(var keys)

        width: Math.min(Scrite.window.width * 0.8, 720)
        height: Math.min(Scrite.window.height * 0.9, 720)

        title: "Pick Notes"

        content: Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                spacing: 10

                SctTextField {
                    id: _search

                    Layout.fillWidth: true

                    label: ""
                    placeholderText: "Search the Notebook ..."

                    onTextChanged: _lens.searchText = text

                    Component.onCompleted: Qt.callLater(forceActiveFocus)
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: Runtime.colors.primary.c50.background
                    border.color: Runtime.colors.primary.c50.text
                    border.width: 1

                    TreeView {
                        id: _treeView

                        ScrollBar.vertical: SctScrollBar { }

                        anchors.fill: parent
                        anchors.margins: 1

                        clip: true
                        model: _lens
                        boundsBehavior: Flickable.StopAtBounds
                        columnWidthProvider: (column) => { return _treeView.width }

                        delegate: TreeViewDelegate {
                            id: _treeDelegate

                            required property int column
                            required property var notebookItemData
                            required property bool itemCheckable
                            required property bool itemChecked

                            readonly property var liveModelIndex: _treeView.index(_treeDelegate.row, _treeDelegate.column)
                            readonly property int itemType: notebookItemData ? notebookItemData.notebookItemType : -1
                            readonly property var itemObject: notebookItemData ? notebookItemData.notebookItemObject : null

                            implicitHeight: Runtime.idealFontMetrics.height + 20

                            // Same coloring as the Notebook's own tree, see NotesTreeViewDelegate.
                            background: Rectangle {
                                id: _treeBackground

                                color: {
                                    let baseColor = undefined

                                    if(_treeDelegate.itemType === NotebookModel.NotesType) {
                                        switch(_treeDelegate.itemObject.ownerType) {
                                        case Notes.SceneOwner:
                                        case Notes.CharacterOwner:
                                            baseColor = _treeDelegate.itemObject.color
                                            break
                                        default:
                                            break
                                        }
                                    } else if(_treeDelegate.itemType === NotebookModel.NoteType)
                                        baseColor = _treeDelegate.itemObject.color

                                    if(baseColor)
                                        return Runtime.colors.tintTx(baseColor, Runtime.colors.sceneHeadingTint)

                                    return Runtime.colors.primary.editor.background
                                }
                            }

                            contentItem: Row {
                                id: _treeLayout

                                width: Math.max(0, _treeView.width - _treeDelegate.leftPadding - _treeDelegate.rightPadding)
                                height: _treeDelegate.implicitHeight

                                spacing: 5

                                SctCheckBox {
                                    anchors.verticalCenter: parent.verticalCenter

                                    visible: _treeDelegate.itemCheckable

                                    checked: _treeDelegate.itemChecked

                                    onToggled: _lens.setItemChecked(_treeDelegate.liveModelIndex, checked)
                                }

                                Image {
                                    anchors.verticalCenter: parent.verticalCenter

                                    width: parent.height * 0.6
                                    height: width

                                    mipmap: true
                                    visible: source != ""
                                    opacity: {
                                        switch(_treeDelegate.itemType) {
                                        case NotebookModel.EpisodeBreakType:
                                        case NotebookModel.ActBreakType:
                                            return _treeDelegate.itemObject ? 1 : 0.5
                                        }
                                        return 1
                                    }

                                    source: Runtime.themedIcon(_source)

                                    property string _source: {
                                        switch(_treeDelegate.itemType) {
                                        case NotebookModel.EpisodeBreakType:
                                            return Runtime.themedIcon("qrc:/icons/content/episode.png")
                                        case NotebookModel.ActBreakType:
                                            return Runtime.themedIcon("qrc:/icons/content/act.png")
                                        case NotebookModel.NotesType:
                                            switch(_treeDelegate.itemObject.ownerType) {
                                            case Notes.SceneOwner:
                                                return Runtime.themedIcon("qrc:/icons/content/scene.png")
                                            case Notes.CharacterOwner:
                                                return Runtime.themedIcon("qrc:/icons/content/person_outline.png")
                                            case Notes.BreakOwner:
                                                return Runtime.themedIcon("qrc:/icons/content/story.png")
                                            default:
                                                break
                                            }
                                            break
                                        case NotebookModel.NoteType:
                                            switch(_treeDelegate.itemObject.type) {
                                            case Note.TextNoteType:
                                                return Runtime.themedIcon("qrc:/icons/content/note.png")
                                            case Note.FormNoteType:
                                                return Runtime.themedIcon("qrc:/icons/content/form.png")
                                            case Note.CheckListNoteType:
                                                return Runtime.themedIcon("qrc:/icons/content/checklist.png")
                                            default:
                                                break
                                            }
                                            break
                                        }

                                        return ""
                                    }
                                }

                                SctLabel {
                                    anchors.verticalCenter: parent.verticalCenter

                                    width: _treeLayout.width - x

                                    color: Color.textColorFor(_treeBackground.color)
                                    elide: Text.ElideRight
                                    padding: 5
                                    text: _treeDelegate.notebookItemData && _treeDelegate.notebookItemData.notebookItemTitle ? _treeDelegate.notebookItemData.notebookItemTitle : ""

                                    font.family: Runtime.idealFontMetrics.font.family
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    font.capitalization: Runtime.idealFontMetrics.font.capitalization
                                    font.bold: _treeDelegate.itemType === NotebookModel.CategoryType ||
                                               (_treeDelegate.itemType === NotebookModel.NotesType &&
                                                _treeDelegate.itemObject.ownerType === Notes.StructureOwner)
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    SctLabel {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: true

                        text: _lens.checkedCount === 0 ? "Tick the notes to add." : ("" + _lens.checkedCount + " Item(s) Ticked")
                    }

                    SctButton {
                        text: "Add"
                        enabled: _lens.checkedCount > 0

                        onClicked: _dialog.acceptAction.trigger()
                    }
                }
            }

            NotebookTreeLens {
                id: _lens

                sourceModel: _dialog.notebook
                kinds: NotebookItemFilter.NoteKind | NotebookItemFilter.SceneKind | NotebookItemFilter.CharacterKind
            }

            // The Notebook makes its branches again as the document changes, and the tree is narrowed
            // as the user searches, which closes branches. All of them are kept open.
            Connections {
                target: _lens

                function onModelReset() { Qt.callLater(_treeView.expandRecursively) }
                function onRowsInserted() { Qt.callLater(_treeView.expandRecursively) }
                function onLayoutChanged() { Qt.callLater(_treeView.expandRecursively) }
            }

            Component.onCompleted: Qt.callLater(_treeView.expandRecursively)

            ActionHandler {
                action: _dialog.acceptAction
                enabled: _lens.checkedCount > 0

                onTriggered: () => {
                    _dialog.itemsPicked(_lens.checkedKeys)
                    _dialog.close()
                }
            }
        }
    }
}
