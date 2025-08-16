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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/notebook"
import "qrc:/qml/structureview"
import "qrc:/qml/notifications"

Rectangle {
    id: notebookView

    property real toolbarSize: 46
    property real toolbarSpacing: 0
    property real toolbarLeftMargin: 0
    property real toolButtonSize: Math.max(toolbarSize - 4, 20)
    property real maxTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 80
    property real minTextAreaSize: Runtime.idealFontMetrics.averageCharacterWidth * 20

    function switchToStoryTab() {
        switchTo(Scrite.document.structure.notes)
    }

    function switchToSceneTab() {
        var currentScene = Scrite.document.screenplay.activeScene
        if(currentScene)
            switchTo(currentScene.notes)
    }

    function switchToCharacterTab(name) {
        var character = Scrite.document.structure.findCharacter(name)
        if(character)
            switchTo(character.notes)
    }

    function switchTo(item) {
        if(typeof item === "string") {
            var midx
            if(item === "Notebook Bookmarks")
                midx = notebookTree.model.findModelIndexForCategory(NotebookModel.BookmarksCategory)
            else if(item === "Notebook Story")
                midx = notebookTree.model.findModelIndexForCategory(NotebookModel.ScreenplayCategory)
            else if(item === "Notebook Characters")
                midx = notebookTree.model.findModelIndexForCategory(NotebookModel.CharactersCategory)
            else
                midx = notebookTree.model.findModelIndexForTopLevelItem(item)
            notebookTree.setCurrentIndex( midx )
        } else
            notebookTree.setCurrentIndex( notebookTree.model.findModelIndexFor(item) )
    }

    Announcement.onIncoming: (type, data) => {
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

    NotebookModel {
        id: notebookModel
        document: Scrite.document.loading ? null : Scrite.document

        onAboutToRefresh: noteCurrentItem()
        onJustRefreshed: restoreCurrentItem()
        onAboutToReloadScenes: noteCurrentItem()
        onJustReloadedScenes: restoreCurrentItem()
        onAboutToReloadCharacters: noteCurrentItem()
        onJustReloadedCharacters: restoreCurrentItem()

        property var preferredItem
        property var currentItem

        function noteCurrentItem() {
            currentItem = notebookTree.currentData.notebookItemObject
        }

        function restoreCurrentItem() {
            if(preferredItem)
                switchTo(preferredItem)
            else
                switchTo(currentItem)
            currentItem = null
            preferredItem = null
        }
    }

    Connections {
        target: Runtime.screenplayAdapter.isSourceScreenplay ? Scrite.document.screenplay : null
        function onElementInserted(element, index) {
            notebookModel.preferredItem = element.elementType === ScreenplayElement.BreakElementType ? element : element.scene.notes
        }
        function onElementMoved(element, from, to) {
            notebookModel.preferredItem = element.elementType === ScreenplayElement.BreakElementType ? element : element.scene.notes
        }
        function onCurrentElementIndexChanged(val) {
            if(Runtime.workspaceSettings.syncCurrentSceneOnNotebook && !notebookTree.activatingScreenplayElement)
                notebookTree.activateFromCurrentScreenplayElement()
        }
    }

    Connections {
        target: Scrite.document
        ignoreUnknownSignals: true
        function onLoadingChanged() {
            if(!Scrite.document.loading)
                notebookTree.activateFromCurrentScreenplayElement()
        }
    }

    Component.onCompleted: {
        notebookTree.activateFromCurrentScreenplayElement()
        Scrite.user.logActivity1("notebook")
    }

    FontMetrics {
        id: fontMetrics
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
    }

    Rectangle {
        id: toolbar
        width: toolButtonSize+4
        height: parent.height
        color: Runtime.colors.primary.c100.background

        Column {
            id: toolbarLayout
            width: toolButtonSize
            anchors.horizontalCenter: parent.horizontalCenter

            FlatToolButton {
                id: structureTabButton
                visible: Runtime.showNotebookInStructure
                iconSource: "qrc:/icons/navigation/structure_tab.png"
                ToolTip.text: "Structure Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                onClicked: Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Structure")
            }

            FlatToolButton {
                id: notebookTabButton
                visible: Runtime.showNotebookInStructure
                iconSource: "qrc:/icons/navigation/notebook_tab.png"
                down: true
                ToolTip.text: "Notebook\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            FlatToolButton {
                checkable: true
                iconSource: "qrc:/icons/navigation/sync.png"
                ToolTip.text: "If checked; episodes, acts and scenes selected on the notebook will be made current in screenplay editor & timeline"
                checked: Runtime.workspaceSettings.syncCurrentSceneOnNotebook
                onToggled: {
                    Runtime.workspaceSettings.syncCurrentSceneOnNotebook = checked
                    if(checked)
                        notebookTree.activateFromCurrentScreenplayElement()
                }
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
            }

            FlatToolButton {
                id: refreshButton
                iconSource: "qrc:/icons/navigation/refresh.png"
                ToolTip.text: "Reloads the notebook tree."
                onClicked: notebookModel.refresh()
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
            }

            FlatToolButton {
                id: pdfExportButton
                iconSource: "qrc:/icons/file/generate_pdf.png"
                ToolTip.text: notebookContentLoader.reportDescription
                onClicked: notebookContentLoader.generateReport()
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                enabled: notebookContentLoader.hasReport
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Runtime.colors.primary.separatorColor
                opacity: 0.5
            }

            FlatToolButton {
                id: newNoteToolButton
                iconSource: "qrc:/icons/action/note_add.png"
                hasMenu: true
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                property Notes notes: notebookTree.currentNotes
                enabled: notes && !Scrite.document.readOnly
                ToolTip.text: {
                    var ret = "Adds a new text or form note"
                    if(!enabled)
                        return ret
                    ret += " to " + notebookTree.currentData.notebookItemTitle
                    return ret
                }
                down: newNoteMenu.visible
                onClicked: {
                    newNoteMenu.notes = notes
                    newNoteMenu.popup(newNoteToolButton, newNoteToolButton.width, 0)
                }

                shortcut: "Ctrl+T"
                ShortcutsModelItem.group: "Notebook"
                ShortcutsModelItem.title: "New Note"
                ShortcutsModelItem.shortcut: shortcut
                ShortcutsModelItem.enabled: enabled
            }

            FlatToolButton {
                id: noteColorButton
                property Character character: notebookTree.currentCharacter
                property Note note: notebookTree.currentNote
                property Notes notes: notebookTree.currentNotes
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                hasMenu: true
                enabled: (character || note || (notes && notes.ownerType === Notes.SceneOwner)) && !Scrite.document.readOnly
                iconSource: {
                    if(note)
                        return "image://color/" + note.color + "/1"
                    if(character)
                        return "image://color/" + character.color + "/1"
                    if(notes && notes.ownerType === Notes.SceneOwner)
                        return "image://color/" + notes.scene.color + "/1"
                    return "image://color/#00ffffff/1"
                }
                down: noteColorMenu.visible
                onClicked: noteColorMenu.popup(noteColorButton, noteColorButton.width, 0)

                ColorMenu {
                    id: noteColorMenu
                    enabled: noteColorButton.enabled
                    onMenuItemClicked: {
                        if(noteColorButton.note)
                            noteColorButton.note.color = color
                        else if(noteColorButton.character)
                            noteColorButton.character.color = color
                        else if(noteColorButton.notes && noteColorButton.notes.ownerType === Notes.SceneOwner)
                            noteColorButton.notes.scene.color = color
                    }
                }
            }

            FlatToolButton {
                id: bookmarkButton
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                ToolTip.text: "Toggle bookmark of a Note, Scene, Episode/Act Notes or Character"
                enabled: noteColorButton.note || noteColorButton.notes || noteColorButton.character
                property var notebookObject: notebookTree.currentData.notebookItemObject
                onNotebookObjectChanged: updateIcon()
                onClicked: {
                    notebookModel.bookmarkedNotes.toggleBookmark(notebookObject)
                    updateIcon()
                }
                function updateIcon() {
                    if(enabled && notebookModel.bookmarkedNotes.isBookmarked(notebookObject))
                        iconSource = "qrc:/icons/content/bookmark.png"
                    else
                        iconSource = "qrc:/icons/content/bookmark_outline.png"
                }

                shortcut: "Ctrl+D"
                ShortcutsModelItem.group: "Notebook"
                ShortcutsModelItem.title: "Toggle Bookmark"
                ShortcutsModelItem.shortcut: shortcut
                ShortcutsModelItem.enabled: enabled
            }

            FlatToolButton {
                id: deleteNoteButton
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                enabled: (noteColorButton.note || noteColorButton.character) && !Scrite.document.readOnly
                ToolTip.text: "Delete the current note or character"
                iconSource: "qrc:/icons/action/delete.png"
                onClicked: notebookContentLoader.confirmAndDelete()
            }
        }

        Rectangle {
            width: 1
            height: parent.height
            anchors.right: parent.right
            color: Runtime.colors.primary.borderColor
        }
    }

    SplitView {
        orientation: Qt.Horizontal
        anchors.top: parent.top
        anchors.left: toolbar.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        Material.background: Qt.darker(Runtime.colors.primary.button.background, 1.1)

        OldControls.TreeView {
            id: notebookTree
            SplitView.preferredWidth: Math.min(350, notebookView.width*0.25)
            SplitView.minimumWidth: 150
            clip: true
            headerVisible: false
            model: notebookModel
            frameVisible: false
            backgroundVisible: false
            alternatingRowColors: false
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
            rowDelegate: Rectangle {
                height: fontMetrics.height + 20
                color: styleData.selected ? Runtime.colors.primary.highlight.background : Runtime.colors.primary.c10.background
            }
            EventFilter.events: [EventFilter.Wheel]
            EventFilter.onFilter: {
                if(event.type === EventFilter.Wheel && event.orientation === Qt.Horizontal) {
                    result.filter = true
                    result.acceptEvent = true
                }
            }

            property var currentData: model.modelIndexData(currentIndex)
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
            property Note currentNote: currentData.notebookItemType === NotebookModel.NoteType ? currentData.notebookItemObject : null
            property Character currentCharacter: currentNotes && currentNotes.ownerType === Notes.CharacterOwner ? currentNotes.character : null

            itemDelegate: Item {
                Rectangle {
                    width: notebookTree.width - parent.x
                    height: fontMetrics.height + 20
                    color: {
                        if(styleData.selected)
                            return Runtime.colors.primary.highlight.background

                        var baseColor = undefined

                        if(styleData.value.notebookItemType === NotebookModel.NotesType) {
                            switch(styleData.value.notebookItemObject.ownerType) {
                            case Notes.SceneOwner:
                            case Notes.CharacterOwner:
                                baseColor = styleData.value.notebookItemObject.color
                                break
                            default:
                                break
                            }
                        } else if(styleData.value.notebookItemType === NotebookModel.NoteType)
                            baseColor = styleData.value.notebookItemObject.color

                        if(baseColor)
                            return Qt.tint(baseColor, "#E7FFFFFF")

                        return Runtime.colors.primary.c10.background
                    }

                    Row {
                        width: parent.width
                        height: parent.height
                        spacing: 5

                        Item {
                            width: 1
                            height: parent.height
                            visible: itemDelegateIcon.visible
                        }

                        Image {
                            id: itemDelegateIcon
                            width: parent.height * 0.6
                            height: width
                            mipmap: true
                            anchors.verticalCenter: parent.verticalCenter
                            visible: source != ""
                            opacity: {
                                switch(styleData.value.notebookItemType) {
                                case NotebookModel.EpisodeBreakType:
                                case NotebookModel.ActBreakType:
                                    return styleData.value.notebookItemObject ? 1 : 0.5
                                }
                                return 1
                            }

                            source: {
                                switch(styleData.value.notebookItemType) {
                                case NotebookModel.EpisodeBreakType:
                                    return "qrc:/icons/content/episode.png"
                                case NotebookModel.ActBreakType:
                                    return "qrc:/icons/content/act.png"
                                case NotebookModel.NotesType:
                                    switch(styleData.value.notebookItemObject.ownerType) {
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
                                    switch(styleData.value.notebookItemObject.type) {
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
                            id: itemDelegateText
                            padding: 5
                            font.family: fontMetrics.font.family
                            font.pointSize: fontMetrics.font.pointSize
                            font.capitalization: fontMetrics.font.capitalization
                            font.bold: styleData.value.notebookItemType === NotebookModel.CategoryType ||
                                       (styleData.value.notebookItemType === NotebookModel.NotesType &&
                                        styleData.value.notebookItemObject.ownerType === Notes.StructureOwner)
                            text: styleData.value.notebookItemTitle ? styleData.value.notebookItemTitle : ""
                            color: Scrite.app.textColorFor(parent.parent.color)
                            elide: Text.ElideRight
                            width: parent.width-(itemDelegateIcon.visible ? (itemDelegateIcon.width+parent.spacing) : 0)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: {
                        if(styleData.value.notebookItemType === NotebookModel.NoteType) {
                            noteContextMenu.note = styleData.value.notebookItemObject
                            noteContextMenu.popup()
                        } else if(styleData.value.notebookItemType === NotebookModel.NotesType &&
                                  styleData.value.notebookItemObject.ownerType === Notes.CharacterOwner) {
                            characterContextMenu.character = styleData.value.notebookItemObject.character
                            characterContextMenu.characterItem = parent
                            characterContextMenu.popup()
                        }
                    }
                }
            }
            OldControls.TableViewColumn {
                title: "Name"
                role: "notebookItemData"
                width: 300
                movable: false
                resizable: false
            }

            function activateFromCurrentScreenplayElement() {
                var spobj = Scrite.document.screenplay
                var element = spobj.elementAt(spobj.currentElementIndex)
                if(element) {
                    if(element.elementType === ScreenplayElement.BreakElementType)
                        switchTo(element)
                    else
                        switchTo(element.scene.notes)
                }
            }

            property bool activatingScreenplayElement: false
            function activateScreenplayElement(_modelData) {
                activatingScreenplayElement = true
                Qt.callLater( () => { notebookTree.activatingScreenplayElement = false })

                var makeSceneCurrent = function(notes) {
                    if(notes.ownerType === Notes.SceneOwner) {
                        var scene = notes.owner
                        var idxes = scene.screenplayElementIndexList
                        if(idxes.length > 0)
                            Scrite.document.screenplay.currentElementIndex = idxes[0]
                    }
                }

                switch(_modelData.notebookItemType) {
                case NotebookModel.EpisodeBreakType:
                case NotebookModel.ActBreakType:
                    if(_modelData.notebookItemObject)
                        Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.indexOfElement(_modelData.notebookItemObject)
                    break
                case NotebookModel.NotesType:
                    makeSceneCurrent(_modelData.notebookItemObject)
                    break
                case NotebookModel.NoteType:
                    makeSceneCurrent(_modelData.notebookItemObject.notes)
                    break
                default:
                    break
                }
            }

            onClicked: {
                if(Runtime.mainWindowTab !== Runtime.e_StructureTab || Runtime.workspaceSettings.syncCurrentSceneOnNotebook)
                    activateScreenplayElement( notebookModel.modelIndexData(index) )
            }

            onDoubleClicked: {
                activateScreenplayElement( notebookModel.modelIndexData(index) )
                if(isExpanded(index))
                    collapse(index)
                else
                    expand(index)
            }

            function setCurrentIndex(modelIndex) {
                var pmi = modelIndex.parent
                while(pmi.valid) {
                    notebookTree.expand(pmi)
                    pmi = pmi.parent
                }

                var row = 0
                while(1) {
                    var idx = notebookTree.__model.mapRowToModelIndex(row)
                    if(!idx.valid)
                        break
                    if(idx === modelIndex) {
                        notebookTree.__listView.currentIndex = row
                        notebookTree.__listView.positionViewAtIndex(row, ListView.Contain)
                        break
                    }
                    ++row
                }
            }
        }

        Rectangle {
            color: {
                // Keep these colors in sync with actual component colors loaded
                // by notebookContentLoader
                if(!notebookTree.currentData)
                    return Qt.rgba(0,0,0,0)

                switch(notebookTree.currentData.notebookItemType) {
                case NotebookModel.CategoryType:
                    switch(notebookTree.currentData.notebookItemCategory) {
                    case NotebookModel.ScreenplayCategory:
                        return "white"
                    case NotebookModel.UnusedScenesCategory:
                    case NotebookModel.CharactersCategory:
                        return Qt.rgba(0,0,0,0)
                    case NotebookModel.BookmarksCategory:
                        return Scrite.app.translucent(Runtime.colors.primary.c100.background, 0.5)
                    }
                    break
                case NotebookModel.NotesType:
                    switch(notebookTree.currentData.notebookItemObject.ownerType) {
                    case Notes.CharacterOwner: {
                        const character = notebookTree.currentData.notebookItemObject.character
                        return Qt.tint(character.color, "#e7ffffff")
                        }
                    case Notes.SceneOwner: {
                        const notes = notebookTree.currentData.notebookItemObject
                        const scene = notes.scene
                        return Qt.tint(scene.color, "#e7ffffff")
                        }
                    default:
                        return Scrite.app.translucent(Runtime.colors.primary.c100.background, 0.5)
                    }
                case NotebookModel.NoteType:
                    switch(notebookTree.currentData.notebookItemObject.type) {
                    case Note.TextNoteType:
                    case Note.FormNoteType:
                        const note = notebookTree.currentData.notebookItemObject
                        return Qt.tint(note.color, "#E7FFFFFF")
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

                property int currentNotebookItemId: notebookTree.currentData ? notebookTree.currentData.notebookItemId : -1

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
                    if(!notebookTree.currentData)
                        return unknownComponent

                    switch(notebookTree.currentData.notebookItemType) {
                    case NotebookModel.CategoryType:
                        switch(notebookTree.currentData.notebookItemCategory) {
                        case NotebookModel.ScreenplayCategory:
                            return screenplayComponent
                        case NotebookModel.UnusedScenesCategory:
                            return unusedScenesComponent
                        case NotebookModel.CharactersCategory:
                            return charactersComponent
                        case NotebookModel.BookmarksCategory:
                            return bookmarksComponent
                        }
                        break
                    case NotebookModel.NotesType:
                        switch(notebookTree.currentData.notebookItemObject.ownerType) {
                        case Notes.CharacterOwner:
                            return characterNotesComponent
                        case Notes.SceneOwner:
                            return sceneNotesComponent
                        default:
                            return notesComponent
                        }
                    case NotebookModel.NoteType:
                        switch(notebookTree.currentData.notebookItemObject.type) {
                        case Note.TextNoteType:
                            return textNoteComponent
                        case Note.FormNoteType:
                            return formNoteComponent
                        case Note.CheckListNoteType:
                            return checkListNoteComponent
                        }
                        break
                    case NotebookModel.EpisodeBreakType:
                    case NotebookModel.ActBreakType:
                        return episodeOrActBreakComponent
                    }

                    return unknownComponent
                }
                onLoaded: item.componentData = notebookTree.currentData

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
                    color: Scrite.app.translucent(Runtime.colors.primary.c600.background,0.85)
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
                                if(notebookTree.currentNote)
                                    return "Are you sure you want to delete this note?"
                                if(notebookTree.currentCharacter)
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
                                visible: notebookTree.currentNote || notebookTree.currentCharacter
                            }

                            VclButton {
                                text: notebookTree.currentNote || notebookTree.currentCharacter ? "No" : "OK"
                                focusPolicy: Qt.NoFocus
                                onClicked: deleteConfirmationBox.active = false
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: unknownComponent

        Item {
            property var componentData
        }
    }

    Component {
        id: bookmarksComponent

        Rectangle {
            id: bookmarksItem
            property var componentData
            color: Scrite.app.translucent(Runtime.colors.primary.c100.background, 0.5)
            border.width: 1
            border.color: Runtime.colors.primary.borderColor
            clip: true

            GridView {
                id: bookmarksView
                anchors.fill: parent
                anchors.rightMargin: contentHeight > height ? 17 : 12
                model: notebookModel.bookmarkedNotes
                property real idealCellWidth: Math.min(250,width)
                property int nrColumns: Math.floor(width/idealCellWidth)
                cellWidth: width/nrColumns
                cellHeight: 150
                ScrollBar.vertical: bookmarksViewScrollbar
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
                    width: bookmarksView.cellWidth
                    height: bookmarksView.cellHeight

                    Rectangle {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.topMargin: 12
                        border.width: 1
                        border.color: bookmarksView.currentIndex === index ? "darkgray" : Runtime.colors.primary.borderColor

                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            Row {
                                width: parent.width
                                spacing: 5

                                Image {
                                    width: 32
                                    height: 32
                                    anchors.verticalCenter: parent.verticalCenter
                                    mipmap: true
                                    source: {
                                        if(Scrite.app.typeName(noteObject) === "Notes") {
                                            switch(noteObject.ownerType) {
                                            case Notes.SceneOwner:
                                                return "qrc:/icons/content/scene.png"
                                            case Notes.CharacterOwner:
                                                return "qrc:/icons/content/person_outline.png"
                                            case Notes.BreakOwner:
                                                return "qrc:/icons/content/story.png"
                                            default:
                                                break
                                            }
                                        } else if(Scrite.app.typeName(noteObject) === "Character")
                                            return "qrc:/icons/content/person_outline.png"
                                        else if(Scrite.app.typeName(noteObject) === "Note") {
                                            switch(styleData.value.notebookItemObject.type) {
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
                                    id: headingText
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    font.bold: true
                                    maximumLineCount: 1
                                    width: parent.width-32-parent.spacing
                                    elide: Text.ElideRight
                                    text: noteTitle
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            VclLabel {
                                width: parent.width
                                height: parent.height - headingText.height - parent.spacing
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                                text: noteSummary
                                color: headingText.color
                                opacity: 0.75
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: bookmarksView.currentIndex = index
                        onDoubleClicked: {
                            bookmarksView.currentIndex = index
                            switchTo(noteObject)
                        }
                    }
                }
            }

            VclScrollBar {
                id: bookmarksViewScrollbar
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                orientation: Qt.Vertical
                flickable: bookmarksView
            }
        }
    }

    property int sceneNotesTabIndex: 0
    Component {
        id: sceneNotesComponent

        Rectangle {
            id: sceneNotesItem
            property var componentData
            property Notes notes: componentData ? componentData.notebookItemObject : null
            property Scene scene: notes ? notes.scene : null
            color: Qt.tint(scene.color, "#e7ffffff")

            TextTabBar {
                id: sceneTabBar
                tabIndex: sceneNotesTabIndex
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 8
                onTabIndexChanged: sceneNotesTabIndex = tabIndex
                name: componentData ? componentData.notebookItemTitle.substr(0, componentData.notebookItemTitle.indexOf(']')+1) : "Scene"
                tabs: ["Synopsis", "Relationships", "Notes", "Comments"]
            }

            Item {
                id: sceneTabContentArea
                anchors.top: sceneTabBar.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.topMargin: 10
                clip: true

                Item {
                    visible: sceneTabBar.tabIndex === 0
                    width: sceneTabContentArea.width
                    height: sceneTabContentArea.height

                    TabSequenceManager {
                        id: sceneTabSequence
                        enabled: parent.visible
                    }

                    Column {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: sceneAttachments.top
                        anchors.margins: 10
                        anchors.bottomMargin: 0
                        spacing: 10

                        EventFilter.events: [EventFilter.Wheel]
                        EventFilter.onFilter: {
                            EventFilter.forwardEventTo(sceneSynopsisField)
                            result.filter = true
                            result.accepted = true
                        }

                        VclTextField {
                            id: sceneHeadingField

                            TabSequenceItem.manager: sceneTabSequence
                            TabSequenceItem.sequence: 0

                            text: scene.heading.text
                            label: ""
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            wrapMode: Text.WordWrap
                            placeholderText: "Scene Heading"
                            readOnly: Scrite.document.readOnly
                            enabled: scene.heading.enabled
                            onEditingComplete: scene.heading.parseFrom(text)
                            font.capitalization: Font.AllUppercase
                            font.family: Scrite.document.formatting.elementFormat(SceneElement.Heading).font.family
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize+2
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        VclTextField {
                            id: sceneTitleField

                            TabSequenceItem.manager: sceneTabSequence
                            TabSequenceItem.sequence: 1

                            text: scene.structureElement.nativeTitle
                            label: ""
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            wrapMode: Text.WordWrap
                            placeholderText: "Scene Title"
                            readOnly: Scrite.document.readOnly
                            onEditingComplete: scene.structureElement.title = text
                            backTabItem: sceneHeadingField
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Flow {
                            id: sceneCharactersList
                            spacing: 5
                            width: sceneTitleField.width
                            anchors.horizontalCenter: parent.horizontalCenter
                            flow: Flow.LeftToRight

                            VclLabel {
                                id: sceneCharactersListHeading
                                text: "Characters: "
                                font.bold: true
                                topPadding: 5
                                bottomPadding: 5
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                visible: !scene.hasCharacters
                            }

                            Repeater {
                                model: scene ? scene.characterNames : 0

                                TagText {
                                    id: characterNameLabel
                                    property var colors: containsMouse ? Runtime.colors.accent.c900 : Runtime.colors.accent.c600
                                    border.width: 1
                                    border.color: colors.text
                                    color: colors.background
                                    textColor: colors.text
                                    text: modelData
                                    topPadding: 5; bottomPadding: 5
                                    leftPadding: 10; rightPadding: 10
                                    font.family: "Courier Prime"
                                    font.capitalization: Font.AllUppercase
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    closable: scene.isCharacterMute(modelData) && !Scrite.document.readOnly
                                    onCloseRequest: {
                                        if(!Scrite.document.readOnly)
                                            scene.removeMuteCharacter(modelData)
                                    }
                                }
                            }

                            Loader {
                                id: newCharacterNameInputLoader
                                active: false
                                width: active && item ? Math.max(250, item.contentWidth) : 0
                                visible: active
                                sourceComponent: VclTextField {
                                    id: newCharacterNameInput
                                    readOnly: Scrite.document.readOnly
                                    font.family: "Courier Prime"
                                    font.capitalization: length > 0 ? Font.AllUppercase : Font.MixedCase
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    wrapMode: Text.NoWrap
                                    completionStrings: Scrite.document.structure.characterNames
                                    placeholderText: "New Character Name"
                                    onEditingFinished: {
                                        if(text === "")
                                            tabItem.forceActiveFocus()
                                        else {
                                            scene.addMuteCharacter(text)
                                            clear()
                                        }
                                    }
                                    onActiveFocusChanged: {
                                        if(!activeFocus)
                                            newCharacterNameInputLoader.active = false
                                    }
                                    tabItemUponReturn: false
                                    tabItem: synopsisContentTabView.currentTabIndex === 0 ? synopsisContentTabView.currentTabItem.textArea : null
                                    backTabItem: sceneTitleField
                                    Component.onCompleted: forceActiveFocus()
                                }
                            }

                            Image {
                                source: "qrc:/icons/content/add_box.png"
                                width: sceneCharactersListHeading.height
                                height: width
                                opacity: 0.5
                                visible: !newCharacterNameInputLoader.active
                                enabled: !Scrite.document.readOnly

                                MouseArea {
                                    ToolTip.text: "Click here to add a new character to this scene."
                                    ToolTip.delay: 1000
                                    ToolTip.visible: containsMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onContainsMouseChanged: parent.opacity = containsMouse ? 1 : 0.5
                                    onClicked: newCharacterNameInputLoader.active = true
                                }
                            }
                        }

                        TrapeziumTabView {
                            id: synopsisContentTabView
                            tabNames: ["Synopsis", "Featured Photo"]
                            tabColor: scene.color
                            currentTabContent: currentTabIndex === 0 ? sceneSynopsisFieldComponent : featuredPhotoComponent
                            currentTabIndex: Runtime.screenplayEditorSettings.commentsPanelTabIndex
                            onCurrentTabIndexChanged: Runtime.screenplayEditorSettings.commentsPanelTabIndex = currentTabIndex
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            height: parent.height - sceneHeadingField.height - sceneTitleField.height - sceneCharactersList.height - parent.spacing*3
                            anchors.horizontalCenter: parent.horizontalCenter

                            Component {
                                id: sceneSynopsisFieldComponent

                                ColumnLayout {
                                    property alias textArea: sceneSynopsisField.textArea

                                    FlickableTextArea {
                                        id: sceneSynopsisField

                                        Layout.fillWidth: true
                                        Layout.fillHeight: true

                                        TabSequenceItem.manager: sceneTabSequence
                                        TabSequenceItem.sequence: 2
                                        TabSequenceItem.onAboutToReceiveFocus: Qt.callLater(textArea.forceActiveFocus)

                                        // Unfortunately, focus scope doesnt really work!
                                        EventFilter.target: textArea
                                        EventFilter.events: [EventFilter.KeyPress]
                                        EventFilter.active: textArea.activeFocus
                                        EventFilter.onFilter: (watched, event) => {
                                                                  if(event.key === Qt.Key_Tab)
                                                                    TabSequenceItem.focusNext()
                                                                  else if(event.key === Qt.Key_Backtab)
                                                                    TabSequenceItem.focusPrevious()
                                                              }

                                        text: scene.synopsis
                                        readOnly: Scrite.document.readOnly
                                        background: Rectangle {
                                            color: Runtime.colors.primary.windowColor
                                            opacity: 0.15
                                        }
                                        placeholderText: "Scene Synopsis"
                                        undoRedoEnabled: true
                                        adjustTextWidthBasedOnScrollBar: false

                                        onTextChanged: scene.synopsis = text
                                    }

                                    IndexCardFields {
                                        id: indexCardFields

                                        Layout.fillWidth: true

                                        lod: eHIGH
                                        visible: hasFields

                                        structureElement: scene.structureElement

                                        startTabSequence: 3
                                        tabSequenceEnabled: true
                                        tabSequenceManager: sceneTabSequence
                                    }
                                }
                            }

                            Component {
                                id: featuredPhotoComponent

                                SceneFeaturedImage {
                                    scene: sceneNotesItem.scene
                                    fillModeAttrib: "notebookFillMode"
                                    defaultFillMode: Image.PreserveAspectFit
                                    mipmap: true
                                }
                            }
                        }
                    }

                    AttachmentsView {
                        id: sceneAttachments
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        attachments: scene ? scene.attachments : null
                    }

                    AttachmentsDropArea {
                        id: sceneAttachmentsDropArea
                        target: scene ? scene.attachments : null
                        allowMultiple: true
                        anchors.fill: synopsisContentTabView.currentTabIndex === 1 ? sceneAttachments : parent
                    }
                }

                Loader {
                    width: sceneTabContentArea.width
                    height: sceneTabContentArea.height
                    visible: sceneTabBar.tabIndex === 1
                    active: Runtime.appFeatures.characterRelationshipGraph.enabled
                    sourceComponent: CharacterRelationshipsGraphView {
                        id: crGraphView
                        scene: null
                        structure: null
                        showBusyIndicator: true
                        onCharacterDoubleClicked: {
                            var ch = Scrite.document.structure.findCharacter(characterName)
                            if(ch)
                                switchTo(ch.notes)
                        }
                        function prepare() {
                            if(visible) {
                                scene = sceneNotesItem.scene
                                structure = Scrite.document.structure
                                showBusyIndicator = false
                            }
                        }
                        Component.onCompleted: Utils.execLater(sceneTabContentArea, 100, prepare)
                        onVisibleChanged: Utils.execLater(sceneTabContentArea, 100, prepare)

                        property bool pdfExportPossible: !graphIsEmpty && visible
                        onPdfExportPossibleChanged: Announcement.shout("4D37E093-1F58-4978-8060-CD6B9AD4E03C", pdfExportPossible ? 1 : -1)
                        Component.onDestruction: if(pdfExportPossible) Announcement.shout("4D37E093-1F58-4978-8060-CD6B9AD4E03C", -1)

                        Announcement.onIncoming: (type,data) => {
                            const stype = ""+type
                            const sdata = ""+data
                            if(stype === "3F96A262-A083-478C-876E-E3AFC26A0507") {
                                if(sdata === "refresh") {
                                    crGraphView.resetGraph()
                                    refreshButton.refreshAck = true
                                } else if(sdata == "pdfexport")
                                    crGraphView.exportToPdf(pdfExportButton)
                            }
                        }
                    }

                    DisabledFeatureNotice {
                        color: Qt.rgba(0,0,0,0)
                        anchors.fill: parent
                        visible: !Runtime.appFeatures.characterRelationshipGraph.enabled
                        featureName: "Relationship Map"
                    }
                }

                Loader {
                    width: sceneTabContentArea.width
                    height: sceneTabContentArea.height
                    sourceComponent: notesComponent
                    active: sceneNotesItem.notes
                    onLoaded: item.notes = sceneNotesItem.notes
                    visible: sceneTabBar.tabIndex === 2
                }

                Item {
                    width: sceneTabContentArea.width
                    height: sceneTabContentArea.height
                    visible: sceneTabBar.tabIndex === 3

                    EventFilter.events: [EventFilter.Wheel]
                    EventFilter.onFilter: {
                        EventFilter.forwardEventTo(sceneCommentsField)
                        result.filter = true
                        result.accepted = true
                    }

                    Item {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10

                        FlickableTextArea {
                            id: sceneCommentsField
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            height: parent.height
                            text: scene.comments
                            placeholderText: "Scene Comments"
                            readOnly: Scrite.document.readOnly
                            onTextChanged: scene.comments = text
                            undoRedoEnabled: true
                            ScrollBar.vertical: sceneCommentsVScrollBar
                            adjustTextWidthBasedOnScrollBar: false
                            anchors.horizontalCenter: parent.horizontalCenter
                            background: Rectangle {
                                color: Runtime.colors.primary.windowColor
                                opacity: 0.15
                            }
                        }
                    }

                    VclScrollBar {
                        id: sceneCommentsVScrollBar
                        orientation: Qt.Vertical
                        flickable: sceneCommentsField
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                    }
                }
            }
        }
    }

    Component {
        id: notesComponent

        Rectangle {
            id: notesSummary
            property var componentData
            property Notes notes: componentData.notebookItemObject
            property real minimumNoteSize: Math.max(200, Scrite.window.width*0.15)
            property real noteSize: notesFlick.width > minimumNoteSize ? notesFlick.width / Math.floor(notesFlick.width/minimumNoteSize) : notesFlick.width
            clip: true
            color: Scrite.app.translucent(Runtime.colors.primary.c100.background, 0.5)
            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            // Report support
            property bool hasReport: {
                return notes.ownerType === Notes.StructureOwner || notes.ownerType === Notes.SceneOwner
            }
            property string reportDescription: {
                switch(notes.ownerType) {
                case Notes.StructureOwner:
                    return "Exports all story notes into a PDF or ODT."
                case Notes.SceneOwner:
                    return "Exports all scene notes into a PDF or ODT."
                }
                return ""
            }
            function createReportGenerator() {
                var generator = Scrite.document.createReportGenerator("Notebook Report")
                generator.section = notes.owner
                return generator
            }

            Flickable {
                id: notesFlick
                anchors.fill: parent
                anchors.margins: 20
                property int currentIndex: 0
                contentWidth: width
                contentHeight: noteItemsFlow.height
                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                Flow {
                    id: noteItemsFlow
                    width: notesFlick.width

                    Repeater {
                        model: notes

                        Item {
                            width: noteSize; height: noteSize

                            BoxShadow {
                                anchors.fill: noteVisual
                                visible: notesFlick.currentIndex === index
                                opacity: 0.5
                            }

                            Rectangle {
                                id: noteVisual
                                anchors.fill: parent
                                anchors.margins: 10
                                color: notesFlick.currentIndex === index ? Qt.tint(objectItem.color, "#A0FFFFFF") : Qt.tint(objectItem.color, "#E7FFFFFF")

                                Behavior on color {
                                    enabled: Runtime.applicationSettings.enableAnimations
                                    ColorAnimation { duration: 250 }
                                }

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 8

                                    VclLabel {
                                        id: headingText
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                        font.bold: true
                                        maximumLineCount: 1
                                        width: parent.width
                                        elide: Text.ElideRight
                                        text: objectItem.title
                                        color: Scrite.app.isLightColor(parent.parent.color) ? Qt.rgba(0.2,0.2,0.2,1.0) : Qt.rgba(0.9,0.9,0.9,1.0)
                                    }

                                    VclLabel {
                                        width: parent.width
                                        height: parent.height - headingText.height - parent.spacing
                                        wrapMode: Text.WordWrap
                                        elide: Text.ElideRight
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                                        text: objectItem.type === Note.TextNoteType ? deltaDoc.plainText : objectItem.summary
                                        color: headingText.color
                                        opacity: 0.75

                                        DeltaDocument {
                                            id: deltaDoc
                                            content: objectItem.type === Note.TextNoteType ? objectItem.content : {}
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: {
                                    parent.forceActiveFocus()
                                    notesFlick.currentIndex = index
                                    if(mouse.button === Qt.RightButton) {
                                        noteContextMenu.note = objectItem
                                        noteContextMenu.popup()
                                    }
                                }
                                onDoubleClicked: {
                                    parent.forceActiveFocus()
                                    switchTo(objectItem)
                                }
                            }
                        }
                    }

                    Item {
                        width: noteSize; height: noteSize
                        visible: !Scrite.document.readOnly

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 10
                            color: Scrite.app.translucent(Runtime.colors.primary.c100.background, 0.5)
                            border.width: 1
                            border.color: Runtime.colors.primary.borderColor

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    newNoteMenu.notes = notesSummary.notes
                                    newNoteMenu.popup(newNoteButton, 0, newNoteButton.height)
                                }
                            }
                        }

                        FlatToolButton {
                            id: newNoteButton
                            anchors.centerIn: parent
                            ToolTip.text: "Add a new text or form note."
                            iconSource: "qrc:/icons/action/note_add.png"
                            down: newNoteMenu.visible
                            onClicked: {
                                newNoteMenu.notes = notesSummary.notes
                                newNoteMenu.popup(newNoteButton, 0, newNoteButton.height)
                            }
                        }
                    }
                }

                ScrollBar.vertical: notesFlickScrollBar
            }

            VclScrollBar {
                id: notesFlickScrollBar
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                orientation: Qt.Vertical
                flickable: notesFlick
            }
        }
    }

    Component {
        id: textNoteComponent

        Rectangle {
            property var componentData
            property Note note: componentData ? componentData.notebookItemObject : null
            color: Qt.tint(note.color, "#E7FFFFFF")

            // Report support
            property bool hasReport: true
            property string reportDescription: "Export this text note as a PDF or ODT."
            function createReportGenerator() {
                var generator = Scrite.document.createReportGenerator("Notebook Report")
                generator.section = note
                return generator
            }

            function deleteSelf() {
                var notes = note.notes
                notes.removeNote(note)
                switchTo(notes)
            }

            TextNoteView {
                anchors.fill: parent
                note: parent.note
            }
        }
    }

    Component {
        id: formNoteComponent

        Rectangle {
            id: formNoteItem
            property var componentData
            property Note note: componentData.notebookItemObject
            color: Qt.tint(note.color, "#E7FFFFFF")

            // Report support
            property bool hasReport: true
            property string reportDescription: "Export this form as a PDF or ODT."
            function createReportGenerator() {
                var generator = Scrite.document.createReportGenerator("Notebook Report")
                generator.section = note
                return generator
            }

            function deleteSelf() {
                var notes = note.notes
                notes.removeNote(note)
                switchTo(notes)
            }

            FormView {
                anchors.fill: parent
                note: parent.note
            }
        }
    }

    Component {
        id: checkListNoteComponent

        Rectangle {
            id: checkListNoteItem
            property var componentData
            property Note note: componentData.notebookItemObject
            color: Qt.tint(note.color, "#E7FFFFFF")

            // Report support
            property bool hasReport: true
            property string reportDescription: "Export this checklist as a PDF or ODT."
            function createReportGenerator() {
                checkListView.commitPendingItems()

                var generator = Scrite.document.createReportGenerator("Notebook Report")
                generator.section = note
                return generator
            }

            function deleteSelf() {
                var notes = note.notes
                notes.removeNote(note)
                switchTo(notes)
            }

            CheckListView {
                id: checkListView
                note: checkListNoteItem.note
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: checkListAttachments.top
            }

            AttachmentsView {
                id: checkListAttachments
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                attachments: checkListNoteItem.note ? checkListNoteItem.note.attachments : null
            }

            AttachmentsDropArea {
                id: checkListAttachmentsDropArea
                anchors.fill: parent
                allowMultiple: true
                target: checkListNoteItem.note ? checkListNoteItem.note.attachments : null
            }
        }
    }

    Component {
        id: episodeOrActBreakComponent

        Item {
            property var componentData
            property ScreenplayElement breakElement: componentData.notebookItemObject
            property string breakKind: componentData.notebookItemType === NotebookModel.EpisodeBreakType ? "Episode" : "Act"

            Loader {
                active: breakElement !== null
                anchors.fill: parent
                sourceComponent: Item {

                    EventFilter.events: [EventFilter.Wheel]
                    EventFilter.onFilter: {
                        EventFilter.forwardEventTo(breakElementSummaryField)
                        result.filter = true
                        result.accepted = true
                    }

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: breakElementAttachmentsView.top
                        anchors.margins: 10
                        anchors.bottomMargin: 0
                        spacing: 10

                        Row {
                            id: breakElementHeadingRow
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 10

                            VclLabel {
                                id: headingLabel
                                text: breakElement.breakTitle + ": "
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 3
                                anchors.baseline: breakElementHeadingField.baseline
                            }

                            VclTextField {
                                id: breakElementHeadingField
                                text: breakElement.breakSubtitle
                                width: parent.width - headingLabel.width - parent.spacing
                                label: ""
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 5
                                placeholderText: breakKind + " Name"
                                onTextChanged: breakElement.breakSubtitle = text
                                tabItem: breakElementSummaryField.textArea
                            }
                        }

                        FlickableTextArea {
                            id: breakElementSummaryField
                            placeholderText: breakKind + " Summary ..."
                            text: breakElement.breakSummary
                            onTextChanged: breakElement.breakSummary = text
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            height: parent.height - breakElementHeadingRow.height - parent.spacing
                            anchors.horizontalCenter: parent.horizontalCenter
                            backTabItem: breakElementHeadingField
                            adjustTextWidthBasedOnScrollBar: false
                            ScrollBar.vertical: breakSummaryVScrollBar
                            background: Rectangle {
                                color: Runtime.colors.primary.windowColor
                                opacity: 0.15
                            }
                        }
                    }


                    VclScrollBar {
                        id: breakSummaryVScrollBar
                        orientation: Qt.Vertical
                        flickable: breakElementSummaryField
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: breakElementAttachmentsView.top
                    }

                    AttachmentsView {
                        id: breakElementAttachmentsView
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        attachments: breakElement ? breakElement.attachments : null
                    }
                }
            }

            AttachmentsDropArea {
                id: attachmentsDropArea
                anchors.fill: parent
                allowMultiple: true
                target: breakElement ? breakElement.attachments : null
            }

            VclLabel {
                width: parent.width * 0.6
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: "Create " + breakKind.toLowerCase() + " break in the screenplay to capture a summary for it."
                wrapMode: Text.WordWrap
                visible: breakElement === null
            }
        }
    }

    property int screenplayNotesTabIndex: 0
    Component {
        id: screenplayComponent

        Rectangle {
            property var componentData
            property Screenplay screenplay: Scrite.document.screenplay

            // Report support
            property bool hasReport: true
            property string reportDescription: "Export notes of all scenes in the screenplay into a PDF or ODT."
            function createReportGenerator() {
                var generator = Scrite.document.createReportGenerator("Notebook Report")
                generator.section = screenplay
                return generator
            }

            FontMetrics {
                id: screenplayFontMetrics
                font.family: Scrite.document.formatting.defaultFont.family
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }

            TextTabBar {
                id: screenplayTabBar
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 8
                tabIndex: screenplayNotesTabIndex
                onTabIndexChanged: screenplayNotesTabIndex = Math.min(tabIndex, 2)
                name: "Screenplay"
                tabs: ["Title Page", "Logline", "Notes", "Stats"]
            }

            Item {
                id: screenplayTabContentArea
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: screenplayTabBar.bottom
                anchors.bottom: parent.bottom
                anchors.topMargin: 10
                clip: true

                Item {
                    width: screenplayTabContentArea.width
                    height: screenplayTabContentArea.height
                    visible: screenplayTabBar.tabIndex === 0

                    Flickable {
                        id: titlePageFlickable
                        width: Math.min(contentWidth, parent.width)
                        height: Math.min(contentHeight, parent.height)
                        contentWidth: titlePageLayout.width
                        contentHeight: titlePageLayout.height
                        anchors.centerIn: parent
                        property bool vscrollBarRequired: contentHeight > height
                        property bool hscrollBarRequired: contentWidth > width
                        ScrollBar.vertical: titlePageVScrollBar
                        ScrollBar.horizontal: titlePageHScrollBar
                        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                        Column {
                            id: titlePageLayout
                            width: Math.max(550, screenplayTabContentArea.width)
                            spacing: 10
                            property real maxWidth: Math.min(550, width)

                            Image {
                                width: {
                                    switch(Scrite.document.screenplay.coverPagePhotoSize) {
                                    case Screenplay.SmallCoverPhoto:
                                        return parent.maxWidth / 4
                                    case Screenplay.MediumCoverPhoto:
                                        return parent.maxWidth / 2
                                    }
                                    return parent.maxWidth
                                }
                                source: visible ? "file:///" + Scrite.document.screenplay.coverPagePhoto : ""
                                visible: Scrite.document.screenplay.coverPagePhoto !== ""
                                smooth: true; mipmap: true
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: -border.width - 4
                                    color: Qt.rgba(1,1,1,0.1)
                                    border { width: 2; color: titleLink.hoverColor }
                                    visible: coverPicMouseArea.containsMouse
                                }

                                MouseArea {
                                    id: coverPicMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: TitlePageDialog.launch()
                                }
                            }

                            Link {
                                id: titleLink
                                font.family: screenplayFontMetrics.font.family
                                font.pointSize: screenplayFontMetrics.font.pointSize + 2
                                font.bold: true
                                font.underline: containsMouse
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.title === "" ? "<untitled>" : Scrite.document.screenplay.title
                                onClicked: TitlePageDialog.launch()
                            }

                            Link {
                                font.family: screenplayFontMetrics.font.family
                                font.pointSize: screenplayFontMetrics.font.pointSize
                                font.bold: true
                                font.underline: containsMouse
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.subtitle
                                visible: Scrite.document.screenplay.subtitle !== ""
                                onClicked: TitlePageDialog.launch()
                            }

                            Column {
                                width: parent.width
                                spacing: 0

                                VclLabel {
                                    font: screenplayFontMetrics.font
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    horizontalAlignment: Text.AlignHCenter
                                    text: "Written By"
                                }

                                Link {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize
                                    font.underline: containsMouse
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    horizontalAlignment: Text.AlignHCenter
                                    text: (Scrite.document.screenplay.author === "" ? "<unknown author>" : Scrite.document.screenplay.author)
                                    onClicked: TitlePageDialog.launch()
                                }
                            }

                            Link {
                                font.family: screenplayFontMetrics.font.family
                                font.pointSize: screenplayFontMetrics.font.pointSize
                                font.underline: containsMouse
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.version === "" ? "Initial Version" : Scrite.document.screenplay.version
                                onClicked: TitlePageDialog.launch()
                            }

                            Link {
                                font.family: screenplayFontMetrics.font.family
                                font.pointSize: screenplayFontMetrics.font.pointSize
                                font.underline: containsMouse
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.basedOn
                                visible: Scrite.document.screenplay.basedOn !== ""
                                onClicked: TitlePageDialog.launch()
                            }

                            Item { width: parent.width; height: parent.spacing/2 }

                            Column {
                                spacing: parent.spacing/2
                                width: parent.width * 0.5
                                anchors.right: parent.horizontalCenter
                                anchors.rightMargin: -width*0.25

                                Link {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    font.underline: containsMouse
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.contact
                                    visible: text !== ""
                                    onClicked: TitlePageDialog.launch()
                                }

                                Link {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    font.underline: containsMouse
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.address
                                    visible: text !== ""
                                    onClicked: TitlePageDialog.launch()
                                }

                                Link {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    font.underline: containsMouse
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.phoneNumber
                                    visible: text !== ""
                                    onClicked: TitlePageDialog.launch()
                                }

                                Link {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    font.underline: containsMouse
                                    color: "blue"
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.email
                                    visible: text !== ""
                                    onClicked: TitlePageDialog.launch()
                                }

                                Link {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    font.underline: containsMouse
                                    color: "blue"
                                    width: parent.width
                                    elide: Text.ElideRight
                                    text: Scrite.document.screenplay.website
                                    visible: text !== ""
                                    onClicked: TitlePageDialog.launch()
                                }
                            }
                        }
                    }

                    FlatToolButton {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.rightMargin: titlePageFlickable.vscrollBarRequired ? 20 : 0
                        iconSource: "qrc:/icons/action/edit_title_page.png"
                        onClicked: TitlePageDialog.launch()
                        enabled: !Scrite.document.readOnly
                    }

                    VclScrollBar {
                        id: titlePageVScrollBar
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        orientation: Qt.Vertical
                        flickable: titlePageFlickable
                    }

                    VclScrollBar {
                        id: titlePageHScrollBar
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        orientation: Qt.Horizontal
                        flickable: titlePageFlickable
                    }
                }

                Item {
                    width: screenplayTabContentArea.width
                    height: screenplayTabContentArea.height
                    visible: screenplayTabBar.tabIndex === 1

                    EventFilter.events: [EventFilter.Wheel]
                    EventFilter.onFilter: {
                        EventFilter.forwardEventTo(loglineFieldArea)
                        result.filter = true
                        result.accepted = true
                    }

                    Item {
                        id: loglineForm
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10

                        TextLimiterSyntaxHighlighterDelegate {
                            id: textLimitHighlighter
                            textLimiter: TextLimiter {
                                id: textLimiter
                                maxWordCount: 50
                                maxLetterCount: 240
                                countMode: TextLimiter.CountInText
                            }
                        }

                        Column {
                            spacing: 0
                            x: Math.max(0, (parent.width-width)/2)
                            width: Math.min(Runtime.idealFontMetrics.averageCharacterWidth*50, parent.width-20)

                            VclLabel {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                topPadding: 20
                                bottomPadding: 10
                                text: "A logline should swiftly convey what a screenplay is about, including the main character, central conflict, setup and antagonist."
                            }

                            Link {
                                width: parent.width
                                elide: Text.ElideMiddle
                                text: "https://online.pointpark.edu/screenwriting/loglines/"
                                onClicked: Qt.openUrlExternally(text)
                                bottomPadding: 20
                            }

                            FlickableTextArea {
                                id: loglineFieldArea
                                text: Scrite.document.screenplay.logline
                                onTextChanged: Scrite.document.screenplay.logline = text
                                placeholderText: "Type your logline here."
                                font.family: Scrite.document.displayFormat.defaultFont2.family
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                                width: parent.width
                                height: Math.max(Runtime.idealFontMetrics.lineSpacing*10, contentHeight+10)
                                readOnly: Scrite.document.readOnly
                                undoRedoEnabled: true
                                ScrollBar.vertical: loglineVScrollBar
                                adjustTextWidthBasedOnScrollBar: false
                                background: Rectangle {
                                    color: Runtime.colors.primary.windowColor
                                    opacity: 0.15
                                }
                                Component.onCompleted: syntaxHighlighter.addDelegate(textLimitHighlighter)
                            }

                            VclLabel {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                topPadding: 5
                                text: (textLimiter.limitReached ? "WARNING: " : "") + "Words: " + textLimiter.wordCount + "/" + textLimiter.maxWordCount +
                                    ", Letters: " + textLimiter.letterCount + "/" + textLimiter.maxLetterCount
                                color: textLimiter.limitReached ? "darkred" : Runtime.colors.primary.a700.background
                            }
                        }
                    }

                    VclScrollBar {
                        id: loglineVScrollBar
                        orientation: Qt.Vertical
                        flickable: loglineFieldArea
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                    }
                }

                Loader {
                    width: screenplayTabContentArea.width
                    height: screenplayTabContentArea.height
                    sourceComponent: notesComponent
                    onLoaded: item.notes = Scrite.document.structure.notes
                    visible: screenplayTabBar.tabIndex === 2
                }

                Loader {
                    width: screenplayTabContentArea.width
                    height: screenplayTabContentArea.height
                    sourceComponent: storyStatsReport
                    visible: screenplayTabBar.tabIndex === 3
                    active: false
                    onVisibleChanged: {
                        if(visible)
                            Qt.callLater(function() { active = true })
                    }
                }
            }
        }
    }

    Component {
        id: unusedScenesComponent

        Item {
            property var componentData

            VclLabel {
                anchors.fill: parent
                anchors.margins: 20
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: "<b><font size=\"+2\">Unused Scenes</font></b><br/><br/>Unused scenes are those that are placed on structure but are not yet dragged into the screenplay (or timeline). Click on any of the unused scenes in the tree to the left to view their notes."
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        }
    }

    property int charactersNotesTabIndex: 0
    Component {
        id: charactersComponent

        Item {
            property var componentData

            // Report support
            property bool hasReport: true
            property string reportDescription: "Export information about all characters."
            function createReportGenerator() {
                var generator = Scrite.document.createReportGenerator("Notebook Report")
                generator.section = Scrite.document.structure
                generator.options = { "intent": "characters" }
                return generator
            }

            TextTabBar {
                id: charactersTabBar
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 8
                tabIndex: charactersNotesTabIndex
                onTabIndexChanged: charactersNotesTabIndex = tabIndex
                name: "Characters"
                tabs: ["List", "Relationships"]
            }

            Item {
                id: charactersTabContentArea
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: charactersTabBar.bottom
                anchors.bottom: parent.bottom
                anchors.topMargin: 10

                Item {
                    width: charactersTabContentArea.width
                    height: charactersTabContentArea.height
                    visible: charactersTabBar.tabIndex === 0

                    SortFilterObjectListModel {
                        id: sortedCharactersModel
                        sourceModel: Scrite.document.structure.charactersModel
                        sortByProperty: "name"
                    }

                    GridView {
                        id: charactersView
                        anchors.fill: parent
                        anchors.rightMargin: contentHeight > height ? 17 : 0
                        clip: true
                        model: sortedCharactersModel
                        property real idealCellWidth: Math.min(250,width)
                        property int nrColumns: Math.floor(width/idealCellWidth)
                        cellWidth: width/nrColumns
                        cellHeight: 120
                        ScrollBar.vertical: charactersListViewScrollBar
                        highlightMoveDuration: 0
                        highlight: Item {
                            BoxShadow {
                                anchors.fill: highlightedItem
                                opacity: 0.5
                            }
                            Item {
                                id: highlightedItem
                                anchors.fill: parent
                                anchors.margins: 5
                            }
                        }
                        delegate: Item {
                            width: charactersView.cellWidth
                            height: charactersView.cellHeight
                            property Character character: objectItem

                            function polishStr(val,defval) {
                                return val === "" ? defval : val
                            }

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 5
                                color: Qt.tint(character.color, charactersView.currentIndex === index ? "#A0FFFFFF" : "#E7FFFFFF")
                                border.width: 1
                                border.color: Scrite.app.isLightColor(character.color) ? (charactersView.currentIndex === index ? "darkgray" : Runtime.colors.primary.borderColor) : character.color

                                Row {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 10

                                    Image {
                                        width: parent.height
                                        height: parent.height
                                        source: {
                                            if(character.hasKeyPhoto > 0)
                                                return "file:///" + character.keyPhoto
                                            return "qrc:/icons/content/character_icon.png"
                                        }
                                        fillMode: Image.PreserveAspectCrop
                                        mipmap: true; smooth: true
                                    }

                                    Column {
                                        width: parent.width - parent.height - parent.spacing
                                        spacing: parent.spacing/2
                                        anchors.verticalCenter: parent.verticalCenter

                                        VclLabel {
                                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                            font.bold: true
                                            text: character.name
                                            width: parent.width
                                            elide: Text.ElideRight
                                        }

                                        VclLabel {
                                            font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                                            text: "Role: " + polishStr(character.designation, "-")
                                            width: parent.width
                                            elide: Text.ElideRight
                                            opacity: 0.75
                                        }

                                        VclLabel {
                                            font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                                            text: ["Age: " + polishStr(character.age, "-"), "Gender: " + polishStr(character.gender, "-")].join(", ")
                                            width: parent.width
                                            elide: Text.ElideRight
                                            opacity: 0.75
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: {
                                    charactersView.currentIndex = index
                                    if(mouse.button === Qt.RightButton) {
                                        characterContextMenu.character = character
                                        characterContextMenu.popup()
                                    }
                                }
                                onDoubleClicked: switchTo(character.notes)
                            }
                        }

                        property int nrVisibleRows: Math.ceil((height-60)/cellHeight)
                        property int nrRows: Math.ceil(model.objectCount/nrColumns)
                        footer: nrRows > nrVisibleRows ? addNewCharacter : null
                        header: addNewCharacter

                        Component.onCompleted: {
                            if(charactersTabBar.tabIndex === 0)
                                headerItem.assumeFocus()
                        }
                    }

                    Component {
                        id: addNewCharacter

                        Item {
                            width: charactersView.width
                            height: 60
                            enabled: charactersTabBar.tabIndex === 0

                            function assumeFocus() {
                                characterNameField.forceActiveFocus()
                            }

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 5
                                color: Scrite.app.translucent(Runtime.colors.primary.windowColor, 0.5)
                                border { width: 1; color: Runtime.colors.primary.borderColor }

                                RowLayout {
                                    spacing: 10
                                    width: parent.width-20
                                    anchors.centerIn: parent

                                    VclTextField {
                                        id: characterNameField
                                        Layout.fillWidth: true

                                        label: ""
                                        placeholderText: Scrite.document.readOnly ? "Enter character name to search." : "Enter character name to search/add."
                                        completionStrings: Scrite.document.structure.characterNames

                                        onReturnPressed: characterAddButton.click()
                                    }

                                    FlatToolButton {
                                        id: characterAddButton
                                        iconSource: "qrc:/icons/content/person_add.png"
                                        ToolTip.text: "Add Character"
                                        onClicked: {
                                            var chName = characterNameField.text
                                            var ch = Scrite.document.structure.findCharacter(chName)
                                            if(ch)
                                                switchTo(ch.notes)
                                            else if(!Scrite.document.readOnly) {
                                                ch = Scrite.document.structure.addCharacter(chName)
                                                notebookModel.preferredItem = ch.notes
                                            }
                                        }
                                    }

                                    FlatToolButton {
                                        id: addExistingCharactersButton
                                        iconSource: "qrc:/icons/content/persons_add.png"
                                        ToolTip.text: "Add Existing Characters"
                                        onClicked: AddCharactersDialog.launch()
                                    }
                                }
                            }
                        }
                    }

                    VclScrollBar {
                        id: charactersListViewScrollBar
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        orientation: Qt.Vertical
                        flickable: charactersView
                    }
                }

                Loader {
                    width: charactersTabContentArea.width
                    height: charactersTabContentArea.height
                    visible: charactersTabBar.tabIndex === 1
                    active: Runtime.appFeatures.characterRelationshipGraph.enabled
                    sourceComponent: CharacterRelationshipsGraphView {
                        id: crGraphView
                        structure: null
                        showBusyIndicator: true
                        onCharacterDoubleClicked: {
                            var ch = Scrite.document.structure.findCharacter(characterName)
                            if(ch)
                                switchTo(ch.notes)
                        }
                        function prepare() {
                            if(visible) {
                                structure = Scrite.document.structure
                                showBusyIndicator = false
                            }
                        }
                        Component.onCompleted: Utils.execLater(charactersTabContentArea, 100, prepare)
                        onVisibleChanged: Utils.execLater(charactersTabContentArea, 100, prepare)

                        property bool pdfExportPossible: !graphIsEmpty && visible
                        onPdfExportPossibleChanged: Announcement.shout("4D37E093-1F58-4978-8060-CD6B9AD4E03C", pdfExportPossible ? 1 : -1)
                        Component.onDestruction: if(pdfExportPossible) Announcement.shout("4D37E093-1F58-4978-8060-CD6B9AD4E03C", -1)

                        Announcement.onIncoming: (type,data) => {
                            const stype = "" + type
                            const sdata = "" + data
                            if(stype === "3F96A262-A083-478C-876E-E3AFC26A0507") {
                                if(sdata === "refresh") {
                                    crGraphView.resetGraph()
                                    refreshButton.refreshAck = true
                                } else if(sdata == "pdfexport")
                                    crGraphView.exportToPdf(pdfExportButton)
                            }
                        }
                    }

                    DisabledFeatureNotice {
                        color: Qt.rgba(0,0,0,0)
                        anchors.fill: parent
                        visible: !Runtime.appFeatures.characterRelationshipGraph.enabled
                        featureName: "Relationship Map"
                    }
                }
            }
        }
    }

    property int characterNotesTabIndex: 0
    Component {
        id: characterNotesComponent

        Rectangle {
            id: characterNotes
            property var componentData
            property Character character: componentData ? componentData.notebookItemObject.character : null
            color: Qt.tint(character.color, "#e7ffffff")

            signal characterDoubleClicked(string characterName)

            // Report support
            property bool hasReport: true
            property string reportDescription: "Export character summary & notes into a PDF or ODT."
            function createReportGenerator() {
                var generator = Scrite.document.createReportGenerator("Notebook Report")
                generator.section = character
                return generator
            }

            function deleteSelf() {
                notebookModel.preferredItem = "Characters"
                Scrite.document.structure.removeCharacter(character)
            }

            TextTabBar {
                id: characterTabBar
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 8
                tabIndex: characterNotesTabIndex
                onTabIndexChanged: characterNotesTabIndex = tabIndex
                name: character.name
                tabs: ["Information", "Relationships", "Notes"]
            }

            Item {
                id: characterTabContentArea
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: characterTabBar.bottom
                anchors.bottom: parent.bottom
                anchors.topMargin: 10

                Item {
                    width: characterTabContentArea.width
                    height: characterTabContentArea.height
                    visible: characterTabBar.tabIndex === 0

                    EventFilter.events: [31]
                    EventFilter.onFilter: {
                        EventFilter.forwardEventTo(characterQuickInfoView)
                        result.filter = true
                        result.accepted = true
                    }

                    Flickable {
                        id: quickInfoFlickable
                        clip: true
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: attachmentsView.top
                        contentWidth: quickInfoFlickableContent.width
                        contentHeight: quickInfoFlickableContent.height
                        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                        property bool scrollBarVisible: contentWidth > width

                        Row {
                            id: quickInfoFlickableContent
                            width: Math.max(550, quickInfoFlickable.width)
                            height: quickInfoFlickable.scrollBarVisible ? quickInfoFlickable.height-17 : quickInfoFlickable.height

                            Rectangle {
                                id: characterQuickInfoArea
                                width: Runtime.workspaceSettings.showNotebookInStructure ? 300 : Math.max(300, Scrite.window.width*0.3)
                                height: parent.height
                                color: Scrite.app.translucent(Runtime.colors.primary.c100.background, 0.5)

                                Connections {
                                    target: characterNotes
                                    function onCharacterChanged() {
                                        Utils.execLater(this, 100, function() {
                                            photoSlides.currentIndex = character.hasKeyPhoto ? character.keyPhotoIndex : 0
                                        } )
                                    }
                                }
                                Component.onCompleted: Utils.execLater(this, 100, function() {
                                    photoSlides.currentIndex = character.hasKeyPhoto ? character.keyPhotoIndex : 0
                                } )

                                VclFileDialog {
                                    id: fileDialog
                                    nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                                    selectFolder: false
                                    selectMultiple: false
                                    sidebarVisible: true
                                    selectExisting: true
                                    folder: Runtime.workspaceSettings.lastOpenPhotosFolderUrl
                                     // The default Ctrl+U interfers with underline
                                    onFolderChanged: Runtime.workspaceSettings.lastOpenPhotosFolderUrl = folder

                                    onAccepted: {
                                        if(fileUrl != "") {
                                            character.addPhoto(Scrite.app.urlToLocalFile(fileUrl))
                                            photoSlides.currentIndex = character.photos.length - 1
                                        }
                                    }
                                }

                                TabSequenceManager {
                                    id: characterInfoTabSequence
                                    wrapAround: true
                                }

                                Flickable {
                                    id: characterQuickInfoView
                                    width: parent.width-20
                                    height: Math.min(parent.height, contentHeight)
                                    anchors.top: parent.top
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    contentWidth: characterQuickInfoViewContent.width
                                    contentHeight: characterQuickInfoViewContent.height
                                    clip: true
                                    ScrollBar.vertical: characterQuickInfoViewScrollBar
                                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                                    function scrollIntoView(field) {
                                        const fy = field.mapToItem(characterQuickInfoViewContent, 0, 0).y
                                        const fh = field.height
                                        if(fy < contentY)
                                            contentY = fy
                                        else if(fy+fh > contentY+height)
                                            contentY = fy+fh-height
                                    }

                                    Column {
                                        id: characterQuickInfoViewContent
                                        width: characterQuickInfoView.width
                                        spacing: 10

                                        Rectangle {
                                            property bool fillWidth: parent.width < 320
                                            width: parent.width-(fillWidth ? 0 : 90)
                                            height: width
                                            color: photoSlides.currentIndex === photoSlides.count-1 ? Qt.rgba(0,0,0,0.25) : Qt.rgba(0,0,0,0.75)
                                            border.width: 1
                                            border.color: Runtime.colors.primary.borderColor
                                            anchors.horizontalCenter: parent.horizontalCenter

                                            SwipeView {
                                                id: photoSlides
                                                anchors.fill: parent
                                                anchors.margins: 2
                                                currentIndex: 0
                                                clip: true

                                                Repeater {
                                                    model: character.photos

                                                    Image {
                                                        width: photoSlides.width
                                                        height: photoSlides.height
                                                        fillMode: Image.PreserveAspectFit
                                                        source: "file:///" + modelData
                                                    }
                                                }

                                                Item {
                                                    width: photoSlides.width
                                                    height: photoSlides.height

                                                    VclButton {
                                                        anchors.centerIn: parent
                                                        text: "Add Photo"
                                                        onClicked: fileDialog.open()
                                                        enabled: !Scrite.document.readOnly && photoSlides.count <= 6
                                                    }
                                                }
                                            }

                                            FlatToolButton {
                                                anchors.verticalCenter: photoSlides.verticalCenter
                                                anchors.left: parent.left
                                                anchors.leftMargin: parent.fillWidth ? 0 : -width
                                                iconSource: parent.fillWidth ? "qrc:/icons/navigation/arrow_left_inverted.png" : "qrc:/icons/navigation/arrow_left.png"
                                                enabled: photoSlides.currentIndex > 0
                                                onClicked: photoSlides.currentIndex = Math.max(photoSlides.currentIndex-1, 0)
                                            }

                                            FlatToolButton {
                                                anchors.verticalCenter: photoSlides.verticalCenter
                                                anchors.right: parent.right
                                                anchors.rightMargin: parent.fillWidth ? 0 : -width
                                                iconSource: parent.fillWidth ? "qrc:/icons/navigation/arrow_right_inverted.png" : "qrc:/icons/navigation/arrow_right.png"
                                                enabled: photoSlides.currentIndex < photoSlides.count-1
                                                onClicked: photoSlides.currentIndex = Math.min(photoSlides.currentIndex+1, photoSlides.count-1)
                                            }

                                            FlatToolButton {
                                                anchors.top: parent.top
                                                anchors.right: parent.right
                                                anchors.rightMargin: parent.fillWidth ? 0 : -width
                                                iconSource: parent.fillWidth ? "qrc:/icons/action/delete_inverted.png" : "qrc:/icons/action/delete.png"
                                                visible: photoSlides.currentIndex < photoSlides.count-1
                                                onClicked: {
                                                    var ci = photoSlides.currentIndex
                                                    character.removePhoto(photoSlides.currentIndex)
                                                    Qt.callLater( function() { photoSlides.currentIndex = Math.min(ci,photoSlides.count-1) } )
                                                }
                                            }

                                            FlatToolButton {
                                                anchors.top: parent.top
                                                anchors.left: parent.left
                                                anchors.leftMargin: parent.fillWidth ? 0 : -width
                                                iconSource: parent.fillWidth ? "qrc:/icons/action/pin_inverted.png" : "qrc:/icons/action/pin.png"
                                                down: photoSlides.currentIndex === character.keyPhotoIndex
                                                onClicked: {
                                                    if(photoSlides.currentIndex === character.keyPhotoIndex)
                                                        character.keyPhotoIndex = 0
                                                    else
                                                        character.keyPhotoIndex = photoSlides.currentIndex
                                                }
                                            }
                                        }

                                        PageIndicator {
                                            count: photoSlides.count
                                            currentIndex: photoSlides.currentIndex
                                            onCurrentIndexChanged: photoSlides.currentIndex = currentIndex
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            interactive: true
                                        }

                                        VclTextField {
                                            id: designationField
                                            label: "Role / Designation:"
                                            width: parent.width
                                            labelAlwaysVisible: true
                                            placeholderText: "Hero/Heroine/Villian/Other <max 50 letters>"
                                            maximumLength: 50
                                            text: character.designation
                                            TabSequenceItem.sequence: 0
                                            TabSequenceItem.manager: characterInfoTabSequence
                                            onTextEdited: character.designation = text
                                            enableTransliteration: true
                                            readOnly: Scrite.document.readOnly
                                            onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(designationField)
                                        }

                                        Column {
                                            width: parent.width
                                            spacing: parent.spacing/2

                                            VclTextField {
                                                id: newTagField
                                                label: "Tags:"
                                                width: parent.width
                                                labelAlwaysVisible: true
                                                placeholderText: Scrite.app.isMacOSPlatform ? "<type & hit Return, max 25 chars>" : "<type and hit Enter, max 25 chars>"
                                                maximumLength: 25
                                                TabSequenceItem.sequence: 1
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onEditingComplete: {
                                                    character.addTag(text)
                                                    clear()
                                                }
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(newTagField)
                                            }

                                            Flow {
                                                id: tagsFlow
                                                visible: character.tags.length > 0
                                                width: parent.width

                                                Repeater {
                                                    model: character.tags

                                                    TagText {
                                                        property var colors: containsMouse ? Runtime.colors.accent.c900 : Runtime.colors.accent.c500
                                                        border.color: colors.text
                                                        border.width: 1
                                                        color: colors.background
                                                        textColor: colors.text
                                                        text: modelData
                                                        topPadding: 4; bottomPadding: 4
                                                        leftPadding: 12; rightPadding: 8
                                                        closable: Scrite.document.readOnly ? false : true
                                                        onCloseRequest: {
                                                            if(!Scrite.document.readOnly)
                                                                character.removeTag(text)
                                                        }
                                                    }
                                                }
                                            }

                                            Item {
                                                width: parent.width
                                                height: 1
                                            }
                                        }

                                        Column {
                                            width: parent.width

                                            VclLabel {
                                                function priority(val) {
                                                    var ret = ""
                                                    if(val >= -2 && val <= 2)
                                                        ret = "Normal"
                                                    else if(val >= -6 && val <= -3)
                                                        ret = "Low"
                                                    else if(val <=-7)
                                                        ret = "Very Low"
                                                    else if(val>=3 && val <=6)
                                                        ret = "High"
                                                    else if(val >= 7)
                                                        ret = "Very High"

                                                    return ret += " (" + val + ")"
                                                }

                                                text: "Priority: " + priority(character.priority) + ""
                                                width: parent.width
                                                elide: Text.ElideMiddle
                                                font.pointSize: 2*Runtime.idealFontMetrics.font.pointSize/3
                                            }

                                            Slider {
                                                id: prioritySlider
                                                width: parent.width-10
                                                orientation: Qt.Horizontal
                                                from: -10
                                                to: 10
                                                padding: 0
                                                stepSize: 1
                                                value: character.priority
                                                onValueChanged: character.priority = value
                                                TabSequenceItem.sequence: 2
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(prioritySlider)
                                            }
                                        }

                                        VclTextField {
                                            id: aliasesField
                                            label: "Aliases:"
                                            width: parent.width
                                            labelAlwaysVisible: true
                                            placeholderText: "<max 50 letters>"
                                            maximumLength: 50
                                            text: character.aliases.join(", ")
                                            TabSequenceItem.sequence: 3
                                            TabSequenceItem.manager: characterInfoTabSequence
                                            onEditingComplete: character.aliases = text.split(",")
                                            enableTransliteration: true
                                            readOnly: Scrite.document.readOnly
                                            onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(aliasesField)
                                        }

                                        Row {
                                            spacing: 10
                                            width: parent.width

                                            VclTextField {
                                                id: typeField
                                                label: "Type:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "Human/Animal/Robot <max 25 letters>"
                                                maximumLength: 25
                                                text: character.type
                                                TabSequenceItem.sequence: 4
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.type = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(typeField)
                                            }

                                            VclTextField {
                                                id: genderField
                                                label: "Gender:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "<max 20 letters>"
                                                maximumLength: 20
                                                text: character.gender
                                                TabSequenceItem.sequence: 5
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.gender = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(genderField)
                                            }
                                        }

                                        Row {
                                            spacing: 10
                                            width: parent.width

                                            VclTextField {
                                                id: ageField
                                                label: "Age:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "<max 20 letters>"
                                                maximumLength: 20
                                                text: character.age
                                                TabSequenceItem.sequence: 6
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.age = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(ageField)
                                            }

                                            VclTextField {
                                                id: bodyTypeField
                                                label: "Body Type:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "<max 20 letters>"
                                                maximumLength: 20
                                                text: character.bodyType
                                                TabSequenceItem.sequence: 7
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.bodyType = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(bodyTypeField)
                                            }
                                        }

                                        Row {
                                            spacing: 10
                                            width: parent.width

                                            VclTextField {
                                                id: heightField
                                                label: "Height:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "<max 20 letters>"
                                                maximumLength: 20
                                                text: character.height
                                                TabSequenceItem.sequence: 8
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.height = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(heightField)
                                            }

                                            VclTextField {
                                                id: weightField
                                                label: "Weight:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "<max 20 letters>"
                                                maximumLength: 20
                                                text: character.weight
                                                TabSequenceItem.sequence: 9
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.weight = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                                onActiveFocusChanged: if(activeFocus) characterQuickInfoView.scrollIntoView(weightField)
                                            }
                                        }
                                    }
                                }

                                VclScrollBar {
                                    id: characterQuickInfoViewScrollBar
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    orientation: Qt.Vertical
                                    flickable: characterQuickInfoView
                                }

                                AttachmentsDropArea {
                                    anchors.fill: parent
                                    attachmentNoticeSuffix: "Drop here to capture as character pic(s)."
                                    allowedType: Attachments.PhotosOnly
                                    allowMultiple: true
                                    onDropped: {
                                        const dus = dropUrls
                                        dus.forEach( (url) => { character.addPhoto(Scrite.app.urlToLocalFile(url)) } )
                                        photoSlides.currentIndex = character.photos.length - 1
                                    }
                                }
                            }

                            Item {
                                width: parent.width - characterQuickInfoArea.width
                                height: parent.height

                                AttachmentsDropArea {
                                    id: characterAttachments
                                    anchors.fill: parent
                                    allowMultiple: true
                                    target: character ? character.attachments : null
                                }

                                LodLoader {
                                    id: summaryLoader

                                    property Character character: characterNotes.character

                                    width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                                    height: parent.height
                                    anchors.centerIn: parent
                                    anchors.horizontalCenterOffset: -5

                                    lod: Runtime.notebookSettings.richTextNotesEnabled ? eHIGH : eLOW
                                    sanctioned: character
                                    resetWidthBeforeLodChange: false
                                    resetHeightBeforeLodChange: false

                                    lowDetailComponent: FlickableTextArea {
                                        DeltaDocument {
                                            id: summaryContent
                                            content: summaryLoader.character.summary
                                        }

                                        text: summaryContent.plainText
                                        placeholderText: "Character Summary"
                                        tabSequenceIndex: 10
                                        tabSequenceManager: characterInfoTabSequence
                                        background: Rectangle {
                                            color: Runtime.colors.primary.windowColor
                                            opacity: 0.15
                                        }

                                        onTextChanged: if(textArea.activeFocus) summaryLoader.character.summary = text
                                    }

                                    highDetailComponent: RichTextEdit {
                                        text: summaryLoader.character.summary
                                        placeholderText: "Character Summary"
                                        tabSequenceIndex: 10
                                        tabSequenceManager: characterInfoTabSequence
                                        background: Rectangle {
                                            color: Runtime.colors.primary.windowColor
                                            opacity: 0.15
                                        }
                                        adjustTextWidthBasedOnScrollBar: false
                                        // ScrollBar.vertical: characterSummaryVScrollBar

                                        onTextChanged: summaryLoader.character.summary = text
                                    }
                                }
                            }
                        }

                        ScrollBar.horizontal: VclScrollBar { flickable: quickInfoFlickable }
                    }

                    AttachmentsView {
                        id: attachmentsView
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        attachments: character ? character.attachments : null

                        AttachmentsDropArea {
                            anchors.fill: parent
                            noticeWidthFactor: 0.8
                            allowMultiple: true
                            target: character ? character.attachments : null
                        }
                    }
                }

                Loader {
                    width: characterTabContentArea.width
                    height: characterTabContentArea.height
                    visible: characterTabBar.tabIndex === 1
                    active: Runtime.appFeatures.characterRelationshipGraph.enabled
                    sourceComponent: CharacterRelationshipsGraphView {
                        id: crGraphView
                        character: null
                        structure: null
                        showBusyIndicator: true
                        editRelationshipsEnabled: !Scrite.document.readOnly
                        onCharacterDoubleClicked:  {
                            if(characterNotes.character.name === characterName) {
                                AddRelationshipDialog.launch(character)
                                return
                            }

                            var ch = Scrite.document.structure.findCharacter(characterName)
                            if(ch)
                                switchTo(ch.notes)
                        }
                        onAddNewRelationshipRequest: AddRelationshipDialog.launch(character)
                        onRemoveRelationshipWithRequest: {
                            var relationship = character.findRelationship(otherCharacter)
                            character.removeRelationship(relationship)
                        }

                        function prepare() {
                            if(visible) {
                                character = characterNotes.character
                                structure = Scrite.document.structure
                                showBusyIndicator = false
                            }
                        }
                        Component.onCompleted: Utils.execLater(characterTabContentArea, 100, prepare)
                        onVisibleChanged: Utils.execLater(characterTabContentArea, 100, prepare)
                    }

                    DisabledFeatureNotice {
                        color: Qt.rgba(0,0,0,0)
                        anchors.fill: parent
                        visible: !Runtime.appFeatures.characterRelationshipGraph.enabled
                        featureName: "Relationship Map"
                    }
                }

                Loader {
                    width: characterTabContentArea.width
                    height: characterTabContentArea.height
                    sourceComponent: notesComponent
                    active: characterNotes.character
                    onLoaded: item.componentData = characterNotes.componentData
                    visible: characterTabBar.tabIndex === 2
                }
            }
        }
    }

    VclMenu {
        id: newNoteMenu
        property Notes notes
        enabled: notes
        onAboutToHide: notes = null

        ColorMenu {
            title: "Text Note"
            onMenuItemClicked: (color) => {
                                   var note = newNoteMenu.notes.addTextNote()
                                   if(note) {
                                       note.color = color
                                       note.objectName = "_newNote"
                                       Utils.execLater(note, 10, function() {
                                           switchTo(note);
                                       })
                                   }
                                   newNoteMenu.close()
                               }
        }

        FormMenu {
            title: "Form Note"
            notes: newNoteMenu.notes
            onNoteAdded: (note) => {
                             Utils.execLater(note, 10, function() {
                                 switchTo(note);
                             })
                             newNoteMenu.close()
                         }
        }

        ColorMenu {
            title: "Checklist Note"
            onMenuItemClicked: (color) => {
                                   var note = newNoteMenu.notes.addCheckListNote()
                                   if(note) {
                                       note.color = color
                                       note.objectName = "_newNote"
                                       Utils.execLater(note, 10, function() {
                                            switchTo(note)
                                       })
                                   }
                                   newNoteMenu.close()
                               }
        }
    }

    VclMenu {
        id: noteContextMenu
        property Note note
        enabled: note

        onAboutToHide: note = null

        ColorMenu {
            title: "Note Color"
            onMenuItemClicked: {
                noteContextMenu.note.color = color
                noteContextMenu.close()
            }
        }

        MenuSeparator { }

        VclMenuItem {
            text: "Delete Note"
            onClicked: {
                if(notebookTree.currentNote == noteContextMenu.note)
                    notebookContentLoader.confirmAndDelete()
                else {
                    notebookView.switchTo(noteContextMenu.note)
                    Utils.execLater( notebookContentLoader, 500, () => {
                                     notebookContentLoader.confirmAndDelete()
                                 } )
                }
                noteContextMenu.close()
            }
        }
    }

    VclMenu {
        id: characterContextMenu

        property Character character
        property Item characterItem

        width: 250
        enabled: character

        onAboutToHide: character = null

        ColorMenu {
            title: "Character Color"
            onMenuItemClicked: {
                characterContextMenu.character.color = color
                characterContextMenu.close()
            }
        }

        VclMenuItem {
            text: "Rename/Merge Character"
            onClicked: RenameCharacterDialog.launch(characterContextMenu.character)
        }

        VclMenu {
            title: "Reports"

            width: 250

            Repeater {
                model: Runtime.characterListReports

                VclMenuItem {
                    required property var modelData

                    text: modelData.name
                    icon.source: "qrc" + modelData.icon

                    onTriggered: ReportConfigurationDialog.launch(modelData.name,
                                                                  {"characterNames": [characterContextMenu.character.name]},
                                                                  {"initialPage": modelData.group})
                }
            }
        }

        MenuSeparator { }

        VclMenuItem {
            text: "Delete Character"
            onClicked: {
                if(notebookTree.currentCharacter == characterContextMenu.character)
                    notebookContentLoader.confirmAndDelete()
                else {
                    notebookView.switchTo(characterContextMenu.character.notes)
                    Utils.execLater( notebookContentLoader, 100, () => {
                                     notebookContentLoader.confirmAndDelete()
                                 } )
                }
                characterContextMenu.close()
            }
        }
    }

    FocusTracker.window: Scrite.window
    FocusTracker.onHasFocusChanged: Runtime.undoStack.notebookActive = FocusTracker.hasFocus

    Loader {
        id: structureIconAnimator
        active: Runtime.workspaceSettings.animateStructureIcon && Runtime.showNotebookInStructure
        anchors.fill: parent
        sourceComponent: UiElementHighlight {
            uiElement: structureTabButton
            onDone: Runtime.workspaceSettings.animateStructureIcon = false
            description: structureTabButton.ToolTip.text
            property bool scaleDone: false
            onScaleAnimationDone: scaleDone = true
            Component.onDestruction: {
                if(scaleDone)
                    Runtime.workspaceSettings.animateStructureIcon = false
            }
        }
    }

    Component {
        id: storyStatsReport

        PdfView {
            id: storyStatsView
            closable: false
            pagesPerRow: 1
            displayRefreshButton: true

            FileManager {
                id: fileManager
            }

            BusyOverlay {
                id: busyMessage
                anchors.fill: parent
                busyMessage: "Loading Stats ..."
                visible: true
            }

            onRefreshRequest: {
                busyMessage.visible = true
                Qt.callLater(generateStatsReport)
            }

            function generateStatsReport() {
                const fileName = fileManager.generateUniqueTemporaryFileName("pdf")
                var generator = Scrite.document.createReportGenerator("Statistics Report")
                generator.fileName = fileName
                generator.generate()
                fileManager.addToAutoDeleteList(fileName)
                pagesPerRow = 1
                source = Scrite.app.localFileToUrl(fileName)
                busyMessage.visible = false
            }

            Component.onCompleted: Qt.callLater(generateStatsReport)
        }
    }

    HelpTipNotification {
        tipName: "notebook"
    }
}
