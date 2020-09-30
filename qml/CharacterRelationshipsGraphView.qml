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
import Scrite 1.0

Item {
    CharacterRelationshipsGraph {
        id: crgraph
        structure: scriteDocument.loading ? null : scriteDocument.structure
        nodeSize: Qt.size(350,100)
        maxTime: 500
    }

    ScrollArea {
        id: scrollArea
        anchors.fill: parent
        contentWidth: scrollAreaItem.width
        contentHeight: scrollAreaItem.height
        handlePinchZoom: true

        Item {
            id: scrollAreaItem
            width: Math.max(crgraph.graphBoundingRect.width, 1000)
            height: Math.max(crgraph.graphBoundingRect.height, 1000)
            scale: scrollArea.suggestedScale

            Repeater {
                model: crgraph.edges

                PainterPathItem {
                    outlineWidth: 3
                    outlineColor: "black"
                    renderType: PainterPathItem.OutlineOnly
                    renderingMechanism: PainterPathItem.UseOpenGL

                    property string pathString: modelData.pathString
                    onPathStringChanged: setPathFromString(pathString)
                }
            }

            Repeater {
                model: crgraph.nodes

                CharacterBox {
                    character: modelData.character
                    x: modelData.rect.x
                    y: modelData.rect.y
                    width: modelData.rect.width
                    height: modelData.rect.height
                }
            }
        }
    }
}
