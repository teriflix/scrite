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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

Rectangle {
    id: root

    /**
      groupBox is a JSON object of the form
      {
        "name": ".....",
        "sceneCount": ...,"
        "sceneIndexes": [ .., .., .., .., .., ...... ],
        "geometry": { x: .., y: .. , width: .., height: .. }
      }
      */
    required property var groupBox
    required property int groupBoxIndex
    required property int groupBoxCount

    required property Item canvasScrollViewport
    required property rect canvasScrollViewportRect

    required property Repeater elementItems

    required property BoundingBoxEvaluator canvasItemsBoundingBox

    signal setSelectionRequest(var items) // must be an array if Item objects.
    signal clearSelectionRequest()

    BoundingBoxItem.evaluator: canvasItemsBoundingBox
    BoundingBoxItem.stackOrder: 2.0 + (groupBoxIndex/groupBoxCount)
    BoundingBoxItem.livePreview: false
    BoundingBoxItem.viewportItem: canvasScrollViewport
    BoundingBoxItem.viewportRect: canvasScrollViewportRect
    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
    BoundingBoxItem.previewEnabled: false

    x: groupBox.geometry.x - 20
    y: groupBox.geometry.y - 20 - _private.tipMarginForStacks
    width: groupBox.geometry.width + 40
    height: groupBox.geometry.height + 40 + _private.tipMarginForStacks

    color: Color.translucent(Runtime.colors.accent.c100.background, Scrite.document.structure.forceBeatBoardLayout ? 0.3 : 0.1)
    radius: 0

    border.width: 1
    border.color: Runtime.colors.accent.borderColor

    MouseArea {
        id: _canvasBeatMouseArea

        property bool __controlPressed: false

        anchors.fill: parent

        drag.axis: __controlPressed || Scrite.document.structure.forceBeatBoardLayout ? Drag.None : Drag.XAndYAxis
        drag.target: __controlPressed || Scrite.document.structure.forceBeatBoardLayout ? null : root

        cursorShape: Qt.SizeAllCursor

        onPressed: (mouse) => {
                       __controlPressed = mouse.modifiers & Qt.ControlModifier
                       if(__controlPressed) {
                           mouse.accepted = false
                           return
                       }
                   }

        onDoubleClicked: _private.selectBeatItems()

        drag.onActiveChanged: {
            root.clearSelectionRequest()

            _private.refX = root.x
            _private.refY = root.y

            root.groupsBeingMoved = drag.active
        }
    }

    Rectangle {
        anchors.fill: _beatLabel
        anchors.margins: -parent.radius

        color: Color.translucent(Runtime.colors.accent.c200.background, 0.4)

        border.width: parent.border.width
        border.color: parent.border.color

        MouseArea {
            id: _canvasBeatLabelMouseArea

            property bool __controlPressed: false

            anchors.fill: parent

            drag.axis: __controlPressed || Scrite.document.structure.forceBeatBoardLayout ? Drag.None : Drag.XAndYAxis
            drag.target: __controlPressed || Scrite.document.structure.forceBeatBoardLayout ? null : root

            cursorShape: Qt.SizeAllCursor

            onPressed: (mouse) => {
                           __controlPressed = mouse.modifiers & Qt.ControlModifier
                           if(__controlPressed) {
                               mouse.accepted = false
                               return
                           }
                       }

            drag.onActiveChanged: {
                root.clearSelectionRequest()

                _private.refX = root.x
                _private.refY = root.y

                root.groupsBeingMoved = drag.active
            }

            onDoubleClicked: _private.selectBeatItems()
        }
    }

    VclLabel {
        id: _beatLabel

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.top
        anchors.bottomMargin: parent.radius-parent.border.width

        text: "<b>" + groupBox.name + "</b><font size=\"-2\">: " + groupBox.sceneCount + (groupBox.sceneCount === 1 ? " Scene": " Scenes") + "</font>"
        color: Runtime.colors.accent.c200.text
        elide: Text.ElideRight
        padding: 10

        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 3
    }

    onXChanged: _private.maybeMoveBeat()
    onYChanged: _private.maybeMoveBeat()

    QtObject {
        id: _private

        property real refX: root.x
        property real refY: root.y

        property real tipMarginForStacks: Scrite.document.structure.elementStacks.objectCount > 0 ? 15 : 0

        function maybeMoveBeat() {
            if(_canvasBeatMouseArea.drag.active || _canvasBeatLabelMouseArea.drag.active)
                Qt.callLater(moveBeat)
        }

        function moveBeat() {
            let dx = root.x - _private.refX
            let dy = root.y - _private.refY
            let nrElements = root.groupBox.sceneCount
            let idxList = root.groupBox.sceneIndexes
            let movedIdxList = []
            for(let i=0; i<nrElements; i++) {
                let idx = idxList[i]
                if(movedIdxList.indexOf(idx) < 0) {
                    let item = elementItems.itemAt(idxList[i])
                    item.x = item.x + dx
                    item.y = item.y + dy
                    movedIdxList.push(idx)
                }
            }
            _private.refX = x
            _private.refY = y
        }

        function selectBeatItems() {
            let items = []
            let nrElements = root.groupBox.sceneCount
            let idxList = root.groupBox.sceneIndexes
            let selIdxList = []
            for(let i=0; i<nrElements; i++) {
                let idx = idxList[i]
                if(selIdxList.indexOf(idx) < 0) {
                    let item = elementItems.itemAt(idxList[i])
                    items.push(item)
                    selIdxList.push(idx)
                }
            }

            root.setSelectionRequest(items)
        }
    }
}
