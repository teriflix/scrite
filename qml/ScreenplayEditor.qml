/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    id: screenplayEditor
    property Item currentSceneEditor
    property TextArea currentSceneContentEditor: currentSceneEditor ? currentSceneEditor.editor : null
    signal requestEditor()

    SearchBar {
        id: searchBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
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


    FocusIndicator {
        id: focusIndicator
        active: sceneEditorUndoStack.active
        anchors.fill: screenplayListView
        anchors.margins: -3
    }

    ListView {
        id: screenplayListView
        property var lastSceneResetInfo
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: searchBar.bottom
        anchors.margins: 3
        clip: true
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            minimumSize: 0.1
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
        }
        currentIndex: -1
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds
        Transition {
            id: moveAndDisplace
            NumberAnimation { properties: "x,y"; duration: 250 }
        }

        FocusTracker.window: qmlWindow
        FocusTracker.indicator.target: sceneEditorUndoStack

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
            color: "white"

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
            height: layout.height + 20
            color: selected ? sceneColor : Qt.tint(sceneColor, "#C0FFFFFF")

            Row {
                id: layout
                width: parent.width-10
                height: Math.max(sceneTitleText.height, sceneEditor.height)
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: sceneTitle
                    implicitHeight: sceneTitleText.width
                    height: parent.height
                    width: 50
                    color: selected ? sceneColor : Qt.rgba(0,0,0,0)
                    clip: true

                    Text {
                        id: sceneTitleText
                        text: element.scene.title
                        anchors.top: parent.top
                        anchors.topMargin: width/2
                        anchors.horizontalCenter: parent.horizontalCenter
                        rotation: -90
                        font.pixelSize: 24
                        font.bold: delegateItem.selected
                        font.letterSpacing: 2
                        color: {
                            if(sceneColor === "white" || sceneColor === "yellow")
                                return "black"
                            delegateItem.selected ? "white" : "black"
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: scriteDocument.screenplay.currentElementIndex = index
                    }
                }

                SceneEditor {
                    id: sceneEditor
                    scene: element.scene
                    height: fullHeight
                    width: parent.width - sceneTitle.width - parent.spacing
                    scrollable: false
                    showOnlyEnabledSceneHeadings: true
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
                    onRequestScrollUp: {
                        var item = null
                        var idx = index
                        while(idx > 0) {
                            item = screenplayListView.itemAtIndex(idx-1)
                            if(item && item.hasSceneContent) {
                                item.item.assumeFocusAt(-1)
                                break
                            }
                            idx = idx-1
                        }
                    }

                    onRequestScrollDown: {
                        var item = null
                        var idx = index
                        while(idx < scriteDocument.screenplay.elementCount) {
                            item = screenplayListView.itemAtIndex(idx+1)
                            if(item && item.hasSceneContent) {
                                item.item.assumeFocusAt(0)
                                break
                            }
                            idx = idx+1
                        }
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
