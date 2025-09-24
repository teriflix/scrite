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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

AbstractStructureElementUI {
    id: root

    readonly property alias isEditing: _private.isEditing
    readonly property alias isSelected: _private.isSelected
    readonly property alias isBeingDragged: _private.isBeingDragged

    signal editorRequest()
    signal requestContextMenu(StructureElement element)
    signal resetAnnotationGripRequest()

    Keys.onPressed: (event) => { _private.handleKeyPressEvent(event) }

    Drag.active: _dragMouseArea.drag.active
    Drag.source: root.element.scene
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        let md = {}
        md[Runtime.timelineViewSettings.dropAreaKey] = root.element.scene.id
        return md
    }
    Drag.hotSpot.x: _private.dragImageSize.width/2 // dragHandle.x + dragHandle.width/2
    Drag.hotSpot.y: _private.dragImageSize.height/2 // dragHandle.y + dragHandle.height/2
    Drag.supportedActions: Qt.LinkAction

    BoundingBoxItem.evaluator: root.canvasItemsBoundingBox
    BoundingBoxItem.stackOrder: 3.0 + (root.elementIndex/Scrite.document.structure.elementCount)
    BoundingBoxItem.livePreview: false
    BoundingBoxItem.viewportRect: root.canvasScrollViewportRect
    BoundingBoxItem.viewportItem: root.canvasScrollViewport
    BoundingBoxItem.visibilityMode: BoundingBoxItem.VisibleUponViewportIntersection
    BoundingBoxItem.previewFillColor: _private.isSelected ? Qt.darker(root.element.scene.color) : root.element.scene.color
    BoundingBoxItem.previewBorderColor: Scrite.app.isLightColor(root.element.scene.color) ? "black" : _background.color
    BoundingBoxItem.previewBorderWidth: _private.isSelected ? 3 : 1.5

    Component.onCompleted: root.element.follow = root

    function finishEditing() {
        _titleText.editMode = false
        root.element.objectName = "oldElement"
    }

    x: _positionBinder.get.x
    y: _positionBinder.get.y
    width: _titleText.width + 10
    height: _titleText.height + 10

    Rectangle {
        id: _background

        anchors.fill: parent

        color: Qt.tint(root.element.scene.color, Runtime.colors.sceneControlTint)
        border.width: _private.selected ? 2 : 1
        border.color: (root.element.scene.color === Qt.rgba(1,1,1,1) ? "gray" : root.element.scene.color)

        Behavior on border.width {
            enabled: Runtime.applicationSettings.enableAnimations
            NumberAnimation { duration: 400 }
        }
    }

    TextViewEdit {
        id: _titleText

        property bool editMode: root.element.objectName === "newElement"

        Keys.onReturnPressed: editingFinished()

        anchors.centerIn: parent

        width: 250

        text: root.element.scene.synopsis
        readOnly: !(editMode && root.elementIndex === Scrite.document.structure.currentElementIndex)
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignLeft

        topPadding: 5
        leftPadding: 17
        rightPadding: 17
        bottomPadding: 5

        font.pointSize: 13

        onTextEdited: root.element.scene.synopsis = text

        onHighlightRequest: Scrite.document.structure.currentElementIndex = root.elementIndex

        onEditingFinished: {
            editMode = false
            root.element.objectName = "oldElement"
        }
    }

    MouseArea {
        anchors.fill: _titleText

        enabled: _titleText.readOnly === true
        acceptedButtons: Qt.LeftButton

        drag.target: Scrite.document.readOnly || Scrite.document.structure.forceBeatBoardLayout ? null : root
        drag.axis: Drag.XAndYAxis
        drag.minimumX: 0
        drag.minimumY: 0

        drag.onActiveChanged: {
            root.canvasActiveFocusRequest()
            Scrite.document.structure.currentElementIndex = root.elementIndex
            if(drag.active === false) {
                root.x = Scrite.document.structure.snapToGrid(parent.x)
                root.y = Scrite.document.structure.snapToGrid(parent.y)
            } else
                root.element.syncWithFollow = true
        }

        onDoubleClicked: {
            root.resetAnnotationGripRequest()
            root.canvasActiveFocusRequest()
            Scrite.document.structure.currentElementIndex = root.elementIndex
            if(!Scrite.document.readOnly)
                _titleText.editMode = true
        }

        onClicked: {
            root.resetAnnotationGripRequest()
            root.canvasActiveFocusRequest()
            Scrite.document.structure.currentElementIndex = root.elementIndex
            root.editorRequest()
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: {
            root.canvasActiveFocusRequest()
            Scrite.document.structure.currentElementIndex = root.elementIndex
            root.requestContextMenu(root.element)
        }
    }

    SceneTypeImage {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 3

        width: 18
        height: 18

        opacity: 0.5
        sceneType: root.element.scene.type
        showTooltip: false
        lightBackground: Scrite.app.isLightColor(_background.color)
    }

    Image {
        id: _dragHandle

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 3
        anchors.bottomMargin: 1

        width: 18
        height: 18

        scale: _dragMouseArea.pressed ? 2 : 1
        source: root.element.scene.addedToScreenplay || root.Drag.active ? "qrc:/icons/action/view_array.png" : "qrc:/icons/content/add_circle_outline.png"
        visible: !_private.isEditing && !Scrite.document.readOnly
        enabled: /*!StructureModule.canvas.editElementItem &&*/ !Scrite.document.readOnly
        opacity: _private.isSelected ? 1 : 0.1

        Behavior on scale {
            enabled: Runtime.applicationSettings.enableAnimations
            NumberAnimation { duration: Runtime.stdAnimationDuration }
        }

        MouseArea {
            id: _dragMouseArea

            anchors.fill: parent

            drag.target: parent
            drag.onActiveChanged: {
                _private.isBeingDragged = drag.active
                if(drag.active)
                    root.canvasActiveFocusRequest()
            }

            onPressed: {
                _private.isBeingDragged = true

                root.canvasActiveFocusRequest()
                root.grabToImage(function(result) {
                    root.Drag.imageSource = result.url
                }, _private.dragImageSize)
            }

            onReleased: { _private.isBeingDragged = false }

            onClicked: {
                if(!root.element.scene.addedToScreenplay)
                    Scrite.document.screenplay.addScene(root.element.scene)
            }
        }
    }

    DelayedPropertyBinder {
        id: _positionBinder

        set: root.element.position
        initial: Qt.point(root.element.x, root.element.y)
        onGetChanged: {
            root.x = get.x
            root.y = get.y
        }
    }

    onFinishEditingRequest: finishEditing()

    QtObject {
        id: _private

        readonly property size maxDragImageSize: Qt.size(36, 36)

        property size dragImageSize: {
            const s = root.width > root.height ? maxDragImageSize.width / root.width : maxDragImageSize.height / root.height
            return Qt.size( root.width*s, root.height*s )
        }

        property bool isSelected: Scrite.document.structure.currentElementIndex === root.elementIndex
        property bool isEditing: !_titleText.readOnly && _titleText.hasFocus
        property bool isBeingDragged: false

        function handleKeyPressEvent(event) {
            if(event.key === Qt.Key_F2) {
                root.canvasActiveFocusRequest()
                Scrite.document.structure.currentElementIndex = root.elementIndex
                if(!Scrite.document.readOnly)
                    _titleText.editMode = true
                event.accepted = true
            } else
                event.accepted = false
        }
    }
}
