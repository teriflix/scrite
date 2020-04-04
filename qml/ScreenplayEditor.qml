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
    property Item currentSceneEditor
    property TextArea currentSceneContentEditor: currentSceneEditor ? currentSceneEditor.editor : null
    signal requestEditor()

    ScrollView {
        anchors.fill: parent

        ListView {
            id: screenplayListView
            model: scriteDocument.screenplay
            delegate: expandedDelegate
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            boundsMovement: Flickable.StopAtBounds

            Connections {
                target: currentSceneContentEditor
                onCursorRectangleChanged: {
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
        }
    }

    Component {
        id: expandedDelegate

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

    Component {
        id: collapsedDelegate

        Item {
            // TODO
        }
    }

    Connections {
        id: currentElementIndexConnections
        target: scriteDocument.screenplay
        onCurrentElementIndexChanged: screenplayListView.positionViewAtIndex(scriteDocument.screenplay.currentElementIndex, ListView.Beginning)
    }

    Component.onCompleted: screenplayListView.positionViewAtIndex(scriteDocument.screenplay.currentElementIndex, ListView.Beginning)
}
