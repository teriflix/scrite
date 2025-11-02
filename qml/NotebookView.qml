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
import "qrc:/qml/notebookview/pages"
import "qrc:/qml/structureview"
import "qrc:/qml/notifications"

Rectangle {
    id: root

    function switchTo(item) { _private.switchTo(item) }
    function switchToCharacterTab(name) { _private.switchToCharacterTab(name) }
    function switchToSceneTab() { _private.switchToSceneTab() }
    function switchToStoryTab() { _private.switchToStoryTab() }

    SplitView {
        Material.background: Qt.darker(Runtime.colors.primary.button.background, 1.1)

        anchors.fill: parent

        orientation: Qt.Horizontal

        NotesTreeView {
            id: _notebookTree

            SplitView.minimumWidth: 150
            SplitView.preferredWidth: Math.min(350, notebookView.width*0.25)

            notebookModel: _notebookModel

            onSwitchRequest: (item) => { _private.switchTo(item) }
        }

        Rectangle {
            color: {
                // Keep these colors in sync with actual component colors loaded
                // by notebookContentLoader
                if(!_notebookTree.currentData)
                    return Qt.rgba(0,0,0,0)

                switch(_notebookTree.currentData.notebookItemType) {
                case NotebookModel.CategoryType:
                    switch(_notebookTree.currentData.notebookItemCategory) {
                    case NotebookModel.ScreenplayCategory:
                        return "white"
                    case NotebookModel.UnusedScenesCategory:
                    case NotebookModel.CharactersCategory:
                        return Qt.rgba(0,0,0,0)
                    case NotebookModel.BookmarksCategory:
                        return Color.translucent(Runtime.colors.primary.c100.background, 0.5)
                    }
                    break
                case NotebookModel.NotesType:
                    switch(_notebookTree.currentData.notebookItemObject.ownerType) {
                    case Notes.CharacterOwner: {
                        const character = _notebookTree.currentData.notebookItemObject.character
                        return Qt.tint(character.color, "#e7ffffff")
                        }
                    case Notes.SceneOwner: {
                        const notes = _notebookTree.currentData.notebookItemObject
                        const scene = notes.scene
                        return Qt.tint(scene.color, "#e7ffffff")
                        }
                    default:
                        return Color.translucent(Runtime.colors.primary.c100.background, 0.5)
                    }
                case NotebookModel.NoteType:
                    switch(_notebookTree.currentData.notebookItemObject.type) {
                    case Note.TextNoteType:
                    case Note.FormNoteType:
                        const note = _notebookTree.currentData.notebookItemObject
                        return Qt.tint(note.color, Runtime.colors.sceneHeadingTint)
                    }
                    break
                case NotebookModel.EpisodeBreakType:
                case NotebookModel.ActBreakType:
                    return Qt.rgba(0,0,0,0)
                }

                return Qt.rgba(0,0,0,0)
            }

            Loader {
                id: notebookContentLoader
                opacity: notebookContentActiveProperty.value ? 1 : 0
                anchors.fill: parent
                Behavior on opacity {
                    NumberAnimation { duration: notebookContentActiveProperty.delay-50 }
                }
                active: opacity > 0

                property int currentNotebookItemId: _notebookTree.currentData ? _notebookTree.currentData.notebookItemId : -1

                property bool hasReport: item && item.hasReport && item.hasReport === true
                property string reportDescription: hasReport ? item.reportDescription : ""
                function generateReport() {
                    if(hasReport) {
                        var rgen = item.createReportGenerator()
                        if(!rgen)
                            return

                        ReportConfigurationDialog.launch(rgen)
                    }
                }

                ResetOnChange {
                    id: notebookContentActiveProperty
                    trackChangesOn: notebookContentLoader.currentNotebookItemId
                    from: false
                    to: true
                    delay: 250
                }

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
                            return notesComponent
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
                        return _private.breakNotePage
                    }

                    return _private.genericPage
                }
                onLoaded: item.componentData = _notebookTree.currentData

                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(0,0,0,0.05)
                    visible: notebookContentActiveProperty.value === false

                    BusyIcon {
                        running: notebookContentActiveProperty.value === false
                        anchors.centerIn: parent
                    }
                }

                function confirmAndDelete() {
                    deleteConfirmationBox.active = true
                }
                onActiveChanged: deleteConfirmationBox.active = false
            }

            Loader {
                id: deleteConfirmationBox
                anchors.fill: parent
                active: false
                sourceComponent: Rectangle {
                    id: deleteConfirmationItem
                    color: Color.translucent(Runtime.colors.primary.c600.background,0.85)
                    focus: true

                    MouseArea {
                        anchors.fill: parent
                    }

                    Column {
                        width: parent.width-20
                        anchors.centerIn: parent
                        spacing: 40

                        VclLabel {
                            text: {
                                if(_notebookTree.currentNote)
                                    return "Are you sure you want to delete this note?"
                                if(_notebookTree.currentCharacter)
                                    return "Are you sure you want to delete this character?"
                                return "Cannot remove this item."
                            }
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            color: Runtime.colors.primary.c600.text
                        }

                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 20

                            VclButton {
                                text: "Yes"
                                focusPolicy: Qt.NoFocus
                                onClicked: {
                                    notebookContentLoader.item.deleteSelf()
                                    deleteConfirmationBox.active = false
                                }
                                visible: _notebookTree.currentNote || _notebookTree.currentCharacter
                            }

                            VclButton {
                                text: _notebookTree.currentNote || _notebookTree.currentCharacter ? "No" : "OK"
                                focusPolicy: Qt.NoFocus
                                onClicked: deleteConfirmationBox.active = false
                            }
                        }
                    }
                }
            }
        }
    }

    FocusTracker.window: Scrite.window
    FocusTracker.onHasFocusChanged: Runtime.undoStack.notebookActive = FocusTracker.hasFocus

    HelpTipNotification {
        tipName: "notebook"
    }

    NotebookModel {
        id: _notebookModel

        property var currentItem
        property var preferredItem

        Component.onCompleted: ObjectRegistry.add(_notebookModel, "notebookModel")

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
        }

        property int sceneNotesPageTabIndex: 0
        readonly property Component sceneNotesPage: SceneNotesPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            currentTab: _private.sceneNotesPageTabIndex

            onCurrentTabChanged: _private.sceneNotesPageTabIndex = currentTab
        }

        readonly property Component notesPage: NotesPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        readonly property Component textNotePage: TextNotePage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        readonly property Component formNotePage: FormNotePage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        readonly property Component checkListNotePage: CheckListNotePage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        readonly property Component breakNotePage: BreakNotePage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        property int screenplayPageTabIndex: 0
        readonly property Component screenplayPage: ScreenplayPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            currentTab: _private.screenplayPageTabIndex

            onCurrentTabChanged: _private.screenplayPageTabIndex = currentTab
        }

        readonly property Component unusedScenesPage: UnusedScenesPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize
        }

        property int charactersPageTabIndex: 0
        readonly property Component charactersPage: CharactersPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            currentTab: _private.charactersPageTabIndex

            onCurrentTabChanged: _private.charactersPageTabIndex = currentTab
        }

        property int characterPageTabIndex: 0
        readonly property Component characterPage: CharacterPage {
            pageData: _notebookTree.currentData
            notebookModel: _notebookModel
            maxTextAreaSize: _private.maxTextAreaSize
            minTextAreaSize: _private.minTextAreaSize

            currentTab: _private.characterPageTabIndex

            onCurrentTabChanged: _private.characterPageTabIndex = currentTab
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
                if(item === "Notebook Bookmarks")
                    midx = _notebookTree.model.findModelIndexForCategory(NotebookModel.BookmarksCategory)
                else if(item === "Notebook Story")
                    midx = _notebookTree.model.findModelIndexForCategory(NotebookModel.ScreenplayCategory)
                else if(item === "Notebook Characters")
                    midx = _notebookTree.model.findModelIndexForCategory(NotebookModel.CharactersCategory)
                else
                    midx = _notebookTree.model.findModelIndexForTopLevelItem(item)
                _notebookTree.setCurrentIndex( midx )
            } else
                _notebookTree.setCurrentIndex( _notebookTree.model.findModelIndexFor(item) )
        }

        Announcement.onIncoming: (type, data) => {
                                     // Check this..
                                     if(type === Runtime.announcementIds.characterNotesRequest) {
                                         switchToCharacterTab(data)
                                     } else if(type === Runtime.announcementIds.sceneNotesRequest) {
                                         switchToSceneTab()
                                     } else if(type === Runtime.announcementIds.notebookNodeRequest) {
                                         if(typeof data === "string") {
                                             switch(data) {
                                                 case "Story":
                                                    switchToStoryTab()
                                                    break
                                                 case "Screenplay":
                                                    switchTo("Notebook Story");
                                                    break
                                                 case "Characters":
                                                    switchTo("Notebook Characters");
                                                    break
                                             }
                                         }
                                     }
                                 }

        Component.onCompleted: {
            _notebookTree.activateFromCurrentScreenplayElement()
            Scrite.user.logActivity1("notebook")
        }
    }
}
