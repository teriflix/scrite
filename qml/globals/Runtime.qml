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
import Qt.labs.settings 1.0
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

Item {
    id: root

    enum MainWindowTab { ScreenplayTab, StructureTab, NotebookTab, ScritedTab }

    signal resetMainWindowUi(var callback)
    signal aboutToSwitchTab(int from, int to)
    signal finishedTabSwitch(int to)

    readonly property int stdAnimationDuration: 250
    readonly property int subscriptionTreshold: 15 // if active subscription has less than these many days, then reminders are shown upon login

    readonly property var characterReports: Scrite.document.characterReports
    readonly property var sceneReports: Scrite.document.sceneReports
    readonly property var episodeReports: Scrite.document.episodeReports
    readonly property var keywordReports: Scrite.document.keywordReports
    readonly property var tagReports: Scrite.document.tagReports

    readonly property real iconImageSize: 30 // min width or height of icon Image QML elements
    readonly property real maxSceneSidePanelWidth: 400
    readonly property real minSceneSidePanelWidth: 250
    readonly property real minWindowWidthForShowingNotebookInStructure: 1600

    property int mainWindowTab: Runtime.MainWindowTab.ScreenplayTab
    property int placeholderInterval: bounded(50, Runtime.screenplayEditorSettings.placeholderInterval, 1000)
    property int visibleTooltipCount: 0

    property var helpTips: undefined

    property bool allowAppUsage: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription
    property bool canShowNotebookInStructure: width > minWindowWidthForShowingNotebookInStructure
    property bool currentUseSoftwareRenderer
    property bool loadMainUiContent: true
    property bool showNotebookInStructure: workspaceSettings.showNotebookInStructure && canShowNotebookInStructure

    property string currentTheme

    // This property holds reference to an instance of ScreenplayEditor
    property Item screenplayEditor

    // This property holds reference to an instance of StructureView
    property Item structureView

    // This property holds reference to the global screenplay editor toolbar
    property Item screenplayEditorToolbar

    property ObjectListModel dialogs: ObjectListModel { }

    readonly property QtObject language: QtObject {
        readonly property AvailableLanguages available: LanguageEngine.availableLanguages
        readonly property LanguageEngine engine: LanguageEngine
        readonly property SupportedLanguages supported: LanguageEngine.supportedLanguages

        property int activeCode: supported.activeLanguageCode

        property var active: supported.activeLanguage
        property var activeTransliterationOption: active.valid ? active.preferredTransliterationOption() : undefined

        property AbstractTransliterationEngine activeTransliterator: activeTransliterationOption.valid ? activeTransliterationOption.transliterator : null

        function setActiveCode(code) {
            if(activeCode === code)
                return

            supported.activeLanguageCode = code
            Scrite.document.displayFormat.activeLanguageCode = activeCode
            logActivity("language-activate", supported.activeLanguage)
        }

        function logActivity(activity, lang) {
            if(lang && Scrite.user.info.consentToActivityLog) {
                const txOption = lang.preferredTransliterationOption()
                const portableShortcut = Gui.portableShortcut(lang.keySequence)
                const shortcut = portableShortcut === "" ? "<no-shortcut>" : portableShortcut
                const details = [lang.name, shortcut, txOption.id, txOption.name, lang.font().family, Platform.typeString].join(";")
                Scrite.user.logActivity2(activity, details)
            }
        }
    }

    // Persistent Settings
    readonly property Settings userAccountDialogSettings: Settings {
        property bool welcomeScreenShown: false
        property string userOnboardingStatus: "unknown"

        category: "UserAccountDialog"
        fileName: Platform.settingsFile
    }

    readonly property Settings scrollAreaSettings: Settings {
        property real zoomFactor: 0.05

        category: "ScrollArea"
        fileName: Platform.settingsFile
    }

    readonly property Settings structureCanvasSettings: Settings {
        property bool displayAnnotationProperties: true
        property bool showGrid: true
        property bool showPreview: true
        property bool showPullHandleAnimation: true

        property real connectorLineWidth: 2
        property real overflowFactor: 0.05
        property real previewSize: 300

        property color canvasColor: Runtime.colors.accent.c50.background
        property color gridColor: Runtime.colors.accent.c400.background

        function restoreDefaultGridColor() {
            gridColor = Runtime.colors.accent.c400.background
        }

        function restoreDefaultCanvasColor() {
            canvasColor = Runtime.colors.accent.c50.background
        }

        category: "Structure Tab"
        fileName: Platform.settingsFile

    }

    readonly property Settings timelineViewSettings: Settings {
        readonly property string dropAreaKey: "scrite/sceneID"

        property bool showCursor: true
        property string textMode: "HeadingOrTitle"

        category: "Timeline View"
        fileName: Platform.settingsFile
    }

    readonly property Settings screenplayEditorSettings: Settings {
        category: "Screenplay Editor"
        fileName: Platform.settingsFile

        ObjectRegister.name: "screenplayEditorSettings"

        property var zoomLevelModifiers: { "tab0": 0, "tab1": 0, "tab2": 0, "tab3": 0 }

        property int commentsPanelTabIndex: 1
        property int embeddedEditorZoomValue: -1
        property int lastLanguageRefreshNoticeBoxTimestamp: 0
        property int lastSpellCheckRefreshNoticeBoxTimestamp: 0
        property int longSceneWordTreshold: 150
        property int mainEditorZoomValue: -1
        property int placeholderInterval: 250 // ms after which placeholder is swapped to content in delegates
        property int sceneSidePanelActiveTab: 0
        property int slpSynopsisLineCount: 2
        property int jumpToSceneFilterMode: 0

        property bool allowDiacriticEditing: true
        property bool allowSelectedTextTranslation: false
        property bool allowTaggingOfScenes: false
        property bool applyUserDefinedLanguageFonts: true
        property bool autoAdjustEditorWidthInScreenplayEditor: true
        property bool captureInvisibleCharacters: false
        property bool copyAsFountain: true
        property bool copyFountainUsingStrictSyntax: true
        property bool copyFountainWithEmphasis: true
        property bool displayAddSceneBreakButtons: true
        property bool displayEmptyTitleCard: true
        property bool displayIndexCardFields: true
        property bool displayRuler: false
        property bool displaySceneCharacters: false
        property bool displaySceneComments: false
        property bool displaySceneSynopsis: false
        property bool enableAutoCapitalizeSentences: true
        property bool enableAutoPolishParagraphs: true // for automatically adding/removing CONT'D where appropriate
        property bool enableSpellCheck: true // Since this is now fixed: https://github.com/teriflix/scrite/issues/138
        property bool focusCursorOnSceneHeadingInNewScenes: false
        property bool highlightCurrentLine: true
        property bool includeTitlePageInPreview: true
        property bool languageInputPreferenceChecked: false
        property bool longSceneWarningEnabled: true
        property bool markupToolsDockVisible: false
        property bool optimiseScrolling: false
        property bool pasteAfterResolvingEmphasis: true
        property bool pasteAsFountain: true
        property bool pasteByLinkingScenesWhenPossible: true
        property bool pasteByMergingAdjacentElements: true
        property bool refreshButtonInStatsReportAnimationDone: false
        property bool restartEpisodeScenesAtOne: false // If set, each episode starts with scene number 1
        property bool sceneSidePanelOpen: false
        property bool screenplayEditorAddButtonsAnimationShown: false
        property bool showLanguageRefreshNoticeBox: true
        property bool showLoglineEditor: false
        property bool showSpellCheckRefreshNoticeBox: true
        property bool singleClickAutoComplete: true

        property real sidePanelWidth: 400
        property real spaceBetweenScenes: 0
    }

    readonly property Settings screenplayTracksSettings: Settings {
        category: "ScreenplayTracks"
        fileName: Platform.settingsFile

        property bool displayTracks: true
        property bool displayStacks: true
        property bool displayKeywordsTracks: true
        property bool displayStructureTracks: true
    }

    readonly property Settings pdfExportSettings: Settings {
        property bool usePdfDriver: true

        category: "PdfExport"
        fileName: Platform.settingsFile
    }

    readonly property Settings titlePageSettings: Settings {
        fileName: Platform.settingsFile
        category: "TitlePage"

        property bool includeTimestamp: false

        property string address
        property string author
        property string contact
        property string email
        property string phone
        property string website
    }

    readonly property Settings richTextEditorSettings: Settings {
        property bool languageNoteShown: false

        category: "Rich Text Editor"
        fileName: Platform.settingsFile
    }

    readonly property Settings sceneListPanelSettings: Settings {
        property bool showTooltip: true

        property bool displayTracks: true
        property string displaySceneLength: "TIME" // can be NO, PAGE, TIME
        property string sceneTextMode: "HEADING" // can be SUMMARY also

        category: "Scene List Panel"
        fileName: Platform.settingsFile
    }

    readonly property Settings markupToolsSettings: Settings {
        property real contentX: 20
        property real contentY: 20

        category: "Markup Tools"
        fileName: Platform.settingsFile
    }

    readonly property Settings scritedSettings: Settings {
        property bool codecsNoticeDisplayed: false
        property bool experimentalFeatureNoticeDisplayed: false
        property bool videoPlayerVisible: true

        property real playerAreaRatio: 0.5

        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)

        category: "Scrited"
        fileName: Platform.settingsFile
    }

    readonly property Settings shortcutsDockWidgetSettings: Settings {
        property real contentX: -1
        property real contentY: -1

        property bool visible: true

        category: "Shortcuts Dock Widget"
        fileName: Platform.settingsFile
    }

    readonly property Settings helpNotificationSettings: Settings {
        property string dayZero
        property string tipsShown: ""

        function daysSinceZero() {
            const today = new Date()
            const dzero = dayZero === "" ? today : new Date(dayZero + "Z")
            const days = Math.floor((today.getTime() - dzero.getTime()) / (24*60*60*1000))
            return days
        }

        function isTipShown(val) {
            const ts = tipsShown.split(",")
            return ts.indexOf(val) >= 0
        }

        function markTipAsShown(val) {
            let ts = tipsShown.length > 0 ? tipsShown.split(",") : []
            if(ts.indexOf(val) < 0)
                ts.push(val)
            tipsShown = ts.join(",")
        }

        fileName: Platform.settingsFile
        category: "Help"
    }

    readonly property Settings notebookSettings: Settings {
        property int characterPageTab: 0
        property int charactersPageTab: 0
        property int screenplayPageTab: 0
        property int sceneNotesPageTab: 0
        property int sceneSynopsisTabIndex: 0
        property int graphLayoutMaxIterations: 50000
        property int graphLayoutMaxTime: 1000

        property bool richTextNotesEnabled: true
        property bool showAllFormQuestions: true

        fileName: Platform.settingsFile
        category: "Notebook"
    }

    readonly property Settings workspaceSettings: Settings {
        fileName: Platform.settingsFile
        category: "Workspace"

        property var customColors: []
        property var defaultSceneColor: SceneColors.palette[0]

        property bool animateNotebookIcon: true
        property bool animateStructureIcon: true
        property bool autoOpenLastFile: false
        property bool mouseWheelZoomsInCharacterGraph: Platform.isWindowsDesktop || Platform.isLinuxDesktop
        property bool mouseWheelZoomsInStructureCanvas: Platform.isWindowsDesktop || Platform.isLinuxDesktop
        property bool scriptalayIntroduced: false
        property bool showNotebookInStructure: true
        property bool showScritedTab: false
        property bool syncCurrentSceneOnNotebook: true

        property real flickScrollSpeedFactor: 1.0
        property real screenplayEditorWidth: -1
        property real workspaceHeight

        property string lastOpenExportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenImportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenPhotosFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        property string lastOpenReportsFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
    }

    readonly property Settings applicationSettings: Settings {
        id: _applicationSettings

        property int accentColor: colors.defaultAccentColor
        property int primaryColor: colors.defaultPrimaryColor
        property int joinDiscordPromptCounter: 0

        property real colorIntensity: 0.5

        property bool enableAnimations: true
        property bool notifyMissingRecentFiles: true
        property bool reloadPrompt: true
        property bool useNativeTextRendering: false
        property bool useSoftwareRenderer: false

        property string theme: "Material"

        Component.onCompleted: {
            colorIntensity = bounded(0, colorIntensity, 1)
            Qt.callLater( () => {
                             Runtime.currentTheme = theme
                             Runtime.currentUseSoftwareRenderer = useSoftwareRenderer
                         })
        }

        category: "Application"
        fileName: Platform.settingsFile
    }

    // Global undo-redo stack
    readonly property UndoStack undoStack: UndoStack {
        ObjectRegister.name: objectName

        objectName: "MainUndoStack"
        active: true
    }

    // App-wide font-metrics
    readonly property FontMetrics minimumFontMetrics: FontMetrics {
        font.pointSize: idealFontMetrics.font.pointSize-2
    }

    readonly property FontMetrics idealFontMetrics: FontMetrics {
        font.pointSize: Scrite.app.idealFontPointSize
    }

    readonly property FontMetrics shortcutFontMetrics: FontMetrics {
        font.pointSize: idealFontMetrics.font.pointSize
        font.family: {
            // We need ZERO and the letter O to be rendered distinctly
            // We also need small-L and capital-I and digit-1 to look disctinct.
            switch(Platform.type) {
            case Platform.WindowsDesktop: return "Consolas"
            case Platform.MacOSDesktop: return "Monaco"
            case Platform.LinuxDesktop: return "DejaVu Sans Mono"
            }
            return "Courier Prime"
        }
    }

    readonly property FontMetrics minimumShortcutFontMetrics: FontMetrics {
        font.pointSize: minimumFontMetrics.font.pointSize
        font.family: shortcutFontMetrics.font.family
    }

    readonly property FontMetrics sceneEditorFontMetrics: FontMetrics {
        readonly property int lettersPerLine: 70
        readonly property int marginLetters: 5

        readonly property real pageWidth: Math.ceil(paragraphWidth + 2*paragraphMargin)
        readonly property real paragraphMargin: Math.ceil(marginLetters*averageCharacterWidth)
        readonly property real paragraphWidth: Math.ceil(lettersPerLine*averageCharacterWidth)

        font: Scrite.document.formatting.defaultFont2
    }

    // Color Palettes
    component Colors : QtObject {
        property int key: Material.Indigo

        property var button: c200
        property var highlight: c400

        property color borderColor: c400.background
        property color separatorColor: c400.background
        property color windowColor: c300.background

        property QtObject regular: QtObject {
            property color background: Material.color(key)
            property color text: Color.textColorFor(background)
        }

        property QtObject c10: QtObject {
            property color background: Qt.rgba(1,1,1,0)
            property color text: "black"
        }

        property QtObject c50: QtObject {
            property color background: Material.color(key, Material.Shade50)
            property color text: Color.textColorFor(background)
        }

        property QtObject c100: QtObject {
            property color background: Material.color(key, Material.Shade100)
            property color text: Color.textColorFor(background)
        }

        property QtObject c200: QtObject {
            property color background: Material.color(key, Material.Shade200)
            property color text: Color.textColorFor(background)
        }

        property QtObject c300: QtObject {
            property color background: Material.color(key, Material.Shade300)
            property color text: Color.textColorFor(background)
        }

        property QtObject c400: QtObject {
            property color background: Material.color(key, Material.Shade400)
            property color text: Color.textColorFor(background)
        }

        property QtObject c500: QtObject {
            property color background: Material.color(key, Material.Shade500)
            property color text: Color.textColorFor(background)
        }

        property QtObject c600: QtObject {
            property color background: Material.color(key, Material.Shade600)
            property color text: Color.textColorFor(background)
        }

        property QtObject c700: QtObject {
            property color background: Material.color(key, Material.Shade700)
            property color text: Color.textColorFor(background)
        }

        property QtObject c800: QtObject {
            property color background: Material.color(key, Material.Shade800)
            property color text: Color.textColorFor(background)
        }

        property QtObject c900: QtObject {
            property color background: Material.color(key, Material.Shade900)
            property color text: Color.textColorFor(background)
        }

        property QtObject a100: QtObject {
            property color background: Material.color(key, Material.ShadeA100)
            property color text: Color.textColorFor(background)
        }

        property QtObject a200: QtObject {
            property color background: Material.color(key, Material.ShadeA200)
            property color text: Color.textColorFor(background)
        }

        property QtObject a400: QtObject {
            property color background: Material.color(key, Material.ShadeA400)
            property color text: Color.textColorFor(background)
        }

        property QtObject a700: QtObject {
            property color background: Material.color(key, Material.ShadeA700)
            property color text: Color.textColorFor(background)
        }
    }

    readonly property QtObject colors: Item {
        ObjectRegister.name: "runtimeColors"

        readonly property int   defaultAccentColor: Material.DeepPurple
        readonly property int   defaultPrimaryColor: Material.Grey
        readonly property int   theme: Material.Light

        readonly property var   forDocument: ["#e60000", "#ff9900", "#ffff00", "#008a00", "#0066cc", "#9933ff", "#ffffff", "#facccc", "#ffebcc", "#ffffcc", "#cce8cc", "#cce0f5", "#ebd6ff", "#bbbbbb", "#f06666", "#ffc266", "#ffff66", "#66b966", "#66a3e0", "#c285ff", "#888888", "#a10000", "#b26b00", "#b2b200", "#006100", "#0047b2", "#6b24b2", "#444444", "#5c0000", "#663d00", "#666600", "#003700", "#002966", "#3d1466"]
        readonly property var   forScene: SceneColors.palette

        property real sceneControlTint: _applicationSettings.colorIntensity*0.4
        property real sceneHeadingTint: _applicationSettings.colorIntensity*0.4
        property real currentNoteTint: _applicationSettings.colorIntensity*0.4
        property real currentLineHightlightTint: _applicationSettings.colorIntensity*0.2
        property real screenplayTracksTint: root.bounded(0.4, _applicationSettings.colorIntensity, 1)
        property color selectedSceneControlTint: Color.translucent(primary.c100.background, root.bounded(0.2,1-_applicationSettings.colorIntensity,0.8))
        property color selectedSceneHeadingTint:  Color.translucent(primary.c100.background, root.bounded(0.2,1-_applicationSettings.colorIntensity,0.8))

        readonly property color transparent: "transparent"

        readonly property Colors primary: Colors {
            ObjectRegister.name: "primaryColors"

            key: Material.Grey // applicationSettings.primaryColor

            property QtObject editor: QtObject {
                readonly property color background: colors.theme === Material.Light ? "white" : "black"
                readonly property color text: colors.theme === Material.Light ? "black" : "white"
            }
        }

        readonly property Colors accent: Colors {
            ObjectRegister.name: "accentColors"

            key: _applicationSettings.accentColor
        }

        function tint(a, b) {
            return Color.stacked( Color.tint(a, b), theme === Material.Light ? "white" : "black" )
        }
    }

    // All the app-features
    readonly property QtObject appFeatures: QtObject {
        readonly property AppFeature screenplay: AppFeature {
            feature: Scrite.ScreenplayFeature
        }

        readonly property AppFeature structure: AppFeature {
            feature: Scrite.StructureFeature
        }

        readonly property AppFeature notebook: AppFeature {
            feature: Scrite.NotebookFeature
        }

        readonly property AppFeature scrited: AppFeature {
            feature: Scrite.ScritedFeature
        }

        readonly property AppFeature characterRelationshipGraph: AppFeature {
            feature: Scrite.RelationshipGraphFeature
        }

        readonly property AppFeature watermark: AppFeature {
            feature: Scrite.WatermarkFeature
        }

        readonly property AppFeature importer: AppFeature {
            feature: Scrite.ImportFeature
        }

        readonly property AppFeature exporter: AppFeature {
            feature: Scrite.ExportFeature
        }

        readonly property AppFeature scriptalay: AppFeature {
            feature: Scrite.ScriptalayFeature
        }

        readonly property AppFeature templates: AppFeature {
            feature: Scrite.TemplateFeature
        }

        readonly property AppFeature emailSupport: AppFeature {
            featureName: "support/email"
        }
    }

    // This model provides access to recently accessed files. It is updated from
    // different parts of the UI where opening / saving of files is triggered.
    // Contents of this model is listed in the HomeScreen.
    readonly property ScriteFileListModel recentFiles: ScriteFileListModel {
        id: _recentFiles

        property var missingFiles: []

        property bool preferTitleVersionText: true

        Component.onCompleted: {
            Scrite.document.justLoaded.connect(onDocumentJustLoaded)
            Scrite.document.justSaved.connect(onDocumentJustSaved)
            filesChanged.connect(captureChangeInFiles)
        }

        function onDocumentJustSaved() {
            Qt.callLater(addDocumentFile)
        }

        function onDocumentJustLoaded() {
            Qt.callLater(addDocumentFile)
        }

        function addDocumentFile() {
            const docFilePath = Scrite.document.fileName
            if(docFilePath !== "")
                add(docFilePath)

            // Remove this file from missing files list.
            if(Array.isArray(missingFiles) || missingFiles.length) {
                let f = missingFiles
                missingFiles = f.filter(item => item === docFilePath);
            }
        }

        function captureChangeInFiles() {
            _recentFilesSettings.files = files
        }

        notifyMissingFiles: _applicationSettings.notifyMissingRecentFiles
        source: ScriteFileListModel.RecentFiles

        onFilesMissing: (files) => {
            let f = Array.isArray(missingFiles) || missingFiles.length ? missingFiles : []
            f.push(...files)
            missingFiles = f
        }

        onNotifyMissingFilesChanged: {
            if(!notifyMissingFiles)
                missingFiles = []
        }
    }

    readonly property LibraryService libraryService : LibraryService { }

    // This model is how the screenplay of the current ScriteDocument is accessed.
    readonly property ScreenplayAdapter screenplayAdapter: ScreenplayAdapter {
        property string sessionId

        source: {
            // if(Scrite.document.sessionId !== sessionId)
            //     return null

            if(mainWindowTab !== Runtime.MainWindowTab.ScreenplayTab && Scrite.document.screenplay.currentElementIndex < 0) {
                let index = Scrite.document.structure.currentElementIndex
                let element = Scrite.document.structure.elementAt(index)
                if(element) {
                    if(element.scene.addedToScreenplay) {
                        Scrite.document.screenplay.currentElementIndex = element.scene.screenplayElementIndexList[0]
                        return Scrite.document.screenplay
                    }
                    return element.scene
                }
            }

            return Scrite.document.screenplay
        }
    }

    readonly property ScreenplayPaginator paginator : ScreenplayPaginator {
        property bool paused: false
        property ScreenplayElement currentElement: screenplay !== null ? Runtime.screenplayAdapter.currentElement : null

        enabled: !paused && !Scrite.document.loading
        format: Scrite.document.printFormat
        screenplay: Runtime.screenplayAdapter.screenplay
        cursorPosition: currentElement ? (currentElement.scene ? Math.max(currentElement.scene.cursorPosition,0) : 0) : -1

        function toggle() { paused = !paused }
        function pause() { paused = true }
        function resume() { paused = false }
    }

    // Announcement IDs
    readonly property QtObject announcementIds: QtObject {
        readonly property string characterNotesRequest: "7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8"
        readonly property string closeDialogBoxRequest: "A6456A87-FC8C-405B-BDD7-7625F86272BA"
        readonly property string closeHomeScreenRequest: "4F8F6B5B-5BEB-4D01-97BA-B0018241BD38"
        readonly property string embeddedTabRequest: "190B821B-50FE-4E47-A4B2-BDBB2A13B72C"
        readonly property string englishFontFamilyChanged: "763E8FAD-8681-4F64-B574-F9BB7CF8A7F1"
        readonly property string focusRequest: "2E3BBE4F-05FE-49EE-9C0E-3332825B72D8"
        readonly property string loginRequest: "97369507-721E-4A7F-886C-4CE09A5BCCFB"
        readonly property string notebookNodeRequest: "1DC67418-2584-4598-A68A-DE5205BBC028"
        readonly property string reloadMainUiRequest: "9A7F0F35-346F-461D-BB85-F5C6DC08A01D"
        readonly property string sceneNotesRequest: "41EE5E06-FF97-4DB6-B32D-F938418C9529"
        readonly property string sceneTextEditorReceivedFocus: "598E1699-465B-40D5-8CF4-E9753E2C16E7"
        readonly property string showHelpTip: "B168E17C-14CA-454F-9DF8-CAA381D9A8A2"
        readonly property string notebookRequest: "ABCD190B821B-50FE-4E47-A4B2-BDBB2A13B72C"
        readonly property string userAccountDialogScreen: "24A8C9F3-1F62-4B14-A65E-250E53350152"
        readonly property string userProfileScreenPage: "D97FD221-5257-4A20-B9A2-744594E99D76"
    }

    readonly property QtObject announcementData: QtObject {
        readonly property QtObject focusOptions: QtObject {
            readonly property string addMuteCharacter: "Add Mute Character"
            readonly property string addSceneTag: "Add Scene Tag"
            readonly property string scene: "Scene"
            readonly property string sceneHeading: "Scene Heading"
            readonly property string sceneNumber: "Scene Number"
            readonly property string sceneSynopsis: "Scene Synopsis"
        }
    }

    // Global file-manager
    readonly property FileManager fileNamager: FileManager {

    }

    function init(_parent) {
        if( !(_parent && Object.isOfType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        parent = _parent
        visible = false
        anchors.fill = parent
    }

    function closeAllDialogs() {
        Runtime.shoutout(Runtime.announcementIds.closeDialogBoxRequest, undefined)
    }

    function requiresUserOnboarding() {
        return "required" === userAccountDialogSettings.userOnboardingStatus
    }

    function estimateTypeSize(itemNameOrCode, imports) {
        const defaultImports = ["QtQuick 2.15",
                                "QtQuick.Controls 2.15",
                                "QtQuick.Controls.Material 2.15",
                                "io.scrite.components 1.0",
                                "\"qrc:/qml/globals\"",
                                "\"qrc:/qml/helpers\"",
                                "\"qrc:/qml/controls\""]
        if(imports === undefined)
            imports = defaultImports
        else
            imports = imports.concat(defaultImports)

        let code = ""
        for(let i=0; i<imports.length; i++) {
            code += "import " + imports[i] + "\n"
        }

        if(itemNameOrCode.indexOf("{") >= 0)
            code += itemNameOrCode + "\n"
        else
            code += itemNameOrCode + " { visible: false }\n"

        let instance = Qt.createQmlObject(code, root)
        let instanceSize = Qt.size(0, 0)
        if(instance) {
            instanceSize = Qt.size(instance.width, instance.height)
            instance.destroy()
        }

        return instanceSize
    }

    function activateMainWindowTab(tab) {
        if(mainWindowTab !== tab) {
            _mainWindowTabSwitchTask.targetTab = tab
            _mainWindowTabSwitchTask.start()
        }
    }

    function shoutout(type, data, delay) {
        if(typeof delay === "number" && delay > 0) {
            execLater(root, delay, (args) => {
                          root.shoutout(args[0], args[1], 0)
                      }, [type, data])
        } else {
            Announcement.shout(type, data)
        }
    }

    function shoutoutLater(type, data) {
        shoutout(type, data, stdAnimationDuration + 50)
    }

    function showHelpTip(tipName) {
        if(helpTips !== undefined)
            Runtime.shoutout(Runtime.announcementIds.showHelpTip, tipName)
    }

    function execLater(contextObject, delay, callback, args) {
        let timer = Qt.createQmlObject("import QtQml 2.15; Timer { }", contextObject);
        timer.interval = delay === undefined ? 100 : delay
        timer.repeat = false
        timer.triggered.connect(() => {
            if (args)
                callback(args)
                else callback()
                timer.destroy()
        })
        timer.start()

        return timer
    }

    function margins(left, top, right, bottom) {
        return { "top": top, "left": left, "right": right, "bottom": bottom }
    }

    function newAnnotation(parent, type, geometry, config) {
        if(!parent || !type || !geometry)
            return null

        let annot = Qt.createQmlObject("import io.scrite.components 1.0; Annotation { objectName: \"ica\" }", parent)
        annot.type = type
        annot.geometry = geometry
        if(config) {
            for(member in config)
                annot.setAttribute(member, config[member])
        }

        return annot
    }

    function bounded(min, val, max) {
        return Math.min(Math.max(min, val), max)
    }

    function todayWithZeroTime() {
        let today = new Date();
        today.setHours(0, 0, 0, 0);
        return today;
    }

    function formatDate(date) {
        const months =
                [ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];

        const day = date.getDate();
        const month = months[date.getMonth()];

        return day + " " + month;
    }

    function formatDateIncludingYear(date) {
        return formatDate(date) + " " + date.getFullYear();
    }

    function formatDateRangeAsString(start_date, end_date) {
        if (typeof end_date === "number") {
            const nrDays = end_date

            end_date = new Date(start_date)
            end_date.setDate(start_date.getDate() + nrDays)
        }

        if (start_date.getFullYear() === end_date.getFullYear()) {
            return formatDate(start_date) + " - " + formatDateIncludingYear(end_date)
        }

        return formatDateIncludingYear(start_date) + " - " + formatDateIncludingYear(end_date)
    }

    function daysSpanAsString(nrDays) {
        let ret = ""
        if (nrDays < 0) {
            ret = "Already"
        } else if (nrDays === 0) { ret = "Today" }
        else if (nrDays === 1) { ret = "Tomorrow" }
        else {
            const years = Math.floor(nrDays / 365)
            const days = nrDays % 365
            if (years == 0) {
                ret = days + " days"
            } else {
                if (years === 1) {
                    ret = "1 year"
                } else {
                    ret = years + " years"
                }

                if (days > 1) {
                    ret += ", and " + days + " days"
                } else if (days === 1) {
                    ret += ", and 1 day"
                }
            }
        }

        return ret
    }

    function daysBetween(start_date, end_date) {
        let from = new Date(start_date);
        let until = new Date(end_date);

        from.setHours(0, 0, 0, 0)
        until.setHours(0, 0, 0, 0)

        return Math.ceil((until - from) / (1000 * 60 * 60 * 24));
    }

    function dateSpanAsString(start_date, end_date) {
        const nr_days_remaining = daysBetween(start_date, end_date)
        return daysSpanAsString(nr_days_remaining)
    }

    function toTitleCase(str) {
        return str
                .toLowerCase() // Convert the entire string to lowercase
                .split(' ') // Split into words by spaces
                .map(word => word.charAt(0).toUpperCase()
                             + word.slice(1)) // Capitalize the first letter of each word
                .join(' '); // Join the words back with spaces
    }

    function validateEmail(email) {
        const emailRegex =
                /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
        return emailRegex.test(email);
    }

    visible: false

    onShowNotebookInStructureChanged: activateMainWindowTab(Runtime.MainWindowTab.ScreenplayTab)

    // Private objects
    Settings {
        id: _recentFilesSettings

        property var files: []

        property alias missingFiles: _recentFiles.missingFiles
        property alias preferTitleVersionText: _recentFiles.preferTitleVersionText

        category: "RecentFiles"
        fileName: Platform.settingsFile
    }

    SequentialAnimation {
        id: _mainWindowTabSwitchTask

        property int targetTab: -1

        loops: 1

        ScriptAction {
            script: root.aboutToSwitchTab(root.mainWindowTab, _mainWindowTabSwitchTask.targetTab)
        }

        PauseAnimation {
            duration: root.stdAnimationDuration/2
        }

        ScriptAction {
            script: root.mainWindowTab = _mainWindowTabSwitchTask.targetTab
        }

        PauseAnimation {
            duration: root.stdAnimationDuration/2
        }

        ScriptAction {
            script: {
                root.finishedTabSwitch(_mainWindowTabSwitchTask.targetTab)
                _mainWindowTabSwitchTask.targetTab = -1
            }
        }
    }

    Connections {
        target: Scrite.user

        enabled: root.helpTips === undefined

        function onLoggedInChanged() {
            if(Scrite.user.loggedIn) {
                let api = Qt.createQmlObject("import io.scrite.components 1.0; UserHelpTipsRestApiCall {}", root)
                api.finished.connect( () => {
                                          root.helpTips = api.helpTips
                                          api.destroy()
                                      })
                if(!api.call())
                    api.destroy()
            }
        }
    }

    readonly property Connections documentConnections: Connections {
        target: Scrite.document

        function onJustReset() {
            Runtime.notebookSettings.characterPageTab = 0
            Runtime.notebookSettings.charactersPageTab = 0
            Runtime.notebookSettings.screenplayPageTab = 0
            Runtime.notebookSettings.sceneNotesPageTab = 0
            Runtime.notebookSettings.sceneSynopsisTabIndex = 0
            Runtime.screenplayEditorSettings.sceneSidePanelOpen = false
            Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScreenplayTab)

            Scrite.document.displayFormat.activeLanguageCode = Runtime.language.activeLanguageCode
        }

        function onJustLoaded() {
            Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
        }
    }
}
