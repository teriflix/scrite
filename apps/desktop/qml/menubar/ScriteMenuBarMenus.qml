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

import QtQml
import QtQuick
import QtQuick.Controls

import Scrite.App

import "../globals"

/**
  The menus of the menu bar, as groups that every platform's ScriteMenuBar shows. Extensions can
  merge into them with the uri of the group, or add menus of their own with the same hostUri.

  Scrite.MenuBar.File, .Language, .Screenplay, .Structure, .Notebook, .Scrited, .Tabs, .Tools,
  .Account and .Help.
  */
Item {
    id: root

    // The uri of the menu bars, which every group below gives as its hostUri.
    readonly property string menuBarUri: "Scrite.App.MainWindow.MenuBar"

    visible: false

    MenuGroup {
        uri: "Scrite.MenuBar.File"
        hostUri: root.menuBarUri
        order: 10
        title: "File"

        entries: [
            ActionHub.fileOperations.find("fileNew"),
            ActionHub.fileOperations.find("fileOpen"),
            ActionHub.fileOperations.find("fileSave"),
            ActionHub.fileOperations.find("fileSaveAs"),
            ActionHub.fileOperations.find("fileClose"),
            null,
            ActionHub.recentFileOperations,
            ActionHub.fileOperations.find("import"),
            ActionHub.templateOperations,
            ActionHub.scriptalayOperations,
            null,
            ActionHub.exportOptions,
            ActionHub.reportOptions
        ]
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Language"
        hostUri: root.menuBarUri
        order: 20
        actionManager: ActionHub.languageOptions
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Language"
        inline: true
        visible: _alphabetMappings.enabled
        entries: [_alphabetMappings]

        readonly property Action _alphabetMappings: ActionHub.inputOptions.find("alphabetMappings")
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Screenplay"
        hostUri: root.menuBarUri
        order: 30
        title: "Screenplay"
        visible: Runtime.screenplayEditor !== null

        entries: [
            ActionHub.editOptions.find("cut"),
            ActionHub.editOptions.find("copy"),
            ActionHub.editOptions.find("paste"),
            ActionHub.editOptions.find("selectAll"),
            null,
            ActionHub.editOptions.find("undo"),
            ActionHub.editOptions.find("redo"),
            null,
            ActionHub.editOptions.find("find"),
            ActionHub.editOptions.find("replace"),
            null
        ]
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Screenplay"
        order: 1
        title: "Components"
        actionManager: ActionHub.screenplayOperations
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Screenplay"
        order: 2
        actionManager: ActionHub.editOptions
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Screenplay"
        order: 3
        actionManager: ActionHub.markupTools
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Screenplay"
        order: 4
        actionManager: ActionHub.paragraphFormats
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Screenplay"
        order: 5
        title: "Editor Options"
        actionManager: ActionHub.screenplayEditorOptions
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Screenplay"
        order: 6
        title: "Scene List Panel"
        visible: Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab
        actionManager: ActionHub.sceneListPanelOptions
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Structure"
        hostUri: root.menuBarUri
        order: 40
        title: "Structure"
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Structure"
        order: 1
        title: "Canvas"
        visible: Runtime.mainWindowTab === Runtime.MainWindowTab.StructureTab

        entries: [
            ActionHub.structureCanvasOperations.find("copy"),
            ActionHub.structureCanvasOperations.find("paste"),
            ActionHub.structureCanvasOperations.find("delete"),
            ActionHub.structureCanvasOperations.find("selectAll"),
            null,
            ActionHub.structureCanvasOperations.find("zoomIn"),
            ActionHub.structureCanvasOperations.find("zoomOut"),
            ActionHub.structureCanvasOperations.find("zoomOne"),
            ActionHub.structureCanvasOperations.find("zoomFit"),
            null,
            ActionHub.structureCanvasOperations.find("beatBoardLayout")
        ]
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Structure"
        order: 2
        title: "Story"
        actionManager: ActionHub.storyStructureOptions
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Structure"
        order: 3
        visible: Runtime.mainWindowTab === Runtime.MainWindowTab.StructureTab ||
                 Runtime.showNotebookInStructure && Runtime.mainWindowTab === Runtime.MainWindowTab.NotebookTab
        actionManager: ActionHub.timelineOperations
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Notebook"
        hostUri: root.menuBarUri
        order: 50
        visible: Runtime.mainWindowTab === Runtime.MainWindowTab.NotebookTab
        actionManager: ActionHub.notebookOperations
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Scrited"
        hostUri: root.menuBarUri
        order: 60
        visible: Runtime.mainWindowTab === Runtime.MainWindowTab.ScritedTab
        actionManager: ActionHub.scritedOptions
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Tabs"
        hostUri: root.menuBarUri
        order: 70
        title: "Tabs"
        actionManager: ActionHub.mainWindowTabs
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Tools"
        hostUri: root.menuBarUri
        order: 80
        title: "Tools"
        actionManager: ActionHub.appOptions
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Account"
        hostUri: root.menuBarUri
        order: 90
        title: "Account"
        actionManager: ActionHub.userOptions
    }

    MenuGroup {
        uri: "Scrite.MenuBar.Help"
        hostUri: root.menuBarUri
        order: 100
        title: "Help"
        actionManager: ActionHub.helpSupportOptions
    }

    MenuGroup {
        hostUri: root.menuBarUri
        mergeIntoUri: "Scrite.MenuBar.Help"
        inline: true
        entries: [ActionHub.appOptions.find("helpCenter")]
    }
}
