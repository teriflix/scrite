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
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

VclMenu {
    id: root

    property StructureElement element

    signal refitSelectionRequest()
    signal ensureItemVisibleRequest(Item item)
    signal deleteElementRequest(StructureElement element)

    width: 250

    VclMenuItem {
        action: Action {
            text: "Scene Heading"
            checkable: true
            checked: root.element ? root.element.scene.heading.enabled : false
        }
        enabled: root.element

        onTriggered: {
            root.element.scene.heading.enabled = action.checked
            root.element = null
        }
    }

    ColorMenu {
        title: "Color"
        enabled: root.element

        onMenuItemClicked: {
            root.element.scene.color = color
            root.element = null
        }
    }

    MarkSceneAsMenu {
        title: "Mark Scene As"
        scene: root.element ? root.element.scene : null

        onTriggered: root.element = null
    }

    VclMenuItem {
        property Scene lastScene: Scrite.document.screenplay.elementCount > 0 && Scrite.document.screenplay.elementAt(Scrite.document.screenplay.elementCount-1).scene

        text: "Add To Timeline"
        enabled: root.element && root.element.scene !== lastScene

        onClicked: {
            let lastScreenplayScene = null
            if(Scrite.document.screenplay.elementCount > 0)
                lastScreenplayScene = Scrite.document.screenplay.elementAt(Scrite.document.screenplay.elementCount-1).scene
            if(lastScreenplayScene === null || root.element.scene !== lastScreenplayScene)
                Scrite.document.screenplay.addScene(root.element.scene)
            root.element = null
        }
    }

    VclMenuItem {
        text: "Remove From Timeline"
        enabled: root.element && root.element.scene.addedToScreenplay

        onClicked: {
            Scrite.document.screenplay.removeSceneElements(root.element.scene)
            root.ensureItemVisibleRequest(root.element.follow)
            root.element = null
        }
    }

    StructureGroupsMenu {
        sceneGroup: _sceneGroup
        enabled: !Scrite.document.readOnly

        onToggled: root.refitSelectionRequest()
    }

    VclMenuItem {
        text: "Keywords"
        enabled: !Scrite.document.readOnly

        onClicked: SceneGroupKeywordsDialog.launch(_sceneGroup)
    }

    VclMenuItem {
        text: "Index Card Fields"
        enabled: root.element

        onClicked: StructureIndexCardFieldsDialog.launch()
    }

    MenuSeparator { }

    VclMenuItem {
        text: "Delete"
        enabled: root.element

        onClicked: {
            let element = root.element
            root.element = null
            if(Scrite.document.structure.canvasUIMode === Structure.IndexCardUI && element.follow)
                element.follow.confirmAndDeleteSelf()
            else
                root.deleteElementRequest(element)
        }
    }

    SceneGroup {
        id: _sceneGroup

        structure: Scrite.document.structure
    }

    onClosed: _sceneGroup.clearScenes()
    onAboutToShow: {
        _sceneGroup.clearScenes()
        _sceneGroup.addScene(root.element.scene)
    }

    onElementChanged: {
        if(element)
            popup()
        else
            close()
    }
}
