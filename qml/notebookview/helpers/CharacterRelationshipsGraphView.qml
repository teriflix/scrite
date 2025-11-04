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
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/notebookview/dialogs"

Rectangle {
    id: root

    property bool editRelationshipsEnabled: false
    property bool showBusyIndicator: false

    property alias scene: _graph.scene
    property alias character: _graph.character
    property alias structure: _graph.structure
    property alias graphIsEmpty: _graph.empty

    signal characterDoubleClicked(string characterName, Item chNodeItem)
    signal addNewRelationshipRequest(Item sourceItem)
    signal removeRelationshipWithRequest(Character otherCharacter, Item sourceItem)

    function resetGraph() { _graph.reset() }
    function exportToPdf(popupSource) {
        ExportConfigurationDialog.launch(_graph.createExporterObject())
    }

    color: Color.translucent(Runtime.colors.primary.c100.background, 0.5)
    border.width: 1
    border.color: Runtime.colors.primary.borderColor

    CharacterRelationshipGraph {
        id: _graph

        leftMargin: 1000
        maxIterations: Runtime.notebookSettings.graphLayoutMaxIterations
        maxTime: Runtime.notebookSettings.graphLayoutMaxTime
        nodeSize: Qt.size(150,150)
        structure: Scrite.document.loading ? null : Scrite.document.structure
        topMargin: 1000

        onUpdated: {
            Runtime.execLater(_graph, 250, function() {
                _canvasScroll.animatePanAndZoom = false
                _canvas.zoomFit()
                _canvasScroll.animatePanAndZoom = true
                _canvas.selectedNodeItem = _canvas.mainCharacterNodeItem
            })
        }
    }

    ScrollArea {
        id: _canvasScroll

        function zoomOneMiddleArea() {
            _nodeItemsBoxEvaluator.recomputeBoundingBox()
            const bboxCenter = _nodeItemsBoxEvaluator.center
            const middleArea = Qt.rect(bboxCenter.x - _canvasScroll.width/2,
                                     bboxCenter.y - _canvasScroll.height/2,
                                     _canvasScroll.width,
                                     _canvasScroll.height)
            _canvasScroll.zoomOne()
            _canvasScroll.ensureVisible(middleArea)
        }

        function zoomOneToItem(item) {
            if(item === null)
                return
            const bbox = _nodeItemsBoxEvaluator.boundingBox
            const itemRect = Qt.rect(item.x, item.y, item.width, item.height)
            const atBest = Qt.size(_canvasScroll.width, _canvasScroll.height)
            const visibleArea = GMath.querySubRectangle(bbox, itemRect, atBest)
            _canvasScroll.zoomFit(visibleArea)
        }

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: _statusBar.top

        contentWidth: _canvas.width * _canvas.scale
        contentHeight: _canvas.height * _canvas.scale
        initialContentWidth: _canvas.width
        initialContentHeight: _canvas.height

        clip: true
        handlePinchZoom: true
        minimumScale: _nodeItemsBoxEvaluator.itemCount > 0 ? Math.min(0.25, width/_nodeItemsBoxEvaluator.width, height/_nodeItemsBoxEvaluator.height) : 0.25
        showScrollBars: true
        zoomOnScroll: Runtime.workspaceSettings.mouseWheelZoomsInCharacterGraph

        Item {
            id: _canvas

            property Character activeCharacter: selectedNodeItem ? selectedNodeItem.character : null
            property Item selectedNodeItem
            property Item mainCharacterNodeItem


            function reloadIfDirty() {
                if(_graph.dirty)
                    _graph.reload()
            }

            function zoomFit() {
                if(_nodeItemsBox.width > _canvasScroll.width || _nodeItemsBox.height > _canvasScroll.height)
                    _canvasScroll.zoomFit(Qt.rect(_nodeItemsBox.x,_nodeItemsBox.y,_nodeItemsBox.width,_nodeItemsBox.height))
                else {
                    const centerX = _nodeItemsBox.x + _nodeItemsBox.width/2
                    const centerY = _nodeItemsBox.y + _nodeItemsBox.height/2
                    const w = _canvasScroll.width
                    const h = _canvasScroll.height
                    const x = centerX - w/2
                    const y = centerY - h/2
                    _canvasScroll.zoomFit(Qt.rect(x,y,w,h))
                }
            }

            width: 120000
            height: 120000

            scale: _canvasScroll.suggestedScale
            transformOrigin: Item.TopLeft

            MouseArea {
                anchors.fill: parent

                enabled: _canvas.selectedNodeItem || _graph.dirty

                onClicked: {
                    _canvas.selectedNodeItem = null;
                    _canvas.reloadIfDirty();
                }
            }

            Rectangle {
                id: _nodeItemsBox

                x: _graph.nodes.objectCount > 0 ? (_nodeItemsBoxEvaluator.boundingBox.x - 50) : 0
                y: _graph.nodes.objectCount > 0 ? _nodeItemsBoxEvaluator.boundingBox.y - 50 : 0
                width: _graph.nodes.objectCount > 0 ? (_nodeItemsBoxEvaluator.boundingBox.width + 100) : Math.floor(Math.min(_canvasScroll.width,_canvasScroll.height)/100)*100
                height: _graph.nodes.objectCount > 0 ? (_nodeItemsBoxEvaluator.boundingBox.height + 100) : width

                color: Qt.rgba(0,0,0,0)
                radius: 6

                border.width: _graph.nodes.objectCount > 0 ? 1 : 0
                border.color: Runtime.colors.primary.borderColor
            }

            BoundingBoxEvaluator {
                id: _nodeItemsBoxEvaluator
            }

            Item {
                id: _edgeItems

                Repeater {
                    id: _edgeItemsRepeater
                    model: _graph.edges
                    delegate: _crGraphEdgeDelegate
                }
            }

            Item {
                id: _nodeItems

                Repeater {
                    id: _nodeItemsRepeater
                    model: _graph.nodes
                    delegate: _crGraphNodeDelegate
                }

                Item {
                    anchors.fill: _canvas.selectedNodeItem

                    visible: _canvas.selectedNodeItem

                    BoxShadow {
                        anchors.fill: _floatingToolBarArea
                        visible: _floatingToolBarArea.visible
                    }

                    ActionManager {
                        id: _floatingToolBarActions

                        Action {
                            property bool visible: enabled

                            text: "Refresh"
                            enabled: !Scrite.document.readOnly && (_graph.character ? _graph.character === _canvas.activeCharacter : true)
                            icon.source: "qrc:/icons/navigation/refresh.png"

                            onTriggered: _graph.reset()
                        }

                        Action {
                            property bool visible: enabled

                            text: "Export to PDF"
                            enabled: !Scrite.document.readOnly && (_graph.character ? _graph.character === _canvas.activeCharacter : true)
                            icon.source: "qrc:/icons/file/generate_pdf.png"

                            onTriggered: (source) => { root.exportToPdf(source) }
                        }

                        Action {
                            property bool visible: enabled

                            text: "Add A New Relationship"
                            enabled: _graph.character && (_graph.character && _graph.character === _canvas.activeCharacter) && editRelationshipsEnabled && !Scrite.document.readOnly
                            icon.source: "qrc:/icons/content/add_circle_outline.png"

                            onTriggered: (source) => {
                                             _canvas.reloadIfDirty()
                                             root.addNewRelationshipRequest(source)
                                         }
                        }

                        Action {
                            property bool visible: enabled

                            text: _canvas.activeCharacter ? ("Remove relationship with " + _canvas.activeCharacter.name) : "Remove Relationship"
                            enabled: _graph.character && _canvas.activeCharacter !== _graph.character && _canvas.activeCharacter && editRelationshipsEnabled && !Scrite.document.readOnly
                            icon.source: "qrc:/icons/action/delete.png"

                            onTriggered: _removeRelationshipConfirmation.active = true
                        }
                    }

                    Rectangle {
                        id: _floatingToolBarArea

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.top
                        anchors.bottomMargin: _canvas.selectedNodeItem ? Math.min(_canvas.selectedNodeItem.width,_canvas.selectedNodeItem.height)*0.075+5 : 5

                        height: _floatingToolbarLayout.height + 4
                        width: _floatingToolbarLayout.width + 6

                        color: Runtime.colors.primary.windowColor
                        enabled: !_removeRelationshipConfirmation.active
                        opacity: enabled ? 1 : 0.5
                        visible: _canvas.activeCharacter

                        border.width: 1
                        border.color: Runtime.colors.primary.borderColor

                        ActionManagerToolBar {
                            id: _floatingToolbarLayout

                            anchors.centerIn: parent
                            actionManager: _floatingToolBarActions
                        }
                    }
                }

                Item {
                    anchors.fill: _canvas.selectedNodeItem
                    visible: _canvas.selectedNodeItem
                    z: 10

                    Loader {
                        id: _removeRelationshipConfirmation

                        anchors.centerIn: parent

                        active: false

                        sourceComponent: Rectangle {
                            id: _removeRelationshipConfirmationItem

                            property bool initialized: false
                            property Item currentItem: _canvas.selectedNodeItem

                            function zoom() {
                                var area = mapToItem(_canvas, 0, 0, width, height)
                                _canvasScroll.ensureVisible(area)
                                forceActiveFocus()
                                initialized = true
                            }

                            Component.onCompleted: {
                                if(_canvasScroll.zoomScale !== 1) {
                                    _canvasScroll.zoomOne()
                                    _zoomTimer.start()
                                } else
                                    zoom()
                            }

                            width: _removeRelationshipConfirmationContentLayout.width + 45
                            height: _removeRelationshipConfirmationContentLayout.height + 40

                            color: Color.translucent(Runtime.colors.primary.c600.background,0.85)
                            focus: true

                            Timer {
                                id: _zoomTimer
                                running: false
                                interval: 250
                                repeat: false

                                onTriggered: _removeRelationshipConfirmationItem.zoom()
                            }

                            MouseArea {
                                anchors.fill: parent
                            }

                            Column {
                                id: _removeRelationshipConfirmationContentLayout

                                anchors.centerIn: parent
                                anchors.verticalCenterOffset: 5

                                width: _buttonRow.width * 1.4
                                spacing: 40

                                VclLabel {
                                    width: parent.width

                                    color: Runtime.colors.primary.c600.text
                                    horizontalAlignment: Text.AlignHCenter
                                    text: "<b>Are you sure you want to delete this relationship?</b><br/><br/>NOTE: This action cannot be undone!!"
                                    wrapMode: Text.WordWrap

                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                }

                                Row {
                                    id: _buttonRow

                                    anchors.horizontalCenter: parent.horizontalCenter

                                    spacing: 20

                                    VclButton {
                                        text: "Yes"
                                        focusPolicy: Qt.NoFocus

                                        onClicked: {
                                            removeRelationshipWithRequest(_canvas.activeCharacter, this);
                                            _canvas.reloadIfDirty();
                                            _removeRelationshipConfirmation.active = false
                                        }
                                    }

                                    VclButton {
                                        text: "No"
                                        focusPolicy: Qt.NoFocus

                                        onClicked: _removeRelationshipConfirmation.active = false
                                    }
                                }
                            }

                            onCurrentItemChanged: if(initialized) _removeRelationshipConfirmation.active = false
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: _statusBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        height: 30

        clip: true
        color: Runtime.colors.primary.windowColor

        border.width: 1
        border.color: Runtime.colors.primary.borderColor

        Row {
            id: _statusBarControls

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            height: parent.height-6

            spacing: 10

            FlatToolButton {
                ToolTip.text: "Mouse wheel currently " + (checked ? "zooms" : "scrolls") + ". Click this button to make it " + (checked ? "scroll" : "zoom") + "."

                suggestedWidth: parent.height
                suggestedHeight: parent.height

                autoRepeat: false
                checkable: true
                checked: Runtime.workspaceSettings.mouseWheelZoomsInCharacterGraph
                iconSource: "qrc:/icons/hardware/mouse.png"

                onCheckedChanged: Runtime.workspaceSettings.mouseWheelZoomsInCharacterGraph = checked
            }

            FlatToolButton {
                ToolTip.text: "Zoom One"

                suggestedWidth: parent.height
                suggestedHeight: parent.height

                autoRepeat: true
                iconSource: "qrc:/icons/navigation/zoom_one.png"

                onClicked: {
                    _canvas.reloadIfDirty()

                    const item = _canvas.selectedNodeItem
                    if(item)
                        _canvasScroll.zoomOneToItem(item)
                    else
                        _canvasScroll.zoomOneMiddleArea()
                }
            }

            FlatToolButton {
                ToolTip.text: "Zoom Fit"

                suggestedWidth: parent.height
                suggestedHeight: parent.height

                autoRepeat: true
                iconSource: "qrc:/icons/navigation/zoom_fit.png"

                onClicked: { _canvas.reloadIfDirty(); _canvas.zoomFit() }
            }

            ZoomSlider {
                id: _zoomSlider

                function applyZoom() {
                    _canvasScroll.animatePanAndZoom = false
                    _canvasScroll.zoomTo(zoomLevel)
                    _canvasScroll.animatePanAndZoom = true
                }

                anchors.verticalCenter: parent.verticalCenter

                height: parent.height

                from: _canvasScroll.minimumScale
                stepSize: 0.0
                to: _canvasScroll.maximumScale
                value: _canvas.scale

                onSliderMoved: Qt.callLater(applyZoom)
                onZoomInRequest: _canvasScroll.zoomIn()
                onZoomOutRequest: _canvasScroll.zoomOut()
            }
        }
    }

    Item {
        anchors.fill: parent

        visible: _graph.dirty

        VclLabel {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 25

            width: parent.width * 0.65

            color: Runtime.colors.primary.c900.background
            text: "Graph will be refreshed when you use it next."
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }

        MouseArea {
            anchors.fill: parent

            enabled: _graph.busy
        }
    }

    BusyIcon {
        running: _graph.busy || showBusyIndicator

        anchors.centerIn: parent
    }

    Component {
        id: _crGraphNodeDelegate

        Rectangle {
            id: _nodeItem

            required property var modelData

            property CharacterRelationshipGraphNode node: modelData
            property Character character: node.character

            Component.onCompleted: {
                node.item = _nodeItem
                if(_graph.character === node.character)
                    _canvas.mainCharacterNodeItem = _nodeItem
            }

            BoundingBoxItem.evaluator: _nodeItemsBoxEvaluator

            x: node.rect.x
            y: node.rect.y
            z: _canvas.selectedNodeItem == _nodeItem ? 1 : 0
            width: node.rect.width
            height: node.rect.height

            color: character.photos.length === 0 ? Qt.tint(character.color, Runtime.colors.sceneControlTint) : Qt.rgba(0,0,0,0)

            Rectangle {
                anchors.fill: parent
                anchors.margins: -Math.min(width,height)*0.075

                color: Color.translucent(character.color, 0.15)
                radius: Math.min(width,height)*0.0375
                visible: character.photos.length > 0

                border.width: 1
                border.color: Color.translucent(character.color, 0.5)
            }

            Image {
                anchors.fill: parent

                z: character === _canvas.activeCharacter ? 1 : 0

                fillMode: Image.PreserveAspectCrop
                mipmap: true
                smooth: true

                source: {
                    if(character.hasKeyPhoto > 0)
                        return "file:///" + character.keyPhoto
                    return "qrc:/icons/content/character_icon.png"
                }

                Rectangle {
                    anchors.fill: _infoLabel
                    anchors.margins: -4

                    color: node.marked ? Runtime.colors.accent.a700.background : Qt.tint(character.color, Runtime.colors.sceneControlTint)
                    opacity: character.photos.length === 0 ? 1 : 0.8
                    radius: 4

                    border.width: 1
                    border.color: node.marked ? Runtime.colors.accent.a700.text : "black"
                }

                VclText {
                    id: _infoLabel

                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: 15

                    width: parent.width - 30

                    color: node.marked ? Runtime.colors.accent.a700.text : "black"
                    horizontalAlignment: Text.AlignHCenter
                    maximumLineCount: 3
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    font.pixelSize: 10

                    text: {
                        let fields = []
                        fields.push("<b>" + character.name + "</b>");
                        if(character.designation !== "")
                            fields.push("<i>" + character.designation + "</i>")
                        return fields.join("<br/>")
                    }
                }

                Rectangle {
                    property real alpha: {
                        if(!_canvas.activeCharacter || character === _canvas.activeCharacter)
                            return 0
                        return character.isDirectlyRelatedTo(_canvas.activeCharacter) ? 0 : 0.75
                    }

                    Behavior on alpha {
                        enabled: Runtime.applicationSettings.enableAnimations
                        NumberAnimation { duration: Runtime.stdAnimationDuration }
                    }

                    anchors.fill: parent

                    color: Qt.rgba(1,1,1,alpha)

                    border.width: character === _canvas.activeCharacter ? 3 : 1
                    border.color: character === _canvas.activeCharacter ? "black" : Runtime.colors.primary.borderColor
                }
            }

            MouseArea {
                ToolTip.text: (_graph.character && character.name === _graph.character.name) ?
                              "Double click to add a relationship to this character." :
                              "Double click to switch to " + character.name + "'s notes."
                ToolTip.delay: 1500
                ToolTip.visible: containsMouse && !_removeRelationshipConfirmation.active && !pressed

                anchors.fill: parent

                hoverEnabled: true

                drag.axis: Drag.XAndYAxis
                drag.target: !Scrite.document.readOnly ? parent : null

                onPressed: {
                    _canvasScroll.interactive = false
                    _canvas.selectedNodeItem = parent
                    _canvas.reloadIfDirty()
                }
                onReleased: _canvasScroll.interactive = true
                onDoubleClicked: characterDoubleClicked(character.name, parent)
            }
        }
    }

    Component {
        id: _crGraphEdgeDelegate

        PainterPathItem {
            required property var modelData

            property string pathString: modelData.pathString

            Behavior on opacity {
                enabled: Runtime.applicationSettings.enableAnimations
                NumberAnimation { duration: Runtime.stdAnimationDuration }
            }

            z: opacity

            outlineColor: Runtime.colors.primary.c700.background
            outlineWidth: Scrite.app.devicePixelRatio * _canvas.scale * Runtime.structureCanvasSettings.lineWidthOfConnectors
            renderType: PainterPathItem.OutlineOnly
            renderingMechanism: PainterPathItem.UseOpenGL

            opacity: {
                if(_canvas.activeCharacter)
                    return (modelData.relationship.of === _canvas.activeCharacter || modelData.relationship.withCharacter === _canvas.activeCharacter) ? 1 : 0.2
                return 1
            }

            Rectangle {
                x: modelData.labelPosition.x - width/2
                y: modelData.labelPosition.y - height/2
                width: _nameLabel.width + 10
                height: _nameLabel.height + 4

                color: _nameLabelMouseArea.containsMouse ? Runtime.colors.accent.c700.background : Runtime.colors.primary.c700.background
                rotation: modelData.labelAngle

                VclLabel {
                    id: _nameLabel

                    anchors.centerIn: parent

                    color: _nameLabelMouseArea.containsMouse ? Runtime.colors.accent.c700.text : Runtime.colors.primary.c700.text
                    horizontalAlignment: Text.AlignHCenter
                    text: modelData.relationship.name

                    font.pointSize: Math.floor(Runtime.idealFontMetrics.font.pointSize*0.75)
                }

                MouseArea {
                    id: _nameLabelMouseArea

                    anchors.fill: parent

                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    enabled: !Scrite.document.readOnly
                    hoverEnabled: enabled

                    onClicked: RelationshipNameEditorDialog.launch(modelData.relationship)
                }
            }

            onPathStringChanged: setPathFromString(pathString)
        }
    }

    onVisibleChanged: {
        if(visible) {
            _canvas.reloadIfDirty()
            _canvas.zoomFit()
            _canvas.selectedNodeItem = _canvas.mainCharacterNodeItem
        }
    }
}


