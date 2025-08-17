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

import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/helpers"
import "qrc:/qml/controls"

AbstractSelection {
    id: root

    signal zoomOneRequest()
    signal denyCanvasPreviewRequest()
    signal allowCanvasPreviewRequest()
    signal ensureItemVisibleRequest(Item item)
    signal rectangleAnnotationRequest(real x, real y, real w, real h)
    signal initiateSelectionInBoundaryRequest(var boundary)

    function layout(type) {
        if(Scrite.document.readOnly || Scrite.document.structure.forceBeatBoardLayout)
            return

        if(!hasItems) {
            root.denyCanvasPreviewRequest()
            const rect = Scrite.document.structure.layoutElements(type)
            Utils.execLater(_selection, 1000, function() {
                root.zoomOneRequest()
                root.allowCanvasPreviewRequest()
            })
            return
        }

        if(!canLayout)
            return

        _private.layoutAnimation.layoutType = type
        _private.layoutAnimation.start()
    }

    interactive: !Scrite.document.readOnly && !Scrite.document.structure.forceBeatBoardLayout

    contextMenu: SelectionContextMenu {
        selection: root

        onEnsureItemVisibleRequest: (item) => {
                                        root.ensureItemVisibleRequest(item)
                                    }

        onRectangleAnnotationRequest: (x, y, w, h) => {
                                          root.rectangleAnnotationRequest(x, y, w, h)
                                      }
    }

    onMoveItem: {
        item.x = item.x + dx
        item.y = item.y + dy
    }

    onPlaceItem: {
        item.x = Scrite.document.structure.snapToGrid(item.x)
        item.y = Scrite.document.structure.snapToGrid(item.y)
    }

    QtObject {
        id: _private

        readonly property SequentialAnimation layoutAnimation: StructureElementsLayoutTask {
            onClearSelectionRequest: root.clear()
            onDenyCanvasPreviewRequest: root.denyCanvasPreviewRequest()
            onAllowCanvasPreviewRequest: root.allowCanvasPreviewRequest()
            onSelectItemsInBoundaryRequest: (boundary) => {
                                                 root.initiateSelectionInBoundaryRequest(boundary)
                                             }
        }
    }
}

