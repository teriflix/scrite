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
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0

import io.scrite.components 1.0

import "qrc:/qml/tasks"
import "qrc:/qml/dialogs"

Item {
    id: root

    function setBinder(binder) { _private.setBinder(binder) }
    function resetBinder(binder) { _private.resetBinder(binder) }

    readonly property ActionManager fileOperations: ActionManager {
        name: "File"

        Action {
            readonly property string defaultShortcut: "Ctrl+O"

            enabled: Runtime.allowAppUsage
            objectName: "fileOpen"
            shortcut: defaultShortcut
            text: "Home"

            icon.source: "qrc:/icons/action/home.png"

            onTriggered: HomeScreen.launch()
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+N"

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
            property bool visible: Scrite.document.backupFilesModel.count > 0

            enabled: Runtime.allowAppUsage && visible
            objectName: "backupOpen"
            text: "Backup"

            icon.source: "qrc:/icons/file/backup_open.png"

            onTriggered: BackupsDialog.launch()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+S"

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
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+P"

            text: "Export To PDF"
            enabled: Runtime.allowAppUsage
            shortcut: defaultShortcut

            icon.source: "qrc:/icons/exporter/pdf.png"

            onTriggered: ExportConfigurationDialog.launch("Screenplay/Adobe PDF")
        }
    }

    readonly property ActionManager exportOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/action/share.png"

        name: "Export"
    }

    Repeater {
        model: Scrite.document.supportedExportFormats

        // Repeater delegates can only be Item {}, they cannot be QObject types.
        // So, that rules out creating just Action {} as delegate. It has to be
        // nested in an Item.
        delegate: Item {
            required property var modelData // { name, icon, key, description, category }

            visible: false

            Action {
                property string tooltip: modelData.description

                ActionManager.target: root.exportOptions

                text: modelData.name
                enabled: Runtime.allowAppUsage

                icon.source: "qrc" + modelData.icon

                onTriggered: ExportConfigurationDialog.launch(modelData.key)
            }
        }
    }

    readonly property ActionManager reportOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/exporter/pdf.png"

        name: "Export"
    }

    Repeater {
        model: Scrite.document.supportedReports

        // Repeater delegates can only be Item {}, they cannot be QObject types.
        // So, that rules out creating just Action {} as delegate. It has to be
        // nested in an Item.
        delegate: Item {
            required property var modelData // { name, icon, description }

            visible: false

            Action {
                property string tooltip: modelData.description

                ActionManager.target: root.reportOptions

                text: modelData.name
                enabled: Runtime.allowAppUsage

                icon.source: "qrc" + modelData.icon

                onTriggered: ReportConfigurationDialog.launch(modelData.name)
            }
        }
    }

    readonly property ActionManager appOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/action/settings_applications.png"

        name: "Options"

        Action {
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
                if(Scrite.app.isMacOSPlatform)
                    return "qrc:/icons/navigation/shortcuts_macos.png"
                if(Scrite.app.isWindowsPlatform)
                    return "qrc:/icons/navigation/shortcuts_windows.png"
                return "qrc:/icons/navigation/shortcuts_linux.png"
            }

            onTriggered: Runtime.shortcutsDockWidgetSettings.visible = !Runtime.shortcutsDockWidgetSettings.visible
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
            readonly property string defaultShortcut: "F1"

            text: "Help"
            objectName: "help"
            shortcut: defaultShortcut

            icon.source: "qrc:/icons/action/help.png"

            onTriggered: Qt.openUrlExternally("https://www.scrite.io/index.php/help/")
        }

        Action {
            text: "About"
            objectName: "about"

            icon.source: "qrc:/icons/action/info.png"

            onTriggered: AboutDialog.launch()
        }
    }

    readonly property ActionManager languageOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/content/language.png"

        name: "Languages"

        Action {
            property int sortOrder: LanguageEngine.supportedLanguages.count + 1

            text: "More Languages ..."

            onTriggered: LanguageOptionsDialog.launch()
        }
    }

    readonly property ActionManager otherOptions: ActionManager {
        name: "Other"

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+K"

            property string tooltip: "Show English to " + Runtime.language.active.name + " alphabet mappings.\t" + Scrite.app.polishShortcutTextForDisplay(shortcut)

            objectName: "alphabetMappings"
            enabled: Runtime.language.activeCode !== QtLocale.English &&
                     Runtime.language.activeTransliterator.name === DefaultTransliteration.driver &&
                     DefaultTransliteration.supportsLanguageCode(Runtime.language.activeCode)
            shortcut: defaultShortcut
            text: "Alphabet Mappings"
            icon.source: "qrc:/icons/hardware/keyboard.png"
        }
    }

    Repeater {
        model: LanguageEngine.supportedLanguages

        delegate: Item {
            required property int index
            required property var language // This is of type Language, but we have to use var here.
                                           // You cannot use Q_GADGET struct names as type names in QML
                                           // that privilege is only reserved for QObject types.

            Action {
                property int sortOrder: index
                property string tooltip: text + "\t" + Scrite.app.polishShortcutTextForDisplay(shortcut)

                ActionManager.target: root.languageOptions

                text: language.name
                shortcut: language.shortcut()
                // TODO iconSource to source icons through a QQuickAsyncImageProvider

                onTriggered: Runtime.language.setActiveCode(language.code)
            }
        }
    }

    readonly property ActionManager paragraphFormats: ActionManager {
        function action(type) { return _paragraphFormatActions.action(type) }

        name: "Paragraph Format"
    }

    Repeater {
        id: _paragraphFormatActions

        function action(type) {
            if(type < 0 || type >= _private.availableParagraphFormats.length)
                return null

            return _paragraphFormatActions.itemAt(type)
        }

        model: _private.availableParagraphFormats

        // Repeater delegates can only be Item {}, they cannot be QObject types.
        // So, that rules out creating just Action {} as delegate. It has to be
        // nested in an Item.
        delegate: Item {
            required property int index
            required property var modelData // { value, name, display, icon }

            visible: false

            Action {
                property int sortOrder: index
                property string defaultShortcut: "Ctrl+" + index

                property string tooltip: modelData.display + "\t" + Scrite.app.polishShortcutTextForDisplay(shortcut)

                ActionManager.target: root.paragraphFormats

                checkable: true
                checked: _private.binder !== null ? (_private.binder.currentElement ? _private.binder.currentElement.type === modelData.value : false) : false
                enabled: Runtime.allowAppUsage && _private.binder !== null
                icon.source: modelData.icon
                objectName: modelData.name
                shortcut: defaultShortcut
                text: modelData.display

                onTriggered: {
                    if(index === 0) {
                        if(!_private.binder.scene.heading.enabled)
                        _private.binder.scene.heading.enabled = true
                        Announcement.shout(Runtime.announcementIds.focusRequest, Runtime.announcementData.focusOptions.sceneHeading)
                    } else {
                        _private.binder.currentElement.type = modelData.value
                    }
                }
            }
        }
    }

    function init(_parent) {
        if( !(_parent && Scrite.app.verifyType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        parent = _parent
        visible = false
        anchors.fill = parent
    }

    visible: false

    QtObject {
        id: _private

        property SceneDocumentBinder binder // reference to the current binder on which paragraph formatting must be applied

        function setBinder(binder) {
            _private.binder = binder
        }

        function resetBinder(binder) {
            if(_private.binder === binder)
                _private.binder = null
        }

        readonly property var availableParagraphFormats: [
            { "value": SceneElement.Heading, "name": "headingParagraph", "display": "Current Scene Heading", "icon": "qrc:/icons/screenplay/heading.png" },
            { "value": SceneElement.Action, "name": "actionParagraph", "display": "Action", "icon": "qrc:/icons/screenplay/action.png" },
            { "value": SceneElement.Character, "name": "characterParagraph", "display": "Character", "icon": "qrc:/icons/screenplay/character.png" },
            { "value": SceneElement.Dialogue, "name": "dialogueParagraph", "display": "Dialogue", "icon": "qrc:/icons/screenplay/dialogue.png" },
            { "value": SceneElement.Parenthetical, "name": "parentheticalParagraph", "display": "Parenthetical", "icon": "qrc:/icons/screenplay/parenthetical.png" },
            { "value": SceneElement.Shot, "name": "shotParagraph", "display": "Shot", "icon": "qrc:/icons/screenplay/shot.png" },
            { "value": SceneElement.Transition, "name": "transitionParagraph", "display": "Transition", "icon": "qrc:/icons/screenplay/transition.png" }
        ]
    }
}
