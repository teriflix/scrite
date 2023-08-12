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
        enabled: !Scrite.document.readOnly && !omitIncludeMenuItem.omitted
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
        text: "Copy"
        onClicked: Scrite.document.screenplay.copySelection()
    }

    MenuItem2 {
        text: "Paste After"
        enabled: Scrite.document.screenplay.canPaste
        onClicked: Scrite.document.screenplay.pasteAfter( Scrite.document.screenplay.indexOfElement(element) )
    }

    MenuSeparator { }

    MenuItem2 {
        id: omitIncludeMenuItem
        property bool omitted: Scrite.document.screenplay.selectedElementsOmitStatus !== Screenplay.NotOmitted
        text: omitted ? "Include" : "Omit"
        onClicked: {
            screenplayContextMenu.close()
            if(omitted)
                Scrite.document.screenplay.includeSelectedElements()
            else
                Scrite.document.screenplay.omitSelectedElements()
        }
    }

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
