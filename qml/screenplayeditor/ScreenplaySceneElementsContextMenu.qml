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
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/structureview"
import "qrc:/qml/dialogs"

VclMenu {
    id: root

    property ScreenplayElement element

    VclMenuItem {
        enabled: _sceneGroup.sceneCount === 1 && root.element && root.element.scene

        action: Action {
            text: "Scene Heading"
            checkable: true
            checked: root.element && root.element.scene && root.element.scene.heading.enabled
        }

        onTriggered: root.element.scene.heading.enabled = action.checked
    }

    VclMenu {
        enabled: _sceneGroup.sceneCount === 1

        title: "Page Breaks"

        VclMenuItem {
            action: Action {
                text: "Before"
                checkable: true
                checked: root.element && root.element.pageBreakBefore
            }

            onTriggered: root.element.pageBreakBefore = action.checked
        }

        VclMenuItem {
            action: Action {
                text: "After"
                checkable: true
                checked: root.element && root.element.pageBreakAfter
            }

            onTriggered: root.element.pageBreakAfter = action.checked
        }
    }

    ColorMenu {
        title: "Color"
        enabled: !Scrite.document.readOnly && root.element

        onMenuItemClicked: {
            for(var i=0; i<_sceneGroup.sceneCount; i++) {
                _sceneGroup.sceneAt(i).color = color
            }
            root.close()
        }
    }

    MarkSceneAsMenu {
        title: "Mark Scene As"
        scene: root.element ? root.element.scene : null
        enabled: !Scrite.document.readOnly && !omitIncludeMenuItem.omitted

        onTriggered: {
            for(var i=0; i<_sceneGroup.sceneCount; i++) {
                _sceneGroup.sceneAt(i).type = scene.type
            }
            root.close()
        }
    }

    VclMenuItem {
        text: "Make Sequence"

        enabled: !Scrite.document.readOnly && _sceneGroup.canBeStacked

        onTriggered: {
            if(!_sceneGroup.stack()) {
                MessageBox.information("Make Sequence Error",
                                       "Couldn't stack these scenes to make a sequence. Please try doing this on the Structure Tab.")
            }
        }
    }

    VclMenuItem {
        text: "Break Sequence"

        enabled: !Scrite.document.readOnly && _sceneGroup.canBeUnstacked

        onTriggered: {
            if(!_sceneGroup.unstack()) {
                MessageBox.information("Break Sequence Error",
                                       "Couldn't unstack these scenes to make a sequence. Please try doing this on the Structure Tab.")
            }
        }
    }

    StructureGroupsMenu {
        sceneGroup: _sceneGroup
        enabled: !Scrite.document.readOnly
    }

    VclMenuItem {
        text: "Keywords"
        enabled: !Scrite.document.readOnly

        onClicked: SceneGroupKeywordsDialog.launch(_sceneGroup)
    }

    VclMenu {
        title: "Reports"

        width: 250

        Repeater {
            model: Runtime.sceneListReports

            VclMenuItem {
                required property var modelData

                text: modelData.name
                icon.source: "qrc" + modelData.icon

                onTriggered: ReportConfigurationDialog.launch(modelData.name,
                                                              {"sceneNumbers": Scrite.document.screenplay.selectedElementIndexes()},
                                                              {"initialPage": modelData.group})
            }
        }
    }

    MenuSeparator { }

    VclMenuItem {
        text: "Copy"

        onClicked: Scrite.document.screenplay.copySelection()
    }

    VclMenuItem {
        text: "Paste After"
        enabled: Scrite.document.screenplay.canPaste

        onClicked: Scrite.document.screenplay.pasteAfter( Scrite.document.screenplay.indexOfElement(element) )
    }

    MenuSeparator { }

    VclMenuItem {
        id: omitIncludeMenuItem

        property bool omitted: Scrite.document.screenplay.selectedElementsOmitStatus !== Screenplay.NotOmitted

        text: omitted ? "Include" : "Omit"

        onClicked: {
            root.close()
            if(omitted)
                Scrite.document.screenplay.includeSelectedElements()
            else
                Scrite.document.screenplay.omitSelectedElements()
        }
    }

    VclMenuItem {
        text: "Remove"
        enabled: !Scrite.document.readOnly

        onClicked: {
            if(_sceneGroup.sceneCount <= 1)
                Scrite.document.screenplay.removeElement(root.element)
            else
                Scrite.document.screenplay.removeSelectedElements();
            root.close()
        }
    }

    SceneGroup {
        id: _sceneGroup

        structure: Scrite.document.structure
    }

    onAboutToShow: {
        if(element.selected) {
            Scrite.document.screenplay.gatherSelectedScenes(_sceneGroup)
        } else {
            Scrite.document.screenplay.clearSelection()
            element.selected = true
            _sceneGroup.addScene(element.scene)
        }
    }

    onClosed: {
        element = null
        _sceneGroup.clearScenes()
    }
}
