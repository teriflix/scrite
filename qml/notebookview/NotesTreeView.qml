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
import QtQuick.Controls 1.4 as OldControls

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/structureview"
import "qrc:/qml/notifications"
import "qrc:/qml/notebookview/menus"

OldControls.TreeView {
    id: root

    required property NotebookModel notebookModel

    property var currentData: model.modelIndexData(currentIndex)

    property bool activatingScreenplayElement: false

    property Note currentNote: currentData.notebookItemType === NotebookModel.NoteType ? currentData.notebookItemObject : null

    property Notes currentNotes: {
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

        let pmi = modelIndex.parent
        while(pmi.valid) {
            root.expand(pmi)
            pmi = pmi.parent
        }

        let row = 0
        while(1) {
            const idx = root.__model.mapRowToModelIndex(row)
            if(!idx.valid)
                break
            if(idx === modelIndex) {
                root.__listView.currentIndex = row
                root.__listView.positionViewAtIndex(row, ListView.Contain)
                break
            }
            ++row
        }
    }

    EventFilter.events: [EventFilter.Wheel]
    EventFilter.onFilter: (object, event, result) => {
                              if(event.type === EventFilter.Wheel && event.orientation === Qt.Horizontal) {
                                  result.filter = true
                                  result.acceptEvent = true
                              }
                          }

    alternatingRowColors: false
    backgroundVisible: false
    clip: true
    frameVisible: false
    headerVisible: false
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    model: notebookModel
    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

    rowDelegate: Rectangle {
        property var itemData: styleData

        height: Runtime.idealFontMetrics.height + 20
        color: itemData.selected ? Runtime.colors.primary.highlight.background : Runtime.colors.primary.c10.background
    }

    itemDelegate: NotesTreeViewDelegate {
        id: _delegate

        itemData: styleData
        treeViewWidth: root.width

        onNoteMenuRequest: (note) => {
                               _private.popupNoteMenu(note, _delegate)
                           }

        onCharacterMenuRequest: (character) => {
                                    _private.popupCharacterMenu(character, _delegate)
                                }
    }

    OldControls.TableViewColumn {
        title: "Name"
        role: "notebookItemData"
        width: 300
        movable: false
        resizable: false
    }

    onClicked: (index) => {
                   if(Runtime.mainWindowTab !== Runtime.MainWindowTab.StructureTab || Runtime.workspaceSettings.syncCurrentSceneOnNotebook) {
                       activateScreenplayElement( notebookModel.modelIndexData(index) )
                   }
               }

    onDoubleClicked: (index) => {
                         activateScreenplayElement( notebookModel.modelIndexData(index) )
                         if(isExpanded(index)) {
                             collapse(index)
                         } else {
                             expand(index)
                         }
                     }

    QtObject {
        id: _private

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

