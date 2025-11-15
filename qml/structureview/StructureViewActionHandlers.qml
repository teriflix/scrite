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
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/overlays"
import "qrc:/qml/structureview/structureelements"

Item {
    id: root

    required property StructureCanvasPreview canvasPreview
    required property StructureCanvasScrollArea canvasScroll

    property alias newSceneColor: _newSceneHandler.activeColor
    property alias selectionMode: _selectionModeHandler.checked

    signal zoomOneRequest()
    signal newSceneRequest()
    signal selectAllRequest()
    signal newAnnotationRequest(string annotationType)
    signal groupCategoryRequest(string groupCategory)
    signal newColoredSceneRequest(color sceneColor)
    signal deleteElementRequest(StructureElement element)
    signal selectionLayoutRequest(int layout) // here layout should be one of Structure.LayoutType which are
                                              // Structure.HorizontalLayout, Structure.VerticalLayout,
                                              // Structure.FlowHorizontalLayout, Structure.FlowVerticalLayout

    ActionHandler {
        id: _newSceneHandler

        property color activeColor: "white"

        action: ActionHub.structureCanvasOperations.find("newScene")
        down: _newSceneMenu.visible
        enabled: !Scrite.document.readOnly

        onTriggered: (source) => {
                         _newSceneMenu.popup(source)
                     }

        VclMenu {
            id: _newSceneMenu

            VclMenuItem {
                text: "New Scene"
                enabled: !Scrite.document.readOnly

                onClicked: () => {
                               Qt.callLater( _newSceneMenu.close )
                               root.newSceneRequest()
                           }
            }

            ColorMenu {
                title: "Colored Scene"
                enabled: !Scrite.document.readOnly
                selectedColor: _newSceneHandler.activeColor

                onMenuItemClicked: (color) => {
                                       Qt.callLater( _newSceneMenu.close )
                                       _newSceneHandler.activeColor = color
                                       root.newColoredSceneRequest( _newSceneHandler.activeColor)
                                   }
            }
        }
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("newAnnotation")
        down: _newAnnotationMenu.visible
        enabled: !Scrite.document.readOnly

        onTriggered: (source) => {
                         _newAnnotationMenu.popup(source)
                     }

        VclMenu {
            id: _newAnnotationMenu

            Repeater {
                model: root.canvasScroll.availableAnnotationKeys

                VclMenuItem {
                    required property var modelData

                    text: modelData.title
                    enabled: !Scrite.document.readOnly && modelData.what !== ""

                    onClicked: () => {
                                   Qt.callLater( _newAnnotationMenu.close )
                                   root.newAnnotationRequest(modelData.what)
                               }
                }
            }
        }
    }

    ActionHandler {
        id: _selectionModeHandler

        action: ActionHub.structureCanvasOperations.find("selectionMode")
        enabled: !Scrite.document.readOnly && (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.canLayout : Scrite.document.structure.elementCount >= 2)

        onTriggered: root.selectionLayoutRequest(Structure.HorizontalLayout)
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("selectAll")
        enabled: !Scrite.document.readOnly && Scrite.document.structure.elementCount >= 2

        onTriggered: root.selectAllRequest()
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("layout")
        down: _layoutOptionsMenu.visible
        enabled: !Scrite.document.readOnly && (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout

        onTriggered: (source) => {
                         _layoutOptionsMenu.popup(source)
                     }

        VclMenu {
            id: _layoutOptionsMenu
            width: 250

            Repeater {
                model: [
                    { "text": "Layout Horizonatally", "icon": "layout_horizontally.png", "type": Structure.HorizontalLayout },
                    { "text": "Layout Vertically", "icon": "layout_vertically.png", "type": Structure.VerticalLayout },
                    { "text": "Flow Horizontally", "icon": "layout_flow_horizontally.png", "type": Structure.FlowHorizontalLayout },
                    { "text": "Flow Vertically", "icon": "layout_flow_vertically.png", "type": Structure.FlowVerticalLayout }
                ]

                VclMenuItem {
                    required property var modelData

                    text: modelData.text
                    icon.source: "qrc:/icons/action/" + modelData.icon

                    onClicked: selectionLayoutRequest(modelData.type)
                }
            }
        }
    }

    ActionHandler {
        id: _beatBoardLayoutActionHandler

        Component.onCompleted: checked = Scrite.document.structure.forceBeatBoardLayout

        action: ActionHub.structureCanvasOperations.find("beatBoardLayout")
        enabled: !Scrite.document.readOnly
        checked: false

        onToggled: {
            root.canvasPreview.allowed = false
            Scrite.document.structure.forceBeatBoardLayout = checked
            if(checked && Scrite.document.structure.elementCount > 0)
                Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)

            Runtime.execLater(root.canvasPreview, 1000, function() {
                zoomOneRequest()
                root.canvasPreview.allowed = true
            })
        }

        Connections {
            target: Scrite.document.structure

            function onForceBeatBoardLayoutChanged() {
                _beatBoardLayoutActionHandler.checked = Scrite.document.structure.forceBeatBoardLayout
            }

            function onIndexCardFieldsChanged() {
                Qt.callLater( function() {
                    if(_beatBoardLayoutActionHandler.checked)
                        Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)
                })
            }
        }

        Connections {
            target: Scrite.document.screenplay
            enabled: Scrite.document.structure.forceBeatBoardLayout

            function onElementRemoved(element, index) {
                Qt.callLater( function() {
                    Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)
                })
            }
        }
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("grouping")
        down: _layoutGroupingMenu.visible

        onTriggered: (source) => {
                         _layoutGroupingMenu.popup(source)
                     }

        VclMenu {
            id: _layoutGroupingMenu

            width: 350

            /*VclMenuItem {
                text: "None"
                font.bold: currentGroupCategory === "{NONE}"

                onTriggered: groupCategoryRequest("{NONE}")
            }*/

            VclMenuItem {
                text: "Acts"
                font.bold: root.canvasScroll.groupCategory === ""

                onTriggered: groupCategoryRequest("")
            }

            Repeater {
                model: Scrite.document.structure.groupCategories

                VclMenuItem {
                    required property string modelData

                    text: SMath.titleCased(modelData)
                    font.bold: root.canvasScroll.groupCategory === modelData

                    onTriggered: groupCategoryRequest(modelData)
                }
            }

            MenuSeparator { }

            VclMenuItem {
                text: "Customise"

                onTriggered: StructureStoryBeatsDialog.launch()
            }
        }
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("tag")
        down: _structureGroupsMenu.visible
        enabled: (root.canvasScroll.selection.hasItems || root.canvasScroll.currentElementItem !== null) && Scrite.document.structure.canvasUIMode === Structure.IndexCardUI

        onTriggered: (source) => {
                         _structureGroupsMenu.popup(source)
                     }

        StructureGroupsMenu {
            id: _structureGroupsMenu

            innerTitle: {
                if(root.canvasScroll.selection.hasItems)
                    return "Tag the " + root.canvasScroll.selection.items.length + " selected index card(s)"
                else if(root.canvasScroll.currentElementItem !== null)
                    return "Tag the selected index card."
                return ""
            }

            sceneGroup: SceneGroup {
                structure: Scrite.document.structure
            }

            onToggled: {
                if(root.canvasScroll.selection.hasItems)
                    Runtime.execLater(root.canvasScroll.selection, 250, function() { root.canvasScroll.selection.refit() })
            }

            onAboutToShow: {
                sceneGroup.clearScenes()

                if(root.canvasScroll.selection.hasItems) {
                    let items = root.canvasScroll.selection.items
                    items.forEach( function(item) {
                        sceneGroup.addScene(item.element.scene)
                    })
                } else {
                    sceneGroup.addScene(root.canvasScroll.currentElementItem.element.scene)
                }
            }

            onClosed: sceneGroup.clearScenes()
        }
    }

    ActionHandler {
        property Scene scene: root.canvasScroll.currentElementItem ? root.canvasScroll.currentElementItem.element.scene :
                              (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.items[0].element.scene : null)

        action: ActionHub.structureCanvasOperations.find("sceneColor")
        down: _sceneColorMenu.visible
        enabled: (root.canvasScroll.selection.hasItems || root.canvasScroll.currentElementItem !== null)
        iconSource: scene ? "image://color/" + scene.color + "/1" : "image://color/gray/1"

        onTriggered: (source) => {
                         _sceneColorMenu.popup(source)
                     }

        ColorMenu {
            id: _sceneColorMenu

            title: "Scenes Color"

            onMenuItemClicked: {
                if(root.canvasScroll.selection.hasItems) {
                    let items = root.canvasScroll.selection.items
                    items.forEach( function(item) {
                        item.element.scene.color = color
                    })
                } else {
                    root.canvasScroll.currentElementItem.element.scene.color = color
                }

                _colorMenuLoader.active = false
            }
        }
    }

    ActionHandler {
        id: _sceneTypeActionHandler

        readonly property var sceneTypeModel: Object.typeEnumModel("Scene", "Type", _sceneTypeActionHandler)
        property Scene scene: root.canvasScroll.currentElementItem ? root.canvasScroll.currentElementItem.element.scene :
                              (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.items[0].element.scene : null)
        property int sceneType: (scene && scene.type !== Scene.Standard) ? scene.type : Scene.Standard

        action: ActionHub.structureCanvasOperations.find("sceneType")
        down: _sceneTypeMenu.visible
        enabled: (root.canvasScroll.selection.hasItems || root.canvasScroll.currentElementItem !== null)
        iconSource: {
            if(sceneType === Scene.Standard)
                return "qrc:/icons/content/standard_scene.png"
            return sceneTypeModel[sceneType].icon
        }

        onTriggered: (source) => {
                         _sceneTypeMenu.popup(source)
                     }

        MarkSceneAsMenu {
            id: _sceneTypeMenu

            enableValidation: false

            onTriggered: {
                if(root.canvasScroll.selection.hasItems) {
                    let items = root.canvasScroll.selection.items
                    items.forEach( function(item) {
                        item.element.scene.type = type
                    })
                } else {
                    root.canvasScroll.currentElementItem.element.scene.type = type
                }
                _sceneTypeMenuLoader.active = false
            }
        }
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("delete")
        enabled: !root.canvasScroll.selection.hasItems && root.canvasScroll.currentElementItem

        onTriggered: {
            let element = root.canvasScroll.currentElementItem.element
            deleteElementRequest(element)
        }
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("copy")
        enabled: !root.canvasScroll.selection.hasItems && (root.canvasScroll.currentAnnotation != null || root.canvasScroll.currentElementItem !== null)

        onTriggered: {
            if(root.canvasScroll.currentAnnotation != null) {
                Scrite.document.structure.copy(root.canvasScroll.currentAnnotation)
                AnimatedTextOverlay.show("Annotation Copied")
            } else {
                let spe = Scrite.document.structure.elementAt(Scrite.document.structure.currentElementIndex)
                if(spe !== null) {
                    Scrite.document.structure.copy(spe)
                    AnimatedTextOverlay.show("Scene Copied")
                }
            }
        }
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("paste")
        enabled: !Scrite.document.readOnly && Scrite.document.structure.canPaste

        onTriggered: {
            let gpos = MouseCursor.position()
            let pos = root.canvasScroll.mapFromGlobal(gpos.x, gpos.y)
            if(pos.x < 0 || pos.y < 0 || pos.x >= root.canvasScroll.width || pos.y >= root.canvasScroll.height)
                Scrite.document.structure.paste()
            else {
                pos = root.canvasScroll.canvas.mapFromGlobal(gpos.x, gpos.y)
                Scrite.document.structure.paste(Qt.point(pos.x,pos.y))
            }
        }
    }

    ActionHandler {
        action: ActionHub.structureCanvasOperations.find("pdfExport")

        onTriggered: ExportConfigurationDialog.launch(Scrite.document.structure.createExporterObject())
    }
}
