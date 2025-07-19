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

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

VclMenu {
    id: root

    required property AbstractSelection selection

    signal ensureItemVisibleRequest(Item item)
    signal rectangleAnnotationRequest(real x, real y, real w, real h)

    width: 250

    ColorMenu {
        title: "Scenes Color"

        onMenuItemClicked: {
            let items = selection.items
            items.forEach( function(item) {
                item.element.scene.color = color
            })
            root.close()
        }
    }

    MarkSceneAsMenu {
        title: "Mark Scenes As"
        enableValidation: false

        onTriggered: {
            let items = selection.items
            items.forEach( function(item) {
                item.element.scene.type = type
            })
            root.close()
        }
    }

    VclMenu {
        title: "Layout"

        VclMenuItem {
            text: "Layout Horizontally"
            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
            icon.source: "qrc:/icons/action/layout_horizontally.png"

            onClicked: selection.layout(Structure.HorizontalLayout)
        }

        VclMenuItem {
            icon.source: "qrc:/icons/action/layout_vertically.png"
            text: "Layout Vertically"
            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout

            onClicked: selection.layout(Structure.VerticalLayout)
        }

        VclMenuItem {
            icon.source: "qrc:/icons/action/layout_flow_horizontally.png"
            text: "Flow Horizontally"
            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout

            onClicked: selection.layout(Structure.FlowHorizontalLayout)
        }

        VclMenuItem {
            text: "Flow Vertically"
            enabled: !Scrite.document.readOnly && (selection.hasItems ? selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
            icon.source: "qrc:/icons/action/layout_flow_vertically.png"

            onClicked: selection.layout(Structure.FlowVerticalLayout)
        }
    }

    VclMenuItem {
        text: "Annotate With Rectangle"

        onClicked: {
            root.rectangleAnnotationRequest(selection.rect.x-10, selection.rect.y-10, selection.rect.width+20, selection.rect.height+20)
            selection.clear()
        }
    }

    VclMenuItem {
        text: "Stack"

        enabled: {
            if(Scrite.document.structure.canvasUIMode !== Structure.IndexCardUI)
                return false

            let items = selection.items
            let actIndex = -1
            for(let i=0; i<items.length; i++) {
                let item = items[i]
                if(item.element.stackId !== "")
                    return false

                if(i === 0)
                    actIndex = item.element.scene.actIndex
                else if(actIndex !== item.element.scene.actIndex)
                    return false
            }

            if(actIndex < 0)
                return false

            return true
        }

        onTriggered: {
            let items = selection.items
            let id = Scrite.app.createUniqueId()
            items.forEach( function(item) {
                item.element.stackId = id
            })
            selection.clear()
        }
    }

    VclMenuItem {
        text: "Add To Timeline"

        onClicked: {
            let items = selection.items
            items.forEach( function(item) {
                Scrite.document.screenplay.addScene(item.element.scene)
            })
        }
    }

    VclMenuItem {
        text: "Remove From Timeline"

        enabled: {
            if(!selection.hasItems)
                return false
            let items = selection.items
            for(var i=0; i<items.length; i++) {
                if(items[i].element.scene.addedToScreenplay)
                    continue
                return false
            }
            return true
        }

        onClicked: {
            let items = selection.items
            let firstItem = items[0]
            items.forEach( function(item) {
                Scrite.document.screenplay.removeSceneElements(item.element.scene)
            })
            selection.clear()
            root.ensureItemVisibleRequest(firstItem)
        }
    }

    StructureGroupsMenu {
        sceneGroup: SceneGroup {
            structure: Scrite.document.structure
        }

        onClosed: sceneGroup.clearScenes()

        onToggled: Utils.execLater(selection, 250, function() { selection.refit() })

        onAboutToShow: {
            sceneGroup.clearScenes()

            let items = selection.items
            items.forEach( function(item) {
                sceneGroup.addScene(item.element.scene)
            })
        }
    }
}
