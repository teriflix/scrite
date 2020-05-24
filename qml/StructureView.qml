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
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    signal requestEditor()
    signal releaseEditor()

    Rectangle {
        id: toolbar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 1
        color: primaryColors.c100.background
        radius: 3
        height: toolbarLayout.height+4

        Row {
            id: toolbarLayout
            spacing: 3
            width: parent.width-4
            anchors.verticalCenter: parent.verticalCenter

            Row {
                id: toolbarButtons
                spacing: parent.spacing
                anchors.verticalCenter: parent.verticalCenter

                ToolButton2 {
                    id: newSceneButton
                    icon.source: "../icons/content/add_box.png"
                    text: "Add Scene"
                    shortcutText: "N"
                    ToolTip.text: "Press " + shortcutText + " to create a new scene under the mouse on the canvas."
                    suggestedWidth: 130
                    suggestedHeight: 50
                    display: ToolButton.TextBesideIcon
                    down: newSceneColorMenuLoader.active
                    onClicked: newSceneColorMenuLoader.active = true
                    anchors.verticalCenter: parent.verticalCenter
                    property color activeColor: "white"

                    Loader {
                        id: newSceneColorMenuLoader
                        width: parent.width; height: 1
                        anchors.top: parent.bottom
                        sourceComponent: ColorMenu {
                            selectedColor: newSceneButton.activeColor
                        }
                        active: false
                        onItemChanged: {
                            if(item)
                                item.open()
                        }

                        Connections {
                            target: newSceneColorMenuLoader.item
                            onAboutToHide: newSceneColorMenuLoader.active = false
                            onMenuItemClicked: {
                                newSceneButton.activeColor = color
                                createElementMouseHandler.enabled = true
                                newSceneColorMenuLoader.active = false
                            }
                        }
                    }
                }

                ToolButton2 {
                    icon.source: "../icons/navigation/zoom_in.png"
                    text: "Zoom In"
                    display: ToolButton.IconOnly
                    suggestedHeight: 45
                    anchors.verticalCenter: parent.verticalCenter
                    autoRepeat: true
                    onClicked: canvasScroll.zoomIn()
                }

                ToolButton2 {
                    icon.source: "../icons/navigation/zoom_out.png"
                    text: "Zoom Out"
                    display: ToolButton.IconOnly
                    suggestedHeight: 45
                    anchors.verticalCenter: parent.verticalCenter
                    autoRepeat: true
                    onClicked: canvasScroll.zoomOut()
                }
            }

            SearchBar {
                id: searchBar
                width: parent.width-toolbarButtons.width-parent.spacing
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Rectangle {
        anchors.fill: canvasScroll
        color: structureCanvasSettings.canvasColor
    }

    ScrollArea {
        id: canvasScroll
        anchors.left: parent.left
        anchors.top: toolbar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 3
        contentWidth: canvas.width * canvas.scale
        contentHeight: canvas.height * canvas.scale
        initialContentWidth: canvas.width
        initialContentHeight: canvas.height
        clip: true
        showScrollBars: scriteDocument.structure.elementCount >= 1
        interactive: !(rubberBand.active || selection.active) && mouseOverItem === null && editItem === null
        property Item mouseOverItem
        property Item editItem

        GridBackground {
            id: canvas
            antialiasing: false
            majorTickLineWidth: 2
            minorTickLineWidth: 1
            width: widthBinder.get
            height: heightBinder.get
            tickColorOpacity: 0.25 * scale
            scale: canvasScroll.suggestedScale
            border.width: 2
            border.color: structureCanvasSettings.gridColor
            gridIsVisible: structureCanvasSettings.showGrid
            majorTickColor: structureCanvasSettings.gridColor
            minorTickColor: structureCanvasSettings.gridColor
            tickDistance: scriteDocument.structure.canvasGridSize
            transformOrigin: Item.TopLeft

            function createElement(x, y, c) {
                var props = {
                    "x": Math.max(scriteDocument.structure.snapToGrid(x), 130),
                    "y": Math.max(scriteDocument.structure.snapToGrid(y), 50)
                }

                var element = newStructureElementComponent.createObject(scriteDocument.structure, props)
                element.scene.color = c
                scriteDocument.structure.addElement(element)
                scriteDocument.structure.currentElementIndex = scriteDocument.structure.elementCount-1
                requestEditor()
                element.scene.undoRedoEnabled = true
            }

            DelayedPropertyBinder {
                id: widthBinder
                initial: 1000
                set: Math.max((Math.ceil(canvas.childrenRect.right / 100) * 100), initial)
                onGetChanged: scriteDocument.structure.canvasWidth = get
            }

            DelayedPropertyBinder {
                id: heightBinder
                initial: 1000
                set: Math.max((Math.ceil(canvas.childrenRect.bottom / 100) * 100), initial)
                onGetChanged: scriteDocument.structure.canvasHeight = get
            }

            FocusTracker.window: qmlWindow
            FocusTracker.indicator.target: mainUndoStack
            FocusTracker.indicator.property: "structureEditorActive"

            MouseArea {
                id: createElementMouseHandler
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                enabled: false
                onClicked: {
                    canvas.createElement(mouse.x-130, mouse.y-22, newSceneButton.activeColor)
                    enabled = false
                }
            }

            RubberBand {
                id: rubberBand
                enabled: !createElementMouseHandler.enabled
                anchors.fill: parent
                z: active ? 1000 : -1
                onTryStart: {
                    parent.forceActiveFocus()
                    active = true // TODO
                }
                onSelect: selection.init(elementItems, rectangle)
            }

            BorderImage {
                source: "../icons/content/shadow.png"
                anchors.fill: currentElementItem
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
                anchors { leftMargin: -11; topMargin: -11; rightMargin: -10; bottomMargin: -10 }
                border { left: 21; top: 21; right: 21; bottom: 21 }
                visible: currentElementItem !== null
                opacity: 0.55
                property Item currentElementItem: elementItems.count > scriteDocument.structure.currentElementIndex ? elementItems.itemAt(scriteDocument.structure.currentElementIndex) : null
                onCurrentElementItemChanged: app.execLater(currentElementItem, 100, function() { canvasScroll.ensureItemVisible(currentElementItem, canvas.scale) })
            }

            Repeater {
                id: elementConnectorItems
                model: displayContent ? scriteDocument.structureElementSequence : 0
                delegate: elementConnectorComponent
            }

            MouseArea {
                anchors.fill: parent
                enabled: canvasScroll.editItem !== null
                onClicked: canvasScroll.editItem.finishEditing()
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onPressed: canvasContextMenu.popup()
            }

            Repeater {
                id: elementItems
                model: displayContent ? scriteDocument.structure.elements : 0
                delegate: structureElementDelegate
            }

            Selection {
                id: selection
                anchors.fill: parent
                onMoveItem: {
                    item.x = item.x + dx
                    item.y = item.y + dy
                }
                onPlaceItem: {
                    item.x = scriteDocument.structure.snapToGrid(item.x)
                    item.y = scriteDocument.structure.snapToGrid(item.y)
                }
            }

            Menu2 {
                id: canvasContextMenu

                MenuItem2 {
                    text: "New Scene"
                    onClicked: canvas.createElement(canvasContextMenu.x-130, canvasContextMenu.y-22, newSceneButton.activeColor)
                }

                ColorMenu {
                    title: "Colored Scene"
                    selectedColor: newSceneButton.activeColor
                    onMenuItemClicked: {
                        newSceneButton.activeColor = color
                        canvas.createElement(canvasContextMenu.x-130, canvasContextMenu.y-22, newSceneButton.activeColor)
                        canvasContextMenu.close()
                    }
                }
            }

            Menu2 {
                id: elementContextMenu
                property StructureElement element
                onElementChanged: {
                    if(element)
                        popup()
                    else
                        close()
                }

                MenuItem2 {
                    action: Action {
                        text: "Scene Heading"
                        checkable: true
                        checked: elementContextMenu.element ? elementContextMenu.element.scene.heading.enabled : false
                    }
                    enabled: elementContextMenu.element !== null
                    onTriggered: {
                        elementContextMenu.element.scene.heading.enabled = action.checked
                        elementContextMenu.element = null
                    }
                }

                ColorMenu {
                    title: "Color"
                    enabled: elementContextMenu.element !== null
                    onMenuItemClicked: {
                        elementContextMenu.element.scene.color = color
                        elementContextMenu.element = null
                    }
                }

                MenuItem2 {
                    text: "Duplicate"
                    enabled: elementContextMenu.element !== null
                    onClicked: {
                        releaseEditor()
                        elementContextMenu.element.duplicate()
                        elementContextMenu.element = null
                    }
                }

                MenuSeparator { }

                MenuItem2 {
                    text: "Delete"
                    enabled: elementContextMenu.element !== null
                    onClicked: {
                        releaseEditor()
                        scriteDocument.screenplay.removeSceneElements(elementContextMenu.element.scene)
                        scriteDocument.structure.removeElement(elementContextMenu.element)
                        elementContextMenu.element = null
                    }
                }
            }
        }
    }

    Loader {
        width: parent.width*0.7
        anchors.centerIn: parent
        active: scriteDocument.structure.elementCount === 0
        sourceComponent: TextArea {
            readOnly: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 30
            enabled: false
            // renderType: Text.NativeRendering
            text: "Create scenes by clicking on the 'Add Scene' button OR right click to see options."
        }
    }

    Component {
        id: newStructureElementComponent

        StructureElement {
            objectName: "newElement"
            scene: Scene {
                title: "New Scene"
                heading.locationType: "INT"
                heading.location: "SOMEWHERE"
                heading.moment: "DAY"
            }
        }
    }

    Component {
        id: structureElementDelegate

        Item {
            id: elementItem
            property StructureElement element: modelData
            Component.onCompleted: element.follow = elementItem
            enabled: selection.active === false

            readonly property bool selected: scriteDocument.structure.currentElementIndex === index
            readonly property bool editing: titleText.readOnly === false
            onEditingChanged: {
                if(editing)
                    canvasScroll.editItem = elementItem
                else if(canvasScroll.editItem === elementItem)
                    canvasScroll.editItem = null
            }

            function finishEditing() {
                titleText.editMode = false
                element.objectName = "oldElement"
            }

            width: titleText.width + 10
            height: titleText.height + 10
            x: positionBinder.get.x
            y: positionBinder.get.y

            DelayedPropertyBinder {
                id: positionBinder
                initial: Qt.point(element.x, element.y)
                set: element.position
                onGetChanged: {
                    elementItem.x = get.x
                    elementItem.y = get.y
                }
            }

            Rectangle {
                id: background
                radius: 3
                anchors.fill: parent
                border.width: elementItem.selected ? 2 : 1
                border.color: (element.scene.color === Qt.rgba(1,1,1,1) ? "lightgray" : element.scene.color)
                color: Qt.tint(element.scene.color, "#C0FFFFFF")
                Behavior on border.width { NumberAnimation { duration: 400 } }
            }

            TextViewEdit {
                id: titleText
                width: 250
                wrapMode: Text.WordWrap
                text: element.scene.title
                anchors.centerIn: parent
                font.pointSize: 15
                horizontalAlignment: Text.AlignHCenter
                onTextEdited: element.scene.title = text
                onEditingFinished: {
                    editMode = false
                    element.objectName = "oldElement"
                }
                onHighlightRequest: scriteDocument.structure.currentElementIndex = index
                Keys.onReturnPressed: editingFinished()
                searchEngine: searchBar.searchEngine
                searchSequenceNumber: index
                property bool editMode: element.objectName === "newElement"
                readOnly: !(editMode && index === scriteDocument.structure.currentElementIndex)
            }

            MouseArea {
                anchors.fill: titleText
                enabled: titleText.readOnly === true
                onPressedChanged: {
                    if(pressed)
                        canvasScroll.mouseOverItem = elementItem
                    else if(canvasScroll.mouseOverItem === elementItem)
                        canvasScroll.mouseOverItem = null
                }
                acceptedButtons: Qt.LeftButton
                onDoubleClicked: {
                    canvas.forceActiveFocus()
                    searchBar.searchEngine.clearSearch()
                    scriteDocument.structure.currentElementIndex = index
                    titleText.editMode = true
                }
                onClicked: {
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    requestEditor()
                }

                drag.target: elementItem
                drag.axis: Drag.XAndYAxis
                drag.minimumX: 0
                drag.minimumY: 0
                drag.onActiveChanged: {
                    canvas.forceActiveFocus()
                    if(drag.active === false) {
                        elementItem.x = scriteDocument.structure.snapToGrid(parent.x)
                        elementItem.y = scriteDocument.structure.snapToGrid(parent.y)
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    canvas.forceActiveFocus()
                    scriteDocument.structure.currentElementIndex = index
                    elementContextMenu.element = elementItem.element
                }
            }

            // Drag to timeline support
            Drag.active: dragMouseArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.LinkAction
            Drag.hotSpot.x: dragHandle.x + dragHandle.width/2
            Drag.hotSpot.y: dragHandle.y + dragHandle.height/2
            Drag.mimeData: {
                "scrite/sceneID": element.scene.id
            }
            Drag.source: element.scene

            Image {
                id: dragHandle
                visible: !parent.editing
                source: "../icons/action/view_array.png"
                width: 24; height: 24
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 1
                anchors.rightMargin: 3
                opacity: dragMouseArea.containsMouse ? 1 : 0.1
                scale: dragMouseArea.containsMouse ? 2 : 1
                Behavior on scale { NumberAnimation { duration: 250 } }

                MouseArea {
                    id: dragMouseArea
                    hoverEnabled: true
                    anchors.fill: parent
                    drag.target: parent
                    cursorShape: Qt.SizeAllCursor
                    onPressed: {
                        canvas.forceActiveFocus()
                        elementItem.grabToImage(function(result) {
                            elementItem.Drag.imageSource = result.url
                        })
                    }
                }
            }
        }
    }

    Component {
        id: elementConnectorComponent

        StructureElementConnector {
            lineType: StructureElementConnector.CurvedLine
            fromElement: scriteDocument.structure.elementAt(modelData.from)
            toElement: scriteDocument.structure.elementAt(modelData.to)
            arrowAndLabelSpacing: labelBg.width
            outlineWidth: app.devicePixelRatio*2

            Rectangle {
                id: labelBg
                width: Math.max(label.width,label.height)+20
                height: width; radius: width/2
                border.width: 1; border.color: primaryColors.borderColor
                x: parent.suggestedLabelPosition.x - radius
                y: parent.suggestedLabelPosition.y - radius
                color: Qt.tint(parent.outlineColor, "#E0FFFFFF")

                Text {
                    id: label
                    anchors.centerIn: parent
                    font.pixelSize: 12
                    text: (index+1)
                }
            }
        }
    }

    readonly property ProgressReport documentProgress: Aggregation.findProgressReport(scriteDocument)
    readonly property bool displayContent: documentProgress.status === ProgressReport.Finished || documentProgress.status === ProgressReport.NotStarted
}
