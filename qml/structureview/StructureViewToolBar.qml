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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/overlays"
import "qrc:/qml/structureview/structureelements"

Rectangle {
    id: root

    required property StructureCanvasPreview canvasPreview
    required property StructureCanvasScrollArea canvasScroll

    property alias newSceneColor: _newSceneButton.activeColor
    property alias selectionMode: _selectionModeButton.checked

    signal zoomOneRequest()
    signal newSceneRequest()
    signal selectAllRequest()
    signal notebookTabRequest()
    signal newAnnotationRequest(string annotationType)
    signal groupCategoryRequest(string groupCategory)
    signal newColoredSceneRequest(color sceneColor)
    signal deleteElementRequest(StructureElement element)
    signal selectionLayoutRequest(int layout) // here layout should be one of Structure.LayoutType which are
                                              // Structure.HorizontalLayout, Structure.VerticalLayout,
                                              // Structure.FlowHorizontalLayout, Structure.FlowVerticalLayout

    color: Runtime.colors.primary.c100.background

    width: _toolbarLayout.width+4

    Flow {
        id: _toolbarLayout

        property real columnWidth: _structureTabButton.width

        anchors.horizontalCenter: parent.horizontalCenter

        height: parent.height-5

        flow: Flow.TopToBottom
        spacing: 1
        layoutDirection: Qt.RightToLeft

        FlatToolButton {
            id: _structureTabButton

            ToolTip.text: "Structure\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"

            down: true
            visible: Runtime.showNotebookInStructure
            iconSource: "qrc:/icons/navigation/structure_tab.png"
        }

        FlatToolButton {
            id: _notebookTabButton

            ToolTip.text: "Notebook Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"

            visible: Runtime.showNotebookInStructure
            iconSource: "qrc:/icons/navigation/notebook_tab.png"

            onClicked: notebookTabRequest()
        }

        ToolBarSeparator { }

        FlatToolButton {
            id: _newSceneButton

            property color activeColor: "white"

            ToolTip.text: "Add Scene"

            down: _newSceneMenu.visible
            hasMenu: true
            enabled: !Scrite.document.readOnly
            iconSource: "qrc:/icons/action/add_scene.png"

            onClicked: _newSceneMenu.open()

            Item {
                anchors.top: parent.top
                anchors.right: parent.right

                VclMenu {
                    id: _newSceneMenu

                    VclMenuItem {
                        text: "New Scene"
                        enabled: !Scrite.document.readOnly

                        onClicked: {
                            newSceneRequest()
                            Qt.callLater( function() { _newSceneMenu.close() } )
                        }
                    }

                    ColorMenu {
                        title: "Colored Scene"
                        enabled: !Scrite.document.readOnly
                        selectedColor: _newSceneButton.activeColor

                        onMenuItemClicked: (color) => {
                            _newSceneButton.activeColor = color
                            newColoredSceneRequest( _newSceneButton.activeColor)
                            Qt.callLater( function() { _newSceneMenu.close() } )
                        }
                    }
                }
            }
        }

        FlatToolButton {
            id: _newAnnotationButton

            ToolTip.text: "Add Annotation"

            down: _newAnnotationMenu.visible
            enabled: !Scrite.document.readOnly
            hasMenu: true
            iconSource: "qrc:/icons/action/add_annotation.png"

            onClicked: _newAnnotationMenu.open()

            Item {
                id: _newAnnotationMenuArea

                anchors.top: parent.top
                anchors.right: parent.right

                VclMenu {
                    id: _newAnnotationMenu

                    Repeater {
                        model: root.canvasScroll.annotationList

                        VclMenuItem {
                            required property var modelData

                            text: modelData.title
                            enabled: !Scrite.document.readOnly && modelData.what !== ""

                            onClicked: {
                                newAnnotationRequest(modelData.what)
                                Qt.callLater( function() { _newAnnotationMenu.close() } )
                            }
                        }
                    }
                }
            }
        }

        ToolBarSeparator { }

        FlatToolButton {
            id: _selectionModeButton

            ToolTip.text: "Selection mode"

            enabled: !Scrite.document.readOnly && (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.canLayout : Scrite.document.structure.elementCount >= 2)
            iconSource: "qrc:/icons/action/selection_drag.png"
            checkable: true

            onClicked: selectionLayoutRequest(Structure.HorizontalLayout)
        }

        FlatToolButton {
            ToolTip.text: "Select All"

            enabled: !Scrite.document.readOnly && Scrite.document.structure.elementCount >= 2
            iconSource: "qrc:/icons/content/select_all.png"

            onClicked: selectAllRequest()
        }

        FlatToolButton {
            ToolTip.text: "Layout Options"

            down: _layoutOptionsMenu.visible
            hasMenu: true
            enabled: !Scrite.document.readOnly && (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.canLayout : Scrite.document.structure.elementCount >= 2) && !Scrite.document.structure.forceBeatBoardLayout
            iconSource: "qrc:/icons/action/layout_options.png"

            onClicked: _layoutOptionsMenu.visible = true

            Item {
                anchors.top: parent.top
                anchors.right: parent.right

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
                            icon.source: modelData.icon

                            onClicked: selectionLayoutRequest(modelData.type)
                        }
                    }
                }
            }
        }

        FlatToolButton {
            id: _beatBoardLayoutToolButton

            ToolTip.text: "Beat Board Layout"

            enabled: !Scrite.document.readOnly
            checked: false
            checkable: true
            iconSource: "qrc:/icons/action/layout_beat_sheet.png"

            onToggled: {
                root.canvasPreview.allowed = false
                Scrite.document.structure.forceBeatBoardLayout = checked
                if(checked && Scrite.document.structure.elementCount > 0)
                    Scrite.document.structure.placeElementsInBeatBoardLayout(Scrite.document.screenplay)

                Utils.execLater(root.canvasPreview, 1000, function() {
                    zoomOneRequest()
                    root.canvasPreview.allowed = true
                })
            }

            Component.onCompleted: checked = Scrite.document.structure.forceBeatBoardLayout

            Connections {
                target: Scrite.document.structure

                function onForceBeatBoardLayoutChanged() {
                    _beatBoardLayoutToolButton.checked = Scrite.document.structure.forceBeatBoardLayout
                }

                function onIndexCardFieldsChanged() {
                    Qt.callLater( function() {
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

        FlatToolButton {
            ToolTip.text: "Grouping Options"

            down: _layoutGroupingMenu.visible
            iconSource: "qrc:/icons/action/layout_grouping.png"

            onClicked: _layoutGroupingMenu.popup()

            Item {
                anchors.top: parent.top
                anchors.right: parent.right

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

                            text: Scrite.app.camelCased(modelData)
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
        }

        FlatToolButton {
            id: _tagMenuOption

            ToolTip.text: {
                if(root.canvasScroll.selection.hasItems)
                    return "Tag the " + root.canvasScroll.selection.items.length + " selected index card(s)"
                else if(root.canvasScroll.currentElementItem !== null)
                    return "Tag the selected index card."
                return ""
            }

            down: _tagMenuLoader.active
            enabled: (root.canvasScroll.selection.hasItems || root.canvasScroll.currentElementItem !== null) && Scrite.document.structure.canvasUIMode === Structure.IndexCardUI
            iconSource: "qrc:/icons/action/tag.png"

            onClicked: _tagMenuLoader.popup()

            MenuLoader {
                id: _tagMenuLoader

                anchors.top: parent.top
                anchors.right: parent.right

                menu: StructureGroupsMenu {
                    innerTitle: _tagMenuOption.ToolTip.text

                    sceneGroup: SceneGroup {
                        structure: Scrite.document.structure
                    }

                    onToggled: {
                        if(root.canvasScroll.selection.hasItems)
                            Utils.execLater(root.canvasScroll.selection, 250, function() { root.canvasScroll.selection.refit() })
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
        }

        FlatToolButton {
            id: _changeColorOption

            property Scene scene: root.canvasScroll.currentElementItem ? root.canvasScroll.currentElementItem.element.scene :
                                  (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.items[0].element.scene : null)

            ToolTip.text: "Change current scene(s) color."

            down: _colorMenuLoader.active
            enabled: (root.canvasScroll.selection.hasItems || root.canvasScroll.currentElementItem !== null)
            iconSource: scene ? "image://color/" + scene.color + "/1" : "image://color/gray/1"

            onClicked: _colorMenuLoader.popup()

            MenuLoader {
                id: _colorMenuLoader

                anchors.top: parent.top
                anchors.right: parent.right

                menu: ColorMenu {
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
        }

        FlatToolButton {
            id: _changeSceneTypeOption

            readonly property var sceneTypeModel: Scrite.app.enumerationModelForType("Scene", "Type")
            property Scene scene: root.canvasScroll.currentElementItem ? root.canvasScroll.currentElementItem.element.scene :
                                  (root.canvasScroll.selection.hasItems ? root.canvasScroll.selection.items[0].element.scene : null)
            property int sceneType: (scene && scene.type !== Scene.Standard) ? scene.type : Scene.Standard

            ToolTip.text: enabled ? "Change scene type from '" + (sceneTypeModel[sceneType].key) + "' to something else." : ""

            down: _sceneTypeMenuLoader.active
            enabled: _changeColorOption.enabled
            iconSource: {
                if(sceneType === Scene.Standard)
                    return "qrc:/icons/content/standard_scene.png"
                return sceneTypeModel[sceneType].icon
            }

            onClicked: _sceneTypeMenuLoader.popup()

            MenuLoader {
                id: _sceneTypeMenuLoader

                anchors.top: parent.top
                anchors.right: parent.right

                menu: MarkSceneAsMenu {
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
        }

        FlatToolButton {
            id: _deleteSceneOption

            ToolTip.text: enabled ? "Delete selected scene." : ""

            enabled: !root.canvasScroll.selection.hasItems && root.canvasScroll.currentElementItem
            iconSource: "qrc:/icons/action/delete.png"

            onClicked: {
                let element = root.canvasScroll.currentElementItem.element
                deleteElementRequest(element)
            }
        }

        ToolBarSeparator { }

        FlatToolButton {
            ToolTip.text: "Copy the selected scene or annotation."

            ShortcutsModelItem.group: "Edit"
            ShortcutsModelItem.title: "Copy Annotation"
            ShortcutsModelItem.enabled: enabled
            ShortcutsModelItem.shortcut: Scrite.app.polishShortcutTextForDisplay("Ctrl+C")

            enabled: !root.canvasScroll.selection.hasItems && (root.canvasScroll.currentAnnotation != null || root.canvasScroll.currentElementItem !== null)
            shortcut: "Ctrl+C"
            iconSource: "qrc:/icons/content/content_copy.png"

            onClicked: {
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

        FlatToolButton {
            ToolTip.text: "Paste from clipboard"

            ShortcutsModelItem.group: "Edit"
            ShortcutsModelItem.title: "Paste"
            ShortcutsModelItem.enabled: enabled
            ShortcutsModelItem.shortcut: Scrite.app.polishShortcutTextForDisplay(shortcut)

            shortcut: "Ctrl+V"
            enabled: !Scrite.document.readOnly && Scrite.document.structure.canPaste
            iconSource: "qrc:/icons/content/content_paste.png"

            onClicked: {
                let gpos = Scrite.app.globalMousePosition()
                let pos = root.canvasScroll.mapFromGlobal(gpos.x, gpos.y)
                if(pos.x < 0 || pos.y < 0 || pos.x >= root.canvasScroll.width || pos.y >= root.canvasScroll.height)
                    Scrite.document.structure.paste()
                else {
                    pos = root.canvasScroll.canvas.mapFromGlobal(gpos.x, gpos.y)
                    Scrite.document.structure.paste(Qt.point(pos.x,pos.y))
                }
            }
        }

        FlatToolButton {
            id: _pdfExportButton

            ToolTip.text: "Export the contents of the structure canvas to PDF."

            iconSource: "qrc:/icons/file/generate_pdf.png"

            onClicked: ExportConfigurationDialog.launch(Scrite.document.structure.createExporterObject())
        }
    }

    Rectangle {
        width: 1
        height: parent.height

        anchors.right: parent.right
        color: Runtime.colors.primary.borderColor
    }

    component ToolBarSeparator : Rectangle {
        width: _toolbarLayout.columnWidth
        height: 1

        color: Runtime.colors.primary.separatorColor
        opacity: 0.5
    }

    Loader {
        anchors.fill: parent

        active: Runtime.workspaceSettings.animateNotebookIcon && Runtime.showNotebookInStructure

        sourceComponent: UiElementHighlight {
            property bool scaleDone: false

            uiElement: _notebookTabButton
            description: _notebookTabButton.ToolTip.text

            Component.onDestruction: {
                if(scaleDone)
                    Runtime.workspaceSettings.animateNotebookIcon = false
            }

            onDone: Runtime.workspaceSettings.animateNotebookIcon = false
            onScaleAnimationDone: scaleDone = true
        }
    }
}
