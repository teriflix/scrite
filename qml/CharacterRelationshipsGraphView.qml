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
    property alias scene: crgraph.scene
    property alias character: crgraph.character
    property bool editRelationshipsEnabled: false

    signal characterDoubleClicked(string characterName)
    signal addNewRelationshipRequest(Item sourceItem)
    signal removeRelationshipWithRequest(Character otherCharacter, Item sourceItem)

    CharacterRelationshipsGraph {
        id: crgraph
        structure: scriteDocument.loading ? null : scriteDocument.structure
        nodeSize: Qt.size(150,150)
        maxTime: notebookSettings.graphLayoutMaxTime
        maxIterations: notebookSettings.graphLayoutMaxIterations
        leftMargin: 1000
        topMargin: 1000
        onUpdated: app.execLater(crgraph, 250, function() {
            canvas.zoomFit()
            canvas.selectedNodeItem = canvas.mainCharacterNodeItem
        })
    }

    onVisibleChanged: {
        if(visible) {
            canvas.reloadIfDirty()
            canvas.zoomFit()
            canvas.selectedNodeItem = canvas.mainCharacterNodeItem
        }
    }

    ScrollArea {
        id: scrollArea
        clip: true
        anchors.fill: parent
        contentWidth: canvas.width * canvas.scale
        contentHeight: canvas.height * canvas.scale
        initialContentWidth: canvas.width
        initialContentHeight: canvas.height
        showScrollBars: true
        handlePinchZoom: true
        zoomOnScroll: workspaceSettings.mouseWheelZoomsInCharacterGraph

        Item {
            id: canvas
            width: 120000
            height: 120000
            transformOrigin: Item.TopLeft
            scale: scrollArea.suggestedScale

            MouseArea {
                anchors.fill: parent
                enabled: canvas.selectedNodeItem !== null || crgraph.dirty
                onClicked: { canvas.selectedNodeItem = null; canvas.reloadIfDirty(); }
            }

            property Character activeCharacter: selectedNodeItem ? selectedNodeItem.character : null
            property Item selectedNodeItem
            property Item mainCharacterNodeItem

            Rectangle {
                id: nodeItemsBox
                x: crgraph.nodes.objectCount > 0 ? (nodeItemsBoxEvaluator.boundingBox.x - 50) : 0
                y: crgraph.nodes.objectCount > 0 ? nodeItemsBoxEvaluator.boundingBox.y - 50 : 0
                width: crgraph.nodes.objectCount > 0 ? (nodeItemsBoxEvaluator.boundingBox.width + 100) : Math.floor(Math.min(scrollArea.width,scrollArea.height)/100)*100
                height: crgraph.nodes.objectCount > 0 ? (nodeItemsBoxEvaluator.boundingBox.height + 100) : width
                color: Qt.rgba(0,0,0,0)
                border.width: crgraph.nodes.objectCount > 0 ? 1 : 0
                border.color: primaryColors.borderColor
                radius: 6
            }

            BoundingBoxEvaluator {
                id: nodeItemsBoxEvaluator
            }

            function reloadIfDirty() {
                if(crgraph.dirty)
                    crgraph.reload()
            }

            function zoomFit() {
                if(nodeItemsBox.width > scrollArea.width || nodeItemsBox.height > scrollArea.height)
                    scrollArea.zoomFit(Qt.rect(nodeItemsBox.x,nodeItemsBox.y,nodeItemsBox.width,nodeItemsBox.height))
                else {
                    var centerX = nodeItemsBox.x + nodeItemsBox.width/2
                    var centerY = nodeItemsBox.y + nodeItemsBox.height/2
                    var w = scrollArea.width
                    var h = scrollArea.height
                    var x = centerX - w/2
                    var y = centerY - h/2
                    scrollArea.zoomFit(Qt.rect(x,y,w,h))
                }
            }

            Item {
                id: edgeItems

                Repeater {
                    id: edgeItemsRepeater
                    model: crgraph.edges

                    PainterPathItem {
                        outlineWidth: app.devicePixelRatio*canvas.scale*structureCanvasSettings.connectorLineWidth
                        outlineColor: primaryColors.c700.background
                        renderType: PainterPathItem.OutlineOnly
                        renderingMechanism: PainterPathItem.UseOpenGL
                        opacity: {
                            if(canvas.activeCharacter)
                                return (modelData.relationship.of === canvas.activeCharacter || modelData.relationship.withCharacter === canvas.activeCharacter) ? 1 : 0.2
                            return 1
                        }
                        z: opacity
                        Behavior on opacity {
                            enabled: screenplayEditorSettings.enableAnimations
                            NumberAnimation { duration: 250 }
                        }

                        property string pathString: modelData.pathString
                        onPathStringChanged: setPathFromString(pathString)

                        Rectangle {
                            x: modelData.labelPosition.x - width/2
                            y: modelData.labelPosition.y - height/2
                            rotation: modelData.labelAngle
                            width: nameLabel.width + 10
                            height: nameLabel.height + 4
                            color: nameLabelMouseArea.containsMouse ? accentColors.c700.background : primaryColors.c700.background

                            Text {
                                id: nameLabel
                                text: modelData.relationship.name
                                font.pointSize: Math.floor(app.idealFontPointSize*0.75)
                                anchors.centerIn: parent
                                color: nameLabelMouseArea.containsMouse ? accentColors.c700.text : primaryColors.c700.text
                                horizontalAlignment: Text.AlignHCenter
                            }

                            MouseArea {
                                id: nameLabelMouseArea
                                hoverEnabled: enabled
                                anchors.fill: parent
                                enabled: !scriteDocument.readOnly
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: {
                                    modalDialog.closeable = false
                                    modalDialog.popupSource = parent
                                    modalDialog.initItemCallback = function(item) {
                                        item.relationship = modelData.relationship
                                    }
                                    modalDialog.sourceComponent = relationshipNameEditorDialog
                                    modalDialog.active = true
                                }
                            }
                        }
                    }
                }
            }

            Item {
                id: nodeItems

                BoxShadow {
                    anchors.fill: canvas.selectedNodeItem
                    visible: canvas.selectedNodeItem !== null
                }

                Repeater {
                    id: nodeItemsRepeater
                    model: crgraph.nodes

                    Rectangle {
                        id: nodeItem
                        property Character character: modelData.character
                        x: modelData.rect.x
                        y: modelData.rect.y
                        width: modelData.rect.width
                        height: modelData.rect.height
                        color: character.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                        Component.onCompleted: {
                            modelData.item = nodeItem
                            if(crgraph.character === modelData.character)
                                canvas.mainCharacterNodeItem = nodeItem
                        }

                        BoundingBoxItem.evaluator: nodeItemsBoxEvaluator

                        Image {
                            anchors.fill: parent
                            source: {
                                if(character.photos.length > 0)
                                    return "file:///" + character.photos[0]
                                return "../icons/content/character_icon.png"
                            }
                            fillMode: Image.PreserveAspectCrop
                            mipmap: true; smooth: true
                            z: character === canvas.activeCharacter ? 1 : 0

                            Rectangle {
                                anchors.fill: infoLabel
                                anchors.margins: -4
                                radius: 4
                                color: modelData.marked ? accentColors.a700.background : "white"
                                opacity: character.photos.length === 0 ? 1 : 0.8
                                border.width: 1
                                border.color: modelData.marked ? accentColors.a700.text : "black"
                            }

                            Text {
                                id: infoLabel
                                width: parent.width - 30
                                anchors.bottom: parent.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottomMargin: 15
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                horizontalAlignment: Text.AlignHCenter
                                font.pixelSize: 10
                                maximumLineCount: 3
                                color: modelData.marked ? accentColors.a700.text : "black"
                                text: {
                                    var fields = []
                                    fields.push("<b>" + character.name + "</b>");
                                    if(character.designation !== "")
                                        fields.push("<i>" + character.designation + "</i>")
                                    return fields.join("<br/>")
                                }
                            }

                            Rectangle {
                                anchors.fill: parent
                                border.width: character === canvas.activeCharacter ? 3 : 1
                                border.color: character === canvas.activeCharacter ? "black" : primaryColors.borderColor
                                color: Qt.rgba(1,1,1,alpha)
                                property real alpha: {
                                    if(canvas.activeCharacter === null || character === canvas.activeCharacter)
                                        return 0
                                    return character.isDirectlyRelatedTo(canvas.activeCharacter) ? 0 : 0.75
                                }
                                Behavior on alpha {
                                    enabled: screenplayEditorSettings.enableAnimations
                                    NumberAnimation { duration: 250 }
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            drag.target: !scriteDocument.readOnly ? parent : null
                            drag.axis: Drag.XAndYAxis
                            hoverEnabled: true
                            onPressed: {
                                scrollArea.interactive = false
                                canvas.selectedNodeItem = parent
                                canvas.reloadIfDirty()
                            }
                            onReleased: scrollArea.interactive = true
                            onDoubleClicked: characterDoubleClicked(character.name)
                            ToolTip.text: "Double click to switch to " + character.name + " tab"
                            ToolTip.delay: 1500
                            ToolTip.visible: containsMouse
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: floatingToolbar
        width: floatingToolbarLayout.width + 10
        height: floatingToolbarLayout.height + 10
        anchors.top: scrollArea.top
        anchors.left: scrollArea.left
        anchors.margins: 20
        color: crgraph.dirty ? primaryColors.c200.background : primaryColors.c100.background
        border.color: primaryColors.c100.text
        border.width: 1
        radius: 6

        Row {
            id: floatingToolbarLayout
            anchors.centerIn: parent
            spacing: 5

            ToolButton3 {
                iconSource: "../icons/hardware/mouse.png"
                autoRepeat: false
                ToolTip.text: "Mouse wheel currently " + (checked ? "zooms" : "scrolls") + ". Click this button to make it " + (checked ? "scroll" : "zoom") + "."
                checkable: true
                checked: workspaceSettings.mouseWheelZoomsInCharacterGraph
                onCheckedChanged: workspaceSettings.mouseWheelZoomsInCharacterGraph = checked
            }

            ToolButton3 {
                onClicked: { canvas.reloadIfDirty(); scrollArea.zoomIn() }
                iconSource: "../icons/navigation/zoom_in.png"
                autoRepeat: true
                ToolTip.text: "Zoom In"
            }

            ToolButton3 {
                onClicked: { canvas.reloadIfDirty(); scrollArea.zoomOut() }
                iconSource: "../icons/navigation/zoom_out.png"
                autoRepeat: true
                ToolTip.text: "Zoom Out"
            }

            ToolButton3 {
                onClicked: {
                    canvas.reloadIfDirty()
                    var item = canvas.selectedNodeItem
                    if(item === null)
                        canvasScroll.zoomOneMiddleArea()
                    else
                        canvasScroll.zoomOneToItem(item)
                }
                iconSource: "../icons/navigation/zoom_one.png"
                autoRepeat: true
                ToolTip.text: "Zoom One"
            }

            ToolButton3 {
                onClicked: { canvas.reloadIfDirty(); canvas.zoomFit() }
                iconSource: "../icons/navigation/zoom_fit.png"
                autoRepeat: true
                ToolTip.text: "Zoom Fit"
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
                opacity: 0.5
            }

            ToolButton3 {
                onClicked: crgraph.reset()
                iconSource: "../icons/navigation/refresh.png"
                autoRepeat: true
                ToolTip.text: "Refresh"
                visible: !scriteDocument.readOnly
            }

            Rectangle {
                width: 1
                height: parent.height
                color: primaryColors.separatorColor
                opacity: 0.5
                visible: editRelationshipsEnabled && !scriteDocument.readOnly
            }

            ToolButton3 {
                onClicked: { canvas.reloadIfDirty(); addNewRelationshipRequest(this) }
                iconSource: "../icons/content/add_circle_outline.png"
                autoRepeat: false
                ToolTip.text: "Add A New Relationship"
                enabled: crgraph.character !== null && editRelationshipsEnabled && !scriteDocument.readOnly
                visible: crgraph.character !== null && editRelationshipsEnabled && !scriteDocument.readOnly
            }

            ToolButton3 {
                onClicked: { canvas.reloadIfDirty(); removeRelationshipWithRequest(canvas.activeCharacter, this) }
                iconSource: "../icons/action/delete.png"
                autoRepeat: false
                ToolTip.text: canvas.activeCharacter ? ("Remove relationship with " + canvas.activeCharacter.name) : "Remove Relationship"
                enabled: crgraph.character !== null && canvas.activeCharacter !== crgraph.character && canvas.activeCharacter && editRelationshipsEnabled && !scriteDocument.readOnly
                visible: crgraph.character !== null && canvas.activeCharacter !== crgraph.character && canvas.activeCharacter && editRelationshipsEnabled && !scriteDocument.readOnly
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: crgraph.dirty

        Text {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 25
            font.pointSize: app.idealFontPointSize
            width: parent.width * 0.65
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: "Graph will be refreshed when you use it next."
            color: primaryColors.c900.background
        }

        MouseArea {
            anchors.fill: parent
            enabled: crgraph.busy
        }

        BusyIndicator {
            running: crgraph.busy
        }
    }

    Component {
        id: relationshipNameEditorDialog

        Rectangle {
            property Relationship relationship
            property Character ofCharacter: relationship.direction === Relationship.OfWith ? relationship.ofCharacter : relationship.withCharacter
            property Character withCharacter: relationship.direction === Relationship.OfWith ? relationship.withCharacter : relationship.ofCharacter
            width: 800
            height: dialogLayout.height + 50

            Component.onCompleted: {
                app.execLater(dialogLayout, 100, function() {
                    txtRelationshipName.forceActiveFocus()
                })
            }

            Column {
                id: dialogLayout
                spacing: 30
                width: parent.width - 50
                anchors.centerIn: parent

                Text {
                    font.pointSize: app.idealFontPointSize + 4
                    text: "Edit Relationship"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Column {
                        spacing: 10
                        width: 180

                        Rectangle {
                            width: 150; height: 150
                            color: ofCharacter.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                            anchors.horizontalCenter: parent.horizontalCenter
                            border.width: 1
                            border.color: "black"

                            Image {
                                anchors.fill: parent
                                source: {
                                    if(ofCharacter.photos.length > 0)
                                        return "file:///" + ofCharacter.photos[0]
                                    return "../icons/content/character_icon.png"
                                }
                                fillMode: Image.PreserveAspectCrop
                                mipmap: true; smooth: true
                            }
                        }

                        Text {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            maximumLineCount: 2
                            elide: Text.ElideRight
                            text: app.camelCased(ofCharacter.name)
                            font.pointSize: app.idealFontPointSize
                        }
                    }

                    TextField2 {
                        id: txtRelationshipName
                        anchors.verticalCenter: parent.verticalCenter
                        text: relationship.name
                        label: "Relationship:"
                        font.pointSize: app.idealFontPointSize
                        placeholderText: "husband of, wife of, friends with, reports to ..."
                        maximumLength: 50
                        width: 300
                        enableTransliteration: true
                        readOnly: scriteDocument.readOnly
                        onReturnPressed: doneButton.click()
                        focus: true
                    }

                    Column {
                        spacing: 10
                        width: 180

                        Rectangle {
                            width: 150; height: 150
                            color: withCharacter.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                            anchors.horizontalCenter: parent.horizontalCenter
                            border.width: 1
                            border.color: "black"

                            Image {
                                anchors.fill: parent
                                anchors.margins: 1
                                source: {
                                    if(withCharacter.photos.length > 0)
                                        return "file:///" + withCharacter.photos[0]
                                    return "../icons/content/character_icon.png"
                                }
                                fillMode: Image.PreserveAspectCrop
                                mipmap: true; smooth: true
                            }
                        }

                        Text {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            maximumLineCount: 2
                            elide: Text.ElideRight
                            text: app.camelCased(withCharacter.name)
                            font.pointSize: app.idealFontPointSize
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: Math.max(revertButton.height, doneButton.height)

                    Button {
                        id: revertButton
                        text: "Revert"
                        enabled: txtRelationshipName.text !== relationship.name
                        onClicked: {
                            txtRelationshipName.text = relationship.name
                        }
                        anchors.left: parent.left
                    }

                    Button {
                        id: doneButton
                        text: revertButton.enabled ? "Change" : "Ok"
                        enabled: txtRelationshipName.length > 0
                        onClicked: click()
                        function click() {
                            relationship.name = txtRelationshipName.text.trim()
                            modalDialog.close()
                        }
                        anchors.right: parent.right
                    }
                }
            }
        }
    }
}


