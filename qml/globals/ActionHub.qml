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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/tasks"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

Item {
    id: root

    readonly property alias binder: _private.binder

    function setBinder(binder) { _private.setBinder(binder) }
    function resetBinder(binder) { _private.resetBinder(binder) }
    function isOperationAllowedByUser(operation) { return _private.isOperationAllowedByUser(operation) }

    function triggerLater(action, delay) {
        if(action)
            Runtime.execLater(action, delay, () => {
                                  if(action && action.enabled)
                                    action.trigger()
                              })
    }

    function toggleLater(action, delay) {
        if(action)
            Runtime.execLater(action, delay, () => {
                                  if(action && action.enabled)
                                    action.toggle()
                              })
    }

    readonly property ActionManager mainWindowTabs: ActionManager {
        title: "Scrite Window Tabs"
        objectName: "mainWindowTabs"

        Action {
            readonly property string defaultShortcut: "Alt+1"
            property bool down: Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab

            objectName: "screenplayTab"
            shortcut: defaultShortcut
            text: "Screenplay"

            icon.source: "qrc:/icons/navigation/screenplay_tab.png"

            onTriggered: Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScreenplayTab)
        }

        Action {
            readonly property string defaultShortcut: "Alt+2"
            property bool down: Runtime.mainWindowTab === Runtime.MainWindowTab.StructureTab

            objectName: "structureTab"
            shortcut: defaultShortcut
            text: "Structure"

            icon.source: "qrc:/icons/navigation/structure_tab.png"

            onTriggered: Runtime.activateMainWindowTab(Runtime.MainWindowTab.StructureTab)
        }

        Action {
            readonly property string defaultShortcut: "Alt+3"
            property bool down: Runtime.mainWindowTab === Runtime.MainWindowTab.NotebookTab

            objectName: "notebookTab"
            shortcut: defaultShortcut
            text: "Notebook"

            icon.source: "qrc:/icons/navigation/notebook_tab.png"

            onTriggered: Runtime.activateMainWindowTab(Runtime.MainWindowTab.NotebookTab)
        }

        Action {
            readonly property string defaultShortcut: "Alt+4"
            property bool down: Runtime.mainWindowTab === Runtime.MainWindowTab.ScritedTab
            property bool visible: Runtime.workspaceSettings.showScritedTab

            enabled: visible
            objectName: "scritedTab"
            shortcut: defaultShortcut
            text: "Scrited"

            icon.source: "qrc:/icons/navigation/scrited_tab.png"

            onTriggered: Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScritedTab)
        }
    }

    readonly property ActionManager fileOperations: ActionManager {
        title: "File"
        objectName: "fileOperations"

        Action {
            readonly property var keywords: ["file", "open"]
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Open)

            enabled: Runtime.allowAppUsage
            objectName: "fileOpen"
            shortcut: defaultShortcut
            text: "Home"

            icon.source: "qrc:/icons/action/home.png"

            onTriggered: HomeScreen.launch()
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.New)

            enabled: Runtime.allowAppUsage
            objectName: "fileNew"
            shortcut: defaultShortcut
            text: "New"

            onTriggered: HomeScreen.launch()
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Shift+O"

            enabled: Runtime.allowAppUsage
            objectName: "scriptalayOpen"
            shortcut: defaultShortcut
            text: "Scriptalay"

            onTriggered: HomeScreen.launch(text)
        }

        Action {
            readonly property bool visible: false
            readonly property bool allowShortcut: true
            readonly property string tooltip: "Import from Final Draft, HTML, Plain Text & Fountain file formats."

            enabled: Runtime.allowAppUsage
            objectName: "import"
            text: "Import"

            onTriggered: HomeScreen.launch(text)
        }

        Action {
            readonly property var keywords: ["recover", "restore"]
            readonly property bool allowShortcut: true
            property bool visible: Scrite.document.backupFilesModel.count > 0

            enabled: Runtime.allowAppUsage && visible
            objectName: "backupOpen"
            text: "Backup"

            icon.source: "qrc:/icons/file/backup_open.png"

            onTriggered: BackupsDialog.launch()
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false

            enabled: Scrite.document.backupFilesModel.count > 0
            objectName: "revealBackupsFolder"
            text: "Reveal Backups Folder"

            onTriggered: File.revealOnDesktop(Scrite.document.backupFilesModel.backupsFolder)
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false
            readonly property var keywords: ["vault"]

            objectName: "recoveryVault"
            text: "Recover"

            onTriggered: HomeScreen.launch(text)
        }

        Action {
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Save)

            enabled: Runtime.allowAppUsage && (Scrite.document.modified || Scrite.document.fileName === "") && !Scrite.document.readOnly
            objectName: "fileSave"
            shortcut: defaultShortcut
            text: "Save"

            icon.source: "qrc:/icons/content/save.png"

            onTriggered: {
                if(Scrite.document.fileName === "")
                SaveFileTask.saveAs()
                else
                SaveFileTask.saveSilently()
            }
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+S"

            enabled: Runtime.allowAppUsage
            objectName: "fileSaveAs"
            shortcut: defaultShortcut
            text: "Save As"

            icon.source: "qrc:/icons/file/file_download.png"

            onTriggered: SaveFileTask.saveAs()
        }

        Action {
            readonly property var keywords: ["print"]
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Print)

            text: "Export To PDF"
            enabled: Runtime.allowAppUsage
            shortcut: defaultShortcut

            icon.source: "qrc:/icons/exporter/pdf.png"

            onTriggered: ExportConfigurationDialog.launch("Screenplay/Adobe PDF")
        }

        // Action to reveal vault folder
        // Action to reveal backup folder
        // Action to reveal settings folder
    }

    readonly property ActionManager recentFileOperations: ActionManager {
        title: "Open Recent"
        objectName: "recentFileOperations"
    }

    Instantiator {
        model: Runtime.recentFiles

        delegate: Action {
            required property int index
            required property var fileInfo

            readonly property bool visible: true
            property var keywords: [fileInfo.subtitle, fileInfo.author, fileInfo.logline]
            property string tooltip: fileInfo.filePath

            ActionManager.target: root.recentFileOperations

            ImageIcon.image: fileInfo.hasCoverPage ? fileInfo.coverPageImage : Gui.emptyQImage

            text: {
                let ret = ""
                if(fileInfo.title !== "") {
                    ret += fileInfo.title

                    if(fileInfo.version !== "")
                    ret += " (" + fileInfo.version + ")"
                } else
                ret = fileInfo.baseFileName
                return ret
            }
            enabled: Scrite.document.fileName !== fileInfo.filePath

            icon.color: "transparent"
            icon.source: fileInfo.hasCoverPage ? ImageIcon.url : "qrc:/icons/filetype/document.png"

            onTriggered: {
                SaveFileTask.save( () => {
                                        OpenFileTask.open(fileInfo.filePath)
                                    } )
            }
        }
    }

    readonly property ActionManager templateOperations: ActionManager {
        title: "Templates"
        objectName: "templateOperations"

        Action {
            readonly property bool visible: true
            readonly property string tooltip: "Create a new screenplay by interpreting text on the clipboard as fountain file."

            enabled: Scrite.document.canImportFromClipboard
            text: "New from Clipboard"
            icon.source: "qrc:/icons/filetype/document.png"

            onTriggered: {
                SaveFileTask.save( () => {
                                        Scrite.document.importFromClipboard()
                                    } )
            }
        }
    }

    Instantiator {
        model: DelayedProperty.value ? Runtime.libraryService.templates : []

        /*
          During application start, the appFeatures and libraryService may change their enabled
          and busy status several times in a short span. Instead of reacting to each change
          and creating/destroying actions, we delay responding to those changes by 100ms (which
          is the default delay of DelayedProperty), and handle all changes at once.
          */
        DelayedProperty.watch: Runtime.appFeatures.templates.enabled && !Runtime.libraryService.busy
        DelayedProperty.initial: false

        delegate: Action {
            required property int index
            required property var record

            readonly property bool visible: true
            property var keywords: index === 0 ? ["new", "close", "file", "document", "screenplay"] : []
            property string tooltip: record.description

            ActionManager.target: root.templateOperations

            text: record.name
            enabled: true

            icon.color: "transparent"
            icon.source: index === 0 ? record.poster : Runtime.libraryService.templates.baseUrl + "/" + record.poster

            onTriggered: {
                SaveFileTask.save( () => {
                                      OpenFromLibraryTask.openTemplateAt(Runtime.libraryService, index)
                                  })
            }
        }
    }

    readonly property ActionManager scriptalayOperations: ActionManager {
        title: "Scriptalay"
        objectName: "scriptalayOperations"
    }

    Instantiator {
        model: DelayedProperty.value ? Runtime.libraryService.screenplays : []

        /*
          During application start, the appFeatures and libraryService may change their enabled
          and busy status several times in a short span. Instead of reacting to each change
          and creating/destroying actions, we delay responding to those changes by 100ms (which
          is the default delay of DelayedProperty), and handle all changes at once.
          */
        DelayedProperty.watch: Runtime.appFeatures.scriptalay.enabled && !Runtime.libraryService.busy
        DelayedProperty.initial: false

        delegate: Action {
            required property int index
            required property var record

            readonly property bool visible: true
            property string tooltip: record.authors
            property var keywords: [record.logline]

            ActionManager.target: root.scriptalayOperations

            text: record.name
            enabled: true

            icon.color: "transparent"
            icon.source: Runtime.libraryService.screenplays.baseUrl + "/" + record.poster

            onTriggered: {
                SaveFileTask.save( () => {
                                      OpenFromLibraryTask.openScreenplayAt(Runtime.libraryService, index)
                                  })
            }
        }
    }

    readonly property ActionManager exportOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/action/share.png"

        title: "Export"
        objectName: "exportOptions"
    }

    Instantiator {
        model: Scrite.document.supportedExportFormats

        delegate: Action {
            required property var modelData // { className, name, icon, key, description, category, keywords }

            readonly property bool allowShortcut: true
            readonly property string tooltip: modelData.description
            readonly property string keywords: modelData.keywords

            ActionManager.target: root.exportOptions

            text: modelData.name
            enabled: Runtime.allowAppUsage
            objectName: "export" + modelData.className

            icon.source: "qrc" + modelData.icon

            onTriggered: ExportConfigurationDialog.launch(modelData.key)
        }
    }

    readonly property ActionManager reportOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/exporter/pdf.png"

        title: "Reports"
        objectName: "reportOptions"
    }

    Instantiator {
        model: Scrite.document.supportedReports

        delegate: Action {
            required property var modelData // { className, name, icon, description }

            readonly property bool allowShortcut: true
            readonly property string tooltip: modelData.description
            readonly property string keywords: modelData.keywords

            ActionManager.target: root.reportOptions

            text: modelData.name
            enabled: Runtime.allowAppUsage
            objectName: "report" + modelData.className

            icon.source: "qrc" + modelData.icon

            onTriggered: ReportConfigurationDialog.launch(modelData.name)
        }
    }

    readonly property ActionManager appOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/action/settings_applications.png"

        title: "Options"
        objectName: "appOptions"

        Action {
            readonly property var keywords: ["theme", "dark mode", "colors"]
            readonly property string defaultShortcut: "Ctrl+,"

            text: "Settings"
            objectName: "settings"
            shortcut: defaultShortcut

            icon.source: "qrc:/icons/action/settings_applications.png"

            onTriggered: SettingsDialog.launch()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+E"

            text: "Shortcuts"
            objectName: "shortcuts"
            shortcut: defaultShortcut

            icon.source: {
                if(Platform.isMacOSDesktop) {
                    return "qrc:/icons/navigation/shortcuts_macos.png"
                }
                if(Platform.isWindowsDesktop) {
                    return "qrc:/icons/navigation/shortcuts_windows.png"
                }
                return "qrc:/icons/navigation/shortcuts_linux.png"
            }

            onTriggered: ShortcutEditorDialog.launch()
        }

        Action {
            readonly property bool hideInCommandCenter: true
            readonly property string defaultShortcut: "Ctrl+/"

            enabled: ActionHandler.canHandle
            objectName: "commandCenter"
            shortcut: defaultShortcut
            text: "Command Center"

            icon.source: "qrc:/icons/action/command_center.png"
        }

        Action {
            readonly property string defaultShortcut: "F7"

            text: "Toggle Fullscreen"
            objectName: "fullscreen"
            shortcut: defaultShortcut

            icon.source: "qrc:/icons/navigation/fullscreen.png"

            onTriggered: Scrite.app.toggleFullscreen(Scrite.window)
        }

        Action {
            readonly property bool allowShortcut: true

            text: "About"
            objectName: "about"

            icon.source: "qrc:/icons/action/info.png"

            onTriggered: AboutDialog.launch()
        }
    }

    readonly property ActionManager helpSupportOptions: ActionManager {
        title: "Help & Support"
        objectName: "helpSupport"

        Action {
            readonly property string defaultShortcut: "F1"

            text: "User Guides"
            objectName: "userGuides"
            shortcut: defaultShortcut

            icon.source: "qrc:/icons/action/help.png"

            onTriggered: Qt.openUrlExternally("https://www.scrite.io/help/")
        }

        Action {
            readonly property var keywords: ["mobile", "device count", "cloud", "realtime collaboration"]

            text: "FAQs"

            onTriggered: Qt.openUrlExternally("https://www.scrite.io/faqs/")
        }

        Action {
            readonly property var keywords: ["support", "community", "question", "bug", "feedback", "feature request"]

            text: "Discord"

            onTriggered: Qt.openUrlExternally("https://www.scrite.io/forum/")
        }

        Action {
            readonly property var keywords: ["terms", "privacy", "refund"]

            text: "Policies"

            onTriggered: Qt.openUrlExternally("https://www.scrite.io/policies/")
        }

        Action {
            readonly property var keywords: ["contact", "phone"]

            text: "Email Support"

            onTriggered: {
                if(Runtime.appFeatures.emailSupport.enabled)
                    Qt.openUrlExternally("mailto:support@scrite.io")
                else
                    MessageBox.information("No Email Support", "Your subscription plan does not include email support. Please seek support on Discord.")
            }
        }

        Action {
            readonly property var keywords: ["github"]

            text: "Source Code"

            onTriggered: {
                Qt.openUrlExternally("https://github.com/teriflix/scrite")
            }
        }
    }

    readonly property ActionManager languageOptions: ActionManager {
        property string iconSource: LanguageEngine.supportedLanguages.activeLanguage.iconSource

        title: "Languages"
        objectName: "languageOptions"

        Action {
            readonly property var keywords: ["add language", "input methods", "input tools", "baraha", "nudi", "ism", "font"]
            property int sortOrder: LanguageEngine.supportedLanguages.count + 1

            text: "More Languages ..."

            onTriggered: LanguageOptionsDialog.launch()
        }
    }

    Instantiator {
        model: LanguageEngine.supportedLanguages

        delegate: Action {
            required property int index
            required property var language // This is of type Language, but we have to use var here.
            // You cannot use Q_GADGET struct names as type names in QML
            // that privilege is only reserved for QObject types.

            property int sortOrder: index

            ActionManager.target: root.languageOptions

            checkable: true
            checked: Runtime.language.activeCode === language.code
            shortcut: language.shortcut()
            text: language.name

            icon.source: language.iconSource

            onTriggered: Runtime.language.setActiveCode(language.code)
        }
    }

    readonly property ActionManager inputOptions: ActionManager {
        title: "Input"
        objectName: "inputOptions"

        Action {
            readonly property string defaultShortcut: "Ctrl+K"
            property bool visible: enabled

            property string tooltip: "Show English to " + Runtime.language.active.name + " alphabet mappings."

            enabled: Runtime.language.activeCode !== QtLocale.English &&
                     Runtime.language.activeTransliterator.name === DefaultTransliteration.driver &&
                     DefaultTransliteration.supportsLanguageCode(Runtime.language.activeCode)
            shortcut: defaultShortcut
            text: "Alphabet Mappings"
            objectName: "alphabetMappings"

            icon.source: "qrc:/icons/hardware/keyboard.png"
        }
    }

    readonly property ActionManager paragraphFormats: ActionManager {
        function setBinder(binder) { root.setBinder(binder) }
        function resetBinder(binder) { root.resetBinder(binder) }

        title: "Paragraph Format"
        objectName: "paragraphFormats"

        Action {
            enabled: ActionHandler.canHandle
            objectName: "nextFormat"
            shortcut: "Tab"
            text: "Next Format"

            icon.source: "qrc:/icons/action/keyboard_tab.png"
        }
    }

    Instantiator {
        id: _paragraphFormatActions

        model: _private.availableParagraphFormats

        delegate: Action {
            required property int index
            required property int enumValue
            required property string enumKey
            required property string enumIcon

            property int sortOrder: enumValue === SceneElement.Heading ? 0 : (index+1)
            property string defaultShortcut: "Ctrl+" + sortOrder

            ActionManager.target: root.paragraphFormats

            checkable: true
            checked: (_private.binder !== null ? (_private.binder.currentElement ? _private.binder.currentElement.type === enumValue : false) : false)
            enabled: Runtime.allowAppUsage && (enumValue === SceneElement.Heading ? ActionHandler.canHandle : _private.binder !== null)
            objectName: enumKey.toLowerCase() + "Paragraph"
            shortcut: defaultShortcut
            text: enumKey

            icon.source: enumIcon

            onTriggered: {
                // When index=0, its scene heading and that's handled separately.
                if(enumValue !== SceneElement.Heading)
                _private.binder.currentElement.type = enumValue
            }
        }
    }

    readonly property ActionManager editOptions: ActionManager {
        title: "Scene Editor"
        objectName: "editOptions"

        Action {
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Find)

            enabled: ActionHandler.canHandle
            objectName: "find"
            shortcut: defaultShortcut
            text: "Find"

            icon.source: "qrc:/icons/action/search.png"
        }

        Action {
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Replace)
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "replace"
            shortcut: defaultShortcut
            text: "Replace"

            icon.source: "qrc:/icons/action/find_replace.png"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "selectAll"
            shortcut: Gui.standardShortcut(StandardKey.SelectAll)
            text: "Select All"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "cut"
            shortcut: Gui.standardShortcut(StandardKey.Cut)
            text: "Cut"
        }

        Action {
            property bool visible: ActionHandler.canHandle

            enabled: ActionHandler.canHandle
            objectName: "copy"
            shortcut: Gui.standardShortcut(StandardKey.Copy)
            text: "Copy"

            icon.source: "qrc:/icons/content/content_copy.png"
        }

        Action {
            property bool visible: ActionHandler.canHandle

            enabled: ActionHandler.canHandle
            objectName: "paste"
            shortcut: Gui.standardShortcut(StandardKey.Paste)
            text: "Paste"

            icon.source: "qrc:/icons/content/content_paste.png"
        }

        Action {
            property bool visible: ActionHandler.canHandle

            enabled: true
            objectName: "undo"
            shortcut: Gui.standardShortcut(StandardKey.Undo)
            text: "Undo"

            icon.source: "qrc:/icons/content/undo.png"

            onTriggered: (source) => {
                if(!ActionHandler.canHandle) {
                    UndoHub.undo()
                }
            }
        }

        Action {
            property bool visible: ActionHandler.canHandle

            enabled: true
            objectName: "redo"
            shortcut: Gui.standardShortcut(StandardKey.Redo)
            text: "Redo"

            icon.source: "qrc:/icons/content/redo.png"

            onTriggered: (source) => {
                if(!ActionHandler.canHandle) {
                    UndoHub.redo()
                }
            }
        }

        Action {
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Refresh)

            enabled: ActionHandler.canHandle
            objectName: "reload"
            shortcut: defaultShortcut
            text: "Reload"

            icon.source: "qrc:/icons/navigation/refresh.png"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "F3"

            enabled: ActionHandler.canHandle
            objectName: "smileysAndSymbols"
            shortcut: defaultShortcut
            text: "Symbols & Smileys"                            
        }

        Action {
            readonly property string defaultShortcut: Platform.isMacOSDesktop ? "Ctrl+Shift+Return" : "Ctrl+Shift+Enter"

            enabled: ActionHandler.canHandle
            objectName: "splitScene"
            shortcut: defaultShortcut
            text: "Split Scene"

            icon.source: "qrc:/icons/action/split_scene.png"
        }

        Action {
            readonly property string defaultShortcut: Platform.isMacOSDesktop ? "Ctrl+Shift+Delete" : "Ctrl+Shift+Backspace"

            enabled: ActionHandler.canHandle
            objectName: "mergeScene"
            shortcut: defaultShortcut
            text: "Join Previous Scene"

            icon.source: "qrc:/icons/action/merge_scene.png"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Alt+,"

            enabled: ActionHandler.canHandle
            checkable: true
            checked: ActionHandler.active ? ActionHandler.active.checked : false
            objectName: "toggleCommentsPanel"
            shortcut: defaultShortcut
            text: "Show/Hide Comments Panel"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Alt+."

            enabled: ActionHandler.canHandle
            objectName: "cycleCommandPanelTab"
            shortcut: defaultShortcut
            text: "Cycle Command Panel Tab"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+`"

            enabled: ActionHandler.canHandle
            objectName: "editSceneContent"
            shortcut: defaultShortcut
            text: "Edit Scene Content"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+7"

            enabled: ActionHandler.canHandle
            objectName: "addMuteCharacter"
            shortcut: defaultShortcut
            text: "Add Mute Character"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Shift+7"

            enabled: ActionHandler.canHandle
            objectName: "addOpenTag"
            shortcut: defaultShortcut
            text: "Add Keyword / Open Tag"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+8"

            enabled: ActionHandler.canHandle
            objectName: "editSceneSynopsis"
            shortcut: defaultShortcut
            text: "Edit Scene Synopsis"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Alt+0"

            enabled: ActionHandler.canHandle
            objectName: "editSceneNumber"
            shortcut: defaultShortcut
            text: "Edit Scene Number"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.ControlModifier+Qt.AltModifier+Qt.Key_Up)

            enabled: ActionHandler.canHandle
            objectName: "jumpFirstScene"
            shortcut: defaultShortcut
            text: "First Scene"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.ControlModifier+Qt.AltModifier+Qt.Key_Down)

            enabled: ActionHandler.canHandle
            objectName: "jumpLastScene"
            shortcut: defaultShortcut
            text: "Last Scene"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Platform.isMacOSDesktop ? Gui.shortcut(Qt.AltModifier+Qt.Key_Up) : Gui.shortcut(Qt.ControlModifier+Qt.Key_Up)

            enabled: ActionHandler.canHandle
            objectName: "jumpPreviousScene"
            shortcut: defaultShortcut
            text: "Previous Scene"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Platform.isMacOSDesktop ? Gui.shortcut(Qt.AltModifier+Qt.Key_Down) : Gui.shortcut(Qt.ControlModifier+Qt.Key_Down)

            enabled: ActionHandler.canHandle
            objectName: "jumpNextScene"
            shortcut: defaultShortcut
            text: "Next Scene"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "scrollPreviousScene"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "scrollNextScene"
        }

        // This is the main jump-to-scene handler components in the app
        // can hook to and handle. If nobody is hooking up to this handle, then
        // we offer a global one below.
        Action {
            id: _jumpToSceneNumber

            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+G"

            enabled: ActionHandler.canHandle
            objectName: "jumpToSceneNumber"
            shortcut: defaultShortcut
            text: "Jump to Scene Number"
        }

        // This is the global jump-to-scene handler
        Action {
            readonly property bool visible: false

            enabled: !_jumpToSceneNumber.enabled
            shortcut: _jumpToSceneNumber.shortcut

            onTriggered: (source) => {
                JumpToSceneNumberDialog.launch(Runtime.screenplayAdapter)
            }
        }

        Action {
            readonly property bool visible: false

            property int cursorPosition: -2
            property int sceneElementIndex: -1

            function set(idx, pos) {
                sceneElementIndex = idx
                cursorPosition = pos
            }

            function get(idx) {
                if(sceneElementIndex === idx) {
                    const ret = cursorPosition
                    cursorPosition = -2
                    sceneElementIndex = -1
                    return ret
                }
                return -2
            }

            enabled: false
            objectName: "focusCursorPosition"
        }
    }

    readonly property ActionManager markupTools: ActionManager {
        function setBinder(binder) { root.setBinder(binder) }
        function resetBinder(binder) { root.resetBinder(binder) }

        title: "Markup Tools"
        objectName: "markupTools"

        Action {
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Bold)

            checkable: true
            checked: _private.textFormat ? _private.textFormat.bold : false
            enabled: _private.textFormat && Runtime.allowAppUsage
            objectName: "bold"
            shortcut: defaultShortcut
            text: "Bold"

            icon.source: "qrc:/icons/editor/format_bold.png"

            onTriggered: _private.textFormat.toggleBold()
        }

        Action {
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Italic)

            enabled: _private.textFormat && Runtime.allowAppUsage
            shortcut: defaultShortcut
            checkable: true
            checked: _private.textFormat ? _private.textFormat.italics : false
            objectName: "italics"
            text: "Italics"

            icon.source: "qrc:/icons/editor/format_italics.png"

            onTriggered: _private.textFormat.toggleItalics()
        }

        Action {
            readonly property string defaultShortcut: Gui.standardShortcut(StandardKey.Underline)

            enabled: _private.textFormat && Runtime.allowAppUsage
            shortcut: defaultShortcut
            checkable: true
            checked: _private.textFormat ? _private.textFormat.underline : false
            objectName: "underline"
            text: "Underline"

            icon.source: "qrc:/icons/editor/format_underline.png"

            onTriggered: _private.textFormat.toggleUnderline()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+R"

            enabled: _private.textFormat && Runtime.allowAppUsage
            shortcut: defaultShortcut
            checkable: true
            checked: _private.textFormat ? _private.textFormat.strikeout : false
            objectName: "strikeout"
            text: "Strikeout"

            icon.source: "qrc:/icons/editor/format_strikethrough.png"

            onTriggered: _private.textFormat.toggleStrikeout()
        }

        Action {
            property bool down: popup !== null
            property QtObject popup

            enabled: _private.textFormat && Runtime.allowAppUsage
            objectName: "colors"
            text: "Colors"

            onTriggered: (source) => {
                if(popup)
                    popup.destroy()

                popup = _private.textColorsPopup.createObject(source, {"source": source, "textFormat": _private.textFormat})
                if(popup) {
                    popup.closed.connect(popup.destroy)
                    popup.open()
                }
            }
        }

        Action {
            enabled: _private.textFormat && Runtime.allowAppUsage
            objectName: "clear"
            text: "Clear Formatting"

            icon.source: "qrc:/icons/editor/format_clear.png"

            onTriggered: _private.textFormat.reset()
        }

        Action {
            readonly property bool allowShortcut: true

            enabled: _private.sceneElement && Runtime.allowAppUsage
            checkable: true
            checked: _private.sceneElement ? _private.sceneElement.alignment === Qt.AlignLeft : false
            objectName: "alignLeft"
            text: "Left Align"

            icon.source: "qrc:/icons/editor/format_align_left.png"

            onTriggered: _private.toggleSelectedElementsAlignment(Qt.AlignLeft)
        }

        Action {
            readonly property bool allowShortcut: true

            enabled: _private.sceneElement && Runtime.allowAppUsage
            checkable: true
            checked: _private.sceneElement ? _private.sceneElement.alignment === Qt.AlignHCenter : false
            objectName: "alignCenter"
            text: "Center Align"

            icon.source: "qrc:/icons/editor/format_align_center.png"

            onTriggered: _private.toggleSelectedElementsAlignment(Qt.AlignHCenter)
        }

        Action {
            readonly property bool allowShortcut: true

            enabled: _private.sceneElement && Runtime.allowAppUsage
            checkable: true
            checked: _private.sceneElement ? _private.sceneElement.alignment === Qt.AlignRight : false
            objectName: "alignRight"
            text: "Right Align"

            icon.source: "qrc:/icons/editor/format_align_right.png"

            onTriggered: _private.toggleSelectedElementsAlignment(Qt.AlignRight)
        }

        Action {
            readonly property string tooltip: "ALL CAPS"
            readonly property string defaultShortcut: "Shift+F3"

            enabled: _private.binder && Runtime.allowAppUsage
            text: "AB"
            objectName: "toUppercase"

            onTriggered: _private.binder.changeTextCase(SceneDocumentBinder.UpperCase)
        }

        Action {
            readonly property string tooltip: "all small"
            readonly property string defaultShortcut: "Ctrl+Shift+F3"

            enabled: _private.binder && Runtime.allowAppUsage
            text: "ab"
            objectName: "toLowercase"

            onTriggered: _private.binder.changeTextCase(SceneDocumentBinder.LowerCase)
        }
    }

    readonly property ActionManager sceneListPanelOptions: ActionManager {
        title: "Scene List Panel"
        objectName: "sceneListPanelOptions"

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Alt+0"

            objectName: "sidePanelVisibility"
            checkable: true
            checked: ActionHandler.active ? ActionHandler.active.checked : false
            enabled: ActionHandler.canHandle
            shortcut: defaultShortcut
            text: "Display Panel"
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false

            objectName: "displayScreenplayTracks"
            checkable: true
            checked: ActionHandler.active ? ActionHandler.active.checked : false
            enabled: ActionHandler.canHandle
            text: "Display Screenplay Tracks"
        }
    }

    readonly property ActionManager screenplayEditorOptions: ActionManager {
        title: "Screenplay Editor"
        objectName: "screenplayEditorOptions"

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayRuler
            text: "Show Ruler"

            onToggled: Runtime.screenplayEditorSettings.displayRuler = !Runtime.screenplayEditorSettings.displayRuler
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.showLoglineEditor
            text: "Show Logline Editor"

            onToggled: Runtime.screenplayEditorSettings.showLoglineEditor = !Runtime.screenplayEditorSettings.showLoglineEditor
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayEmptyTitleCard
            text: "Display Empty Title Card"

            onToggled: Runtime.screenplayEditorSettings.displayEmptyTitleCard = !Runtime.screenplayEditorSettings.displayEmptyTitleCard
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons
            text: "Show Act, Episode, Scene Controls"

            onToggled: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons = !Runtime.screenplayEditorSettings.displayAddSceneBreakButtons
        }

        Action {
            property bool __sceneBlocksVisible: Runtime.screenplayEditorSettings.spaceBetweenScenes > 0

            checkable: true
            checked: __sceneBlocksVisible
            text: "Show Scene Blocks"

            onToggled: Runtime.screenplayEditorSettings.spaceBetweenScenes = __sceneBlocksVisible ? 0 : 40
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.markupToolsDockVisible
            text: "Show Markup Tools"

            onToggled: Runtime.screenplayEditorSettings.markupToolsDockVisible = !Runtime.screenplayEditorSettings.markupToolsDockVisible
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+S"

            checkable: true
            checked: Runtime.screenplayEditorSettings.displaySceneSynopsis
            shortcut: defaultShortcut
            objectName: "synopsis"
            text: "Show Synopsis in Editor"

            onToggled: Runtime.screenplayEditorSettings.displaySceneSynopsis = !Runtime.screenplayEditorSettings.displaySceneSynopsis
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+M"

            checkable: true
            checked: Runtime.screenplayEditorSettings.displaySceneComments
            shortcut: defaultShortcut
            objectName: "comments"
            text: "Show Comments Panel"

            onToggled: Runtime.screenplayEditorSettings.displaySceneComments = !Runtime.screenplayEditorSettings.displaySceneComments
        }


        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayIndexCardFields
            enabled: Runtime.screenplayEditorSettings.displaySceneComments
            text: "Show Index Card Fields in Comments Panel"

            onToggled: Runtime.screenplayEditorSettings.displayIndexCardFields = !Runtime.screenplayEditorSettings.displayIndexCardFields
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+C"

            checkable: true
            checked: Runtime.screenplayEditorSettings.displaySceneCharacters
            shortcut: defaultShortcut
            objectName: "charactersAndTags"
            text: "Show Characters and Tags in Editor"

            onToggled: Runtime.screenplayEditorSettings.displaySceneCharacters = !Runtime.screenplayEditorSettings.displaySceneCharacters
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+G"

            checkable: true
            checked: Runtime.screenplayEditorSettings.allowTaggingOfScenes
            shortcut: defaultShortcut
            objectName: "tagging"
            text: "Allow Structure Tags"

            onToggled: Runtime.screenplayEditorSettings.allowTaggingOfScenes = !Runtime.screenplayEditorSettings.allowTaggingOfScenes
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+L"

            checkable: true
            checked: Runtime.screenplayEditorSettings.enableSpellCheck
            shortcut: defaultShortcut
            objectName: "spellCheck"
            text: "Enable Spell Check"

            onToggled: Runtime.screenplayEditorSettings.enableSpellCheck = !Runtime.screenplayEditorSettings.enableSpellCheck
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+H"

            checkable: true
            checked: Runtime.screenplayEditorSettings.highlightCurrentLine
            shortcut: defaultShortcut
            objectName: "lineHighlight"
            text: "Show Current Line Highlight"

            onToggled: Runtime.screenplayEditorSettings.highlightCurrentLine = !Runtime.screenplayEditorSettings.highlightCurrentLine
        }

        Action {
            readonly property bool visible: false
            readonly property bool allowShortcut: true

            objectName: "scanMuteCharacters"
            text: "Scan Mute Characters"
            enabled: !Scrite.document.readOnly

            onTriggered: Scrite.document.structure.scanForMuteCharacters()
        }

        Action {
            readonly property bool visible: false
            readonly property bool allowShortcut: true

            objectName: "resetSceneNumbers"
            text: "Reset Scene Numbers"
            enabled: !Scrite.document.readOnly

            onTriggered: Scrite.document.structure.resetSceneNumbers()
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+="

            enabled: ActionHandler.canHandle
            objectName: "zoomIn"
            shortcut: defaultShortcut
            text: "Zoom In"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+-"

            enabled: ActionHandler.canHandle
            objectName: "zoomOut"
            shortcut: defaultShortcut
            text: "Zoom Out"
        }
    }

    readonly property ActionManager screenplayOperations: ActionManager {
        title: "Screenplay"
        objectName: "screenplayOperations"

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+T"

            enabled: !Scrite.document.readOnly
            shortcut: defaultShortcut
            text: "Title Page"
            objectName: "titlePage"

            icon.source: "qrc:/icons/action/edit_title_page.png"

            onTriggered: TitlePageDialog.launch()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+N"

            enabled: !Scrite.document.readOnly
            shortcut: defaultShortcut
            text: "Add/Insert New Scene"
            objectName: "newScene"

            icon.source: "qrc:/icons/action/add_scene.png"

            onTriggered: _private.addScene()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+B"

            enabled: !Scrite.document.readOnly
            shortcut: defaultShortcut
            text: "Add/Insert Act Break"
            objectName: "actBreak"

            icon.source: "qrc:/icons/action/add_act.png"

            onTriggered: _private.addAct()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+P"

            enabled: !Scrite.document.readOnly
            shortcut: defaultShortcut
            text: "Add/Insert Episode Break"
            objectName: "episodeBreak"

            icon.source: "qrc:/icons/action/add_episode.png"

            onTriggered: _private.addEpisode()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+L"
            readonly property bool visible: false

            enabled: !Scrite.document.readOnly
            shortcut: defaultShortcut
            text: "Add/Insert Interval Break"
            objectName: "intervalBreak"

            onTriggered: _private.addInterval()
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false
            readonly property var keywords: ["watermark", "header", "footer", "page size"]

            text: "Page Setup & Watermark"
            objectName: "pageSetup"

            onTriggered: ScreenplayPageSetupDialog.launch()
        }
    }

    readonly property ActionManager structureCanvasOperations: ActionManager {
        title: "Structure"
        objectName: "structureCanvasOperations"

        Action {
            readonly property bool allowShortcut: true
            readonly property bool hideInCommandCenter: true

            property bool down: ActionHandler.canHandle ? ActionHandler.active.down : false

            enabled: ActionHandler.canHandle
            objectName: "newScene"
            text: "New Scene"

            icon.source: "qrc:/icons/action/add_scene.png"
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool hideInCommandCenter: true

            property bool down: ActionHandler.canHandle ? ActionHandler.active.down : false

            enabled: ActionHandler.canHandle
            objectName: "newAnnotation"
            text: "New Annotation"

            icon.source: "qrc:/icons/action/add_annotation.png"
        }

        Action {
            readonly property bool allowShortcut: true

            checkable: true
            checked: ActionHandler.canHandle ? ActionHandler.active.checked : false
            enabled: ActionHandler.canHandle
            objectName: "selectionMode"
            text: "Selection Mode"

            icon.source: "qrc:/icons/action/selection_drag.png"
        }

        Action {
            readonly property bool allowShortcut: true

            enabled: ActionHandler.canHandle
            objectName: "selectAll"
            text: "Select All"

            icon.source: "qrc:/icons/content/select_all.png"
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool hideInCommandCenter: true

            enabled: ActionHandler.canHandle
            objectName: "layout"
            text: "Layout Options"

            icon.source: "qrc:/icons/action/layout_options.png"
        }

        Action {
            readonly property bool allowShortcut: true

            checkable: true
            checked: ActionHandler.canHandle ? ActionHandler.active.checked : false
            enabled: ActionHandler.canHandle
            objectName: "beatBoardLayout"
            text: "Beat Board Layout"

            icon.source: "qrc:/icons/action/layout_beat_sheet.png"
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool hideInCommandCenter: true

            property bool down: ActionHandler.canHandle ? ActionHandler.active.down : false

            enabled: ActionHandler.canHandle
            objectName: "grouping"
            text: "Grouping Options"

            icon.source: "qrc:/icons/action/layout_grouping.png"
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool hideInCommandCenter: true

            property bool down: ActionHandler.canHandle ? ActionHandler.active.down : false

            enabled: ActionHandler.canHandle
            objectName: "tag"
            text: "Tag"

            icon.source: "qrc:/icons/action/tag.png"
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool hideInCommandCenter: true

            property bool down: ActionHandler.canHandle ? ActionHandler.active.down : false

            enabled: ActionHandler.canHandle
            objectName: "sceneColor"
            text: "Color"

            icon.source: ActionHandler.canHandle ? ActionHandler.active.iconSource : "image://color/gray/1"
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool hideInCommandCenter: true
            readonly property string tooltip: "Scene Type (Action/Montage/Song)"
            property bool down: ActionHandler.canHandle ? ActionHandler.active.down : false

            enabled: ActionHandler.canHandle
            objectName: "sceneType"
            text: "Type"

            icon.source: ActionHandler.canHandle ? ActionHandler.active.iconSource : "qrc:/icons/content/standard_scene.png"
        }

        Action {
            readonly property bool allowShortcut: true
            enabled: ActionHandler.canHandle
            objectName: "delete"
            text: "Delete"

            icon.source: "qrc:/icons/action/delete.png"
        }

        Action {
            property Action editCopy: editOptions.find("copy")

            enabled: ActionHandler.canHandle
            objectName: "copy"
            shortcut: editCopy.shortcut
            text: "Copy"

            icon.source: "qrc:/icons/content/content_copy.png"
        }

        Action {
            property Action editPaste: editOptions.find("paste")

            enabled: ActionHandler.canHandle
            objectName: "paste"
            shortcut: editPaste.shortcut
            text: "Paste"

            icon.source: "qrc:/icons/content/content_paste.png"
        }

        Action {
            readonly property bool allowShortcut: true

            enabled: ActionHandler.canHandle
            objectName: "pdfExport"
            text: "Structure Canvas PDF"

            icon.source: "qrc:/icons/file/generate_pdf.png"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Alt+="

            enabled: ActionHandler.canHandle
            objectName: "zoomIn"
            shortcut: defaultShortcut
            text: "Zoom In"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Alt+-"

            enabled: ActionHandler.canHandle
            objectName: "zoomOut"
            shortcut: defaultShortcut
            text: "Zoom Out"
        }
    }

    readonly property ActionManager timelineOperations: ActionManager {
        title: "Timeline"
        objectName: "timelineOperations"

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Alt+="

            enabled: ActionHandler.canHandle
            objectName: "zoomIn"
            shortcut: defaultShortcut
            text: "Zoom In"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Alt+-"

            enabled: ActionHandler.canHandle
            objectName: "zoomOut"
            shortcut: defaultShortcut
            text: "Zoom Out"
        }
    }

    readonly property ActionManager storyStructureOptions: ActionManager {
        title: "Story Structure"
        objectName: "storyStructureOptions"

        Action {
            readonly property bool visible: false
            readonly property bool allowShortcut: true

            objectName: "customizeGrouping"
            text: "Customize Beats"

            onTriggered: StructureStoryBeatsDialog.launch()
        }

        Action {
            readonly property bool visible: false
            readonly property bool allowShortcut: true

            objectName: "customizeIndexCardFields"
            text: "Customize Index Card Fields"

            onTriggered: StructureIndexCardFieldsDialog.launch()
        }

        Action {
            enabled: Runtime.mainWindowTab === Runtime.MainWindowTab.StructureTab
            checkable: true
            checked: Scrite.document.structure.preferredGroupCategory === ""
            text: "Acts"

            onToggled: {
                if(checked)
                    Scrite.document.structure.preferredGroupCategory = ""
            }
        }
    }

    Instantiator {
        model: Scrite.document.structure.groupCategories

        delegate: Action {
            required property string modelData

            ActionManager.target: root.storyStructureOptions

            checkable: true
            checked: Scrite.document.structure.preferredGroupCategory === modelData
            text: SMath.titleCased(modelData)

            onToggled: {
                if(checked) {
                    Scrite.document.structure.preferredGroupCategory = modelData
                }
            }
        }
    }

    readonly property ActionManager notebookOperations: ActionManager {
        title: "Notebook"
        objectName: "notebookOperations"

        Action {
            readonly property bool allowShortcut: true
            readonly property string tooltip: "If checked; episodes, acts and scenes selected on the notebook will be made current in screenplay editor & timeline"

            checkable: true
            checked: ActionHandler.active ? ActionHandler.active.checked : false
            enabled: ActionHandler.canHandle
            objectName: "sync"
            text: "Sync"

            icon.source: "qrc:/icons/navigation/sync.png"
        }

        Action {
            // We won't provide shortcut here, because NotebookView is expected
            // to handle editOptions.refresh which already has F5 mapped to it.
            readonly property bool allowShortcut: true
            readonly property string tooltip: "Reloads the notebook tree."
            readonly property int triggerMethod: ActionHandler.TriggerFirst

            enabled: ActionHandler.canHandle
            objectName: "reload"
            text: "Reload"

            icon.source: "qrc:/icons/navigation/refresh.png"
        }

        Action {
            // This is different from the action we have for generating PDF
            // export of the screenplay itself. This is specifically for
            // generating PDF export of the Notebook report.
            readonly property bool allowShortcut: true
            readonly property var keywords: ["pdf"]
            readonly property int triggerMethod: ActionHandler.TriggerFirst

            property string tooltip: ActionHandler.active ? ActionHandler.active.tooltip : text

            enabled: ActionHandler.canHandle
            objectName: "report"
            text: "Notebook Report"

            icon.source: "qrc:/icons/file/generate_pdf.png"
        }

        Action {
            property bool down: ActionHandler.active ? ActionHandler.active.down : false
            property string tooltip: ActionHandler.active ? ActionHandler.active.tooltip : "New text or form note."
            readonly property string defaultShortcut: "Ctrl+T"

            enabled: ActionHandler.canHandle
            objectName: "addNote"
            shortcut: defaultShortcut
            text: "Add Note"

            icon.source: "qrc:/icons/action/note_add.png"
        }

        Action {
            readonly property bool allowShortcut: true
            property bool down: ActionHandler.active ? ActionHandler.active.down : false

            enabled: ActionHandler.canHandle
            objectName: "noteColor"
            text: "Note Color"

            icon.color: "transparent"
            icon.source: ActionHandler.active ? ActionHandler.active.iconSource : "image://color/#ffffff/1"
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+D"
            property string tooltip: ActionHandler.active ? ActionHandler.active.tooltip : "Toggle bookmark of a Note, Scene, Episode/Act Notes or Character"

            enabled: ActionHandler.canHandle
            objectName: "toggleBookmark"
            shortcut: defaultShortcut
            text: "Toggle Bookmark"

            icon.source: ActionHandler.active ? ActionHandler.active.iconSource : "qrc:/icons/content/bookmark_outline.png"
        }

        Action {
            readonly property bool allowShortcut: true
            property string tooltip: ActionHandler.active ? ActionHandler.active.tooltip : "Delete the current note or character"

            enabled: ActionHandler.canHandle
            objectName: "delete"
            text: "Delete"

            icon.source: "qrc:/icons/action/delete.png"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Shift+K"
            property int triggerCount: 0

            enabled: Runtime.allowAppUsage && Runtime.appFeatures.notebook.enabled
            objectName: "bookmarkedNotes"
            shortcut: defaultShortcut
            text: "Bookmarked Notes"

            onTriggered: {
                Runtime.activateMainWindowTab(Runtime.NotebookTab)
                triggerCount = triggerCount+1
            }
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Shift+Y"
            property int triggerCount: 0

            enabled: Runtime.allowAppUsage && Runtime.appFeatures.notebook.enabled
            objectName: "storyNotes"
            shortcut: defaultShortcut
            text: "Story Notes"

            onTriggered: {
                Runtime.activateMainWindowTab(Runtime.NotebookTab)
                triggerCount = triggerCount+1
            }
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Shift+R"

            property int triggerCount: 0
            property string characterName

            enabled: Runtime.allowAppUsage && Runtime.appFeatures.notebook.enabled
            objectName: "characterNotes"
            shortcut: defaultShortcut
            text: "Character Notes"

            onTriggered: {
                Runtime.activateMainWindowTab(Runtime.NotebookTab)
                triggerCount = triggerCount+1
            }
        }
    }

    readonly property ActionManager scritedOptions : ActionManager {
        title: "Scrited"
        objectName: "scritedOptions"

        Action {
            readonly property string tooltip: "Load a video file for this screenplay"

            enabled: ActionHandler.canHandle
            objectName: "loadMovie"
            text: "Load Movie"

            icon.source: "qrc:/icons/mediaplayer/movie.png"
        }

        Action {
            readonly property string tooltip: "Toggle media playback"
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Space)

            checkable: true
            enabled: ActionHandler.canHandle
            objectName: "togglePlayback"
            shortcut: defaultShortcut
            text: checked ? "Pause" : "Play"

            icon.source: checked ? "qrc:/icons/mediaplayer/pause.png" : "qrc:/icons/mediaplayer/play_arrow.png"
        }

        Action {
            readonly property string tooltip: "Rewind 10 seconds"
            readonly property string defaultShortcut: Gui.shortcut(Qt.ControlModifier+Qt.Key_Left)

            enabled: ActionHandler.canHandle
            objectName: "rewind10"
            shortcut: defaultShortcut
            text: "Rewind 10s"

            icon.source: "qrc:/icons/mediaplayer/rewind_10.png"
        }

        Action {
            readonly property string tooltip: "Rewind one second"
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Left)

            enabled: ActionHandler.canHandle
            objectName: "rewind1"
            shortcut: defaultShortcut
            text: "Rewind 1s"

            icon.source: "qrc:/icons/mediaplayer/fast_rewind.png"
        }

        Action {
            readonly property string tooltip: "Forward one second"
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Right)

            enabled: ActionHandler.canHandle
            objectName: "forward1"
            shortcut: defaultShortcut
            text: "Forward 1s"

            icon.source: "qrc:/icons/mediaplayer/fast_forward.png"
        }

        Action {
            readonly property string tooltip: "Forward ten seconds"
            readonly property string defaultShortcut: Gui.shortcut(Qt.ControlModifier+Qt.Key_Right)

            enabled: ActionHandler.canHandle
            objectName: "forward10"
            shortcut: defaultShortcut
            text: "Forward 10s"

            icon.source: "qrc:/icons/mediaplayer/forward_10.png"
        }

        Action {
            readonly property string tooltip: "Previous Scene"
            readonly property string defaultShortcut: Gui.shortcut(Qt.ControlModifier+Qt.Key_Up)

            enabled: ActionHandler.canHandle
            objectName: "previousScene"
            shortcut: defaultShortcut
            text: "Previous Scene"

            icon.source: "qrc:/icons/action/keyboard_arrow_up.png"
        }

        Action {
            readonly property string tooltip: "Next Scene"
            readonly property string defaultShortcut: Gui.shortcut(Qt.ControlModifier+Qt.Key_Down)

            enabled: ActionHandler.canHandle
            objectName: "nextScene"
            shortcut: defaultShortcut
            text: "Next Scene"

            icon.source: "qrc:/icons/action/keyboard_arrow_down.png"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Up)

            enabled: ActionHandler.canHandle
            objectName: "scrollUp"
            shortcut: defaultShortcut
            text: "Scroll Up"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Down)

            enabled: ActionHandler.canHandle
            objectName: "scrollDown"
            shortcut: defaultShortcut
            text: "Scroll Down"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.AltModifier+Qt.Key_Up)

            enabled: ActionHandler.canHandle
            objectName: "previousPage"
            shortcut: defaultShortcut
            text: "Previous Page"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.AltModifier+Qt.Key_Down)

            enabled: ActionHandler.canHandle
            objectName: "nextPage"
            shortcut: defaultShortcut
            text: "Next Page"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.ShiftModifier+Qt.Key_Up)

            enabled: ActionHandler.canHandle
            objectName: "previousScreen"
            shortcut: defaultShortcut
            text: "Previous Screen"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.ShiftModifier+Qt.Key_Down)

            enabled: ActionHandler.canHandle
            objectName: "nextScreen"
            shortcut: defaultShortcut
            text: "Next Screen"
        }

        Action {
            readonly property string tooltip: "Use video time as current scene time offset"
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Greater)

            enabled: ActionHandler.canHandle
            objectName: "syncTime"
            shortcut: defaultShortcut
            text: "Sync Time"

            icon.source: "qrc:/icons/mediaplayer/sync_with_screenplay.png"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Greater)

            enabled: ActionHandler.canHandle
            objectName: "noteOffset"
            shortcut: defaultShortcut
            text: "Note Current Offset"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.ControlModifier+Qt.Key_Greater)

            enabled: ActionHandler.canHandle
            objectName: "adjustOffsets"
            shortcut: defaultShortcut
            text: "Note Current & Adjust Subsequent Offsets"
        }

        Action {
            readonly property string tooltip: "Reset time offset of all scenes."

            enabled: ActionHandler.canHandle
            objectName: "resetOffsets"
            text: "Reset Offsets"

            icon.source: "qrc:/icons/mediaplayer/reset_screenplay_offsets.png"
        }

        Action {
            readonly property string tooltip: "Toggle time column."

            checkable: true
            checked: false
            enabled: ActionHandler.canHandle
            objectName: "toggleTimeColumn"
            text: "Time Column"

            icon.source: "qrc:/icons/mediaplayer/time_column.png"
        }

        Action {
            readonly property string tooltip: "Check this to keep media playback and screenplay in sync."

            checkable: true
            checked: false
            enabled: ActionHandler.canHandle
            objectName: "autoScroll"
            text: "Auto Scroll"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "toggleSceneTimeLock"
            text: "Toggle Current Scene Time Lock"
            shortcut: "L"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "unlockAllSceneTimes"
            text: "Unlock All Scene Times"
            shortcut: "U"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "markStart"
            text: "Mark Start"
            shortcut: "S"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "markEnd"
            text: "Mark End"
            shortcut: "E"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "markKeyFrame"
            text: "Mark Key Frame"
            shortcut: "K"
        }

        Action {
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "adjustUnlockedTimes"
            text: "Adjust Unlocked Times"
            shortcut: "A"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_BraceLeft)

            enabled: ActionHandler.canHandle
            objectName: "decreaseVideoHeight"
            shortcut: defaultShortcut
            text: "Decrease Video Height"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_BraceRight)

            enabled: ActionHandler.canHandle
            objectName: "increaseVideoHeight"
            shortcut: defaultShortcut
            text: "Increase Video Height"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: Gui.shortcut(Qt.Key_Asterisk)

            enabled: ActionHandler.canHandle
            objectName: "resetVideoHeight"
            shortcut: defaultShortcut
            text: "Reset Video Height"
        }
    }

    readonly property ActionManager applicationOptions: ActionManager {
        title: "Application"
        objectName: "applicationOptions"

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+."

            enabled: ActionHandler.canHandle
            checkable: true
            checked: ActionHandler.active ? ActionHandler.active.checked : true
            text: "Toggle ToolBar Visibility"
            shortcut: defaultShortcut
            objectName: "toggleToolbarVisibility"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+M"

            text: "New Scrite Window"
            shortcut: defaultShortcut
            objectName: "newScriteWindow"

            onTriggered: Scrite.app.launchNewInstance(Scrite.window)
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Alt+A"

            checkable: true
            checked: Runtime.applicationSettings.enableAnimations
            text: (Runtime.applicationSettings.enableAnimations ? "Disable " : "Enable ") + "Animations"
            shortcut: defaultShortcut
            objectName: "animations"

            onToggled: Runtime.applicationSettings.enableAnimations = !Runtime.applicationSettings.enableAnimations
        }

        Action {
            readonly property var keywords: ["colors", "color intensity"]

            objectName: "configureScreenplayEditorOptions"
            text: "Screenplay Editor Options"
            icon.source: "qrc:/icons/content/view_options.png"

            onTriggered: ScreenplayEditorOptionsDialog.launch()
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false

            text: "Configure Screenplay Tracks"
            objectName: "configScreenplayTracks"

            onTriggered: ScreenplayTracksDialog.launch()
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false

            objectName: "displayScreenplayTracks"
            checkable: true
            checked: Runtime.screenplayTracksSettings.displayTracks
            enabled: Runtime.appFeatures.structure.enabled
            text: "Display Screenplay Tracks"

            onToggled: Runtime.screenplayTracksSettings.displayTracks = !Runtime.screenplayTracksSettings.displayTracks
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false

            objectName: "displayKeywordsTracks"
            checkable: true
            checked: Runtime.screenplayTracksSettings.displayKeywordsTracks
            enabled: Runtime.appFeatures.structure.enabled
            text: "Display Keywords Tracks"

            onToggled: Runtime.screenplayTracksSettings.displayKeywordsTracks = !Runtime.screenplayTracksSettings.displayKeywordsTracks
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false

            objectName: "displayStructureTracks"
            checkable: true
            checked: Runtime.screenplayTracksSettings.displayStructureTracks
            enabled: Runtime.appFeatures.structure.enabled
            text: "Display Structure Tracks"

            onToggled: Runtime.screenplayTracksSettings.displayStructureTracks = !Runtime.screenplayTracksSettings.displayStructureTracks
        }

        Action {
            readonly property bool allowShortcut: true
            readonly property bool visible: false

            text: "Reveal Settings Folder"
            objectName: "revealSettingsFolder"

            onTriggered: {
                const fileInfo = File.info(Platform.settingsPath)
                File.revealOnDesktop(fileInfo.absolutePath)
            }
        }
    }

    readonly property ActionManager userOptions: ActionManager {
        title: "User"
        objectName: "userOptions"

        Action {
            readonly property var keywords: ["logout", "logs", "activity", "analytics"]
            readonly property string defaultShortcut: "F8"
            property bool visible: enabled

            enabled: ActionHandler.canHandle
            objectName: "userAccount"
            shortcut: defaultShortcut
            text: "User Account"
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "F9"

            enabled: ActionHandler.canHandle
            objectName: "messages"
            shortcut: defaultShortcut
            text: "Messages"
        }

        Action {
            readonly property bool visible: false
            readonly property var keywords: ["plan", "paid", "pay", "purchase", "buy", "license", "pricing", "price", "discount"]
            readonly property string defaultShortcut: "F10"

            enabled: ActionHandler.canHandle
            objectName: "subscriptions"
            shortcut: defaultShortcut
            text: "Subscriptions"
        }

        Action {
            readonly property bool visible: false
            readonly property var keywords: ["device"]
            readonly property string defaultShortcut: "F11"

            enabled: ActionHandler.canHandle
            objectName: "installations"
            shortcut: defaultShortcut
            text: "Installations"
        }
    }

    function init(_parent) {
        if( !(_parent && Object.isOfType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        parent = _parent
        visible = false
        anchors.fill = parent
    }

    visible: false

    QtObject {
        id: _private

        property TextFormat textFormat: binder ? binder.textFormat : null
        property SceneElement sceneElement: binder ? binder.currentElement : null
        property SceneDocumentBinder binder // reference to the current binder on which paragraph formatting must be applied

        function setBinder(binder) {
            _private.binder = binder
        }

        function resetBinder(binder) {
            if(_private.binder === binder)
                _private.binder = null
        }

        function isOperationAllowedByUser(operation) {
            if(Scrite.document.hasCollaborators && !Scrite.document.canModifyCollaborators) {
                MessageBox.information("Action Prohibitted",
                                       "This document is locked by <b>" + Scrite.document.collaborators[0] + "</b>. " +
                                       operation + " can only be done by them.")
                return false
            }
            return true
        }

        readonly property EnumerationModel availableParagraphFormats: EnumerationModel {
            className: "SceneElement"
            enumeration: "Type"
            ignoreList: [SceneElement.All]
        }

        readonly property Component textColorsPopup: Popup {
            id: _textColorsPopup

            required property Item source
            required property TextFormat textFormat

            parent: source

            x: 0
            y: source.height
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

            contentItem: Item {
                implicitWidth: 500
                implicitHeight: 250

                ColumnLayout {
                    anchors.centerIn: parent

                    spacing: 10

                    Label {
                        Layout.fillWidth: true

                        text: "text"
                        font: Runtime.sceneEditorFontMetrics.font
                        color: _textColorsPopup.textFormat.textColor === Runtime.colors.transparent ? "black" : _textColorsPopup.textFormat.textColor
                        padding: Runtime.sceneEditorFontMetrics.averageCharacterWidth
                        horizontalAlignment: Text.AlignHCenter

                        background: Item {
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 1

                                border.width: 1
                                border.color: "black"
                                color: _textColorsPopup.textFormat.backgroundColor
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        spacing: parent.spacing

                        ColumnLayout {
                            Layout.fillWidth: true

                            Text {
                                Layout.fillWidth: true

                                font: Runtime.minimumFontMetrics.font
                                text: "Background Color"
                            }

                            ColorPalette {
                                colors: Runtime.colors.forDocument
                                selectedColor: _textColorsPopup.textFormat.backgroundColor
                                onColorPicked: (color) => {
                                    _textColorsPopup.textFormat.backgroundColor = color
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillHeight: true
                            Layout.preferredWidth: 1

                            color: Runtime.colors.primary.borderColor
                        }

                        ColumnLayout {
                            Layout.fillWidth: true

                            Text {
                                Layout.fillWidth: true

                                font: Runtime.minimumFontMetrics.font
                                text: "Text Color"
                            }

                            ColorPalette {
                                colors: Runtime.colors.forDocument
                                selectedColor: _textColorsPopup.textFormat.textColor
                                onColorPicked: (color) => {
                                    _textColorsPopup.textFormat.textColor = color
                                }
                            }
                        }
                    }
                }
            }
        }

        property int breakInsertIndex: {
            var idx = Scrite.document.screenplay.currentElementIndex
            if(idx < 0)
            return -1

            ++idx

            if(Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab || FocusInspector.hasFocus(Runtime.screenplayEditor)) {
                while(idx < Scrite.document.screenplay.elementCount) {
                    const e = Scrite.document.screenplay.elementAt(idx)
                    if(e === null) {
                        break
                    }
                    if(e.elementType === ScreenplayElement.BreakElementType) {
                        ++idx
                    } else {
                        break
                    }
                }
            }

            return idx
        }

        function addAct() {
            if(Scrite.document.readOnly)
                return

            if(breakInsertIndex < 0)
                Scrite.document.screenplay.addBreakElement(Screenplay.Act)
            else
                Scrite.document.screenplay.insertBreakElement(Screenplay.Act, _private.breakInsertIndex)
        }

        function addScene() {
            if(Scrite.document.readOnly)
                return

            Scrite.document.createNewScene(Runtime.mainWindowTab !== Runtime.MainWindowTab.ScreenplayTab ? FocusInspector.hasFocus(Runtime.screenplayEditor) : false)
        }

        function addEpisode() {
            if(Scrite.document.readOnly)
                return

            if(_private.breakInsertIndex < 0)
                Scrite.document.screenplay.addBreakElement(Screenplay.Episode)
            else
                Scrite.document.screenplay.insertBreakElement(Screenplay.Episode, _private.breakInsertIndex)
        }

        function addInterval() {
            if(Scrite.document.readOnly)
                return

            if(_private.breakInsertIndex < 0)
                Scrite.document.screenplay.addBreakElement(Screenplay.Interval)
            else
                Scrite.document.screenplay.insertBreakElement(Screenplay.Interval, _private.breakInsertIndex)
        }

        function toggleSelectedElementsAlignment(givenAlignment) {
            const alignment = _private.sceneElement.alignment === givenAlignment ? 0 : givenAlignment
            _private.sceneElement.alignment = alignment

            const selectedElements = binder.selectedElements
            selectedElements.forEach( (element) => { element.alignment = alignment })
        }
    }
}
