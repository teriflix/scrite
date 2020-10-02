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
    CharacterRelationshipsGraph {
        id: crgraph
        structure: scriteDocument.loading ? null : scriteDocument.structure
        nodeSize: Qt.size(150,150)
        maxTime: 1000
        maxIterations: -1
        onUpdated: app.execLater(crgraph, 100, function() {
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

        GridBackground {
            id: canvas
            width: 120000
            height: 120000
            antialiasing: false
            tickColorOpacity: 0.25 * scale
            majorTickLineWidth: 2*app.devicePixelRatio
            minorTickLineWidth: 1*app.devicePixelRatio
            gridIsVisible: structureCanvasSettings.showGrid
            majorTickColor: structureCanvasSettings.gridColor
            minorTickColor: structureCanvasSettings.gridColor
            tickDistance: scriteDocument.structure.canvasGridSize
            transformOrigin: Item.TopLeft
            scale: scrollArea.suggestedScale

            function zoomFit() {
                scrollArea.zoomFit( Qt.rect(graphArea.x, graphArea.y, graphArea.width, graphArea.height) )
            }

            MouseArea {
                anchors.fill: parent
                enabled: graphArea.selectedNodeItem !== null
                onClicked: graphArea.selectedNodeItem = null
            }

            Item {
                id: graphArea
                anchors.centerIn: parent
                width: crgraph.graphBoundingRect.width
                height: crgraph.graphBoundingRect.height

                property Character activeCharacter: selectedNodeItem ? selectedNodeItem.character : null
                property Item selectedNodeItem

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
                                if(graphArea.activeCharacter)
                                    return (modelData.relationship.of === graphArea.activeCharacter || modelData.relationship.withCharacter === graphArea.activeCharacter) ? 1 : 0.2
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
                                    text: modelData.relationship.name
                                    font.pointSize: app.idealFontPointSize
                                    anchors.centerIn: parent
                                    color: primaryColors.c700.text
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

                            Image {
                                anchors.fill: parent
                                source: {
                                    if(character.photos.length > 0)
                                        return "file:///" + character.photos[0]
                                    return "../icons/content/character_icon.png"
                                }
                                fillMode: Image.PreserveAspectCrop
                                mipmap: true; smooth: true
                                z: character === graphArea.activeCharacter ? 1 : 0

                                Rectangle {
                                    anchors.fill: infoLabel
                                    anchors.margins: -4
                                    radius: 4
                                    color: "white"
                                    opacity: character.photos.length === 0 ? 1 : 0.8
                                    border.width: 1
                                    border.color: "black"
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
                                    border.width: character === graphArea.activeCharacter ? 3 : 1
                                    border.color: character === graphArea.activeCharacter ? "black" : primaryColors.borderColor
                                    color: Qt.rgba(1,1,1,alpha)
                                    property real alpha: {
                                        if(graphArea.activeCharacter === null || character === graphArea.activeCharacter)
                                            return 0
                                        return character.isRelatedTo(graphArea.activeCharacter) ? 0 : 0.75
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
                                onClicked: {
                                    if(graphArea.selectedNodeItem === parent)
                                        graphArea.selectedNodeItem = null
                                    else
                                        graphArea.selectedNodeItem = parent
                                }
                            }
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
                    var item = graphArea.selectedNodeItem
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
