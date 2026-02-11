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
import "qrc:/qml/notebookview/pages"
import "qrc:/qml/structureview"
import "qrc:/qml/notifications"

Item {
    id: root

    function scheduleSwitchTo(item) { _private.scheduleSwitchTo(item) }
    function switchTo(item) { _private.switchTo(item) }
    function switchToCharacterTab(name) { _private.switchToCharacterTab(name) }
    function switchToSceneTab() { _private.switchToSceneTab() }
    function switchToStoryTab() { _private.switchToStoryTab() }

    ObjectRegister.name: "notebookView"

    HelpTipNotification {
        tipName: "notebook"
    }

    Rectangle {
        anchors.fill: parent

        color: Runtime.colors.primary.c100.background
    }

    SplitView {
        Material.background: Qt.darker(Runtime.colors.primary.button.background, 1.1)

        anchors.fill: parent

        orientation: Qt.Horizontal

        NotesTreeView {
            id: _notebookTree

            SplitView.minimumWidth: 150
            SplitView.preferredWidth: Math.min(350, root.width*0.25)

            notebookModel: _notebookModel

            onSwitchRequest: (item) => { _private.scheduleSwitchTo(item) }

            onDeleteNoteRequest: (note) => {
                                     _private.switchTo(note)
                                     _private.scheduleDeleteRequest() // Must be called after switchTo
                                 }

            onDeleteCharacterRequest: (character) => {
                                          _private.switchTo(character.notes)
                                          _private.scheduleDeleteRequest(character.notes) // Must be called after switchTo
                                      }
        }

        Loader {
            id: _contentLoader

            property int currentNotebookItemId: _notebookTree.currentData !== undefined && _notebookTree.currentData.notebookItemId !== undefined ? _notebookTree.currentData.notebookItemId : -1

            active: opacity > 0
            opacity: _contentActiveProperty.value ? 1 : 0
            sourceComponent: {
                if(!_notebookTree.currentData)
                    return _private.genericPage

                switch(_notebookTree.currentData.notebookItemType) {
                case NotebookModel.CategoryType:
                    switch(_notebookTree.currentData.notebookItemCategory) {
                    case NotebookModel.ScreenplayCategory:
                        return _private.screenplayPage
                    case NotebookModel.UnusedScenesCategory:
                        return _private.unusedScenesPage
                    case NotebookModel.CharactersCategory:
                        return _private.charactersPage
                    case NotebookModel.BookmarksCategory:
                        return _private.bookmarksPage
                    }
                    break
                case NotebookModel.NotesType:
                    switch(_notebookTree.currentData.notebookItemObject.ownerType) {
                    case Notes.CharacterOwner:
                        return _private.characterPage
                    case Notes.SceneOwner:
                        return _private.sceneNotesPage
                    default:
                        return _private.notesPage
                    }
                case NotebookModel.NoteType:
                    switch(_notebookTree.currentData.notebookItemObject.type) {
                    case Note.TextNoteType:
                        return _private.textNotePage
                    case Note.FormNoteType:
                        return _private.formNotePage
                    case Note.CheckListNoteType:
                        return _private.checkListNotePage
                    }
                    break
                case NotebookModel.EpisodeBreakType:
                case NotebookModel.ActBreakType:
                    return _private.breakSummaryPage
                }

                return _private.genericPage
            }

            Behavior on opacity {
                NumberAnimation { duration: _contentActiveProperty.delay-50 }
            }

            ResetOnChange {
                id: _contentActiveProperty

                delay: 250
                from: false
                to: true
                trackChangesOn: _contentLoader.currentNotebookItemId
            }

            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(0,0,0,0.05)
                visible: _contentActiveProperty.value === false

                BusyIcon {
                    running: _contentActiveProperty.value === false
                    anchors.centerIn: parent
                }
            }

            onStatusChanged: {
                if(status === Loader.Loading) {
                    if(_private.deleteTriggerTimer) {
                        _private.deleteTriggerTimer.stop()
                        _private.deleteTriggerTimer.destroy()
                    }
                }
            }
        }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("sync")
        checked: Runtime.workspaceSettings.syncCurrentSceneOnNotebook

        onToggled: (source) => {
                       Runtime.workspaceSettings.syncCurrentSceneOnNotebook = !checked
                       if(Runtime.workspaceSettings.syncCurrentSceneOnNotebook) {
                           _notebookTree.activateFromCurrentScreenplayElement()
                       }
                   }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("reload")

        onTriggered: (source) => {
            _notebookModel.refresh()
        }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("report")
        priority: -1 // Meaning we get to run this only if the active page doesnt handle it
        tooltip: "Export entire notebook as PDF or ODT."

        onTriggered: (source) => {
                         ReportConfigurationDialog.launch("Notebook Report")
                     }
    }

    ActionHandler {
        readonly property alias currentNote: _notebookTree.currentNote
        property bool currentNoteIsBookmarked: false

        function determineIfCurrentNoteIsBookmarked() {
            currentNoteIsBookmarked = enabled && _notebookModel.bookmarkedNotes.isBookmarked(currentNote)
        }

        action: ActionHub.notebookOperations.find("toggleBookmark")
        enabled: currentNote !== null
        iconSource: currentNoteIsBookmarked ? "qrc:/icons/content/bookmark.png" : "qrc:/icons/content/bookmark_outline.png"
        tooltip: currentNoteIsBookmarked ? "Remove bookmark on this note" : "Bookmark this note"

        onTriggered: (source) => {
                         if(_notebookModel.bookmarkedNotes.toggleBookmark(currentNote))
                            determineIfCurrentNoteIsBookmarked()
                     }

        onCurrentNoteChanged: determineIfCurrentNoteIsBookmarked()
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("bookmarkedNotes")

        onTriggerCountChanged: (value) => { _private.scheduleSwitchTo("Bookmarks") }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("storyNotes")

        onTriggerCountChanged: (value) => { _private.scheduleSwitchTo("Story") }
    }

    ActionHandler {
        action: ActionHub.notebookOperations.find("characterNotes")

        onTriggerCountChanged: (value) => {
                                   const chName = action.characterName
                                   action.characterName = ""

                                   if(chName === "") {
                                       _private.scheduleSwitchTo("Characters")
                                   } else {
                                       const ch = Scrite.document.structure.findCharacter(chName)
                                       if(ch === null) {
                                           MessageBox.question("Add Character",
                                                               "A section for <b>" + chName.toUpperCase() + "</b> needs to be added to Notebook. Please confirm.",
                                                               ["Confirm", "Cancel"], (answer) => {
                                                                   if(answer === "Confirm") {
                                                                       const newCh = Scrite.document.structure.addCharacter(chName)
                                                                       if(newCh) {
                                                                           _private.scheduleSwitchTo(newCh.notes)
                                                                       }
                                                                   }
                                                               })
                                       } else {
                                           _private.scheduleSwitchTo(ch.notes)
                                       }
                                   }
                               }
    }

    NotebookModel {
        id: _notebookModel

        property var currentItem
        property var preferredItem

        ObjectRegister.name: "notebookModel"

        function noteCurrentItem() {
            currentItem = _notebookTree.currentData.notebookItemObject
        }

        function restoreCurrentItem() {
            if(preferredItem)
                switchTo(preferredItem)
            else
                switchTo(currentItem)
            currentItem = null
            preferredItem = null
        }

        document: Scrite.document.loading ? null : Scrite.document

        onAboutToRefresh: noteCurrentItem()
        onAboutToReloadCharacters: noteCurrentItem()
        onAboutToReloadScenes: noteCurrentItem()
        onJustRefreshed: restoreCurrentItem()
        onJustReloadedCharacters: restoreCurrentItem()
        onJustReloadedScenes: restoreCurrentItem()
    }

    Connections {
        target: Runtime.screenplayAdapter.isSourceScreenplay ? Scrite.document.screenplay : null

        function onElementInserted(element, index) {
            _notebookModel.preferredItem = element.elementType === ScreenplayElement.BreakElementType ? element : element.scene.notes
        }

        function onElementMoved(element, from, to) {
            _notebookModel.preferredItem = element.elementType === ScreenplayElement.BreakElementType ? element : element.scene.notes
        }

        function onCurrentElementIndexChanged(val) {
            if(Runtime.workspaceSettings.syncCurrentSceneOnNotebook && !_notebookTree.activatingScreenplayElement)
                _notebookTree.activateFromCurrentScreenplayElement()
        }
    }

    Connections {
        target: Scrite.document

        ignoreUnknownSignals: true

        function onLoadingChanged() {
            if(!Scrite.document.loading)
                _notebookTree.activateFromCurrentScreenplayElement()
        }
    }

    QtObject {
        id: _private

        property real maxTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 80
        property real minTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 20

        readonly property Component genericPage: GenericNotebookPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        readonly property Component bookmarksPage: BookmarksNotebookPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }
        }

        readonly property Component sceneNotesPage: SceneNotesPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }

            onDeleteNoteRequest: (note) => {
                                     _private.switchTo(note)
                                     _private.scheduleDeleteRequest() // Must be called after switchTo
                                 }
        }

        readonly property Component notesPage: NotesPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }

            onDeleteNoteRequest: (note) => {
                                     _private.switchTo(note)
                                     _private.scheduleDeleteRequest() // Must be called after switchTo
                                 }
        }

        readonly property Component textNotePage: TextNotePage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }
        }

        readonly property Component formNotePage: FormNotePage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }
        }

        readonly property Component checkListNotePage: CheckListNotePage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }
        }

        readonly property Component breakSummaryPage: BreakSummaryPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        readonly property Component screenplayPage: ScreenplayPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }

            onDeleteNoteRequest: (note) => {
                                     _private.switchTo(note)
                                     _private.scheduleDeleteRequest() // Must be called after switchTo
                                 }
        }

        readonly property Component unusedScenesPage: UnusedScenesPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        readonly property Component charactersPage: CharactersPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }

            onDeleteCharacterRequest: (character) => {
                                          _private.switchTo(character.notes)
                                          _private.scheduleDeleteRequest(character.notes) // Must be called after switchTo
                                      }
        }

        readonly property Component characterPage: CharacterPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            onSwitchRequest: (item) => {
                                 _private.scheduleSwitchTo(item)
                             }

            onDeleteNoteRequest: (note) => {
                                     _private.switchTo(note)
                                     _private.scheduleDeleteRequest() // Must be called after switchTo
                                 }
        }

        function switchToStoryTab() {
            switchTo(Scrite.document.structure.notes)
        }

        function switchToSceneTab() {
            const currentScene = Scrite.document.screenplay.activeScene
            if(currentScene)
                switchTo(currentScene.notes)
        }

        function switchToCharacterTab(name) {
            const character = Scrite.document.structure.findCharacter(name)
            if(character)
                switchTo(character.notes)
        }

        function switchTo(item) {
            if(typeof item === "string") {
                let midx
                if(item === "Bookmarks")
                    midx = _notebookTree.model.findModelIndexForCategory(NotebookModel.BookmarksCategory)
                else if(item === "Story")
                    midx = _notebookTree.model.findModelIndexForCategory(NotebookModel.ScreenplayCategory)
                else if(item === "Characters")
                    midx = _notebookTree.model.findModelIndexForCategory(NotebookModel.CharactersCategory)
                else
                    midx = _notebookTree.model.findModelIndexForTopLevelItem(item)
                _notebookTree.setCurrentIndex( midx )
            } else
                _notebookTree.setCurrentIndex( _notebookTree.model.findModelIndexFor(item) )
        }

        readonly property Timer switchTimer: Timer {
            property var item

            interval: Runtime.stdAnimationDuration
            repeat: false
            running: false

            onTriggered: {
                if(item === undefined)
                    return
                _private.switchTo(item)
                item = undefined
            }
        }

        function scheduleSwitchTo(item) {
            if(switchTimer.running)
                return

            switchTimer.item = item
            switchTimer.start()
        }

        readonly property Action deleteAction: ActionHub.notebookOperations.find("delete")
        property Timer deleteTriggerTimer: null
        function scheduleDeleteRequest() {
            if(deleteTriggerTimer)
                deleteTriggerTimer.restart()
            else
                deleteTriggerTimer = Runtime.execLater(deleteAction, Runtime.stdAnimationDuration, deleteAction.trigger)
        }

        Component.onCompleted: {
            _notebookTree.activateFromCurrentScreenplayElement()
            Scrite.user.logActivity1("notebook")
        }
    }
}
