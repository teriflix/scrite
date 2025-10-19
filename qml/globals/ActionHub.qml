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

    readonly property ActionManager fileOperations: ActionManager {
        title: "File"
        objectName: "fileOperations"

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

        title: "Export"
        objectName: "exportOptions"
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

        title: "Export"
        objectName: "reportOptions"
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

        title: "Options"
        objectName: "appOptions"

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

        title: "Languages"
        objectName: "languageOptions"

        Action {
            property int sortOrder: LanguageEngine.supportedLanguages.count + 1

            text: "More Languages ..."

            onTriggered: LanguageOptionsDialog.launch()
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

                ActionManager.target: root.languageOptions

                text: language.name
                shortcut: language.shortcut()
                // TODO iconSource to source icons through a QQuickAsyncImageProvider

                onTriggered: Runtime.language.setActiveCode(language.code)
            }
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
        function action(type) { return _paragraphFormatActions.action(type) }
        function setBinder(binder) { root.setBinder(binder) }
        function resetBinder(binder) { root.resetBinder(binder) }

        title: "Paragraph Format"
        objectName: "paragraphFormats"
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

                property string tooltip: modelData.display

                ActionManager.target: root.paragraphFormats

                checkable: true
                checked: _private.binder !== null ? (_private.binder.currentElement ? _private.binder.currentElement.type === modelData.value : false) : false
                enabled: Runtime.allowAppUsage && _private.binder !== null
                objectName: modelData.name
                shortcut: defaultShortcut
                text: modelData.display

                icon.source: modelData.icon

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

    readonly property ActionManager editOptions: ActionManager {
        title: "Edit"
        objectName: "editOptions"

        Action {
            readonly property string defaultShortcut: "Ctrl+F"

            enabled: ActionHandler.canHandle
            objectName: "find"
            shortcut: defaultShortcut
            text: "Find"

            icon.source: "qrc:/icons/action/search.png"
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+F"
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "replace"
            shortcut: defaultShortcut
            text: "Replace"

            icon.source: "qrc:/icons/action/find_replace.png"
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+X"
            readonly property bool visible: false

            enabled: ActionHandler.canHandle
            objectName: "cut"
            shortcut: defaultShortcut
            text: "Cut"
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+C"
            property bool visible: ActionHandler.canHandle

            enabled: ActionHandler.canHandle
            objectName: "copy"
            shortcut: defaultShortcut
            text: "Copy"

            icon.source: "qrc:/icons/content/content_copy.png"
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+V"
            property bool visible: ActionHandler.canHandle

            enabled: ActionHandler.canHandle
            objectName: "paste"
            shortcut: defaultShortcut
            text: "Paste"

            icon.source: "qrc:/icons/content/content_paste.png"
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Z"
            property bool visible: ActionHandler.canHandle

            enabled: ActionHandler.canHandle
            objectName: "undo"
            shortcut: defaultShortcut
            text: "Undo"

            icon.source: "qrc:/icons/content/undo.png"
        }

        Action {
            readonly property string defaultShortcut: Scrite.app.isWindowsPlatform ? "Ctrl+Y" : "Ctrl+Shift+Z"
            property bool visible: ActionHandler.canHandle

            enabled: ActionHandler.canHandle
            objectName: "redo"
            shortcut: defaultShortcut
            text: "Redo"

            icon.source: "qrc:/icons/content/redo.png"
        }

        Action {
            readonly property string defaultShortcut: "F5"

            enabled: ActionHandler.canHandle
            objectName: "reload"
            shortcut: defaultShortcut
            text: "Reload"

            icon.source: "qrc:/icons/navigation/refresh.png"
        }
    }

    readonly property ActionManager markupTools: ActionManager {
        function setBinder(binder) { root.setBinder(binder) }
        function resetBinder(binder) { root.resetBinder(binder) }

        title: "Markup Tools"
        objectName: "markupTools"

        Action {
            readonly property string defaultShortcut: "Ctrl+B"

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
            readonly property string defaultShortcut: "Ctrl+I"

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
            readonly property string defaultShortcut: "Ctrl+U"

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
            enabled: _private.textFormat && Runtime.allowAppUsage
            objectName: "colors"
            text: "Colors"

            onTriggered: (source) => {
                let popup = _private.textColorsPopup.createObject(source, {"source": source, "textFormat": _private.textFormat})
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
            enabled: _private.sceneElement && Runtime.allowAppUsage
            checkable: true
            checked: _private.sceneElement ? _private.sceneElement.alignment === Qt.AlignLeft : false
            objectName: "alignLeft"
            text: "Left Align"

            icon.source: "qrc:/icons/editor/format_align_left.png"

            onTriggered: _private.toggleSelectedElementsAlignment(Qt.AlignLeft)
        }

        Action {
            enabled: _private.sceneElement && Runtime.allowAppUsage
            checkable: true
            checked: _private.sceneElement ? _private.sceneElement.alignment === Qt.AlignHCenter : false
            objectName: "alignCenter"
            text: "Center Align"

            icon.source: "qrc:/icons/editor/format_align_center.png"

            onTriggered: _private.toggleSelectedElementsAlignment(Qt.AlignHCenter)
        }

        Action {
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

    readonly property ActionManager screenplayEditorOptions: ActionManager {
        readonly property string iconSource: "qrc:/icons/content/view_options.png"

        title: "Screenplay Editor Options"
        objectName: "screenplayEditorOptions"

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayRuler
            text: "Ruler"

            onToggled: Runtime.screenplayEditorSettings.displayRuler = !Runtime.screenplayEditorSettings.displayRuler
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.showLoglineEditor
            text: "Logline Editor"

            onToggled: Runtime.screenplayEditorSettings.showLoglineEditor = !Runtime.screenplayEditorSettings.showLoglineEditor
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayEmptyTitleCard
            text: "Empty Title Card"

            onToggled: Runtime.screenplayEditorSettings.displayEmptyTitleCard = !Runtime.screenplayEditorSettings.displayEmptyTitleCard
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons
            text: "Act, Episode, Scene Controls"

            onToggled: Runtime.screenplayEditorSettings.displayAddSceneBreakButtons = !Runtime.screenplayEditorSettings.displayAddSceneBreakButtons
        }

        Action {
            property bool __sceneBlocksVisible: Runtime.screenplayEditorSettings.spaceBetweenScenes > 0

            checkable: true
            checked: __sceneBlocksVisible
            text: "Block Display"

            onToggled: Runtime.screenplayEditorSettings.spaceBetweenScenes = __sceneBlocksVisible ? 0 : 40
        }

        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.markupToolsDockVisible
            text: "Markup Tools"

            onToggled: Runtime.screenplayEditorSettings.markupToolsDockVisible = !Runtime.screenplayEditorSettings.markupToolsDockVisible
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+S"

            checkable: true
            checked: Runtime.screenplayEditorSettings.displaySceneSynopsis
            shortcut: defaultShortcut
            objectName: "synopsis"
            text: "Synopsis"

            onToggled: Runtime.screenplayEditorSettings.displaySceneSynopsis = !Runtime.screenplayEditorSettings.displaySceneSynopsis
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+M"

            checkable: true
            checked: Runtime.screenplayEditorSettings.displaySceneComments
            shortcut: defaultShortcut
            objectName: "comments"
            text: "Comments"

            onToggled: Runtime.screenplayEditorSettings.displaySceneComments = !Runtime.screenplayEditorSettings.displaySceneComments
        }


        Action {
            checkable: true
            checked: Runtime.screenplayEditorSettings.displayIndexCardFields
            enabled: Runtime.screenplayEditorSettings.displaySceneComments
            text: "Index Card Fields"

            onToggled: Runtime.screenplayEditorSettings.displayIndexCardFields = !Runtime.screenplayEditorSettings.displayIndexCardFields
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+C"

            checkable: true
            checked: Runtime.screenplayEditorSettings.displaySceneCharacters
            shortcut: defaultShortcut
            objectName: "charactersAndTags"
            text: "Characters and Tags"

            onToggled: Runtime.screenplayEditorSettings.displaySceneCharacters = !Runtime.screenplayEditorSettings.displaySceneCharacters
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+G"

            checkable: true
            checked: Runtime.screenplayEditorSettings.allowTaggingOfScenes
            shortcut: defaultShortcut
            objectName: "tagging"
            text: "Tagging"

            onToggled: Runtime.screenplayEditorSettings.allowTaggingOfScenes = !Runtime.screenplayEditorSettings.allowTaggingOfScenes
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Alt+L"

            checkable: true
            checked: Runtime.screenplayEditorSettings.enableSpellCheck
            shortcut: defaultShortcut
            objectName: "spellCheck"
            text: "Spell Check"

            onToggled: Runtime.screenplayEditorSettings.enableSpellCheck = !Runtime.screenplayEditorSettings.enableSpellCheck
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+H"

            checkable: true
            checked: Runtime.screenplayEditorSettings.highlightCurrentLine
            shortcut: defaultShortcut
            objectName: "lineHighlight"
            text: "Line Highlight"

            onToggled: Runtime.screenplayEditorSettings.highlightCurrentLine = !Runtime.screenplayEditorSettings.highlightCurrentLine
        }

        Action {
            text: "Scan Mute Characters"
            enabled: !Scrite.document.readOnly && Runtime.screenplayEditorSettings.displaySceneCharacters

            onTriggered: Scrite.document.structure.scanForMuteCharacters()
        }

        Action {
            text: "Reset Scene Numbers"
            enabled: !Scrite.document.readOnly

            onTriggered: Scrite.document.structure.resetSceneNumbers()
        }
    }

    readonly property ActionManager screenplayOperations: ActionManager {
        title: "Screenplay Operations"
        objectName: "screenplayOperations"

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+N"

            enabled: !Scrite.document.enabled
            shortcut: defaultShortcut
            text: "New Scene"
            objectName: "newScene"

            icon.source: "qrc:/icons/action/add_scene.png"

            onTriggered: _private.addScene()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+B"

            enabled: !Scrite.document.enabled
            shortcut: defaultShortcut
            text: "Act Break"
            objectName: "actBreak"

            icon.source: "qrc:/icons/action/add_act.png"

            onTriggered: _private.addAct()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+P"

            enabled: !Scrite.document.enabled
            shortcut: defaultShortcut
            text: "Episode Break"
            objectName: "episodeBreak"

            icon.source: "qrc:/icons/action/add_episode.png"

            onTriggered: _private.addEpisode()
        }

        Action {
            readonly property string defaultShortcut: "Ctrl+Shift+L"
            readonly property bool visible: false

            enabled: !Scrite.document.enabled
            shortcut: defaultShortcut
            text: "Interval Break"
            objectName: "intervalBreak"

            onTriggered: _private.addInterval()
        }
    }

    readonly property ActionManager applicationOptions: ActionManager {
        title: "Application"
        objectName: "applicationOptions"

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+Alt+A"

            checkable: true
            checked: Runtime.applicationSettings.enableAnimations
            text: "Animations"
            shortcut: defaultShortcut
            objectName: "animations"

            onToggled: Runtime.applicationSettings.enableAnimations = !Runtime.applicationSettings.enableAnimations
        }

        Action {
            readonly property bool visible: false
            readonly property string defaultShortcut: "Ctrl+M"

            text: "New Scrite Window"
            shortcut: defaultShortcut
            objectName: "newScriteWindow"

            onTriggered: Scrite.app.launchNewInstance(Scrite.window)
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

        readonly property var availableParagraphFormats: [
            { "value": SceneElement.Heading, "name": "headingParagraph", "display": "Current Scene Heading", "icon": "qrc:/icons/screenplay/heading.png" },
            { "value": SceneElement.Action, "name": "actionParagraph", "display": "Action", "icon": "qrc:/icons/screenplay/action.png" },
            { "value": SceneElement.Character, "name": "characterParagraph", "display": "Character", "icon": "qrc:/icons/screenplay/character.png" },
            { "value": SceneElement.Dialogue, "name": "dialogueParagraph", "display": "Dialogue", "icon": "qrc:/icons/screenplay/dialogue.png" },
            { "value": SceneElement.Parenthetical, "name": "parentheticalParagraph", "display": "Parenthetical", "icon": "qrc:/icons/screenplay/parenthetical.png" },
            { "value": SceneElement.Shot, "name": "shotParagraph", "display": "Shot", "icon": "qrc:/icons/screenplay/shot.png" },
            { "value": SceneElement.Transition, "name": "transitionParagraph", "display": "Transition", "icon": "qrc:/icons/screenplay/transition.png" }
        ]

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

            if(Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab || Runtime.undoStack.screenplayEditorActive) {
                while(idx < Scrite.document.screenplay.elementCount) {
                    const e = Scrite.document.screenplay.elementAt(idx)
                    if(e === null)
                    break
                    if(e.elementType === ScreenplayElement.BreakElementType)
                    ++idx
                    else
                    break
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

            Scrite.document.createNewScene(Runtime.mainWindowTab !== Runtime.MainWindowTab.ScreenplayTab ? Runtime.undoStack.screenplayEditorActive : false)
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
