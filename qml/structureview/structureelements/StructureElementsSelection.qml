/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

AbstractSelection {
    id: root

    signal zoomOneRequest()
    signal deleteElementsRequest(var elementList)
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
            Runtime.execLater(_selection, 1000, function() {
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

    function confirmDelete() {
        const what = Scrite.document.structure.canvasUIMode === Structure.IndexCardUI ? " index cards" : " scenes"
        MessageBox.question("Delete Confirmation",
                            "Are you sure you want to delete the selelected " + (items.length) + what,
                            ["Yes", "No"],
                            (answer) => {
                                if(answer === "Yes") {
                                    _private.deleteSelection()
                                }
                            })
    }

    contextMenu: SelectionContextMenu {
        selection: root

        onDeleteSelectionRequest: () => {
                                      root.confirmDelete()
                                  }

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

        function deleteSelection() {
            let elements = []
            for(let i=0; i<root.items.length; i++) {
                elements.push( root.items[i].element )
            }

            root.clear()
            root.deleteElementsRequest(elements)
        }
    }
}

