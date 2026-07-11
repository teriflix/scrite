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

import io.scrite.components

import "../globals"

Native.MenuBar {
    id: root

    Native.Menu {
        id: _fileMenu

        title: "File"
        // type: Native.Menu.DefaultMenu

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
                visible: action.enabled
            }

            onObjectAdded: (index, object) => _languageMenu.addItem(object)
            onObjectRemoved: (index, object) => _languageMenu.removeItem(object)
        }
    }

    Native.Menu {
        title: "Screenplay"
        type: Native.Menu.DefaultMenu
        visible: Runtime.screenplayEditor !== null

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("cut")
            visible: true
        }

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("copy")
            visible: true
        }

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("paste")
            visible: true
        }

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("selectAll")
            visible: true
        }

        Native.MenuSeparator { }

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("undo")
            visible: true
        }

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("redo")
            visible: true
        }

        Native.MenuSeparator { }

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("find")
            visible: true
        }

        NativeActionMenuItem {
            action: ActionHub.editOptions.find("replace")
            visible: true
        }

        Native.MenuSeparator { }

        ActionManagerNativeMenu {
            title: "Components"
            actionManager: ActionHub.screenplayOperations
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.editOptions
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.markupTools
        }

        ActionManagerNativeMenu {
            actionManager: ActionHub.paragraphFormats
        }

        ActionManagerNativeMenu {
            title: "Editor Options"
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
        // type: Native.Menu.DefaultMenu

        Native.Menu {
            title: "Canvas"
            // type: Native.Menu.DefaultMenu
            visible: Runtime.mainWindowTab === Runtime.StructureTab

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("copy")
            }

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("paste")
            }

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("delete")
            }

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("selectAll")
            }

            Native.MenuSeparator {}

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("zoomIn")
            }

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("zoomOut")
            }

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("zoomOne")
            }

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("zoomFit")
            }

            Native.MenuSeparator {}

            NativeActionMenuItem {
                action: ActionHub.structureCanvasOperations.find("beatBoardLayout")
            }
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
        title: "Tabs"
        actionManager: ActionHub.mainWindowTabs
    }

    ActionManagerNativeMenu {
        title: "Tools"
        actionManager: ActionHub.appOptions

        // Catch all menu for every other action thats not already covered.
        // macOS requires every action that can have a shortcut to go here.
        Native.Menu {
            id: _catchAllMenu

            title: "Misc"
            visible: false

            Instantiator {
                model: Runtime.nativelyNotShownActions

                delegate: NativeActionMenuItem {
                    required property int index
                    required property var qmlAction
                    required property var actionManager
                    required property bool shortcutIsEditable
                    required property string groupName

                    property bool nativelyShownAlready: Object.queryProperty(action, "#nativelyShown")

                    action: qmlAction
                    text: actionManager.title + ": " + action.text
                    visible: true
                }

                onObjectAdded: (index, object) => _catchAllMenu.addItem(object)
                onObjectRemoved: (index, object) => _catchAllMenu.removeItem(object)
            }
        }
    }

    ActionManagerNativeMenu {
        title: "Account"
        actionManager: ActionHub.userOptions
    }

    ActionManagerNativeMenu {
        id: _helpMenu

        title: "Help"

        actionManager: ActionHub.helpSupportOptions

        Instantiator {
            model: 1

            delegate: NativeActionMenuItem {
                action: ActionHub.appOptions.find("helpCenter")
                visible: true
            }

            onObjectAdded: (index, object) => _helpMenu.addItem(object)
            onObjectRemoved: (index, object) => _helpMenu.removeItem(object)
        }
    }
}
