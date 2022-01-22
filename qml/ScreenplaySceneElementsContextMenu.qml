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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Menu2 {
    id: screenplayContextMenu
    property ScreenplayElement element

    SceneGroup {
        id: elementItemMenuSceneGroup
        structure: Scrite.document.structure
    }

    onAboutToShow: {
        mainUndoStack.sceneListPanelActive = true
        if(element.selected) {
            Scrite.document.screenplay.gatherSelectedScenes(elementItemMenuSceneGroup)
        } else {
            Scrite.document.screenplay.clearSelection()
            element.selected = true
            elementItemMenuSceneGroup.addScene(element.scene)
        }
    }

    onClosed: {
        element = null
        elementItemMenuSceneGroup.clearScenes()
    }

    ColorMenu {
        title: "Color"
        enabled: !Scrite.document.readOnly && screenplayContextMenu.element
        onMenuItemClicked: {
            for(var i=0; i<elementItemMenuSceneGroup.sceneCount; i++) {
                elementItemMenuSceneGroup.sceneAt(i).color = color
            }
            screenplayContextMenu.close()
        }
    }

    MarkSceneAsMenu {
        title: "Mark Scene As"
        scene: screenplayContextMenu.element ? screenplayContextMenu.element.scene : null
        enabled: !Scrite.document.readOnly
        onTriggered: {
            mainUndoStack.sceneListPanelActive = true
            for(var i=0; i<elementItemMenuSceneGroup.sceneCount; i++) {
                elementItemMenuSceneGroup.sceneAt(i).type = scene.type
            }
            screenplayContextMenu.close()
        }
    }

    StructureGroupsMenu {
        sceneGroup: elementItemMenuSceneGroup
        enabled: !Scrite.document.readOnly
    }

    MenuSeparator { }

    MenuItem2 {
        text: "Remove"
        enabled: !Scrite.document.readOnly
        onClicked: {
            if(elementItemMenuSceneGroup.sceneCount <= 1)
                Scrite.document.screenplay.removeElement(screenplayContextMenu.element)
            else
                Scrite.document.screenplay.removeSelectedElements();
            screenplayContextMenu.close()
        }
    }
}
