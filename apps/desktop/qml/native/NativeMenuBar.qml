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
import Qt.labs.platform as Native

import "../globals"

Native.MenuBar {
    id: root

    property bool enabled: true

    Native.Menu {
        id: _fileMenu

        title: "File"

        NativeActionMenuItem {
            text: "New"
            action: ActionHub.fileOperations.find("fileNew")
            visible: true
        }

        NativeActionMenuItem {
            text: "Open"
            action: ActionHub.fileOperations.find("fileOpen")
        }

        NativeActionMenuItem {
            text: "Save"
            action: ActionHub.fileOperations.find("fileSave")
            visible: true
        }

        NativeActionMenuItem {
            text: "Save As"
            action: ActionHub.fileOperations.find("fileSaveAs")
            visible: true
        }

        NativeActionMenuItem {
            text: "Close"
            action: ActionHub.fileOperations.find("fileClose")
            visible: true
        }

        Native.MenuSeparator { }

        ActionManagerNativeMenu {
            actionManager: ActionHub.recentFileOperations
        }

        NativeActionMenuItem {
            action: ActionHub.fileOperations.find("import")
            visible: true
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.templateOperations
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.scriptalayOperations
        }

        Native.MenuSeparator { }

        ActionManagerNativeMenu {
            actionManager: ActionHub.exportOptions
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.reportOptions
        }
    }

    ActionManagerNativeMenu {
        id: _languageMenu

        actionManager: ActionHub.languageOptions

        Instantiator {
            model: 1

            delegate: NativeActionMenuItem {
                action: ActionHub.inputOptions.find("alphabetMappings")
            }

            onObjectAdded: (index, object) => _languageMenu.addItem(object)
            onObjectRemoved: (index, object) => _languageMenu.removeItem(object)
        }
    }

    Native.Menu {
        title: "Screenplay"
        visible: Runtime.screenplayEditor !== null

        ActionManagerNativeMenu {
            actionManager: ActionHub.editOptions
        }

        ActionManagerNativeMenu {
            title: "Options"

            actionManager: ActionHub.screenplayOperations
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.markupTools
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.paragraphFormats
        }

        ActionManagerNativeMenu {
            title: "Editor"
            actionManager: ActionHub.screenplayEditorOptions
        }

        ActionManagerNativeMenu {
            visible: Runtime.mainWindowTab === Runtime.ScreenplayTab
            title: "Scene List Panel"

            actionManager: ActionHub.sceneListPanelOptions
        }
    }

    Native.Menu {
        title: "Structure"

        NativeActionMenuItem {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
            action: ActionHub.structureCanvasOperations.find("copy")
        }

        NativeActionMenuItem {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
            action: ActionHub.structureCanvasOperations.find("paste")
        }

        NativeActionMenuItem {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
            action: ActionHub.structureCanvasOperations.find("delete")
        }

        NativeActionMenuItem {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
            action: ActionHub.structureCanvasOperations.find("selectAll")
        }

        Native.MenuSeparator {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
        }

        NativeActionMenuItem {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
            action: ActionHub.structureCanvasOperations.find("zoomIn")
        }

        NativeActionMenuItem {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
            action: ActionHub.structureCanvasOperations.find("zoomOut")
        }

        Native.MenuSeparator {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
        }

        NativeActionMenuItem {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
            action: ActionHub.structureCanvasOperations.find("beatBoardLayout")
        }

        Native.MenuSeparator {
            visible: Runtime.mainWindowTab === Runtime.StructureTab
        }

        ActionManagerNativeMenu {
            title: "Story"

            actionManager: ActionHub.storyStructureOptions
        }

        ActionManagerNativeMenu {
            visible: Runtime.mainWindowTab === Runtime.StructureTab ||
                     Runtime.showNotebookInStructure && Runtime.mainWindowTab === Runtime.NotebookTab
            actionManager: ActionHub.timelineOperations
        }
    }

    ActionManagerNativeMenu {
        visible: Runtime.mainWindowTab === Runtime.MainWindowTab.NotebookTab

        actionManager: ActionHub.notebookOperations
    }

    ActionManagerNativeMenu {
        visible: Runtime.mainWindowTab === Runtime.MainWindowTab.ScritedTab

        actionManager: ActionHub.scritedOptions
    }

    ActionManagerNativeMenu {
        title: "Tools"
        actionManager: ActionHub.appOptions
    }

    ActionManagerNativeMenu {
        title: "Tabs"
        actionManager: ActionHub.mainWindowTabs
    }

    ActionManagerNativeMenu {
        id: _helpMenu

        title: "Help"

        actionManager: ActionHub.helpSupportOptions

        Instantiator {
            model: 1

            delegate: NativeActionMenuItem {
                action: ActionHub.appOptions.find("helpCenter")
            }

            onObjectAdded: (index, object) => _helpMenu.addItem(object)
            onObjectRemoved: (index, object) => _helpMenu.removeItem(object)
        }
    }
}
