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

import QtQml 2.15
import QtQuick 2.15
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls 1.4 as OldControls

import io.scrite.components 1.0

Rectangle {
    id: notebookView

    property real toolbarSize: 46
    property real toolbarSpacing: 0
    property real toolbarLeftMargin: 0
    property real toolButtonSize: Math.max(toolbarSize - 4, 20)
    property real maxTextAreaSize: idealAppFontMetrics.averageCharacterWidth * 80
    property real minTextAreaSize: idealAppFontMetrics.averageCharacterWidth * 20

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
        target: screenplayAdapter.isSourceScreenplay ? Scrite.document.screenplay : null
        function onElementInserted(element, index) {
            notebookModel.preferredItem = element.elementType === ScreenplayElement.BreakElementType ? element : element.scene.notes
        }
        function onElementMoved(element, from, to) {
            notebookModel.preferredItem = element.elementType === ScreenplayElement.BreakElementType ? element : element.scene.notes
        }
        function onCurrentElementIndexChanged(val) {
            if(workspaceSettings.syncCurrentSceneOnNotebook)
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
        font.pointSize: Math.ceil(Scrite.app.idealFontPointSize*0.75)
    }

    Rectangle {
        id: toolbar
        width: toolButtonSize+4
        height: parent.height
        color: primaryColors.c100.background

        Column {
            id: toolbarLayout
            width: toolButtonSize
            anchors.horizontalCenter: parent.horizontalCenter

            ToolButton3 {
                id: structureTabButton
                visible: ui.showNotebookInStructure
                iconSource: "../icons/navigation/structure_tab.png"
                ToolTip.text: "Structure Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                onClicked: Announcement.shout("190B821B-50FE-4E47-A4B2-BDBB2A13B72C", "Structure")
            }

            ToolButton3 {
                id: notebookTabButton
                visible: ui.showNotebookInStructure
                iconSource: "../icons/navigation/notebook_tab.png"
                down: true
                ToolTip.text: "Notebook\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
            }

            Rectangle {
                width: parent.width
                height: 1
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                checkable: true
                iconSource: "../icons/navigation/sync.png"
                ToolTip.text: "If checked; episodes, acts and scenes selected on the notebook will be made current in screenplay editor & timeline"
                checked: workspaceSettings.syncCurrentSceneOnNotebook
                onToggled: {
                    workspaceSettings.syncCurrentSceneOnNotebook = checked
                    if(checked)
                        notebookTree.activateFromCurrentScreenplayElement()
                }
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
            }

            ToolButton3 {
                id: refreshButton
                iconSource: "../icons/navigation/refresh.png"
                ToolTip.text: "Reloads the current character relationships graph and the notebook tree."
                property bool refreshAck: false
                onClicked: {
                    refreshAck = false
                    Announcement.shout("3F96A262-A083-478C-876E-E3AFC26A0507", "refresh")
                    if(refreshAck)
                        Scrite.app.execLater(refreshButton, 250, function() { notebookModel.refresh() })
                    else
                        notebookModel.refresh()
                }
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
            }

            ToolButton3 {
                id: pdfExportButton
                iconSource: "../icons/file/generate_pdf.png"
                ToolTip.text: "Export the current character relationship graph to PDF."
                onClicked: Announcement.shout("3F96A262-A083-478C-876E-E3AFC26A0507", "pdfexport")
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                property int crGraphViewCount: 0
                enabled: crGraphViewCount > 0
                Announcement.onIncoming: (type,data) => {
                    const stype = ""+type
                    const idata = 0+data
                    if(stype === "4D37E093-1F58-4978-8060-CD6B9AD4E03C")
                        crGraphViewCount += idata
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                id: newNoteToolButton
                iconSource: "../icons/action/note_add.png"
                hasMenu: true
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                property Notes notes: notebookTree.currentNotes
                enabled: notes && !Scrite.document.readOnly
                ToolTip.text: {
                    var ret = "Adds a new text or form note"
                    if(!enabled)
                        return ret
                    ret += " to" + notebookTree.currentData.notebookItemTitle
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

            ToolButton3 {
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

            ToolButton3 {
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
                        iconSource = "../icons/content/bookmark.png"
                    else
                        iconSource = "../icons/content/bookmark_outline.png"
                }

                shortcut: "Ctrl+D"
                ShortcutsModelItem.group: "Notebook"
                ShortcutsModelItem.title: "Toggle Bookmark"
                ShortcutsModelItem.shortcut: shortcut
                ShortcutsModelItem.enabled: enabled
            }

            ToolButton3 {
                id: deleteNoteButton
                suggestedWidth: toolButtonSize
                suggestedHeight: toolButtonSize
                enabled: (noteColorButton.note || noteColorButton.character) && !Scrite.document.readOnly
                ToolTip.text: "Delete the current note or character"
                iconSource: "../icons/action/delete.png"
                onClicked: notebookContentLoader.confirmAndDelete()
            }
        }

        Rectangle {
            width: 1
            height: parent.height
            anchors.right: parent.right
            color: primaryColors.borderColor
        }
    }

    SplitView {
        orientation: Qt.Horizontal
        anchors.top: parent.top
        anchors.left: toolbar.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        Material.background: Qt.darker(primaryColors.button.background, 1.1)

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
                color: styleData.selected ? primaryColors.highlight.background : primaryColors.c10.background
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
                            return primaryColors.highlight.background

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

                        return primaryColors.c10.background
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
                                    return "../icons/content/episode.png"
                                case NotebookModel.ActBreakType:
                                    return "../icons/content/act.png"
                                case NotebookModel.NotesType:
                                    switch(styleData.value.notebookItemObject.ownerType) {
                                    case Notes.SceneOwner:
                                        return "../icons/content/scene.png"
                                    case Notes.CharacterOwner:
                                        return "../icons/content/person_outline.png"
                                    case Notes.BreakOwner:
                                        return "../icons/content/story.png"
                                    default:
                                        break
                                    }
                                    break;
                                case NotebookModel.NoteType:
                                    switch(styleData.value.notebookItemObject.type) {
                                    case Note.TextNoteType:
                                        return "../icons/content/note.png"
                                    case Note.FormNoteType:
                                        return "../icons/content/form.png"
                                    default:
                                        break
                                    }
                                    break;
                                }

                                return ""
                            }
                        }

                        Text {
                            id: itemDelegateText
                            padding: 5
                            font.family: fontMetrics.font.family
                            font.pointSize: fontMetrics.font.pointSize
                            font.capitalization: fontMetrics.font.capitalization
                            font.bold: styleData.value.notebookItemType === NotebookModel.CategoryType ||
                                       (styleData.value.notebookItemType === NotebookModel.NotesType &&
                                        styleData.value.notebookItemObject.ownerType === Notes.StructureOwner)
                            text: styleData.value.notebookItemTitle ? styleData.value.notebookItemTitle : ""
                            color: Scrite.app.isLightColor(parent.parent.color) ? "black" : "white"
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

            function activateScreenplayElement(_modelData) {
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
                if(mainTabBar.currentIndex != 1 || workspaceSettings.syncCurrentSceneOnNotebook)
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

        Item {
            Loader {
                id: notebookContentLoader
                active: notebookContentActiveProperty.value
                anchors.fill: parent

                property int currentNotebookItemId: notebookTree.currentData ? notebookTree.currentData.notebookItemId : -1

                ResetOnChange {
                    id: notebookContentActiveProperty
                    trackChangesOn: notebookContentLoader.currentNotebookItemId
                    from: false
                    to: true
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
                    color: Scrite.app.translucent(primaryColors.c600.background,0.85)
                    focus: true

                    MouseArea {
                        anchors.fill: parent
                    }

                    Column {
                        width: parent.width-20
                        anchors.centerIn: parent
                        spacing: 40

                        Text {
                            text: {
                                if(notebookTree.currentNote)
                                    return "Are you sure you want to delete this note?"
                                if(notebookTree.currentCharacter)
                                    return "Are you sure you want to delete this character?"
                                return "Cannot remove this item."
                            }
                            font.bold: true
                            font.pointSize: Scrite.app.idealFontPointSize
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            color: primaryColors.c600.text
                        }

                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 20

                            Button2 {
                                text: "Yes"
                                focusPolicy: Qt.NoFocus
                                onClicked: notebookContentLoader.item.deleteSelf()
                                visible: notebookTree.currentNote || notebookTree.currentCharacter
                            }

                            Button2 {
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
            color: Scrite.app.translucent(primaryColors.c100.background, 0.5)
            border.width: 1
            border.color: primaryColors.borderColor
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
                        border.color: bookmarksView.currentIndex === index ? "darkgray" : primaryColors.borderColor

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
                                                return "../icons/content/scene.png"
                                            case Notes.CharacterOwner:
                                                return "../icons/content/person_outline.png"
                                            case Notes.BreakOwner:
                                                return "../icons/content/story.png"
                                            default:
                                                break
                                            }
                                        } else if(Scrite.app.typeName(noteObject) === "Character")
                                            return "../icons/content/person_outline.png"
                                        else if(Scrite.app.typeName(noteObject) === "Note") {
                                            switch(styleData.value.notebookItemObject.type) {
                                            case Note.TextNoteType:
                                                return "../icons/content/note.png"
                                            case Note.FormNoteType:
                                                return "../icons/content/form.png"
                                            default:
                                                break
                                            }
                                        }
                                        return "../icons/content/bookmark.png"
                                    }
                                }

                                Text {
                                    id: headingText
                                    font.pointSize: Scrite.app.idealFontPointSize
                                    font.bold: true
                                    maximumLineCount: 1
                                    width: parent.width-32-parent.spacing
                                    elide: Text.ElideRight
                                    text: noteTitle
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Text {
                                width: parent.width
                                height: parent.height - headingText.height - parent.spacing
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                                font.pointSize: Scrite.app.idealFontPointSize-2
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

            ScrollBar2 {
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

                        TextField2 {
                            id: sceneHeadingField
                            text: scene.heading.text
                            label: ""
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            wrapMode: Text.WordWrap
                            placeholderText: "Scene Heading"
                            readOnly: Scrite.document.readOnly
                            enabled: scene.heading.enabled
                            onEditingComplete: scene.heading.parseFrom(text)
                            tabItem: sceneTitleField
                            font.capitalization: Font.AllUppercase
                            font.family: Scrite.document.formatting.elementFormat(SceneElement.Heading).font.family
                            font.pointSize: Scrite.app.idealFontPointSize+2
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        TextField2 {
                            id: sceneTitleField
                            text: scene.structureElement.nativeTitle
                            label: ""
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            wrapMode: Text.WordWrap
                            placeholderText: "Scene Title"
                            readOnly: Scrite.document.readOnly
                            onEditingComplete: scene.structureElement.title = text
                            tabItem: sceneSynopsisField.textArea
                            backTabItem: sceneHeadingField
                            font.capitalization: Font.AllUppercase
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Flow {
                            id: sceneCharactersList
                            spacing: 5
                            width: sceneTitleField.width
                            anchors.horizontalCenter: parent.horizontalCenter
                            flow: Flow.LeftToRight

                            Text {
                                id: sceneCharactersListHeading
                                text: "Characters: "
                                font.bold: true
                                topPadding: 5
                                bottomPadding: 5
                                font.pointSize: Scrite.app.idealFontPointSize
                                visible: !scene.hasCharacters
                            }

                            Repeater {
                                model: scene ? scene.characterNames : 0

                                TagText {
                                    id: characterNameLabel
                                    property var colors: containsMouse ? accentColors.c900 : accentColors.c600
                                    border.width: 1
                                    border.color: colors.text
                                    color: colors.background
                                    textColor: colors.text
                                    text: modelData
                                    topPadding: 5; bottomPadding: 5
                                    leftPadding: 10; rightPadding: 10
                                    font.family: "Courier Prime"
                                    font.capitalization: Font.AllUppercase
                                    font.pointSize: Scrite.app.idealFontPointSize
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
                                sourceComponent: TextField2 {
                                    id: newCharacterNameInput
                                    readOnly: Scrite.document.readOnly
                                    font.family: "Courier Prime"
                                    font.capitalization: length > 0 ? Font.AllUppercase : Font.MixedCase
                                    font.pointSize: Scrite.app.idealFontPointSize
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
                                    tabItem: sceneSynopsisField.textArea
                                    backTabItem: sceneTitleField
                                    Component.onCompleted: forceActiveFocus()
                                }
                            }

                            Image {
                                source: "../icons/content/add_box.png"
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

                        FlickableTextArea {
                            id: sceneSynopsisField
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            height: parent.height - sceneHeadingField.height - sceneTitleField.height - parent.spacing*2
                            text: scene.title
                            placeholderText: "Scene Synopsis"
                            readOnly: Scrite.document.readOnly
                            onTextChanged: scene.title = text
                            undoRedoEnabled: true
                            backTabItem: sceneTitleField
                            adjustTextWidthBasedOnScrollBar: false
                            ScrollBar.vertical: sceneSynopsisVScrollBar
                            anchors.horizontalCenter: parent.horizontalCenter
                            background: Rectangle {
                                color: primaryColors.windowColor
                                opacity: 0.15
                            }
                        }
                    }

                    ScrollBar2 {
                        id: sceneSynopsisVScrollBar
                        orientation: Qt.Vertical
                        flickable: sceneSynopsisField
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: sceneAttachments.top
                    }

                    AttachmentsView {
                        id: sceneAttachments
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        attachments: scene ? scene.attachments : null
                    }

                    AttachmentsDropArea2 {
                        anchors.fill: parent
                        target: scene ? scene.attachments : null
                    }
                }

                Loader {
                    width: sceneTabContentArea.width
                    height: sceneTabContentArea.height
                    visible: sceneTabBar.tabIndex === 1
                    active: crgraphAppFeature.enabled
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
                        Component.onCompleted: Scrite.app.execLater(sceneTabContentArea, 100, prepare)
                        onVisibleChanged: Scrite.app.execLater(sceneTabContentArea, 100, prepare)

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
                        visible: !crgraphAppFeature.enabled
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
                                color: primaryColors.windowColor
                                opacity: 0.15
                            }
                        }
                    }

                    ScrollBar2 {
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
            property real minimumNoteSize: Math.max(200, ui.width*0.15)
            property real noteSize: notesFlick.width > minimumNoteSize ? notesFlick.width / Math.floor(notesFlick.width/minimumNoteSize) : notesFlick.width
            clip: true
            color: Scrite.app.translucent(primaryColors.c100.background, 0.5)
            border.width: 1
            border.color: primaryColors.borderColor

            Flickable {
                id: notesFlick
                anchors.fill: parent
                anchors.margins: 20
                property int currentIndex: 0
                contentWidth: width
                contentHeight: noteItemsFlow.height
                FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

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
                                    enabled: applicationSettings.enableAnimations
                                    ColorAnimation { duration: 250 }
                                }

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 8

                                    Text {
                                        id: headingText
                                        font.pointSize: Scrite.app.idealFontPointSize
                                        font.bold: true
                                        maximumLineCount: 1
                                        width: parent.width
                                        elide: Text.ElideRight
                                        text: objectItem.title
                                        color: Scrite.app.isLightColor(parent.parent.color) ? Qt.rgba(0.2,0.2,0.2,1.0) : Qt.rgba(0.9,0.9,0.9,1.0)
                                    }

                                    Text {
                                        width: parent.width
                                        height: parent.height - headingText.height - parent.spacing
                                        wrapMode: Text.WordWrap
                                        elide: Text.ElideRight
                                        font.pointSize: Scrite.app.idealFontPointSize-2
                                        text: objectItem.type === Note.TextNoteType ? objectItem.content : objectItem.summary
                                        color: headingText.color
                                        opacity: 0.75
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
                            color: Scrite.app.translucent(primaryColors.c100.background, 0.5)
                            border.width: 1
                            border.color: primaryColors.borderColor

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    newNoteMenu.notes = notesSummary.notes
                                    newNoteMenu.popup(newNoteButton, 0, newNoteButton.height)
                                }
                            }
                        }

                        ToolButton3 {
                            id: newNoteButton
                            anchors.centerIn: parent
                            ToolTip.text: "Add a new text or form note."
                            iconSource: "../icons/action/note_add.png"
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

            ScrollBar2 {
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

                            Text {
                                id: headingLabel
                                text: breakElement.breakTitle + ": "
                                font.pointSize: Scrite.app.idealFontPointSize + 3
                                anchors.baseline: breakElementHeadingField.baseline
                            }

                            TextField2 {
                                id: breakElementHeadingField
                                text: breakElement.breakSubtitle
                                width: parent.width - headingLabel.width - parent.spacing
                                label: ""
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                font.pointSize: Scrite.app.idealFontPointSize + 5
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
                                color: primaryColors.windowColor
                                opacity: 0.15
                            }
                        }
                    }


                    ScrollBar2 {
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

            AttachmentsDropArea2 {
                id: attachmentsDropArea
                anchors.fill: parent
                target: breakElement ? breakElement.attachments : null
            }

            Text {
                width: parent.width * 0.6
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Scrite.app.idealFontPointSize
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

            FontMetrics {
                id: screenplayFontMetrics
                font.family: Scrite.document.formatting.defaultFont.family
                font.pointSize: Scrite.app.idealFontPointSize
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
                        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

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
                            }

                            Text {
                                font.family: screenplayFontMetrics.font.family
                                font.pointSize: screenplayFontMetrics.font.pointSize + 2
                                font.bold: true
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.title === "" ? "<untitled>" : Scrite.document.screenplay.title
                            }

                            Text {
                                font.family: screenplayFontMetrics.font.family
                                font.pointSize: screenplayFontMetrics.font.pointSize
                                font.bold: true
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.subtitle
                                visible: Scrite.document.screenplay.subtitle !== ""
                            }

                            Column {
                                width: parent.width
                                spacing: 0

                                Text {
                                    font: screenplayFontMetrics.font
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    horizontalAlignment: Text.AlignHCenter
                                    text: "Written By"
                                }

                                Text {
                                    font: screenplayFontMetrics.font
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    horizontalAlignment: Text.AlignHCenter
                                    text: (Scrite.document.screenplay.author === "" ? "<unknown author>" : Scrite.document.screenplay.author)
                                }
                            }

                            Text {
                                font: screenplayFontMetrics.font
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.version === "" ? "Initial Version" : Scrite.document.screenplay.version
                            }

                            Text {
                                font: screenplayFontMetrics.font
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                text: Scrite.document.screenplay.basedOn
                                visible: Scrite.document.screenplay.basedOn !== ""
                            }

                            Item { width: parent.width; height: parent.spacing/2 }

                            Column {
                                spacing: parent.spacing/2
                                width: parent.width * 0.5
                                anchors.right: parent.horizontalCenter
                                anchors.rightMargin: -width*0.25

                                Text {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.contact
                                    visible: text !== ""
                                }

                                Text {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.address
                                    visible: text !== ""
                                }

                                Text {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.phoneNumber
                                    visible: text !== ""
                                }

                                Text {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    font.underline: true
                                    color: "blue"
                                    width: parent.width
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                    text: Scrite.document.screenplay.email
                                    visible: text !== ""

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: Qt.openUrlExternally("mailto:" + parent.text)
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }

                                Text {
                                    font.family: screenplayFontMetrics.font.family
                                    font.pointSize: screenplayFontMetrics.font.pointSize-2
                                    font.underline: true
                                    color: "blue"
                                    width: parent.width
                                    elide: Text.ElideRight
                                    text: Scrite.document.screenplay.website
                                    visible: text !== ""

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: Qt.openUrlExternally(parent.text)
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }
                            }
                        }
                    }

                    ToolButton3 {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.rightMargin: titlePageFlickable.vscrollBarRequired ? 20 : 0
                        iconSource: "../icons/action/edit_title_page.png"
                        onClicked: {
                            modalDialog.arguments = {"activeTabIndex": 2}
                            modalDialog.popupSource = this
                            modalDialog.sourceComponent = optionsDialogComponent
                            modalDialog.active = true
                        }
                        enabled: !Scrite.document.readOnly
                    }

                    ScrollBar2 {
                        id: titlePageVScrollBar
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        orientation: Qt.Vertical
                        flickable: titlePageFlickable
                    }

                    ScrollBar2 {
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
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10

                        FlickableTextArea {
                            id: loglineFieldArea
                            text: Scrite.document.screenplay.logline
                            onTextChanged: Scrite.document.screenplay.logline = text
                            placeholderText: "Logline: a one-sentence summary or description."
                            width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                            height: parent.height
                            readOnly: Scrite.document.readOnly
                            undoRedoEnabled: true
                            ScrollBar.vertical: loglineVScrollBar
                            adjustTextWidthBasedOnScrollBar: false
                            anchors.horizontalCenter: parent.horizontalCenter
                            background: Rectangle {
                                color: primaryColors.windowColor
                                opacity: 0.15
                            }
                        }
                    }

                    ScrollBar2 {
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

            Text {
                anchors.fill: parent
                anchors.margins: 20
                font.pointSize: Scrite.app.idealFontPointSize
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
                                border.color: Scrite.app.isLightColor(character.color) ? (charactersView.currentIndex === index ? "darkgray" : primaryColors.borderColor) : character.color

                                Row {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 10

                                    Image {
                                        width: parent.height
                                        height: parent.height
                                        source: {
                                            if(character.photos.length > 0)
                                                return "file:///" + character.photos[0]
                                            return "../icons/content/character_icon.png"
                                        }
                                        fillMode: Image.PreserveAspectCrop
                                        mipmap: true; smooth: true
                                    }

                                    Column {
                                        width: parent.width - parent.height - parent.spacing
                                        spacing: parent.spacing/2
                                        anchors.verticalCenter: parent.verticalCenter

                                        Text {
                                            font.pointSize: Scrite.app.idealFontPointSize
                                            font.bold: true
                                            text: character.name
                                            width: parent.width
                                            elide: Text.ElideRight
                                        }

                                        Text {
                                            font.pointSize: Scrite.app.idealFontPointSize - 2
                                            text: "Role: " + polishStr(character.designation, "-")
                                            width: parent.width
                                            elide: Text.ElideRight
                                            opacity: 0.75
                                        }

                                        Text {
                                            font.pointSize: Scrite.app.idealFontPointSize - 2
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
                                color: Scrite.app.translucent(primaryColors.windowColor, 0.5)
                                border { width: 1; color: primaryColors.borderColor }

                                Row {
                                    spacing: 10
                                    width: parent.width-20
                                    anchors.centerIn: parent

                                    TextField2 {
                                        id: characterNameField
                                        completionStrings: Scrite.document.structure.characterNames
                                        width: parent.width - characterAddButton.width - parent.spacing
                                        placeholderText: Scrite.document.readOnly ? "Enter character name to search." : "Enter character name to search/add."
                                        label: ""
                                        onReturnPressed: characterAddButton.click()
                                    }

                                    ToolButton3 {
                                        id: characterAddButton
                                        iconSource: "../icons/content/person_add.png"
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
                                }
                            }
                        }
                    }

                    ScrollBar2 {
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
                    active: crgraphAppFeature.enabled
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
                        Component.onCompleted: Scrite.app.execLater(charactersTabContentArea, 100, prepare)
                        onVisibleChanged: Scrite.app.execLater(charactersTabContentArea, 100, prepare)

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
                        visible: !crgraphAppFeature.enabled
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
                        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

                        property bool scrollBarVisible: contentWidth > width

                        Row {
                            id: quickInfoFlickableContent
                            width: Math.max(550, quickInfoFlickable.width)
                            height: quickInfoFlickable.scrollBarVisible ? quickInfoFlickable.height-17 : quickInfoFlickable.height

                            Rectangle {
                                id: characterQuickInfoArea
                                width: workspaceSettings.showNotebookInStructure ? 300 : Math.max(300, ui.width*0.3)
                                height: parent.height
                                color: Scrite.app.translucent(primaryColors.c100.background, 0.5)

                                Connections {
                                    target: characterNotes
                                    function onCharacterChanged() {
                                        Scrite.app.execLater(this, 100, function() { photoSlides.currentIndex = 0 } )
                                    }
                                }
                                Component.onCompleted: Scrite.app.execLater(this, 100, function() { photoSlides.currentIndex = 0 } )

                                FileDialog {
                                    id: fileDialog
                                    nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                                    selectFolder: false
                                    selectMultiple: false
                                    sidebarVisible: true
                                    selectExisting: true
                                    folder: workspaceSettings.lastOpenPhotosFolderUrl
                                    onFolderChanged: workspaceSettings.lastOpenPhotosFolderUrl = folder

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
                                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

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
                                            border.color: primaryColors.borderColor
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

                                                    Button2 {
                                                        anchors.centerIn: parent
                                                        text: "Add Photo"
                                                        onClicked: fileDialog.open()
                                                        enabled: !Scrite.document.readOnly && photoSlides.count <= 6
                                                    }
                                                }
                                            }

                                            ToolButton3 {
                                                anchors.verticalCenter: photoSlides.verticalCenter
                                                anchors.left: parent.left
                                                anchors.leftMargin: parent.fillWidth ? 0 : -width
                                                iconSource: parent.fillWidth ? "../icons/navigation/arrow_left_inverted.png" : "../icons/navigation/arrow_left.png"
                                                enabled: photoSlides.currentIndex > 0
                                                onClicked: photoSlides.currentIndex = Math.max(photoSlides.currentIndex-1, 0)
                                            }

                                            ToolButton3 {
                                                anchors.verticalCenter: photoSlides.verticalCenter
                                                anchors.right: parent.right
                                                anchors.rightMargin: parent.fillWidth ? 0 : -width
                                                iconSource: parent.fillWidth ? "../icons/navigation/arrow_right_inverted.png" : "../icons/navigation/arrow_right.png"
                                                enabled: photoSlides.currentIndex < photoSlides.count-1
                                                onClicked: photoSlides.currentIndex = Math.min(photoSlides.currentIndex+1, photoSlides.count-1)
                                            }

                                            ToolButton3 {
                                                anchors.top: parent.top
                                                anchors.right: parent.right
                                                anchors.rightMargin: parent.fillWidth ? 0 : -width
                                                iconSource: parent.fillWidth ? "../icons/action/delete_inverted.png" : "../icons/action/delete.png"
                                                visible: photoSlides.currentIndex < photoSlides.count-1
                                                onClicked: {
                                                    var ci = photoSlides.currentIndex
                                                    character.removePhoto(photoSlides.currentIndex)
                                                    Qt.callLater( function() { photoSlides.currentIndex = Math.min(ci,photoSlides.count-1) } )
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

                                        TextField2 {
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
                                        }

                                        Column {
                                            width: parent.width
                                            spacing: parent.spacing/2

                                            TextField2 {
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
                                            }

                                            Flow {
                                                id: tagsFlow
                                                visible: character.tags.length > 0
                                                width: parent.width

                                                Repeater {
                                                    model: character.tags

                                                    TagText {
                                                        property var colors: containsMouse ? accentColors.c900 : accentColors.c500
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

                                            Text {
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
                                                font.pointSize: 2*Scrite.app.idealFontPointSize/3
                                            }

                                            Slider {
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
                                            }
                                        }

                                        TextField2 {
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
                                        }

                                        Row {
                                            spacing: 10
                                            width: parent.width

                                            TextField2 {
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
                                            }

                                            TextField2 {
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
                                            }
                                        }

                                        Row {
                                            spacing: 10
                                            width: parent.width

                                            TextField2 {
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
                                            }

                                            TextField2 {
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
                                            }
                                        }

                                        Row {
                                            spacing: 10
                                            width: parent.width

                                            TextField2 {
                                                id: heightField
                                                label: "Height:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "<max 20 letters>"
                                                maximumLength: 20
                                                text: character.height
                                                TabSequenceItem.sequence: 7
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.height = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                            }

                                            TextField2 {
                                                id: weightField
                                                label: "Weight:"
                                                width: (parent.width - parent.spacing)/2
                                                labelAlwaysVisible: true
                                                placeholderText: "<max 20 letters>"
                                                maximumLength: 20
                                                text: character.weight
                                                TabSequenceItem.sequence: 8
                                                TabSequenceItem.manager: characterInfoTabSequence
                                                onTextEdited: character.weight = text
                                                enableTransliteration: true
                                                readOnly: Scrite.document.readOnly
                                            }
                                        }
                                    }
                                }

                                ScrollBar2 {
                                    id: characterQuickInfoViewScrollBar
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    orientation: Qt.Vertical
                                    flickable: characterQuickInfoView
                                }

                                AttachmentsDropArea2 {
                                    anchors.fill: parent
                                    attachmentNoticeSuffix: "Drop this photo here to capture as character pic."
                                    allowedType: Attachments.PhotosOnly
                                    onDropped: {
                                        character.addPhoto(attachment.filePath)
                                        photoSlides.currentIndex = character.photos.length - 1
                                    }
                                }
                            }

                            Item {
                                width: parent.width - characterQuickInfoArea.width
                                height: parent.height

                                AttachmentsDropArea2 {
                                    anchors.fill: parent
                                    target: character ? character.attachments : null
                                }

                                FlickableTextArea {
                                    id: characterSummaryField
                                    width: parent.width >= maxTextAreaSize+20 ? maxTextAreaSize : parent.width-20
                                    height: parent.height
                                    anchors.centerIn: parent
                                    anchors.horizontalCenterOffset: -5
                                    text: character.summary
                                    onTextChanged: character.summary = text
                                    placeholderText: "Character Summary"
                                    tabSequenceIndex: 8
                                    tabSequenceManager: characterInfoTabSequence
                                    background: Rectangle {
                                        color: primaryColors.windowColor
                                        opacity: 0.15
                                    }
                                    adjustTextWidthBasedOnScrollBar: false
                                    ScrollBar.vertical: characterSummaryVScrollBar
                                }

                                ScrollBar2 {
                                    id: characterSummaryVScrollBar
                                    orientation: Qt.Vertical
                                    flickable: characterSummaryField
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                }
                            }
                        }

                        ScrollBar.horizontal: ScrollBar2 { flickable: quickInfoFlickable }
                    }

                    AttachmentsView {
                        id: attachmentsView
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        attachments: character ? character.attachments : null

                        AttachmentsDropArea2 {
                            anchors.fill: parent
                            noticeWidthFactor: 0.8
                            target: character ? character.attachments : null
                        }
                    }
                }

                Loader {
                    width: characterTabContentArea.width
                    height: characterTabContentArea.height
                    visible: characterTabBar.tabIndex === 1
                    active: crgraphAppFeature.enabled
                    sourceComponent: CharacterRelationshipsGraphView {
                        id: crGraphView
                        character: null
                        structure: null
                        showBusyIndicator: true
                        editRelationshipsEnabled: !Scrite.document.readOnly
                        onCharacterDoubleClicked:  {
                            if(characterNotes.character.name === characterName) {
                                doAddNewRelationship(chNodeItem)
                                return
                            }

                            var ch = Scrite.document.structure.findCharacter(characterName)
                            if(ch)
                                switchTo(ch.notes)
                        }
                        onAddNewRelationshipRequest: doAddNewRelationship(sourceItem)
                        onRemoveRelationshipWithRequest: {
                            var relationship = character.findRelationship(otherCharacter)
                            character.removeRelationship(relationship)
                        }

                        function doAddNewRelationship(psitem) {
                            modalDialog.closeable = false
                            modalDialog.popupSource = psitem
                            modalDialog.arguments = character
                            modalDialog.sourceComponent = addRelationshipDialogComponent
                            modalDialog.active = true
                        }

                        function prepare() {
                            if(visible) {
                                character = characterNotes.character
                                structure = Scrite.document.structure
                                showBusyIndicator = false
                            }
                        }
                        Component.onCompleted: Scrite.app.execLater(characterTabContentArea, 100, prepare)
                        onVisibleChanged: Scrite.app.execLater(characterTabContentArea, 100, prepare)

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
                        visible: !crgraphAppFeature.enabled
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

    Component {
        id: addRelationshipDialogComponent

        Rectangle {
            width: Math.max(maxTextAreaSize, ui.width*0.5)
            height: Math.min(ui.height*0.85, Math.min(charactersList.height, 500) + title.height + searchBar.height + addRelationshipDialogButtons.height + 80)
            color: primaryColors.c10.background

            Component.onCompleted: {
                character = modalDialog.arguments
                modalDialog.arguments = undefined
            }

            property Character character
            property var unrelatedCharacterNames: [] // Loaded after 160 ms

            Item {
                anchors.fill: parent
                anchors.margins: 20

                Text {
                    id: title
                    width: parent.width
                    anchors.top: parent.top
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    text: "Name applicable relationships between <strong>" + character.name + "</strong> and others in the screenplay."
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    anchors.fill: charactersListScroll
                    anchors.margins: -1
                    border.width: 1
                    border.color: primaryColors.borderColor
                    visible: charactersListScroll.height >= 600
                }

                SearchBar {
                    id: searchBar
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: title.bottom
                    anchors.topMargin: 10
                    searchEngine.objectName: "Characters Search Engine"
                    visible: charactersList.height > charactersListScroll.height
                }

                TabSequenceManager {
                    id: characterListTabManager
                }

                ScrollArea {
                    id: charactersListScroll
                    anchors.top: searchBar.bottom
                    anchors.bottom: addRelationshipDialogButtons.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10
                    contentWidth: charactersList.width
                    contentHeight: charactersList.height
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ScrollBar.vertical.policy: charactersList.height > height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                    ScrollBar.vertical.opacity: active ? 1 : 0.2

                    Column {
                        id: charactersList
                        width: charactersListScroll.width-20

                        Repeater {
                            id: otherCharacterItems
                            model: unrelatedCharacterNames

                            Rectangle {
                                id: characterRowItem
                                property string thisCharacterName: Scrite.app.camelCased(character.name)
                                property string otherCharacterName: modelData
                                property bool checked: relationshipName.length > 0
                                property string relationship: relationshipName.text
                                width: charactersList.width
                                height: characterRow.height*1.15

                                Row {
                                    id: characterRow
                                    width: parent.width - 20
                                    anchors.right: parent.right
                                    spacing: 10

                                    Image {
                                        width: 24; height: 24
                                        source: "../icons/navigation/check.png"
                                        anchors.verticalCenter: parent.verticalCenter
                                        opacity: relationshipName.length > 0 ? 1 : 0.05
                                    }

                                    Text {
                                        id: characterRowLabel1
                                        font.pointSize: Scrite.app.idealFontPointSize
                                        text: thisCharacterName + ": "
                                        color: foregroundColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        width: Math.min(implicitWidth, (parent.width-100)/3)
                                        horizontalAlignment: Text.AlignRight
                                    }

                                    TextField2 {
                                        id: relationshipName
                                        enableTransliteration: true
                                        width: parent.width - 32 - characterRowLabel1.width - characterRowLabel2.width - 3*parent.spacing
                                        label: ""
                                        color: foregroundColor
                                        font.pointSize: Scrite.app.idealFontPointSize
                                        placeholderText: "husband of, wife of, friends with, reports to ..."
                                        Material.background: backgroundColor
                                        Material.foreground: foregroundColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        TabSequenceItem.manager: characterListTabManager
                                        TabSequenceItem.sequence: index
                                        maximumLength: 50
                                    }

                                    Text {
                                        id: characterRowLabel2
                                        font.pointSize: Scrite.app.idealFontPointSize
                                        text: Scrite.app.camelCased(otherCharacterName) + "."
                                        color: foregroundColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        width: Math.min(implicitWidth, (parent.width-100)/3)
                                        rightPadding: 10
                                    }
                                }

                                color: backgroundColor
                                property bool highlight: false
                                property color backgroundColor: highlight ? accentColors.c100.background : primaryColors.c10.background
                                property color foregroundColor: highlight ? accentColors.c100.text : primaryColors.c10.text

                                SearchAgent.engine: searchBar.searchEngine
                                SearchAgent.onSearchRequest: {
                                    SearchAgent.searchResultCount = SearchAgent.indexesOf(string, otherCharacterName).length > 0 ? 1 : 0
                                }
                                SearchAgent.onCurrentSearchResultIndexChanged: {
                                    highlight = SearchAgent.currentSearchResultIndex >= 0
                                    charactersListScroll.ensureItemVisible(characterRowItem,1,10)
                                }
                            }
                        }
                    }
                }

                Row {
                    id: addRelationshipDialogButtons
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    spacing: 10

                    Button2 {
                        text: "Cancel"
                        onClicked: modalDialog.close()
                    }

                    Button2 {
                        text: "Create Relationships"
                        onClicked: {
                            for(var i=0; i<otherCharacterItems.count; i++) {
                                var item = otherCharacterItems.itemAt(i)
                                if(item.checked) {
                                    var otherCharacter = Scrite.document.structure.addCharacter(item.otherCharacterName)
                                    if(otherCharacter) {
                                        character.addRelationship(item.relationship, otherCharacter)
                                        character.characterRelationshipGraph = {}
                                    }
                                }
                            }
                            modalDialog.close()
                        }
                    }
                }
            }

            BusyOverlay {
                anchors.fill: parent
                busyMessage: "Querying Characters ..."
                visible: unrelatedCharacterNamesTimer.running
            }

            Behavior on height {
                enabled: applicationSettings.enableAnimations
                NumberAnimation { duration: 250 }
            }

            Timer {
                id: unrelatedCharacterNamesTimer
                running: true
                interval: 160
                repeat: false
                onTriggered: unrelatedCharacterNames = character.unrelatedCharacterNames()
            }
        }
    }

    Menu2 {
        id: newNoteMenu
        property Notes notes
        enabled: notes
        onAboutToHide: notes = null

        ColorMenu {
            title: "Text Note"
            onMenuItemClicked: {
                var note = newNoteMenu.notes.addTextNote()
                if(note) {
                    note.color = color
                    note.objectName = "_newNote"
                    Scrite.app.execLater(note, 10, function() {
                        switchTo(note);
                    })
                }
                newNoteMenu.close()
            }
        }

        FormMenu {
            title: "Form Note"
            notes: newNoteMenu.notes
            onNoteAdded: {
                Scrite.app.execLater(note, 10, function() {
                    switchTo(note);
                })
                newNoteMenu.close()
            }
        }
    }

    Menu2 {
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

        MenuItem2 {
            text: "Delete Note"
            onClicked: notebookContentLoader.confirmAndDelete()
        }
    }

    Menu2 {
        id: characterContextMenu
        property Character character
        property Item characterItem
        enabled: character

        onAboutToHide: character = null

        ColorMenu {
            title: "Character Color"
            onMenuItemClicked: {
                characterContextMenu.character.color = color
                characterContextMenu.close()
            }
        }

        MenuItem2 {
            text: "Rename Character"
            onClicked: {
                modalDialog.arguments = characterContextMenu.character
                modalDialog.popupSource = characterContextMenu.characterItem
                modalDialog.sourceComponent = renameCharacterDialog
                modalDialog.active = true
                characterContextMenu.close()
            }
        }

        MenuSeparator { }

        MenuItem2 {
            text: "Delete Character"
            onClicked: notebookContentLoader.confirmAndDelete()
        }
    }

    FocusTracker.window: Scrite.window
    FocusTracker.onHasFocusChanged: mainUndoStack.notebookActive = FocusTracker.hasFocus

    Loader {
        id: structureIconAnimator
        active: workspaceSettings.animateStructureIcon && !modalDialog.active && ui.showNotebookInStructure
        anchors.fill: parent
        sourceComponent: UiElementHighlight {
            uiElement: structureTabButton
            onDone: workspaceSettings.animateStructureIcon = false
            description: structureTabButton.ToolTip.text
            property bool scaleDone: false
            onScaleAnimationDone: scaleDone = true
            Component.onDestruction: {
                if(scaleDone)
                    workspaceSettings.animateStructureIcon = false
            }
        }
    }

    Component {
        id: renameCharacterDialog

        RenameCharacterDialog { }
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
}
