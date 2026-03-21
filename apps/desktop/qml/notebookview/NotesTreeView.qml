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
import QtQml.Models
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../helpers"
import "../dialogs"
import "../controls"
import "../structureview"
import "../notifications"
import "./menus"

TreeView {
    id: root

    required property NotebookModel notebookModel

    property var currentData: _private.currentModelIndex && _private.currentModelIndex.valid ? model.modelIndexData(_private.currentModelIndex) : undefined

    property bool activatingScreenplayElement: false

    property Note currentNote: currentData && currentData.notebookItemType === NotebookModel.NoteType ? currentData.notebookItemObject : null

    property Notes currentNotes: {
        if(!currentData)
            return null
        if(currentData.notebookItemType === NotebookModel.NotesType)
            return currentData.notebookItemObject
        if(currentData.notebookItemType === NotebookModel.NoteType)
            return currentData.notebookItemObject.notes
        if(currentData.notebookItemType === NotebookModel.CategoryType &&
                currentData.notebookItemCategory === NotebookModel.ScreenplayCategory)
            return Scrite.document.structure.notes
        return null
    }

    property Character currentCharacter: currentNotes && currentNotes.ownerType === Notes.CharacterOwner ? currentNotes.character : null

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.
    signal deleteCharacterRequest(Character character)
    signal deleteNoteRequest(Note note)

    function activateFromCurrentScreenplayElement() {
        const spobj = Scrite.document.screenplay
        const element = spobj.elementAt(spobj.currentElementIndex)
        if(element) {
            if(element.elementType === ScreenplayElement.BreakElementType)
                root.switchRequest(element)
            else
                root.switchRequest(element.scene.notes)
        }
    }

    function activateScreenplayElement(modelData) {
        activatingScreenplayElement = true
        Qt.callLater( () => { root.activatingScreenplayElement = false })

        const makeSceneCurrent = function(notes) {
            if(notes.ownerType === Notes.SceneOwner) {
                const scene = notes.owner
                const idxes = scene.screenplayElementIndexList
                if(idxes.length > 0)
                    Scrite.document.screenplay.currentElementIndex = idxes[0]
            }
        }

        switch(modelData.notebookItemType) {
        case NotebookModel.EpisodeBreakType:
        case NotebookModel.ActBreakType:
            if(modelData.notebookItemObject)
                Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.indexOfElement(modelData.notebookItemObject)
            break
        case NotebookModel.NotesType:
            makeSceneCurrent(modelData.notebookItemObject)
            break
        case NotebookModel.NoteType:
            makeSceneCurrent(modelData.notebookItemObject.notes)
            break
        default:
            break
        }
    }

    function setCurrentIndex(modelIndex) {
        if(!modelIndex.valid)
            return

        const ancestorIndexes = []
        let pmi = modelIndex.parent
        while(pmi.valid) {
            ancestorIndexes.unshift(pmi)
            pmi = pmi.parent
        }

        for(let i=0; i<ancestorIndexes.length; i++) {
            const row = root.rowAtIndex(ancestorIndexes[i])
            if(row >= 0)
                root.expand(row)
        }

        const row = root.rowAtIndex(modelIndex)
        if(row >= 0) {
            selectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows)
            root.positionViewAtRow(row, TableView.Contain)
            _private.currentModelIndex = modelIndex
        } else {
            Qt.callLater(() => {
                const delayedRow = root.rowAtIndex(modelIndex)
                if(delayedRow >= 0) {
                    selectionModel.setCurrentIndex(modelIndex, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows)
                    root.positionViewAtRow(delayedRow, TableView.Contain)
                    _private.currentModelIndex = modelIndex
                }
            })
        }
    }

    function handleClick(modelIndex) {
        if(!modelIndex || !modelIndex.valid)
            return
        if(Runtime.mainWindowTab !== Runtime.MainWindowTab.StructureTab || Runtime.workspaceSettings.syncCurrentSceneOnNotebook) {
            activateScreenplayElement(notebookModel.modelIndexData(modelIndex))
        }
    }

    function handleDoubleClick(modelIndex) {
        if(!modelIndex || !modelIndex.valid)
            return

        activateScreenplayElement(notebookModel.modelIndexData(modelIndex))

        const row = root.rowAtIndex(modelIndex)
        if(row >= 0) {
            if(root.isExpanded(row))
                root.collapse(row)
            else
                root.expand(row)
        }
    }

    ScrollBar.horizontal: VclScrollBar {
        policy: ScrollBar.AlwaysOff
    }

    ScrollBar.vertical: VclScrollBar {
        policy: ScrollBar.AsNeeded
    }

    selectionModel: ItemSelectionModel {
        model: root.model

        onCurrentIndexChanged: {
            if(currentIndex && currentIndex.valid)
                _private.currentModelIndex = currentIndex
            else
                _private.currentModelIndex = undefined
        }
    }

    columnWidthProvider: (column) => {
        return root.width
    }

    delegate: NotesTreeViewDelegate {
        id: _delegate

        property var rowModelIndex: root.index(row, 0)
        property var rowData: rowModelIndex && rowModelIndex.valid ? root.model.modelIndexData(rowModelIndex) : ({})

        itemData: ({
                       "selected": current,
                       "value": rowData,
                       "isExpanded": expanded,
                       "hasChildren": hasChildren,
                       "index": rowModelIndex
                   })
        treeViewWidth: root.width
        modelIndex: rowModelIndex

        onMakeCurrentRequest: () => {
            root.setCurrentIndex(rowModelIndex)
        }

        onModelIndexClicked: (index) => {
            root.handleClick(index)
        }

        onModelIndexDoubleClicked: (index) => {
            root.handleDoubleClick(index)
        }

        onNoteMenuRequest: (note) => {
            _private.popupNoteMenu(note, _delegate)
        }

        onCharacterMenuRequest: (character) => {
            _private.popupCharacterMenu(character, _delegate)
        }
    }

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object, event, result) => {
        if(event.type === EventFilter.Wheel && event.orientation === Qt.Horizontal) {
            result.filter = true
            result.acceptEvent = true
        }
    }

    clip: true
    model: notebookModel

    onCurrentRowChanged: {
        if(currentRow >= 0) {
            const idx = root.index(currentRow, 0)
            if(idx && idx.valid)
                _private.currentModelIndex = idx
        }
    }

    QtObject {
        id: _private

        property var currentModelIndex: undefined

        readonly property Component noteMenu: NoteMenu {
            onDeleteNoteRequest: () => { root.deleteNoteRequest(note) }
        }

        function popupNoteMenu(note, source) {
            let menu = noteMenu.createObject(source, {"note": note})
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
        }

        readonly property Component characterMenu: CharacterMenu {
            onDeleteCharacterRequest: () => { root.deleteCharacterRequest(character) }
        }

        function popupCharacterMenu(character, source) {
            let menu = characterMenu.createObject(source, {"character": character})
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
        }
    }
}
