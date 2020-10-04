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

    CharacterRelationshipsGraph {
        id: crgraph
        structure: scriteDocument.loading ? null : scriteDocument.structure
        nodeSize: Qt.size(150,150)
        maxTime: 1000
        maxIterations: 5000
        leftMargin: 1000
        topMargin: 1000
        onUpdated: app.execLater(crgraph, 250, function() {
            canvas.zoomFit()
        })
    }

    onVisibleChanged: {
        if(visible)
            canvas.zoomFit()
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

        Item {
            id: canvas
            width: 120000
            height: 120000
            transformOrigin: Item.TopLeft
            scale: scrollArea.suggestedScale

            MouseArea {
                anchors.fill: parent
                enabled: canvas.selectedNodeItem !== null
                onClicked: canvas.selectedNodeItem = null
            }

            property Character activeCharacter: selectedNodeItem ? selectedNodeItem.character : null
            property Item selectedNodeItem

            Item {
                id: nodeItemsBox
                x: crgraph.nodes.objectCount > 0 ? (nodeItemsBoxEvaluator.boundingBox.x - 50) : 0
                y: crgraph.nodes.objectCount > 0 ? nodeItemsBoxEvaluator.boundingBox.y - 50 : 0
                width: crgraph.nodes.objectCount > 0 ? (nodeItemsBoxEvaluator.boundingBox.width + 100) : Math.floor(Math.min(scrollArea.width,scrollArea.height)/100)*100
                height: crgraph.nodes.objectCount > 0 ? (nodeItemsBoxEvaluator.boundingBox.height + 100) : width
            }

            TightBoundingBoxEvaluator {
                id: nodeItemsBoxEvaluator
            }

            function zoomFit() {
                scrollArea.zoomFit(Qt.rect(nodeItemsBox.x,nodeItemsBox.y,nodeItemsBox.width,nodeItemsBox.height))
            }

            Item {
                id: edgeItems

                Repeater {
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
                            color: primaryColors.c700.background

                            Text {
                                id: nameLabel
                                text: {
                                    if(modelData.forwardLabel === "" && modelData.reverseLabel === "")
                                        return "Related To"
                                    if(modelData.forwardLabel === modelData.reverseLabel || modelData.reverseLabel === "")
                                        return modelData.forwardLabel
                                    if(modelData.forwardLabel === "")
                                        return modelData.reverseLabel
                                    return modelData.forwardLabel + "<br/>" + modelData.reverseLabel
                                }
                                font.pointSize: app.idealFontPointSize
                                anchors.centerIn: parent
                                color: primaryColors.c700.text
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
            }

            Item {
                id: nodeItems

                Repeater {
                    model: crgraph.nodes

                    Rectangle {
                        property Character character: modelData.character
                        x: modelData.rect.x
                        y: modelData.rect.y
                        width: modelData.rect.width
                        height: modelData.rect.height
                        color: character.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                        Component.onCompleted: modelData.item = this

                        TightBoundingBoxItem.evaluator: nodeItemsBoxEvaluator

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
                                width: Math.min(parent.width - 30, contentWidth)
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
                            drag.target: parent
                            drag.axis: Drag.XAndYAxis
                            onPressed: {
                                scrollArea.interactive = false
                                if(canvas.selectedNodeItem === parent)
                                    canvas.selectedNodeItem = null
                                else
                                    canvas.selectedNodeItem = parent
                            }
                            onReleased: scrollArea.interactive = true
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
        color: primaryColors.c100.background
        border.color: primaryColors.c100.text
        border.width: 1
        radius: 6

        Row {
            id: floatingToolbarLayout
            anchors.centerIn: parent
            spacing: 5

            ToolButton3 {
                onClicked: scrollArea.zoomIn()
                iconSource: "../icons/navigation/zoom_in.png"
                autoRepeat: true
                ToolTip.text: "Zoom In"
            }

            ToolButton3 {
                onClicked: scrollArea.zoomOut()
                iconSource: "../icons/navigation/zoom_out.png"
                autoRepeat: true
                ToolTip.text: "Zoom Out"
            }

            ToolButton3 {
                onClicked: {
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
                onClicked: canvas.zoomFit()
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
            }
        }
    }
}
