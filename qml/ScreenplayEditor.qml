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
        model: scriteDocument.screenplay.elementCount

        Item {
            property ScreenplayElement screenplayElement: scriteDocument.screenplay.elementAt(index)
            property Scene scene: screenplayElement ? screenplayElement.scene : null
            property string searchString
            SearchAgent.engine: searchBar.searchEngine
            SearchAgent.sequenceNumber: index
            SearchAgent.onSearchRequest: {
                searchString = string
                var nrElements = scene.elementCount
                var nrResults = 0
                for(var i=0; i<nrElements; i++) {
                    var element = scene.elementAt(i)
                    var posList = SearchAgent.indexesOf(string, element.text)
                    nrResults += posList.length
                }
                SearchAgent.searchResultCount = nrResults
            }
            SearchAgent.onCurrentSearchResultIndexChanged: {
                if(SearchAgent.currentSearchResultIndex >= 0) {
                    var data = {
                        "searchString": searchString,
                        "currentSearchResultIndex": SearchAgent.currentSearchResultIndex,
                        "searchResultCount": SearchAgent.searchResultCount
                    }
                    screenplayElement.userData = data
                    scriteDocument.screenplay.currentElementIndex = index
                }
            }
            SearchAgent.onClearSearchRequest: {
                searchString = ""
                screenplayElement.userData = undefined
            }
        }
    }

    ListView {
        id: screenplayListView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: searchBar.bottom
        clip: true
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AlwaysOn }
        model: scriteDocument.screenplay
        delegate: screenplayElementDelegate
        currentIndex: -1
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds
        Transition {
            id: moveAndDisplace
            NumberAnimation { properties: "x,y"; duration: 250 }
        }

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
            var endY = screenplayListView.contentY + screenplayListView.height
            if( startY < pt.y && pt.y < endY )
                return

            endY = endY-40
            if( pt.y < startY )
                screenplayListView.contentY = pt.y
            else if( pt.y > endY )
                screenplayListView.contentY = (pt.y + 40) - screenplayListView.height
        }
    }

    onCurrentSceneContentEditorChanged: screenplayListView.adjustScroll()

    Component {
        id: screenplayElementDelegate

        Rectangle {
            id: delegateItem
            property ScreenplayElement element: screenplayElement
            property color sceneColor: element.scene.color
            property bool selected: scriteDocument.screenplay.currentElementIndex === index
            signal assumeFocusAt(int pos)
            onAssumeFocusAt: sceneEditor.assumeFocusAt(pos)

            width: screenplayListView.width
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
                    onEditorHasActiveFocusChanged: {
                        if(editorHasActiveFocus) {
                            currentElementIndexConnections.enabled = false
                            scriteDocument.screenplay.currentElementIndex = index
                            currentElementIndexConnections.enabled = true
                            currentSceneEditor = sceneEditor
                        }
                    }
                    onRequestScrollUp: {
                        if(index > 0) {
                            var item = screenplayListView.itemAtIndex(index-1)
                            item.assumeFocusAt(-1)
                        }
                    }

                    onRequestScrollDown: {
                        if(index < scriteDocument.screenplay.elementCount) {
                            var item = screenplayListView.itemAtIndex(index+1)
                            item.assumeFocusAt(0)
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
    }

    Component.onCompleted: screenplayListView.positionViewAtIndex(scriteDocument.screenplay.currentElementIndex, ListView.Beginning)
}
