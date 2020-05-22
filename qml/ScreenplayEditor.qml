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

import QtQuick 2.13
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13

import Scrite 1.0

Item {
    id: screenplayEditor
    property bool  displaySceneNumbers: false
    property bool  displaySceneMenu: false

    property Item currentSceneEditor
    property TextArea currentSceneContentEditor: currentSceneEditor ? currentSceneEditor.editor : null
    signal requestEditor()

    Settings {
        id: screenplayEditorSettings
        fileName: app.settingsFilePath
        category: "Screenplay Editor"
        property bool displaySceneCharacters: true
    }

    Rectangle {
        id: toolbar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 1
        color: primaryColors.c100.background
        radius: 3
        height: Math.max(toolbarLayout.height, searchBar.height)
        visible: scriteDocument.screenplay.elementCount > 0
        border.width: 1
        border.color: primaryColors.borderColor
        readonly property real margin: Math.max( Math.round((width-sceneEditorFontMetrics.pageWidth)/2), sceneEditorFontMetrics.height*2 )
        readonly property real padding: sceneEditorFontMetrics.paragraphMargin + margin

        Row {
            id: toolbarLayout
            anchors.left: parent.left
            anchors.leftMargin: toolbar.margin

            ToolButton2 {
                icon.source: "../icons/screenplay/character.png"
                ToolTip.text: "Toggle display of character names under scene headings and scan for hidden characters in each scene."
                ToolTip.delay: 1000
                down: sceneCharactersMenu.visible
                onClicked: sceneCharactersMenu.visible = true

                Item {
                    width: parent.width
                    height: 1
                    anchors.top: parent.bottom

                    Menu2 {
                        id: sceneCharactersMenu
                        width: 300

                        MenuItem2 {
                            text: "Display scene characters"
                            checkable: true
                            checked: screenplayEditorSettings.displaySceneCharacters
                            onToggled: screenplayEditorSettings.displaySceneCharacters = checked
                        }

                        MenuItem2 {
                            text: "Scan for mute characters"
                            onClicked: scriteDocument.structure.scanForMuteCharacters()
                        }
                    }
                }
            }
        }

        Rectangle {
            anchors.left: toolbarLayout.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: primaryColors.borderColor
        }

        SearchBar {
            id: searchBar
            anchors.left: toolbarLayout.right
            anchors.leftMargin: 5
            anchors.right: parent.right
            anchors.rightMargin: toolbar.margin
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Repeater {
        id: searchAgents
        model: scriteDocument.screenplay.elementCount > 0 ? 1 : 0

        Item {
            property string searchString
            property var searchResults: []
            property int previousSearchResultIndex: -1

            SearchAgent.engine: searchBar.searchEngine

            SearchAgent.onSearchRequest: {
                searchString = string
                searchResults = scriteDocument.screenplay.search(string, 0)
                SearchAgent.searchResultCount = searchResults.length
            }

            SearchAgent.onCurrentSearchResultIndexChanged: {
                clearPreviousSearchResultUserData()
                if(SearchAgent.currentSearchResultIndex >= 0) {
                    var searchResult = searchResults[SearchAgent.currentSearchResultIndex]
                    var sceneIndex = searchResult["sceneIndex"]
                    var screenplayElement = scriteDocument.screenplay.elementAt(sceneIndex)
                    var data = {
                        "searchString": searchString,
                        "currentSearchResultIndex": SearchAgent.currentSearchResultIndex,
                        "searchResultCount": SearchAgent.searchResultCount
                    }
                    screenplayElement.userData = data
                    scriteDocument.screenplay.currentElementIndex = sceneIndex
                    previousSearchResultIndex = sceneIndex
                }
            }

            SearchAgent.onClearSearchRequest: {
                searchString = ""
                clearPreviousSearchResultUserData()
            }

            function clearPreviousSearchResultUserData() {
                if(previousSearchResultIndex >= 0) {
                    var screenplayElement = scriteDocument.screenplay.elementAt(previousSearchResultIndex)
                    if(screenplayElement)
                        screenplayElement.userData = undefined
                }
                previousSearchResultIndex = -1
            }
        }
    }

    ListView {
        id: screenplayListView
        property var lastSceneResetInfo
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: toolbar.bottom
        anchors.margins: 3
        cacheBuffer: Math.ceil(height / 300) * 2
        clip: true
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            minimumSize: 0.1
            palette {
                mid: Qt.rgba(0,0,0,0.5)
                dark: "black"
            }
            opacity: active ? 1 : 0.2
            Behavior on opacity { NumberAnimation { duration: 250 } }
        }
        model: scriteDocument.screenplay
        delegate: Loader {
            property bool hasSceneContent: screenplayElement.elementType === ScreenplayElement.SceneElementType
            width: screenplayListView.width
            sourceComponent: hasSceneContent ? screenplayElementDelegate : breakElementDelegate
            onItemChanged: {
                if(item) {
                    item.index = index
                    item.element = screenplayElement
                }
            }

            active: resetActiveOnIndexChange.value
            ResetOnChange {
                id: resetActiveOnIndexChange
                trackChangesOn: index
                from: false
                to: true
            }
        }
        footer: Item {
            width: screenplayListView.width
            height: scriteDocument.screenplay.elementCount > 0 ? screenplayListView.height/2 : 0
        }
        onMovingChanged: {
            if(moving)
                characterMenu.close()
        }
        currentIndex: -1
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds
        Transition {
            id: moveAndDisplace
            NumberAnimation { properties: "x,y"; duration: 250 }
        }

        FocusTracker.window: qmlWindow
        FocusTracker.indicator.target: mainUndoStack
        FocusTracker.indicator.property: "screenplayEditorActive"

        moveDisplaced: moveAndDisplace
        move: moveAndDisplace

        Connections {
            target: currentSceneContentEditor
            ignoreUnknownSignals: true
            onCursorRectangleChanged: screenplayListView.adjustScroll()
        }

        function adjustScroll() {
            if(currentSceneContentEditor == null)
                return

            var rect = currentSceneContentEditor.cursorRectangle
            var pt = currentSceneContentEditor.mapToItem(screenplayListView.contentItem, rect.x, rect.y)
            var startY = screenplayListView.contentY
            var endY = screenplayListView.contentY + screenplayListView.height - rect.height
            if( startY < pt.y && pt.y < endY )
                return

            if( pt.y < startY )
                screenplayListView.contentY = pt.y
            else if( pt.y > endY )
                screenplayListView.contentY = (pt.y + 2*rect.height) - screenplayListView.height
        }
    }

    onCurrentSceneContentEditorChanged: screenplayListView.adjustScroll()

    Component {
        id: breakElementDelegate

        Rectangle {
            property ScreenplayElement element
            property int index: -1
            height: 50
            color: primaryColors.windowColor
            border.width: 1
            border.color: primaryColors.borderColor

            Text {
                anchors.centerIn: parent
                font.pixelSize: 30
                font.bold: true
                text: element.sceneID
            }
        }
    }

    Component {
        id: screenplayElementDelegate

        Rectangle {
            id: delegateItem
            property ScreenplayElement element
            property int index: -1
            property color sceneColor: element.scene.color
            property bool selected: scriteDocument.screenplay.currentElementIndex === index
            signal assumeFocusAt(int pos)
            onAssumeFocusAt: sceneEditor.assumeFocusAt(pos)
            height: sceneEditor.height + 4
            color: selected ? Qt.tint(sceneColor, "#B0FFFFFF") : Qt.tint(sceneColor, "#F0FFFFFF")

            SceneEditor {
                id: sceneEditor
                scene: element.scene
                height: fullHeight
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 2
                anchors.rightMargin: 2
                scrollable: false
                showOnlyEnabledSceneHeadings: !screenplayEditor.displaySceneMenu
                allowSplitSceneRequest: true
                sceneNumber: parent.element.sceneNumber
                active: parent.selected
                displaySceneNumber: screenplayEditor.displaySceneNumbers
                displaySceneMenu: screenplayEditor.displaySceneMenu
                showCharacterNames: screenplayEditorSettings.displaySceneCharacters
                binder.onDocumentInitialized: {
                    var info = screenplayListView.lastSceneResetInfo
                    screenplayListView.lastSceneResetInfo = undefined
                    if(info) {
                        var position = binder.cursorPositionAtBlock(info.sceneElementIndex)
                        assumeFocusAt(position)
                    }
                }

                onEditorHasActiveFocusChanged: {
                    if(editorHasActiveFocus) {
                        if(scriteDocument.screenplay.currentElementIndex !== index) {
                            currentElementIndexConnections.enabled = false
                            scriteDocument.screenplay.currentElementIndex = index
                            currentElementIndexConnections.enabled = true
                        }
                        currentSceneEditor = sceneEditor
                    }
                }

                onSplitSceneRequest: {
                    scriteDocument.screenplay.splitElement(parent.element, sceneElement, textPosition)
                }

                onRequestScrollUp: {
                    var idx = scriteDocument.screenplay.previousSceneElementIndex()
                    if(idx === 0 && idx === index) {
                        screenplayListView.positionViewAtBeginning()
                        assumeFocusAt(0)
                        return
                    }

                    screenplayListView.positionViewAtIndex(idx, ListView.Visible)
                    var item = screenplayListView.itemAtIndex(idx)
                    item.item.assumeFocusAt(-1)
                }

                onRequestScrollDown: {
                    var idx = scriteDocument.screenplay.nextSceneElementIndex()
                    if(idx === scriteDocument.screenplay.elementCount-1 && idx === index) {
                        screenplayListView.positionViewAtEnd()
                        assumeFocusAt(-1)
                        return
                    }

                    screenplayListView.positionViewAtIndex(idx, ListView.Visible)
                    var item = screenplayListView.itemAtIndex(idx)
                    item.item.assumeFocusAt(0)
                }

                onRequestCharacterMenu: {
                    characterMenu.characterName = characterName
                    characterMenu.popup()
                }

                TextDocumentSearch {
                    textDocument: sceneEditor.editor.textDocument
                    searchString: sceneEditor.binder.documentLoadCount > 0 ? (element.userData ? element.userData.searchString : "") : ""
                    currentResultIndex: searchResultCount > 0 ? (element.userData ? element.userData.currentSearchResultIndex : -1) : -1
                    onHighlightText: {
                        currentSceneEditor = sceneEditor
                        sceneEditor.editor.cursorPosition = start
                        sceneEditor.editor.select(start, end)
                    }
                    onClearHighlight: {
                        sceneEditor.editor.deselect()
                    }
                }

                Connections {
                    target: scriteDocument
                    onNewSceneCreated: {
                        if(screenplayIndex === index)
                            sceneEditor.assumeFocus()
                    }
                }
            }
        }
    }

    Menu2 {
        id: characterMenu
        width: 300
        property string characterName

        MenuItem2 {
            text: "Generate Character Report"
            enabled: characterMenu.characterName !== ""
            onClicked: {
                reportGeneratorTimer.reportArgs = {"reportName": "Character Report", "configuration": {"characterNames": [characterMenu.characterName]}}
                characterMenu.close()
                characterMenu.characterName = ""
            }
        }
    }

    Connections {
        id: currentElementIndexConnections
        target: scriteDocument.screenplay
        onCurrentElementIndexChanged: screenplayListView.positionViewAtIndex(scriteDocument.screenplay.currentElementIndex, ListView.Beginning)
        onSceneReset: {
            screenplayListView.lastSceneResetInfo = {"sceneIndex": sceneIndex, "sceneElementIndex": sceneElementIndex}
            scriteDocument.screenplay.currentElementIndex = sceneIndex
        }
    }

    Component.onCompleted: screenplayListView.positionViewAtIndex(scriteDocument.screenplay.currentElementIndex, ListView.Beginning)
}
